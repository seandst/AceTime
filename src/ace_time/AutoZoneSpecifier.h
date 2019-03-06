#ifndef ACE_TIME_AUTO_ZONE_SPECIFIER_H
#define ACE_TIME_AUTO_ZONE_SPECIFIER_H

#include <Arduino.h>
#include <string.h> // strchr()
#include <stdint.h>
#include "common/ZonePolicy.h"
#include "common/ZoneInfo.h"
#include "UtcOffset.h"
#include "LocalDate.h"
#include "OffsetDateTime.h"
#include "ZoneSpecifier.h"
#include "common/logger.h"

class AutoZoneSpecifierTest_init_primitives;
class AutoZoneSpecifierTest_init;
class AutoZoneSpecifierTest_createAbbreviation;
class AutoZoneSpecifierTest_calcStartDayOfMonth;
class AutoZoneSpecifierTest_calcRuleOffsetCode;

namespace ace_time {

namespace internal {

/**
 * Data structure that captures the matching ZoneEra and its ZoneRule
 * transitions for a given year. Can be cached based on the year.
 */
struct ZoneMatch {
  /**
   * Longest abbreviation seems to be 5 characters.
   * https://www.timeanddate.com/time/zones/
   */
  static const uint8_t kAbbrevSize = 5 + 1;

  /** The ZoneEra that matched the given year. NonNullable. */
  const common::ZoneEra* era;

  /**
   * The Zone transition rule that matched for the the given year. Set to
   * nullptr if the RULES column is '-'. We do not support a RULES column that
   * contains a UTC offset. There are only 2 time zones that has this property
   * as of version 2018g: Europe/Istanbul and America/Argentina/San_Luis.
   */
  const common::ZoneRule* rule;

  /** Year which applies to the ZoneEra or ZoneRule. */
  int8_t yearTiny;

  /** The calculated transition time of the given rule. */
  acetime_t startEpochSeconds;

  /** The calculated effective UTC offsetCode at the start of transition. */
  int8_t offsetCode;

  /** The calculated effective time zone abbreviation, e.g. "PST" or "PDT". */
  char abbrev[kAbbrevSize];

  /** Used only for debugging. */
  void log() const {
    if (sizeof(acetime_t) == sizeof(int)) {
      common::logger("startEpochSeconds: %d", startEpochSeconds);
    } else {
      common::logger("startEpochSeconds: %ld", startEpochSeconds);
    }
    common::logger("offsetCode: %d", offsetCode);
    common::logger("abbrev: %s", abbrev);
    if (rule != nullptr) {
      common::logger("Rule.fromYear: %d", rule->fromYearTiny);
      common::logger("Rule.toYear: %d", rule->toYearTiny);
      common::logger("Rule.inMonth: %d", rule->inMonth);
      common::logger("Rule.onDayOfMonth: %d", rule->onDayOfMonth);
    }
  }
};

}

/**
 * Manages a given ZoneInfo. The ZoneRule and ZoneEra records that match the
 * year of the given epochSeconds are cached internally for performance. The
 * expectation is that repeated calls to the various methods will have
 * epochSeconds which do not vary too greatly and will occur in the same year.
 *
 * The Rule records are transition points which look like this:
 * @verbatim
 * Rule  NAME  FROM    TO  TYPE    IN     ON        AT      SAVE    LETTER/S
 * @endverbatim
 *
 * Each record is represented by common::ZoneRule and the entire
 * collection is represented by common::ZonePolicy.
 *
 * The Zone records define the region which follows a specific set of Rules
 * for certain time periods (given by UNTIL below):
 * @verbatim
 * Zone NAME              GMTOFF    RULES FORMAT  [UNTIL]
 * @endverbatim
 *
 * Each record is represented by common::ZoneEra and the entire collection is
 * represented by common::ZoneInfo.
 *
 * Limitations:
 *
 *  - Zone untilTimeModifier works only for 'w' (not 's' or 'u')
 *  - but Rule atTimeModifier supports all three ('w', 's', and 'u')
 *  - Zone UNTIL field supports only year component, not month, day, or time
 *  - RULES column supports only a named Rule reference, not an offset (hh:mm)
 *
 * Not thread-safe.
 */
class AutoZoneSpecifier: public ZoneSpecifier {
  public:
    /**
     * Constructor.
     * @param zoneInfo pointer to a ZoneInfo. Can be nullptr which is
     * interpreted as UTC.
     */
    explicit AutoZoneSpecifier(const common::ZoneInfo* zoneInfo = nullptr):
        mZoneInfo(zoneInfo) {}

    /**
     * Copy constructor. This is needed because some applications (e.g.
     * WorldClock) find it convenient to initialize the various ZoneSpecifiers
     * in a setup() method, after the ZoneSpecifier variables have been already
     * allocated. To reset the specifiers, we need a copy constructor. If
     * deferred initialization is removed, then this copy constructor can be
     * removed as well.
     */
    explicit AutoZoneSpecifier(const AutoZoneSpecifier& that):
      mZoneInfo(that.mZoneInfo),
      mIsFilled(false) {}

    /** Return the underlying ZoneInfo. */
    const common::ZoneInfo* getZoneInfo() const { return mZoneInfo; }

    uint8_t getType() const override { return kTypeAuto; }

    /** Return the UTC offset at epochSeconds. */
    UtcOffset getUtcOffset(acetime_t epochSeconds) {
      if (mZoneInfo == nullptr) return UtcOffset();
      const internal::ZoneMatch* zoneMatch = getZoneMatch(epochSeconds);
      return UtcOffset::forOffsetCode(zoneMatch->offsetCode);
    }

    /** Return the DST delta offset at epochSeconds. */
    UtcOffset getDeltaOffset(acetime_t epochSeconds) {
      if (mZoneInfo == nullptr) return UtcOffset();
      const internal::ZoneMatch* zoneMatch = getZoneMatch(epochSeconds);
      if (zoneMatch->rule == nullptr) return UtcOffset();
      return UtcOffset::forOffsetCode(zoneMatch->rule->deltaCode);
    }

    /** Return the time zone abbreviation. */
    const char* getAbbrev(acetime_t epochSeconds) {
      if (mZoneInfo == nullptr) return "UTC";
      const internal::ZoneMatch* zoneMatch = getZoneMatch(epochSeconds);
      return zoneMatch->abbrev;
    }

    /** Used only for debugging. */
    void log() const {
      if (!mIsFilled) {
        common::logger("*not initialized*");
        return;
      }
      common::logger("mYear: %d", mYear);
      common::logger("mNumMatches: %d", mNumMatches);
      common::logger("---- mPrevMatch");
      mPreviousMatch.log();
      for (int i = 0; i < mNumMatches; i++) {
        common::logger("---- Match: %d", i);
        mMatches[i].log();
      }
    }

  private:
    friend class ::AutoZoneSpecifierTest_init_primitives;
    friend class ::AutoZoneSpecifierTest_init;
    friend class ::AutoZoneSpecifierTest_createAbbreviation;
    friend class ::AutoZoneSpecifierTest_calcStartDayOfMonth;
    friend class ::AutoZoneSpecifierTest_calcRuleOffsetCode;
    friend bool operator==(const AutoZoneSpecifier& a,
        const AutoZoneSpecifier& b);

    static const uint8_t kMaxCacheEntries = 4;

    /**
     * The smallest ZoneMatch.startEpochSeconds which represents -Infinity.
     * Can't use INT32_MIN because that is used internally to indicate
     * "invalid".
     */
    static const acetime_t kMinEpochSeconds = INT32_MIN + 1;

    /** Return the ZoneMatch at the given epochSeconds. */
    const internal::ZoneMatch* getZoneMatch(acetime_t epochSeconds) {
      LocalDate ld = LocalDate::forEpochSeconds(epochSeconds);
      init(ld);
      return findMatch(epochSeconds);
    }

    /**
     * Initialize the zone rules cache, keyed by the "current" year.
     *
     * If the UTC date is 1/1, the local date could be the previous year.
     * Unfortunately, there are some countries that decided to make a time
     * change on 12/31, e.g. Dhaka). So, let's assume that there are no DST
     * transitions on 1/1, consider the "current year" to be the previous year,
     * extract the various rules based upon that year, and determine the DST
     * offset using the matching rules of the previous year.
     */
    void init(const LocalDate& ld) {
      int16_t year = ld.year();
      if (ld.month() == 1 && ld.day() == 1) {
        year--;
      }

      if (!isFilled(year)) {
        mYear = year;
        mNumMatches = 0; // clear cache

        addRulePriorToYear(year);
        addRulesForYear(year);
        calcTransitions();
        calcAbbreviations();
        mIsFilled = true;
      }
    }

    /** Check if the ZoneRule cache is filled for the given year. */
    bool isFilled(int16_t year) const {
      return mIsFilled && (year == mYear);
    }

    /**
     * Add the last matching rule just prior to the given year. This determines
     * the offset at the beginning of the current year.
     */
    void addRulePriorToYear(int16_t year) {
      int8_t yearTiny = year - LocalDate::kEpochYear;
      int8_t priorYearTiny = yearTiny - 1;

      // Find the prior Era.
      const common::ZoneEra* const era = findZoneEraPriorTo(year);

      // If the prior ZoneEra is a simple Era (no zone policy), then create a
      // ZoneMatch using a rule==nullptr. Otherwise, find the latest rule
      // within the ZoneEra.
      const common::ZonePolicy* const zonePolicy = era->zonePolicy;
      const common::ZoneRule* latest = nullptr;
      if (zonePolicy != nullptr) {
        // Find the latest rule for the matching ZoneEra whose
        // ZoneRule::toYearTiny < yearTiny. Assume that there are no more than
        // 1 rule per month.
        for (uint8_t i = 0; i < zonePolicy->numRules; i++) {
          const common::ZoneRule* const rule = &zonePolicy->rules[i];
          // Check if rule is effective prior to the given year
          if (rule->fromYearTiny < yearTiny) {
            if ((latest == nullptr)
                || compareZoneRule(year, rule, latest) > 0) {
              latest = rule;
            }
          }
        }
      }
      mPreviousMatch = {
        era,
        latest /*rule*/,
        priorYearTiny /*yearTiny*/,
        0 /*epochSeconds*/,
        0 /*offsetCode*/,
        {0} /*abbrev*/
      };
    }

    /** Compare two ZoneRules which are valid prior to the given year. */
    static int8_t compareZoneRule(int16_t year,
        const common::ZoneRule* a, const common::ZoneRule* b) {
      int16_t aYear = effectiveRuleYear(year, a);
      int16_t bYear = effectiveRuleYear(year, b);
      if (aYear < bYear) return -1;
      if (aYear > bYear) return 1;
      if (a->inMonth < b->inMonth) return -1;
      if (a->inMonth > b->inMonth) return 1;
      return 0;
    }

    /**
     * Return the largest effective year of the rule, prior to given year.
     * Return 0 if rule is greater than the given year.
     */
    static int16_t effectiveRuleYear(int16_t year,
        const common::ZoneRule* rule) {
      int8_t yearTiny = year - LocalDate::kEpochYear;
      if (rule->toYearTiny < yearTiny) {
        return rule->toYearTiny + LocalDate::kEpochYear;
      }
      if (rule->fromYearTiny < yearTiny) {
        return year - 1;
      }
      return 0;
    }

    /** Add all matching rules from the current year. */
    void addRulesForYear(int16_t year) {
      const common::ZoneEra* const era = findZoneEra(year);

      // If the ZonePolicy has no rules, then we need to add a Transition which
      // takes effect at the start time of the current year.
      const common::ZonePolicy* const zonePolicy = era->zonePolicy;
      if (zonePolicy == nullptr) {
        addRule(year, era, nullptr);
        return;
      }

      // Find all matching transition rules, and add them to the mMatches list,
      // in sorted order according to the ZoneRule::inMonth field.
      int8_t yearTiny = year - LocalDate::kEpochYear;
      for (uint8_t i = 0; i < zonePolicy->numRules; i++) {
        const common::ZoneRule* const rule = &zonePolicy->rules[i];
        if ((rule->fromYearTiny <= yearTiny) &&
            (yearTiny <= rule->toYearTiny)) {
          addRule(year, era, rule);
        }
      }
    }

    /**
     * Add (era, rule) to the cache, in sorted order according to the
     * 'ZoneRule::inMonth' field. This assumes that there are no more than one
     * transition per month.
     *
     * Essentially, this is doing an Insertion Sort of the ZoneMatch elements.
     * Even through it is O(N^2), for small number of ZoneMatch elements, this
     * is faster than than the O(N log(N)) algorithms such as Merge Sort, Heap
     * Sort, Quick Sort. The nice property of this Insertion Sort is that if
     * the ZoneInfoEntries are already sorted, then the loop terminates early
     * and the total sort time is O(N).
     */
    void addRule(int16_t year, const common::ZoneEra* era,
          const common::ZoneRule* rule) const {
      if (mNumMatches >= kMaxCacheEntries) return;

      // insert new element at the end of the list
      int8_t yearTiny = year - LocalDate::kEpochYear;
      mMatches[mNumMatches] = {
        era,
        rule,
        yearTiny,
        0 /*epochSeconds*/,
        0 /*offsetCode*/,
        {0} /*abbrev*/
      };
      mNumMatches++;

      // perform an insertion sort
      for (uint8_t i = mNumMatches - 1; i > 0; i--) {
        internal::ZoneMatch& left = mMatches[i - 1];
        internal::ZoneMatch& right = mMatches[i];
        // assume only 1 rule per month
        if ((left.rule != nullptr && right.rule != nullptr &&
              left.rule->inMonth > right.rule->inMonth)
            || (left.rule != nullptr && right.rule == nullptr)) {
          internal::ZoneMatch tmp = left;
          left = right;
          right = tmp;
        }
      }
    }

    /**
     * Find the ZoneEra which applies to the given year. The era will
     * satisfy (year < ZoneEra.untilYearTiny + kEpochYear). Since the
     * largest untilYearTiny is 127, the largest supported 'year' is 2126.
     */
    const common::ZoneEra* findZoneEra(int16_t year) const {
      for (uint8_t i = 0; i < mZoneInfo->numEras; i++) {
        const common::ZoneEra* era = &mZoneInfo->eras[i];
        if (year < era->untilYearTiny + LocalDate::kEpochYear) return era;
      }
      return nullptr;
    }

    /**
     * Find the most recent ZoneEra which was in effect just before the
     * beginning of the given year, in other words, just before {year}-01-01
     * 00:00:00. It will be first era just after the latest era whose untilYear
     * < year. Since the ZoneEras are in increasing order of untilYear, this is
     * the same as matching the first ZoneEra whose untilYear >= year.
     *
     * This should never return nullptr because the code generator for
     * zone_infos.cpp verified that the final ZoneEra contains an empty
     * untilYear, interpreted as 'max', and set to 127.
     */
    const common::ZoneEra* findZoneEraPriorTo(int16_t year) const {
      for (uint8_t i = 0; i < mZoneInfo->numEras; i++) {
        const common::ZoneEra* era = &mZoneInfo->eras[i];
        if (year <= era->untilYearTiny + LocalDate::kEpochYear) return era;
      }
      return nullptr;
    }

    /** Calculate the transitional epochSeconds of each ZoneMatch rule. */
    void calcTransitions() {
      mPreviousMatch.startEpochSeconds = kMinEpochSeconds;
      mPreviousMatch.offsetCode = mPreviousMatch.era->offsetCode;
      mPreviousMatch.offsetCode += (mPreviousMatch.rule == nullptr)
            ? 0 : mPreviousMatch.rule->deltaCode;
      const internal::ZoneMatch* previousMatch = &mPreviousMatch;

      // Loop through ZoneMatch items to calculate 2 things:
      // 1) ZoneMatch::startEpochSeconds
      // 2) ZoneMatch::offsetCode
      for (uint8_t i = 0; i < mNumMatches; i++) {
        internal::ZoneMatch& match = mMatches[i];
        const int16_t year = match.yearTiny + LocalDate::kEpochYear;

        if (match.rule == nullptr) {
          const int8_t offsetCode = calcRuleOffsetCode(
              previousMatch->offsetCode,
              match.era->offsetCode,
              'w' /*modifier*/);
          OffsetDateTime startDateTime = OffsetDateTime::forComponents(
              year, 1, 1, 0, 0, 0,
              UtcOffset::forOffsetCode(offsetCode));
          match.startEpochSeconds = startDateTime.toEpochSeconds();
          match.offsetCode = match.era->offsetCode;
        } else {
          // Determine the start date of the rule.
          const uint8_t startDayOfMonth = calcStartDayOfMonth(
              year, match.rule->inMonth, match.rule->onDayOfWeek,
              match.rule->onDayOfMonth);

          // Determine the offset of the 'atTimeModifier'. The 'w' modifier
          // requires the offset of the previous match.
          const int8_t offsetCode = calcRuleOffsetCode(
              previousMatch->offsetCode,
              match.era->offsetCode,
              match.rule->atTimeModifier);

          // startDateTime
          const uint8_t atHour = match.rule->atTimeCode / 4;
          const uint8_t atMinute = (match.rule->atTimeCode % 4) * 15;
          OffsetDateTime startDateTime = OffsetDateTime::forComponents(
              year, match.rule->inMonth, startDayOfMonth,
              atHour, atMinute, 0 /*second*/,
              UtcOffset::forOffsetCode(offsetCode));
          match.startEpochSeconds = startDateTime.toEpochSeconds();

          // Determine the effective offset code
          match.offsetCode = match.era->offsetCode + match.rule->deltaCode;
        }

        previousMatch = &match;
      }
    }

    /**
     * Calculate the actual dayOfMonth of the expresssion
     * (onDayOfWeek >= onDayOfMonth). The "last{dayOfWeek}" expression is
     * expressed by onDayOfMonth being 0. An exact match on dayOfMonth is
     * expressed by setting onDayOfWeek to 0.
     */
    static uint8_t calcStartDayOfMonth(int16_t year, uint8_t month,
        uint8_t onDayOfWeek, uint8_t onDayOfMonth) {
      if (onDayOfWeek == 0) return onDayOfMonth;


      // Convert "last{Xxx}" to "last{Xxx}>={daysInMonth-6}".
      if (onDayOfMonth == 0) {
        onDayOfMonth = LocalDate::daysInMonth(year, month) - 6;
      }

      LocalDate limitDate = LocalDate::forComponents(
          year, month, onDayOfMonth);
      uint8_t dayOfWeekShift = (onDayOfWeek - limitDate.dayOfWeek() + 7) % 7;
      return onDayOfMonth + dayOfWeekShift;
    }

    /**
     * Determine the offset of the 'atTimeModifier'. If 'w', then we
     * must use the offset of the *previous* zone rule. If 's', use the
     * current base offset. If 'u', 'g', 'z', then use 0 offset.
     */
    static int8_t calcRuleOffsetCode(int8_t prevEffectiveOffsetCode,
        int8_t currentBaseOffsetCode, uint8_t modifier) {
      if (modifier == 'w') {
        return prevEffectiveOffsetCode;
      } else if (modifier == 's') {
        // TODO: Is this right, use the current matched base offset code?
        return currentBaseOffsetCode;
      } else { // 'u', 'g' or 'z'
        return 0;
      }
    }

    /** Determine the time zone abbreviations. */
    void calcAbbreviations() {
      calcAbbreviation(&mPreviousMatch);
      for (uint8_t i = 0; i < mNumMatches; i++) {
        calcAbbreviation(&mMatches[i]);
      }
    }

    /** Calculate the time zone abbreviation of the current zoneMatch. */
    static void calcAbbreviation(internal::ZoneMatch* zoneMatch) {
      createAbbreviation(
          zoneMatch->abbrev,
          internal::ZoneMatch::kAbbrevSize,
          zoneMatch->era->format,
          zoneMatch->rule != nullptr ? zoneMatch->rule->deltaCode : 0,
          zoneMatch->rule != nullptr ? zoneMatch->rule->letter : '\0');
    }

    /**
     * Create the time zone abbreviation in dest from the format string
     * (e.g. "P%T", "E%T"), the time zone deltaCode (!= 0 means DST), and the
     * replacement letter (e.g. 'S', 'D', or '-').
     *
     * @param dest destination string buffer
     * @param destSize size of buffer
     * @param format encoded abbreviation, '%' is a character substitution
     * @param deltaCode the offsetCode (0 for standard, != 0 for DST)
     * @param letter during standard or DST time ('S', 'D', '-' for no
     *        substitution, or '\0' when zoneMatch.rule == nullptr)
     */
    static void createAbbreviation(char* dest, uint8_t destSize,
        const char* format, uint8_t deltaCode, char letter) {
      if (deltaCode == 0 && letter == '\0') {
        strncpy(dest, format, destSize);
        dest[destSize - 1] = '\0';
        return;
      }

      if (strchr(format, '%') != nullptr) {
        copyAndReplace(dest, destSize, format, '%', letter);
      } else {
        const char* slashPos = strchr(format, '/');
        if (slashPos != nullptr) {
          if (deltaCode == 0) {
            copyAndReplace(dest, destSize, format, '/', '\0');
          } else {
            memmove(dest, slashPos+1, strlen(slashPos+1) + 1);
          }
        } else {
          strncpy(dest, format, destSize);
          dest[destSize - 1] = '\0';
        }
      }
    }

    /**
     * Copy at most dstSize characters from src to dst, while replacing all
     * occurance of oldChar with newChar. If newChar is '-', then replace with
     * nothing. The resulting dst string is always NUL terminated.
     */
    static void copyAndReplace(char* dst, uint8_t dstSize, const char* src,
        char oldChar, char newChar) {
      while (*src != '\0' && dstSize > 0) {
        if (*src == oldChar) {
          if (newChar == '-') {
            src++;
          } else {
            *dst = newChar;
            dst++;
            src++;
            dstSize--;
          }
        } else {
          *dst++ = *src++;
          dstSize--;
        }
      }

      if (dstSize == 0) {
        --dst;
      }
      *dst = '\0';
    }

    /** Search the cache and find closest ZoneMatch. */
    const internal::ZoneMatch* findMatch(acetime_t epochSeconds) const {
      const internal::ZoneMatch* closestMatch = &mPreviousMatch;
      for (uint8_t i = 0; i < mNumMatches; i++) {
        const internal::ZoneMatch* m = &mMatches[i];
        if (m->startEpochSeconds <= epochSeconds) {
          closestMatch = m;
        }
      }
      return closestMatch;
    }

    const common::ZoneInfo* mZoneInfo;

    mutable int16_t mYear = 0;
    mutable bool mIsFilled = false;
    mutable uint8_t mNumMatches = 0;
    mutable internal::ZoneMatch mMatches[kMaxCacheEntries];
    mutable internal::ZoneMatch mPreviousMatch; // previous year's match
};

inline bool operator==(const AutoZoneSpecifier& a, const AutoZoneSpecifier& b) {
  return a.getZoneInfo() == b.getZoneInfo();
}

inline bool operator!=(const AutoZoneSpecifier& a, const AutoZoneSpecifier& b) {
  return ! (a == b);
}

}

#endif

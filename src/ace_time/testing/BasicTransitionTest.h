/*
 * MIT License
 * Copyright (c) 2019 Brian T. Park
 */

#ifndef ACE_TIME_BASIC_TRANSITION_TEST_H
#define ACE_TIME_BASIC_TRANSITION_TEST_H

#include <AUnit.h>
#include <AceTime.h>
#include "ValidationDataType.h"

#define BASIC_TRANSITION_TEST_DEBUG 0

namespace ace_time {
namespace testing {

class BasicTransitionTest: public aunit::TestOnce {
  protected:
    void assertValid(
        const basic::ZoneInfo* const zoneInfo,
        const ValidationData* testData,
        bool validateDst = false) {

      if (BASIC_TRANSITION_TEST_DEBUG) {
        enableVerbosity(aunit::Verbosity::kAssertionPassed);
      }

      BasicZoneProcessor zoneProcessor;
      TimeZone tz = TimeZone::forZoneInfo(zoneInfo, &zoneProcessor);
      for (uint16_t i = 0; i < testData->numItems; i++) {
        const ValidationItem& item = testData->items[i];
        acetime_t epochSeconds = item.epochSeconds;

        // Call getUtcOffset() to prime the ZoneProcessor cache, before
        // zoneProcessor.log() is called.
        TimeOffset timeOffset = tz.getUtcOffset(epochSeconds);

        if (BASIC_TRANSITION_TEST_DEBUG) {
          ace_time::logging::printf("==== test index: %d\n", i);
          if (sizeof(acetime_t) == sizeof(int)) {
            ace_time::logging::printf("epochSeconds: %d\n", epochSeconds);
          } else {
            ace_time::logging::printf("epochSeconds: %ld\n", epochSeconds);
          }
          zoneProcessor.log();
        }

        // Verify total UTC timeOffset
        assertEqual(item.timeOffsetMinutes, timeOffset.toMinutes());

        // Verify DST offset.
        if (validateDst) {
          TimeOffset deltaOffset = tz.getDeltaOffset(epochSeconds);
          assertEqual(item.deltaOffsetMinutes, deltaOffset.toMinutes());
        }

        // Verify date components
        ZonedDateTime dt = ZonedDateTime::forEpochSeconds(epochSeconds, tz);
        assertEqual(item.year, dt.year());
        assertEqual(item.month, dt.month());
        assertEqual(item.day, dt.day());
        assertEqual(item.hour, dt.hour());
        assertEqual(item.minute, dt.minute());
        assertEqual(item.second, dt.second());

        // Verify abbreviation if it is defined.
        if (item.abbrev != nullptr) {
          assertEqual(item.abbrev, tz.getAbbrev(epochSeconds));
        }
      }
    }
};

}
}

#undef BASIC_TRANSITION_TEST_DEBUG

#endif

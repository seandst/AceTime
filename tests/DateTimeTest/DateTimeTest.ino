#line 2 "DateTimeTest.ino"

#include <AUnit.h>
#include <AceTime.h>

using namespace aunit;
using namespace ace_time;

// --------------------------------------------------------------------------
// OffsetDateTime
// --------------------------------------------------------------------------

test(OffsetDateTimeTest, accessors) {
  OffsetDateTime dt = OffsetDateTime::forComponents(2001, 2, 3, 4, 5, 6);
  assertEqual((int16_t) 2001, dt.year());
  assertEqual(1, dt.yearShort());
  assertEqual(2, dt.month());
  assertEqual(3, dt.day());
  assertEqual(4, dt.hour());
  assertEqual(5, dt.minute());
  assertEqual(6, dt.second());
  assertEqual(0, dt.utcOffset().toOffsetCode());
}

test(OffsetDateTimeTest, invalidSeconds) {
  OffsetDateTime dt = OffsetDateTime::forEpochSeconds(
      LocalDate::kInvalidEpochSeconds);
  assertTrue(dt.isError());
  assertEqual(LocalDate::kInvalidEpochSeconds, dt.toEpochSeconds());
  assertEqual(LocalDate::kInvalidEpochDays, dt.toEpochDays());
}

test(OffsetDateTimeTest, setError) {
  OffsetDateTime dt = OffsetDateTime().setError();
  assertTrue(dt.isError());
}

test(OffsetDateTimeTest, isError) {
  // 2018-01-01 00:00:00Z
  OffsetDateTime dt = OffsetDateTime::forComponents(2018, 1, 1, 0, 0, 0);
  assertFalse(dt.isError());

  dt = OffsetDateTime::forComponents(2018, 0, 1, 0, 0, 0);
  assertTrue(dt.isError());

  dt = OffsetDateTime::forComponents(2018, 255, 1, 0, 0, 0);
  assertTrue(dt.isError());

  dt = OffsetDateTime::forComponents(2018, 1, 0, 0, 0, 0);
  assertTrue(dt.isError());

  dt = OffsetDateTime::forComponents(2018, 1, 255, 0, 0, 0);
  assertTrue(dt.isError());

  dt = OffsetDateTime::forComponents(2018, 1, 1, 255, 0, 0);
  assertTrue(dt.isError());

  dt = OffsetDateTime::forComponents(2018, 1, 1, 0, 255, 0);
  assertTrue(dt.isError());

  dt = OffsetDateTime::forComponents(2018, 1, 1, 0, 0, 255);
  assertTrue(dt.isError());
}

test(OffsetDateTimeTest, forComponents) {
  OffsetDateTime dt;

  // 1931-12-13 20:45:52Z, smalltest datetime using int32_t from AceTime Epoch.
  // Let's use +1 of that since INT_MIN will be used to indicate an error.
  dt = OffsetDateTime::forComponents(1931, 12, 13, 20, 45, 53);
  assertEqual((acetime_t) -24856, dt.toEpochDays());
  assertEqual((acetime_t) -13899, dt.toUnixDays());
  assertEqual((acetime_t) (INT32_MIN + 1), dt.toEpochSeconds());
  assertEqual(LocalDate::kSunday, dt.dayOfWeek());

  // 2000-01-01 00:00:00Z Saturday
  dt = OffsetDateTime::forComponents(2000, 1, 1, 0, 0, 0);
  assertEqual((acetime_t) 0, dt.toEpochDays());
  assertEqual((acetime_t) 10957, dt.toUnixDays());
  assertEqual((acetime_t) 0, dt.toEpochSeconds());
  assertEqual(LocalDate::kSaturday, dt.dayOfWeek());

  // 2000-01-02 00:00:00Z Sunday
  dt = OffsetDateTime::forComponents(2000, 1, 2, 0, 0, 0);
  assertEqual((acetime_t) 1, dt.toEpochDays());
  assertEqual((acetime_t) 10958, dt.toUnixDays());
  assertEqual((acetime_t) 86400, dt.toEpochSeconds());
  assertEqual(LocalDate::kSunday, dt.dayOfWeek());

  // 2000-02-29 00:00:00Z Tuesday
  dt = OffsetDateTime::forComponents(2000, 2, 29, 0, 0, 0);
  assertEqual((acetime_t) 59, dt.toEpochDays());
  assertEqual((acetime_t) 11016, dt.toUnixDays());
  assertEqual((acetime_t) 86400 * 59, dt.toEpochSeconds());
  assertEqual(LocalDate::kTuesday, dt.dayOfWeek());

  // 2018-01-01 00:00:00Z Monday
  dt = OffsetDateTime::forComponents(2018, 1, 1, 0, 0, 0);
  assertEqual((acetime_t) 6575, dt.toEpochDays());
  assertEqual((acetime_t) 17532, dt.toUnixDays());
  assertEqual(6575 * (acetime_t) 86400, dt.toEpochSeconds());
  assertEqual(LocalDate::kMonday, dt.dayOfWeek());

  // 2018-01-01 00:00:00+00:15 Monday
  dt = OffsetDateTime::forComponents(2018, 1, 1, 0, 0, 0,
      UtcOffset::forOffsetCode(1));
  assertEqual((acetime_t) 6574, dt.toEpochDays());
  assertEqual((acetime_t) 17531, dt.toUnixDays());
  assertEqual(6575 * (acetime_t) 86400 - 15*60, dt.toEpochSeconds());
  assertEqual(LocalDate::kMonday, dt.dayOfWeek());

  // 2038-01-19 03:14:07Z (largest value using Unix Epoch)
  dt = OffsetDateTime::forComponents(2038, 1, 19, 3, 14, 7);
  assertEqual((acetime_t) 13898, dt.toEpochDays());
  assertEqual((acetime_t) 24855, dt.toUnixDays());
  assertEqual((acetime_t) 1200798847, dt.toEpochSeconds());
  assertEqual(LocalDate::kTuesday, dt.dayOfWeek());

  // 2068-01-19 03:14:06Z (largest value for AceTime Epoch).
  // INT32_MAX is used as a sentinel invalid value.
  // TODO: Change this to INT32_MIN.
  dt = OffsetDateTime::forComponents(2068, 1, 19, 3, 14, 6);
  assertEqual((acetime_t) 24855, dt.toEpochDays());
  assertEqual((acetime_t) 35812, dt.toUnixDays());
  assertEqual((acetime_t) (INT32_MAX - 1), dt.toEpochSeconds());
  assertEqual(LocalDate::kThursday, dt.dayOfWeek());
}

test(OffsetDateTimeTest, toAndForUnixSeconds) {
  OffsetDateTime dt;
  OffsetDateTime udt;

  // 1931-12-13 20:45:52Z, smalltest datetime using int32_t from AceTime Epoch.
  // Let's use +1 of that since INT_MIN will be used to indicate an error.
  dt = OffsetDateTime::forComponents(1931, 12, 13, 20, 45, 53);
  assertEqual((acetime_t) -1200798847, dt.toUnixSeconds());
  udt = OffsetDateTime::forUnixSeconds(dt.toUnixSeconds());
  assertTrue(dt == udt);

  // 1970-01-01 00:00:00Z
  dt = OffsetDateTime::forComponents(1970, 1, 1, 0, 0, 0);
  assertEqual((acetime_t) 0, dt.toUnixSeconds());
  udt = OffsetDateTime::forUnixSeconds(dt.toUnixSeconds());
  assertTrue(dt == udt);

  // 2000-01-01 00:00:00Z
  dt = OffsetDateTime::forComponents(2000, 1, 1, 0, 0, 0);
  assertEqual((acetime_t) 946684800, dt.toUnixSeconds());
  udt = OffsetDateTime::forUnixSeconds(dt.toUnixSeconds());
  assertTrue(dt == udt);

  // 2018-01-01 00:00:00Z
  dt = OffsetDateTime::forComponents(2018, 1, 1, 0, 0, 0);
  assertEqual((acetime_t) 1514764800, dt.toUnixSeconds());
  udt = OffsetDateTime::forUnixSeconds(dt.toUnixSeconds());
  assertTrue(dt == udt);

  // 2018-08-30T06:45:01-07:00
  dt = OffsetDateTime::forComponents(2018, 8, 30, 6, 45, 1,
      UtcOffset::forHour(-7));
  assertEqual((acetime_t) 1535636701, dt.toUnixSeconds());
  udt = OffsetDateTime::forUnixSeconds(dt.toUnixSeconds(),
      UtcOffset::forHour(-7));
  assertTrue(dt == udt);

  // 2038-01-19 03:14:06Z (largest value - 1 using Unix Epoch)
  dt = OffsetDateTime::forComponents(2038, 1, 19, 3, 14, 6);
  assertEqual((acetime_t) (INT32_MAX - 1), dt.toUnixSeconds());
  udt = OffsetDateTime::forUnixSeconds(dt.toUnixSeconds());
  assertTrue(dt == udt);
}

test(OffsetDateTimeTest, forEpochSeconds) {
  // 2029-12-31 23:59:59Z Monday
  OffsetDateTime dt = OffsetDateTime::forEpochSeconds(
      10958 * (acetime_t) 86400 - 1);

  assertEqual((int16_t) 2029, dt.year());
  assertEqual(29, dt.yearShort());
  assertEqual(12, dt.month());
  assertEqual(31, dt.day());
  assertEqual(23, dt.hour());
  assertEqual(59, dt.minute());
  assertEqual(59, dt.second());
  assertEqual(LocalDate::kMonday, dt.dayOfWeek());

  // 2029-12-31 15:59:59-08:00 Monday
  UtcOffset offset = UtcOffset::forOffsetCode(-32); // UTC-08:00
  dt = OffsetDateTime::forEpochSeconds(10958 * (acetime_t) 86400 - 1, offset);
  assertEqual((int16_t) 2029, dt.year());
  assertEqual(29, dt.yearShort());
  assertEqual(12, dt.month());
  assertEqual(31, dt.day());
  assertEqual(15, dt.hour());
  assertEqual(59, dt.minute());
  assertEqual(59, dt.second());
  assertEqual(LocalDate::kMonday, dt.dayOfWeek());
}

test(OffsetDateTimeTest, convertToUtcOffset) {
  OffsetDateTime a = OffsetDateTime::forComponents(2018, 1, 1, 12, 0, 0);
  OffsetDateTime b = a.convertToUtcOffset(UtcOffset::forHour(-7));

  assertEqual((int16_t) 2018, b.year());
  assertEqual(18, b.yearShort());
  assertEqual(1, b.month());
  assertEqual(1, b.day());
  assertEqual(5, b.hour());
  assertEqual(0, b.minute());
  assertEqual(0, b.second());
  assertEqual(-28, b.utcOffset().toOffsetCode());
}

test(OffsetDateTimeTest, compareTo) {
  OffsetDateTime a = OffsetDateTime::forComponents(2018, 1, 1, 12, 0, 0);
  OffsetDateTime b = OffsetDateTime::forComponents(2018, 1, 1, 12, 0, 0);
  assertEqual(a.compareTo(b), 0);
  assertTrue(a == b);
  assertFalse(a != b);

  a = OffsetDateTime::forComponents(2018, 1, 1, 12, 0, 0);
  b = OffsetDateTime::forComponents(2018, 1, 1, 12, 0, 1);
  assertLess(a.compareTo(b), 0);
  assertMore(b.compareTo(a), 0);
  assertTrue(a != b);

  a = OffsetDateTime::forComponents(2018, 1, 1, 12, 0, 0);
  b = OffsetDateTime::forComponents(2018, 1, 1, 12, 1, 0);
  assertLess(a.compareTo(b), 0);
  assertMore(b.compareTo(a), 0);
  assertTrue(a != b);

  a = OffsetDateTime::forComponents(2018, 1, 1, 11, 0, 0);
  b = OffsetDateTime::forComponents(2018, 1, 1, 12, 0, 0,
      UtcOffset::forOffsetCode(1));
  assertLess(a.compareTo(b), 0);
  assertMore(b.compareTo(a), 0);
  assertTrue(a != b);

  a = OffsetDateTime::forComponents(2018, 1, 1, 12, 0, 0);
  b = OffsetDateTime::forComponents(2018, 1, 2, 12, 0, 0);
  assertLess(a.compareTo(b), 0);
  assertMore(b.compareTo(a), 0);
  assertTrue(a != b);

  a = OffsetDateTime::forComponents(2018, 1, 1, 12, 0, 0);
  b = OffsetDateTime::forComponents(2018, 2, 1, 12, 0, 0);
  assertLess(a.compareTo(b), 0);
  assertMore(b.compareTo(a), 0);
  assertTrue(a != b);

  a = OffsetDateTime::forComponents(2018, 1, 1, 12, 0, 0);
  b = OffsetDateTime::forComponents(2019, 1, 1, 12, 0, 0);
  assertLess(a.compareTo(b), 0);
  assertMore(b.compareTo(a), 0);
  assertTrue(a != b);

  // 2018-1-1 12:00:00+01:00
  a = OffsetDateTime::forComponents(2018, 1, 1, 12, 0, 0,
      UtcOffset::forHour(1));
  // 2018-1-1 12:00:00-08:00
  b = OffsetDateTime::forComponents(2018, 1, 1, 12, 0, 0,
      UtcOffset::forHour(-8));
  assertLess(a.compareTo(b), 0);
  assertMore(b.compareTo(a), 0);
  assertTrue(a != b);
}

test(OffsetDateTimeTest, dayOfWeek) {
  // 2018-01-01 00:00:00Z Monday
  OffsetDateTime dt = OffsetDateTime::forComponents(2018, 1, 1, 0, 0, 0);
  assertEqual(LocalDate::kMonday, dt.dayOfWeek());

  dt.hour(23); // 2018-01-01 23:00:00Z, no change to dayOfWeek
  assertEqual(LocalDate::kMonday, dt.dayOfWeek());

  dt.minute(40); // 2018-01-01 23:40:00Z, no change to dayOfWeek
  assertEqual(LocalDate::kMonday, dt.dayOfWeek());

  dt.second(3); // 2018-01-01 23:40:03Z, no change to dayOfWeek
  assertEqual(LocalDate::kMonday, dt.dayOfWeek());

  // 2018-01-01 23:40:03+00:45, no change to dayOfWeek
  dt.utcOffset(UtcOffset::forOffsetCode(3));
  assertEqual(LocalDate::kMonday, dt.dayOfWeek());

  dt.day(2); // 2018-01-02 23:40:03+00:45, changes dayOfWeek
  assertEqual(LocalDate::kTuesday, dt.dayOfWeek());

  dt.month(2); // 2018-02-02 23:40:03+00:45, changes dayOfWeek
  assertEqual(LocalDate::kFriday, dt.dayOfWeek());

  dt.yearShort(19); // 2019-02-02 23:40:03+00:45, changes dayOfWeek
  assertEqual(LocalDate::kSaturday, dt.dayOfWeek());

  dt.year(2020); // 2020-02-02 23:40:03+00:45, changes dayOfWeek
  assertEqual(LocalDate::kSunday, dt.dayOfWeek());
}

test(OffsetDateTimeTest, forDateString_errors) {
  // empty string, too short
  OffsetDateTime dt = OffsetDateTime::forDateString("");
  assertTrue(dt.isError());

  // not enough components
  dt = OffsetDateTime::forDateString(F("2018-08-31"));
  assertTrue(dt.isError());

  // too long
  dt = OffsetDateTime::forDateString(F("2018-08-31T13:48:01-07:00X"));
  assertTrue(dt.isError());

  // too short
  dt = OffsetDateTime::forDateString(F("2018-08-31T13:48:01-07:0"));
  assertTrue(dt.isError());

  // missing UTC
  dt = OffsetDateTime::forDateString(F("2018-08-31T13:48:01"));
  assertTrue(dt.isError());

  // parser cares about UTC+/- offset
  dt = OffsetDateTime::forDateString(F("2018-08-31 13:48:01&07:00"));
  assertTrue(dt.isError());
}

test(OffsetDateTimeTest, forDateString) {
  // exact ISO8601 format
  OffsetDateTime dt = OffsetDateTime::forDateString(
      F("2018-08-31T13:48:01-07:00"));
  assertFalse(dt.isError());
  assertEqual((int16_t) 2018, dt.year());
  assertEqual(18, dt.yearShort());
  assertEqual(8, dt.month());
  assertEqual(31, dt.day());
  assertEqual(13, dt.hour());
  assertEqual(48, dt.minute());
  assertEqual(1, dt.second());
  assertEqual(-28, dt.utcOffset().toOffsetCode());
  assertEqual(LocalDate::kFriday, dt.dayOfWeek());

  // parser does not care about most separators, this may change in the future
  dt = OffsetDateTime::forDateString(F("2018/08/31 13#48#01+07#00"));
  assertFalse(dt.isError());
  assertEqual((int16_t) 2018, dt.year());
  assertEqual(18, dt.yearShort());
  assertEqual(8, dt.month());
  assertEqual(31, dt.day());
  assertEqual(13, dt.hour());
  assertEqual(48, dt.minute());
  assertEqual(1, dt.second());
  assertEqual(28, dt.utcOffset().toOffsetCode());
  assertEqual(LocalDate::kFriday, dt.dayOfWeek());
}

test(OffsetDateTimeTest, increment) {
  OffsetDateTime dt = OffsetDateTime::forComponents(2001, 2, 3, 4, 5, 6);
  assertEqual((int16_t) 2001, dt.year());
  assertEqual(2, dt.month());
  assertEqual(3, dt.day());
  assertEqual(4, dt.hour());
  assertEqual(5, dt.minute());
  assertEqual(6, dt.second());
  assertEqual(0, dt.utcOffset().toOffsetCode());

  dt.incrementYear();
  assertEqual((int16_t) 2002, dt.year());
  assertEqual(2, dt.month());
  assertEqual(3, dt.day());
  assertEqual(4, dt.hour());
  assertEqual(5, dt.minute());
  assertEqual(6, dt.second());
  assertEqual(0, dt.utcOffset().toOffsetCode());

  dt.incrementMonth();
  assertEqual((int16_t) 2002, dt.year());
  assertEqual(3, dt.month());
  assertEqual(3, dt.day());
  assertEqual(4, dt.hour());
  assertEqual(5, dt.minute());
  assertEqual(6, dt.second());
  assertEqual(0, dt.utcOffset().toOffsetCode());

  dt.incrementDay();
  assertEqual((int16_t) 2002, dt.year());
  assertEqual(3, dt.month());
  assertEqual(4, dt.day());
  assertEqual(4, dt.hour());
  assertEqual(5, dt.minute());
  assertEqual(6, dt.second());
  assertEqual(0, dt.utcOffset().toOffsetCode());

  dt.incrementHour();
  assertEqual((int16_t) 2002, dt.year());
  assertEqual(3, dt.month());
  assertEqual(4, dt.day());
  assertEqual(5, dt.hour());
  assertEqual(5, dt.minute());
  assertEqual(6, dt.second());
  assertEqual(0, dt.utcOffset().toOffsetCode());

  dt.incrementMinute();
  assertEqual((int16_t) 2002, dt.year());
  assertEqual(3, dt.month());
  assertEqual(4, dt.day());
  assertEqual(5, dt.hour());
  assertEqual(6, dt.minute());
  assertEqual(6, dt.second());
  assertEqual(0, dt.utcOffset().toOffsetCode());
}

// --------------------------------------------------------------------------
// DateTime
// --------------------------------------------------------------------------

test(DateTimeTest, forComponents) {
  DateTime dt;

  // 1931-12-13 20:45:52Z, smalltest datetime using int32_t from AceTime Epoch.
  // Let's use +1 of that since INT_MIN will be used to indicate an error.
  dt = DateTime::forComponents(1931, 12, 13, 20, 45, 53);
  assertEqual((acetime_t) -24856, dt.toEpochDays());
  assertEqual((acetime_t) -13899, dt.toUnixDays());
  assertEqual((acetime_t) (INT32_MIN + 1), dt.toEpochSeconds());
  assertEqual(LocalDate::kSunday, dt.dayOfWeek());

  // 2000-01-01 00:00:00Z Saturday
  dt = DateTime::forComponents(2000, 1, 1, 0, 0, 0);
  assertEqual((acetime_t) 0, dt.toEpochDays());
  assertEqual((acetime_t) 10957, dt.toUnixDays());
  assertEqual((acetime_t) 0, dt.toEpochSeconds());
  assertEqual(LocalDate::kSaturday, dt.dayOfWeek());

  // 2000-01-02 00:00:00Z Sunday
  dt = DateTime::forComponents(2000, 1, 2, 0, 0, 0);
  assertEqual((acetime_t) 1, dt.toEpochDays());
  assertEqual((acetime_t) 10958, dt.toUnixDays());
  assertEqual((acetime_t) 86400, dt.toEpochSeconds());
  assertEqual(LocalDate::kSunday, dt.dayOfWeek());

  // 2000-02-29 00:00:00Z Tuesday
  dt = DateTime::forComponents(2000, 2, 29, 0, 0, 0);
  assertEqual((acetime_t) 59, dt.toEpochDays());
  assertEqual((acetime_t) 11016, dt.toUnixDays());
  assertEqual((acetime_t) 86400 * 59, dt.toEpochSeconds());
  assertEqual(LocalDate::kTuesday, dt.dayOfWeek());

  // 2018-01-01 00:00:00Z Monday
  dt = DateTime::forComponents(2018, 1, 1, 0, 0, 0);
  assertEqual((acetime_t) 6575, dt.toEpochDays());
  assertEqual((acetime_t) 17532, dt.toUnixDays());
  assertEqual(6575 * (acetime_t) 86400, dt.toEpochSeconds());
  assertEqual(LocalDate::kMonday, dt.dayOfWeek());

  // 2018-01-01 00:00:00+00:15 Monday
  dt = DateTime::forComponents(2018, 1, 1, 0, 0, 0,
      TimeZone::forUtcOffset(UtcOffset::forOffsetCode(1)));
  assertEqual((acetime_t) 6574, dt.toEpochDays());
  assertEqual((acetime_t) 17531, dt.toUnixDays());
  assertEqual(6575 * (acetime_t) 86400 - 15*60, dt.toEpochSeconds());
  assertEqual(LocalDate::kMonday, dt.dayOfWeek());

  // 2038-01-19 03:14:07Z (largest value using Unix Epoch)
  dt = DateTime::forComponents(2038, 1, 19, 3, 14, 7);
  assertEqual((acetime_t) 13898, dt.toEpochDays());
  assertEqual((acetime_t) 24855, dt.toUnixDays());
  assertEqual((acetime_t) 1200798847, dt.toEpochSeconds());
  assertEqual(LocalDate::kTuesday, dt.dayOfWeek());

  // 2068-01-19 03:14:06Z (largest value for AceTime Epoch).
  // INT32_MAX is used as a sentinel invalid value.
  // TODO: Change this to INT32_MIN.
  dt = DateTime::forComponents(2068, 1, 19, 3, 14, 6);
  assertEqual((acetime_t) 24855, dt.toEpochDays());
  assertEqual((acetime_t) 35812, dt.toUnixDays());
  assertEqual((acetime_t) (INT32_MAX - 1), dt.toEpochSeconds());
  assertEqual(LocalDate::kThursday, dt.dayOfWeek());
}

test(DateTime, toAndForUnixSeconds) {
  DateTime dt;
  DateTime udt;

  // 1931-12-13 20:45:52Z, smalltest datetime using int32_t from AceTime Epoch.
  // Let's use +1 of that since INT_MIN will be used to indicate an error.
  dt = DateTime::forComponents(1931, 12, 13, 20, 45, 53);
  assertEqual((acetime_t) -1200798847, dt.toUnixSeconds());
  udt = DateTime::forUnixSeconds(dt.toUnixSeconds());
  assertTrue(dt == udt);

  // 1970-01-01 00:00:00Z
  dt = DateTime::forComponents(1970, 1, 1, 0, 0, 0);
  assertEqual((acetime_t) 0, dt.toUnixSeconds());
  udt = DateTime::forUnixSeconds(dt.toUnixSeconds());
  assertTrue(dt == udt);

  // 2000-01-01 00:00:00Z
  dt = DateTime::forComponents(2000, 1, 1, 0, 0, 0);
  assertEqual((acetime_t) 946684800, dt.toUnixSeconds());
  udt = DateTime::forUnixSeconds(dt.toUnixSeconds());
  assertTrue(dt == udt);

  // 2018-01-01 00:00:00Z
  dt = DateTime::forComponents(2018, 1, 1, 0, 0, 0);
  assertEqual((acetime_t) 1514764800, dt.toUnixSeconds());
  udt = DateTime::forUnixSeconds(dt.toUnixSeconds());
  assertTrue(dt == udt);

  // 2018-08-30T06:45:01-07:00
  TimeZone tz = TimeZone::forUtcOffset(UtcOffset::forHour(-7));
  dt = DateTime::forComponents(2018, 8, 30, 6, 45, 1, tz);
  assertEqual((acetime_t) 1535636701, dt.toUnixSeconds());
  udt = DateTime::forUnixSeconds(dt.toUnixSeconds(), tz);
  assertTrue(dt == udt);

  // 2038-01-19 03:14:06Z (largest value - 1 using Unix Epoch)
  dt = DateTime::forComponents(2038, 1, 19, 3, 14, 6);
  assertEqual((acetime_t) (INT32_MAX - 1), dt.toUnixSeconds());
  udt = DateTime::forUnixSeconds(dt.toUnixSeconds());
  assertTrue(dt == udt);
}

test(DateTimeTest, TimeZone_Manual) {
  TimeZone tz = TimeZone::forUtcOffset(UtcOffset::forHour(-8));
  DateTime dt = DateTime::forComponents(2018, 3, 11, 1, 59, 59, tz);

  UtcOffset pst = UtcOffset::forHour(-8);
  OffsetDateTime otz = OffsetDateTime::forComponents(2018, 3, 11, 1, 59, 59,
      pst);

  assertEqual(otz.toEpochSeconds(), dt.toEpochSeconds());
}

test(DateTimeTest, forComponents_beforeDst) {
  ZoneAgent agent(&zonedb::kZoneLos_Angeles);
  TimeZone tz = TimeZone::forZone(&agent);
  DateTime dt = DateTime::forComponents(2018, 3, 11, 1, 59, 59, tz);

  UtcOffset pst = UtcOffset::forHour(-8);
  OffsetDateTime otz = OffsetDateTime::forComponents(2018, 3, 11, 1, 59, 59,
    pst);

  assertEqual(otz.toEpochSeconds(), dt.toEpochSeconds());
}

test(DateTimeTest, forComponents_inDstGap) {
  ZoneAgent agent(&zonedb::kZoneLos_Angeles);
  TimeZone tz = TimeZone::forZone(&agent);
  DateTime dt = DateTime::forComponents(2018, 3, 11, 2, 0, 1, tz);

  UtcOffset pdt = UtcOffset::forHour(-7);
  OffsetDateTime odt = OffsetDateTime::forComponents(2018, 3, 11, 2, 0, 1, pdt);

  assertEqual(odt.toEpochSeconds(), dt.toEpochSeconds());
}

test(DateTimeTest, forComponents_inDst) {
  ZoneAgent agent(&zonedb::kZoneLos_Angeles);
  TimeZone tz = TimeZone::forZone(&agent);
  DateTime dt = DateTime::forComponents(2018, 3, 11, 3, 0, 1, tz);

  UtcOffset pdt = UtcOffset::forHour(-7);
  OffsetDateTime odt = OffsetDateTime::forComponents(2018, 3, 11, 3, 0, 1, pdt);

  assertEqual(odt.toEpochSeconds(), dt.toEpochSeconds());
}

test(DateTimeTest, forComponents_beforeStd) {
  ZoneAgent agent(&zonedb::kZoneLos_Angeles);
  TimeZone tz = TimeZone::forZone(&agent);
  DateTime dt = DateTime::forComponents(2018, 11, 4, 0, 59, 59, tz);

  UtcOffset pdt = UtcOffset::forHour(-7);
  OffsetDateTime odt = OffsetDateTime::forComponents(2018, 11, 4, 0, 59, 59,
      pdt);

  assertEqual(odt.toEpochSeconds(), dt.toEpochSeconds());
}

test(DateTimeTest, forComponents_inOverlap) {
  ZoneAgent agent(&zonedb::kZoneLos_Angeles);
  TimeZone tz = TimeZone::forZone(&agent);
  DateTime dt = DateTime::forComponents(2018, 11, 4, 1, 0, 1, tz); // ambiguous

  UtcOffset pdt = UtcOffset::forHour(-8);
  OffsetDateTime odt = OffsetDateTime::forComponents(2018, 11, 4, 1, 0, 1, pdt);

  assertEqual(odt.toEpochSeconds(), dt.toEpochSeconds());
}


test(DateTimeTest, forComponents_afterOverlap) {
  ZoneAgent agent(&zonedb::kZoneLos_Angeles);
  TimeZone tz = TimeZone::forZone(&agent);
  DateTime dt = DateTime::forComponents(2018, 11, 4, 2, 0, 1, tz); // ambiguous

  UtcOffset pdt = UtcOffset::forHour(-8);
  OffsetDateTime odt = OffsetDateTime::forComponents(2018, 11, 4, 2, 0, 1, pdt);

  assertEqual(odt.toEpochSeconds(), dt.toEpochSeconds());
}
// --------------------------------------------------------------------------

void setup() {
  delay(1000); // wait for stability on some boards to prevent garbage Serial
  Serial.begin(115200); // ESP8266 default of 74880 not supported on Linux
  while(!Serial); // for the Arduino Leonardo/Micro only
}

void loop() {
  TestRunner::run();
}

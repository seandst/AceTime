#line 2 "ZonedDateTimeTest.ino"

#include <AUnit.h>
#include <AceTime.h>

using namespace aunit;
using namespace ace_time;

test(ZonedDateTimeTest, forComponents) {
  ZonedDateTime dt;

  // 1931-12-13 20:45:52Z, smalltest datetime using int32_t from AceTime Epoch.
  // Let's use +1 of that since INT_MIN will be used to indicate an error.
  dt = ZonedDateTime::forComponents(1931, 12, 13, 20, 45, 53);
  assertEqual((acetime_t) -24856, dt.toEpochDays());
  assertEqual((acetime_t) -13899, dt.toUnixDays());
  assertEqual((acetime_t) (INT32_MIN + 1), dt.toEpochSeconds());
  assertEqual(LocalDate::kSunday, dt.dayOfWeek());

  // 2000-01-01 00:00:00Z Saturday
  dt = ZonedDateTime::forComponents(2000, 1, 1, 0, 0, 0);
  assertEqual((acetime_t) 0, dt.toEpochDays());
  assertEqual((acetime_t) 10957, dt.toUnixDays());
  assertEqual((acetime_t) 0, dt.toEpochSeconds());
  assertEqual(LocalDate::kSaturday, dt.dayOfWeek());

  // 2000-01-02 00:00:00Z Sunday
  dt = ZonedDateTime::forComponents(2000, 1, 2, 0, 0, 0);
  assertEqual((acetime_t) 1, dt.toEpochDays());
  assertEqual((acetime_t) 10958, dt.toUnixDays());
  assertEqual((acetime_t) 86400, dt.toEpochSeconds());
  assertEqual(LocalDate::kSunday, dt.dayOfWeek());

  // 2000-02-29 00:00:00Z Tuesday
  dt = ZonedDateTime::forComponents(2000, 2, 29, 0, 0, 0);
  assertEqual((acetime_t) 59, dt.toEpochDays());
  assertEqual((acetime_t) 11016, dt.toUnixDays());
  assertEqual((acetime_t) 86400 * 59, dt.toEpochSeconds());
  assertEqual(LocalDate::kTuesday, dt.dayOfWeek());

  // 2018-01-01 00:00:00Z Monday
  dt = ZonedDateTime::forComponents(2018, 1, 1, 0, 0, 0);
  assertEqual((acetime_t) 6575, dt.toEpochDays());
  assertEqual((acetime_t) 17532, dt.toUnixDays());
  assertEqual(6575 * (acetime_t) 86400, dt.toEpochSeconds());
  assertEqual(LocalDate::kMonday, dt.dayOfWeek());

  // 2018-01-01 00:00:00+00:15 Monday
  ManualZoneSpecifier zoneSpecifier(UtcOffset::forMinutes(15), UtcOffset::forHour(1));
  dt = ZonedDateTime::forComponents(2018, 1, 1, 0, 0, 0, TimeZone(&zoneSpecifier));
  assertEqual((acetime_t) 6574, dt.toEpochDays());
  assertEqual((acetime_t) 17531, dt.toUnixDays());
  assertEqual(6575 * (acetime_t) 86400 - 15*60, dt.toEpochSeconds());
  assertEqual(LocalDate::kMonday, dt.dayOfWeek());

  // 2038-01-19 03:14:07Z (largest value using Unix Epoch)
  dt = ZonedDateTime::forComponents(2038, 1, 19, 3, 14, 7);
  assertEqual((acetime_t) 13898, dt.toEpochDays());
  assertEqual((acetime_t) 24855, dt.toUnixDays());
  assertEqual((acetime_t) 1200798847, dt.toEpochSeconds());
  assertEqual(LocalDate::kTuesday, dt.dayOfWeek());

  // 2068-01-19 03:14:06Z (largest value for AceTime Epoch).
  // INT32_MAX is used as a sentinel invalid value.
  // TODO: Change this to INT32_MIN.
  dt = ZonedDateTime::forComponents(2068, 1, 19, 3, 14, 6);
  assertEqual((acetime_t) 24855, dt.toEpochDays());
  assertEqual((acetime_t) 35812, dt.toUnixDays());
  assertEqual((acetime_t) (INT32_MAX - 1), dt.toEpochSeconds());
  assertEqual(LocalDate::kThursday, dt.dayOfWeek());
}

test(ZonedDateTimeTest, toAndForUnixSeconds) {
  ZonedDateTime dt;
  ZonedDateTime udt;

  // 1931-12-13 20:45:52Z, smalltest datetime using int32_t from AceTime Epoch.
  // Let's use +1 of that since INT_MIN will be used to indicate an error.
  dt = ZonedDateTime::forComponents(1931, 12, 13, 20, 45, 53);
  assertEqual((acetime_t) -1200798847, dt.toUnixSeconds());
  udt = ZonedDateTime::forUnixSeconds(dt.toUnixSeconds());
  assertTrue(dt == udt);

  // 1970-01-01 00:00:00Z
  dt = ZonedDateTime::forComponents(1970, 1, 1, 0, 0, 0);
  assertEqual((acetime_t) 0, dt.toUnixSeconds());
  udt = ZonedDateTime::forUnixSeconds(dt.toUnixSeconds());
  assertTrue(dt == udt);

  // 2000-01-01 00:00:00Z
  dt = ZonedDateTime::forComponents(2000, 1, 1, 0, 0, 0);
  assertEqual((acetime_t) 946684800, dt.toUnixSeconds());
  udt = ZonedDateTime::forUnixSeconds(dt.toUnixSeconds());
  assertTrue(dt == udt);


  // 2018-01-01 00:00:00Z
  dt = ZonedDateTime::forComponents(2018, 1, 1, 0, 0, 0);
  assertEqual((acetime_t) 1514764800, dt.toUnixSeconds());
  udt = ZonedDateTime::forUnixSeconds(dt.toUnixSeconds());
  assertTrue(dt == udt);

  // 2018-08-30T06:45:01-07:00
  ManualZoneSpecifier zoneSpecifier(UtcOffset::forHour(-7), UtcOffset::forHour(1));
  TimeZone tz(&zoneSpecifier);
  dt = ZonedDateTime::forComponents(2018, 8, 30, 6, 45, 1, tz);
  assertEqual((acetime_t) 1535636701, dt.toUnixSeconds());
  udt = ZonedDateTime::forUnixSeconds(dt.toUnixSeconds(), tz);
  assertTrue(dt == udt);

  // 2038-01-19 03:14:06Z (largest value - 1 using Unix Epoch)
  dt = ZonedDateTime::forComponents(2038, 1, 19, 3, 14, 6);
  assertEqual((acetime_t) (INT32_MAX - 1), dt.toUnixSeconds());
  udt = ZonedDateTime::forUnixSeconds(dt.toUnixSeconds());
  assertTrue(dt == udt);
}

test(ZonedDateTimeTest, ManualZoneSpecifier) {
  ManualZoneSpecifier zoneSpecifier(UtcOffset::forHour(-8), UtcOffset::forHour(1));
  TimeZone tz(&zoneSpecifier);
  ZonedDateTime dt = ZonedDateTime::forComponents(2018, 3, 11, 1, 59, 59, tz);

  OffsetDateTime otz = OffsetDateTime::forComponents(2018, 3, 11, 1, 59, 59,
      UtcOffset::forHour(-8));

  assertEqual(otz.toEpochSeconds(), dt.toEpochSeconds());
}

test(ZonedDateTimeTest, convertToTimeZone) {
  ManualZoneSpecifier stdSpec(UtcOffset::forHour(-8), UtcOffset::forHour(1));
  TimeZone stdTz(&stdSpec);
  ZonedDateTime std = ZonedDateTime::forComponents(
      2018, 3, 11, 1, 59, 59, stdTz);
  acetime_t stdEpochSeconds = std.toEpochSeconds();

  ManualZoneSpecifier dstSpec(stdSpec);
  dstSpec.isDst(true);
  TimeZone dstTz(&dstSpec);
  ZonedDateTime dst = std.convertToTimeZone(dstTz);
  acetime_t dstEpochSeconds = dst.toEpochSeconds();

  assertEqual(stdEpochSeconds, dstEpochSeconds);

  assertEqual((int16_t) 2018, dst.year());
  assertEqual(3, dst.month());
  assertEqual(11, dst.day());
  assertEqual(2, dst.hour());
  assertEqual(59, dst.minute());
  assertEqual(59, dst.second());
  assertEqual(-7*60, dst.timeZone().getUtcOffset(stdEpochSeconds).toMinutes());
}

test(ZonedDateTimeTest, forComponents_beforeDst) {
  AutoZoneSpecifier zoneSpecifier(&zonedb::kZoneLos_Angeles);
  TimeZone tz(&zoneSpecifier);
  ZonedDateTime dt = ZonedDateTime::forComponents(2018, 3, 11, 1, 59, 59, tz);

  UtcOffset pst = UtcOffset::forHour(-8);
  OffsetDateTime otz = OffsetDateTime::forComponents(2018, 3, 11, 1, 59, 59,
    pst);

  assertEqual(otz.toEpochSeconds(), dt.toEpochSeconds());
}

test(ZonedDateTimeTest, forComponents_inDstGap) {
  AutoZoneSpecifier zoneSpecifier(&zonedb::kZoneLos_Angeles);
  TimeZone tz(&zoneSpecifier);
  ZonedDateTime dt = ZonedDateTime::forComponents(2018, 3, 11, 2, 0, 1, tz);

  UtcOffset pdt = UtcOffset::forHour(-7);
  OffsetDateTime odt = OffsetDateTime::forComponents(2018, 3, 11, 2, 0, 1, pdt);

  assertEqual(odt.toEpochSeconds(), dt.toEpochSeconds());
}

test(ZonedDateTimeTest, forComponents_inDst) {
  AutoZoneSpecifier zoneSpecifier(&zonedb::kZoneLos_Angeles);
  TimeZone tz(&zoneSpecifier);
  ZonedDateTime dt = ZonedDateTime::forComponents(2018, 3, 11, 3, 0, 1, tz);

  UtcOffset pdt = UtcOffset::forHour(-7);
  OffsetDateTime odt = OffsetDateTime::forComponents(2018, 3, 11, 3, 0, 1, pdt);

  assertEqual(odt.toEpochSeconds(), dt.toEpochSeconds());
}

test(ZonedDateTimeTest, forComponents_beforeStd) {
  AutoZoneSpecifier zoneSpecifier(&zonedb::kZoneLos_Angeles);
  TimeZone tz(&zoneSpecifier);
  ZonedDateTime dt = ZonedDateTime::forComponents(2018, 11, 4, 0, 59, 59, tz);

  UtcOffset pdt = UtcOffset::forHour(-7);
  OffsetDateTime odt = OffsetDateTime::forComponents(2018, 11, 4, 0, 59, 59,
      pdt);

  assertEqual(odt.toEpochSeconds(), dt.toEpochSeconds());
}

test(ZonedDateTimeTest, forComponents_inOverlap) {
  AutoZoneSpecifier zoneSpecifier(&zonedb::kZoneLos_Angeles);
  TimeZone tz(&zoneSpecifier);
  ZonedDateTime dt = ZonedDateTime::forComponents(
      2018, 11, 4, 1, 0, 1, tz); // ambiguous

  UtcOffset pdt = UtcOffset::forHour(-8);
  OffsetDateTime odt = OffsetDateTime::forComponents(2018, 11, 4, 1, 0, 1, pdt);

  assertEqual(odt.toEpochSeconds(), dt.toEpochSeconds());
}

test(ZonedDateTimeTest, forComponents_afterOverlap) {
  AutoZoneSpecifier zoneSpecifier(&zonedb::kZoneLos_Angeles);
  TimeZone tz(&zoneSpecifier);
  ZonedDateTime dt = ZonedDateTime::forComponents(
      2018, 11, 4, 2, 0, 1, tz); // ambiguous

  UtcOffset pdt = UtcOffset::forHour(-8);
  OffsetDateTime odt = OffsetDateTime::forComponents(2018, 11, 4, 2, 0, 1, pdt);

  assertEqual(odt.toEpochSeconds(), dt.toEpochSeconds());
}

// --------------------------------------------------------------------------
// data_time_mutation
// --------------------------------------------------------------------------

test(DateTimeMutationTest, increment) {
  ZonedDateTime dt = ZonedDateTime::forComponents(2001, 2, 3, 4, 5, 6);
  assertEqual((int16_t) 2001, dt.year());
  assertEqual(2, dt.month());
  assertEqual(3, dt.day());
  assertEqual(4, dt.hour());
  assertEqual(5, dt.minute());
  assertEqual(6, dt.second());
  assertEqual(0, dt.timeZone().getUtcOffset(0).toMinutes());

  date_time_mutation::incrementYear(dt);
  assertEqual((int16_t) 2002, dt.year());
  assertEqual(2, dt.month());
  assertEqual(3, dt.day());
  assertEqual(4, dt.hour());
  assertEqual(5, dt.minute());
  assertEqual(6, dt.second());
  assertEqual(0, dt.timeZone().getUtcOffset(0).toMinutes());

  date_time_mutation::incrementMonth(dt);
  assertEqual((int16_t) 2002, dt.year());
  assertEqual(3, dt.month());
  assertEqual(3, dt.day());
  assertEqual(4, dt.hour());
  assertEqual(5, dt.minute());
  assertEqual(6, dt.second());
  assertEqual(0, dt.timeZone().getUtcOffset(0).toMinutes());

  date_time_mutation::incrementDay(dt);
  assertEqual((int16_t) 2002, dt.year());
  assertEqual(3, dt.month());
  assertEqual(4, dt.day());
  assertEqual(4, dt.hour());
  assertEqual(5, dt.minute());
  assertEqual(6, dt.second());
  assertEqual(0, dt.timeZone().getUtcOffset(0).toMinutes());

  date_time_mutation::incrementHour(dt);
  assertEqual((int16_t) 2002, dt.year());
  assertEqual(3, dt.month());
  assertEqual(4, dt.day());
  assertEqual(5, dt.hour());
  assertEqual(5, dt.minute());
  assertEqual(6, dt.second());
  assertEqual(0, dt.timeZone().getUtcOffset(0).toMinutes());

  date_time_mutation::incrementMinute(dt);
  assertEqual((int16_t) 2002, dt.year());
  assertEqual(3, dt.month());
  assertEqual(4, dt.day());
  assertEqual(5, dt.hour());
  assertEqual(6, dt.minute());
  assertEqual(6, dt.second());
  assertEqual(0, dt.timeZone().getUtcOffset(0).toMinutes());
}

// --------------------------------------------------------------------------

void setup() {
#if !defined(__linux__) && !defined(__APPLE__)
  delay(1000); // wait for stability on some boards to prevent garbage Serial
#endif
  Serial.begin(115200); // ESP8266 default of 74880 not supported on Linux
  while(!Serial); // for the Arduino Leonardo/Micro only
}

void loop() {
  TestRunner::run();
}

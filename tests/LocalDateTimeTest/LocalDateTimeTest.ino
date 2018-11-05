#line 2 "LocalDateTimeTest.ino"

#include <AUnit.h>
#include <AceTime.h>

using namespace aunit;
using namespace ace_time;
using namespace ace_time::common;

// --------------------------------------------------------------------------
// LocalDate
// --------------------------------------------------------------------------

test(LocalDate, accessors) {
  LocalDate ld = LocalDate::forComponents(1, 2, 3);
  assertEqual(1, ld.year());
  assertEqual(2, ld.month());
  assertEqual(3, ld.day());
}

// Verify that toEpochDays()/forEpochDays() and
// toEpochSeconds()/forEpochSeconds() support round trip conversions when when
// isError()==true.
test(LocalDate, setError) {
  LocalDate ld;

  ld = LocalDate().setError();
  assertTrue(ld.isError());
  assertEqual(LocalDate::kInvalidEpochDays, ld.toEpochDays());
  assertEqual(LocalDate::kInvalidEpochSeconds, ld.toEpochSeconds());

  ld = LocalDate::forEpochDays(LocalDate::kInvalidEpochDays);
  assertTrue(ld.isError());

  ld = LocalDate::forEpochSeconds(LocalDate::kInvalidEpochSeconds);
  assertTrue(ld.isError());
}

test(LocalDate, dayOfWeek) {
  // year 2000 (leap year due to every 400 rule)
  assertEqual(LocalDate::kSaturday,
      LocalDate::forComponents(0, 1, 1).dayOfWeek());
  assertEqual(LocalDate::kMonday,
      LocalDate::forComponents(0, 1, 31).dayOfWeek());

  assertEqual(LocalDate::kTuesday,
      LocalDate::forComponents(0, 2, 1).dayOfWeek());
  assertEqual(LocalDate::kTuesday,
      LocalDate::forComponents(0, 2, 29).dayOfWeek());

  assertEqual(LocalDate::kWednesday,
      LocalDate::forComponents(0, 3, 1).dayOfWeek());
  assertEqual(LocalDate::kFriday,
      LocalDate::forComponents(0, 3, 31).dayOfWeek());

  assertEqual(LocalDate::kSaturday,
      LocalDate::forComponents(0, 4, 1).dayOfWeek());
  assertEqual(LocalDate::kSunday,
      LocalDate::forComponents(0, 4, 30).dayOfWeek());

  assertEqual(LocalDate::kMonday,
      LocalDate::forComponents(0, 5, 1).dayOfWeek());
  assertEqual(LocalDate::kWednesday,
      LocalDate::forComponents(0, 5, 31).dayOfWeek());

  assertEqual(LocalDate::kThursday,
      LocalDate::forComponents(0, 6, 1).dayOfWeek());
  assertEqual(LocalDate::kFriday,
      LocalDate::forComponents(0, 6, 30).dayOfWeek());

  assertEqual(LocalDate::kSaturday,
      LocalDate::forComponents(0, 7, 1).dayOfWeek());
  assertEqual(LocalDate::kMonday,
      LocalDate::forComponents(0, 7, 31).dayOfWeek());

  assertEqual(LocalDate::kTuesday,
      LocalDate::forComponents(0, 8, 1).dayOfWeek());
  assertEqual(LocalDate::kThursday,
      LocalDate::forComponents(0, 8, 31).dayOfWeek());

  assertEqual(LocalDate::kFriday,
      LocalDate::forComponents(0, 9, 1).dayOfWeek());
  assertEqual(LocalDate::kSaturday,
      LocalDate::forComponents(0, 9, 30).dayOfWeek());

  assertEqual(LocalDate::kSunday,
      LocalDate::forComponents(0, 10, 1).dayOfWeek());
  assertEqual(LocalDate::kTuesday,
      LocalDate::forComponents(0, 10, 31).dayOfWeek());

  assertEqual(LocalDate::kWednesday,
      LocalDate::forComponents(0, 11, 1).dayOfWeek());
  assertEqual(LocalDate::kThursday,
      LocalDate::forComponents(0, 11, 30).dayOfWeek());

  assertEqual(LocalDate::kFriday,
      LocalDate::forComponents(0, 12, 1).dayOfWeek());
  assertEqual(LocalDate::kSunday,
      LocalDate::forComponents(0, 12, 31).dayOfWeek());

  // year 2001
  assertEqual(LocalDate::kMonday,
      LocalDate::forComponents(1, 1, 1).dayOfWeek());
  assertEqual(LocalDate::kWednesday,
      LocalDate::forComponents(1, 1, 31).dayOfWeek());

  // year 2004 (leap year)
  assertEqual(LocalDate::kSunday,
      LocalDate::forComponents(4, 2, 1).dayOfWeek());
  assertEqual(LocalDate::kSunday,
      LocalDate::forComponents(4, 2, 29).dayOfWeek());
  assertEqual(LocalDate::kMonday,
      LocalDate::forComponents(4, 3, 1).dayOfWeek());

  // year 2099
  assertEqual(LocalDate::kThursday,
      LocalDate::forComponents(99, 1, 1).dayOfWeek());
  assertEqual(LocalDate::kThursday,
      LocalDate::forComponents(99, 12, 31).dayOfWeek());

  // year 2100 (not leap year due to every 100 rule)
  assertEqual(LocalDate::kSunday,
      LocalDate::forComponents(100, 2, 28).dayOfWeek());
  assertEqual(LocalDate::kMonday,
      LocalDate::forComponents(100, 3, 1).dayOfWeek());

  // year 2255-12-31
  assertEqual(LocalDate::kMonday,
      LocalDate::forComponents(255, 12, 31).dayOfWeek());
}

test(LocalDate, toAndFromEpochDays) {
  LocalDate ld;

  ld = LocalDate::forComponents(0, 1, 1);
  assertEqual((uint32_t) 0, ld.toEpochDays());
  assertTrue(ld == LocalDate::forEpochDays(0));

  ld = LocalDate::forComponents(0, 2, 29);
  assertEqual((uint32_t) 59, ld.toEpochDays());
  assertTrue(ld == LocalDate::forEpochDays(59));

  ld = LocalDate::forComponents(18, 1, 1);
  assertEqual((uint32_t) 6575, ld.toEpochDays());
  assertTrue(ld == LocalDate::forEpochDays(6575));

  ld = LocalDate::forComponents(99, 12, 31);
  assertEqual((uint32_t) 36524, ld.toEpochDays());
  assertTrue(ld == LocalDate::forEpochDays(36524));

  ld = LocalDate::forComponents(255, 12, 31);
  assertEqual((uint32_t) 93501, ld.toEpochDays());
  assertTrue(ld == LocalDate::forEpochDays(93501));
}

test(LocalDate, toAndFromEpochSeconds) {
  LocalDate ld;

  ld = LocalDate::forComponents(0, 1, 1);
  assertEqual((uint32_t) 0, ld.toEpochSeconds());
  assertTrue(ld == LocalDate::forEpochSeconds(0));

  ld = LocalDate::forComponents(0, 2, 29);
  assertEqual((uint32_t) 59 * 86400, ld.toEpochSeconds());
  assertTrue(ld == LocalDate::forEpochSeconds((uint32_t) 59 * 86400 + 1));

  ld = LocalDate::forComponents(18, 1, 1);
  assertEqual((uint32_t) 6575 * 86400, ld.toEpochSeconds());
  assertTrue(ld == LocalDate::forEpochSeconds((uint32_t) 6575 * 86400 + 2));

  ld = LocalDate::forComponents(99, 12, 31);
  assertEqual((uint32_t) 36524 * 86400, ld.toEpochSeconds());
  assertTrue(ld == LocalDate::forEpochSeconds(
      (uint32_t) 36524 * 86400 + 84399));

  ld = LocalDate::forComponents(136, 2, 7);
  assertEqual((uint32_t) 49710 * 86400, ld.toEpochSeconds());
  assertTrue(ld == LocalDate::forEpochSeconds((uint32_t) 49710 * 86400 + 4));
}

test(LocalDate, compareTo) {
  LocalDate a, b;

  a = LocalDate::forComponents(0, 1, 1);
  b = LocalDate::forComponents(0, 1, 1);
  assertEqual(a.compareTo(b), 0);
  assertTrue(a == b);
  assertFalse(a != b);

  a = LocalDate::forComponents(0, 1, 1);
  b = LocalDate::forComponents(0, 1, 2);
  assertLess(a.compareTo(b), 0);
  assertMore(b.compareTo(a), 0);
  assertTrue(a != b);

  a = LocalDate::forComponents(0, 1, 1);
  b = LocalDate::forComponents(0, 2, 1);
  assertLess(a.compareTo(b), 0);
  assertMore(b.compareTo(a), 0);
  assertTrue(a != b);

  a = LocalDate::forComponents(0, 1, 1);
  b = LocalDate::forComponents(1, 1, 1);
  assertLess(a.compareTo(b), 0);
  assertMore(b.compareTo(a), 0);
  assertTrue(a != b);
}

test(LocalDate, forDateString) {
  LocalDate ld;
  ld = LocalDate::forDateString("2000-01-01");
  assertTrue(ld == LocalDate::forComponents(0, 1, 1));

  ld = LocalDate::forDateString("2099-02-28");
  assertTrue(ld == LocalDate::forComponents(99, 2, 28));

  ld = LocalDate::forDateString("2255-12-31");
  assertTrue(ld == LocalDate::forComponents(255, 12, 31));
}

test(LocalDate, forDateString_invalid) {
  LocalDate ld = LocalDate::forDateString("2000-01");
  assertTrue(ld.isError());
}

test(LocalDate, isLeapYear) {
  assertTrue(LocalDate::forComponents(0, 1, 1).isLeapYear());
  assertFalse(LocalDate::forComponents(1, 1, 1).isLeapYear());
  assertTrue(LocalDate::forComponents(4, 1, 1).isLeapYear());
  assertFalse(LocalDate::forComponents(100, 1, 1).isLeapYear());
  assertFalse(LocalDate::forComponents(200, 1, 1).isLeapYear());
}

test(LocalDate, daysInMonth) {
  assertEqual(31, LocalDate::forComponents(0, 1, 1).daysInMonth());
  assertEqual(29, LocalDate::forComponents(0, 2, 1).daysInMonth());
  assertEqual(31, LocalDate::forComponents(0, 3, 1).daysInMonth());
  assertEqual(30, LocalDate::forComponents(0, 4, 1).daysInMonth());
  assertEqual(31, LocalDate::forComponents(0, 5, 1).daysInMonth());
  assertEqual(30, LocalDate::forComponents(0, 6, 1).daysInMonth());
  assertEqual(31, LocalDate::forComponents(0, 7, 1).daysInMonth());
  assertEqual(31, LocalDate::forComponents(0, 8, 1).daysInMonth());
  assertEqual(30, LocalDate::forComponents(0, 9, 1).daysInMonth());
  assertEqual(31, LocalDate::forComponents(0, 10, 1).daysInMonth());
  assertEqual(30, LocalDate::forComponents(0, 11, 1).daysInMonth());
  assertEqual(31, LocalDate::forComponents(0, 12, 1).daysInMonth());

  assertEqual(28, LocalDate::forComponents(1, 2, 1).daysInMonth());
  assertEqual(29, LocalDate::forComponents(4, 2, 1).daysInMonth());
  assertEqual(28, LocalDate::forComponents(100, 2, 1).daysInMonth());
}

// --------------------------------------------------------------------------
// LocalTime
// --------------------------------------------------------------------------

test(LocalTime, accessors) {
  LocalTime lt = LocalTime::forComponents(1, 2, 3);
  assertEqual(1, lt.hour());
  assertEqual(2, lt.minute());
  assertEqual(3, lt.second());
}

test(LocalTime, setError) {
  LocalTime lt = LocalTime().setError();
  assertTrue(lt.isError());
  assertEqual(LocalTime::kInvalidSeconds, lt.toSeconds());

  lt = LocalTime::forSeconds(LocalTime::kInvalidSeconds);
  assertTrue(lt.isError());
}

test(LocalTime, toAndFromSeconds) {
  LocalTime lt;

  lt = LocalTime::forSeconds(0);
  assertTrue(lt == LocalTime::forComponents(0, 0, 0));
  assertEqual((uint32_t) 0, lt.toSeconds());

  lt = LocalTime::forSeconds(3662);
  assertTrue(lt == LocalTime::forComponents(1, 1, 2));
  assertEqual((uint32_t) 3662, lt.toSeconds());

  lt = LocalTime::forSeconds(86399);
  assertTrue(lt == LocalTime::forComponents(23, 59, 59));
  assertEqual((uint32_t) 86399, lt.toSeconds());
}

test(LocalTime, compareTo) {
  LocalTime a, b;

  a = LocalTime::forComponents(0, 1, 1);
  b = LocalTime::forComponents(0, 1, 1);
  assertEqual(a.compareTo(b), 0);
  assertTrue(a == b);
  assertFalse(a != b);

  a = LocalTime::forComponents(0, 1, 1);
  b = LocalTime::forComponents(0, 1, 2);
  assertLess(a.compareTo(b), 0);
  assertMore(b.compareTo(a), 0);
  assertTrue(a != b);

  a = LocalTime::forComponents(0, 1, 1);
  b = LocalTime::forComponents(0, 2, 1);
  assertLess(a.compareTo(b), 0);
  assertMore(b.compareTo(a), 0);
  assertTrue(a != b);

  a = LocalTime::forComponents(0, 1, 1);
  b = LocalTime::forComponents(1, 1, 1);
  assertLess(a.compareTo(b), 0);
  assertMore(b.compareTo(a), 0);
  assertTrue(a != b);
}

test(LocalTime, forTimeString) {
  LocalTime lt;
  lt = LocalTime::forTimeString("00:00:00");
  assertTrue(lt == LocalTime::forComponents(0, 0, 0));

  lt = LocalTime::forTimeString("01:02:03");
  assertTrue(lt == LocalTime::forComponents(1, 2, 3));
}

test(LocalTime, fortimeString_invalid) {
  LocalTime lt = LocalTime::forTimeString("01:02");
  assertTrue(lt.isError());
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

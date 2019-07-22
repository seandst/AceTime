# AceTime

This library provides date and time classes for the Arduino platform with
support for time zones in the [IANA TZ
database](https://www.iana.org/time-zones). Date and time from one timezone can
be converted to another timezone. The library also provides a SystemClock that
can be synchronized from a more reliable external time source, such as an
[NTP](https://en.wikipedia.org/wiki/Network_Time_Protocol) server or a [DS3231
RTC](https://www.maximintegrated.com/en/products/analog/real-time-clocks/DS3231.html)
chip. This library can be an alternative to the [Arduino
Time](https://github.com/PaulStoffregen/Time) and [Arduino
Timezone](https://github.com/JChristensen/Timezone) libraries.

The AceTime classes are organized into roughly 4 bundles, placed in different
C++ namespaces:

* date and time classes and types
    * `ace_time::common::DateStrings`
    * `ace_time::acetime_t`
    * `ace_time::LocalTime`
    * `ace_time::LocalDate`
    * `ace_time::LocalDateTime`
    * `ace_time::TimeOffset`
    * `ace_time::OffsetDateTime`
    * `ace_time::ZoneProcessor`
        * `ace_time::BasicZoneProcessor`
        * `ace_time::ExtendedZoneProcessor`
    * `ace_time::TimeZone`
    * `ace_time::ZonedDateTime`
    * `ace_time::TimePeriod`
    * `ace_time::BasicZone`
    * `ace_time::ExtendedZone`
    * `ace_time::BasicZoneManager`
    * `ace_time::ExtendedZoneManager`
    * mutation helpers
        * `ace_time::local_date_mutation::`
        * `ace_time::time_offset_mutation::`
        * `ace_time::time_period_mutation::`
        * `ace_time::zoned_date_time_mutation::`
* TZ Database zone files
    * C++ files generated by a code-generator from the TZ Database zone files
    * `ace_time::zonedb` (270 zones and 182 links)
        * `ace_time::zonedb::kZoneAfrica_Abidjan`
        * `ace_time::zonedb::kZoneAfrica_Accra`
        * ...
        * `ace_time::zonedb::kZonePacific_Wake`
        * `ace_time::zonedb::kZonePacific_Wallis`
    * `ace_time::zonedbx` (387 zones and 205 links)
        * `ace_time::zonedbx::kZoneAfrica_Abidjan`
        * `ace_time::zonedbx::kZoneAfrica_Accra`
        * ...
        * `ace_time::zonedbx::kZonePacific_Wake`
        * `ace_time::zonedbx::kZonePacific_Wallis`
* system clock classes
    * `ace_time::clock::TimeProvider`
        * `ace_time::clock::TimeKeeper`
            * `ace_time::clock::DS3231TimeKeeper`
            * `ace_time::clock::SystemClock`
        * `ace_time::clock::NtpTimeProvider`
    * `ace_time::clock::SystemClockSyncCoroutine`
    * `ace_time::clock::SystemClockSyncLoop`
* internal helper classes (not normally used by app developers)
    * `ace_time::basic::ZoneContext`
    * `ace_time::basic::ZoneEra`
    * `ace_time::basic::ZoneInfo`
    * `ace_time::basic::ZonePolicy`
    * `ace_time::basic::ZoneRule`
    * `ace_time::extended::ZoneContext`
    * `ace_time::extended::ZoneInfo`
    * `ace_time::extended::ZoneEra`
    * `ace_time::extended::ZonePolicy`
    * `ace_time::extended::ZoneRule`

The "date and time" classes provide an abstraction layer to make it easier
to use and manipulate date and time fields. For example, each of the
`LocalDateTime`, `OffsetDateTime` and `ZonedDateTime` classes provide the
`toEpochSeconds()` which returns the number of seconds from a epoch date, the
`forEpochSeconds()` which constructs the date and time fields from the epoch
seconds, the `forComponents()` method which constructs the object from the
individual (year, month, day, hour, minute, second) components, and the
`dayOfWeek()` method which returns the day of the week of the given date.

The Epoch in AceTime is defined to be 2000-01-01T00:00:00Z, in contrast to the
Epoch in Unix which is 1970-01-01T00:00:00Z. Internally, the current time is
represented as "seconds from Epoch" stored as a 32-bit signed integer
(`acetime_t` aliased to `int32_t`). Internally, the smallest 32-bit signed
integer (`-2^31`) is used to indicate an internal Error condition, so the
smallest valid `acetime_t` value is `-2^31+1`. The largest valid value is
`2^31-1`. Therefore, the range of dates that the `acetime_t` type can handle is
1931-12-13T20:45:53Z to 2068-01-19T03:14:07Z (inclusive). (In contrast, the
32-bit Unix `time_t` range is 1901-12-13T20:45:52Z to 2038-01-19T03:14:07Z).

The various date classes (`LocalDate`, `LocalDateTime`, `OffsetDateTime`,
`ZonedDateTime`) store the year component as a signed 8-bit integer offset from
the year 2000. The range of this integer is -128 to +127, but -128 is used to
indicate an internal Error condition, so the actual range is -127 to +127.
Therefore, these classes can represent dates from 1873-01-01T00:00:00 to
2127-12-31T23:59:59 (inclusive). Notice that these classes can represent all
dates that can be expressed by the `acetime_t` type, but the reverse is not
true. There are date objects that cannot be converted into a valid `acetime_t`
value. To be safe, users of this library should stay at least 1 day away from
the lower and upper limits of `acetime_t` (i.e. stay within the year 1932 to
2067, inclusive).

The `ZonedDateTime` class works with the `TimeZone` class to provide access to
the TZ Database and allow conversions to other timezones using the
`ZonedDateTime::convertToTimeZone()` method.

The library provides 2 sets of zoneinfo files created from the IANA TZ Database:

* [zonedb/zone_infos.h](src/ace_time/zonedb/zone_infos.h) contains `kZone*`
  declarations (e.g. `kZoneAmerica_Los_Angeles`) for 270 zones and 182 links
  which have (relatively) simple time zone transition rules, and intended to be
  used with the `BasicZoneProcessor` class.
* [zonedbx/zone_infos.h](src/ace_time/zonedbx/zone_infos.h) contains `kZone*`
  (e.g. `kZoneAfrica_Casablanca`) declarations for 387 zones and 205 links in
  the TZ Database (essentially the entire database) intended to be used with
  the `ExtendedZoneProcessor` class.

These zoneinfo files have been validated to match the UTC offsets calculated
using the Python [pytz](https://pypi.org/project/pytz/) library from the year
2000 until 2037 (inclusive), and using the [Java 11
Time](https://docs.oracle.com/en/java/javase/11/docs/api/java.base/java/time/package-summary.html)
library from year 2000 to 2049 (inclusive). Custom datasets with smaller or
larger range of years may be generated by developers using scripts provided in
this library (although this is not documented currently). The target application
may be compiled against the custom dataset instead of using `zonedb::` and
`zonedbx::` zone files provided in this library.

It is expected that an application using AceTime will use only a small subset of
these timezones at the same time (1 to 3 zones have been tested). The C++
compiler will include only the subset of zoneinfo files needed to support those
timezones, instead of compiling in the entire TZ database.

The `ace_time::clock` classes collaborate together to implement the
SystemClock which can obtain its time from various sources, such as a DS3231 RTC
chip, or an Network Time Protocol (NTP) server. Retrieving the current time
from accurate clock sources can be expensive, so the `SystemClock` uses the
built-in `millis()` function to provide fast access to a reasonably accurate
clock, but synchronizes to more accurate clocks periodically.

This library does not perform dynamic allocation of memory, in other words,
it does not call the `new` operator nor the `malloc()` function, and it does not
use the Arduino `String` class. Everything it needs is allocated statically at
initialization time.

The zoneinfo files are stored in flash memory (using the `PROGMEM` compiler
directive) whenever possible so that they do not consume static RAM:

* 270 timezones supported by `BasicZoneProcessor`consume:
    * 14 kB of flash on an 8-bit processor
    * 21 kB of flash on a 32-bit processor
* 387 timezones supported by `ExtendedZoneProcessor` consume:
    * 23 kB of flash on an 8-bit processor
    * 37 kB of flash on a 32-bit processor

Normally a small application will use only a small number of timezones. The
AceTime library with one timezone using the `BasicZoneProcessor` and the
`SystemClock` consumes:
* 8.5 kB of flash and 350 bytes of RAM on an 8-bit Arduino Nano (AVR),
* 11 kB of flash and 850 bytes of RAM on an ESP8266 processor (32-bit).

An example of more complex application is the [WorldClock](examples/WorldClock)
which has 3 OLED displays over SPI, 3 timezones using `BasicZoneProcessor`, a
`SystemClock` synchronized to a DS3231 chip on I2C, and 2 buttons with
debouncing and event dispatching provided by the
[AceButton](https://github.com/bxparks/AceButton) library. This application fits
inside the 30 kB flash size of an Arduino Pro Micro controller.

Conversion from date-time components (year, month, day, etc) to epochSeconds
(`ZonedDateTime::toEpochSeconds()`) takes about:
* 90 microseconds on an 8-bit AVR processor,
* 7 microseconds on an ESP8266,
* 1.4 microseconds on an ESP32,
* 0.5 microseconds on a Teensy 3.2.

Conversion from an epochSeconds to date-time components including timezone
(`ZonedDateTime::forEpochSeconds()`) takes (assuming cache hits):
* 600 microseconds on an 8-bit AVR,
* 25 microseconds on an ESP8266,
* 2.5 microseconds on an ESP32,
* 6 microseconds on a Teensy 3.2.

**Version**: 0.5 (2019-07-21, TZ DB version 2019a, beta)

**Status**: Fully functional. Added `ZoneManager` for dynamic binding of
zoneName or zoneId to the TimeZone.

## HelloDateTime

Here is a simple program (see [examples/HelloDateTime](examples/HelloDateTime))
which demonstrates how to create and manipulate date and times in different time
zones:

```C++
#include <AceTime.h>

using namespace ace_time;

// ZoneProcessor instances should be created statically at initialization time.
static BasicZoneProcessor pacificProcessor;
static BasicZoneProcessor londonProcessor;

void setup() {
  delay(1000);
  Serial.begin(115200);
  while (!Serial); // Wait until Serial is ready - Leonardo/Micro

  auto pacificTz = TimeZone::forZoneInfo(&zonedb::kZoneAmerica_Los_Angeles,
        &pacificProcessor);
  auto londonTz = TimeZone::forZoneInfo(&zonedb::kZoneEurope_London,
        &londonProcessor);

  // Create from components. 2019-03-10T03:00:00 is just after DST change in
  // Los Angeles (2am goes to 3am).
  auto startTime = ZonedDateTime::forComponents(
      2019, 3, 10, 3, 0, 0, pacificTz);

  Serial.print(F("Epoch Seconds: "));
  acetime_t epochSeconds = startTime.toEpochSeconds();
  Serial.println(epochSeconds);

  Serial.print(F("Unix Seconds: "));
  acetime_t unixSeconds = startTime.toUnixSeconds();
  Serial.println(unixSeconds);

  Serial.println(F("=== Los_Angeles"));
  auto pacificTime = ZonedDateTime::forEpochSeconds(epochSeconds, pacificTz);
  Serial.print(F("Time: "));
  pacificTime.printTo(Serial);
  Serial.println();

  Serial.print(F("Day of Week: "));
  Serial.println(
      common::DateStrings().dayOfWeekLongString(pacificTime.dayOfWeek()));

  // Print info about UTC offset
  TimeOffset offset = pacificTime.timeOffset();
  Serial.print(F("Total UTC Offset: "));
  offset.printTo(Serial);
  Serial.println();

  // Print info about the current time zone
  Serial.print(F("Zone: "));
  pacificTz.printTo(Serial);
  Serial.println();

  // Print the current time zone abbreviation, e.g. "PST" or "PDT"
  Serial.print(F("Abbreviation: "));
  pacificTz.printAbbrevTo(Serial, epochSeconds);
  Serial.println();

  // Create from epoch seconds. London is still on standard time.
  auto londonTime = ZonedDateTime::forEpochSeconds(epochSeconds, londonTz);

  Serial.println(F("=== London"));
  Serial.print(F("Time: "));
  londonTime.printTo(Serial);
  Serial.println();

  // Print info about the current time zone
  Serial.print(F("Zone: "));
  londonTz.printTo(Serial);
  Serial.println();

  // Print the current time zone abbreviation, e.g. "PST" or "PDT"
  Serial.print(F("Abbreviation: "));
  londonTz.printAbbrevTo(Serial, epochSeconds);
  Serial.println();

  Serial.println(F("=== Compare ZonedDateTime"));
  Serial.print(F("pacificTime.compareTo(londonTime): "));
  Serial.println(pacificTime.compareTo(londonTime));
  Serial.print(F("pacificTime == londonTime: "));
  Serial.println((pacificTime == londonTime) ? "true" : "false");
}

void loop() {
}
```

Running this should produce the following on the Serial port:
```
Epoch Seconds: 605527200
Unix Seconds: 1552212000
=== Los Angeles
Time: 2019-03-10T03:00:00-07:00[America/Los_Angeles]
Day of Week: Sunday
Total UTC Offset: -07:00
Zone: America/Los_Angeles
Abbreviation: PDT
=== London
Time: 2019-03-10T10:00:00+00:00[Europe/London]
Zone: Europe/London
Abbreviation: GMT
=== Compare ZonedDateTime
pacificTime.compareTo(londonTime): 0
pacificTime == londonTime: false
```

## HelloSystemClock

This is the example code for using the `SystemClock` taken from
[examples/HelloSystemClock](examples/HelloSystemClock).

```C++
#include <AceTime.h>

using namespace ace_time;
using namespace ace_time::clock;

// ZoneProcessor instances should be created statically at initialization time.
static BasicZoneProcessor pacificProcessor;

SystemClock systemClock(nullptr /*sync*/, nullptr /*backup*/);

//------------------------------------------------------------------

void setup() {
  delay(1000);
  Serial.begin(115200);
  while (!Serial); // Wait until Serial is ready - Leonardo/Micro

  systemClock.setup();

  // Creating timezones is cheap, so we can create them on the fly as needed.
  auto pacificTz = TimeZone::forZoneInfo(&zonedb::kZoneAmerica_Los_Angeles,
      &pacificProcessor);

  // Set the SystemClock using these components.
  auto pacificTime = ZonedDateTime::forComponents(
      2019, 6, 17, 19, 50, 0, pacificTz);
  systemClock.setNow(pacificTime.toEpochSeconds());
}

//------------------------------------------------------------------

void printCurrentTime() {
  acetime_t now = systemClock.getNow();

  // Create a time
  auto pacificTz = TimeZone::forZoneInfo(&zonedb::kZoneAmerica_Los_Angeles,
      &pacificProcessor);
  auto pacificTime = ZonedDateTime::forEpochSeconds(now, pacificTz);
  pacificTime.printTo(Serial);
  Serial.println();
}

void loop() {
  printCurrentTime();
  systemClock.keepAlive(); // should be called every 65.535s or less
  delay(2000);
}
```

This will start by setting the SystemClock to 2019-06-17T19:50:00-07:00,
then printing the system time every 2 seconds:
```
2019-06-17T19:50:00-07:00[America/Los_Angeles]
2019-06-17T19:50:02-07:00[America/Los_Angeles]
2019-06-17T19:50:04-07:00[America/Los_Angeles]
```

## Example: WorldClock

Here is a photo of the [WorldClock](examples/WorldClock) that supports 3
OLED displays with 3 timezones, and automatically adjusts the DST transitions
for all 3 zones:

![WorldClock](examples/WorldClock/WorldClock.jpg)

## User Guide

See the [AceTime User Guide](USER_GUIDE.md) for information on:
* Installation
* Date and Time classes
* Mutations
* System Clock classes
* Error Handling
* Benchmarks
* Comparison to Other Libraries
* Bugs and Limitations

## System Requirements

### Tool Chain

This library was developed and tested using:

* [Arduino IDE 1.8.9](https://www.arduino.cc/en/Main/Software)
* [Arduino AVR Core 1.6.23](https://github.com/arduino/ArduinoCore-avr)
* [SparkFun AVR Core 1.1.12](https://github.com/sparkfun/Arduino_Boards)
* [ESP8266 Arduino Core 2.5.2](https://github.com/esp8266/Arduino)
* [ESP32 Arduino Core 1.0.2](https://github.com/espressif/arduino-esp32)
* [Teensydino 1.46](https://www.pjrc.com/teensy/td_download.html)

It should work with [PlatformIO](https://platformio.org/) but I have
not tested it.

### Operating System

I use Ubuntu 18.04 for the vast majority of my development. I expect that the
library will work fine under MacOS and Windows, but I have not tested them.

### Hardware

The library is tested on the following hardware before each release:

* Arduino Nano clone (16 MHz ATmega328P)
* SparkFun Pro Micro clone (16 MHz ATmega32U4)
* WeMos D1 Mini clone (ESP-12E module, 80 MHz ESP8266)
* ESP32 dev board (ESP-WROOM-32 module, 240 MHz dual core Tensilica LX6)

I will occasionally test on the following hardware as a sanity check:

* Teensy 3.2 (72 MHz ARM Cortex-M4)

## Changelog

See [CHANGELOG.md](CHANGELOG.md).

## License

[MIT License](https://opensource.org/licenses/MIT)

## Feedback and Support

If you have any questions, comments, bug reports, or feature requests, please
file a GitHub ticket instead of emailing me unless the content is sensitive.
(The problem with email is that I cannot reference the email conversation when
other people ask similar questions later.) I'd love to hear about how this
software and its documentation can be improved. I can't promise that I will
incorporate everything, but I will give your ideas serious consideration.

## Authors

Created by Brian T. Park (brian@xparks.net).

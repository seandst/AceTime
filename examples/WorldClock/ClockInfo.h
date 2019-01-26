#ifndef WORLD_CLOCK_CLOCK_INFO_H
#define WORLD_CLOCK_CLOCK_INFO_H

#include <AceTime.h>

/** Data that describes the clock of a single time zone. */
struct ClockInfo {
  /** Size of the clock name buffer, including '\0'. */
  static uint8_t const kNameSize = 5;

  /** 12:00:00 AM to 12:00:00 PM */
  static uint8_t const kTwelve = 0;

  /** 00:00:00 - 23:59:59 */
  static uint8_t const kTwentyFour = 1;

  /** Name of this clock, e.g. City or Time Zone ID */
  const char* name;

  /** Hour mode, 12H or 24H. */
  uint8_t hourMode = kTwelve;

  /** Blink the colon in HH:MM. */
  bool blinkingColon = false;

  /** Time zone of the clock. */
  ace_time::TimeZone timeZone;

#if TIME_ZONE_TYPE == TIME_ZONE_TYPE_AUTO
  ace_time::AutoZoneSpecifier zoneSpecifier;
#else
  ace_time::ManualZoneSpecifier zoneSpecifier;
#endif
};

#endif

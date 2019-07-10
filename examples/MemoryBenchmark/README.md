# MemoryBenchmark

The `MemoryBenchmark.ino` was compiled with each `FEATURE_*` and the flash
memory and static RAM sizes were recorded. The `FEATURE_BASELINE` selection is
the baseline, and its memory usage  numbers are subtracted from the subsequent
`FEATURE_*` memory usage.

## Arduino Nano

* AceTime 0.4
* Arduino IDE 1.8.9
* AVR Core 1.6.23

```
+------------------------------------------------------------------+
| Functionality                       | flash/ram | Baseline Delta |
|-------------------------------------+-----------+----------------|
| Baseline                            |   448/ 10 |        0/  0   |
|-------------------------------------+-----------+----------------|
| LocalDateTime                       |  1452/147 |     1003/137   |
| ZonedDateTime                       |  2278/147 |     1830/137   |
| ManualZoneSpecifier                 |  3194/171 |     2746/161   |
| BasicZoneSpecifier                  |  6588/203 |     6140/193   |
| BasicZoneSpecifier (2 zones)        |  7076/241 |     6628/231   |
| BasicZoneSpecifier (all zones)      | 20802/611 |    20354/601   |
| ExtendedZoneSpecifier               |  9034/203 |     8586/193   |
| ExtendedZoneSpecifier (2 zones)     |  9632/209 |     9184/199   |
| ExtendedZoneSpecifier (all zones)   | 32338/713 |    31890/703   |
|-------------------------------------+-----------+----------------|
| SystemClock                         |  4750/276 |     4302/266   |
| SystemClock+BasicZoneSpecifier      |  8884/354 |     8436/344   |
+------------------------------------------------------------------+
```

## ESP8266

* AceTime 0.4
* Arduino IDE 1.8.9
* ESP8266 Core 2.5.2
* ESP32 Core 1.0.2

```
+---------------------------------------------------------------------+
| Functionality                       |    flash/ram | Baseline Delta |
|-------------------------------------+--------------+----------------|
| Baseline                            | 257104/26540 |        0/   0  |
|-------------------------------------+--------------+----------------|
| LocalDateTime                       | 260272/26944 |     3168/ 404  |
| ZonedDateTime                       | 260720/26944 |     3680/ 404  |
| ManualZoneSpecifier                 | 261432/26944 |     4328/ 404  |
| BasicZoneSpecifier                  | 265820/27364 |     8716/ 824  |
| BasicZoneSpecifier (2 zones)        | 266236/27364 |    91322/ 824  |
| BasicZoneSpecifier (all zones)      | 287248/27392 |    30144/ 852  |
| ExtendedZoneSpecifier               | 267728/27472 |    10624/ 932  |
| ExtendedZoneSpecifier (2 zones)     | 268256/27472 |    11152/ 938  |
| ExtendedZoneSpecifier (all zones)   | 304644/27492 |    47540/ 952  |
|-------------------------------------+--------------+----------------|
| SystemClock                         | 263820/26960 |     6716/ 420  |
| SystemClock+BasicZoneSpecifier      | 268428/27376 |    11324/ 836  |
+---------------------------------------------------------------------+
```

The static RAM usage jump from 404 bytes to 4628 bytes when `BasicZoneSpecifier`
is added is a bit surprising since `sizeof(BasicZoneSpecifier)` is only 140
bytes. Fortunately, the ESP8266 has at least 80kB of RAM so this should not be
an issue.

## ESP32

* AceTime 0.4
* Arduino IDE 1.8.9
* ESP32 Core 1.0.2

```
+---------------------------------------------------------------------+
| Functionality                       |    flash/ram | Baseline Delta |
|-------------------------------------+--------------+----------------|
| Baseline                            | 193200/12680 |        0/   0  |
|-------------------------------------+--------------+----------------|
| LocalDateTime                       | 203796/14156 |    10596/1476  |
| ZonedDateTime                       | 204240/14156 |    11040/1476  |
| ManualZoneSpecifier                 | 204900/14156 |    11700/1476  |
| BasicZoneSpecifier                  | 208136/14156 |    14936/1476  |
| BasicZoneSpecifier (2 zones)        | 208576/14156 |    15376/1476  |
| BasicZoneSpecifier (all zones)      | 229388/14156 |    36188/1476  |
| ExtendedZoneSpecifier               | 209768/14156 |    16568/1476  |
| ExtendedZoneSpecifier (2 zones)     | 210292/14156 |    17092/1476  |
| ExtendedZoneSpecifier (all zones)   | 246536/14156 |    53336/1476  |
|-------------------------------------+--------------+----------------|
| SystemClock                         | 211480/14260 |    18280/1580  |
| SystemClock+BasicZoneSpecifier      | 215224/14260 |    22024/1580  |
+---------------------------------------------------------------------+
```

RAM usage remains constant as more objects are created, which indicates that an
initial pool of a certain minimum size is created regardless of the actual RAM
usage by objects.

## Teensy 3.2

* AceTime 0.4
* Arduino IDE 1.8.9
* Teensyduino 1.46

```
+---------------------------------------------------------------------+
| Functionality                       |    flash/ram | Baseline Delta |
|-------------------------------------+--------------+----------------|
| Baseline                            |  88480/ 3436 |        0/   0  |
|-------------------------------------+--------------+----------------|
| LocalDateTime                       |  14492/ 5204 |     5644/1768  |
| ZonedDateTime                       |  14492/ 5204 |     5644/1768  |
| ManualZoneSpecifier                 |  16180/ 5204 |     7332/1768  |
| BasicZoneSpecifier                  |  21852/ 5204 |    13004/1768  |
| BasicZoneSpecifier (2 zones)        |  22596/ 5204 |    13748/1768  |
| BasicZoneSpecifier (all zones)      |  43856/ 5204 |    35008/1768  |
| ExtendedZoneSpecifier               |  25196/ 5204 |    16348/1768  |
| ExtendedZoneSpecifier (2 zones)     |  25940/ 5204 |    17092/1768  |
| ExtendedZoneSpecifier (all zones)   |  62656/ 5204 |    53808/1768  |
|-------------------------------------+--------------+----------------|
| SystemClock                         |  17368/ 5204 |     8520/1768  |
| SystemClock+BasicZoneSpecifier      |  24960/ 5204 |    16112/1768  |
+---------------------------------------------------------------------+
```
# myIOT2 v3

ESP8266/ESP32 IoT base platform. Handles WiFi, MQTT, NTP, OTA and flash
persistence so a device sketch can be about the device.

Self-contained. No UI, no web server, no external library beyond the three
dependencies below. It runs alone.

## Features

- WiFi and MQTT connection management with automatic reconnect
- NTP time sync, POSIX timezone support
- OTA updates, time-windowed and off by default
- Config and MQTT topics persisted to flash (LittleFS via myJflash)
- Reset safety — detects reboot loops and falls back to a known-good state
- AP mode fallback — reconfigure credentials without reflashing
- Optional log sink — mirror log output to any `Print` object

## Install

PlatformIO, pinned to a tag:

```ini
lib_deps =
    https://github.com/guydvir2/myIOT2.git#v3.0.2
```

## Setup

Copy `secretsIOT_example.h` to `secretsIOT.h` and fill in your defaults.
`secretsIOT.h` is gitignored and must never be committed.

Minimal sketch:

```cpp
#include <myIOT2.h>

myIOT2 iot;

void mqttCallback(char *msg, char *topic)
{
    iot.inline_read(msg);   // splits msg into iot.inline_param[0..3]
}

void setup()
{
    iot.start_services(mqttCallback);
}

void loop()
{
    iot.looper();
}
```

## Log sink

Log output goes to Serial. A sketch may additionally mirror it to any
`Print`-derived object by assigning the sink pointer:

```cpp
iotLogSink = &myPrintObject;
```

Null by default, meaning Serial only. myIOT2 does not know or care what is
attached. Cost when unused is one null pointer test per log line.

## Dependencies

- ArduinoJson ^7.4.3
- PubSubClient ^2.8
- myJflash

## Versioning

The version string in `myIOT2.h` (`ver[]`), the `version` field in
`library.json`, and the git tag must always agree.

## License

MIT
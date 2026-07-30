# myIOT2

A lightweight IoT base platform for **ESP8266 / ESP32** built on the Arduino framework.  
Handles everything below the application layer so your `main.cpp` stays clean.

---

## What it does

| Feature | Details |
|---|---|
| **WiFi** | Auto-connect, reconnect, power-outage vs relocation detection |
| **MQTT** | Pub/sub, auto-reconnect, retained availability topic |
| **NTP** | Time sync, timestamped log messages |
| **OTA** | Over-the-air firmware updates (enable/disable at runtime) |
| **Flash persistence** | Network config and MQTT topics stored in LittleFS JSON files |
| **Reset safety** | Crash-loop detection — reverts to safe topics after repeated reboots |
| **Telemetry** | Publishes device info to `iot2_directory/ESP_XXXXXX` on connect + every 5 min |
| **Boot log** | Publishes IP, RSSI, uptime, and registered topics on first MQTT connect |

Works fully standalone — no web portal required.

---

## Dependencies

- [PubSubClient](https://github.com/knolleary/pubsubclient)
- [ArduinoJson v7](https://arduinojson.org/)
- [myJflash](https://github.com/guydvir2/myJflash) — LittleFS JSON read/write helper

---

## Installation

Add to your `platformio.ini`:

```ini
lib_deps =
    https://github.com/guydvir2/myIOT2#v3.0
```

---

## Quick start

### 1. `secretsIOT.h` — credentials (never commit this)

```cpp
#define SSID_ID   "your_wifi_ssid"
#define PASS_WIFI "your_wifi_password"
#define MQTT_SERVER "192.168.1.x"
#define MQTT_USER   "user"
#define MQTT_PASS   "pass"
```

Add to `.gitignore`:
```
include/secretsIOT.h
```

### 2. `iot2_config.h` — device setup

```cpp
#include <myIOT2.h>
#include "secretsIOT.h"

extern myIOT2 iot;
extern void mqttCallback(char *msg, char *topic);

void setupTopics()
{
    iot.add_pubTopic("myHome/Device/Avail");  // pub[0] required
    iot.add_pubTopic("myHome/Device/State");  // pub[1] optional
    iot.add_subTopic("myHome/Device");        // sub[0] required
    iot.add_gen_pubTopic("myHome/Messages");  // gen[0]
    iot.add_gen_pubTopic("myHome/log");       // gen[1]
    iot.add_gen_pubTopic("myHome/debug");     // gen[2]
}

void setupIOT()
{
    iot.useSerial          = true;
    iot.ignore_boot_msg    = false;
    iot.noNetwork_reset    = 8;       // reset after 8 min with no network
    iot.setOtaEnabled(true);
    iot.setResetSafetyConfig(true);

    iot.start_services(mqttCallback);

    if (!iot.topicsReady())   // flash topics take priority; fall back to hardcoded
        setupTopics();
}
```

### 3. `main.cpp`

```cpp
#include <myIOT2.h>
#include "iot2_config.h"

myIOT2 iot;

void mqttCallback(char *msg, char *topic)
{
    iot.inline_read(msg);  // splits into iot.inline_param[0..3]

    if (strcmp(iot.inline_param[0], "reboot") == 0)
        iot.sendReset();
}

void setup() { setupIOT(); }
void loop()  { iot.looper(); }
```

---

## Topic layout

myIOT2 uses three topic arrays — registered in order before `start_services()`:

| Array | Index | Role |
|---|---|---|
| `topics_pub` | 0 | Availability (`online` / `offline`) — **required** |
| `topics_pub` | 1 | State publish — optional |
| `topics_pub` | 2–6 | Extra publish topics |
| `topics_sub` | 0 | Command topic (incoming MQTT) — **required** |
| `topics_sub` | 1–5 | Extra subscribe topics |
| `topics_gen_pub` | 0 | Messages |
| `topics_gen_pub` | 1 | Log |
| `topics_gen_pub` | 2 | Debug |

Topics can be registered in code (`setupTopics`) **or** persisted to flash via the WebPortal.  
Flash always wins — hardcoded topics are skipped if flash topics are present.

---

## Flash persistence

Two JSON files stored in LittleFS:

| File | Contents |
|---|---|
| `/netconfig.JSON` | WiFi SSID/password, MQTT server/user/password, device name, timezone, OTA, reset safety |
| `/topics.JSON` | All pub/sub/gen topics |

Managed via the [WebPortal](https://github.com/guydvir2/WebPortal) library, or directly via `persistConfig()` / `persistTopics()`.

Delete config: `iot.deleteConfig()` — reverts to compiled-in defaults on next boot.  
Delete topics: `iot.deleteTopics()` — MQTT stays idle until topics are reconfigured.

---

## Telemetry

On every MQTT connect and every 5 minutes, the device publishes a **retained** JSON payload to:

```
iot2_directory/ESP_XXXXXX
```

Payload fields: `ip`, `deviceName`, `mainTopic`, `ssid`, `rssi`, `bootTime`, `lastSeen`, `uptime`, `ver`

Use this topic to discover and monitor all devices on your network from a single MQTT subscription.

---

## Key API

```cpp
// Lifecycle
void start_services(cb_func callback);
void looper();

// Topics
void add_pubTopic(const char *topic);
void add_subTopic(const char *topic);
void add_gen_pubTopic(const char *topic);
bool topicsReady();
bool loadPersistedTopics();
bool persistTopics(...);

// Publish helpers
void pub_log(const char *msg);
void pub_msg(const char *msg);
void pub_debug(const char *msg);
void pub_state(const char *msg, bool retained = true);

// Status
bool isWifiConnected();
bool isMqttConnected();
time_t now();
uint8_t getBootCounter();

// Config setters (persisted to flash)
bool setDeviceName(const char *value);
bool setSsid(const char *value);
bool setMqttServer(const char *value);
bool setOtaEnabled(bool value);
bool setResetSafetyConfig(bool value);

// Actions
void sendReset(const char *header = nullptr);
bool deleteConfig();
bool deleteTopics();
void startAP();
```

---

## Compatible with

- [WebPortal v0.1](https://github.com/guydvir2/WebPortal) — optional web UI for configuration and monitoring

---

## License

MIT

# myIOT2 v3 + WebPortal v0.1

ESP8266/ESP32 IoT base platform with a built-in web configuration portal.

## What it does

- WiFi, MQTT, NTP, OTA — managed automatically
- Web portal served locally — no cloud, no internet dependency
- Configure network, MQTT, topics from any browser on the local network
- Serial capture — view device logs in the browser terminal tab
- Reset safety — detects and survives reboot loops
- AP mode fallback — reconfigure credentials without reflashing

## Project structure

```
lib/
  myIOT2/         — core IoT platform
  WebPortal/      — web config portal (optional)
  SerialCapture/  — serial log capture (optional)
src/
  main.cpp        — your device sketch
reference_examples/
  example_myIOT2_WebPortal.cpp  — full feature example
```

## Getting started

1. Copy `lib/myIOT2/secretsIOT_example.h` to `lib/myIOT2/secretsIOT.h`
2. Fill in your WiFi and MQTT credentials
3. Copy `reference_examples/example_myIOT2_WebPortal.cpp` to `src/main.cpp`
4. Build and flash
5. Open browser at device IP — portal is at port 80

## First boot

On first boot with no saved topics, MQTT stays idle but WiFi and the portal come up normally. Set your topics in the **Topics** tab and save — device reboots and MQTT connects.

## Dependencies

- ArduinoJson >= 7.0
- PubSubClient
- [myJflash](https://github.com/guydvir2/myJflash)

## WebPortal features

| Tab | What it does |
|-----|-------------|
| Status | Live connection indicators, device parameters, custom data rows |
| Network | WiFi, MQTT credentials, device name, timezone |
| Topics | MQTT topic table (6 standard + 5 extra pub/sub) |
| Behavior | OTA, serial, reset safety, terminal |
| Terminal | Live serial log (when terminal enabled) |
| Maintenance | Reboot, AP mode, delete credentials/topics |

## Portal buttons (Controls section)

Define up to 4 buttons in your sketch — toggle or momentary, with custom labels and callbacks:

```cpp
portal.setButton(0, "Arm Alarm",  true,  onAlarmToggle);  // toggle
portal.setButton(1, "Open",       false, onOpen);          // momentary
```

## AP mode

Press **Switch to AP mode** in the Maintenance tab. Connect to `ESP_XXXXXX` on your phone/laptop, then open `192.168.4.1` in the browser to reconfigure credentials.

## License

MIT

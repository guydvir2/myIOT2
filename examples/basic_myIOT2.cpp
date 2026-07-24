// ============================================================
// basic_myIOT2.cpp
// Minimal example for myIOT2 v3 — no WebPortal.
//
// Shows: WiFi, MQTT, NTP, OTA, reset safety, flash params.
// Copy to src/main.cpp in your project.
// ============================================================

#include "myIOT2.h"

myIOT2 iot;

// ============================================================
// OPTIONAL: enable reset safety
// Detects rapid reboot loops and halts before damage occurs.
// Call in setup() before start_services().
// ============================================================
// iot.strtClk_rstSft(3);  // halt after 3 rapid reboots

// ============================================================
// MQTT CALLBACK
// Called when a message arrives on any subscribed topic.
// msg = payload, topic = topic string
// ============================================================
void mqttCallback(char *msg, char *topic)
{
    // Parse inline params from msg (space-separated)
    // iot.inline_read(msg);
    // iot.inline_param[0] = first param, [1] = second, etc.

    // Example: respond to a command
    // if (strcmp(msg, "status") == 0)
    //     iot.pub_state("OK");
}

void setup()
{
    Serial.begin(115200);

    // ~~~ OPTIONAL: OTA ~~~
    // iot.setOtaEnabled(true);

    // ~~~ OPTIONAL: Reset safety ~~~
    // iot.setResetSafetyConfig(true);
    // iot.setResetSafetyThreshold(3);

    // ~~~ OPTIONAL: suppress boot messages ~~~
    // iot.ignore_boot_msg = true;

    // ~~~ OPTIONAL: no-network reset timer (minutes) ~~~
    // iot.noNetwork_reset = 8;

    // Start WiFi, MQTT, NTP — credentials from secretsIOT.h
    // or overridden by saved /netconfig.JSON on flash.
    iot.start_services(mqttCallback);

    // ~~~ OPTIONAL: publish boot message ~~~
    // iot.pub_msg("Device online");
}

void loop()
{
    iot.looper();  // handles WiFi/MQTT reconnect, OTA, reset safety

    // your device logic here
}

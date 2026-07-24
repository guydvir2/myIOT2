// ============================================================
// example_myIOT2_WebPortal.cpp
// Full feature example for myIOT2 v3 + WebPortal v0.1
//
// HOW TO USE:
//   - Mandatory: fill in onGetConfig, onSetConfig, onGetStatus
//   - Optional features: uncomment to enable
//   - Copy to src/main.cpp in your project
// ============================================================

#include "myIOT2.h"
#include "WebPortal.h"
#include "SerialCapture.h"

myIOT2 iot;
WebPortal portal;

// ============================================================
// YOUR DEVICE STATE
// Update these from your own logic — they appear in Device Data
// ============================================================
static bool  deviceActive = false;
static float sensorValue  = 0.0f;
// static int   motorPosition = 0;  // example: curtain/blind position

// ============================================================
// OPTIONAL: BUTTON CALLBACKS
// Called by WebPortal when user presses a button.
// index = 0..3, state = true if toggle ON or momentary press
// ============================================================
void onButton0(uint8_t index, bool state)
{
    deviceActive = state;
    // add your logic here — e.g. relay, motor, alarm
}

void onButton1(uint8_t index, bool state)
{
    // momentary action example
}

// ============================================================
// MANDATORY: CONFIG READ
// Fill WebPortalConfig from myIOT2 — called on GET /config
// ============================================================
bool onGetConfig(WebPortalConfig &out)
{
    strlcpy(out.deviceName, iot.getDeviceName(), sizeof(out.deviceName));
    strlcpy(out.timezone,   iot.getTimezone(),   sizeof(out.timezone));
    strlcpy(out.ssid,       iot.getSsid(),       sizeof(out.ssid));
    out.wifiPwdSet = strlen(iot.getWifiPwd()) > 0;
    strlcpy(out.mqttHost, iot.getMqttServer(), sizeof(out.mqttHost));
    out.mqttPort = 1883;
    strlcpy(out.mqttUser, iot.getMqttUser(), sizeof(out.mqttUser));
    out.mqttPwdSet = strlen(iot.getMqttPwd()) > 0;

    out.useSerial             = iot.useSerial;
    out.otaEnabled            = iot.isOtaEnabled();
    out.terminalEnabled       = iot.isTerminalEnabled();
    out.resetSafetyEnabled    = iot.getResetSafetyConfig();
    out.resetSafetyThreshold  = iot.getResetSafetyThreshold();
    out.ignoreBootMsg         = iot.ignore_boot_msg;
    out.useFlashP             = iot.useFlashP;
    out.noNetworkResetMinutes = iot.noNetwork_reset;

    if (iot.topics_pub[0])    strlcpy(out.topicPubAvail,    iot.topics_pub[0],    sizeof(out.topicPubAvail));
    if (iot.topics_pub[1])    strlcpy(out.topicPubState,    iot.topics_pub[1],    sizeof(out.topicPubState));
    if (iot.topics_sub[0])    strlcpy(out.topicSubCmd,      iot.topics_sub[0],    sizeof(out.topicSubCmd));
    if (iot.topics_gen_pub[0]) strlcpy(out.topicGenMessages, iot.topics_gen_pub[0], sizeof(out.topicGenMessages));
    if (iot.topics_gen_pub[1]) strlcpy(out.topicGenLog,      iot.topics_gen_pub[1], sizeof(out.topicGenLog));
    if (iot.topics_gen_pub[2]) strlcpy(out.topicGenDebug,    iot.topics_gen_pub[2], sizeof(out.topicGenDebug));
    out.topicsReady = iot.topicsReady();

    // Extra pub topics (topics_pub[2..6])
    for (uint8_t i = 0; i < 5; i++)
        if (iot.topics_pub[2 + i]) strlcpy(out.extraPub[i], iot.topics_pub[2 + i], sizeof(out.extraPub[i]));

    // Extra sub topics (topics_sub[1..5])
    for (uint8_t i = 0; i < 5; i++)
        if (iot.topics_sub[1 + i]) strlcpy(out.extraSub[i], iot.topics_sub[1 + i], sizeof(out.extraSub[i]));

    return true;
}

// ============================================================
// MANDATORY: CONFIG WRITE
// Apply validated config from portal — called on POST /config
// ============================================================
bool onSetConfig(const WebPortalConfigUpdate &in, char *errMsg, size_t errLen)
{
    bool ok = iot.setDeviceName(in.deviceName);
    ok &= iot.setTimezone(in.timezone);
    ok &= iot.setSsid(in.ssid);
    if (strlen(in.wifiPwd) > 0) ok &= iot.setWifiPwd(in.wifiPwd);
    ok &= iot.setMqttServer(in.mqttHost);
    ok &= iot.setMqttUser(in.mqttUser);
    if (strlen(in.mqttPwd) > 0) ok &= iot.setMqttPwd(in.mqttPwd);
    if (!ok) { strlcpy(errMsg, "one or more fields rejected", errLen); return false; }

    iot.useSerial = in.useSerial;
    iot.setOtaEnabled(in.otaEnabled);
    iot.setTerminalEnabled(in.terminalEnabled);
    iot.setResetSafetyConfig(in.resetSafetyEnabled);
    iot.setResetSafetyThreshold(in.resetSafetyThreshold);
    iot.ignore_boot_msg = in.ignoreBootMsg;
    iot.useFlashP       = in.useFlashP;
    iot.noNetwork_reset = in.noNetworkResetMinutes;

    if (!iot.persistConfig())
        { strlcpy(errMsg, "failed to persist to flash", errLen); return false; }

    if (!iot.persistTopics(in.topicPubAvail, in.topicPubState, in.topicSubCmd,
                            in.topicGenMessages, in.topicGenLog, in.topicGenDebug,
                            in.extraPub[0], in.extraPub[1], in.extraPub[2], in.extraPub[3], in.extraPub[4],
                            in.extraSub[0], in.extraSub[1], in.extraSub[2], in.extraSub[3], in.extraSub[4]))
        { strlcpy(errMsg, "topics rejected or failed to persist", errLen); return false; }

    return true;
}

// ============================================================
// MANDATORY: STATUS
// Fill live status for portal — called every few seconds
// ============================================================
void onGetStatus(WebPortalStatus &out)
{
    out.wifiConnected = iot.isWifiConnected();
    out.mqttConnected = iot.isMqttConnected();
    out.ntpSynced     = iot.now() > 1700000000;
    out.otaActive     = iot.isOtaEnabled();

#if defined(ESP8266)
    snprintf(out.deviceId, sizeof(out.deviceId), "ESP8266_%06X", (unsigned)(ESP.getChipId() & 0xFFFFFF));
    strlcpy(out.espType, "ESP8266", sizeof(out.espType));
#elif defined(ESP32)
    snprintf(out.deviceId, sizeof(out.deviceId), "ESP32_%06X", (unsigned)(ESP.getEfuseMac() & 0xFFFFFF));
    strlcpy(out.espType, "ESP32", sizeof(out.espType));
#endif

    if (iot.topics_sub[0]) strlcpy(out.primaryTopic, iot.topics_sub[0], sizeof(out.primaryTopic));
    out.ignoreBootMsg         = iot.ignore_boot_msg;
    out.useFlashP             = iot.useFlashP;
    out.noNetworkResetMinutes = iot.noNetwork_reset;
    out.resetSafetyEnabled    = iot.isResetSafetyEnabled();
    out.resetSafetyCounter    = iot.getBootCounter();
    out.resetSafetyBootWasNormal = iot.getResult_rstStf();

    // ~~~ Device Data rows — shown in portal Status tab ~~~
    strlcpy(out.customLabel[0], "Active", sizeof(out.customLabel[0]));
    strlcpy(out.customValue[0], deviceActive ? "Yes" : "No", sizeof(out.customValue[0]));

    char buf[16];
    snprintf(buf, sizeof(buf), "%.1f", sensorValue);
    strlcpy(out.customLabel[1], "Sensor", sizeof(out.customLabel[1]));
    strlcpy(out.customValue[1], buf, sizeof(out.customValue[1]));

    // slots 2 and 3 left empty — won't render
}

// ============================================================
// OPTIONAL: TERMINAL
// Returns captured serial log for the portal terminal tab.
// Only active when terminalEnabled = true in Behavior settings.
// ============================================================
String onGetLog() { return SerialCapture::getBuffer(); }

// ============================================================
// OPTIONAL: REBOOT BUTTON
// Called when user presses Reboot in the portal.
// ============================================================
void onResetRequest()  { iot.sendReset("Web portal"); }
bool onDeleteConfig()  { return iot.deleteConfig(); }   // wipes credentials, caller reboots
bool onDeleteTopics()  { return iot.deleteTopics(); }   // wipes topic table
void onStartAP()       { iot.startAP(); }               // switches to AP mode for reconfiguration

// ============================================================
// YOUR MQTT CALLBACK
// Receives incoming MQTT messages
// ============================================================
void mqttCallback(char *msg, char *topic)
{
    // parse msg / topic and act on them
    // iot.inline_read(msg) — splits msg into iot.inline_param[]
}

// ============================================================
void setup()
{
    iot.start_services(mqttCallback);

    // ~~~ OPTIONAL: Portal buttons ~~~
    // setButton(index, label, isToggle, callback)
    // Leave callback nullptr to grey out that slot.
    portal.setButton(0, "Activate", true,  onButton0);  // toggle
    portal.setButton(1, "Trigger",  false, onButton1);  // momentary
    // portal.setButton(2, "...",   false, onButton2);
    // portal.setButton(3, "...",   false, onButton3);

    portal.begin(onGetConfig, onSetConfig, onGetStatus, onGetLog, onResetRequest,
                 onDeleteConfig, onDeleteTopics, onStartAP);

    // ~~~ OPTIONAL: Use portal WITHOUT maintenance callbacks ~~~
    // portal.begin(onGetConfig, onSetConfig, onGetStatus, onGetLog, onResetRequest);
}

void loop()
{
    iot.looper();
    portal.handle();
}

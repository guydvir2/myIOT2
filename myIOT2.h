// Updated: 2026-08-05
#ifndef myIOT2_h
#define myIOT2_h

// myIOT2 v3 — ESP8266/ESP32 IoT base platform
// Handles WiFi, MQTT, NTP, OTA and flash persistence.
// Log output goes to Serial, plus an optional user-supplied Print sink.

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <TZ.h>
#elif defined(ESP32)
#include <WiFi.h>
#include <ESPmDNS.h>
#define TZ_Asia_Jerusalem PSTR("IST-2IDT,M3.4.4/26,M10.5.0")
#endif

#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <myJflash.h>
#include "secretsIOT.h"

#define MS2MINUTES 60000

inline Print *iotLogSink = nullptr;

// Force our PRNT/PRNTL to win over myJflash's version.
#undef PRNT
#undef PRNTL

// Defined here in myIOT2.h so the macro applies inside myIOT2.cpp itself —
// that is what makes the library's own boot/WiFi/MQTT output reach the sink.
// Defining it in a consumer's header cannot work: a macro only affects
// translation units that include that header, and myIOT2.cpp does not.
#define PRNT(a)  do { if (useSerial) Serial.print(a);   if (iotLogSink) iotLogSink->print(a);   } while (0)
#define PRNTL(a) do { if (useSerial) Serial.println(a); if (iotLogSink) iotLogSink->println(a); } while (0)

class myIOT2
{
public:
    WiFiClient espClient;
    PubSubClient mqttClient;
    typedef void (*cb_func)(char *msg1, char *_topic);

protected:
    char ver[12] = "iot_v3.1.0";

public:
    // Topic arrays — [0] is always the primary slot; extras fill upward.
    const char *topics_pub[7]{};      // pub[0]=Avail, pub[1]=State, pub[2..6]=extra
    const char *topics_sub[6]{};      // sub[0]=Cmd, sub[1..5]=extra
    const char *topics_gen_pub[3]{};  // gen[0]=Messages, gen[1]=Log, gen[2]=Debug

    // ~~~ Runtime flags ~~~
    bool useSerial = true;
    bool ignore_boot_msg = false;
    uint8_t noNetwork_reset = 4;  // minutes before resetting if no network

    uint8_t num_p = 0;
    static const uint8_t num_param = 4;
    const int mqtt_len = 300;
    char inline_param[num_param][20];

    // ~~~ Status ~~~
    inline bool isWifiConnected() const { return _wifiConnected; }
    inline bool isMqttConnected() const { return _mqttConnected; }

    // ~~~ Network credential getters — returns live pointer, treat as read-only ~~~
    inline const char *getSsid()       const { return _ssid; }
    inline const char *getWifiPwd()    const { return _wifi_pwd; }
    inline const char *getMqttServer() const { return _mqtt_server; }
    inline uint16_t getMqttPort() const { return _mqttPort; }
    inline const char *getMqttUser()   const { return _mqtt_user; }
    inline const char *getMqttPwd()    const { return _mqtt_pwd; }
    inline const char *getDeviceName() const { return _deviceName; }
    inline const char *getTimezone()   const { return _timezone; }

    // Setters return false if value exceeds buffer capacity.
    bool setSsid(const char *value);
    bool setWifiPwd(const char *value);
    bool setMqttServer(const char *value);
    bool setMqttPort(uint16_t value);   // 1-65535; rejects 0
    bool setMqttUser(const char *value);
    bool setMqttPwd(const char *value);
    bool setDeviceName(const char *value);
    bool setTimezone(const char *value);  // POSIX TZ string — malformed string silently keeps UTC

    bool persistConfig();
    bool loadPersistedNetworkConfig();
    bool deleteConfig();   // wipe /netconfig.JSON — reverts to compiled-in defaults on next boot
    bool deleteTopics();   // wipe /topics.JSON — MQTT stays idle until topics reconfigured
    void startAP();        // switch to AP mode — portal accessible at 192.168.4.1

    // ~~~ OTA ~~~
    // Off by default. When enabled, accepts OTA within a time window after boot.
    inline bool isOtaEnabled() const        { return _otaEnabled; }
    inline void setOtaEnabled(bool value)   { _otaEnabled = value; }

    // ~~~ Reset safety ~~~
    // Detects rapid reboot loops and halts. Threshold = max reboots before halting.
    inline bool    isResetSafetyEnabled()          const { return _use_rstSft; }
    inline uint8_t getBootCounter()                const { return bootcounter; }
    inline bool    getResetSafetyConfig()          const { return _resetSafetyConfig; }
    inline uint8_t getResetSafetyThreshold()       const { return _resetSafetyThreshold; }
    inline void    setResetSafetyConfig(bool value)      { _resetSafetyConfig = value; }
    bool setResetSafetyThreshold(uint8_t value);  // 1-20; rejects 0

    // ~~~ Topic persistence ~~~
    // pubAvail + subCmd are required; all others may be "".
    // topicsReady() = false means MQTT stays idle until topics are configured.
    inline bool topicsReady() const { return _topicsReady; }

    // Library version string, e.g. "iot_v3.0.2". Exposed so the app (or the
    // web portal, via the app) can display it without touching `ver` directly.
    inline const char *getVersion() const { return ver; }
    bool persistTopics(const char *pubAvail, const char *pubState, const char *subCmd,
                       const char *genMessages, const char *genLog, const char *genDebug,
                       const char *extraPub1="", const char *extraPub2="", const char *extraPub3="",
                       const char *extraPub4="", const char *extraPub5="",
                       const char *extraSub1="", const char *extraSub2="", const char *extraSub3="",
                       const char *extraSub4="", const char *extraSub5="");
    bool loadPersistedTopics();
    void clearTopics();    // free all registered topics and reset counters (RAM only)

    // Call after registering topics by hand (add_pubTopic/add_subTopic/...).
    // Validates the required slots, marks topics ready, and starts MQTT.
    // Without this, hand-registered topics sit in RAM and MQTT never runs.
    bool commitTopics();

private:
    char _ssid[32];
    char _wifi_pwd[64];
    char _mqtt_pwd[64];
    char _mqtt_user[32];
    char _mqtt_server[40];
    uint16_t _mqttPort = 1883;
    char _deviceName[32]{};
    char _timezone[48] = "IST-2IDT,M3.4.4/26,M10.5.0";

    uint8_t _sub_topic_counter = 0;
    uint8_t _pub_topic_counter = 0;
    uint8_t _gen_topic_counter = 0;

    cb_func ext_mqtt;

    const uint8_t OTA_upload_interval = 10;  // minutes
    unsigned long allowOTA_clock = 0;
    char _telemetryTopic[40]{};
    unsigned long _telemetryTimer = 0;
    time_t _bootTime = 0;

    bool _wifiConnected = false;
    bool _connectingToWifi = false;
    const uint8_t _retryConnectWiFi = 60;
    unsigned long _lastWifiConnectiomAttemptMillis = 0;
    unsigned long _nextWifiConnectionAttemptMillis = 500;

    bool _mqttConnected = false;
    unsigned int _connectionEstablishedCount = 0;
    unsigned int _failedMQTTConnectionAttemptCount = 0;
    unsigned long _nextMqttConnectionAttemptMillis = 0;

    bool _firstRun = true;
    bool _everConnected = false;  // true after first successful MQTT connection
    uint8_t _countCriteria;
    uint8_t bootcounter = 0;
    bool _rstSft_OK = false;
    bool _rstSft_Final = false;
    bool _use_rstSft = false;

    bool _otaEnabled = false;
    bool _resetSafetyConfig = false;
    uint8_t _resetSafetyThreshold = 3;
    bool _topicsReady = false;

    // Single place that owns raw-LittleFS deletion: mounts, verifies, removes.
    // Treats "file already absent" as success so callers get a truthful result.
    bool _deleteFlashFile(const char *fname);

public:
    myIOT2();
    void looper();
    void start_services(cb_func funct, const char *ssid = SSID_ID, const char *password = PASS_WIFI,
                        const char *mqtt_user = MQTT_USER, const char *mqtt_passw = MQTT_PASS,
                        const char *mqtt_broker = MQTT_SERVER1);

    // ~~~ MQTT publish ~~~
    void notifyOnline();
    void pub_msg(const char *inmsg);    // publish to gen/Messages topic
    void pub_log(const char *inmsg);    // publish to gen/Log topic
    void pub_debug(const char *inmsg);  // publish to gen/Debug topic
    void pub_state(const char *inmsg, uint8_t i = 0);  // publish to topics_pub[i]
    void pub_noTopic(const char *inmsg, const char *Topic, bool retain = false);  // publish to arbitrary topic
    void sendReset(const char *header = nullptr);

    // ~~~ Topic registration (called internally by loadPersistedTopics) ~~~
    void add_subTopic(const char *topic);
    void add_subTopic(const char *topic[], uint8_t n);
    void add_pubTopic(const char *topic);
    void add_pubTopic(const char *topic[], uint8_t n);
    void add_gen_pubTopic(const char *topic);
    void add_gen_pubTopic(const char *topic[], uint8_t n);

    // ~~~ Reset safety ~~~
    void strtClk_rstSft(uint8_t n = 3);
    bool getResult_rstStf();
    void loop_rstSft(uint8_t time_criteria = 15);
    void _write_rstSft(uint8_t value, const char *key = "counter", const char *fname = "/bootcounter.JSON");
    void _failure_rstSft();
    uint8_t _read_rstSft(const char *key = "counter", const char *fname = "/bootcounter.JSON");

    // ~~~ Time ~~~
    time_t now();
    void get_timeStamp(char ret[], time_t t = 0);
    void convert_epoch2clock(long t1, long t2, char time_str[], char days_str[] = nullptr);

    // ~~~ Flash parameters ~~~
    uint8_t inline_read(char *inputstr);
    void clear_inline_read();
    bool readJson_inFlash(JsonDocument &DOC, const char *filename);

private:
    void _publishTelemetry();
    void _startWifi(const char *ssid, const char *password);
    bool _startNTP(const char *ntpServer = "time.nist.gov", const char *ntpServer2 = "il.pool.ntp.org");
    bool _NTP_updated();
    bool _WiFi_handler();
    void _onWifiConnect();
    void _onWifiDisconnect();
    void _setMQTT();
    void _subMQTT();
    bool _connectMQTT();
    bool _MQTT_handler();
    void _concate(const char *array[], char outmsg[]);
    void _MQTTcb(char *topic, uint8_t *payload, unsigned int length);
    void _pub_generic(const char *topic, const char *inmsg, bool retain = false, char *devname = nullptr, bool bare = false);
    void _pub_succ_connectivity();
    void _startOTA();
    void _acceptOTA();
    void _endRun_notofications();
};
#endif
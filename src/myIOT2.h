#ifndef myIOT2_h
#define myIOT2_h

#if defined(ESP8266)
#include <ESP8266WiFi.h>
// #include <ESP8266mDNS.h> // OTA libraries
// #include <TZ.h>

// #elif defined(ESP32)
// #include <WiFi.h>
// #include <ESPmDNS.h> // OTA libraries
// #define TZ_Asia_Jerusalem PSTR("IST-2IDT,M3.4.4/26,M10.5.0")
#endif

// #include <WiFiUdp.h>      // OTA
// #include <ArduinoOTA.h>   // OTA
// #include <PubSubClient.h> // MQTT
// #include <ArduinoJson.h>
// #include <myJflash.h>
// #include "secretsIOT.h"

#define MS2MINUTES 60000
#ifndef PRNT
#define PRNT(a)    \
    if (useSerial) \
    Serial.print(a)
#endif
#ifndef PRNTL
#define PRNTL(a)   \
    if (useSerial) \
    Serial.println(a)
#endif

class myIOT2
{
// public:
//     WiFiClient espClient;
//     PubSubClient mqttClient;

//     typedef void (*cb_func)(char *msg1, char *_topic);

// protected:
//     char ver[12] = "iot_v2.06";

// public:
//     const char *topics_pub[4]{};
//     const char *topics_sub[20]{};
//     const char *topics_gen_pub[4]{};
//     const char *parameter_filenames[4]{};

//     // ~~~~~~ Services ~~~~~~~~~
//     bool useSerial = true;
//     bool useFlashP = false;
//     bool ignore_boot_msg = false;
//     uint8_t noNetwork_reset = 4; // minutes
//     // ~~~~~~~ end Services ~~~~~~~

//     uint8_t num_p = 0;                  // number parameters got in MQTT message
//     static const uint8_t num_param = 4; // max MQTT parameters
//     char inline_param[num_param][20];   // values from user

//     inline bool isWifiConnected() const { return _wifiConnected; };
//     inline bool isMqttConnected() const { return _mqttConnected; };

// private:
//     char _ssid[15];
//     char _wifi_pwd[15];
//     char _mqtt_pwd[15];
//     char _mqtt_user[15];
//     char _mqtt_server[20];

//     cb_func ext_mqtt;

//     // time interval parameters
//     const uint8_t OTA_upload_interval = 10; // minutes to try OTA
//     unsigned long allowOTA_clock = 0;       // clock

//     bool _wifiConnected = false;
//     bool _connectingToWifi = false;
//     const uint8_t _retryConnectWiFi = 60; // seconds between fail Wifi reconnect reties
//     unsigned long _lastWifiConnectiomAttemptMillis = 0;
//     unsigned long _nextWifiConnectionAttemptMillis = 500;

//     bool _mqttConnected = false;
//     unsigned int _connectionEstablishedCount = 0; // Incremented before each _connectionEstablishedCallback call
//     unsigned int _failedMQTTConnectionAttemptCount = 0;
//     unsigned long _nextMqttConnectionAttemptMillis = 0;

//     // holds status
//     bool _firstRun = true;
//     bool _OTAloaded = false;

// public: /* Functions */
//     myIOT2();
//     void looper();
//     void start_services(cb_func funct, const char *ssid = SSID_ID, const char *password = PASS_WIFI,
//                         const char *mqtt_user = MQTT_USER, const char *mqtt_passw = MQTT_PASS, const char *mqtt_broker = MQTT_SERVER1);

//     // ~~~~~~~ MQTT ~~~~~~~
//     void notifyOnline();
//     void pub_msg(const char *inmsg);
//     void pub_log(const char *inmsg);
//     void pub_debug(const char *inmsg);
//     void sendReset(const char *header = nullptr);
//     void pub_state(const char *inmsg, uint8_t i = 0);
//     void pub_noTopic(const char *inmsg, char *Topic, bool retain = false);

//     // ~~~~~~~ Clk ~~~~~~~
//     time_t now();
//     void get_timeStamp(char ret[], time_t t = 0);
//     void convert_epoch2clock(long t1, long t2, char time_str[], char days_str[] = nullptr);

//     // ~~~~~~~ Param ~~~~~~~
//     uint8_t inline_read(char *inputstr);
//     void clear_inline_read();
//     void set_pFilenames(const char *fileArray[], uint8_t asize);
//     bool readFlashParameters(JsonDocument &DOC, const char *filename);
//     bool readJson_inFlash(JsonDocument &DOC, const char *filename);

// private:
//     // ~~~~~~~WIFI ~~~~~~~
//     void _startWifi(const char *ssid, const char *password);
//     bool _startNTP(const char *ntpServer = "time.nist.gov", const char *ntpServer2 = "il.pool.ntp.org");
//     bool _NTP_updated();
//     bool _WiFi_handler();
//     void _onWifiConnect();
//     void _onWifiDisconnect();

//     // ~~~~~~~ MQTT ~~~~~~~
//     void _setMQTT();
//     void _subMQTT();
//     bool _connectMQTT();
//     bool _MQTT_handler();
//     void _concate(const char *array[], char outmsg[]);
//     void _MQTTcb(char *topic, uint8_t *payload, unsigned int length);
//     void _pub_generic(const char *topic, const char *inmsg, bool retain = false, char *devname = nullptr, bool bare = false);

//     // ~~~~~~~ OTA  ~~~~~~~
//     void _startOTA();
//     void _acceptOTA();

//     // ~~~~~~~ Param ~~~~~~~
//     uint8_t _getdataType(const char *y);
//     bool _cmdline_flashUpdate(const char *key, const char *new_value);
//     bool _change_flashP_value(const char *key, const char *new_value, JsonDocument &DOC);
};
#endif

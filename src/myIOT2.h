#ifndef myIOT2_h
#define myIOT2_h

// #include <ESP8266WiFi.h>
// #include <ESP8266mDNS.h>
// #include <WiFiUdp.h>
// #include <ArduinoOTA.h>



#include <Arduino.h>
#include <WiFiUdp.h>      // OTA
#include <ArduinoOTA.h>   // OTA
#include <Ticker.h>       //WDT
#include <PubSubClient.h> // MQTT
#include <ArduinoJson.h>
#include "secretsIOT8266.h"
#include <myLOG.h>
#include <Chrono.h>

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h> // OTA libraries
#include <TZ.h>
#define LITFS LittleFS

#elif defined(ESP32)
#include <WiFiUdp.h>      // OTA
#include <ArduinoOTA.h>   // OTA
#include <WiFi.h>
#include <ESPmDNS.h> // OTA libraries
#include <ESP32Ping.h>
#define LITFS LITTLEFS
#define TZ_Asia_Jerusalem PSTR("IST-2IDT,M3.4.4/26,M10.5.0")

// #elif defined(ARDUINO_ARCH_SAMD)
// #include <SPI.h>
// #include <WiFiNINA.h>
// #define TZ_Asia_Jerusalem PSTR("IST-2IDT,M3.4.4/26,M10.5.0")
#endif

class myIOT2
{
#define MY_IOT_JSON_SIZE 500
#define SKETCH_JSON_SIZE 1250
#define MY_IOT_TOPIC_JSON 600
#define MS2MINUTES 60000

#define PRNT(a)    \
    if (useSerial) \
    Serial.print(a)
#define PRNTL(a)   \
    if (useSerial) \
    Serial.println(a)

public:
    WiFiClient espClient;
    PubSubClient mqttClient;
    flashLOG flog;   /* Stores Activity LOG */
    flashLOG clklog; /* Stores Boot clock records */

#if defined(ESP8266)
    Ticker wdt;
#endif

    // ~~~~define generic cb function~~~~
    typedef void (*cb_func)(char *msg1, char *_topic);

public:
    char ver[12] = "iot_v1.60c";
    char myIOT_topics[22] = "/myIOT2_topics.json";
    char myIOT_paramfile[20] = "/myIOT_param.json";
    char sketch_paramfile[20] = "/sketch_param.json";

    /*Variables */
    // ~~~~~~ Services ~~~~~~~~~
    bool useWDT = true;
    bool useOTA = true;
    bool useDebug = false;
    bool useSerial = true;
    bool useFlashP = false;
    bool useResetKeeper = false;
    bool useNetworkReset = true; // allow reset due to no-network timeout
    bool useBootClockLog = false;
    bool ignore_boot_msg = false;
    uint8_t debug_level = 0;      // 0- All, 1- system states; 2- log only
    uint8_t noNetwork_reset = 10; // minutes
    // ~~~~~~~ end Services ~~~~~~~

    uint8_t num_p = 0; // number parameters got in MQTT message
    uint8_t mqtt_detect_reset = 2;

    static const uint8_t num_param = 4; // MQTT parameter count
    char inline_param[num_param][20];   // values from user

    // MQTT Topic variables
    // ~~~~~~~~~~~~~~~~~~~~~~
    StaticJsonDocument<MY_IOT_TOPIC_JSON> TOPICS_JSON;

private:
    // WiFi MQTT broker parameters
    char _ssid[15];
    char _wifi_pwd[15];
    char _mqtt_pwd[15];
    char _mqtt_user[15];
    char _mqtt_server[20];

    cb_func ext_mqtt;
    Chrono _WifiConnCheck;
    Chrono _MQTTConnCheck;
    Chrono _NTPCheck;

    // time interval parameters
    const uint8_t WIFItimeOut = 20;         // sec try to connect WiFi
    const uint8_t retryConnectWiFi = 60;    // seconds between fail Wifi reconnect reties
    const uint8_t OTA_upload_interval = 10; // minute to try OTA
    const uint8_t wdtMaxRetries = 20;       // seconds to bITE
    unsigned long allowOTA_clock = 0;       // clock
    volatile uint8_t wdtResetCounter = 0;

    // holds status
    bool firstRun = true;
    bool _Wifi_and_mqtt_OK = false;

public: /* Functions */
    myIOT2();
    void start_services(cb_func funct, const char *ssid = SSID_ID, const char *password = PASS_WIFI, const char *mqtt_user = MQTT_USER, const char *mqtt_passw = MQTT_PASS, const char *mqtt_broker = MQTT_SERVER1, int log_ents = 100);
    void looper();
    void startOTA();

    bool pingSite(char *externalSite = "www.google.com", uint8_t pings = 3);

    // ~~~~~~~ MQTT ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    void sendReset(char *header);
    void notifyOnline();
    void pub_state(char *inmsg, uint8_t i = 0);
    void pub_msg(char *inmsg);
    void pub_noTopic(char *inmsg, char *Topic, bool retain = false);
    void pub_log(char *inmsg);
    void pub_debug(char *inmsg);
    // void pub_sms(char *inmsg, char *name = nullptr);
    // void pub_email(String &inmsg, char *name = nullptr);

    // ~~~~~~~~~~~~~~ Clk ~~~~~~~~~~~~~~~~~~~~~
    time_t now();
    void get_timeStamp(char ret[], time_t t = 0);
    void convert_epoch2clock(long t1, long t2, char *time_str, char *days_str = nullptr);

    // ~~~~~~~~~~~~~~ Param ~~~~~~~~~~~~~~~~~~~~~
    uint8_t inline_read(char *inputstr);
    bool extract_JSON_from_flash(char *filename, JsonDocument &DOC);
    void get_flashParameters();
    void update_vars_flash_parameters(JsonDocument &DOC);
    String readFile(char *fileName);

private:
    // ~~~~~~~~~~~~~~WIFI ~~~~~~~~~~~~~~~~~~~~~
    bool _startWifi(const char *ssid, const char *password);
    void _shutdown_wifi();
    bool _network_looper();
    bool _start_network_services();
    bool _startNTP(const char *ntpServer = "pool.ntp.org", const char *ntpServer2 = "il.pool.ntp.org");
    bool _NTP_updated();
    bool _try_rgain_wifi();

    // ~~~~~~~ MQTT ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    bool _startMQTT();
    void _constructTopics_fromCode();
    bool _subMQTT();
    void _MQTTcb(char *topic, uint8_t *payload, unsigned int length);
    void _getBootReason_resetKeeper(char *msg);
    void _write_log(char *inmsg, uint8_t x, const char *topic = "_deviceName");
    void _pub_generic(const char *topic, char *inmsg, bool retain = false, char *devname = nullptr, bool bare = false);
    bool _try_regain_MQTT();

    // ~~~~~~~ Services  ~~~~~~~~~~~~~~~~~~~~~~~~
    void _startFS();
    void _startWDT();
    void _acceptOTA();
    void _store_bootclockLOG();
    void _post_boot_check();
    void _feedTheDog();
    void _extract_log(flashLOG &LOG, const char *title, bool _isTimelog = false);

    // ~~~~~~~~~~~~~~ Param ~~~~~~~~~~~~~~~~~~~~~
    uint8_t _getdataType(const char *y);
    bool _cmdline_flashUpdate(const char *key, const char *new_value);
    bool _change_flashP_value(const char *key, const char *new_value, JsonDocument &DOC);
    bool _saveFile(char *filename, JsonDocument &DOC);
};
// void watchdog_timer_triggered_helper(myIOT2 *watchdog)
// {
//     watchdog->_feedTheDog();
// }
#endif

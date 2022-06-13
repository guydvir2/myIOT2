#ifndef myIOT2_h
#define myIOT2_h

#include <Arduino.h>
#include <Ticker.h>       //WDT
#include <PubSubClient.h> // MQTT
#include <WiFiUdp.h>      // OTA
#include <ArduinoOTA.h>   // OTA
#include <ArduinoJson.h>
#include "secretsIOT8266.h"
#include <myLOG.h>

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h> // OTA libraries
#include <TZ.h>
#define LITFS LittleFS

#elif defined(ESP32)
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
#define MAX_NUM_TOPICS 5
#define MS2MINUTES 60000UL
#define MY_IOT_JSON_SIZE 624
#define SKETCH_JSON_SIZE 1250

public:
    WiFiClient espClient;
    PubSubClient mqttClient;
    flashLOG flog;   /* Stores Activity LOG */
    flashLOG clklog; /* Stores Boot clock records */
#if isESP8266
    Ticker wdt;
#endif

    // ~~~~define generic cb function~~~~
    typedef void (*cb_func)(char *msg1, char *_topic);

public:
    char ver[12] = "iot_v1.60a";
    char myIOT_paramfile[20] = "/myIOT_param.json";
    char sketch_paramfile[20] = "/sketch_param.json";

    /*Variables */
    // ~~~~~~ Services ~~~~~~~~~
    bool useWDT = true;
    bool useOTA = true;
    bool useDebug = false;
    bool useSerial = false;
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
    const char *sub_topics[MAX_NUM_TOPICS] = {};
    const char *pub_topics[MAX_NUM_TOPICS] = {};
    const char *sub_data_topics[MAX_NUM_TOPICS] = {};

private:
    // WiFi MQTT broker parameters
    char _ssid[15];
    char _wifi_pwd[15];
    char _mqtt_pwd[15];
    char _mqtt_user[15];
    char _mqtt_server[20];

    cb_func ext_mqtt;

    // time interval parameters
    const uint8_t WIFItimeOut = 20;         // sec try to connect WiFi
    const uint8_t retryConnectWiFi = 1;     // minutes between fail Wifi reconnect reties
    const uint8_t OTA_upload_interval = 10; // minute to try OTA
    const uint8_t wdtMaxRetries = 20;       // seconds to bITE
    unsigned long noNetwork_Clock = 0;      // clock
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
    // void pub_sms(String &inmsg, char *name = nullptr);
    // void pub_sms(char *inmsg, char *name = nullptr);
    // void pub_sms(JsonDocument &sms);
    // void pub_email(String &inmsg, char *name = nullptr);
    // void pub_email(JsonDocument &email);
    void clear_ExtTopicbuff();

    // ~~~~~~~~~~~~~~ Clk ~~~~~~~~~~~~~~~~~~~~~
    time_t now();
    void return_date(char ret_tuple[20]);
    void get_timeStamp(char ret[], time_t t = 0);
    void return_clock(char ret_tuple[20]);
    void convert_epoch2clock(long t1, long t2, char *time_str, char *days_str);

    // ~~~~~~~~~~~~~~ Param ~~~~~~~~~~~~~~~~~~~~~
    uint8_t inline_read(char *inputstr);
    bool read_fPars(char *filename, JsonDocument &DOC, char defs[]);
    void update_fPars();
    String readFile(char *fileName);
    uint8_t _getdataType(const char *y);

private:
    // ~~~~~~~~~~~~~~WIFI ~~~~~~~~~~~~~~~~~~~~~
    bool _startWifi(const char *ssid, const char *password);
    void _shutdown_wifi();
    bool _network_looper();
    bool _start_network_services();
    bool _startNTP(const char *ntpServer = "pool.ntp.org", const char *ntpServer2 = "il.pool.ntp.org");
    bool _NTP_updated();

    // ~~~~~~~ MQTT ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    bool _startMQTT();
    void _subArray(const char *arr[], uint8_t n);
    bool _subMQTT();
    void _MQTTcb(char *topic, uint8_t *payload, unsigned int length);
    void _getBootReason_resetKeeper(char *msg);
    void _write_log(char *inmsg, uint8_t x, const char *topic = "_deviceName");
    void _pub_generic(const char *topic, char *inmsg, bool retain = false, char *devname = nullptr, bool bare = false);
    const char *_devName(char ret[]);
    const char *_availName(char ret[]);

    // ~~~~~~~ Services  ~~~~~~~~~~~~~~~~~~~~~~~~
    void _startWDT();
    void _acceptOTA();
    void _update_bootclockLOG();
    void _post_boot_check();
    void _feedTheDog();
    void _startFS();
};
// void watchdog_timer_triggered_helper(myIOT2 *watchdog)
// {
//     watchdog->_feedTheDog();
// }
#endif

#ifndef myIOT2_h
#define myIOT2_h

#if defined(ARDUINO_ARCH_ESP8266)
#define isESP8266 true
#define isESP32 false
#elif defined(ESP32)
#define isESP32 true
#define isESP8266 false
#else
#error Architecture unrecognized by this code.
#endif

#include <Ticker.h> //WDT
#include <PubSubClient.h> // MQTT
#include <WiFiUdp.h>    // OTA
#include <ArduinoOTA.h> // OTA
#include "secretsIOT8266.h"
#include <myLOG.h>
#include <myJSON.h>

#if isESP8266
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h> // OTA libraries
#include <TZ.h>
#elif isESP32
#include <WiFi.h>
#include <ESPmDNS.h>    // OTA libraries
#include <ESP32Ping.h>
#define TZ_Asia_Jerusalem PSTR("IST-2IDT,M3.4.4/26,M10.5.0")
#endif

// ~~~~define generic cb function~~~~
typedef void (*cb_func)(char msg1[50]);
struct MQTT_msg
{
    char from_topic[40];
    char msg[200];
    char device_topic[40];
};

class myIOT2
{
#define MS2MINUTES 60000UL
public:
    WiFiClient espClient;
    PubSubClient mqttClient;
#if isESP8266
    Ticker wdt;
#endif
    flashLOG flog;
    flashLOG clklog;

public:
    const char *ver = "iot_v1.4";
    char *myIOT_paramfile = "/myIOT_param.json";

    /*Variables */
    // ~~~~~~ Services ~~~~~~~~~
    bool useWDT = true;
    bool useOTA = true;
    bool useDebug = false;
    bool useSerial = false;
    bool useextTopic = false;
    bool useResetKeeper = false;
    bool useNetworkReset = true; // allow reset due to no-network timeout
    bool useBootClockLog = false;
    bool ignore_boot_msg = false;
    bool useAltermqttServer = false;

    uint8_t debug_level = 0;      // 0- All, 1- system states; 2- log only
    uint8_t noNetwork_reset = 30; // minutes

    uint8_t mqtt_detect_reset = 2;
    static const uint8_t _size_extTopic = 2;
    static const uint8_t bootlog_len = 10;                     // nubmer of boot clock records
    static const uint8_t num_param = 4;                        // MQTT parameter count
    static const uint8_t MaxTopicLength = 15;                  // topics
    static const uint8_t MaxTopicLength2 = 3 * MaxTopicLength; // topics
    char inline_param[num_param][20];                          // values from user

    // MQTT Topic variables
    char *prefixTopic = (char *)malloc(MaxTopicLength);
    char *deviceTopic = (char *)malloc(MaxTopicLength);
    char *addGroupTopic = (char *)malloc(MaxTopicLength);
    char *extTopic[_size_extTopic];
    bool extTopic_newmsg_flag = false;

    MQTT_msg *extTopic_msgArray[1] = {nullptr};

private:
    // WiFi MQTT broker parameters
    char *_ssid = (char *)malloc(MaxTopicLength);
    char *_wifi_pwd = (char *)malloc(MaxTopicLength);
    char *_mqtt_server = (char *)malloc(MaxTopicLength);
    char *_mqtt_server2 = MQTT_SERVER2;
    char *_mqtt_user = (char *)malloc(MaxTopicLength);
    char *_mqtt_pwd = (char *)malloc(MaxTopicLength);
    cb_func ext_mqtt;

    // time interval parameters
    const uint8_t WIFItimeOut = 20;         // 30 sec try to connect WiFi
    const uint8_t retryConnectWiFi = 1;     // minutes between fail Wifi reconnect reties
    const uint8_t OTA_upload_interval = 10; // 10 minute to try OTA

    uint8_t time2Reset_noNetwork = noNetwork_reset; // minutues pass without any network
    volatile uint8_t wdtResetCounter = 0;
    const uint8_t wdtMaxRetries = 60;  // seconds to bITE
    unsigned long noNetwork_Clock = 0; // clock
    unsigned long allowOTA_clock = 0;  // clock

    // holds informamtion
    bool firstRun = true;
    bool _Wifi_and_mqtt_OK = false;

public: /* Functions */
    myIOT2();
    void start_services(cb_func funct, char *ssid = SSID_ID, char *password = PASS_WIFI, char *mqtt_user = MQTT_USER, char *mqtt_passw = MQTT_PASS, char *mqtt_broker = MQTT_SERVER1, int log_ents = 50, int log_len = 250);
    void looper();
    void startOTA();
    void return_clock(char ret_tuple[20]);
    void return_date(char ret_tuple[20]);
    static bool checkInternet(char *externalSite = "www.google.com", uint8_t pings = 3);

    void sendReset(char *header);
    void notifyOnline();
    void pub_state(char *inmsg, uint8_t i = 0);
    void pub_msg(char *inmsg);
    void pub_noTopic(char *inmsg, char *Topic, bool retain = false);
    void pub_log(char *inmsg);
    void pub_ext(char *inmsg, char *name = "", bool retain = false, uint8_t i = 0);
    void pub_debug(char *inmsg);
    void pub_sms(String &inmsg, char *name = "");
    void pub_sms(char *inmsg, char *name = "");
    void pub_sms(JsonDocument &sms);
    void pub_email(String &inmsg, char *name = "");
    void pub_email(JsonDocument &email);
    void clear_ExtTopicbuff();
    long get_bootclockLOG(int x);
    char *get_timeStamp(time_t t = 0);
    void convert_epoch2clock(long t1, long t2, char *time_str, char *days_str);
    time_t now();

    uint8_t inline_read(char *inputstr);
    bool read_fPars(char *filename, String &defs, JsonDocument &DOC, int JSIZE = 500);
    char *export_fPars(char *filename, JsonDocument &DOC, int JSIZE = 500);

private:
    // ~~~~~~~~~~~~~~WIFI ~~~~~~~~~~~~~~~~~~~~~
    bool _startWifi(char *ssid, char *password);
    bool _network_looper();
    bool _start_network_services();
    bool _startNTP(const char *ntpServer = "pool.ntp.org");
    bool _NTP_updated();

    // ~~~~~~~ MQTT ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    bool _startMQTT();
    void _selectMQTTbroker();
    bool _subscribeMQTT();
    void _MQTTcb(char *topic, uint8_t *payload, unsigned int length);
    void _getBootReason_resetKeeper(char *msg);
    void _write_log(char *inmsg, uint8_t x, char *topic = "_deviceName");
    void _pub_generic(char *topic, char *inmsg, bool retain = false, char *devname = "", bool bare = false);
    char *_devName();
    char *_availName();

    // ~~~~~~~ Services  ~~~~~~~~~~~~~~~~~~~~~~~~
    void _startWDT();
    void _acceptOTA();
    void _update_bootclockLOG();
    void _post_boot_check();
    void _feedTheDog();

    // ~~~~~~~ EEPROM  ~~~~~~~~~~~~~~~~~~~~~~~~
    // void start_EEPROM_eADR();
    // void EEPROMWritelong(int address, long value);
    // long EEPROMReadlong(long address);
};
// void watchdog_timer_triggered_helper(myIOT2 *watchdog)
// {
//     watchdog->_feedTheDog();
// }
#endif

#ifndef myIOT2_h
#define myIOT2_h

#include <Arduino.h>
#include "secretsIOT8266.h"
#include <Ticker.h> //WDT
#include <TimeLib.h>
#include <PubSubClient.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <myLOG.h>
#include <myJSON.h>
#include <ArduinoJson.h>
#include <WiFiManager.h>

#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#include <NtpClientLib.h>
#include <ESP8266mDNS.h> // OTA libraries

#define isESP8266 true
#define isESP32 false

#elif defined(ESP32)
#include <WiFi.h>
#include <ESPmDNS.h>
#include <time.h>
#include <WiFi.h>
#include <ESP32Ping.h>

#define isESP32 true
#define isESP8266 false

#else
#error Architecture unrecognized by this code.
#endif

// ~~~~define generic cb function~~~~
typedef void (*cb_func)(char msg1[50]);

class myIOT2
{
public:
    WiFiClient espClient;
    PubSubClient mqttClient;
    WiFiManager wm;
#if isESP8266
    Ticker wdt;
#endif
    flashLOG flog;
    flashLOG clklog;

    myIOT2();
    void start_services(cb_func funct, char *ssid = SSID_ID, char *password = PASS_WIFI, char *mqtt_user = MQTT_USER, char *mqtt_passw = MQTT_PASS, char *mqtt_broker = MQTT_SERVER1, int log_ents = 50, int log_len = 250);
    void looper();
    void startOTA();
    void return_clock(char ret_tuple[20]);
    void return_date(char ret_tuple[20]);
    bool checkInternet(char *externalSite = "www.google.com", uint8_t pings = 1);

    void sendReset(char *header);
    void notifyOnline();
    void pub_state(char *inmsg, uint8_t i = 0);
    void pub_msg(char *inmsg);
    void pub_noTopic(char *inmsg, char *Topic);
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
    void convert_epoch2clock(long t1, long t2, char *time_str, char *days_str);

    int inline_read(char *inputstr);
    void feedTheDog();
    bool read_fPars(char *filename, String &defs, JsonDocument &DOC, int JSIZE = 500);
    char *export_fPars(char *filename, JsonDocument &DOC, int JSIZE = 500);

    // ~~~~~~ Services ~~~~~~~~~
    bool useSerial = false;
    bool useWDT = true;
    bool useOTA = true;
    bool extDefine = false; // must to set to true in order to use EXtMQTT
    bool useResetKeeper = false;
    bool resetFailNTP = false; // not needed any more
    bool useextTopic = false;
    bool useNetworkReset = true; // allow reset due to no-network timeout
    bool useAltermqttServer = false;
    bool useDebug = false;
    bool useBootClockLog = false;
    uint8_t debug_level = 0;               // 0- All, 1- system states; 2- log only
    static const uint8_t bootlog_len = 10; // nubmer of boot clock records
    // ~~~~~~~~~~~~~~~~~~~~~~~~~

    const char *ver = "iot_v1.0";
    static const uint8_t num_param = 6;
    char inline_param[num_param][20]; //values from user

    struct MQTT_msg
    {
        char from_topic[50];
        char msg[500];
        char device_topic[50];
    };
    MQTT_msg extTopic_msg;


    uint8_t mqtt_detect_reset = 2;
    uint8_t noNetwork_reset = 30; // minutes

    static const int MaxTopicLength = 64; //topics
    char prefixTopic[MaxTopicLength];
    char deviceTopic[MaxTopicLength];
    char addGroupTopic[MaxTopicLength];
    char *extTopic[2] = {nullptr, nullptr};

    char mqqt_ext_buffer[3][150];
    int max_mqtt_msg = 200;

    char timeStamp[20];
    char *myIOT_paramfile = "/myIOT_param.json";

private:
    char *_ssid;
    char *_wifi_pwd;
    cb_func ext_mqtt;

    // time interval parameters
    const unsigned int clockUpdateInt = 60 * 60;               // seconds to update NTP
    const unsigned int WIFItimeOut = (1000 * 60) * 1 / 4;      // 30 sec try to connect WiFi
    const unsigned int OTA_upload_interval = (1000 * 60) * 10; // 10 minute to try OTA
    const unsigned long retryConnectWiFi = (1000 * 60) * 1;    // 1 minuter between fail Wifi reconnect reties

    unsigned long time2Reset_noNetwork = (1000 * 60UL) * noNetwork_reset; // minutues pass without any network
    volatile uint8_t wdtResetCounter = 0;
    const uint8_t wdtMaxRetries = 60;  //seconds to bITE
    unsigned long noNetwork_Clock = 0; // clock
    unsigned long allowOTA_clock = 0;  // clock
    // ############################

    //MQTT broker parameters
    char *_mqtt_server;
    char *_mqtt_server2 = MQTT_SERVER2;
    char *_mqtt_user = "";
    char *_mqtt_pwd = "";

    const int _maxMQTTmsg = 180;
    const int _maxMQTTheader = 70;

    // ######################################

    // MQTT topics
    char _msgTopic[MaxTopicLength];
    char _groupTopic[MaxTopicLength];
    char _logTopic[MaxTopicLength];
    char _deviceName[MaxTopicLength];
    char _availTopic[MaxTopicLength];
    char _stateTopic[MaxTopicLength];
    char _stateTopic2[MaxTopicLength];
    char _signalTopic[MaxTopicLength];
    char _debugTopic[MaxTopicLength];
    char _smsTopic[MaxTopicLength];
    char _emailTopic[MaxTopicLength];

    char *topicArry[4] = {_deviceName, _groupTopic, _availTopic, addGroupTopic};
    // ##############################################

    // holds informamtion
    char bootTime[50];
    bool firstRun = true;

    // ~~~~~~~~~~~~~~WIFI ~~~~~~~~~~~~~~~~~~~~~
    bool startWifi(char *ssid, char *password);
    void start_clock();
    bool network_looper();
    void start_network_services();

    void _getTimestamp(char ret_timeStamp[25], time_t t = 0);
    void startNTP(const int gmtOffset_sec = 2 * 3600, const int daylightOffset_sec = 3600, const char *ntpServer = "pool.ntp.org");

    // ~~~~~~~ MQTT ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    void startMQTT();
    bool subscribeMQTT();
    void createTopics();
    void callback(char *topic, uint8_t *payload, unsigned int length);
    void msgSplitter(const char *msg_in, int max_msgSize, char *prefix, char *split_msg);
    void firstRun_ResetKeeper(char *msg);
    void write_log(char *inmsg, int x, char *topic = "_deviceName");
    void _pub_generic(char *topic, char *inmsg, bool retain = false, char *devname = "", bool bare = false);

    // ~~~~~~~ Services  ~~~~~~~~~~~~~~~~~~~~~~~~
    void startWDT();
    void acceptOTA();
    void update_bootclockLOG();
    void _post_boot_check();

    // ~~~~~~~ EEPROM  ~~~~~~~~~~~~~~~~~~~~~~~~
    // void start_EEPROM_eADR();
    // void EEPROMWritelong(int address, long value);
    // long EEPROMReadlong(long address);
};
// void watchdog_timer_triggered_helper(myIOT2 *watchdog)
// {
//     watchdog->feedTheDog();
// }
#endif

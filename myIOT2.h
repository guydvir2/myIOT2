#ifndef myIOT2_h
#define myIOT2_h

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h> // OTA libraries
#include <TZ.h>

#elif defined(ESP32)
#include <WiFi.h>
#include <ESPmDNS.h> // OTA libraries
#define TZ_Asia_Jerusalem PSTR("IST-2IDT,M3.4.4/26,M10.5.0")
#endif

#include <WiFiUdp.h>      // OTA
#include <ArduinoOTA.h>   // OTA
#include <PubSubClient.h> // MQTT
#include <ArduinoJson.h>
#include "secretsIOT.h"

#include <FS.h>
#include <LittleFS.h>
#define LITFS LittleFS

class myIOT2
{
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

public:
    WiFiClient espClient;
    PubSubClient mqttClient;

    // ~~~~define generic cb function~~~~
    typedef void (*cb_func)(char *msg1, char *_topic);

protected:
    char ver[12] = "iot_v1.95";

public:
    const char *topics_pub[4] = {nullptr, nullptr, nullptr, nullptr};
    const char *parameter_filenames[4] = {nullptr, nullptr, nullptr, nullptr};
    const char *topics_gen_pub[4] = {nullptr, nullptr, nullptr, nullptr};
    const char *topics_sub[20] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};

    /*Variables */
    // ~~~~~~ Services ~~~~~~~~~
    bool useSerial = true;
    bool useFlashP = false;
    bool useNetworkReset = true; // allow reset due to no-network timeout
    bool useBootClockLog = false;
    bool ignore_boot_msg = false;
    uint8_t noNetwork_reset = 4; // minutes
    // ~~~~~~~ end Services ~~~~~~~

    uint8_t num_p = 0; // number parameters got in MQTT message
    uint8_t mqtt_detect_reset = 2;

    static const uint8_t num_param = 4; // MQTT parameter count
    char inline_param[num_param][20];   // values from user

private:
    // WiFi MQTT broker parameters
    char _ssid[15];
    char _wifi_pwd[15];
    char _mqtt_pwd[15];
    char _mqtt_user[15];
    char _mqtt_server[20];

    cb_func ext_mqtt;

    // time interval parameters
    const uint8_t WIFItimeOut = 30;         // sec try to connect WiFi
    const uint8_t retryConnectWiFi = 60;    // seconds between fail Wifi reconnect reties
    const uint8_t OTA_upload_interval = 10; // minutes to try OTA
    const uint8_t wdtMaxRetries = 45;       // seconds to bITE
    static const int _maxMQTTmsglen = 300;
    unsigned long allowOTA_clock = 0;   // clock
    unsigned long _nonetwork_clock = 0; // clock
    unsigned int _nextRetry = 0;
    uint8_t _wifi_counter = 0;
    uint8_t _mqtt_counter = 0;
    unsigned int _accum_wifi_not_connected = 0;
    unsigned int _accum_mqtt_not_connected = 0;

    // holds status
    bool firstRun = true;
    bool _Wifi_and_mqtt_OK = false;

public: /* Functions */
    myIOT2();
    void looper();
    void startOTA();
    void start_services(cb_func funct, const char *ssid = SSID_ID, const char *password = PASS_WIFI, const char *mqtt_user = MQTT_USER, const char *mqtt_passw = MQTT_PASS, const char *mqtt_broker = MQTT_SERVER1, int log_ents = 100);

    // ~~~~~~~ MQTT ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    void notifyOnline();
    void pub_msg(const char *inmsg);
    void pub_log(const char *inmsg);
    void pub_debug(const char *inmsg);
    void sendReset(const char *header = nullptr);
    void pub_state(const char *inmsg, uint8_t i = 0);
    void pub_noTopic(const char *inmsg, char *Topic, bool retain = false);

    // ~~~~~~~~~~~~~~ Clk ~~~~~~~~~~~~~~~~~~~~~
    time_t now();
    void get_timeStamp(char ret[], time_t t = 0);
    void convert_epoch2clock(long t1, long t2, char time_str[], char days_str[] = nullptr);
    bool _timePassed(unsigned int T);

    // ~~~~~~~~~~~~~~ Param ~~~~~~~~~~~~~~~~~~~~~
    uint8_t inline_read(char *inputstr);
    void set_pFilenames(const char *fileArray[], uint8_t asize);

    bool extract_JSON_from_flash(const char *filename, JsonDocument &DOC);
    void update_vars_flash_parameters(JsonDocument &DOC);
    String readFile(const char *fileName);

private:
    // ~~~~~~~~~~~~~~WIFI ~~~~~~~~~~~~~~~~~~~~~
    void _shutdown_wifi();
    bool _startWifi(const char *ssid, const char *password);
    bool _network_looper();
    bool _start_network_services();
    bool _startNTP(const char *ntpServer = "pool.ntp.org", const char *ntpServer2 = "il.pool.ntp.org");
    bool _NTP_updated();
    bool _try_rgain_wifi();

    // ~~~~~~~ MQTT ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    bool _subMQTT();
    bool _startMQTT();
    bool _try_regain_MQTT();
    void _getBootReason_resetKeeper(char *msg);
    void _MQTTcb(char *topic, uint8_t *payload, unsigned int length);
    void _pub_generic(const char *topic, const char *inmsg, bool retain = false, char *devname = nullptr, bool bare = false);

    // ~~~~~~~ Services  ~~~~~~~~~~~~~~~~~~~~~~~~
    void _startFS();
    void _acceptOTA();
    void _feedTheDog();
    void _store_bootclockLOG();

    // ~~~~~~~~~~~~~~ Param ~~~~~~~~~~~~~~~~~~~~~
    uint8_t _getdataType(const char *y);
    bool _cmdline_flashUpdate(const char *key, const char *new_value);
    bool _change_flashP_value(const char *key, const char *new_value, JsonDocument &DOC);
    bool _saveFile(const char *filename, JsonDocument &DOC);
};
#endif

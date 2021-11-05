#include <myIOT2.h>

myIOT2 iot;

#define DEV_TOPIC "empty"
#define GROUP_TOPIC "none"
#define PREFIX_TOPIC "myHome"

void addiotnalMQTT(char *incoming_msg)
{
    char msg[150];
    char msg2[20];
    if (strcmp(incoming_msg, "status") == 0)
    {
        sprintf(msg, "BOOOOO");
        iot.pub_msg(msg);
    }
    else if (strcmp(incoming_msg, "ver2") == 0)
    {
        // sprintf(msg, "ver #2: [%s], lib: [%s], boardType[%s]", "espVer", VER, boardType);
        // iot.pub_msg(msg);
    }
    else if (strcmp(incoming_msg, "help2") == 0)
    {
        sprintf(msg, "Help2: Commands #2 - [; m; ,x]");
        iot.pub_msg(msg);
    }
}
void startIOTservices()
{
#if USE_SIMPLE_IOT == 1
    iot.useSerial = true;
    iot.useWDT = true;
    iot.useOTA = true;
    iot.useResetKeeper = true;
    iot.useextTopic = false;
    iot.useDebug = true;
    iot.debug_level = 0;
    iot.useNetworkReset = true;
    iot.noNetwork_reset = 10;
    iot.useBootClockLog = true;
    iot.useAltermqttServer = false;
    iot.ignore_boot_msg = false;
    iot.deviceTopic = DEV_TOPIC;
    iot.prefixTopic = PREFIX_TOPIC;
    iot.addGroupTopic= GROUP_TOPIC;



    // strcpy(iot.addGroupTopic, GROUP_TOPIC);
    // strcpy(iot.deviceTopic, DEV_TOPIC);
    // strcpy(iot.prefixTopic, PREFIX_TOPIC);

#elif USE_SIMPLE_IOT == 0

    iot.useSerial = paramJSON["useSerial"];
    iot.useWDT = paramJSON["useWDT"];
    iot.useOTA = paramJSON["useOTA"];
    iot.useResetKeeper = paramJSON["useResetKeeper"];
    iot.useDebug = paramJSON["useDebugLog"];
    iot.debug_level = paramJSON["debug_level"];
    iot.useNetworkReset = paramJSON["useNetworkReset"];
    iot.noNetwork_reset = paramJSON["noNetwork_reset"];
    iot.useextTopic = paramJSON["useextTopic"];
    iot.useBootClockLog = paramJSON["useBootClockLog"];
    strcpy(iot.deviceTopic, paramJSON["deviceTopic"]);
    strcpy(iot.prefixTopic, paramJSON["prefixTopic"]);
    strcpy(iot.addGroupTopic, paramJSON["groupTopic"]);
#endif

    // char a[50];
    // sprintf(a, "%s/%s/%s/%s", iot.prefixTopic, iot.addGroupTopic, iot.deviceTopic, DEBUG_TOPIC);
    // strcpy(iot.extTopic, a);

    iot.start_services(addiotnalMQTT);
    // iot.start_services(addiotnalMQTT, "dvirz_iot", "GdSd13100301", MQTT_USER, MQTT_PASS, "192.168.2.100");
}
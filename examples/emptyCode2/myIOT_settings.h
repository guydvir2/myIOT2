#include <myIOT2.h>

myIOT2 iot;

#define DEV_TOPIC "empty3"
#define GROUP_TOPIC ""
#define PREFIX_TOPIC "myHome"

MQTT_msg extTopic_msg; /* ExtTopic*/

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
    else if (strcmp(incoming_msg, "disco") == 0)
    {
        sprintf(msg, "Disconnect sent to broker");
        iot.pub_msg(msg);
        iot.mqttClient.disconnect();
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
    iot.useDebug = false;
    iot.debug_level = 0;
    iot.useNetworkReset = true;
    iot.noNetwork_reset = 10;
    iot.useBootClockLog = false;
    iot.useAltermqttServer = false;
    iot.ignore_boot_msg = false;
    strcpy(iot.deviceTopic,DEV_TOPIC);
    strcpy(iot.prefixTopic,PREFIX_TOPIC);
    strcpy(iot.addGroupTopic,GROUP_TOPIC);

    iot.extTopic_msgArray[0] = &extTopic_msg;
    iot.extTopic[0] = "myHome/new";

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
    // iot.start_services(addiotnalMQTT, "dvirz_iot", "GdSd13100301", "guy", "kupelu9e", "192.168.2.100");
    iot.start_services(addiotnalMQTT);
}

void extTopic_looper()
{
    if (iot.extTopic_newmsg_flag)
    {
        Serial.println(iot.extTopic_msgArray[0]->msg);
        iot.clear_ExtTopicbuff();
    }
}
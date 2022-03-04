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
    else if (strcmp(incoming_msg, "help2") == 0)
    {
        sprintf(msg, "help #2:No other functions");
        iot.pub_msg(msg);
    }
    else if (strcmp(incoming_msg, "ver2") == 0)
    {
        sprintf(msg, "ver #2:");
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
    iot.ignore_boot_msg = false;
    strcpy(iot.deviceTopic, DEV_TOPIC);
    strcpy(iot.prefixTopic, PREFIX_TOPIC);
    strcpy(iot.addGroupTopic, GROUP_TOPIC);

//     iot.extTopic_msgArray[0] = &extTopic_msg;
//     iot.extTopic[0] = "myHome/new";
#else
    iot.useFlashP = true;
#endif
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
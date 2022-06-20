// #include "defTopics.h"


// // Subscribe Topics
// #define TOPIC_SUB_FULLPATH PREFIX_TOPIC GROUP_TOPIC DEV_TOPIC // Full Path
// #define TOPIC_SUB_ALL PREFIX_TOPIC "All"
// #define TOPIC_SUB_AVAIL TOPIC_SUB_FULLPATH "/Avail" // on-line avail , retained
// #define TOPIC_SUB_STATE TOPIC_SUB_FULLPATH "/State" // last CMD, retained
// #define TOPIC_SUB_GROUP_0 PREFIX_TOPIC GROUP_TOPIC
// // ~~~~~~~~~~~~~~

// // sub_data_topics
// #define TOPIC_SUB_DATA_0 TOPIC_SUB_FULLPATH "/data_1" // data, retained
// #define TOPIC_SUB_DATA_1 TOPIC_SUB_FULLPATH "/data_2" // data, retained
// #define TOPIC_SUB_DATA_2 TOPIC_SUB_FULLPATH "/data_3" // data, retained
// ~~~~~~~~~~~~~~~~~

myIOT2 iot;

void addiotnalMQTT(char *incoming_msg, char *_topic)
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
    else if (strcmp(incoming_msg, "tt") == 0)
    {
        sprintf(msg, "topic[%s]; msg[%s]", _topic, incoming_msg);
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
    iot.useDebug = true;
    iot.debug_level = 0;
    iot.useNetworkReset = true;
    iot.noNetwork_reset = 2;
    iot.useBootClockLog = true;
    iot.ignore_boot_msg = false;

    // iot.sub_topics[0] = TOPIC_SUB_FULLPATH;
    // iot.sub_topics[1] = TOPIC_SUB_ALL;
    // iot.sub_topics[2] = TOPIC_SUB_AVAIL;
    // iot.sub_topics[3] = TOPIC_SUB_STATE;
    // iot.sub_topics[4] = TOPIC_SUB_GROUP_0;

    // iot.sub_data_topics[0] = TOPIC_SUB_DATA_0;
    // iot.sub_data_topics[1] = TOPIC_SUB_DATA_1;
    // iot.sub_data_topics[2] = TOPIC_SUB_DATA_2;

#else
    iot.useFlashP = true;
    iot.useSerial = true;
#endif
    // iot.pub_topics[0] = TOPIC_PUB_MSG;
    // iot.pub_topics[1] = TOPIC_PUB_LOG;
    // iot.pub_topics[2] = TOPIC_PUB_DBG;
    // iot.pub_topics[3] = TOPIC_PUB_SMS;
    // iot.pub_topics[4] = TOPIC_PUB_EML;

    iot.start_services(addiotnalMQTT);
}
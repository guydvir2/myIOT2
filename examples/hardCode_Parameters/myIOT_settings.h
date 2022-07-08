myIOT2 iot;
const char *topicmsg = "myHome/Messages";
const char *topicLog = "myHome/log";
const char *topicDebug = "myHome/debug";
const char *topicClient = "myHome/test/Client";
const char *topicClient_avail = "myHome/test/Client/Avail";
const char *topicClient_state = "myHome/test/Client/State";

const char *topicSub1 = "myHome/alarmMonitor";

void addiotnalMQTT(char *incoming_msg, char *_topic)
{
    char msg[150];
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
    iot.useWDT = true;
    iot.useOTA = true;
    iot.useSerial = true;
    iot.useResetKeeper = false;
    iot.useDebug = true;
    iot.debug_level = 0;
    iot.useFlashP = false;
    iot.useNetworkReset = true;
    iot.noNetwork_reset = 2;
    iot.useBootClockLog = true;
    iot.ignore_boot_msg = false;

    // iot.TOPICS_JSON["pub_gen_topics"][0] = "myHome/Messages";
    // iot.TOPICS_JSON["pub_gen_topics"][1] = "myHome/log";
    // iot.TOPICS_JSON["pub_gen_topics"][2] = "myHome/debug";

    // iot.TOPICS_JSON["pub_topics"][0] = "myHome/group/client/Avail";
    // iot.TOPICS_JSON["pub_topics"][1] = "myHome/group/client/State";

    // iot.TOPICS_JSON["sub_topics"][0] = "myHome/group/client";
    // iot.TOPICS_JSON["sub_topics"][1] = "myHome/All";

    // iot.TOPICS_JSON["sub_data_topics"][0] = "myHome/device1";
    // iot.TOPICS_JSON["sub_data_topics"][1] = "myHome/alarmMonitor";

    iot.topics_gen_pub[0] = topicmsg;
    iot.topics_gen_pub[1] = topicLog;
    iot.topics_gen_pub[2] = topicDebug;

    iot.topics_pub[0] = topicClient_avail;
    iot.topics_pub[1] = topicClient_state;

    iot.topics_sub[0] = topicClient;
    iot.topics_sub[1] = topicSub1;

    iot.start_services(addiotnalMQTT);
}

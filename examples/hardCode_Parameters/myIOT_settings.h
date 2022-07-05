myIOT2 iot;

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

    iot.TOPICS_JSON["pub_gen_topics"][0] = "myHome/Messages";
    iot.TOPICS_JSON["pub_gen_topics"][1] = "myHome/log";
    iot.TOPICS_JSON["pub_gen_topics"][2] = "myHome/debug";

    iot.TOPICS_JSON["pub_topics"][0] = "myHome/group/client/Avail";
    iot.TOPICS_JSON["pub_topics"][1] = "myHome/group/client/State";

    iot.TOPICS_JSON["sub_topics"][0] = "myHome/group/client";
    iot.TOPICS_JSON["sub_topics"][1] = "myHome/All";

    iot.TOPICS_JSON["sub_data_topics"][0] = "myHome/device1";
    iot.TOPICS_JSON["sub_data_topics"][1] = "myHome/alarmMonitor";

    iot.start_services(addiotnalMQTT);
}

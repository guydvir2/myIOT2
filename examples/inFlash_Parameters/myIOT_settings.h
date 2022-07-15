myIOT2 iot;

#define MAX_TOPIC_SIZE 45

char topics_sub[3][MAX_TOPIC_SIZE];
char topics_pub[3][MAX_TOPIC_SIZE];
char topics_gen_pub[3][MAX_TOPIC_SIZE];
char *parameterFiles[3] = {"/myIOT_param.json", "/myIOT2_topics.json", "/sketch_param.json"};

// // ±±±±±±± Genereal pub topic ±±±±±±±±±
// const char *topicLog = "myHome/log";
// const char *topicDebug = "myHome/debug";
// const char *topicmsg = "myHome/Messages";

// // ±±±±±±±±±±±± sub Topics ±±±±±±±±±±±±±±±±±±
// const char *topicAll = "myHome/All";
// const char *topicSub1 = "myHome/alarmMonitor";
// const char *topicClient = "myHome/test/Client";

// // ±±±±±±±±±±±±±±±± Client state pub topics ±±±±±±±±±±±±±±±±
// const char *topicClient_avail = "myHome/test/Client/Avail";
// const char *topicClient_state = "myHome/test/Client/State";


void updateTopicsFlash(JsonDocument &DOC, char ch_array[][MAX_TOPIC_SIZE], const char *dest_array[], const char *topic, const char *defaulttopic, uint8_t ar_size)
{
    for (uint8_t i = 0; i < ar_size; i++)
    {
        sprintf(ch_array[i], DOC[topic][i] | defaulttopic);
        dest_array[i] = ch_array[i];
    }
}
void update_Parameters_fromflash()
{
    StaticJsonDocument<1250> DOC;

    iot.set_pFilenames(parameterFiles, 3);

    iot.extract_JSON_from_flash(iot.parameter_filenames[0], DOC);
    iot.update_vars_flash_parameters(DOC);
    DOC.clear();

    iot.extract_JSON_from_flash(iot.parameter_filenames[1], DOC);
    updateTopicsFlash(DOC, topics_gen_pub, iot.topics_gen_pub, "pub_gen_topics", "myHome/Messages", sizeof(topics_gen_pub) / (sizeof(topics_gen_pub[0])));
    updateTopicsFlash(DOC, topics_pub, iot.topics_pub, "pub_topics", "myHome/Messages", sizeof(topics_pub) / (sizeof(topics_pub[0])));
    updateTopicsFlash(DOC, topics_sub, iot.topics_sub, "sub_topics", "myHome/Messages", sizeof(topics_sub) / (sizeof(topics_sub[0])));
    DOC.clear();

    iot.extract_JSON_from_flash(iot.parameter_filenames[2], DOC);
    DOC.clear();
}
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
    update_Parameters_fromflash();
    iot.start_services(addiotnalMQTT);
}

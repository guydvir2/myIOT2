myIOT2 iot;

char topics_sub[3][40];
char topics_pub[3][45];
char topics_gen_pub[3][18];
const char *parameterFiles[3] = {"/myIOT_param.json", "/myIOT2_topics.json", "/sketch_param.json"};

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

void updateTopicsFlash(JsonDocument &DOC)
{
    for (uint8_t i = 0; i < 3; i++)
    {
        sprintf(topics_gen_pub[i], DOC["pub_gen_topics"][i] | "myHome/Messages");
        iot.topics_gen_pub[i] = topics_gen_pub[i];
    }
    for (uint8_t i = 0; i < 3; i++)
    {
        sprintf(topics_pub[i], DOC["pub_topics"][i] | "myHome/Messages");
        iot.topics_pub[i] = topics_pub[i];
    }
    for (uint8_t i = 0; i < 3; i++)
    {
        sprintf(topics_sub[i], DOC["sub_topics"][i] | "myHome/group/device");
        iot.topics_sub[i] = topics_sub[i];
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
    updateTopicsFlash(DOC);
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

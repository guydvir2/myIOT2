myIOT2 iot;

#define MAX_TOPIC_SIZE 40 // <----- Verfy max Topic size

char topics_sub[3][MAX_TOPIC_SIZE];
char topics_pub[3][MAX_TOPIC_SIZE];
char topics_gen_pub[3][MAX_TOPIC_SIZE];
const char *parameterFiles[3] ={"/myIOT_param.json", "/myIOT2_topics.json", "/sketch_param.json"};

void updateTopics_flash(JsonDocument &DOC, char ch_array[][MAX_TOPIC_SIZE], const char *dest_array[], const char *topic, const char *defaulttopic, uint8_t ar_size)
{
    for (uint8_t i = 0; i < ar_size; i++)
    {
        strlcpy(ch_array[i], DOC[topic][i] | defaulttopic, MAX_TOPIC_SIZE);
        dest_array[i] = ch_array[i];
    }
}
void update_sketch_parameters_flash()
{
    /* Custom paramters for each sketch used IOT2*/
    yield();
}
void update_Parameters_flash()
{
    StaticJsonDocument<1250> DOC;

    /* Part A: update filenames of paramter files */
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    iot.set_pFilenames(parameterFiles, sizeof(parameterFiles) / sizeof(parameterFiles[0]));
    iot.readFlashParameters(DOC, iot.parameter_filenames[0]);
    DOC.clear();
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    /* Part B: Read Topics from flash, and update myIOT Topics */
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    if (iot.readJson_inFlash(DOC, iot.parameter_filenames[1]))
    {
        updateTopics_flash(DOC, topics_gen_pub, iot.topics_gen_pub, "pub_gen_topics", "myHome/Messages", sizeof(topics_gen_pub) / (sizeof(topics_gen_pub[0])));
        updateTopics_flash(DOC, topics_pub, iot.topics_pub, "pub_topics", "myHome/log", sizeof(topics_pub) / (sizeof(topics_pub[0])));
        updateTopics_flash(DOC, topics_sub, iot.topics_sub, "sub_topics", "myHome/log", sizeof(topics_sub) / (sizeof(topics_sub[0])));
    }
    else /* File not found */
    {
        iot.topics_gen_pub[0] = "myHome/Messages";
        iot.topics_gen_pub[1] = "myHome/log";
        iot.topics_pub[0] = "myHome/myDevice";
        iot.topics_pub[0] = "myHome/myDevice/Avail";
        iot.topics_pub[1] = "myHome/myDevice/State";
        iot.topics_sub[0] = "myHome/myDevice";
        iot.topics_sub[1] = "myHome/All";
    }
    DOC.clear();
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    /* Part C: Read Sketch paramters from flash, and update Sketch */
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    if (iot.readJson_inFlash(DOC, iot.parameter_filenames[2]))
    {
        update_sketch_parameters_flash();
    }
    else
    {
        yield();
    }
    DOC.clear();
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
    update_Parameters_flash();
    iot.start_services(addiotnalMQTT);
}

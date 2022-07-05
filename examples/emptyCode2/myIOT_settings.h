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
}
void startIOTservices()
{
    iot.useFlashP = true;
    iot.useSerial = true;
    iot.start_services(addiotnalMQTT);
}
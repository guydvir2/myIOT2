#include <myIOT2.h>
#include "myIOT_settings.h"

#define PINGER 1

void send_q(int interval = 1000)
{
        static unsigned long last_ping = 0;
        static int x=0;
        if (millis() - last_ping >= interval)
        {
                Serial.print("PING #");
                Serial.println(++x);
                last_ping = millis();
                iot.mqttClient.publish("myHome/test", "q");
                // iot.pub_noTopic("q", "myHome/test");
        }
}

void setup()
{
        startIOTservices();
}
void loop()
{
        if (PINGER == 1)
        {
                send_q(1500);
        }
        iot.looper();
        iot.mqttClient.loop();
}

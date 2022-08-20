#include <myIOT2.h>
#include "myIOT_settings.h"

char *path = "myHome/test/Client";
const char *words[2] = {"on", "off"};
const int _delay_ = 750;
const uint8_t numsw = 4;
int msgCounter = 0;

void publish_cmd(uint8_t i, uint8_t dir)
{
        char a[30];
        sprintf(a, "%d,%s", i, words[dir]);
        iot.pub_noTopic(a, path);
}
void setup()
{
        startIOTservices();
}
void loop()
{
        static uint8_t x = 0;
        for (uint8_t n = 0; n < numsw; n++)
        {
                publish_cmd(n, x);
                delay(_delay_);
                Serial.print("msgCounter: #");
                Serial.println(++msgCounter);
        }
        if (x == 0)
        {
                x++;
        }
        else
        {
                x--;
        }
        iot.looper();
}

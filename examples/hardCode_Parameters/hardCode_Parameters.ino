#include <myIOT2.h>
#include "myIOT_settings.h"

int first = 0;

void setup()
{
        Serial.begin(115200);
        startIOTservices();
        first = millis();
}
void loop()
{
        static unsigned long lastentry = 0;
        static unsigned long lastentry2 = 0;
        static int counter = 0;
        if (millis() - lastentry > 200)
        {
                Serial.print("@@@@@@@@@@@@@@@ --->");
                Serial.println(millis() - lastentry);
        }
        lastentry = millis();

        if (millis() - lastentry2 > 2000)
        {
                Serial.print("Total MSGS:#");
                Serial.print(++counter);
                Serial.print("\t AVG:");
                Serial.print((millis() - first) / counter);
                Serial.print("\t LST_TIME:");
                Serial.println((millis() - lastentry2));
                lastentry2 = millis();
                iot.pub_noTopic("1", "myHome/test/Client");
        }

        iot.looper();
}

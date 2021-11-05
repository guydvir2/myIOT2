#include <myIOT2.h>

#define USE_SIMPLE_IOT 1 // Not Using FlashParameters
#if USE_SIMPLE_IOT == 0
#include "empty_param.h"
#endif
#include "myIOT_settings.h"

void setup()
{
#if USE_SIMPLE_IOT == 1
        startIOTservices();
#elif USE_SIMPLE_IOT == 0
        startRead_parameters();
        startIOTservices();
        endRead_parameters();
#endif
        Serial.println("BOOT!!!!");
}

void loop()
{
        iot.looper();
        delay(100);
        if (iot.extTopic_newmsg_flag)
        {
                Serial.println(iot.extTopic_msgArray[0]->msg);
                iot.clear_ExtTopicbuff();
        }
}

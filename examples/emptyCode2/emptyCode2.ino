#include <myIOT2.h>

#define USE_SIMPLE_IOT 1 // Not Using FlashParameters
#if USE_SIMPLE_IOT == 1
#include "empty_param.h"
#endif
#include "myIOT_settings.h"

void setup()
{
#if USE_SIMPLE_IOT == 1
        startIOTservices();
#elif USE_SIMPLE_IOT == 0
        read_flashParameter();
        // Serial.begin(115200);
        startIOTservices();
#endif
}

void loop()
{
        iot.looper();
        // delay(100);
        // extTopic_looper();
}

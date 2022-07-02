#include <myIOT2.h>

#define USE_SIMPLE_IOT 1 // Not Using FlashParameters

#if USE_SIMPLE_IOT == 0
#include "empty_param.h"
#elif USE_SIMPLE_IOT == 1
#endif
#include "myIOT_settings.h"

void setup()
{
        Serial.begin(115200);
#if USE_SIMPLE_IOT == 1
        // serializeJsonPretty(iot.TOPICS_JSON, Serial); 
        startIOTservices();
#elif USE_SIMPLE_IOT == 0
        read_flashParameter();
        startIOTservices();
#endif
}
void loop()
{
        iot.looper();
        // delay(100);
}

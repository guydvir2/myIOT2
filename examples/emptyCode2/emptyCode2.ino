#include <myIOT2.h>
#include <ArduinoJson.h>

#define USE_SIMPLE_IOT 0 // Not Using FlashParameters
#if USE_SIMPLE_IOT == 0
#include "empty_param.h"
#endif
#include "myIOT_settings.h"

void setup()
{
        Serial.begin(115200);
#if USE_SIMPLE_IOT == 1
        startIOTservices();
#elif USE_SIMPLE_IOT == 0
        read_flashParameter();
       
        startIOTservices();
#endif
}
void loop()
{
        iot.looper();
}

#include <myIOT2.h>
#include <Arduino.h>

#define USE_SIMPLE_IOT 1 // Not Using FlashParameters

#if USE_SIMPLE_IOT == 0
#include "empty_param.h"
#endif
#include "myIOT_settings.h"

void show_services()
{
        static bool showed = false;
        if (!showed)
        {
                char msg[300];
//                sprintf(msg, "Service: useSerial[%d], useWDT[%d], useOTA[%d], useResetKeeper[%d], useextTopic[%d], resetFailNTP[%d], useDebug[%d], debug_level[%d], useNetworkReset[%d], noNetwork_reset[%d],useBootClockLog[%d]",
//                        iot.useSerial, iot.useWDT, iot.useOTA, iot.useResetKeeper, iot.useextTopic, iot.resetFailNTP, iot.useDebug, iot.debug_level, iot.useNetworkReset, iot.noNetwork_reset, iot.useBootClockLog);
//                Serial.println(msg);
                showed = true;
        }
}



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
        show_services();
        delay(100);
}

#include <myIOT2.h>

#define USE_SIMPLE_IOT 1 // Not Using FlashParameters
#if USE_SIMPLE_IOT == 0
#include "empty_param.h"
#endif
#include "myIOT_settings.h"
static uint8_t wifistatus = 0;

void setup()
{
#if USE_SIMPLE_IOT == 1
        // wifistatus = WL_DISCONNECTED;
        startIOTservices();
#elif USE_SIMPLE_IOT == 0
        startRead_parameters();
        startIOTservices();
        endRead_parameters();
#endif
}

void loop()
{
        iot.looper();
        delay(100);
        // extTopic_looper();
        // static uint8_t wifistatus = WL_DISCONNECTED;

        // if (WiFi.status() != wifistatus)
        // {
        //         wifistatus = WiFi.status();
        //         Serial.print("WiFi: ");
        //         Serial.println(wifistatus);
        // }
}
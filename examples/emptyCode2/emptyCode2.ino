#include <myIOT2.h>

#define USE_SIMPLE_IOT 0 // Not Using FlashParameters
#if USE_SIMPLE_IOT == 0
#include "empty_param.h"
#endif
#include "myIOT_settings.h"


void setup()
{
#if USE_SIMPLE_IOT == 1
        startIOTservices();
#elif USE_SIMPLE_IOT == 0
        StaticJsonDocument<JSON_SIZE_IOT> myIOT_flash_parameters;
        read_flashParameter(myIOT_flash_parameters);
        startIOTservices(myIOT_flash_parameters);
#endif
}

void loop()
{
        iot.looper();
        delay(100);
        // extTopic_looper();
}

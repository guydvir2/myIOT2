#include <Arduino.h>
#include <myIOT2.h>
#include "myIOT_settings.h"

#define hardCoded_parameters
#define inFlash_paramters

void setup()
{
        startIOTservices();
}
void loop()
{
        iot.looper();
}

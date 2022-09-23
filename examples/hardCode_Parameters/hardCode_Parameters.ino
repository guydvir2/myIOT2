#include <myIOT2.h>
#include "myIOT_settings.h"


void setup()
{
        startIOTservices();
}
void loop()
{
        iot.looper();
}

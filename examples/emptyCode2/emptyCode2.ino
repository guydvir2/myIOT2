#include <myIOT2.h>
#include "myIOT_settings.h"
#include "empty_param.h"

void setup()
{
        read_flashParameter();
        startIOTservices();
}
void loop()
{
        iot.looper();
}

#include <myIOT2.h>
// #include <Arduino.h>
// #include <myLOG.h>

#define USE_SIMPLE_IOT 1 // Not Using FlashParameters

#if USE_SIMPLE_IOT == 0
#include "empty_param.h"
#endif
#include "myIOT_settings.h"

// flashLOG clkLOG;


void run(){
        char a[20];
        sprintf(a,"%d",millis());
        int x=1234;
        // clkLOG.write(a, true);
        delay(100);
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
        // clkLOG.start(5, 20, true, true);
        for (int x=0;x<3;x++){
                run();
        }
        // char zz[60];
        // clkLOG.readline(3,zz);
        // Serial.println(zz);
        // clkLOG.del_last_record();
        // clkLOG.del_line(0);
        // clkLOG.rawPrintfile();

        // time_t T=iot.now();
        // unsigned long T = 1234567890;
        // _Print(T, false);
}

void loop()
{
        iot.looper();
        // clkLOG.looper(10);
        // static unsigned long lastW = 0;
        // if ( millis()-lastW > 15000)
        // {
        //         unsigned long t = iot.now();
        //         lastW = millis();
        //         iot.get_timeStamp();
        //         clkLOG.write(iot.timeStamp);
        // }

        delay(100);
}

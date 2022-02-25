#include <Arduino.h>
#include <ArduinoJson.h>
#include <myJSON.h>

#define JSON_SIZE_IOT 450
#define JSON_SIZE_SKETCH 1000

char *sketch_paramfile = "/sketch_param.json";

char paramA[20];
int paramB = 0;
extern myIOT2 iot;

void update_vars(JsonDocument &DOC)
{
  strcpy(paramA, DOC["paramA"]);
  paramB = DOC["paramB"];
}
void read_flashParameter(JsonDocument &DOC)
{
  StaticJsonDocument<JSON_SIZE_SKETCH> sketchJSON;

  String myIOT_defs = "{\"useSerial\":true,\"useWDT\":false,\"useOTA\":true,\"useResetKeeper\" : false,\"useBootClockLog\": true,\
                        \"useDebugLog\": true,\" useDebugLog\" : true,\"useNetworkReset\":true, \"deviceTopic\" : \"devTopic\",\
                        \"groupTopic\" : \"group\",\"prefixTopic\" : \"myHome\",\"debug_level\":0,\"noNetwork_reset\":1,\"ver\":0.3}";

  String sketch_defs = "{\"paramA\":\"BBB\",\"paramB\":5555}";

  bool a = iot.read_fPars(sketch_paramfile, sketch_defs, sketchJSON);
  bool b = iot.read_fPars(iot.myIOT_paramfile, myIOT_defs, DOC);
  bool readfile_ok = a && b;

  // serializeJsonPretty(sketchJSON, Serial);
  // serializeJsonPretty(DOC, Serial);
  // Serial.flush();

#if USE_SIMPLE_IOT == 0
  update_vars(sketchJSON);
#endif
  if (!readfile_ok)
  {
    iot.pub_log("Error read Parameters from file. Defaults values loaded.");
  }
}
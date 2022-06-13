#include <Arduino.h>
#include <myJSON.h>

#define JSON_SIZE_SKETCH 256

char *sketch_paramfile = "/sketch_param.json";

char paramA[20];
int paramB = 0;
extern myIOT2 iot;

void update_vars(JsonDocument &DOC)
{
  strcpy(paramA, DOC["paramA"]);
  paramB = DOC["paramB"];
}
void read_flashParameter()
{
  StaticJsonDocument<JSON_SIZE_SKETCH> sketchJSON;
  char sketch_defs[] = "{\"paramA\":\"BBB\",\"paramB\":5555}";
  bool a = iot.read_fPars(sketch_paramfile, sketchJSON, sketch_defs);
  // serializeJsonPretty(DOC, Serial);
  // Serial.flush();
  update_vars(sketchJSON);

  if (!a)
  {
    iot.pub_log("Error read Parameters from file. Defaults values loaded.");
  }
}
#include <Arduino.h>

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

  char sketch_defs[] = "{\
                          \"paramA\":\"BBB\",\
                          \"paramB\":5555\
                        }";

  if (!iot.extract_JSON_from_flash(sketch_paramfile, sketchJSON))
  {
    deserializeJson(sketchJSON, sketch_defs);
    iot.pub_log("Error read Parameters from file. Defaults values loaded.");
  }
  // serializeJsonPretty(sketchJSON, Serial);
  // Serial.flush();
  update_vars(sketchJSON);
}
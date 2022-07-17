#include <Arduino.h>
#define JSON_SIZE_SKETCH 256

char *sketch_paramfile = "/sketch_param.json";

char paramA[20];
int paramB = 0;

void update_vars(JsonDocument &DOC)
{
  strlcpy(paramA, DOC["paramA"] | "KAKA", 20);
  paramB = DOC["paramB"] | 0;
}
void read_flashParameter()
{
  StaticJsonDocument<JSON_SIZE_SKETCH> sketchJSON;

  if (!iot.extract_JSON_from_flash(sketch_paramfile, sketchJSON))
  {
    sketchJSON["paramA"] = "sdfgsdfg";
    sketchJSON["paramB"] = 123456;
  }
  // serializeJsonPretty(sketchJSON, Serial);
  // Serial.flush();
  update_vars(sketchJSON);
}
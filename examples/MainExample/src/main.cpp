#include <Arduino.h>
#include <myIOT2.h>

#define FLASH_DEF 0

myIOT2 iot;
void extMQTT(char *incoming_msg, char *_topic)
{
  char msg[270];
  if (strcmp(incoming_msg, "status") == 0)
  {
    sprintf(msg, "[Status]: OK");
    iot.pub_msg(msg);
  }
}

void read_Topics_flash(JsonDocument &DOC, const char *filename)
{
  bool a = iot.readJson_inFlash(DOC, filename);
  bool b = DOC.containsKey("gen_pubTopic") && DOC.containsKey("subTopic") && DOC.containsKey("pubTopics");

  if (!a && !b)
  {
    const char *topics = "{ \"gen_pubTopic\":[\"DvirHome/Messages\",\"DvirHome/log\",\"DvirHome/debug\"],\
                            \"subTopic\":[\"DvirHome/Device\",\"DvirHome/All\"],\
                            \"pubTopics\":[\"DvirHome/Device/Avail\",\"DvirHome/Device/State\"]}";
    deserializeJson(DOC, topics);
  }
}
void def_iot_fromFlash()
{
  StaticJsonDocument<800> DOC;
  const char *a[] = {"/iot_config.json", "/iot_topics.json"};

  iot.set_pFilenames(a, 2);
  iot.readFlashParameters(DOC, iot.parameter_filenames[0]); // iot services
  read_Topics_flash(DOC, iot.parameter_filenames[1]);

  for (uint8_t t = 0; t < DOC["gen_pubTopic"].size(); t++)
  {
    iot.add_gen_pubTopic(DOC["gen_pubTopic"][t]);
  }
  for (uint8_t t = 0; t < DOC["subTopic"].size(); t++)
  {
    iot.add_subTopic(DOC["subTopic"][t]);
  }
  for (uint8_t t = 0; t < DOC["pubTopics"].size(); t++)
  {
    iot.add_pubTopic(DOC["pubTopics"][t]);
  }
}
void def_iot_fromCode()
{
  const char *t[] = {"DvirHome/Messages", "DvirHome/log", "DvirHome/debug"};
  const char *t2[] = {"DvirHome/testDevice", "DvirHome/All"};
  const char *t3[2] = {"DvirHome/testDevice/Avail", "DvirHome/testDevice/State"};

  iot.add_gen_pubTopic(t, sizeof(t) / sizeof(t[0]));
  iot.add_subTopic(t2, sizeof(t2) / sizeof(t2[0]));
  iot.add_pubTopic(t3, sizeof(t3) / sizeof(t3[0]));
}

void init_iot()
{
  FLASH_DEF == 1 ? def_iot_fromCode() : def_iot_fromFlash();
  iot.start_services(extMQTT);
}

void setup()
{
  Serial.begin(115200);
  init_iot();
}

void loop()
{
  iot.looper();
}

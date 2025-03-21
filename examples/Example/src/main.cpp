#include <Arduino.h>
#include <myIOT2.h>

#define USE_HARDCODED_PARAMS false

myIOT2 iot;
constexpr const char *param_filenames[] = {"/iot_params.JSON", "/iot_topics.JSON", "/app_params.JSON"};

void extMQTT(char *incoming_msg, char *_topic)
{
  char msg[270];

  if (strcmp(incoming_msg, "status") == 0)
  {
    // sprintf(msg, "[Status]: State[%s], Avail[%s]",
    //         alarm_states[current_alarm_state_code], alarm_avail ? alarm_avail_states[0] : alarm_avail_states[1]);
    iot.pub_msg(msg);
  }
}

void set_hardcoded_topics()
{
  const char *default_subTopics[] = {"DvirHome/HardCoded", "DvirHome/All"};
  const char *default_pubTopics[] = {"DvirHome/HardCoded/Avail", "DvirHome/HardCoded/State"};
  const char *default_gen_pubTopics[] = {"DvirHome/Messages", "DvirHome/log", "DvirHome/debug"};

  iot.add_subTopic(default_subTopics, sizeof(default_subTopics) / sizeof(default_subTopics[0]));
  iot.add_pubTopic(default_pubTopics, sizeof(default_pubTopics) / sizeof(default_pubTopics[0]));
  iot.add_gen_pubTopic(default_gen_pubTopics, sizeof(default_gen_pubTopics) / sizeof(default_gen_pubTopics[0]));
}
void set_topics_from_flash(JsonDocument &DOC)
{
  JsonArray subTopics = DOC["subTopic"].as<JsonArray>();
  JsonArray pubTopic = DOC["pubTopic"].as<JsonArray>();
  JsonArray gen_pubTopic = DOC["gen_pubTopic"].as<JsonArray>();

  for (const auto &topic : subTopics)
  {
    iot.add_subTopic(topic);
  }
  for (const auto &topic : pubTopic)
  {
    iot.add_pubTopic(topic);
  }
  for (const auto &topic : gen_pubTopic)
  {
    iot.add_gen_pubTopic(topic);
  }
}
void start_iot2()
{
  StaticJsonDocument<600> DOC;

  iot.set_pFilenames((const char **)param_filenames, sizeof(param_filenames) / sizeof(param_filenames[0])); // set the filenames for the parameters

  iot.readFlashParameters(DOC, param_filenames[0]); // read & Update iot2 Parameters                                                         // iot2 Parameters. in case of failure, default values will be used

  if (iot.readJson_inFlash(DOC, param_filenames[1]) && USE_HARDCODED_PARAMS == false) // Topics. read topics from flash or use hardcoded topics
  {
    set_topics_from_flash(DOC);
  }
  else
  {
    set_hardcoded_topics();
  }

  if (iot.readJson_inFlash(DOC, param_filenames[2])) // Application Parameters
  {
    for (uint8_t i = 0; i < DOC["output_pins"].size(); i++)
    {
      Serial.printf("Pin[%d]: %d\n", i, DOC["output_pins"][i].as<int>());
    }
  }

  iot.start_services(extMQTT);
}

void setup()
{
  start_iot2();
}

void loop()
{
  iot.looper();
}
#include <Arduino.h>
#include "myIOT2.h"

#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266Ping.h>
#endif

// ~~~~~~ myIOT2 CLASS ~~~~~~~~~~~ //
myIOT2::myIOT2() : mqttClient(espClient), flog("/myIOTlog.txt"), clklog("/clkLOG.txt")
{
}
void myIOT2::start_services(cb_func funct, const char *ssid, const char *password, const char *mqtt_user, const char *mqtt_passw, const char *mqtt_broker, int log_ents)
{
	strcpy(_mqtt_server, mqtt_broker);
	strcpy(_mqtt_user, mqtt_user);
	strcpy(_mqtt_pwd, mqtt_passw);
	strcpy(_ssid, ssid);
	strcpy(_wifi_pwd, password);
	ext_mqtt = funct; // redirecting to ex-class function ( defined outside)

	if (useSerial)
	{
		Serial.begin(115200);
		delay(10);
	}
	if (useDebug)
	{
		flog.start(log_ents);
	}
	_start_network_services();

	if (useWDT)
	{
		_startWDT();
	}
	if (useOTA)
	{
		startOTA();
	}
	if (useBootClockLog && WiFi.isConnected())
	{
		clklog.start(20); // Dont need looper. saved only once a boot
		_update_bootclockLOG();
	}
}
void myIOT2::_post_boot_check()
{
	static bool checkAgain = true;
	if (checkAgain)
	{
		if (mqtt_detect_reset != 2)
		{
			char a[30];
			checkAgain = false;
			if (mqtt_detect_reset == 0)
			{
				sprintf(a, "Boot Type: [%s]", "Normal Boot");
			}
			else if (mqtt_detect_reset == 1)
			{
				sprintf(a, "Boot Type: [%s]", "Quick Reboot");
			}

			if (ignore_boot_msg == false)
			{
				pub_log(a);
			}
		}
	}
}
void myIOT2::looper()
{
	wdtResetCounter = 0; // reset WDT watchDog
	if (useOTA)
	{
		_acceptOTA();
	}
	if (_network_looper() == false)
	{
		if (noNetwork_Clock > 0 && useNetworkReset && (millis() - noNetwork_Clock > noNetwork_reset * MS2MINUTES))
		{ // no Wifi or no MQTT will cause a reset
			sendReset("NO NETWoRK");
		}
	}
	if (useDebug)
	{
		flog.looper(30); /* 30 seconds from last write or 70% of buffer */
	}
	_post_boot_check();
}

// ~~~~~~~ Wifi functions ~~~~~~~
bool myIOT2::_network_looper()
{
	static unsigned long NTP_err_clk = 0;
	static unsigned long _lastNTP_try = 0;
	static unsigned long _lastReco_try = 0;
	const uint8_t time_retry_mqtt = 15; // sec

	bool cur_NTP_status = _NTP_updated();
	bool cur_wifi_status = WiFi.isConnected();
	bool cur_mqtt_status = mqttClient.connected();

	if (cur_mqtt_status && cur_wifi_status) /* All good */
	{
		mqttClient.loop();
		noNetwork_Clock = 0; // counter resposnble to reset
		return 1;
	}
	else
	{
		if (cur_wifi_status == false) /* No WiFi Connected */
		{
			if (noNetwork_Clock == 0) /* First Time */
			{
				noNetwork_Clock = millis();
				return 0;
			}
			else
			{
				if (millis() > _lastReco_try + retryConnectWiFi * MS2MINUTES) /* try ater time interval */
				{
					_lastReco_try = millis();
					return _start_network_services();
				}
				else
				{
					return 0;
				}
			}
		}
		else if (cur_mqtt_status == false) /* No MQTT */
		{
			if (millis() - _lastReco_try > 1000 * time_retry_mqtt) /* Retry timeout */
			{
				static int ret_counter = 0;
				_lastReco_try = millis();
				ret_counter++;
				if (_Wifi_and_mqtt_OK == false) /* Case of fail at boot */
				{
					return _start_network_services();
				}
				else
				{
					if (_subscribeMQTT()) /* succeed to reconnect */
					{
						ret_counter++;
						mqttClient.loop();
						if (noNetwork_Clock != 0)
						{
							int not_con_period = (int)((millis() - noNetwork_Clock) / 1000UL);
							if (not_con_period > 30)
							{
								char b[50];
								sprintf(b, "MQTT reconnect after [%d] sec", not_con_period);
								pub_log(b);
							}
							if (ret_counter > 3)
							{
								char b[50];
								sprintf(b, "MQTT reconnect after [%d] retries", ret_counter);
								pub_log(b);
							}
						}
						ret_counter = 0;
						_lastReco_try = 0;
						noNetwork_Clock = 0;
						return 1;
					}
					else
					{
						if (noNetwork_Clock == 0)
						{
							noNetwork_Clock = millis();
						}
						mqttClient.disconnect();
						return 0;
					}
				}
			}
		}

		if (cur_NTP_status == false && cur_wifi_status == true) /* Wifi connected NTP failed */
		{
			if (NTP_err_clk == 0)
			{
				NTP_err_clk = millis();
			}
			else if (millis() - NTP_err_clk > 60 * MS2MINUTES)
			{
				sendReset("NTP_RESET");
			}
			else if (millis() - _lastNTP_try > 10 * MS2MINUTES)
			{
				_lastNTP_try = millis();
				if (_startNTP())
				{
					NTP_err_clk = 0;
				}
			}
		}
	}
}
bool myIOT2::_start_network_services()
{
	bool a = false;
	bool b = false;
	_Wifi_and_mqtt_OK = false;

	if (_startWifi(_ssid, _wifi_pwd))
	{
		a = _startNTP();
		b = _startMQTT();
		_Wifi_and_mqtt_OK = a && b;
	}
	return _Wifi_and_mqtt_OK;
}
bool myIOT2::_startWifi(const char *ssid, const char *password)
{
	long startWifiConnection = millis();

	if (useSerial)
	{
		Serial.println();
		Serial.print("Connecting to ");
		Serial.println(ssid);
	}
	WiFi.mode(WIFI_OFF); // <---- NEW
	delay(100);
	WiFi.mode(WIFI_STA);
	WiFi.disconnect(true);
	delay(100);
	WiFi.begin(ssid, password);

	// in case of reboot - timeOUT to wifi
	while (WiFi.status() != WL_CONNECTED && (millis() < WIFItimeOut * MS2MINUTES / 60 + startWifiConnection))
	{
		delay(100);
		if (useSerial)
		{
			Serial.print(".");
		}
	}

	// case of no success - restart due to no wifi
	if (WiFi.status() != WL_CONNECTED)
	{
		if (useSerial)
		{
			Serial.println("no wifi detected");
			Serial.printf("\nConnection status: %d\n", WiFi.status());
		}
		if (noNetwork_Clock == 0)
		{
			noNetwork_Clock = millis();
		}
		return 0;
	}

	// if wifi is OK
	else
	{
		if (useSerial)
		{
			Serial.println("");
			Serial.println("WiFi connected");
			Serial.print("IP address: ");
			Serial.println(WiFi.localIP());
		}
		noNetwork_Clock = 0;
		return 1;
	}
}

// ~~~~~~~ NTP & Clock  ~~~~~~~~
bool myIOT2::_startNTP(const char *ntpServer, const char *ntpServer2)
{
	unsigned long startLoop = millis();
	while (!_NTP_updated() && (millis() - startLoop < 20000))
	{
#if isESP8266
		configTime(TZ_Asia_Jerusalem, ntpServer2, ntpServer); // configuring time offset and an NTP server
#elif isESP32
		configTzTime(TZ_Asia_Jerusalem, ntpServer2, ntpServer);
#endif
		delay(250);
	}
	return (_NTP_updated());
}
char *myIOT2::get_timeStamp(char ret[], time_t t)
{
	if (t == 0)
	{
		t = now();
	}
	struct tm *tm = localtime(&t);
	sprintf(ret, "%04d-%02d-%02d %02d:%02d:%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
	return ret;
}
void myIOT2::return_clock(char ret_tuple[20])
{
	time_t t = now();
	struct tm *tm = localtime(&t);
	sprintf(ret_tuple, "%02d:%02d:%02d", tm->tm_hour, tm->tm_min, tm->tm_sec);
}
void myIOT2::return_date(char ret_tuple[20])
{
	time_t t = now();
	struct tm *tm = localtime(&t);
	sprintf(ret_tuple, "%02d-%02d-%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday);
}
bool myIOT2::checkInternet(char *externalSite, uint8_t pings)
{
	return Ping.ping(externalSite, pings);
}
void myIOT2::convert_epoch2clock(long t1, long t2, char *time_str, char *days_str)
{
	uint8_t days = 0;
	uint8_t hours = 0;
	uint8_t minutes = 0;
	uint8_t seconds = 0;

	const uint8_t sec2minutes = 60;
	const int sec2hours = (sec2minutes * 60);
	const int sec2days = (sec2hours * 24);

	long time_delta = t1 - t2;

	days = (int)(time_delta / sec2days);
	hours = (int)((time_delta - days * sec2days) / sec2hours);
	minutes = (int)((time_delta - days * sec2days - hours * sec2hours) / sec2minutes);
	seconds = (int)(time_delta - days * sec2days - hours * sec2hours - minutes * sec2minutes);

	sprintf(days_str, "%01dd", days);
	sprintf(time_str, "%02d:%02d:%02d", hours, minutes, seconds);
}
time_t myIOT2::now()
{
	return time(nullptr);
}
bool myIOT2::_NTP_updated()
{
	return now() > 1640803233;
}

// ~~~~~~~ MQTT functions ~~~~~~~
bool myIOT2::_startMQTT()
{
	mqttClient.setServer(_mqtt_server, 1883);
	mqttClient.setKeepAlive(45);
	if (useSerial)
	{
		Serial.print("MQTT SERVER: ");
		Serial.println(_mqtt_server);
	}
	mqttClient.setCallback(std::bind(&myIOT2::_MQTTcb, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	return _subscribeMQTT();
}
bool myIOT2::_subscribeMQTT()
{
	if (!mqttClient.connected())
	{
		if (useSerial)
		{
			Serial.print("Attempting MQTT connection...");
		}
		char tempname[15];
#if isESP8266
		sprintf(tempname, "ESP_%s", String(ESP.getChipId()).c_str());
#elif isESP32
		uint64_t chipid = ESP.getEfuseMac();
		sprintf(tempname, "ESP32_%04X", (uint16_t)(chipid >> 32));
#endif
		char DEV[MaxTopicLength2];
		char NAME[MaxTopicLength2];
		char _ALLtopic[MaxTopicLength2];
		char _addgroupTopic[MaxTopicLength2];

		snprintf(_ALLtopic, MaxTopicLength2, "%s/All", prefixTopic);

		if (strcmp(addGroupTopic, "") != 0)
		{
			snprintf(_addgroupTopic, MaxTopicLength2, "%s/%s", prefixTopic, addGroupTopic);
		}
		else
		{
			strcpy(_addgroupTopic, "");
		}

		_devName(DEV);
		_availName(NAME);

		const char *topicArry[4] = {DEV, _ALLtopic, NAME, _addgroupTopic}; /* Subscribe to these topics (listen)*/

		if (mqttClient.connect(tempname, _mqtt_user, _mqtt_pwd, topicArry[2], 1, true, "offline"))
		{
			// Connecting sequence
			for (int i = 0; i < sizeof(topicArry) / sizeof(char *); i++)
			{
				if (strcmp(topicArry[i], "") != 0)
				{
					mqttClient.subscribe(topicArry[i]);
					if (useSerial)
					{
						// DEBUG
						Serial.println();
						Serial.print(topicArry[i]);
						// DEBUG
					}
				}
			}
			if (useextTopic)
			{
				for (int i = 0; i < sizeof(extTopic) / sizeof(char *); i++)
				{
					if (extTopic[i] != nullptr)
					{
						mqttClient.subscribe(extTopic[i]);
					}
				}
			}
			if (useSerial)
			{
				Serial.println("connected");
			}
			if (firstRun)
			{
				char buf[16];
				char msg[60];
				sprintf(msg, "<< PowerON Boot >> IP:[%d.%d.%d.%d]", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);
				if (!ignore_boot_msg)
				{
					pub_log(msg);
				}
				if (!useResetKeeper)
				{
					firstRun = false;
					mqtt_detect_reset = 0;
				}
			}
			notifyOnline();

			return 1;
		}
		else
		{ // fail to connect MQTT
			if (useSerial)
			{
				Serial.print("failed, rc=");
				Serial.println(mqttClient.state());
			}
			return 0;
		}
	}
	else
	{
		if (useSerial)
		{
			// DEBUG
			Serial.println("No need to reconnect. MQTT_already Connected");
			// DEBUG
		}
		return 1;
	}
}
void myIOT2::_MQTTcb(char *topic, uint8_t *payload, unsigned int length)
{
	char incoming_msg[length + 5];
	char msg[200];

	if (useSerial)
	{
		Serial.print("Message arrived [");
		Serial.print(topic);
		Serial.print("] ");
	}
	for (unsigned int i = 0; i < length; i++)
	{
		if (useSerial)
		{
			Serial.print((char)payload[i]);
		}
		incoming_msg[i] = (char)payload[i];
	}
	incoming_msg[length] = 0;

	if (useSerial)
	{
		Serial.println("");
	}
	if (useextTopic)
	{
		for (uint8_t i = 0; i < _size_extTopic; i++)
		{
			if (extTopic[i] != nullptr && strcmp(extTopic[i], topic) == 0)
			{
				strcpy(extTopic_msgArray[0]->msg, incoming_msg);
				strcpy(extTopic_msgArray[0]->device_topic, deviceTopic);
				strcpy(extTopic_msgArray[0]->from_topic, topic);
				extTopic_newmsg_flag = true;
			}
		}
	}

	char NAME[MaxTopicLength2];
	_availName(NAME);
	if (strcmp(topic, NAME) == 0 && useResetKeeper && firstRun)
	{
		_getBootReason_resetKeeper(incoming_msg);
	}

	if (strcmp(incoming_msg, "ota") == 0)
	{
		sprintf(msg, "OTA allowed for %ld seconds", OTA_upload_interval * MS2MINUTES / 1000);
		pub_msg(msg);
		allowOTA_clock = millis();
	}
	else if (strcmp(incoming_msg, "reset") == 0)
	{
		sendReset("MQTT");
	}
	else if (strcmp(incoming_msg, "ver") == 0)
	{
		sprintf(msg, "ver: IOTlib: [%s], flashLOG[%s]", ver, flog.VeR);
		pub_msg(msg);
	}
	else if (strcmp(incoming_msg, "services") == 0)
	{
		sprintf(msg, "Services[#1]: WDT: [%d], OTA: [%d], SERIAL: [%d], ResetKeeper[%d], useDebugLog[%d] , no-networkReset[%d] [%d min]",
				useWDT, useOTA, useSerial, useResetKeeper, useDebug, useNetworkReset, noNetwork_reset);
		pub_msg(msg);

		sprintf(msg, "Services[#2]: useBootLog[%d], extTopic[%d], AlterMQTTserver[%d], ignore_boot_msg[%d], debug_level[%d]",
				useBootClockLog, useextTopic, useAltermqttServer, ignore_boot_msg);
		pub_msg(msg);
	}
	else if (strcmp(incoming_msg, "help") == 0)
	{
		sprintf(msg, "Help: Commands #1 - [status, reset, ota, ver, ver2, help, help2, MCU_type, services, network]");
		pub_msg(msg);
		sprintf(msg, "Help: Commands #2 - [flash_format, del_debuglog, show_debuglog, show_bootlog]");
		pub_msg(msg);
	}
	else if (strcmp(incoming_msg, "MCU_type") == 0)
	{
		if (isESP8266)
		{
			sprintf(msg, "MCU: ESP8266");
		}
		else if (isESP32)
		{
			sprintf(msg, "MCU: ESP32");
		}
		else
		{
			sprintf(msg, "MCU: unKnown");
		}
		pub_msg(msg);
	}
	else if (strcmp(incoming_msg, "show_debuglog") == 0)
	{
		if (useDebug)
		{
			char m[250];
			char clk[25];
			get_timeStamp(clk);
			int x = flog.getnumlines();
			sprintf(m, " \n<<~~~~~~[%s] %s Debuglog : entries [#%d], file-size[%.2f kb] ~~~~~~~~~~>>\n ",
					clk, deviceTopic, flog.getnumlines(), (float)(flog.sizelog() / 1000.0));
			pub_debug(m);
			for (int a = 0; a < x; a++)
			{
				flog.readline(a, m);
				pub_debug(m);
				if (useSerial)
				{
					Serial.println(m);
				}
				delay(20);
			}
			pub_msg("debug_log: extracted");
			sprintf(m, " \n<<~~~~~~ %s Debuglog End ~~~~~~~~~~>> ", deviceTopic);
			pub_debug(m);
		}
	}
	else if (strcmp(incoming_msg, "flash_format") == 0)
	{
		pub_msg("Flash: Starting flash Format. System will reset at end.");
		sendReset("End Format");
	}
	else if (strcmp(incoming_msg, "del_debuglog") == 0)
	{
		if (useDebug)
		{
			flog.delog();
			pub_msg("debug_log: file deleted");
		}
	}
	else if (strcmp(incoming_msg, "show_bootlog") == 0)
	{
		if (useBootClockLog)
		{
			char m[250];
			char clk[25];
			get_timeStamp(clk);
			int x = clklog.getnumlines();
			sprintf(m, " \n<<~~~~~~[%s] %s bootlog : entries [#%d], file-size[%.2f kb] ~~~~~~~~~~>>\n ",
					clk, deviceTopic, x, (float)(clklog.sizelog() / 1000.0));
			pub_debug(m);

			for (uint8_t i = 0; i < x; i++)
			{
				clklog.readline(i, m);
				pub_debug(get_timeStamp(clk, atol(m)));
			}
			sprintf(m, " \n<<~~~~~~ %s bootlog End ~~~~~~~~~~>> ", deviceTopic);
			pub_debug(m);
			pub_msg("[BootLog]: Extracted");
		}
		else
		{
			pub_msg("BootLog: Not supported");
		}
	}
	else if (strcmp(incoming_msg, "network") == 0)
	{
		char IPadd[16];
		char days[10];
		char clock[25];

		unsigned long t = (long)(millis() / 1000);
		convert_epoch2clock(t, 0, clock, days);
		sprintf(IPadd, "%d.%d.%d.%d", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);
		sprintf(msg, "Network: uptime[%s %s], localIP[%s], MQTTserver[%s],  RSSI: [%d dBm]", days, clock, IPadd, _mqtt_server, WiFi.RSSI());

		pub_msg(msg);
	}

	else
	{
		ext_mqtt(incoming_msg);
	}
}
void myIOT2::_pub_generic(char *topic, char *inmsg, bool retain, char *devname, bool bare)
{
	char header[100];
	int lenhdr = 0;
	int lenmsg = strlen(inmsg);
	const int mqtt_defsize = mqttClient.getBufferSize();
	const uint8_t mqtt_overhead_size = 23;

	if (!bare)
	{
		char clk[25];
		get_timeStamp(clk, 0);
		if (strcmp(devname, "") == 0)
		{
			char DEV[MaxTopicLength2];
			_devName(DEV);
			sprintf(header, "[%s] [%s] ", clk, DEV);
		}
		else
		{
			sprintf(header, "[%s] [%s] ", clk, devname);
		}
		lenhdr = strlen(header);
	}
	else
	{
		sprintf(header, "");
	}
	char tmpmsg[lenmsg + lenhdr + 5];
	sprintf(tmpmsg, "%s%s", header, inmsg);

	if (strlen(tmpmsg) + mqtt_overhead_size + strlen(topic) > mqtt_defsize)
	{
		mqttClient.setBufferSize(strlen(tmpmsg) + mqtt_overhead_size + strlen(topic));
		mqttClient.publish(topic, tmpmsg, retain);
		mqttClient.setBufferSize(mqtt_defsize);
	}
	else
	{
		mqttClient.publish(topic, tmpmsg, retain);
	}
}
void myIOT2::pub_msg(char *inmsg)
{
	char _msgTopic[MaxTopicLength2];
	snprintf(_msgTopic, MaxTopicLength2, "%s/Messages", prefixTopic);

	_pub_generic(_msgTopic, inmsg);
	_write_log(inmsg, 0, _msgTopic);
}
void myIOT2::pub_noTopic(char *inmsg, char *Topic, bool retain)
{
	_pub_generic(Topic, inmsg, retain, "", true);
	_write_log(inmsg, 0, Topic);
}
void myIOT2::pub_state(char *inmsg, uint8_t i)
{
	char _stateTopic[MaxTopicLength2];
	char _stateTopic2[MaxTopicLength2];
	char DEV[MaxTopicLength2];
	_devName(DEV);
	snprintf(_stateTopic, MaxTopicLength2, "%s/State", DEV);
	snprintf(_stateTopic2, MaxTopicLength2, "%s/State_2", DEV);

	char *st[] = {_stateTopic, _stateTopic2};
	mqttClient.publish(st[i], inmsg, true);
	_write_log(inmsg, 2, st[i]);
}
void myIOT2::pub_log(char *inmsg)
{
	char _logTopic[MaxTopicLength2];
	snprintf(_logTopic, MaxTopicLength2, "%s/log", prefixTopic);

	_pub_generic(_logTopic, inmsg);
	_write_log(inmsg, 1, _logTopic);
}
void myIOT2::pub_ext(char *inmsg, char *name, bool retain, uint8_t i)
{
	_pub_generic(extTopic[i], inmsg, retain, name);
	_write_log(inmsg, 0, extTopic[i]);
}
void myIOT2::pub_debug(char *inmsg)
{
	char _debugTopic[MaxTopicLength2];
	snprintf(_debugTopic, MaxTopicLength2, "%s/debug", prefixTopic);

	if (strlen(inmsg) + 23 > mqttClient.getBufferSize())
	{
		const int mqtt_defsize = mqttClient.getBufferSize();
		mqttClient.setBufferSize(strlen(inmsg) + 23);
		mqttClient.publish(_debugTopic, inmsg);
		mqttClient.setBufferSize(mqtt_defsize);
	}
	else
	{
		mqttClient.publish(_debugTopic, inmsg);
	}
}
void myIOT2::pub_sms(String &inmsg, char *name)
{
	char _smsTopic[MaxTopicLength2];
	snprintf(_smsTopic, MaxTopicLength2, "%s/sms", prefixTopic);
	int len = inmsg.length() + 1;
	char sms_char[len];
	inmsg.toCharArray(sms_char, len);
	_pub_generic(_smsTopic, sms_char, false, name, true);
	_write_log(sms_char, 0, _smsTopic);
}
void myIOT2::pub_sms(char *inmsg, char *name)
{
	char _smsTopic[MaxTopicLength2];
	snprintf(_smsTopic, MaxTopicLength2, "%s/sms", prefixTopic);
	_pub_generic(_smsTopic, inmsg, false, name, true);
	_write_log(inmsg, 0, _smsTopic);
}
void myIOT2::pub_sms(JsonDocument &sms)
{
	char _smsTopic[MaxTopicLength2];
	snprintf(_smsTopic, MaxTopicLength2, "%s/sms", prefixTopic);
	String output;
	serializeJson(sms, output);
	int len = output.length() + 1;
	char sms_char[len];
	output.toCharArray(sms_char, len);

	_pub_generic(_smsTopic, sms_char, false, "", true);
}
void myIOT2::pub_email(String &inmsg, char *name)
{
	char _emailTopic[MaxTopicLength2];
	snprintf(_emailTopic, MaxTopicLength2, "%s/email", prefixTopic);
	int len = inmsg.length() + 1;
	char email_char[len];
	inmsg.toCharArray(email_char, len);
	_pub_generic(_emailTopic, email_char, false, name, true);
	_write_log(email_char, 0, _emailTopic);
}
void myIOT2::pub_email(JsonDocument &email)
{
	char _emailTopic[MaxTopicLength2];
	snprintf(_emailTopic, MaxTopicLength2, "%s/email", prefixTopic);
	String output;
	serializeJson(email, output);
	int len = output.length() + 1;
	char email_char[len];
	output.toCharArray(email_char, len);

	_pub_generic(_emailTopic, email_char, false, "", true);
	_write_log(email_char, 0, _emailTopic);
}
const char *myIOT2::_devName(char ret[])
{
	if (strcmp(addGroupTopic, "") != 0)
	{
		snprintf(ret, MaxTopicLength2, "%s/%s/%s", prefixTopic, addGroupTopic, deviceTopic);
	}
	else
	{
		snprintf(ret, MaxTopicLength2, "%s/%s", prefixTopic, deviceTopic);
	}
	return ret;
}
const char *myIOT2::_availName(char ret[])
{
	char DEV[MaxTopicLength2];
	_devName(DEV);
	snprintf(ret, MaxTopicLength2, "%s/Avail", DEV);
	return ret;
}

void myIOT2::notifyOnline()
{
	char NAME[MaxTopicLength2];
	_availName(NAME);
	mqttClient.publish(NAME, "online", true);
	_write_log("online", 2, NAME);
}
void myIOT2::_getBootReason_resetKeeper(char *msg)
{
	if (strcmp(msg, "online") == 0)
	{
		mqtt_detect_reset = 1; // bad reboot
	}
	else
	{
		mqtt_detect_reset = 0; // ordinary boot
	}
	firstRun = false;
}
void myIOT2::clear_ExtTopicbuff()
{
	strcpy(extTopic_msgArray[0]->msg, "");
	strcpy(extTopic_msgArray[0]->device_topic, "");
	strcpy(extTopic_msgArray[0]->from_topic, "");
	extTopic_newmsg_flag = false;
}
uint8_t myIOT2::inline_read(char *inputstr)
{
	char *pch;
	int i = 0;

	pch = strtok(inputstr, " ,.-");
	while (pch != NULL && i < num_param)
	{
		sprintf(inline_param[i], "%s", pch);
		pch = strtok(NULL, " ,.-");
		i++;
	}
	return i;
}

// ~~~~~~~~~~ Data Storage ~~~~~~~~~
void myIOT2::_write_log(char *inmsg, uint8_t x, const char *topic)
{
	char a[strlen(inmsg) + 100];

	if (useDebug && debug_level <= x)
	{
		char clk[25];
		sprintf(a, ">>%s<< [%s] %s", get_timeStamp(clk, 0), topic, inmsg);
		flog.write(a);
		if (useSerial)
		{
			Serial.println(a);
		}
	}
}
void myIOT2::_update_bootclockLOG()
{
	char clk_char[25];
#if isESP8266
	sprintf(clk_char, "%lld", now());
#elif isESP32
	sprintf(clk_char, "%d", now());
#endif
	clklog.write(clk_char, true);
}
bool myIOT2::read_fPars(char *filename, String &defs, JsonDocument &DOC, int JSIZE)
{
	myJSON param_on_flash(filename, useSerial, JSIZE);
	param_on_flash.start();

	if (param_on_flash.file_exists())
	{
		if (param_on_flash.readJSON_file(DOC))
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		if (useSerial)
		{
			Serial.printf("\nfile %s not found", filename);
		}
		deserializeJson(DOC, defs);
		return 0;
	}
}
void myIOT2::export_fPars(char *filename, JsonDocument &DOC, int JSIZE)
{
	myJSON param_on_flash(filename, true, JSIZE); /* read stored JSON from file */
	param_on_flash.start();

	if (param_on_flash.file_exists())
	{
		param_on_flash.readJSON_file(DOC);
	}
	else
	{
		if (useSerial)
		{
			Serial.printf("\nfile %s read NOT-OK", filename);
		}
	}
}

// ~~~~~~ Reset and maintability ~~~~~~
void myIOT2::sendReset(char *header)
{
	char temp[150];

	sprintf(temp, "[%s] - Reset sent", header);
	char DEV[MaxTopicLength2];
	_devName(DEV);
	_write_log(temp, 2, DEV);
	if (useSerial)
	{
		Serial.println(temp);
	}
	if (strcmp(header, "null") != 0)
	{
		pub_msg(temp);
	}
	delay(1000);
#if isESP8266
	ESP.reset();
#elif isESP32
	ESP.restart();
#endif
}
void myIOT2::_feedTheDog()
{
	wdtResetCounter++;
#if isEPS8266
	if (wdtResetCounter >= wdtMaxRetries)
	{
		sendReset("Dog goes woof");
	}
#elif isESP32

#endif
}
void myIOT2::_acceptOTA()
{
	if (millis() - allowOTA_clock <= OTA_upload_interval * MS2MINUTES)
	{
		ArduinoOTA.handle();
	}
}
void myIOT2::startOTA()
{
	allowOTA_clock = millis();
	ArduinoOTA.setPort(8266);
	ArduinoOTA.setHostname(deviceTopic);
	ArduinoOTA.onStart([]()
					   {
						   String type;
						   if (ArduinoOTA.getCommand() == U_FLASH)
						   {
							   type = "sketch";
						   }
						   else
						   { 
							   type = "filesystem";
						   } });
	if (useSerial)
	{
		ArduinoOTA.onEnd([]()
						 { Serial.println("\nEnd"); });
		ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
							  { Serial.printf("Progress: %u%%\r", (progress / (total / 100))); });
		ArduinoOTA.onError([](ota_error_t error)
						   {
							   Serial.printf("Error[%u]: ", error);
							   if (error == OTA_AUTH_ERROR)
							   {
								   Serial.println("Auth Failed");
							   }
							   else if (error == OTA_BEGIN_ERROR)
							   {
								   Serial.println("Begin Failed");
							   }
							   else if (error == OTA_CONNECT_ERROR)
							   {
								   Serial.println("Connect Failed");
							   }
							   else if (error == OTA_RECEIVE_ERROR)
							   {
								   Serial.println("Receive Failed");
							   }
							   else if (error == OTA_END_ERROR)
							   {
								   Serial.println("End Failed");
							   } });
	}

	ArduinoOTA.begin();
}
void myIOT2::_startWDT()
{
#if isESP8266
	wdt.attach(1, std::bind(&myIOT2::_feedTheDog, this)); // Start WatchDog
#elif isESP32
	// wdt.attach(1,_feedTheDog); // Start WatchDog
#endif
}

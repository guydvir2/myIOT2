#include <Arduino.h>
#include "myIOT2.h"

#if defined(ESP8266)
#include <ESP8266Ping.h>
#endif

// ~~~~~~ myIOT2 CLASS ~~~~~~~~~~~ //
myIOT2::myIOT2() : mqttClient(espClient),
				   flog("/myIOTlog.txt"),
				   clklog("/clkLOG.txt"),
				   _MQTTConnCheck(Chrono::SECONDS),
				   _WifiConnCheck(Chrono::SECONDS),
				   _NTPCheck(Chrono::SECONDS)
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
		PRNTL(F("\n\n>>> ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Start myIOT2 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ "));
		Serial.flush();
		delay(10);
	}
	if (useFlashP)
	{
		PRNTL(F(">>> Start loading Flash Parameters"));
		get_flashParameters();
	}

	if (useDebug)
	{
		PRNTL(F(">>> Start debuglog services"));
		flog.start(log_ents);
	}

	_start_network_services();

	if (useWDT)
	{
		PRNTL(F(">>> Start Watchdog"));
		_startWDT();
	}
	if (useOTA)
	{
		PRNTL(F(">>> Start OTA"));
		startOTA();
	}
	if (useBootClockLog && WiFi.isConnected())
	{
		PRNTL(F(">>> Start bootlog"));
		clklog.start(20); // Dont need looper. saved only once a boot
		_store_bootclockLOG();
	}
}
void myIOT2::_post_boot_check()
{
	// static bool checkAgain = true;
	// if (checkAgain)
	// {
	// 	if (mqtt_detect_reset != 2)
	// 	{
	// 		char a[30];
	// 		checkAgain = false;
	// 		if (mqtt_detect_reset == 0)
	// 		{
	// 			sprintf(a, "Boot Type: [%s]", "Normal Boot");
	// 		}
	// 		else if (mqtt_detect_reset == 1)
	// 		{
	// 			sprintf(a, "Boot Type: [%s]", "Quick Reboot");
	// 		}

	// 		if (ignore_boot_msg == false)
	// 		{
	// 			pub_log(a);
	// 		}
	// 	}
	// }
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
		if ((_WifiConnCheck.isRunning() && _WifiConnCheck.hasPassed(noNetwork_reset * 60)) || (_MQTTConnCheck.isRunning() && _MQTTConnCheck.hasPassed(noNetwork_reset * 60)))
		{ // no Wifi or no MQTT will cause a reset
			sendReset("Reset due to NO NETWoRK");
		}
	}
	if (useDebug)
	{
		flog.looper(30); /* 30 seconds from last write or 70% of buffer */
	}
	// _post_boot_check();
}

// // ~~~~~~~ Wifi functions ~~~~~~~
bool myIOT2::_network_looper()
{
	const uint8_t time_retry_NTP = 60; // sec
	const int time_reset_NTP = 360;	   // sec

	bool cur_wifi_status = WiFi.isConnected();
	bool cur_mqtt_status = mqttClient.connected();

	if (_NTPCheck.isRunning())
	{
		if (_NTPCheck.hasPassed(time_reset_NTP)) /* Reset after 1 hour */
		{
			sendReset("NTP_RESET");
		}
		else if (_NTPCheck.hasPassed(time_retry_NTP)) /* Retry update every 1 minute*/
		{
			if (!_NTP_updated())
			{
				_startNTP();
			}
		}
	}
	if (cur_mqtt_status && cur_wifi_status) /* All good */
	{
		mqttClient.loop();
		_WifiConnCheck.stop();
		_MQTTConnCheck.stop();
		return 1;
	}
	else
	{
		if (cur_wifi_status == false) /* No WiFi Connected */
		{
			return _try_rgain_wifi();
		}
		else if (cur_mqtt_status == false) /* No MQTT */
		{
			return _try_regain_MQTT();
		}
	}
}
bool myIOT2::_start_network_services()
{
	_Wifi_and_mqtt_OK = false;
	PRNTL(F(">>> Start networking services"));

	if (_startWifi(_ssid, _wifi_pwd))
	{
		if (!_NTP_updated())
		{
			_startNTP();
		}
		_Wifi_and_mqtt_OK = _startMQTT();
	}
	return _Wifi_and_mqtt_OK;
}
bool myIOT2::_startWifi(const char *ssid, const char *password)
{
	long startWifiConnection = millis();

	PRNT(F("\n Connecting to "));
	PRNT(ssid);

	_shutdown_wifi();
	WiFi.begin(ssid, password);

	// in case of reboot - timeOUT to wifi
	while (WiFi.status() != WL_CONNECTED && (millis() < WIFItimeOut * MS2MINUTES / 60 + startWifiConnection))
	{
		delay(100);
		PRNT(".");
	}

	// case of no success - restart due to no wifi
	if (WiFi.status() != WL_CONNECTED)
	{
		PRNTL(F(">>> no wifi connected"));
		PRNT(F("\nConnection status: "));
		PRNTL(WiFi.status());

		if (!_WifiConnCheck.isRunning())
		{
			_WifiConnCheck.restart();
		}
		return 0;
	}

	// if wifi is OK
	else
	{
		PRNTL("");
		PRNTL(F(">>> wifi connected"));
		PRNT(F("IP address: "));
		PRNTL(WiFi.localIP());

		_WifiConnCheck.stop();
		return 1;
	}
}
void myIOT2::_shutdown_wifi()
{
	WiFi.mode(WIFI_OFF); // <---- NEW
	delay(200);
	WiFi.mode(WIFI_STA);
	// WiFi.disconnect(true);
	// delay(200);
}
bool myIOT2::_try_rgain_wifi()
{
	if (!_WifiConnCheck.isRunning())
	{
		_WifiConnCheck.restart();
		return 0;
	}
	else
	{
		if (_WifiConnCheck.hasPassed(retryConnectWiFi))
		{
			_WifiConnCheck.restart();
			if (_start_network_services())
			{
				_WifiConnCheck.stop();
				return 1;
			}
			else
			{
				return 0;
			}
		}
		else
		{
			return 0;
		}
	}
}

// // ~~~~~~~ NTP & Clock  ~~~~~~~~
bool myIOT2::_startNTP(const char *ntpServer, const char *ntpServer2)
{
	unsigned long startLoop = millis();
	while (!_NTP_updated() && (millis() - startLoop < 20000))
	{
#if defined(ESP8266)
		configTime(TZ_Asia_Jerusalem, ntpServer2, ntpServer); // configuring time offset and an NTP server
#elif defined(ESP32)
		configTzTime(TZ_Asia_Jerusalem, ntpServer2, ntpServer);
#endif
		delay(250);
	}
	if (!_NTP_updated())
	{
		PRNTL(F(">>> NTP Update fail"));
		_NTPCheck.restart();
		return 0;
	}
	else
	{
		PRNTL(F(">>> NTP Update OK"));
		_NTPCheck.stop();
		return 1;
	}
}
void myIOT2::get_timeStamp(char ret[], time_t t)
{
	if (t == 0)
	{
		t = now();
	}
	struct tm *tm = localtime(&t);
	sprintf(ret, "%04d-%02d-%02d %02d:%02d:%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
}
bool myIOT2::pingSite(char *externalSite, uint8_t pings)
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
	if (days_str != nullptr)
	{
		sprintf(days_str, "%01dd", days);
	}
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

// // ~~~~~~~ MQTT functions ~~~~~~~

bool myIOT2::_startMQTT()
{
	mqttClient.setServer(_mqtt_server, 1883);
	mqttClient.setKeepAlive(45);
	PRNT(F("MQTT Server: "));
	PRNTL(_mqtt_server);
	mqttClient.setCallback(std::bind(&myIOT2::_MQTTcb, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	return _subMQTT();
}
bool myIOT2::_try_regain_MQTT()
{
	const uint8_t time_retry_mqtt = 15; // sec
	if (!_MQTTConnCheck.isRunning())
	{
		_MQTTConnCheck.restart();
		return 0;
	}
	else
	{
		if (_MQTTConnCheck.hasPassed(time_retry_mqtt))
		{
			_MQTTConnCheck.restart();
			if (!_Wifi_and_mqtt_OK) /* Case of fail at boot */
			{
				return _start_network_services();
			}
			else
			{
				if (_subMQTT()) /* succeed to reconnect */
				{
					mqttClient.loop();

					int not_con_period = (int)((millis() - _MQTTConnCheck.elapsed()) / 1000UL);
					if (not_con_period > 30)
					{
						char b[50];
						sprintf(b, "MQTT reconnect after [%d] sec", not_con_period);
						pub_log(b);
					}
					_MQTTConnCheck.restart(); /* Zeroing counter */
					_MQTTConnCheck.stop();
					return 1;
				}
				else
				{
					if (_MQTTConnCheck.hasPassed(60))
					{
						flog.write("network shutdown", true);
						_shutdown_wifi();
						return _start_network_services();
					}
					else
					{
						return 0;
					}
				}
			}
		}
		else
		{
			return 0;
		}
	}
}
bool myIOT2::_subMQTT()
{
	if (!mqttClient.connected())
	{
		char tempname[15];
#if defined(ESP8266)
		sprintf(tempname, "ESP_%s", String(ESP.getChipId()).c_str());
#elif defined(ESP32)
		uint64_t chipid = ESP.getEfuseMac();
		sprintf(tempname, "ESP32_%04X", (uint16_t)(chipid >> 32));
#endif
		if (mqttClient.connect(tempname, _mqtt_user, _mqtt_pwd, TOPICS_JSON["pub_topics"][0].as<const char *>(), 1, true, "offline"))
		{
			PRNTL(F(">>> MQTT server Connected"));
			PRNTL(F("\n>>> Subscribe Topics:"));

			for (uint8_t i = 0; i < TOPICS_JSON["sub_topics"].size(); i++)
			{
				mqttClient.subscribe(TOPICS_JSON["sub_topics"][i].as<const char *>());
				PRNTL(TOPICS_JSON["sub_topics"][i].as<const char *>());
			}

			for (uint8_t i = 0; i < TOPICS_JSON["sub_data_topics"].size(); i++)
			{
				mqttClient.subscribe(TOPICS_JSON["sub_data_topics"][i].as<const char *>());
				PRNTL(TOPICS_JSON["sub_data_topics"][i].as<const char *>());
			}

			if (firstRun)
			{
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

			PRNT(F("failed rc="));
			PRNTL(mqttClient.state());
			return 0;
		}
	}
	else
	{
		PRNTL(F("No need to reconnect. MQTT_already Connected"));
		return 1;
	}
}
void myIOT2::_MQTTcb(char *topic, uint8_t *payload, unsigned int length)
{
	char incoming_msg[length + 5];
	char msg[200];

	PRNT(F("Message arrived ["));
	PRNT(topic);
	PRNT(F("] "));

	for (unsigned int i = 0; i < length; i++)
	{
		PRNT((char)payload[i]);
		incoming_msg[i] = (char)payload[i];
	}
	incoming_msg[length] = 0;
	PRNTL("");

	if (strcmp(topic, TOPICS_JSON["pub_topics"][0].as<const char *>()) == 0 && useResetKeeper && firstRun)
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

		sprintf(msg, "Services[#2]: useBootLog[%d] , ignore_boot_msg[%d], debug_level[%d], useFlashP[%d]",
				useBootClockLog, ignore_boot_msg, debug_level, useFlashP);
		pub_msg(msg);
	}
	else if (strcmp(incoming_msg, "help") == 0)
	{
		sprintf(msg, "Help: Commands #1 - [status, reset, ota, ver, ver2, help, help2, MCU_type, services, network]");
		pub_msg(msg);
		sprintf(msg, "Help: Commands #2 - [del_debuglog, del_bootlog, show_debuglog, show_bootlog, show_flashParam,{update_flash,[key],[value]}]");
		pub_msg(msg);
	}
	else if (strcmp(incoming_msg, "MCU_type") == 0)
	{
#if defined(ESP8266)
		sprintf(msg, "[MCU]: ESP8266");
#elif defined(ESP32)
		sprintf(msg, "[MCU]: ESP32");
#else
		sprintf(msg, "[MCU]: unKnown");
#endif
		pub_msg(msg);
	}
	else if (strcmp(incoming_msg, "show_debuglog") == 0)
	{
		if (useDebug)
		{
			_extract_log(flog, "debug_log");
		}
		else
		{
			pub_msg("[DebugLog]: Not supported");
		}
	}
	else if (strcmp(incoming_msg, "show_flashParam") == 0)
	{
		if (useFlashP)
		{
			char clk[25];
			get_timeStamp(clk);
			char *filenames[] = {sketch_paramfile, myIOT_paramfile, myIOT_topics};
			sprintf(msg, "\n<<~~~~~~ [%s] [%s] On-Flash Parameters ~~~~~>>", clk, TOPICS_JSON["sub_topics"][0].as<const char *>());
			pub_debug(msg);

			for (uint8_t i = 0; i < sizeof(filenames) / sizeof(filenames[0]); i++)
			{
				// 		StaticJsonDocument<max(MY_IOT_JSON_SIZE, MY_IOT_JSON_SIZE)> DOC;
				pub_debug(filenames[i]);
				// 		extract_JSON_from_flash(filenames[i], DOC);
				String tempstr1 = readFile(filenames[i]);
				// 		// char outmsg[500];
				// 		serializeJsonPretty(DOC, Serial);
				// 		// serializeJson(DOC, outmsg);
				// 		// Serial.println(outmsg);
				char buff[tempstr1.length() + 1];
				tempstr1.toCharArray(buff, tempstr1.length() + 1);
				pub_debug(buff);
			}
			pub_msg("[On-Flash Parameters]: extracted");
			pub_debug("<<~~~~~~~~~~ End ~~~~~~~~~~>>");
		}
		else
		{
			pub_msg("[On-Flash Parameters]: not in use");
		}
	}
	else if (strcmp(incoming_msg, "del_debuglog") == 0)
	{
		if (useDebug)
		{
			flog.delog();
			pub_msg("[debug_log]: file deleted");
		}
	}
	else if (strcmp(incoming_msg, "del_bootlog") == 0)
	{
		if (useDebug)
		{
			clklog.delog();
			pub_msg("[boot_log]: file deleted");
		}
	}
	else if (strcmp(incoming_msg, "show_bootlog") == 0)
	{
		if (useBootClockLog)
		{
			_extract_log(clklog, "boot_log", true);
		}
		else
		{
			pub_msg("[BootLog]: Not supported");
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
		num_p = inline_read(incoming_msg);

		if (num_p > 1 && strcmp(inline_param[0], "update_flash") == 0 && useFlashP)
		{
			_cmdline_flashUpdate(inline_param[1], inline_param[2]);
		}
		else
		{
			ext_mqtt(incoming_msg, topic);
		}
	}
}
void myIOT2::_pub_generic(const char *topic, char *inmsg, bool retain, char *devname, bool bare)
{
	char clk[25];
	get_timeStamp(clk, 0);
	const uint8_t mqtt_overhead_size = 23;
	const int mqtt_defsize = mqttClient.getBufferSize();

	int x = devname == nullptr ? strlen(TOPICS_JSON["sub_topics"][0].as<const char *>()) + 1 : 0;
	char tmpmsg[strlen(inmsg) + x + 8 + 25];

	if (!bare)
	{
		sprintf(tmpmsg, "[%s] [%s] %s", clk, TOPICS_JSON["sub_topics"][0].as<const char *>(), inmsg);
	}
	else
	{
		sprintf(tmpmsg, "%s", inmsg);
	}

	x = strlen(tmpmsg) + mqtt_overhead_size + strlen(topic);
	if (x > mqtt_defsize)
	{
		mqttClient.setBufferSize(x);
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
	_pub_generic(TOPICS_JSON["pub_gen_topics"][0].as<const char *>(), inmsg);
	_write_log(inmsg, 1, TOPICS_JSON["pub_gen_topics"][0].as<const char *>());
}
void myIOT2::pub_noTopic(char *inmsg, char *Topic, bool retain)
{
	_pub_generic(Topic, inmsg, retain, nullptr, true);
	_write_log(inmsg, 0, Topic);
}
void myIOT2::pub_state(char *inmsg, uint8_t i)
{
	char _stateTopic[strlen(TOPICS_JSON["pub_gen_topics"][1].as<const char *>()) + 4];

	if (i == 0)
	{
		sprintf(_stateTopic, "%s", TOPICS_JSON["pub_gen_topics"][1].as<const char *>());
	}
	else
	{
		sprintf(_stateTopic, "%s_%d", TOPICS_JSON["pub_gen_topics"][1].as<const char *>(), i);
	}
	mqttClient.publish(_stateTopic, inmsg, true);
	_write_log(inmsg, 2, _stateTopic);
}
void myIOT2::pub_log(char *inmsg)
{
	_pub_generic(TOPICS_JSON["pub_gen_topics"][1].as<const char *>(), inmsg);
	_write_log(inmsg, 1, TOPICS_JSON["pub_gen_topics"][1].as<const char *>());
}
void myIOT2::pub_debug(char *inmsg)
{
	_pub_generic(TOPICS_JSON["pub_gen_topics"][2].as<const char *>(), inmsg, false, nullptr, true);
}

void myIOT2::notifyOnline()
{
	mqttClient.publish(TOPICS_JSON["pub_topics"][0].as<const char *>(), "online", true);
	_write_log("online", 2, TOPICS_JSON["pub_topics"][0].as<const char *>());
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
uint8_t myIOT2::_getdataType(const char *y)
{
	/* Return values:
	0 - error
	1 - bool
	2 - string
	3 - float
	4 - int
	*/

	int i = atoi(y);
	float f = atof(y);

	if (isAlpha(y[0]))
	{
		if (strcmp(y, "true") == 0 || strcmp(y, "false") == 0)
		{
			return 1;
		}
		else
		{
			return 2;
		}
		return 0;
	}
	else
	{
		if (i != f)
		{
			return 3;
		}
		else
		{
			return 4;
		}
		return 0;
	}
}

// // ~~~~~~~~~~ Data Storage ~~~~~~~~~
void myIOT2::_write_log(char *inmsg, uint8_t x, const char *topic)
{
	char a[strlen(inmsg) + 100];

	if (useDebug && debug_level <= x)
	{
		char clk[25];
		char clk2[20];
		char days[10];
		get_timeStamp(clk, 0);
		convert_epoch2clock(millis() / 1000, 0, clk2, days);
		sprintf(a, ">>%s<< [uptime: %s %s] [%s] %s", clk, days, clk2, topic, inmsg);
		flog.write(a);
		PRNTL(a);
	}
}
void myIOT2::_store_bootclockLOG()
{
	char clk_char[25];
#if defined(ESP8266)
	sprintf(clk_char, "%lld", now());
#elif defined(ESP32)
	sprintf(clk_char, "%d", now());
#endif
	clklog.write(clk_char, true);
}

bool myIOT2::extract_JSON_from_flash(char *filename, JsonDocument &DOC)
{
	_startFS();
	File readFile = LITFS.open(filename, "r");
	DeserializationError error = deserializeJson(DOC, readFile);
	readFile.close();

	PRNT(F("\n>>> "));
	PRNT(filename);

	if (error)
	{
		PRNTL(F(":\tRead [failed]"));
		PRNT(F("error:\t"));
		PRNTL(error.c_str());
		Serial.flush();
		return 0;
	}
	else
	{
		PRNTL(F(":\tRead [OK]"));
		Serial.flush();
		return 1;
	}
}
void myIOT2::update_vars_flash_parameters(JsonDocument &DOC)
{
	useWDT = DOC["useWDT"].as<bool>();
	useOTA = DOC["useOTA"].as<bool>();
	useSerial = DOC["useSerial"].as<bool>();
	useFlashP = DOC["useFlashP"].as<bool>();
	useDebug = DOC["useDebugLog"].as<bool>();
	debug_level = DOC["debug_level"].as<uint8_t>();
	useResetKeeper = DOC["useResetKeeper"].as<bool>();
	useNetworkReset = DOC["useNetworkReset"].as<bool>();
	noNetwork_reset = DOC["noNetwork_reset"].as<uint8_t>();
	useBootClockLog = DOC["useBootClockLog"].as<bool>();
	ignore_boot_msg = DOC["ignore_boot_msg"].as<bool>();

	PRNTL(F(">>> Variables updated using Flash Parameters"));
}
void myIOT2::get_flashParameters()
{
	StaticJsonDocument<MY_IOT_JSON_SIZE> myIOT_P; /* !!! Check if this not has to change !!! */
	delay(50);
	PRNTL(F(">>> Start reading Flash Parameters."));

	if (!extract_JSON_from_flash(myIOT_topics, TOPICS_JSON))
	{
		TOPICS_JSON["pub_gen_topics"][0] = "myHome/Messages";
		TOPICS_JSON["pub_gen_topics"][1] = "myHome/log";
		TOPICS_JSON["pub_topics"][0] = "myHome/group/client/Avail";
		TOPICS_JSON["pub_topics"][1] = "myHome/group/client/State";
		TOPICS_JSON["sub_topics"][0] = "myHome/group/client";
		TOPICS_JSON["sub_topics"][1] = "myHome/All";

		useFlashP = false; /* Update service to false */
		PRNTL(F(">>> Failed read Topics. default values used"));
	}

	if (extract_JSON_from_flash(myIOT_paramfile, myIOT_P)) /* Case pulling from flash fails */
	{
		update_vars_flash_parameters(myIOT_P);
	}
	else
	{
		PRNTL(F(">>> Failed read Parameters.default values used"));
	}
	myIOT_P.clear();
}
bool myIOT2::_change_flashP_value(const char *key, const char *new_value, JsonDocument &DOC)
{
	uint8_t s = _getdataType(new_value);
	if (s == 1)
	{
		if (strcmp(new_value, "true") == 0)
		{
			DOC[key] = true;
		}
		else
		{
			DOC[key] = false;
		}
		return 1;
	}
	else if (s == 2)
	{
		DOC[key] = new_value;
		return 1;
	}
	else if (s == 3)
	{
		DOC[key] = (float)atof(new_value);
		return 1;
	}
	else if (s == 4)
	{
		DOC[key] = (int)atoi(new_value);
		return 1;
	}
	return 0;
}
bool myIOT2::_saveFile(char *filename, JsonDocument &DOC)
{
	File writefile = LITFS.open(filename, "w");
	if (!writefile || (serializeJson(DOC, writefile) == 0))
	{
		writefile.close();
		return 0;
	}
	writefile.close();
	return 1;
}
bool myIOT2::_cmdline_flashUpdate(const char *key, const char *new_value)
{
	char msg[100];
	bool succ_chg = false;
	char *allfiles[2] = {myIOT_paramfile, sketch_paramfile};
	DynamicJsonDocument myIOT_P(max(SKETCH_JSON_SIZE, MY_IOT_JSON_SIZE));

	for (uint8_t n = 0; n < 2; n++)
	{
		if (extract_JSON_from_flash(allfiles[n], myIOT_P))
		{
			if (myIOT_P.containsKey(key))
			{
				if (_change_flashP_value(key, new_value, myIOT_P))
				{
					if (_saveFile(allfiles[n], myIOT_P))
					{
						succ_chg = true;
						sprintf(msg, "[Flash]: parameter[%s] updated to[%s] [OK]", key, new_value);
					}
					else
					{
						succ_chg = false;
						sprintf(msg, "[Flash]: parameter[%s] [Failed] to updated. Save file error", key);
					}
				}
				else
				{
					succ_chg = false;
					sprintf(msg, "[Flash]: parameter[%s] replace [Failed]", key);
				}
				pub_msg(msg);
			}
		}
		else if (n == 1)
		{
			succ_chg = false;
			sprintf(msg, "[Flash]: parameter[%s] [NOT FOUND]", key);
			pub_msg(msg);
		}
	}
	return succ_chg;
}

String myIOT2::readFile(char *fileName)
{
	_startFS();
	File file = LITFS.open(fileName, "r");

	if (file)
	{
		String t = file.readString();
		file.close();
		return t;
	}
	else
	{
		file.close();
		return "";
	}
}
void myIOT2::_startFS()
{
#if defined(ESP8266)
	LITFS.begin();
#elif defined(ESP32)
	LITFS.begin(true);
#endif
}
void myIOT2::_extract_log(flashLOG &_flog, const char *_title, bool _isTimelog)
{
	char clk[25];
	char msg[200];
	get_timeStamp(clk);
	int x = _flog.get_num_saved_records();

	sprintf(msg, " \n<<~~~~~~[%s] %s %s : entries [#%d], file-size[%.2f kb] ~~~~~~~~~~>>\n ",
			clk, TOPICS_JSON["sub_topics"][0].as<const char *>(), _title, x, (float)(_flog.sizelog() / 1000.0));

	pub_debug(msg);
	for (int a = 0; a < x; a++)
	{
		_flog.readline(a, msg);
		if (_isTimelog)
		{
			strcpy(clk, msg);
			get_timeStamp(msg, atol(clk));
		}
		pub_debug(msg);

		PRNTL(msg);
		delay(20);
	}
	pub_msg("[log]: extracted");
	sprintf(msg, " \n<<~~~~~~ %s %s End ~~~~~~~~~~>> ", _title, TOPICS_JSON["sub_topics"][0].as<const char *>());
	pub_debug(msg);
}

// ~~~~~~ Reset and maintability ~~~~~~
void myIOT2::sendReset(char *header)
{
	char temp[17 + strlen(header)];

	sprintf(temp, "[%s] - Reset sent", header);
	_write_log(temp, 2, TOPICS_JSON["pub_gen_topics"][0].as<const char *>());
	PRNTL(temp);

	if (strcmp(header, "null") != 0)
	{
		pub_msg(temp);
	}
	delay(1000);
#if defined(ESP8266)
	ESP.reset();
#elif defined(ESP32)
	ESP.restart();
#endif
}
void myIOT2::_feedTheDog()
{
	wdtResetCounter++;
#if defined(ESP8266)
	if (wdtResetCounter >= wdtMaxRetries)
	{
		sendReset("Dog goes woof");
	}
#elif defined(ESP32)

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
	ArduinoOTA.setHostname(TOPICS_JSON["pub_gen_topics"][0]);
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
#if defined(ESP8266)
	wdt.attach(1, std::bind(&myIOT2::_feedTheDog, this)); // Start WatchDog
#elif defined(ESP32)
	// wdt.attach(1,_feedTheDog); // Start WatchDog
#endif
}

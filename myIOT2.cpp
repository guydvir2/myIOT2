#include "myIOT2.h"

// ~~~~~~ myIOT2 CLASS ~~~~~~~~~~~ //
myIOT2::myIOT2() : mqttClient(espClient)
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
		PRNT(F("\n\n>>> ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Start myIOT2 "));
		PRNT(ver);
		PRNTL(F(" ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"));
		delay(10);
	}

	_start_network_services();
	startOTA();

	PRNTL(F("\n >>>>>>>>>> Summary <<<<<<<<<<<<<"));
	PRNT(F("useSerial:\t\t"));
	PRNTL(useSerial);

	PRNT(F("useNetworkReset:\t"));
	PRNTL(useNetworkReset);
	PRNT(F("ignore_boot_msg:\t"));
	PRNTL(ignore_boot_msg);
	PRNT(F("useFlashP:\t\t"));
	PRNTL(useFlashP);
	PRNT(F("noNetwork_reset:\t"));
	PRNTL(noNetwork_reset);
	PRNT(F("Connected MQTT:\t\t"));
	PRNTL(mqttClient.connected());
	PRNT(F("Connected WiFi:\t\t"));
	PRNTL(WiFi.isConnected());
	PRNT(F("NTP sync:\t\t"));
	PRNTL(now());
	PRNT(F("Bootup sec:\t\t"));
	PRNTL((float)(millis() / 1000.0));
	PRNT(F("ESP type:\t\t"));

	char a[10];
#if defined(ESP8266)
	strcpy(a, "ESP8266");
#elif defined(ESP32)
	strcpy(a, "ESP32");
#endif
	PRNTL(a);
	PRNTL(F("\n >>>>>>>>>> END  <<<<<<<<<<<<<"));

	PRNTL(F("\n>>> ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ END myIOT2 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ \n"));
}
void myIOT2::looper()
{
	_acceptOTA();
	if (_network_looper() == false) /* Wifi or MQTT fails causes reset */
	{
		if (_timePassed(noNetwork_reset * 60))
		{
			sendReset("Reset due to NO NETWoRK");
		}
	}
}

// ~~~~~~~ Wifi functions ~~~~~~~
bool myIOT2::_network_looper()
{
	static bool NTPretry = false;
	bool isNetworkOK = WiFi.isConnected() && mqttClient.connected();

	if (!_NTP_updated())
	{
		if (millis() / 1000 > 60 && NTPretry == false)
		{
			NTPretry = true;
			_startNTP();
		}
		else if (millis() / 1000 > 600)
		{
			sendReset("NTP_RESET");
		}
		else
		{
			yield();
		}
	}

	if (isNetworkOK) /* All good */
	{
		mqttClient.loop();
		if (_nonetwork_clock > 0)
		{
			_nonetwork_clock = 0;
			_nextRetry = 0;
			Serial.println("No_Clock ZEROED");
			Serial.print("Disconnects: Wifi #");
			Serial.print(_wifi_counter);
			Serial.print("; MQTT #");
			Serial.println(_mqtt_counter);
			Serial.print("Accumulated disconnect time: Wifi ");
			Serial.print(_accum_wifi_not_connected);
			Serial.print(" sec; MQTT ");
			Serial.print(_accum_mqtt_not_connected);
			Serial.println(" sec");
		}
		else
		{
			yield();
		}
		return 1;
	}
	else
	{
		if (_nonetwork_clock == 0)
		{
			_nonetwork_clock = millis();
			Serial.println("No_Clock started");
		}

		if (!WiFi.isConnected()) /* No WiFi Connected */
		{
			if (_timePassed(_nextRetry))
			{
				PRNTL(F("~ Fail Wifi"));
				bool a = _try_rgain_wifi();
				if (!a)
				{
					_nextRetry = (millis() - _nonetwork_clock) / 1000 + retryConnectWiFi; // add time to next retry
					Serial.print("Wifi Add:");
					Serial.println(_nextRetry);
				}
				else
				{
					_nextRetry = 0;
				}
				return a;
			}
			else
			{
				return 0;
			}
		}
		else if (!mqttClient.connected()) /* No MQTT */
		{
			if (_timePassed(_nextRetry)) //!_retryTimeout.isRunning())
			{
				PRNTL(F("~ Fail MQTT"));
				bool a = _try_regain_MQTT();
				if (!a)
				{
					_nextRetry = (millis() - _nonetwork_clock) / 1000 + 10; // add time to next retry
					Serial.print("MQTT Add:");
					Serial.println(_nextRetry);
				}
				return a;
			}
			else
			{
				return 0;
			}
		}
		else
		{
			Serial.println("OTHER");
			return 0;
		}
	}
}
bool myIOT2::_start_network_services()
{
	_Wifi_and_mqtt_OK = false;
	PRNTL(F("~~ Start networking services"));

	if (_startWifi(_ssid, _wifi_pwd))
	{
		_startNTP();
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
		PRNT(F("~ Wifi NOT connected, status:"));
		PRNTL(WiFi.status());
		return 0;
	}

	// if wifi is OK
	else
	{
		PRNT(F("\n~ wifi connected. IP address: "));
		PRNTL(WiFi.localIP());
		return 1;
	}
}
void myIOT2::_shutdown_wifi()
{
	PRNTL(F("\n~ Shutting down Wifi"));
	WiFi.mode(WIFI_OFF); // <---- NEW
	delay(100);
#if defined(ESP32)
	WiFi.useStaticBuffers(true);
#endif
	WiFi.mode(WIFI_STA);
	WiFi.disconnect(true);
	delay(100);
}
bool myIOT2::_try_rgain_wifi()
{
	PRNTL(F("~ Try restore Wifi"));
	if (_start_network_services())
	{
		PRNTL(F("~ Wifi restored"));
		_wifi_counter++;
		_accum_wifi_not_connected += (millis() - _nonetwork_clock) / 1000;

		return 1;
	}
	else
	{
		PRNTL(F("~ Wifi failed to restore"));
		return 0;
	}
}

// ~~~~~~~ NTP & Clock  ~~~~~~~~
bool myIOT2::_startNTP(const char *ntpServer, const char *ntpServer2)
{
	const uint8_t _sec_to_NTP = 25;
	unsigned long startLoop = millis();
#if defined(ESP8266)
	configTime(TZ_Asia_Jerusalem, ntpServer2, ntpServer); // configuring time offset and an NTP server
#elif defined(ESP32)
	configTzTime(TZ_Asia_Jerusalem, ntpServer2, ntpServer);
#endif
	PRNT("~ NTP: ");
	while (!_NTP_updated() && (millis() - startLoop < _sec_to_NTP * 1000) && WiFi.isConnected()) /* ESP32 after software reset - doesnt enter here at all*/
	{
		delay(100);
		PRNT("*");
	}

	if (!_NTP_updated())
	{
		PRNTL(F("\n~ NTP Update fail"));
		return 0;
	}
	else
	{
		PRNTL("");
		PRNTL(F("~ NTP Update OK"));
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
void myIOT2::convert_epoch2clock(long t1, long t2, char time_str[], char days_str[])
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
bool myIOT2::_timePassed(unsigned int T)
{
	bool a = (_nonetwork_clock != 0 && (millis() - _nonetwork_clock) / 1000 > T);
	return a;
}

// ~~~~~~~ MQTT functions ~~~~~~~
bool myIOT2::_startMQTT()
{
	mqttClient.setServer(_mqtt_server, 1883);
	mqttClient.setKeepAlive(45);
	PRNT(F("~ MQTT Server: "));
	PRNTL(_mqtt_server);
	mqttClient.setCallback(std::bind(&myIOT2::_MQTTcb, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	return _subMQTT();
}
bool myIOT2::_try_regain_MQTT()
{
	PRNTL(F("~ Try regain MQTT"));
	if (!_Wifi_and_mqtt_OK) /* Case of fail at boot */
	{
		return _start_network_services();
	}
	else
	{
		if (_subMQTT()) /* succeed to reconnect */
		{
			PRNTL(F("~ MQTT restored"));
			mqttClient.loop();

			int not_con_period = (millis() - _nonetwork_clock) / 1000;
			if (not_con_period > 2)
			{
				char b[50];
				sprintf(b, "MQTT reconnect after [%d] sec", not_con_period);
				pub_log(b);
			}
			_mqtt_counter++;
			_accum_mqtt_not_connected += (millis() - _nonetwork_clock) / 1000;
			return 1;
		}
		else
		{
			if (_timePassed(60)) /* Resets all network */
			{
				PRNTL(F("~ MQTT fails, Restarting all network"));
				// flog.write("MQTT fail. network shutdown", true);
				_shutdown_wifi();
				delay(1000);
				return _start_network_services();
			}
			else
			{
				return 0;
			}
		}
	}
}
bool myIOT2::_subMQTT()
{
	if (!mqttClient.connected())
	{
		char tempname[20];
#if defined(ESP8266)
		sprintf(tempname, "ESP_%s", String(ESP.getChipId()).c_str());

#elif defined(ESP32)
		uint64_t chipid = ESP.getEfuseMac();
		sprintf(tempname, "ESP32_%04X", (uint16_t)(chipid >> 32));
#endif
		if (mqttClient.connect(tempname, _mqtt_user, _mqtt_pwd, topics_pub[0], 1, true, "offline"))
		{
			uint8_t m = sizeof(topics_sub) / sizeof(topics_sub[0]);
			PRNTL(F("~ MQTT server Connected"));
			PRNTL(F("\n~~ Subscribe Topics:"));
			PRNTL(F("±±±±±±±±±±±±±±±±±±±±±±±±±"));

			for (uint8_t i = 0; i < m; i++)
			{
				if (topics_sub[i] != nullptr)
				{
					mqttClient.subscribe(topics_sub[i]);
					PRNT(F("~ "));
					PRNTL(topics_sub[i]);
				}
			}
			PRNTL(F("±±±±±±±±±±±±±±±±±±±±±±±±±\n"));

			if (firstRun)
			{
				char msg[60];
				sprintf(msg, "<< PowerON Boot >> IP:[%d.%d.%d.%d] RSSI [%ddB]", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3], WiFi.RSSI());
				if (!ignore_boot_msg)
				{
					pub_log(msg);
				}
				firstRun = false;
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
	char incoming_msg[30];
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

	if (strcmp(incoming_msg, "ota") == 0)
	{
		sprintf(msg, "OTA allowed for %d seconds", OTA_upload_interval * MS2MINUTES / 1000);
		pub_msg(msg);
		allowOTA_clock = millis();
	}
	else if (strcmp(incoming_msg, "reset") == 0)
	{
		sendReset("MQTT");
	}
	else if (strcmp(incoming_msg, "ver") == 0)
	{
		sprintf(msg, "ver: IOTlib: [%s]", ver);
		pub_msg(msg);
	}
	else if (strcmp(incoming_msg, "services") == 0)
	{
		sprintf(msg, "Services: SERIAL[%d], no-networkReset[%d] [%d min], ignore_boot_msg[%d], useFlashP[%d]",
				useSerial, useNetworkReset, noNetwork_reset, ignore_boot_msg, useFlashP);
		pub_msg(msg);
	}
	else if (strcmp(incoming_msg, "help") == 0)
	{
		sprintf(msg, "Help: Commands #1 - [status, reset, ota, ver, ver2, help, help2, MCU_type, services, network]");
		pub_msg(msg);
		sprintf(msg, "Help: Commands #2 - [show_flashParam, free_mem, {update_flash,[key],[value]}]");
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
	else if (strcmp(incoming_msg, "show_flashParam") == 0)
	{
		if (useFlashP)
		{
			char clk[25];
			get_timeStamp(clk);
			sprintf(msg, "\n<<~~~~~~ [%s] [%s] On-Flash Parameters ~~~~~>>", clk, topics_sub[0]);
			pub_debug(msg);

			for (uint8_t i = 0; i < sizeof(parameter_filenames) / sizeof(parameter_filenames[0]); i++)
			{
				if (parameter_filenames[i] != nullptr)
				{
					pub_debug(parameter_filenames[i]);
					String tempstr1 = readFile(parameter_filenames[i]);
					Serial.print("STR:");
					Serial.println(tempstr1.length());
					char buff[tempstr1.length() + 1];
					tempstr1.toCharArray(buff, tempstr1.length() + 1);
					pub_debug(buff);
				}
			}
			pub_msg("[On-Flash Parameters]: extracted");
			pub_debug("<<~~~~~~~~~~ End ~~~~~~~~~~>>");
		}
		else
		{
			pub_msg("[On-Flash Parameters]: not in use");
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
	else if (strcmp(incoming_msg, "free_mem") == 0)
	{
		float rmem;
		unsigned long fmem = ESP.getFreeHeap();
		char result[10];
		char result1[10];

#ifdef ESP8266
		rmem = fmem / MAX_ESP8266_HEAP;
#endif
#ifdef ESP32
		rmem = 100 * (float)(fmem / (float)MAX_ESP32_HEAP);
#endif
		dtostrf(rmem, 6, 2, result);
		dtostrf(fmem / 1000.0, 6, 2, result1);
		sprintf(msg, "Heap: Remain [%skb] [%s%%]", result1, result);
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
void myIOT2::_pub_generic(const char *topic, const char *inmsg, bool retain, char *devname, bool bare)
{
	char clk[25];
	get_timeStamp(clk, 0);
	const uint8_t mqtt_overhead_size = 23;
	const int mqtt_defsize = mqttClient.getBufferSize();

	int x = devname == nullptr ? strlen(topics_sub[0]) + 2 : 0;
	// int totl_len = strlen(inmsg) + x /* devTopic */+ 8 /* clock*/+ mqtt_overhead_size /* mqtt_overh*/ + 10 /*spare*/;
	// char tmpmsg[strlen(inmsg) + x + 8 + 25]; // avoid dynamic allocation
	char tmpmsg[_maxMQTTmsglen];

	if (!bare)
	{
		snprintf(tmpmsg, _maxMQTTmsglen, "[%s] [%s] %s", clk, topics_sub[0], inmsg);
	}
	else
	{
		snprintf(tmpmsg, _maxMQTTmsglen, "%s", inmsg);
	}

	x = strlen(tmpmsg) + mqtt_overhead_size + strlen(topic); /* everything get into account when diff new buffer size*/
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
void myIOT2::pub_msg(const char *inmsg)
{
	_pub_generic(topics_gen_pub[0], inmsg);
}
void myIOT2::pub_noTopic(const char *inmsg, char *Topic, bool retain)
{
	_pub_generic(Topic, inmsg, retain, nullptr, true);
}
void myIOT2::pub_state(const char *inmsg, uint8_t i)
{
	if (i != 0)
	{
		int x = strlen(topics_pub[1]) + 3;
		char t[x];
		sprintf(t, "%s%d", topics_pub[1], i);
		mqttClient.publish(t, inmsg, true);
	}
	else
	{
		mqttClient.publish(topics_pub[1], inmsg, true);
	}
}
void myIOT2::pub_log(const char *inmsg)
{
	_pub_generic(topics_gen_pub[1], inmsg);
}
void myIOT2::pub_debug(const char *inmsg)
{
	_pub_generic(topics_gen_pub[2], inmsg, false, nullptr, true);
}

void myIOT2::notifyOnline()
{
	mqttClient.publish(topics_pub[0], "online", true);
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

// ~~~~~~~~~~ Data Storage ~~~~~~~~~
void myIOT2::set_pFilenames(const char *fileArray[], uint8_t asize)
{
	for (uint8_t i = 0; i < asize; i++)
	{
		parameter_filenames[i] = fileArray[i];
	}
}
bool myIOT2::extract_JSON_from_flash(const char *filename, JsonDocument &DOC)
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
	useSerial = DOC["useSerial"].as<bool>() | useSerial;
	useFlashP = DOC["useFlashP"].as<bool>() | useFlashP;
	useNetworkReset = DOC["useNetworkReset"].as<bool>() | useNetworkReset;
	noNetwork_reset = DOC["noNetwork_reset"].as<uint8_t>() | noNetwork_reset;
	ignore_boot_msg = DOC["ignore_boot_msg"].as<bool>() | ignore_boot_msg;
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
bool myIOT2::_saveFile(const char *filename, JsonDocument &DOC)
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
	DynamicJsonDocument myIOT_P(1250); //<------------------ FIX THIS -----------

	for (uint8_t n = 0; n < sizeof(parameter_filenames) / sizeof(parameter_filenames[0]); n++)
	{
		if (extract_JSON_from_flash(parameter_filenames[n], myIOT_P))
		{
			if (myIOT_P.containsKey(key))
			{
				if (_change_flashP_value(key, new_value, myIOT_P))
				{
					if (_saveFile(parameter_filenames[n], myIOT_P))
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

String myIOT2::readFile(const char *fileName)
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

// ~~~~~~ Reset and maintability ~~~~~~
void myIOT2::sendReset(const char *header)
{
	char temp[20 + strlen(header)];

	sprintf(temp, "[%s] - Reset sent", header);
	PRNTL(temp);

	if (header != nullptr)
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
	ArduinoOTA.setHostname(topics_sub[0]);
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

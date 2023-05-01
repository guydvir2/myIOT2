#include "myIOT2.h"

// // ~~~~~~ myIOT2 CLASS ~~~~~~~~~~~ //
// myIOT2::myIOT2() : mqttClient(espClient)
// {
// }
// void myIOT2::start_services(cb_func funct, const char *ssid, const char *password,
// 							const char *mqtt_user, const char *mqtt_passw, const char *mqtt_broker)
// {
// 	strcpy(_mqtt_server, mqtt_broker);
// 	strcpy(_mqtt_user, mqtt_user);
// 	strcpy(_mqtt_pwd, mqtt_passw);
// 	strcpy(_ssid, ssid);
// 	strcpy(_wifi_pwd, password);
// 	ext_mqtt = funct; // redirecting to ex-class function ( defined outside)

// 	if (useSerial)
// 	{
// 		Serial.begin(115200);
// 		PRNT(F("\n\n>>> ~~~~~~~ Start myIOT2 "));
// 		PRNT(ver);
// 		PRNTL(F(" ~~~~~~~ <<<"));
// 		delay(10);
// 	}

// 	_setMQTT();

// 	PRNTL(F("\n>>> Parameters <<<"));
// 	PRNT(F("useSerial:\t\t"));
// 	PRNTL(useSerial ? "Yes" : "No");

// 	PRNT(F("ignore_boot_msg:\t"));
// 	PRNTL(ignore_boot_msg ? "Yes" : "No");
// 	PRNT(F("useFlashP:\t\t"));
// 	PRNTL(useFlashP ? "Yes" : "No");
// 	PRNT(F("noNetwork_reset:\t"));
// 	PRNTL(noNetwork_reset);
// 	PRNT(F("ESP type:\t\t"));

// 	char a[10];
// #if defined(ESP8266)
// 	strcpy(a, "ESP8266");
// #elif defined(ESP32)
// 	strcpy(a, "ESP32");
// #endif
// 	PRNTL(a);
// }
// void myIOT2::looper()
// {
// 	if (_WiFi_handler())
// 	{
// 		return;
// 	}
// 	if (_MQTT_handler())
// 	{
// 		return;
// 	}
// }

// // ~~~~~~~ Wifi functions ~~~~~~~
// bool myIOT2::_WiFi_handler()
// {
// 	/* First boot loop - reset the wifi radio and schedule the wifi connection */
// 	static bool firstLoopCall = true;
// 	if (firstLoopCall)
// 	{
// 		WiFi.disconnect(true);
// 		_nextWifiConnectionAttemptMillis = millis() + 500;
// 		firstLoopCall = false;
// 		return true;
// 	}

// 	bool isWifiConnected = (WiFi.status() == WL_CONNECTED);

// 	/* After Wifi is connected for the first time - start connections sequence */
// 	if (isWifiConnected && !_wifiConnected /* updates after another succefull loop */)
// 	{
// 		_onWifiConnect();
// 		_connectingToWifi = false;
// 		_nextMqttConnectionAttemptMillis = millis() + 500; /* Better to have 500 millis delay */
// 	}

// 	/* Connection in progress - Wifi still not connected , timeout or wifi message */
// 	else if (_connectingToWifi)
// 	{
// 		if (WiFi.status() == WL_CONNECT_FAILED || millis() - _lastWifiConnectiomAttemptMillis >= _retryConnectWiFi * MS2MINUTES)
// 		{
// 			// Serial.printf("WiFi! Connection attempt failed, delay expired. (%fs). \n", millis() / 1000.0);
// 			PRNTL("WiFi! Connection attempt failed, delay expired.");
// 			WiFi.disconnect(true);

// 			_nextWifiConnectionAttemptMillis = millis() + 500;
// 			_connectingToWifi = false;
// 		}
// 	}

// 	/* Connection lost - detection first time */
// 	else if (!isWifiConnected && _wifiConnected)
// 	{
// 		_onWifiDisconnect();
// 		_nextWifiConnectionAttemptMillis = millis() + 500;
// 	}

// 	// All is good - Connected since at least one loop() call
// 	else if (isWifiConnected && _wifiConnected)
// 	{
// 		_acceptOTA();
// 	}

// 	// Disconnected since at least one loop() call
// 	// Then, if we handle the wifi reconnection process and the waiting delay has expired, we connect to wifi
// 	/* Wifi is not connected, timeout trying to init Wifi connected */
// 	else if (_nextWifiConnectionAttemptMillis > 0 && millis() >= _nextWifiConnectionAttemptMillis)
// 	{
// 		_startWifi(_ssid, _wifi_pwd);
// 		_connectingToWifi = true;
// 		_nextWifiConnectionAttemptMillis = 0;
// 		_lastWifiConnectiomAttemptMillis = millis();
// 	}

// 	if (isWifiConnected != _wifiConnected)
// 	{
// 		_wifiConnected = isWifiConnected;
// 		return true;
// 	}
// 	else
// 	{
// 		return false;
// 	}
// }
// void myIOT2::_onWifiConnect()
// {
// 	char b[50];
// 	sprintf(b, "\n ~Connected ,ip : %s", WiFi.localIP().toString().c_str());
// 	// Serial.printf("WiFi: Connected (%fs), ip : %s \n", millis() / 1000.0, WiFi.localIP().toString().c_str());
// 	PRNTL(b);
// 	if (!_NTP_updated())
// 	{
// 		_startNTP();
// 	}
// 	if (!_OTAloaded)
// 	{
// 		_startOTA();
// 		_OTAloaded = true;
// 	}
// }
// void myIOT2::_onWifiDisconnect()
// {
// 	PRNT(F("WiFi Lost connection "));
// 	PRNT(millis() / 1000);
// 	PRNTL(F("sec"));
// }
// void myIOT2::_startWifi(const char *ssid, const char *password)
// {
// 	PRNTL(F("\n>>> WiFi <<<"));
// 	PRNT(F("~ Connecting to "));
// 	PRNT(ssid);

// #if defined(ESP32)
// 	WiFi.useStaticBuffers(true);
// #endif
// 	WiFi.mode(WIFI_STA);
// 	WiFi.begin(ssid, password);
// }

// // ~~~~~~~ NTP & Clock  ~~~~~~~~
// bool myIOT2::_NTP_updated()
// {
// 	return now() > 1640803233;
// }
// bool myIOT2::_startNTP(const char *ntpServer, const char *ntpServer2)
// {
// 	const uint8_t _sec_to_NTP = 25;
// 	unsigned long startLoop = millis();
// #if defined(ESP8266)
// 	configTime(TZ_Asia_Jerusalem, ntpServer2, ntpServer); // configuring time offset and an NTP server
// #elif defined(ESP32)
// 	configTzTime(TZ_Asia_Jerusalem, ntpServer2, ntpServer);
// #endif
// 	PRNTL(F("\n>>> NTP <<<"));
// 	PRNT("~ NTP: ");
// 	while (!_NTP_updated() && (millis() - startLoop < _sec_to_NTP * 1000) && WiFi.isConnected()) /* ESP32 after software reset - doesnt enter here at all*/
// 	{
// 		delay(100);
// 		PRNT("*");
// 	}

// 	if (!_NTP_updated())
// 	{
// 		PRNTL(F("\n~ NTP Update fail"));
// 		return 0;
// 	}
// 	else
// 	{
// 		PRNTL("");
// 		PRNT(F("~ NTP Updated OK in "));
// 		PRNT((millis() - startLoop) * 0.001);
// 		PRNTL(F("sec"));
// 		return 1;
// 	}
// }
// void myIOT2::get_timeStamp(char ret[], time_t t)
// {
// 	if (t == 0)
// 	{
// 		t = now();
// 	}
// 	struct tm *tm = localtime(&t);
// 	sprintf(ret, "%04d-%02d-%02d %02d:%02d:%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
// }
// void myIOT2::convert_epoch2clock(long t1, long t2, char time_str[], char days_str[])
// {
// 	uint8_t days = 0;
// 	uint8_t hours = 0;
// 	uint8_t minutes = 0;
// 	uint8_t seconds = 0;

// 	const uint8_t sec2minutes = 60;
// 	const int sec2hours = (sec2minutes * 60);
// 	const int sec2days = (sec2hours * 24);

// 	long time_delta = t1 - t2;

// 	days = (int)(time_delta / sec2days);
// 	hours = (int)((time_delta - days * sec2days) / sec2hours);
// 	minutes = (int)((time_delta - days * sec2days - hours * sec2hours) / sec2minutes);
// 	seconds = (int)(time_delta - days * sec2days - hours * sec2hours - minutes * sec2minutes);
// 	if (days_str != nullptr)
// 	{
// 		sprintf(days_str, "%01dd", days);
// 	}
// 	sprintf(time_str, "%02d:%02d:%02d", hours, minutes, seconds);
// }
// time_t myIOT2::now()
// {
// 	return time(nullptr);
// }

// // ~~~~~~~ MQTT functions ~~~~~~~
// bool myIOT2::_MQTT_handler()
// {
// 	unsigned int _mqttReconnectionAttemptDelay = 15 * 1000;

// 	mqttClient.loop();
// 	bool isMqttConnected = (isWifiConnected() && mqttClient.connected());

// 	// Connection re-established
// 	if (isMqttConnected && !_mqttConnected)
// 	{
// 		_mqttConnected = true;
// 		_subMQTT();
// 		// notifyOnline();
// 	}

// 	// Connection lost
// 	else if (!isMqttConnected && _mqttConnected)
// 	{
// 		// sprintf(b, "MQTT! Lost connection (%fs). \n", millis() / 1000.0);
// 		// PRNTL(F(b));
// 		// Serial.printf("MQTT: Retrying to connect in %i seconds. \n", _mqttReconnectionAttemptDelay / 1000);
// 		PRNTL(F("MQTT: Retrying to connect"));
// 		_nextMqttConnectionAttemptMillis = millis() + _mqttReconnectionAttemptDelay;
// 	}

// 	// It's time to re-connect to the MQTT broker
// 	else if (isWifiConnected() && _nextMqttConnectionAttemptMillis > 0 && millis() >= _nextMqttConnectionAttemptMillis)
// 	{
// 		char b[50];
// 		// Connect to MQTT broker
// 		if (_connectMQTT())
// 		{
// 			notifyOnline();
// 			if (!_firstRun)
// 			{
// 				sprintf(b, "MQTT: reconnect after [%d] retries", _failedMQTTConnectionAttemptCount);
// 				PRNTL(b);
// 				pub_log(b);
// 			}
// 			_failedMQTTConnectionAttemptCount = 0;
// 			_nextMqttConnectionAttemptMillis = 0;
// 		}
// 		else
// 		{
// 			_nextMqttConnectionAttemptMillis = millis() + _mqttReconnectionAttemptDelay;
// 			mqttClient.disconnect();
// 			_failedMQTTConnectionAttemptCount++;
// 			sprintf(b, "MQTT!: Failed MQTT connection count: %d", _failedMQTTConnectionAttemptCount);
// 			PRNTL(b);

// 			// When there is too many failed attempt, sometimes it help to reset the WiFi connection or to restart the board.
// 			if (_failedMQTTConnectionAttemptCount == 8)
// 			{
// 				PRNTL(F("MQTT: Can't connect to broker after too many attempt, resetting WiFi ..."));
// 				WiFi.disconnect(true);
// 				_nextWifiConnectionAttemptMillis = millis() + 500;

// 				if (!(noNetwork_reset > 0))
// 				{
// 					_failedMQTTConnectionAttemptCount = 0;
// 				}
// 			}
// 			else if (noNetwork_reset > 0 && _failedMQTTConnectionAttemptCount == 12) // Will reset after 12 failed attempt (3 minutes of retry)
// 			{
// 				PRNTL(F("MQTT!: Can't connect to broker after too many attempt, resetting board ..."));
// 				sendReset("MQTT failure");
// 			}
// 		}
// 	}

// 	if (_mqttConnected != isMqttConnected)
// 	{
// 		_mqttConnected = isMqttConnected;
// 		return true;
// 	}
// 	else
// 	{
// 		return false;
// 	}
// }
// void myIOT2::_setMQTT()
// {
// 	mqttClient.setServer(_mqtt_server, 1883);
// 	mqttClient.setKeepAlive(30);
// 	mqttClient.setCallback(std::bind(&myIOT2::_MQTTcb, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
// }
// bool myIOT2::_connectMQTT()
// {
// 	char tempname[20];
// #if defined(ESP8266)
// 	sprintf(tempname, "ESP_%s", String(ESP.getChipId()).c_str());
// #elif defined(ESP32)
// 	uint64_t chipid = ESP.getEfuseMac();
// 	sprintf(tempname, "ESP32_%04X", (uint16_t)(chipid >> 32));
// #endif
// 	PRNTL(F("\n>>> MQTT <<<"));
// 	if (mqttClient.connect(tempname, _mqtt_user, _mqtt_pwd, topics_pub[0], 1, true, "offline"))
// 	{
// 		PRNT(F("~ MQTT Server: "));
// 		PRNTL(_mqtt_server);
// 		return 1;
// 	}
// 	else
// 	{
// 		PRNT(F("~ Connection failed rc="));
// 		PRNTL(mqttClient.state());
// 		return 0;
// 	}
// }
// void myIOT2::_subMQTT()
// {
// 	uint8_t m = sizeof(topics_sub) / sizeof(topics_sub[0]);
// 	PRNTL(F("~ MQTT server Connected"));
// 	PRNTL(F("\n~~ Subscribe Topics:"));
// 	PRNTL(F("±±±±±±±±±±±±±±±±±±±±±±±±±"));

// 	for (uint8_t i = 0; i < m; i++)
// 	{
// 		if (topics_sub[i] != nullptr)
// 		{
// 			mqttClient.subscribe(topics_sub[i]);
// 			PRNT(F("~ "));
// 			PRNTL(topics_sub[i]);
// 		}
// 	}
// 	PRNTL(F("±±±±±±±±±±±±±±±±±±±±±±±±±\n"));

// 	if (_firstRun)
// 	{
// 		PRNTL(F(">>> ~~~~~~~ END iot2 ~~~~~~~ <<<"));
// 		char msg[60];
// 		sprintf(msg, "<< PowerON Boot >> IP:[%d.%d.%d.%d] RSSI [%ddB]", WiFi.localIP()[0],
// 				WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3], WiFi.RSSI());
// 		if (!ignore_boot_msg)
// 		{
// 			pub_log(msg);
// 		}
// 		_firstRun = false;
// 	}
// }
// void myIOT2::_concate(const char *array[], char outmsg[])
// {
// 	uint8_t i = 0;
// 	while (array[i] != nullptr)
// 	{
// 		if (i > 0)
// 		{
// 			strcat(outmsg, "; ");
// 		}
// 		strcat(outmsg, array[i]);
// 		i++;
// 	}
// 	return;
// }

// void myIOT2::_MQTTcb(char *topic, uint8_t *payload, unsigned int length)
// {
// 	char incoming_msg[30];
// 	char msg[200];

// 	PRNT(F("Message arrived ["));
// 	PRNT(topic);
// 	PRNT(F("] "));

// 	for (unsigned int i = 0; i < length; i++)
// 	{
// 		PRNT((char)payload[i]);
// 		incoming_msg[i] = (char)payload[i];
// 	}
// 	incoming_msg[length] = 0;
// 	PRNTL("");

// 	if (strcmp(incoming_msg, "ota") == 0)
// 	{
// 		sprintf(msg, "OTA allowed for %d seconds", OTA_upload_interval * MS2MINUTES / 1000);
// 		pub_msg(msg);
// 		allowOTA_clock = millis();
// 	}
// 	else if (strcmp(incoming_msg, "reset") == 0)
// 	{
// 		sendReset("MQTT");
// 	}
// 	else if (strcmp(incoming_msg, "ver") == 0)
// 	{
// 		sprintf(msg, "ver: IOTlib: [%s]", ver);
// 		pub_msg(msg);
// 	}
// 	else if (strcmp(incoming_msg, "services") == 0)
// 	{
// 		sprintf(msg, "Services: SERIAL[%d], no-networkReset[%d min], ignore_boot_msg[%d], useFlashP[%d]",
// 				useSerial, noNetwork_reset, ignore_boot_msg, useFlashP);
// 		pub_msg(msg);
// 	}
// 	else if (strcmp(incoming_msg, "help") == 0)
// 	{
// 		sprintf(msg, "Help: Commands #1 - [status, reset, ota, ver, ver2, help, help2, MCU_type, services, network]");
// 		pub_msg(msg);
// 		sprintf(msg, "Help: Commands #2 - [show_flashParam, free_mem, {update_flash,[key],[value]}]");
// 		pub_msg(msg);
// 	}
// 	else if (strcmp(incoming_msg, "MCU_type") == 0)
// 	{
// #if defined(ESP8266)
// 		sprintf(msg, "[MCU]: ESP8266");
// #elif defined(ESP32)
// 		sprintf(msg, "[MCU]: ESP32");
// #else
// 		sprintf(msg, "[MCU]: unKnown");
// #endif
// 		pub_msg(msg);
// 	}
// 	else if (strcmp(incoming_msg, "show_flashParam") == 0)
// 	{
// 		// if (useFlashP)
// 		// {
// 		char clk[25];
// 		get_timeStamp(clk);
// 		sprintf(msg, "\n<<~~~~~~ [%s] [%s] On-Flash Parameters ~~~~~>>", clk, topics_sub[0]);
// 		pub_debug(msg);

// 		for (uint8_t i = 0; i < sizeof(parameter_filenames) / sizeof(parameter_filenames[0]); i++)
// 		{
// 			myJflash jf(useSerial);
// 			if (parameter_filenames[i] != nullptr)
// 			{
// 				jf.set_filename(parameter_filenames[i]);
// 				pub_debug(parameter_filenames[i]);
// 				String tempstr1 = jf.readFile2String(parameter_filenames[i]);
// 				// Serial.print("STR:");
// 				// Serial.println(tempstr1);
// 				char buff[tempstr1.length() + 1];
// 				tempstr1.toCharArray(buff, tempstr1.length() + 1);
// 				pub_debug(buff);
// 			}
// 		}
// 		pub_msg("[On-Flash Parameters]: extracted");
// 		pub_debug("<<~~~~~~~~~~ End ~~~~~~~~~~>>");
// 		// }
// 		// else
// 		// {
// 		// 	pub_msg("[On-Flash Parameters]: not in use");
// 		// }
// 	}
// 	else if (strcmp(incoming_msg, "network") == 0)
// 	{
// 		char IPadd[16];
// 		char days[10];
// 		char clock[25];

// 		unsigned long t = (long)(millis() / 1000);
// 		convert_epoch2clock(t, 0, clock, days);
// 		sprintf(IPadd, "%d.%d.%d.%d", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);
// 		sprintf(msg, "Network: uptime[%s %s], localIP[%s], MQTTserver[%s],  RSSI: [%d dBm]", days, clock, IPadd, _mqtt_server, WiFi.RSSI());

// 		pub_msg(msg);
// 	}
// 	else if (strcmp(incoming_msg, "free_mem") == 0)
// 	{
// 		float rmem;
// 		unsigned long fmem = ESP.getFreeHeap();
// 		char result[10];
// 		char result1[10];

// #ifdef ESP8266
// 		rmem =100* (float)(fmem / (float)MAX_ESP8266_HEAP);
// #endif
// #ifdef ESP32
// 		rmem = 100 * (float)(fmem / (float)MAX_ESP32_HEAP);
// #endif
// 		Serial.println(rmem);
// 		Serial.println(MAX_ESP8266_HEAP);
// 		Serial.println(fmem);
// 		dtostrf(rmem, 6, 2, result);
// 		dtostrf(fmem / 1000.0, 6, 2, result1);
// 		sprintf(msg, "Heap: Remain [%skb] [%s%%]", result1, result);
// 		pub_msg(msg);
// 	}
// 	else if (strcmp(incoming_msg, "topics") == 0)
// 	{
// 		char A[100];

// 		strcpy(A, "");
// 		_concate(topics_pub, A);
// 		sprintf(msg, "[Pub_Topics]: %s", A);
// 		pub_msg(msg);

// 		strcpy(A, "");
// 		_concate(topics_sub, A);
// 		sprintf(msg, "[Sub_Topics]: %s", A);
// 		pub_msg(msg);

// 		strcpy(A, "");
// 		_concate(topics_gen_pub, A);
// 		sprintf(msg, "[gen-Pub_Topics]: %s", A);
// 		pub_msg(msg);
// 	}
// 	else
// 	{
// 		num_p = inline_read(incoming_msg);

// 		if (num_p > 1 && strcmp(inline_param[0], "update_flash") == 0 && useFlashP)
// 		{
// 			_cmdline_flashUpdate(inline_param[1], inline_param[2]);
// 		}
// 		else
// 		{
// 			ext_mqtt(incoming_msg, topic);
// 		}
// 	}
// }
// void myIOT2::_pub_generic(const char *topic, const char *inmsg, bool retain, char *devname, bool bare)
// {
// 	char clk[25];
// 	get_timeStamp(clk, 0);
// 	const int _maxMQTTmsglen = 550;
// 	const uint8_t mqtt_overhead_size = 23;
// 	const int mqtt_defsize = mqttClient.getBufferSize();

// 	int x = devname == nullptr ? strlen(topics_sub[0]) + 2 : 0;
// 	// int totl_len = strlen(inmsg) + x /* devTopic */+ 8 /* clock*/+ mqtt_overhead_size /* mqtt_overh*/ + 10 /*spare*/;
// 	// char tmpmsg[strlen(inmsg) + x + 8 + 25]; // avoid dynamic allocation
// 	char tmpmsg[_maxMQTTmsglen];

// 	if (!bare)
// 	{
// 		snprintf(tmpmsg, _maxMQTTmsglen, "[%s] [%s] %s", clk, topics_sub[0], inmsg);
// 	}
// 	else
// 	{
// 		snprintf(tmpmsg, _maxMQTTmsglen, "%s", inmsg);
// 	}

// 	x = strlen(tmpmsg) + mqtt_overhead_size + strlen(topic); /* everything get into account when diff new buffer size*/
// 	if (x > mqtt_defsize)
// 	{
// 		mqttClient.setBufferSize(x);
// 		mqttClient.publish(topic, tmpmsg, retain);
// 		mqttClient.setBufferSize(mqtt_defsize);
// 	}
// 	else
// 	{
// 		mqttClient.publish(topic, tmpmsg, retain);
// 	}
// }
// void myIOT2::pub_msg(const char *inmsg)
// {
// 	_pub_generic(topics_gen_pub[0], inmsg);
// }
// void myIOT2::pub_noTopic(const char *inmsg, char *Topic, bool retain)
// {
// 	_pub_generic(Topic, inmsg, retain, nullptr, true);
// }
// void myIOT2::pub_state(const char *inmsg, uint8_t i)
// {
// 	if (i != 0)
// 	{
// 		int x = strlen(topics_pub[1]) + 3;
// 		char t[x];
// 		sprintf(t, "%s%d", topics_pub[1], i);
// 		mqttClient.publish(t, inmsg, true);
// 	}
// 	else
// 	{
// 		mqttClient.publish(topics_pub[1], inmsg, true);
// 	}
// }
// void myIOT2::pub_log(const char *inmsg)
// {
// 	_pub_generic(topics_gen_pub[1], inmsg);
// }
// void myIOT2::pub_debug(const char *inmsg)
// {
// 	_pub_generic(topics_gen_pub[2], inmsg, false, nullptr, true);
// }

// void myIOT2::notifyOnline()
// {
// 	mqttClient.publish(topics_pub[0], "online", true);
// }
// void myIOT2::clear_inline_read()
// {
// 	for (uint8_t i = 0; i < num_p; i++)
// 	{
// 		strcpy(inline_param[i], "");
// 	}
// }
// uint8_t myIOT2::inline_read(char *inputstr)
// {
// 	char *pch;
// 	int i = 0;

// 	pch = strtok(inputstr, " ,.-");
// 	while (pch != NULL && i < num_param)
// 	{
// 		sprintf(inline_param[i], "%s", pch);
// 		pch = strtok(NULL, " ,.-");
// 		i++;
// 	}
// 	return i;
// }
// uint8_t myIOT2::_getdataType(const char *y)
// {
// 	/* Return values:
// 	0 - error
// 	1 - bool
// 	2 - string
// 	3 - float
// 	4 - int
// 	*/

// 	int i = atoi(y);
// 	float f = atof(y);

// 	if (isAlpha(y[0]))
// 	{
// 		if (strcmp(y, "true") == 0 || strcmp(y, "false") == 0)
// 		{
// 			return 1;
// 		}
// 		else
// 		{
// 			return 2;
// 		}
// 		return 0;
// 	}
// 	else
// 	{
// 		if (i != f)
// 		{
// 			return 3;
// 		}
// 		else
// 		{
// 			return 4;
// 		}
// 		return 0;
// 	}
// }

// // ~~~~~~~~~~ Data Storage ~~~~~~~~~
// void myIOT2::set_pFilenames(const char *fileArray[], uint8_t asize)
// {
// 	for (uint8_t i = 0; i < asize; i++)
// 	{
// 		parameter_filenames[i] = fileArray[i];
// 	}
// }
// bool myIOT2::readJson_inFlash(JsonDocument &DOC, const char *filename)
// {
// 	myJflash Jflash(useSerial);
// 	return (Jflash.readFile(DOC, filename));
// }
// bool myIOT2::readFlashParameters(JsonDocument &DOC, const char *filename)
// {
// 	if (readJson_inFlash(DOC, filename))
// 	{
// 		useSerial = DOC["useSerial"];
// 		useFlashP = true;
// 		noNetwork_reset = DOC["noNetwork_reset"];
// 		ignore_boot_msg = DOC["ignore_boot_msg"];
// 		PRNTL(F("~ iot2 Parameters loaded from Flash"));
// 		return 1;
// 	}
// 	else
// 	{
// 		useSerial = true;
// 		useFlashP = false;
// 		noNetwork_reset = 9;
// 		ignore_boot_msg = false;
// 		PRNTL(F("~ iot2 Parameters failed to load from Flash. Defaults are used"));
// 		return 0;
// 	}
// }

// bool myIOT2::_change_flashP_value(const char *key, const char *new_value, JsonDocument &DOC)
// {
// 	uint8_t s = _getdataType(new_value);
// 	if (s == 1)
// 	{
// 		if (strcmp(new_value, "true") == 0)
// 		{
// 			DOC[key] = true;
// 		}
// 		else
// 		{
// 			DOC[key] = false;
// 		}
// 		return 1;
// 	}
// 	else if (s == 2)
// 	{
// 		DOC[key] = new_value;
// 		return 1;
// 	}
// 	else if (s == 3)
// 	{
// 		DOC[key] = (float)atof(new_value);
// 		return 1;
// 	}
// 	else if (s == 4)
// 	{
// 		DOC[key] = (int)atoi(new_value);
// 		return 1;
// 	}
// 	return 0;
// }
// bool myIOT2::_cmdline_flashUpdate(const char *key, const char *new_value)
// {
// 	char msg[100];
// 	bool succ_chg = false;
// 	DynamicJsonDocument myIOT_P(1250); //<------------------ FIX THIS -----------

// 	myJflash Jflash(useSerial);
// 	for (uint8_t n = 0; n < sizeof(parameter_filenames) / sizeof(parameter_filenames[0]); n++)
// 	{
// 		if (Jflash.readFile(myIOT_P, parameter_filenames[n]))
// 		{
// 			if (myIOT_P.containsKey(key))
// 			{
// 				if (_change_flashP_value(key, new_value, myIOT_P))
// 				{
// 					if (Jflash.writeFile(myIOT_P, parameter_filenames[n]))
// 					{
// 						succ_chg = true;
// 						sprintf(msg, "[Flash]: parameter[%s] updated to[%s] [OK]", key, new_value);
// 					}
// 					else
// 					{
// 						succ_chg = false;
// 						sprintf(msg, "[Flash]: parameter[%s] [Failed] to updated. Save file error", key);
// 					}
// 				}
// 				else
// 				{
// 					succ_chg = false;
// 					sprintf(msg, "[Flash]: parameter[%s] replace [Failed]", key);
// 				}
// 				pub_msg(msg);
// 			}
// 		}
// 		else if (n == 1)
// 		{
// 			succ_chg = false;
// 			sprintf(msg, "[Flash]: parameter[%s] [NOT FOUND]", key);
// 			pub_msg(msg);
// 		}
// 	}
// 	return succ_chg;
// }

// // ~~~~~~ Reset and maintability ~~~~~~
// void myIOT2::sendReset(const char *header)
// {
// 	char temp[20 + strlen(header)];

// 	sprintf(temp, "[%s] - Reset sent", header);
// 	PRNTL(temp);

// 	if (header != nullptr)
// 	{
// 		pub_msg(temp);
// 	}
// 	delay(1000);
// #if defined(ESP8266)
// 	ESP.reset();
// #elif defined(ESP32)
// 	ESP.restart();
// #endif
// }
// void myIOT2::_acceptOTA()
// {
// 	if (millis() - allowOTA_clock <= OTA_upload_interval * MS2MINUTES)
// 	{
// 		ArduinoOTA.handle();
// 	}
// }
// void myIOT2::_startOTA()
// {
// 	allowOTA_clock = millis();
// 	ArduinoOTA.setPort(8266);
// 	ArduinoOTA.setHostname(topics_sub[0]);
// 	ArduinoOTA.onStart([]()
// 					   {
// 						   String type;
// 						   if (ArduinoOTA.getCommand() == U_FLASH)
// 						   {
// 							   type = "sketch";
// 						   }
// 						   else
// 						   {
// 							   type = "filesystem";
// 						   } });
// 	if (useSerial)
// 	{
// 		ArduinoOTA.onEnd([]()
// 						 { Serial.println("\nEnd"); });
// 		ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
// 							  { Serial.printf("Progress: %u%%\r", (progress / (total / 100))); });
// 		ArduinoOTA.onError([](ota_error_t error)
// 						   {
// 							   Serial.printf("Error[%u]: ", error);
// 							   if (error == OTA_AUTH_ERROR)
// 							   {
// 								   Serial.println("Auth Failed");
// 							   }
// 							   else if (error == OTA_BEGIN_ERROR)
// 							   {
// 								   Serial.println("Begin Failed");
// 							   }
// 							   else if (error == OTA_CONNECT_ERROR)
// 							   {
// 								   Serial.println("Connect Failed");
// 							   }
// 							   else if (error == OTA_RECEIVE_ERROR)
// 							   {
// 								   Serial.println("Receive Failed");
// 							   }
// 							   else if (error == OTA_END_ERROR)
// 							   {
// 								   Serial.println("End Failed");
// 							   } });
// 	}

// 	ArduinoOTA.begin();
// }

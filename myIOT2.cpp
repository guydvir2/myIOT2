#include "Arduino.h"
#include "myIOT2.h"

#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266Ping.h>
#endif

// ~~~~~~ myIOT2 CLASS ~~~~~~~~~~~ //
myIOT2::myIOT2() : mqttClient(espClient), flog("/myIOTlog.txt")
{
}
void myIOT2::start_services(cb_func funct, char *ssid, char *password, char *mqtt_user, char *mqtt_passw, char *mqtt_broker, int log_ents, int log_len)
{
	_mqtt_server = mqtt_broker;
	_mqtt_user = mqtt_user;
	_mqtt_pwd = mqtt_passw;
	_ssid = ssid;
	_wifi_pwd = password;
	ext_mqtt = funct; //redirecting to ex-class function ( defined outside)
	extDefine = true; // maing sure this ext_func was defined

	if (useSerial)
	{
		Serial.begin(115200);
		delay(10);
	}
	if (useDebug)
	{
		flog.start(log_ents, log_len);
	}
	start_network_services();
	if (useWDT)
	{
		startWDT();
	}
	if (useOTA)
	{
		startOTA();
	}
	if (useNetworkReset)
	{
		time2Reset_noNetwork = (1000 * 60L) * noNetwork_reset;
	}
	start_EEPROM_eADR();

	if (useBootClockLog && WiFi.isConnected())
	{
		get_bootclockLOG(0);
		update_bootclockLOG();
	}
}
void myIOT2::looper()
{
	wdtResetCounter = 0; //reset WDT watchDog
	if (useOTA)
	{
		acceptOTA();
	}
	if (network_looper() == 0)
	{
		if (noNetwork_Clock > 0 && useNetworkReset)
		{ // no Wifi or no MQTT will cause a reset
			if (millis() - noNetwork_Clock > time2Reset_noNetwork)
			{
				sendReset("NO NETWoRK");
			}
		}
	}
	if (useAltermqttServer == true)
	{
		if (millis() > time2Reset_noNetwork)
		{
			sendReset("Reset- restore main MQTT server");
		}
	}
	if (useDebug)
	{
		flog.looper(60);
	}
}

// ~~~~~~~ Wifi functions ~~~~~~~
bool myIOT2::startWifi(char *ssid, char *password)
{
	long startWifiConnection = millis();

	if (useSerial)
	{
		Serial.println();
		Serial.print("Connecting to ");
		Serial.println(ssid);
	}
	WiFi.mode(WIFI_OFF); // <---- NEW
	WiFi.mode(WIFI_STA);
	WiFi.disconnect();
	WiFi.begin(ssid, password);
	WiFi.setAutoReconnect(true); // <-- BACK

	// in case of reboot - timeOUT to wifi
	while (WiFi.status() != WL_CONNECTED && millis() < WIFItimeOut + startWifiConnection)
	{
		delay(200);
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
		}
		if (noNetwork_Clock == 0)
		{
			noNetwork_Clock = millis();
		}
		WiFi.disconnect(true); // <--- NEW in case of stuck wifi
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
		return 1;
	}
}
void myIOT2::start_network_services()
{
	if (startWifi(_ssid, _wifi_pwd))
	{
		start_clock();
		startMQTT();
	}
}
bool myIOT2::network_looper()
{
	if (WiFi.status() == WL_CONNECTED)
	{ // wifi is ok
		if (mqttClient.connected())
		{ // mqtt is good
			mqttClient.loop();
			noNetwork_Clock = 0;
			return 1;
		}
		else
		{ // not connected mqtt
			if (subscribeMQTT())
			{ // succeed to reconnect
				mqttClient.loop();
				noNetwork_Clock = 0;
				return 1;
			}
			else
			{ // failed to reconnect
				if (noNetwork_Clock == 0)
				{ // first time fail MQTT
					noNetwork_Clock = millis();
				}

				return 0;
			}
		}
	}
	else
	{ // no WIFI
		if (millis() > noNetwork_Clock + retryConnectWiFi)
		{
			if (!startWifi(_ssid, _wifi_pwd))
			{ // failed to reconnect ?
				if (noNetwork_Clock == 0)
				{ // first time when NO NETWORK ?
					noNetwork_Clock = millis();
					return 0;
				}
			}
			else
			{ // reconnect succeeded
				noNetwork_Clock = 0;
				return 1;
			}
		}
		else
		{
			return 0;
		}
	}
}

// ~~~~~~~ NTP & Clock ESP8266 ~~~~~~~~
void myIOT2::start_clock()
{
	int failcount;
#if isESP8266
	if (startNTP())
	{ //NTP Succeed
		get_timeStamp();
		strcpy(bootTime, timeStamp);
		failcount = (int)EEPROMReadlong(NTP_eADR);
		if (failcount != 0)
		{
			EEPROMWritelong(NTP_eADR, 0);
		}
		// else
		// {
		// 	EEPROMWritelong(NTP_eADR, 0);
		// }
	}
	else
	{
		if (resetFailNTP)
		{
			failcount = (int)EEPROMReadlong(NTP_eADR);
			if (failcount > 3)
			{
				sendReset("NTP");
			}
			else
			{
				EEPROMWritelong(NTP_eADR, failcount + 1);
				NTP_OK = false;
			}
		}
	}
#elif isESP32
	startNTP_32();
	getTimeStamp_32(bootTime);
	if (year(epoch_time) != 1970)
	{
		NTP_OK = true;
	}
	else
	{
		NTP_OK = false;
	}

#endif
}
bool myIOT2::startNTP()
{
#if isESP8266
	byte x = 0;
	byte retries = 5;
	int delay_tries = 300;
	char *NTPserver = "pool.ntp.org";

	NTP.begin(NTPserver, 2, true);
	delay(delay_tries);
	time_t t = now();

	while (x < retries && year(t) == 1970)
	{
		NTP.begin(NTPserver, 2, true);
		delay(delay_tries * (1.2 * (x + 1)));
		t = now();
		x += 1;
	}
	if (x < retries && year(t) != 1970)
	{
		NTP.setInterval(5, clockUpdateInt);
		NTP_OK = true;
		return 1;
	}

#endif
}
void myIOT2::get_timeStamp(time_t t)
{
#if isESP8266
	if (t == 0)
	{
		t = now();
	}
	sprintf(timeStamp, "%02d-%02d-%02d %02d:%02d:%02d", year(t), month(t), day(t), hour(t), minute(t), second(t));
#elif isESP32
	getTimeStamp_32(timeStamp);
#endif
}
void myIOT2::return_clock(char ret_tuple[20])
{
	// ESP8266 only
	time_t t = now();
	sprintf(ret_tuple, "%02d:%02d:%02d", hour(t), minute(t), second(t));
}
void myIOT2::return_date(char ret_tuple[20])
{
	// ESP8266 only
	time_t t = now();
	sprintf(ret_tuple, "%02d-%02d-%02d", year(t), month(t), day(t));
}
bool myIOT2::checkInternet(char *externalSite, byte pings)
{
	return Ping.ping(externalSite, pings);
}
void myIOT2::convert_epoch2clock(long t1, long t2, char *time_str, char *days_str)
{
	byte days = 0;
	byte hours = 0;
	byte minutes = 0;
	byte seconds = 0;

	int sec2minutes = 60;
	int sec2hours = (sec2minutes * 60);
	int sec2days = (sec2hours * 24);
	int sec2years = (sec2days * 365);

	long time_delta = t1 - t2;

	days = (int)(time_delta / sec2days);
	hours = (int)((time_delta - days * sec2days) / sec2hours);
	minutes = (int)((time_delta - days * sec2days - hours * sec2hours) / sec2minutes);
	seconds = (int)(time_delta - days * sec2days - hours * sec2hours - minutes * sec2minutes);

	sprintf(days_str, "%02d days", days);
	sprintf(time_str, "%02d:%02d:%02d", hours, minutes, seconds);
}
// ~~~~~~~ NTP & Clock ESP32 ~~~~~~~~
void myIOT2::startNTP_32(const int gmtOffset_sec, const int daylightOffset_sec, const char *ntpServer)
{
#if isESP32
	configTime(gmtOffset_sec, daylightOffset_sec, ntpServer); //configuring time offset and an NTP server
#endif
}
void myIOT2::getTime_32()
{
#if isESP32
	// getting time in ESP32 sometimes failed due to clock race. some delays were used as commented out here
	// int a = 0;
	// while (a < 3)
	// {
	if (getLocalTime(&timeinfo))
	{
		delay(100);
		time(&epoch_time); // update system clock
						   // }
						   // a++;
	}
#endif
}
void myIOT2::getTimeStamp_32(char ret_timeStamp[25])
{
	getTime_32();
	createDateStamp_32(&timeinfo, ret_timeStamp);
}
struct tm *myIOT2::convEpoch_32(time_t in_time)
{
	struct tm *convTime = localtime(&in_time); //gmtime
	return convTime;
}
void myIOT2::createDateStamp_32(struct tm *t, char retChar[30])
{
	sprintf(retChar, "%04d-%02d-%02d %02d:%02d:%02d", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
}

// ~~~~~~~ MQTT functions ~~~~~~~
void myIOT2::startMQTT()
{
	bool stat = false;
	createTopics();
	// Select MQTT server
	if (Ping.ping(_mqtt_server, 2))
	{
		mqttClient.setServer(_mqtt_server, 1883);
		stat = true;
		useAltermqttServer = false;
		if (useSerial)
		{
			Serial.print("MQTT SERVER: ");
			Serial.println(_mqtt_server);
		}
	}
	else if (Ping.ping(_mqtt_server2), 5)
	{
		mqttClient.setServer(_mqtt_server2, 1883);
		if (useSerial)
		{
			Serial.println("Connected to MQTT SERVER2: ");
			Serial.println(_mqtt_server2);
		}
		useAltermqttServer = true;
		stat = true;
	}
	else
	{
		mqttClient.setServer(MQTT_SERVER3, 1883);
		if (useSerial)
		{
			Serial.println("Connected to EXTERNAL MQTT SERVER: ");
			Serial.println(MQTT_SERVER3);
		}
		useAltermqttServer = true;
		stat = true;
	}
	// Set callback function
	if (stat)
	{
		mqttClient.setCallback(std::bind(&myIOT2::callback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
		subscribeMQTT();
	}
	else
	{
		if (useSerial)
		{
			Serial.println("Not connected to MQTT server");
		}
	}
}
bool myIOT2::subscribeMQTT()
{
	if (!mqttClient.connected())
	{
		long now = millis();
		if (noNetwork_Clock > 0 && now - noNetwork_Clock > 60000L)
		{
			return 0;
		}
		else if (now - lastReconnectAttempt > 2000)
		{
			lastReconnectAttempt = now;
			if (useSerial)
			{
				Serial.println("have wifi, entering MQTT connection");
				Serial.print("Attempting MQTT connection...");
			}
			// Attempt to connect
			char tempname[15];
#if isESP8266
			sprintf(tempname, "ESP_%s", String(ESP.getChipId()).c_str());
#elif isESP32
			uint64_t chipid = ESP.getEfuseMac();
			sprintf(tempname, "ESP32_%04X", (uint16_t)(chipid >> 32));
#endif
			if (mqttClient.connect(tempname, _mqtt_user, _mqtt_pwd, _availTopic, 0, true, "offline"))
			{
				// Connecting sequence
				for (int i = 0; i < sizeof(topicArry) / sizeof(char *); i++)
				{
					if (strcmp(topicArry[i], "") != 0)
					{
						mqttClient.subscribe(topicArry[i]);
					}
				}
				if (useextTopic)
				{
					uint8_t x = sizeof(extTopic) / sizeof(char *);
					for (int i = 0; i < x; i++)
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
					pub_log("<< PowerON Boot >>");
					if (!useResetKeeper)
					{
						firstRun = false;
						mqtt_detect_reset = 0;
						notifyOnline();
					}
					// else
					// { // using reset keeper
					// 	mqttClient.publish(_availTopic, "resetKeeper", true);
					// }
				}
				else
				{ // not first run
					notifyOnline();
				}
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
			return 0;
		}
	}
	else
	{
		return 1;
	}
}
void myIOT2::createTopics()
{
	snprintf(_msgTopic, MaxTopicLength, "%s/Messages", prefixTopic);
	snprintf(_groupTopic, MaxTopicLength, "%s/All", prefixTopic);
	snprintf(_logTopic, MaxTopicLength, "%s/log", prefixTopic);
	snprintf(_signalTopic, MaxTopicLength, "%s/Signal", prefixTopic);
	snprintf(_debugTopic, MaxTopicLength, "%s/debug", prefixTopic);
	snprintf(_smsTopic, MaxTopicLength, "%s/sms", prefixTopic);
	snprintf(_emailTopic, MaxTopicLength, "%s/email", prefixTopic);

	if (strcmp(addGroupTopic, "") != 0)
	{
		char temptopic[MaxTopicLength];
		strcpy(temptopic, addGroupTopic);
		snprintf(addGroupTopic, MaxTopicLength, "%s/%s", prefixTopic,
				 temptopic);

		snprintf(_deviceName, MaxTopicLength, "%s/%s", addGroupTopic,
				 deviceTopic);
	}
	else
	{
		snprintf(_deviceName, MaxTopicLength, "%s/%s", prefixTopic,
				 deviceTopic);
	}

	snprintf(_stateTopic, MaxTopicLength, "%s/State", _deviceName);
	snprintf(_stateTopic2, MaxTopicLength, "%s/State_2", _deviceName);
	snprintf(_availTopic, MaxTopicLength, "%s/Avail", _deviceName);
}
void myIOT2::callback(char *topic, byte *payload, unsigned int length)
{
	char incoming_msg[max_mqtt_msg];
	char msg[100];

	if (useSerial)
	{
		Serial.print("Message arrived [");
		Serial.print(topic);
		Serial.print("] ");
	}
	for (int i = 0; i < length; i++)
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
		uint8_t x = sizeof(extTopic) / sizeof(char *);
		for (int i = 0; i < x; i++)
		{
			if (extTopic[i] != nullptr && strcmp(extTopic[i], topic) == 0)
			{
				strcpy(extTopic_msg.msg, incoming_msg);
				strcpy(extTopic_msg.device_topic, deviceTopic);
				strcpy(extTopic_msg.from_topic, topic);
			}
		}
	}
	if (strcmp(topic, _availTopic) == 0 && useResetKeeper && firstRun)
	{
		firstRun_ResetKeeper(incoming_msg);
	}

	if (strcmp(incoming_msg, "boot") == 0)
	{
		sprintf(msg, "Boot:[%s]", bootTime);
		pub_msg(msg);
	}
	else if (strcmp(incoming_msg, "ip") == 0)
	{
		char buf[16];
		sprintf(buf, "%d.%d.%d.%d", WiFi.localIP()[0], WiFi.localIP()[1],
				WiFi.localIP()[2], WiFi.localIP()[3]);
		sprintf(msg, "IP address:[%s]", buf);
		pub_msg(msg);
	}
	else if (strcmp(incoming_msg, "ota") == 0)
	{
		sprintf(msg, "OTA allowed for %d seconds", OTA_upload_interval / 1000);
		pub_msg(msg);
		allowOTA_clock = millis();
	}
	else if (strcmp(incoming_msg, "reset") == 0)
	{
		sendReset("MQTT");
	}
	else if (strcmp(incoming_msg, "ver") == 0)
	{
		sprintf(msg, "ver: IOTlib: [%s], WDT: [%d], OTA: [%d], SERIAL: [%d], ResetKeeper[%d], FailNTP[%d], useDebugLog[%d] debugLog_VER[%s], no-networkReset[%d]",
				ver, useWDT, useOTA, useSerial, useResetKeeper, resetFailNTP, useDebug, "flog.VeR", useNetworkReset);
		pub_msg(msg);
	}
	else if (strcmp(incoming_msg, "help") == 0)
	{
		sprintf(msg,
				"Help: Commands #1 - [status, boot, reset, ip, ota, ver,ver2, help, help2, MCU_type]");
		pub_msg(msg);
		sprintf(msg, "Help: Commands #2 - [flash_format, debug_log, del_log, show_log]");
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
	else if (strcmp(incoming_msg, "debug_log") == 0)
	{
		if (useDebug)
		{
			int num_lines = flog.getnumlines();
			int filesize = flog.sizelog();
			sprintf(msg,
					"debug_log: active[%s], entries [#%d], file-size[%.2f kb]",
					useDebug ? "Yes" : "No", num_lines,
					(float)(filesize / 1000.0));
			pub_msg(msg);
		}
	}
	else if (strcmp(incoming_msg, "show_log") == 0)
	{
		if (useDebug)
		{
			char m[250];
			int x = flog.getnumlines();
			pub_debug("~~~ Start ~~~");
			for (int a = 0; a < x; a++)
			{
				flog.readline(a, m);
				pub_debug(m);
				Serial.println(m);
				delay(20);
			}
			pub_msg("debug_log: extracted");
			pub_debug("~~~ END ~~~");
		}
	}
	else if (strcmp(incoming_msg, "del_log") == 0)
	{
		if (useDebug)
		{
			flog.delog();
			pub_msg("debug_log: file deleted");
		}
	}
	else if (strcmp(incoming_msg, "flash_format") == 0)
	{
		pub_msg("Flash: Starting flash Format. System will reset at end.");
		sendReset("End Format");
	}
	else if (strcmp(incoming_msg, "del_log") == 0)
	{
		if (useDebug)
		{
			flog.delog();
			pub_msg("debug_log: file deleted");
		}
	}

	else
	{
		if (extDefine)
		{ // if this function was define then:
			ext_mqtt(incoming_msg);
		}
	}
}
void myIOT2::_pub_generic(char *topic, char *inmsg, bool retain, char *devname, bool bare)
{
	char header[100];
	int lenhdr = 0;
	int lenmsg = strlen(inmsg);
	const int mqtt_defsize = mqttClient.getBufferSize();
	const byte mqtt_overhead_size = 23;

	if (!bare)
	{
		get_timeStamp();
		if (strcmp(devname, "") == 0)
		{
			sprintf(header, "[%s] [%s] ", timeStamp, _deviceName);
		}
		else
		{
			sprintf(header, "[%s] [%s] ", timeStamp, devname);
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
	_pub_generic(_msgTopic, inmsg);
	write_log(inmsg, 0);
}
void myIOT2::pub_noTopic(char *inmsg, char *Topic)
{
	_pub_generic(Topic, inmsg, false, "", true);
	write_log(inmsg, 0);
}
void myIOT2::pub_state(char *inmsg, byte i)
{
	char *st[] = {_stateTopic, _stateTopic2};
	mqttClient.publish(st[i], inmsg, true);
	write_log(inmsg, 2);
}
void myIOT2::pub_log(char *inmsg)
{
	_pub_generic(_logTopic, inmsg);
	write_log(inmsg, 1);
}
void myIOT2::pub_ext(char *inmsg, char *name, bool retain, byte i)
{
	_pub_generic(extTopic[i], inmsg, retain, name);
	write_log(inmsg, 0);
}
void myIOT2::pub_debug(char *inmsg)
{
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
	int len = inmsg.length() + 1;
	char sms_char[len];
	inmsg.toCharArray(sms_char, len);
	_pub_generic(_smsTopic, sms_char, false, name, true);
	write_log(sms_char, 0);
}
void myIOT2::pub_sms(char *inmsg, char *name)
{
	_pub_generic(_smsTopic, inmsg, false, name, true);
	write_log(inmsg, 0);
}

void myIOT2::pub_sms(JsonDocument &sms)
{
	String output;
	get_timeStamp();
	serializeJson(sms, output);
	int len = output.length() + 1;
	char sms_char[len];
	output.toCharArray(sms_char, len);

	_pub_generic(_smsTopic, sms_char, false, "", true);
	// write_log(sms_char, 0);
}
void myIOT2::pub_email(String &inmsg, char *name)
{

	int len = inmsg.length() + 1;
	char email_char[len];
	inmsg.toCharArray(email_char, len);
	_pub_generic(_emailTopic, email_char, false, name, true);
	write_log(email_char, 0);

	// int len = output.length() + 1;
	// char email_char[len];
	// output.toCharArray(email_char, len);

	// _pub_generic(_emailTopic, email_char, false, name, true);
	// write_log(email_char, 0);
}
void myIOT2::pub_email(JsonDocument &email)
{
	String output;
	get_timeStamp();
	serializeJson(email, output);
	int len = output.length() + 1;
	char email_char[len];
	output.toCharArray(email_char, len);

	_pub_generic(_emailTopic, email_char, false, "", true);
	write_log(email_char, 0);
}
void myIOT2::msgSplitter(const char *msg_in, int max_msgSize, char *prefix, char *split_msg)
{
	char tmp[280];

	// if (strlen(prefix) + strlen(msg_in) > max_msgSize)
	// {
	Serial.println("BIG SMG");
	int max_chunk = max_msgSize - strlen(prefix) - strlen(split_msg);
	int num = ceil((float)strlen(msg_in) / max_chunk);
	int pre_len;
	Serial.print("NUM CHUNKS: ");
	Serial.println(num);

	for (int k = 0; k < num; k++)
	{
		sprintf(tmp, "%s %s%d: ", prefix, split_msg, k);
		pre_len = strlen(tmp);
		for (int i = 0; i < max_chunk; i++)
		{
			tmp[i + pre_len] = (char)msg_in[i + k * max_chunk];
			tmp[i + 1 + pre_len] = '\0';
		}
		if (mqttClient.connected() == true)
		{
			mqttClient.publish(_msgTopic, tmp);
			Serial.println(tmp);
		}
	}
	// }
	// else
	// {
	// 	if (mqttClient.connected() == true)
	// 	{
	// 		sprintf(tmp, "%s %s", prefix, msg_in);
	// 		mqttClient.publish(_msgTopic, tmp);
	// 	}
	// }
}
int myIOT2::inline_read(char *inputstr)
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
void myIOT2::notifyOnline()
{
	mqttClient.publish(_availTopic, "online", true);
	write_log("online", 2);
}
void myIOT2::notifyOffline()
{
	mqttClient.publish(_availTopic, "offline", true);
}
void myIOT2::firstRun_ResetKeeper(char *msg)
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
	notifyOnline();
}
void myIOT2::clear_ExtTopicbuff()
{
	strcpy(extTopic_msg.msg, "");
	strcpy(extTopic_msg.device_topic, "");
	strcpy(extTopic_msg.from_topic, "");
}

// ~~~~~~~~~~ Data Storage ~~~~~~~~~
void myIOT2::write_log(char *inmsg, int x)
{
	char a[strlen(inmsg) + 100];

	if (useDebug && debug_level <= x)
	{
		get_timeStamp();
		sprintf(a, ">>%s<< [%s] %s", timeStamp, _deviceName, inmsg);
		flog.write(a);
	}
}
bool myIOT2::read_fPars(char *filename, String &defs, JsonDocument &DOC, int JSIZE)
{
	myJSON param_on_flash(filename, true, JSIZE);
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
		Serial.printf("\nfile %s not found", filename);
		deserializeJson(DOC, defs);
		return 0;
	}
}
char *myIOT2::export_fPars(char *filename, JsonDocument &DOC, int JSIZE)
{
	int arraySize = 500;
	char *ret = new char[arraySize];
	myJSON param_on_flash(filename, false, JSIZE); /* read stored JSON from file */
	param_on_flash.start();

	if (param_on_flash.file_exists())
	{
		if (param_on_flash.readJSON_file(DOC))
		{
			param_on_flash.retAllJSON(ret);
			return ret;
		}
		else
		{
			strcpy(ret, "error retrieve file");
			return ret;
		}
	}
	else
	{
		Serial.printf("\nfile %s read NOT-OK", filename);
		return ret;
	}
}
void myIOT2::start_EEPROM_eADR()
{
	EEPROM.begin(120);
	NTP_eADR = _start_eADR;

	for (int i = 0; i < bootlog_len; i++)
	{
		_prevBootclock_eADR[i] = _start_eADR + 4 * (i + 1);
	}
}
void myIOT2::update_bootclockLOG()
{
	byte S = sizeof(_prevBootclock_eADR) / sizeof(_prevBootclock_eADR[0]);
	long n;
#if isESP8266
	n = now();
#elif isESP32
	n = epoch_time;
#endif
	for (int i = S - 1; i > 0; i--)
	{
		EEPROMWritelong(_prevBootclock_eADR[i], EEPROMReadlong(_prevBootclock_eADR[i - 1]));
	}
	EEPROMWritelong(_prevBootclock_eADR[0], n);

	for (int i = 0; i < S; i++)
	{
		Serial.println(EEPROMReadlong(_prevBootclock_eADR[i]));
	}
}
long myIOT2::get_bootclockLOG(int x)
{
	return EEPROMReadlong(_prevBootclock_eADR[x]);
}
// ~~~~~~ Reset and maintability ~~~~~~
void myIOT2::sendReset(char *header)
{
	char temp[150];

	sprintf(temp, "[%s] - Reset sent", header);

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
void myIOT2::feedTheDog()
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
void myIOT2::acceptOTA()
{
	if (millis() - allowOTA_clock <= OTA_upload_interval)
	{
		ArduinoOTA.handle();
	}
}
void myIOT2::startOTA()
{
	char OTAname[100];
	int m = 0;
	// create OTAname from _deviceName
	for (int i = ((String)_deviceName).lastIndexOf("/") + 1;
		 i < strlen(_deviceName); i++)
	{
		OTAname[m] = _deviceName[i];
		OTAname[m + 1] = '\0';
		m++;
	}

	allowOTA_clock = millis();

	// Port defaults to 8266
	ArduinoOTA.setPort(8266);

	// Hostname defaults to esp8266-[ChipID]
	ArduinoOTA.setHostname(OTAname);

	// No authentication by default
	// ArduinoOTA.setPassword("admin");

	// Password can be set with it's md5 value as well
	// MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
	// ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

	ArduinoOTA.onStart([]() {
		String type;
		if (ArduinoOTA.getCommand() == U_FLASH)
		{
			type = "sketch";
		}
		else
		{ // U_SPIFFS
			type = "filesystem";
		}

		// NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
		//    if (useSerial) {
		// Serial.println("Start updating " + type);
		//    }
		// Serial.end();
	});
	if (useSerial)
	{ // for debug
		ArduinoOTA.onEnd([]() {
			Serial.println("\nEnd");
		});
		ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
			Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
		});
		ArduinoOTA.onError([](ota_error_t error) {
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
			}
		});
		// ArduinoOTA.begin();
		// Serial.println("Ready");
		// Serial.print("IP address: ");
		// Serial.println(WiFi.localIP());
	}

	ArduinoOTA.begin();
}
void myIOT2::startWDT()
{
#if isESP8266
	wdt.attach(1, std::bind(&myIOT2::feedTheDog, this)); // Start WatchDog
#elif isESP32
	// wdt.attach(1,feedTheDog); // Start WatchDog
#endif
}

// ~~~~~~  EEPROM ~~~~~~
void myIOT2::EEPROMWritelong(int address, long value)
{
	byte four = (value & 0xFF);
	byte three = ((value >> 8) & 0xFF);
	byte two = ((value >> 16) & 0xFF);
	byte one = ((value >> 24) & 0xFF);

	EEPROM.write(address, four);
	EEPROM.write(address + 1, three);
	EEPROM.write(address + 2, two);
	EEPROM.write(address + 3, one);
	EEPROM.commit();
}
long myIOT2::EEPROMReadlong(long address)
{
	long four = EEPROM.read(address);
	long three = EEPROM.read(address + 1);
	long two = EEPROM.read(address + 2);
	long one = EEPROM.read(address + 3);

	return ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
}
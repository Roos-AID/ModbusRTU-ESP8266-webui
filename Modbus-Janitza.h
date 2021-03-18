/*
***************************************************************************
**  Program  : Modbus-firmware.h
**  Version 1.4.0
**
**  Copyright (c) 2021 Rob Roos
**     based on Framework ESP8266 from Willem Aandewiel and modifications
**     from Robert van Breemen
**
**  TERMS OF USE: MIT License. See bottom of file.
***************************************************************************
*/

//#include <ESP8266WiFi.h>
#include <Arduino.h>
#include <ezTime.h>             // https://github.com/ropg/ezTime
#include <TelnetStream.h>       // https://github.com/jandrassy/TelnetStream/commit/1294a9ee5cc9b1f7e51005091e351d60c8cddecf
#include <ArduinoJson.h>        // https://arduinojson.org/
#include "Wire.h"
#include "ModbusStuff.h"

#include "Debug.h"
#include "safeTimers.h"


#define LED1 D4
#define LED2 D0
void blinkLEDnow();
void blinkLEDnow(uint8_t);
void setLed(int8_t, uint8_t);

#define RELAY D8

//Defaults and macro definitions

#define _HOSTNAME   "Modbus-Janitza"
#define SETTINGS_FILE   "/settings.ini"
#define DEFAULT_TIMEZONE "Europe/Amsterdam"

#define HOME_ASSISTANT_DISCOVERY_PREFIX "homeassistant" // Home Assistant discovery prefix

#define CMSG_SIZE 512
#define JSON_BUFF_MAX   1024
#define CSTR(x) x.c_str()
#define CBOOLEAN(x) (x?"True":"False")
#define CONOFF(x) (x?"On":"Off")
#define CBINARY(x) (x ? "1" : "0")
#define EVALBOOLEAN(x) (stricmp(x, "true") == 0 || stricmp(x, "on") == 0 || stricmp(x, "1") == 0)

WiFiClient  wifiClient;
bool        Verbose = false;
char        cMsg[CMSG_SIZE];
char        fChar[10];
String      lastReset   = "";
uint64_t    upTimeSeconds=0;
uint32_t    rebootCount=0;
uint32_t    reconnectWiFiCount=0;
uint32_t    restartWiFiCount=0;
Timezone    myTZ;
String      sMessage = "";


const char *weekDayName[]  {  "Unknown", "Zondag", "Maandag", "Dinsdag", "Woensdag", "Donderdag", "Vrijdag", "Zaterdag", "Unknown" };
const char *flashMode[]    { "QIO", "QOUT", "DIO", "DOUT", "Unknown" };


//All things that are settings
String    settingHostname = _HOSTNAME;
String    settingNodeId = _HOSTNAME;

//MQTT settings
bool      statusMQTTconnection = false;
bool      settingMQTTenable = false;
bool      settingMQTTsecure = false;
String    settingMQTTbroker= "10.0.0.100";
int16_t   settingMQTTbrokerPort = 1883;
String    settingMQTTuser = "";
String    settingMQTTpasswd = "";
String    settingMQTThaprefix = HOME_ASSISTANT_DISCOVERY_PREFIX;
String    settingMQTTtopTopic = "Modbus-Janitza";
String    settingMQTTPubNamespace = "";
String    settingMQTTSubNamespace = "";

//Modbus Settings
int16_t   settingModbusSlaveAdr = 2;
int32_t   settingModbusBaudrate = 115200;
bool      settingModbusSinglephase = false;

bool      settingTimebasedSwitch = false;
uint8_t   statusRelay = false;

//
bool      settingNTPenable = true;
String    settingNTPtimezone = DEFAULT_TIMEZONE;
bool      settingLEDblink = true;

#include "networkStuff.h"
// eof

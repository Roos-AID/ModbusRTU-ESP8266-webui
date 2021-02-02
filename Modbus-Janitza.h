/*
***************************************************************************
**  Program  : Modbus-firmware.h
**  Version 1.0.1
**
**  Copyright (c) 2021 Rob Roos
**     based on Framework ESP8266 from Willem Aandewiel and modifications
**     from Robert van Breemen
**
**  TERMS OF USE: MIT License. See bottom of file.
***************************************************************************
*/

//#include <ESP8266WiFi.h>

#include <ezTime.h>             // https://github.com/ropg/ezTime
#include <TelnetStream.h>       // https://github.com/jandrassy/TelnetStream/commit/1294a9ee5cc9b1f7e51005091e351d60c8cddecf
#include <ArduinoJson.h>        // https://arduinojson.org/

#include "ModbusStuff.h"

#include "Debug.h"
#include "safeTimers.h"
#include "networkStuff.h"

// #include "Wire.h"

#define _HOSTNAME   "Modbus-Janitza"
#define SETTINGS_FILE   "/settings.ini"
#define CMSG_SIZE 512
#define JSON_BUFF_MAX   1024
#define CSTR(x) x.c_str()
#define CBOOLEAN(x) (x?"True":"False")
#define CONOFF(x) (x?"On":"Off")

WiFiClient  wifiClient;
bool        Verbose = false;
char        cMsg[CMSG_SIZE];
char        fChar[10];
String      lastReset   = "";
uint64_t    upTimeSeconds=0;
uint32_t    rebootCount=0;
uint32_t    reconnectWiFiCount=0;
uint32_t    restartWiFiCount=0;
Timezone    CET;

const char *weekDayName[]  {  "Unknown", "Zondag", "Maandag", "Dinsdag", "Woensdag", "Donderdag", "Vrijdag", "Zaterdag", "Unknown" };
const char *flashMode[]    { "QIO", "QOUT", "DIO", "DOUT", "Unknown" };


//All things that are settings
String    settingHostname = _HOSTNAME;
//MQTT settings
String    settingMQTTbroker= "10.0.0.100";
int16_t   settingMQTTbrokerPort = 1883;
String    settingMQTTuser = "";
String    settingMQTTpasswd = "";
String    settingMQTTtopTopic = "Modbus-Janitza";
//Modbus Settings
int16_t   settingModbusSlaveAdr = 2;
int32_t   settingModbusBaudrate = 115200;
int16_t   settingModbusSinglephase = 1;   // 0 is multiphase


// eof

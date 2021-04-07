
/*
***************************************************************************
**  Program  : ModbusRTU-webui.ino
**  Version 1.7.1
**
**  Copyright (c) 2021 Rob Roos
**     based on Framework ESP8266 from Willem Aandewiel and modifications
**     from Robert van Breemen
**
**  TERMS OF USE: MIT License. See bottom of file.
***************************************************************************
*/

/*
 *  How to install the ModbusRTU-webui on your nodeMCU
 *
 *  Make sure you have all required library's installed:
 *  - ezTime - https://github.com/ropg/ezTime
 *  - TelnetStream - https://github.com/jandrassy/TelnetStream/commit/1294a9ee5cc9b1f7e51005091e351d60c8cddecf
 *  - ArduinoJson - https://arduinojson.org/
 *  - modbus-esp8266  -https://github.com/emelianov/modbus-esp8266
 *  All the library's can be installed using the library manager.
 *
 *  How to upload to your LittleFS?
 *  Just install the LittleFS upload plugin (https://github.com/earlephilhower/arduino-esp8266littlefs-plugin)
 *  and upload it to your LittleFS after first flashing the device.
 *
 *  How to compile this firmware?
 *  - NodeMCU v1.0
 *  - Flashsize (4MB - FS:2MB - OTA ~1019KB)
 *  - CPU frequentcy: 160MHz
 *  - Normal defaults should work fine.
 *  First time: Make sure to flash sketch + wifi or flash ALL contents.
 *  Then load the LittleFS filesystem with the contents of the data folder 
 *
 */

#include "version.h"
#define _FW_VERSION _VERSION

#include "ModbusRTU-ESP8266-webui.h"

#define ON LOW
#define OFF HIGH
#define RELAYON HIGH
#define RELAYOFF LOW

DECLARE_TIMER_SEC(timerreadmodbus, settingModbusReadInterval, CATCH_UP_MISSED_TICKS);

//=====================================================================
void setup()
{

  Serial.begin(115400, SERIAL_8N1);
  while (!Serial) {} //Wait for OK
  Serial.println(F("\r\n[ModbusRTU-webui firmware version]\r\n"));
  Serial.printf("Booting....[%s]\r\n\r\n", String(_FW_VERSION).c_str());
  

  rebootCount = updateRebootCount();

  //setup randomseed the right way
  randomSeed(RANDOM_REG32); //This is 8266 HWRNG used to seed the Random PRNG: Read more: https://config9.com/arduino/getting-a-truly-random-number-in-arduino/

  lastReset     = ESP.getResetReason();
  Serial.printf("Last reset reason: [%s]\r\n", CSTR(ESP.getResetReason()));

  if (settingRelayAllwaysOnSwitch) setRelay(RELAYON);  else
    setRelay(RELAYOFF);
  
  //setup the status LED
  setLed(LED1, ON);
  setLed(LED2, ON);

  LittleFS.begin();
  readSettings(true);    
  
  CHANGE_INTERVAL_SEC(timerreadmodbus, settingModbusReadInterval, CATCH_UP_MISSED_TICKS);

  NodeId = getUniqueId() ;
  
  
  Serial.println(F("Attempting to connect to WiFi network\r"));
  setLed(LED1, ON);
  startWiFi(CSTR(settingHostname), 240);  // timeout 240 seconds
  blinkLED(LED1, 3, 100);
  setLed(LED1, OFF);
  startTelnet(); //start the debug port 23
  delayms(3000);

  startMDNS(CSTR(settingHostname));
  startLLMNR(CSTR(settingHostname));
  startMQTT(); 
  startNTP();

  setupFSexplorer();
  startWebserver();
  setupPing();

  //============== Setup Modbus ======================================
  setupModbus();
  doInitModbusMap();
  doInitDaytimemap();
  printDaytimemap();
  //  printModbusmap() ;
  //  readModbusSetup();
  readModbus();
  
  // Double check if Wifi is still alive (seems sometimes problematic)

  if (WiFi.status() != WL_CONNECTED)
  {
    //disconnected, try to reconnect then...
    DebugTln("Wifi not Connected !!!  Restart Wifi");
    reconnectWiFiCount++;
    restartWiFi(CSTR(settingHostname), 240);
    //check telnet
    startTelnet();
  }

  // Ping default gateway and restart Wifi if it fails.

  if (pinger.Ping(WiFi.gatewayIP(), 1) == false)
  {
    DebugTf("Pinging default gateway with IP %s, FAILED\n", WiFi.gatewayIP().toString().c_str());
    DebugTln("Error during last ping command. Restart Wifi");
    restartWiFiCount++;
    if (restartWiFiCount > 5)
    {
      doRestart("IP Ping failed to often, restart ESP");
    }
    restartWiFi(CSTR(settingHostname), 240);
    //check telnet
    startTelnet();
  }



  Debugf("Reboot count = [%d]\r\n", rebootCount);
  Debugln(F("Setup finished!"));

  //Blink LED2 to signal setup done
  setLed(LED1, OFF);
  blinkLED(LED2, 3, 100);
  setLed(LED2, OFF);

}  // End setup

//=====================================================================


//=====================================================================
//===[ blink status led ]===

void setLed(uint8_t led, uint8_t status)
{
  pinMode(led, OUTPUT);
  digitalWrite(led, status);
}

void blinkLEDms(uint32_t delay)
{
  //blink the statusled, when time passed
  DECLARE_TIMER_MS(timerBlink, delay);
  if (DUE(timerBlink))
  {
    blinkLEDnow();
  }
}

void blinkLED(uint8_t led, int nr, uint32_t waittime_ms)
{
  for (int i = nr; i > 0; i--)
  {
    blinkLEDnow(led);
    delayms(waittime_ms);
    blinkLEDnow(led);
    delayms(waittime_ms);
  }
}

void blinkLEDnow()
{
  blinkLEDnow(LED1);
}

void blinkLEDnow(uint8_t led = LED1)
{
  pinMode(led, OUTPUT);
  digitalWrite(led, !digitalRead(led));
}

//===[ no-blocking delay with running background tasks in ms ]===
void delayms(unsigned long delay_ms)
{
  DECLARE_TIMER_MS(timerDelayms, delay_ms);
  while (DUE(timerDelayms))
    doBackgroundTasks();
}

//=====================================================================

//===[ Do task every 1s ]===
void doTaskEvery1s(){
  //== do tasks ==
  upTimeSeconds++;
}

//===[ Do task every 5s ]===
void doTaskEvery5s(){
  //== do tasks ==
  yield();
}

//===[ Do task every 30s ]===
void doTaskEvery30s(){
  //== do tasks ==
}

//===[ Do task every 60s ]===
void doTaskEvery60s(){
  //== do tasks ==
  checkactivateRelay(true);
  //if no wifi, try reconnecting (once a minute)
  if (WiFi.status() != WL_CONNECTED)
  {
    //disconnected, try to reconnect then...
    reconnectWiFiCount++;
    startWiFi(CSTR(settingHostname), 240);
    //check telnet
    startTelnet();
  }

// Ping default gateway and restart Wifi if it fails.

  if(pinger.Ping(WiFi.gatewayIP(),1) == false)
  {
    DebugTf("Pinging default gateway with IP %s, FAILED\n", WiFi.gatewayIP().toString().c_str());
    DebugTln("Error during last ping command. Restart Wifi");
    restartWiFiCount++ ;
    if (restartWiFiCount > 5) {
      doRestart("IP Ping failed to often, restart ESP");
    }
    restartWiFi(CSTR(settingHostname), 240);
    //check telnet
    startTelnet();
  }

}
// end doTaskEvery60s()

//===[ Do the background tasks ]===
void doBackgroundTasks()
{
  handleMQTT();                 // MQTT transmissions
  httpServer.handleClient();
  MDNS.update();
  events();                     // trigger ezTime update etc.
  delay(1);
  handleDebug();
  // yield();
}

void loop()
{

  DECLARE_TIMER_SEC(timer1s, 1, CATCH_UP_MISSED_TICKS);
  DECLARE_TIMER_SEC(timer5s, 5, CATCH_UP_MISSED_TICKS);
  DECLARE_TIMER_SEC(timer30s, 30, CATCH_UP_MISSED_TICKS);
  DECLARE_TIMER_SEC(timer60s, 60, CATCH_UP_MISSED_TICKS);

  if (DUE(timer1s))       doTaskEvery1s();
  if (DUE(timer5s))       doTaskEvery5s();
  if (DUE(timer30s))      doTaskEvery30s();
  if (DUE(timer60s))      doTaskEvery60s();
  if (DUE(timerreadmodbus)) readModbus();

  doBackgroundTasks();
}

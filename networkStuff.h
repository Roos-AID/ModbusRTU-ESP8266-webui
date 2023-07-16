/*
***************************************************************************
**  Program : networkStuff.h
**  Version 1.9.1
**
**  Copyright (c) 2022 Rob Roos
**     based on Framework ESP8266 from Willem Aandewiel and modifications
**     from Robert van Breemen
**
**  TERMS OF USE: MIT License. See bottom of file.
***************************************************************************
*/


#include <ESP8266WiFi.h>        //ESP8266 Core WiFi Library
#include <ESP8266WebServer.h>   //ESP8266 Core https://github.com/esp8266/Arduino
#include <ESP8266mDNS.h>        // part of ESP8266 Core https://github.com/esp8266/Arduino
#include <ESP8266HTTPClient.h>
#include <ESP8266LLMNR.h>

#include <WiFiUdp.h>            // part of ESP8266 Core https://github.com/esp8266/Arduino
//#include "ESP8266HTTPUpdateServer.h"
#include "ModUpdateServer.h"   // https://github.com/mrWheel/ModUpdateServer
#include "updateServerHtml.h"
#include <WiFiManager.h>        // version 2.0.11-beta - use latest development branch  - https://github.com/tzapu/WiFiManager
// included in main program: #include <TelnetStream.h>       // Version 0.0.1 - https://github.com/jandrassy/TelnetStream
// #include<FS.h> // part of ESP8266 Core https://github.com/esp8266/Arduino
#include <LittleFS.h>

//prototype
bool updateRebootLog(String text) ;

//Use the NTP SDK ESP 8266 
#include <time.h>
extern "C" int clock_gettime(clockid_t unused, struct timespec *tp);

enum NtpStatus_t {
	TIME_NOTSET,
	TIME_SYNC,
	TIME_WAITFORSYNC,
  TIME_NEEDSYNC
};

NtpStatus_t NtpStatus = TIME_NOTSET;
static const time_t EPOCH_2000_01_01 = 946684800;
time_t NtpLastSync = 0; //last sync moment in EPOCH seconds


ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater(true);
// Ping logic
#include <Pinger.h>

extern "C" {  
  #include <lwip/icmp.h> // needed for icmp packet definitions
    }

// Set global to avoid object removing after setup() routine
Pinger pinger;

static      FSInfo LittleFSinfo;
bool        LittleFSmounted;
bool        isConnected = false;

//gets called when WiFiManager enters configuration mode
//===========================================================================================
void configModeCallback (WiFiManager *myWiFiManager)
{
  DebugTln(F("Entered config mode\r"));
  DebugTln(WiFi.softAPIP().toString());
  //if you used auto generated SSID, print it
  DebugTln(myWiFiManager->getConfigPortalSSID());

} // configModeCallback()

//===========================================================================================
void startWiFi(const char *hostname, int timeOut)
{
  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
  WiFi.setSleepMode(WIFI_NONE_SLEEP); // explicitly set Wifi sleep mode off
  // WiFi.setOutputPower(20.5);

  WiFiManager manageWiFi;
  uint32_t lTime = millis();
  String thisAP = String(hostname) + "-" + WiFi.macAddress();

  Serial.println("Start Wifi ...");
  manageWiFi.setDebugOutput(true);

  //--- next line in release needs to be commented out!
  // manageWiFi.resetSettings();

  //--- set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
  manageWiFi.setAPCallback(configModeCallback);

  //--- sets timeout until configuration portal gets turned off
  //--- useful to make it all retry or go to sleep in seconds
  //manageWiFi.setTimeout(240);  // 4 minuten
  manageWiFi.setTimeout(timeOut); // in seconden ...

  //--- fetches ssid and pass and tries to connect
  //--- if it does not connect it starts an access point with the specified name
  //--- here  "<HOSTNAME>-<MAC>"
  //--- and goes into a blocking loop awaiting configuration
  Serial.printf("AutoConnect to: %s", thisAP.c_str());
  if (!manageWiFi.autoConnect(thisAP.c_str()))
  {
    //-- fail to connect? Have you tried turning it off and on again?
    DebugTln(F("failed to connect and hit timeout"));
    updateRebootLog("failed to connect and hit timeout, restart ESP");

    delay(2000); // Enough time for messages to be sent.
    ESP.restart();
    delay(5000); // Enough time to ensure we don't return.
  }

  //WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP

  Debugln();
  DebugT(F("Connected to "));
  Debugln(WiFi.SSID());
  DebugT(F("IP address: "));
  Debugln(WiFi.localIP());
  DebugT(F("IP gateway: "));
  Debugln(WiFi.gatewayIP());
  Debugln();

  httpUpdater.setup(&httpServer);
  httpUpdater.setIndexPage(UpdateServerIndex);
  httpUpdater.setSuccessPage(UpdateServerSuccess);
  DebugTf(" took [%d] seconds => OK!\r\n", (millis() - lTime) / 1000);

} // startWiFi()

void restartWiFi(const char* hostname, int timeOut) {
  WiFi.disconnect() ;
  delay(2000);
  startWiFi(hostname, timeOut);
}

//===========================================================================================
void startTelnet()
{
  Serial.print("Use  'telnet ");
  Serial.print(WiFi.localIP());
  Serial.println("' for debugging");
  TelnetStream.begin();
  DebugTln(F("Telnet server started ..\r\n"));
  TelnetStream.flush();
} // startTelnet()

//=======================================================================
void startMDNS(const char *Hostname)
{
  DebugTf("mDNS setup as [%s.local]\r\n", Hostname);
  if (MDNS.begin(Hostname))               // Start the mDNS responder for Hostname.local
  {
    DebugTf("mDNS responder started as [%s.local]\r\n", Hostname);
  }
  else
  {
    DebugTln(F("Error setting up MDNS responder!\r\n"));
  }
  MDNS.addService("http", "tcp", 80);

} // startMDNS()

void startLLMNR(const char *hostname)
{
  DebugTf("LLMNR setup as [%s]\r\n", hostname);
  if (LLMNR.begin(hostname)) // Start the LLMNR responder for hostname
  {
    DebugTf("LLMNR responder started as [%s]\r\n", hostname);
  }
  else
  {
    DebugTln(F("Error setting up LLMNR responder!\r\n"));
  }
} // startLLMNR()


//==[ NTP stuff ]==============================================================


void startNTP(){
  // Initialisation ezTime
  if (!settingNTPenable) return;
  if (settingNTPtimezone.length()==0) settingNTPtimezone = NTP_DEFAULT_TIMEZONE; //set back to default timezone
  if (settingNTPhostname.length()==0) settingNTPhostname = NTP_HOST_DEFAULT; //set back to default timezone

  //void configTime(int timezone_sec, int daylightOffset_sec, const char* server1, const char* server2, const char* server3)
  configTime(0, 0, CSTR(settingNTPhostname), nullptr, nullptr);
  // Configure NTP before WiFi, so DHCP can override the NTP server(s)
  
  NtpStatus = TIME_WAITFORSYNC;
}


void getNTPtime(){
  struct timespec tp;   //to enable clock_gettime()
  double tNow;
  long dt_sec, dt_ms, dt_nsec;
  clock_gettime(CLOCK_REALTIME, &tp);  
  tNow = tp.tv_sec+(tp.tv_nsec/1.0e9);
  dt_sec = tp.tv_sec;
  dt_ms = tp.tv_nsec / 1000000UL;
  dt_nsec = tp.tv_nsec;
  DebugTf(PSTR("tNow=%20.10f tNow_sec=%16.10ld tNow_nsec=%16.10ld dt_sec=%16li(s) dt_msec=%16li(sm) dt_nsec=%16li(ns)\r\n"), (double)tNow, tp.tv_sec,tp.tv_nsec, dt_sec, dt_ms, dt_nsec);
  DebugFlush();
}

void loopNTP(){
time_t now;
now = time(nullptr); //this is now...
configTime(0, 0, CSTR(settingNTPhostname), nullptr, nullptr);

if (!settingNTPenable) return;
  switch (NtpStatus){
    case TIME_NOTSET:
    case TIME_NEEDSYNC:
      NtpLastSync = now; //remember last sync
      DebugTln(F("Start time syncing"));
      startNTP();
      DebugTf(PSTR("Starting timezone lookup for [%s]\r\n"), CSTR(settingNTPtimezone));
      NtpStatus = TIME_WAITFORSYNC;
      break;
    case TIME_WAITFORSYNC:
      if ((now > EPOCH_2000_01_01) && (now >= NtpLastSync)) { 
        //DebugTf(PSTR("Waited for sync: epoch: %lld\r\n"), time(nullptr));
        NtpLastSync = now; //remember last sync         
        TimeZone myTz =  timezoneManager.createForZoneName(CSTR(settingNTPtimezone));
        if (myTz.isError()){
          DebugTf(PSTR("Error: Timezone Invalid/Not Found: [%s]\r\n"), CSTR(settingNTPtimezone));
          settingNTPtimezone = NTP_DEFAULT_TIMEZONE;
          myTz = timezoneManager.createForZoneName(CSTR(settingNTPtimezone)); //try with default Timezone instead
        } else {
          //found the timezone, now set the time 
          ZonedDateTime myTime = ZonedDateTime::forUnixSeconds64(now, myTz);
          DebugTf(PSTR("%02d:%02d:%02d %02d-%02d-%04d\n\r"), myTime.hour(), myTime.minute(), myTime.second(), myTime.day(), myTime.month(), myTime.year());
          if (!myTime.isError()) {
            //finally time is synced!
            //setTime(myTime.hour(), myTime.minute(), myTime.second(), myTime.day(), myTime.month(), myTime.year());
            NtpStatus = TIME_SYNC;
            DebugTln(F("Time synced!"));
          }
        }
      } 
    break;
    case TIME_SYNC:
      if ((now -  NtpLastSync) > NTP_RESYNC_TIME){
        //when xx seconds have passed, resync using NTP
         DebugTln(F("Time resync needed"));
        NtpStatus = TIME_NEEDSYNC;
      }
    break;
  } 
 
  // DECLARE_TIMER_SEC(timerNTPtime, 10, CATCH_UP_MISSED_TICKS);
  // if DUE(timerNTPtime) 
  // {
  //   //DebugTf(PSTR("Epoch Seconds: %lld\r\n"), now); //timeout, then break out of this loop
  //   DebugT("Now: ");
  //   Debug(now);
  //   Debugln();
  //   DebugT("Timezone : ");
  //   Debugln(CSTR(settingNTPtimezone));
  //   TimeZone myTz =  timezoneManager.createForZoneName(CSTR(settingNTPtimezone));
  //   ZonedDateTime myTime = ZonedDateTime::forUnixSeconds64(now, myTz);
    
  //   DebugTf(PSTR("%02d:%02d:%02d %02d-%02d-%04d\r\n"), myTime.hour(), myTime.minute(), myTime.second(), myTime.day(), myTime.month(), myTime.year());
  // }
  // if DUE(timerNTPtime) getNTPtime();
}

bool isNTPtimeSet(){
  return NtpStatus == TIME_SYNC;
}

// void waitforNTPsync(int16_t timeout = 30){  
//   //wait for time is synced to NTP server, for maximum of timeout seconds
//   //feed the watchdog while waiting 
//   //update NTP status
//   time_t t = time(nullptr); //get current time
//   DebugTf(PSTR("Waiting for NTP sync, timeout: %d\r\n"), timeout);
//   DECLARE_TIMER_SEC(waitforNTPsync, timeout, CATCH_UP_MISSED_TICKS);
//   DECLARE_TIMER_SEC(timerWaiting, 5, CATCH_UP_MISSED_TICKS);
//   while (true){
//     //feed the watchdog while waiting
//     Wire.beginTransmission(0x26);   
//     Wire.write(0xA5);   
//     Wire.endTransmission();
//     delay(100);
//     if DUE(timerWaiting) DebugTf(PSTR("Waiting for NTP sync: %lu seconds\r\n"), (time(nullptr)-t));
//     // update NTP status
//     loopNTP();
//     //stop waiting when NTP is synced 
//     if (isNTPtimeSet()) {
//       Debugln(F("NTP time synced!"));
//       break;
//     }
//     //stop waiting when timeout is reached 
//     if DUE(waitforNTPsync) {
//       DebugTln(F("NTP sync timeout!"));
//       break;
//     } 
//   }
// }


//==[ end of NTP stuff ]=======================================================

String getMacAddress()
{
  uint8_t baseMac[6];
  char baseMacChr[13] = {0};
#if defined(ESP8266)
  WiFi.macAddress(baseMac);
  sprintf(baseMacChr, "%02X%02X%02X%02X%02X%02X", baseMac[0], baseMac[1], baseMac[2], baseMac[3], baseMac[4], baseMac[5]);
#elif defined(ESP32)
  esp_read_mac(baseMac, ESP_MAC_WIFI_STA);
  sprintf(baseMacChr, "%02X%02X%02X%02X%02X%02X", baseMac[0], baseMac[1], baseMac[2], baseMac[3], baseMac[4], baseMac[5]);
#else
  sprintf(baseMacChr, "%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
#endif
  return String(baseMacChr);
}

String getUniqueId()
{
  String uniqueId = settingHostname + (String)getMacAddress();
  return String(uniqueId);
}

//================ Setup Ping =======================================================
void setupPing() {
 pinger.OnReceive([](const PingerResponse& response)
  {
    if (response.ReceivedResponse)
    {
      DebugTf(
        "Reply from %s: bytes=%d time=%lums TTL=%d \r\n",
        response.DestIPAddress.toString().c_str(),
        response.EchoMessageSize - sizeof(struct icmp_echo_hdr),
        response.ResponseTime,
        response.TimeToLive);
      // DebugTln("End Ping");
    }
    else
    {
      DebugTf("Ping request timed out.\n");
    }

    // Return true to continue the ping sequence.
    // If current event returns false, the ping sequence is interrupted.
    return true;
  });

//  pinger.OnEnd([](const PingerResponse& response)
//  {
//    // Evaluate lost packet percentage
//    float loss = 100;
//    if(response.TotalReceivedResponses > 0)
//    {
//      loss = (response.TotalSentRequests - response.TotalReceivedResponses) * 100 / response.TotalSentRequests;
//    }
//
//    // Print packet trip data
//    DebugTf(
//      "Ping statistics for %s:\n",
//      response.DestIPAddress.toString().c_str());
//    DebugTf(
//      "    Packets: Sent = %lu, Received = %lu, Lost = %lu (%.2f%% loss),\n",
//      response.TotalSentRequests,
//      response.TotalReceivedResponses,
//      response.TotalSentRequests - response.TotalReceivedResponses,
//      loss);
//
//    // Print time information
//    if(response.TotalReceivedResponses > 0)
//    {
//      DebugTf("Approximate round trip times in milli-seconds:\n");
//      DebugTf(
//        "    Minimum = %lums, Maximum = %lums, Average = %.2fms\n",
//        response.MinResponseTime,
//        response.MaxResponseTime,
//        response.AvgResponseTime);
//    }
//
//    // Print host data
//    DebugTf("Destination host data:\n");
//    DebugTf(
//      "    IP address: %s\n",
//      response.DestIPAddress.toString().c_str());
//    if(response.DestMacAddress != nullptr)
//    {
//      DebugTf(
//        "    MAC address: " MACSTR "\n",
//        MAC2STR(response.DestMacAddress->addr));
//    }
//    if(response.DestHostname != "")
//    {
//      DebugTf(
//        "    DNS name: %s\n",
//        response.DestHostname.c_str());
//    }
//
//    return true;
//  });
}

/***************************************************************************
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to permit
* persons to whom the Software is furnished to do so, subject to the
* following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT
* OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
* THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
****************************************************************************
*/

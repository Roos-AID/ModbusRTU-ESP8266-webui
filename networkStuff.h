/*
***************************************************************************
**  Program : networkStuff.h
**  Version 1.0.1
**
**  Copyright (c) 2021 Rob Roos
**     based on Framework ESP8266 from Willem Aandewiel and modifications
**     from Robert van Breemen
**
**  TERMS OF USE: MIT License. See bottom of file.
***************************************************************************
*/


#include <ESP8266WiFi.h>        //ESP8266 Core WiFi Library
#include <ESP8266WebServer.h>   //ESP8266 Core https://github.com/esp8266/Arduino
#include <ESP8266mDNS.h>        // part of ESP8266 Core https://github.com/esp8266/Arduino

#include <WiFiUdp.h>            // part of ESP8266 Core https://github.com/esp8266/Arduino
//#include "ESP8266HTTPUpdateServer.h"
#include "ModUpdateServer.h"   // https://github.com/mrWheel/ModUpdateServer
#include "updateServerHtml.h"
#include <WiFiManager.h>        // version 2.0.4-beta - use latest development branch  - https://github.com/tzapu/WiFiManager
// included in main program: #include <TelnetStream.h>       // Version 0.0.1 - https://github.com/jandrassy/TelnetStream
#include <FS.h>                 // part of ESP8266 Core https://github.com/esp8266/Arduino

// Ping logic
#include <Pinger.h>

extern "C"
{
  #include <lwip/icmp.h> // needed for icmp packet definitions
}

// Set global to avoid object removing after setup() routine
Pinger pinger;


ESP8266WebServer        httpServer (80);
ESP8266HTTPUpdateServer httpUpdater(true);


static      FSInfo SPIFFSinfo;
bool        SPIFFSmounted;
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
void startWiFi(const char* hostname, int timeOut)
{
  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP

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
  manageWiFi.setTimeout(timeOut);  // in seconden ...

  //--- fetches ssid and pass and tries to connect
  //--- if it does not connect it starts an access point with the specified name
  //--- here  "<HOSTNAME>-<MAC>"
  //--- and goes into a blocking loop awaiting configuration
  Serial.printf("AutoConnect to: %s", thisAP.c_str());
  if (!manageWiFi.autoConnect(thisAP.c_str()))
  {
    //-- fail to connect? Have you tried turning it off and on again?
    DebugTln(F("failed to connect and hit timeout"));
    delay(2000);  // Enough time for messages to be sent.
    ESP.restart();
    delay(5000);  // Enough time to ensure we don't return.
  }

  //WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP

  Debugln();
  DebugT(F("Connected to " )); Debugln (WiFi.SSID());
  DebugT(F("IP address: " ));  Debugln (WiFi.localIP());
  DebugT(F("IP gateway: " ));  Debugln (WiFi.gatewayIP());
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
  TelnetStream.begin();
  DebugTln(F("\nTelnet server started .."));
  TelnetStream.flush();
} // startTelnet()

//=======================================================================
void startMDNS(const char *Hostname)
{
  DebugTf("[1] mDNS setup as [%s.local]\r\n", Hostname);
  if (MDNS.begin(Hostname))               // Start the mDNS responder for Hostname.local
  {
    DebugTf("[2] mDNS responder started as [%s.local]\r\n", Hostname);
  }
  else
  {
    DebugTln(F("[3] Error setting up MDNS responder!\r\n"));
  }
  MDNS.addService("http", "tcp", 80);

} // startMDNS()


//================ Setup Ping =======================================================
void setupPing() {
 pinger.OnReceive([](const PingerResponse& response)
  {
    if (response.ReceivedResponse)
    {
      DebugTf(
        "Reply from %s: bytes=%d time=%lums TTL=%d\n",
        response.DestIPAddress.toString().c_str(),
        response.EchoMessageSize - sizeof(struct icmp_echo_hdr),
        response.ResponseTime,
        response.TimeToLive);
    }
    else
    {
      DebugTf("Request timed out.\n");
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

/*
***************************************************************************
**  Program  : handleDebug
**  Version 1.6.0
**
**
**  Copyright (c) 2021 Rob Roos
**     based on Framework ESP8266 from Willem Aandewiel and modifications
**     from Robert van Breemen
**
**  TERMS OF USE: MIT License. See bottom of file.
***************************************************************************
*/
void handleDebug(){
    if (TelnetStream.available()>0){
        //read the next 
        char c;
        c = TelnetStream.read();
        switch (c){
        case 'h':
            Debugln("Available commands are:");
            Debugln("l : List Daytimemap");
            Debugln("d : Re-read Daytimemap from Daytimemap.cfg");
            Debugln("t : Toggle Timebased Relay");
            Debugln("m : Configure MQTT Discovery");
            Debugln("r : Reconnecting to wifi");
            Debugln("1 : Toggle debug Modbus msg");
            Debugln("2 : Toggle debug RestAPI");
            Debugln("3 : Toggle debug MQTT");

            break;
        case 'd':
            DebugTln("Read Daytimemap");
            doInitDaytimemap() ;
            printDaytimemap();
            break;
        case 'l':
            DebugTln("List Daytimemap");
            printDaytimemap();
            break;
        case 't':
            DebugTln("Toggle Timebased Relay");
            if (settingTimebasedSwitch && settingNTPenable) {
                DebugTf("Relay current status %d \r\n", statusRelay);

                if (statusRelay==RELAYOFF) 
                setRelay(RELAYON) ;
                else setRelay(RELAYOFF);

            } else {
                DebugTln("NOT POSSIBLE, TIMEBASED SWITCHING NOT ACTIVATED");
            }
        case 'm':
            DebugTln("Configure MQTT Discovery");
            DebugTf("Enable MQTT: %s", CBOOLEAN(settingMQTTenable));
            doAutoConfigure();
            break;
        case 'r':
            if (WiFi.status() != WL_CONNECTED)
            {
                DebugTln("Reconnecting to wifi");
                restartWiFi(CSTR(settingHostname), 240);
                startTelnet();
            }
            else
                DebugTln("Wifi is connected");

            if (!statusMQTTconnection)
            {
                DebugTln("Reconnecting MQTT");
                startMQTT();
            }
            else
                DebugTln("MQTT is connected");
            break;
        case '1':
            bDebugMBmsg = !bDebugMBmsg;
            DebugTf("\r\nDebug Modbus msg: %s\r\n", CBOOLEAN(bDebugMBmsg));
            break;
        case '2':
            bDebugRestAPI = !bDebugRestAPI;
            DebugTf("\r\nDebug RestAPI: %s\r\n", CBOOLEAN(bDebugRestAPI));
            break;
        case '3':
            bDebugMQTT = !bDebugMQTT;
            DebugTf("\r\nDebug MQTT: %s\r\n", CBOOLEAN(bDebugMQTT));
            break;

            break;
        default:
        break;
        }

    }
}

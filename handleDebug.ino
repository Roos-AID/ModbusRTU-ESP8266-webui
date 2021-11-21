/*
***************************************************************************
**  Program  : handleDebug
**  Version 1.8.0
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
            Debugln("p : Print (list) Modbusmap ");
            Debugln("l : List Daytimemap");
            Debugln("d : Re-read Daytimemap from Daytimemap.cfg");
            Debugln("t : Toggle Timebased Relay");
            Debugln("m : Configure MQTT Discovery");
            Debugln("r : Reconnecting to wifi");
            Debugf("1 : Toggle debug Modbus msg, status: %s\r\n", CBOOLEAN(bDebugMBmsg));
            Debugf("2 : Toggle debug RestAPI status %s\r\n", CBOOLEAN(bDebugRestAPI));
            Debugf("3 : Toggle debug MQTT, status: %s\r\n", CBOOLEAN(bDebugMQTT));
            Debugf("8 : Toggle Modbus Byteswap: %s\r\n", CBOOLEAN(settingModbusByteswap));
            Debugf("9 : Toggle debug Modbuslogic, status: %s\r\n", CBOOLEAN(bDebugMBlogic));

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
        case 'p':
            DebugTln("Print Modbusmap");
            printModbusmap();
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
            Debugf("\r\nDebug Modbus msg: %s\r\n", CBOOLEAN(bDebugMBmsg));
            break;
        case '2':
            bDebugRestAPI = !bDebugRestAPI;
            Debugf("\r\nDebug RestAPI: %s\r\n", CBOOLEAN(bDebugRestAPI));
            break;
        case '3':
            bDebugMQTT = !bDebugMQTT;
            Debugf("\r\nDebug MQTT: %s\r\n", CBOOLEAN(bDebugMQTT));
            break;
        case '8':
            settingModbusByteswap = !settingModbusByteswap;
            Debugf("\r\nModbus ByteSwap: %s\r\n", CBOOLEAN(settingModbusByteswap));
            break;
    
        case '9':
            bDebugMBlogic = !bDebugMBlogic;
            Debugf("\r\nDebug Modbuslogic: %s\r\n", CBOOLEAN(bDebugMBlogic));
            break;
        default:  break;
        }

    }
}

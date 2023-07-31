/*
***************************************************************************
**  Program  : handleDebug
**  Version 1.11.0
**
**
**  Copyright (c) 2023 Rob Roos
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
            Debugln(F("Available commands are:"));
            Debugln(F("p : Print (list) Modbusmap "));
            Debugln(F("l : List Daytimemap"));
            Debugln(F("d : Re-read Daytimemap from Daytimemap.cfg"));
            Debugln(F("t : Toggle Timebased Relay"));
            Debugln(F("m : Configure MQTT Discovery"));
            Debugln(F("r : Reconnecting to wifi"));
            Debugf(PSTR("1 : Toggle debug Modbus msg, status: %s\r\n"), CBOOLEAN(bDebugMBmsg));
            Debugf(PSTR("2 : Toggle debug RestAPI status %s\r\n"), CBOOLEAN(bDebugRestAPI));
            Debugf(PSTR("3 : Toggle debug MQTT, status: %s\r\n"), CBOOLEAN(bDebugMQTT));
            Debugf(PSTR("8 : Toggle Modbus Byteswap: %s\r\n"), CBOOLEAN(settingModbusByteswap));
            Debugf(PSTR("9 : Toggle debug Modbuslogic, status: %s\r\n"), CBOOLEAN(bDebugMBlogic));

            break;
        case 'd':
            DebugTln(F("Read Daytimemap"));
            doInitDaytimemap() ;
            printDaytimemap();
            break;
        case 'l':
            DebugTln(F("List Daytimemap"));
            printDaytimemap();
            break;
        case 'p':
            DebugTln(F("Print Modbusmap"));
            printModbusmap();
            break;
        case 't':
            DebugTln(F("Toggle Timebased Relay"));
            if (settingTimebasedSwitch && settingNTPenable) {
                DebugTf("Relay current status %d \r\n", statusRelay);

                if (statusRelay==RELAYOFF) 
                setRelay(RELAYON) ;
                else setRelay(RELAYOFF);

            } else {
                DebugTln(F("NOT POSSIBLE, TIMEBASED SWITCHING NOT ACTIVATED"));
            }
        case 'm':
            DebugTln(F("Configure MQTT Discovery"));
            DebugTf("Enable MQTT: %s", CBOOLEAN(settingMQTTenable));
            doAutoConfigure();
            break;
        case 'r':
            if (WiFi.status() != WL_CONNECTED)
            {
                DebugTln(F("Reconnecting to wifi"));
                restartWiFi(CSTR(settingHostname), 240);
                startTelnet();
            } 
            else
                DebugTln(F("Wifi is connected"));

            if (!statusMQTTconnection)
            {
                DebugTln(F("Reconnecting MQTT"));
                startMQTT();
            }
            else
                DebugTln(F("MQTT is connected"));
            break;
        case '1':
            bDebugMBmsg = !bDebugMBmsg;
            Debugf(PSTR("\r\nDebug Modbus msg: %s\r\n"), CBOOLEAN(bDebugMBmsg));
            break;
        case '2':
            bDebugRestAPI = !bDebugRestAPI;
            Debugf(PSTR("\r\nDebug RestAPI: %s\r\n"), CBOOLEAN(bDebugRestAPI));
            break;
        case '3':
            bDebugMQTT = !bDebugMQTT;
            Debugf(PSTR("\r\nDebug MQTT: %s\r\n"), CBOOLEAN(bDebugMQTT));
            break;
        case '8':
            settingModbusByteswap = !settingModbusByteswap;
            Debugf(PSTR("\r\nModbus ByteSwap: %s\r\n"), CBOOLEAN(settingModbusByteswap));
            break;
    
        case '9':
            bDebugMBlogic = !bDebugMBlogic;
            Debugf(PSTR("\r\nDebug Modbuslogic: %s\r\n"), CBOOLEAN(bDebugMBlogic));
            break;
        default:  break;
        }

    }
}

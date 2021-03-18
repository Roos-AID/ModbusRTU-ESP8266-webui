/*
***************************************************************************
**  Program  : handleDebug
**  Version 1.4.1
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
        case 'r':
            DebugTln("Read Daytimemap");
            doInitDaytimemap() ;
            break;
        case 't':
            DebugTln("List Daytimemap");
            DebugTf("Enable Timebased relay: %s\r\n", CBOOLEAN(settingTimebasedSwitch));
            printDaytimemap();
            break;
        case 's':
            DebugTln("Toggle Timebased Relay");
            if (settingTimebasedSwitch && settingNTPenable) {
                DebugTf("Relay current status %d \r\n", statusRelay);

                if (statusRelay==RELAYOFF) 
                setRelay(RELAYON) ;
                else setRelay(RELAYOFF);

                DebugTf("Relay set to %d \r\n", statusRelay);
                
            } else {
                DebugTln("NOT POSSIBLE, TIMEBASED SWITCHING NOT ACTIVATED");
            }
            break;
        default:
        break;
        }

    }
}

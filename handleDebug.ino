/*
***************************************************************************
**  Program  : handleDebug
**  Version 1.5.0
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
            Debugln("t : List Daytimemap");
            Debugln("r : Re-read Daytimemap from Daytimemap.cfg");
            Debugln("s : Toggle Timebased Relay");
            break;
        case 'r':
            DebugTln("Read Daytimemap");
            doInitDaytimemap() ;
            printDaytimemap();
            break;
        case 't':
            DebugTln("List Daytimemap");
            printDaytimemap();
            break;
        case 's':
            DebugTln("Toggle Timebased Relay");
            if (settingTimebasedSwitch && settingNTPenable) {
                DebugTf("Relay current status %d \r\n", statusRelay);

                if (statusRelay==RELAYOFF) 
                setRelay(RELAYON) ;
                else setRelay(RELAYOFF);

            } else {
                DebugTln("NOT POSSIBLE, TIMEBASED SWITCHING NOT ACTIVATED");
            }
            break;
        default:
        break;
        }

    }
}

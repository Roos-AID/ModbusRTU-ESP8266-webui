void handleDebug(){
    if (TelnetStream.available()>0){
        //read the next 
        char c;
        c = TelnetStream.read();
        switch (c){
            case 't':
                DebugTln("Dump Daytimemap");
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

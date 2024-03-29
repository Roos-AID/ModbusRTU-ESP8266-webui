# ModbusRTU-ESP8266-webui firmware



The features of this ModbusRTU-ESP8266-webui firmware are:
- Implementing the Modbus RTU protocol on the NodeMCU (ESP8266) to display in webUI and publish to MQTT
- A simple schematic is in the Documents folder, there are also some screenshots of the webUI on an iPhone

- Registers and formats to be read can be configured in config file Modbusmap.cfg (see file for spec)
- setting for the config file name, default to Modbusmap.cfg , 
    additional file Modbusmaptest.cfg is to debug the code and is automatically loaded when bDebugMBlogic = true during compile. 
    Additional files are included, eg. MBmapUMG96.cfg for Janitza UMG96 and MBmapSolaredge.cfg as examples. 
- Strings upto 64 chars in length, being 32 registers, can be read, see config file for specification.
    - As Modbus Strings are not clearly defined how to decode, a byteswap option is available (in telnet debug option 8 for now).

- Dynamic MQ Autoconfigure for homeassistant based on registers defined in Modbusmap.cfg
- When MQ is enabled, then the specified Modbus registers are send to MQTT when enabled in Modbusmap.cfg

- Register values can be converted by a set factor in the config file for display in Webui and MQTT (eg. read Wh and display kWh)
    The value can also be negative.
- Warning : Conversion of int values are rounded to int when factor is applied

- Enable telnet listening (for debugging and some commands, enter h for help)
- Telnet commands are eg: t = toggle relay, l = list Daytime map , d = re-read Daytimemap.cfg, m = Configure MQTT Discovery
- Telnet debug commands are 1,2,3 and 8, 9 (see output of h)

- a REST API (http://<ip>/api/v1/Modbus/Modbusmonitor
- settings for Hostname, MQTT and NTP in the webUI 
- settings for Modbus baudrate, RTU slave address and read interval in the webUI

- Time/day based setting to enable external relay (GPIO) to allow for day/night energy monitoring/use
- Initially designed to read Jantitza UMG96RM energy monitor registers and display in webUI


To do:
- Byteswap switch in config screen
- Documentation (configuration and operation)
- InfluxDB client to do direct logging
- Update of specific registers through MQTT (low prio)
- In version 2 we might add multiple Modbus devices to be read (very low prio now)


Looking for the documentation?, see the Documentation folder (work in progress).  

| Version | Release notes |
|-|-|
| 1.7.3 | Byteswap option added for Modbus_String (via debug for now), String length fix |  
| 1.7.2 | Modbusmap config filename via settings, added String type|  
| 1.7.1 | Additional debug options, fix coding bugs|  
| 1.7.0 | Implement additional types for Solaredge, rework Modbus reader code|  
| 1.6.4 | Read interval timer in settings|  
| 1.6.3 | Conversion of factor also for MQTT values, plus smaller bug fixes|  
| 1.6.2 | Attempt to get Autoconfigure working for Openhab|  
| 1.6.1 | Dynamic MQ Autoconfigure from the Modbusmap.cfg file|  
| 1.6.0 | Implement MQTT Uniqueid setting, additional debug options|  
| 1.5.0 | Rename and cleanup, added Relay Allways On switch|  
| 1.4.4 | MQ updates after reading Modbus register|  
| 1.4.3 | MQ Pub/Sub name space repaired|  
| 1.4.2 | Typo in debug info, additional telnet command (re-read Daytimemap|  
| 1.4.1 | Cosmetic changes, cleanup relay function|  
| 1.4.0 | GPIO for time/day based switch added|  
| 1.3.0 | Migration SPIFFS to LittleFS (not published release)|  
| 1.2.0 | MQTT enabled and calculation factor added to config file|  
| 1.1.0 | Modbus registers config in external file|  
| 1.0.1 | Initial version, based on OTGW-firmware from Robert van den Breemen|  
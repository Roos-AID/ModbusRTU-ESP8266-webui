# ModbusRTU-ESP8266-webui firmware



The features of this ModbusRTU-ESP8266-webui firmware are:
- Implementing the Modbus RTU protocol on the NodeMCU (ESP8266) to display in webUI and publish to MQTT
- Registers and formats to be read can be configured in config file Modbusmap.cfg (see file for spec)
- Dynamic MQ Autoconfigure for homeassistant based on registers defined in Modbusmap.cfg
- When MQ is enabled, then read registers are send to MQTT when enabled in Modbusmap.cfg

- Register values can be converted by a set factor in the config file for display in Webui and MQTT (eg. read Wh and display kWh)
- Warning : Conversion of int values are rounded to int when factor is applied

- Enable telnet listening (for debugging and some commands, enter h for help)
- Telnet commands are t = toggle relay, l = list Daytime map , d = re-read Daytimemap.cfg, m = Configure MQTT Discovery

- a REST API (http://<ip>/api/v1/Modbus/Modbusmonitor
- settings for Hostname, MQTT and NTP in the webUI 
- settings for Modbus baudrate, RTU slave address and read interval in the webUI

- Time/day based setting to enable external relay (GPIO) to allow for day/night energy monitoring/use
- Initially designed to read Jantitza UMG96RM energy monitor registers and display in webUI


To do:
- Documentation (hardware schematic, configuration and operation)
- InfluxDB client to do direct logging
- Update of specific registers through MQTT (low prio)



Looking for the documentation, go here (work in progress):  TODO
 Version 	 Release notes 
 1.6.4	Read interval timer in settings
 1.6.3	Conversion of factor also for MQTT values, plus smaller bug fixes
 1.6.2	 Attempt to get Autoconfigure working for Openhab
 1.6.1	 Dynamic MQ Autoconfigure from the Modbusmap.cfg file
 1.6.0	 Implement MQTT Uniqueid setting, additional debug options
 1.5.0 	 Rename and cleanup, added Relay Allways On switch
 1.4.4 	 MQ updates after reading Modbus register
 1.4.3 	 MQ Pub/Sub name space repaired
 1.4.2 	 Typo in debug info, additional telnet command (re-read Daytimemap
 1.4.1 	 Cosmetic changes, cleanup relay function
 1.4.0 	 GPIO for time/day based switch added
 1.3.0 	 Migration SPIFFS to LittleFS (not published release) 
 1.2.0 	 MQTT enabled and calculation factor added to config file-
 1.1.0 	 Modbus registers config in external file
 1.0.1 	 Initial version, based on OTGW-firmware from Robert van den Breemen
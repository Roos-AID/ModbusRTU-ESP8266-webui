# ModbusRTU-webui firmware



The features of this ModbusRTU-webui firmware are:
- Implementing the Modbus RTU protocol on the NodeMCU (8266)
- Initially designed to read Jantitza UMG96RM energy monitor registers and display in webUI
- enable telnet listening (for debugging and some commands)
- a REST API (http://<ip>/api/v1/Modbus/Modbusmonitor
- settings for Hostname, MQTT and NTP in the webUI 
- settings for Modbus baudrate, RTU slave address in the webUI
- Registers and formats to be read can be configured in config file Modbusmap.cfg 
- Register values can be converted by a set factor in the config file for display in Webui (eg. read Wh and display kWh)
- Time/day based GPIO setting to enable external relay to allow for day/night energy monitoring/use
- Telnet commands are s = switch relay (to test function), t = list Daytime map , r = re-read Daytimemap.cfg

To do:
- InfluxDB client to do direct logging
- Update of specific registers through MQTT (low prio)



Looking for the documentation, go here (work in progress):  TODO

| Version | Release notes |
| 1.5.0 | Rename and cleanup, added Relay Allways On switch|
| 1.4.4 | MQ updates after reading Modbus register|
| 1.4.3 | MQ Pub/Sub name space repaired|
| 1.4.2 | Typo in debug info, additional telnet command (re-read Daytimemap|
| 1.4.1 | Cosmetic changes, cleanup relay function|
| 1.4.0 | GPIO for time/day based switch added|
| 1.3.0 | Migration SPIFFS to LittleFS (not published release) |
| 1.2.0 | MQTT enabled and calculation factor added to config file-|
| 1.1.0 | Modbus registers config in external file|
| 1.0.1 | Initial version, based on OTGW-firmware from Robert van den Breemen|

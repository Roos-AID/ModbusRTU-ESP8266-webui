# Modbus-Janitza firmware



The features of this Modbus-Janitza firmware are:
- Implementing the Modbus protocol on the NodeMCU (8266)
- Reading the Jantitza UMG96RM registers, display in webUI
- enable telnet listening (interpreted data and debugging)
- a REST API (http://<ip>/api/v1/Modbus/Modbusmonitor
- settings for Hostname and MQTT (todo) in the webUI (just compile and edit in webUI)
- settings for Modbus baudrate, RTU slave address in the webUI

To do:
- InfluxDB client to do direct logging 
- MQTT client to send to MQTT
- Cleanup code remains from otgw-firmware


Looking for the documentation, go here (work in progress):  TODO

| Version | Release notes |
|-|-|
| 1.0.0 | Initial version, based on OTGW-firmware from Robert van Breemen|

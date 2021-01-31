/*
***************************************************************************
**  Program  : Header file: ModbusStuff.h
**  Version 1.0.1
**
**  Copyright (c) 2021 Rob Roos
**     based on Framework ESP8266 from Willem Aandewiel and modifications
**     from Robert van Breemen
**
**  TERMS OF USE: MIT License. See bottom of file.
***************************************************************************
*/
#include <ModbusRTU.h>
#include "Debug.h"
// Modbus RTU Specific API
//
// bool begin(SoftwareSerial* port, int16_t txPin=-1, bool direct=true); // For ESP8266 only
// bool begin(HardwareSerial* port, int16_t txPin=-1, bool direct=true);
// bool begin(Stream* port);
// Assing Serial port. txPin controls transmit enable for MAX-485. Pass direct=false if txPin uses inverse logic.
//MODBUS Settings
//#define SLAVE_ID 2
//#define FIRST_REG 0
//#define REG_COUNT 2
#define MODBUS_RX 13   // RX  D7 = GPIO 13
#define MODBUS_TX 14   // TX  D5 = GPIO 14
#define MODBUS_RXTX 12 // TX Enable D6 = GPIO 12 , use in Modbus.begin(*Serial,RXTX_PIN)
// #define MbBdRate 38400


#if defined(ESP8266)
 #include <SoftwareSerial.h>
 // SoftwareSerial S(D1, D2, false, 256);

 // receivePin, transmitPin, inverse_logic, bufSize, isrBufSize
 // connect RX to D2 (GPIO4, Arduino pin 4), TX to D1 (GPIO5, Arduino pin 4)
 // SoftwareSerial S(4, 5);
#endif

// Callback Modbus
//
//  ResultCode
//            EX_SUCCESS              = 0x00, // Custom. No error
//            EX_ILLEGAL_FUNCTION     = 0x01, // Function Code not Supported
//            EX_ILLEGAL_ADDRESS      = 0x02, // Output Address not exists
//            EX_ILLEGAL_VALUE        = 0x03, // Output Value not in Range
//            EX_SLAVE_FAILURE        = 0x04, // Slave or Master Device Fails to process request
//            EX_ACKNOWLEDGE          = 0x05, // Not used
//            EX_SLAVE_DEVICE_BUSY    = 0x06, // Not used
//            EX_MEMORY_PARITY_ERROR  = 0x08, // Not used
//            EX_PATH_UNAVAILABLE     = 0x0A, // Not used
//            EX_DEVICE_FAILED_TO_RESPOND = 0x0B, // Not used
//            EX_GENERAL_FAILURE      = 0xE1, // Custom. Unexpected master error
//            EX_DATA_MISMACH         = 0xE2, // Custom. Inpud data size mismach
//            EX_UNEXPECTED_RESPONSE  = 0xE3, // Custom. Returned result doesn't mach transaction
//            EX_TIMEOUT              = 0xE4, // Custom. Operation not finished within reasonable time
//            EX_CONNECTION_LOST      = 0xE5, // Custom. Connection with device lost
//            EX_CANCEL               = 0xE6  // Custom. Transaction/request canceled
//



#define MODBUS_COMMAND_TOPIC "command"

typedef struct {
	uint16_t 	LastResult = 0;
  uint16_t  ModbusErrors = 0;
} ModbusdataStruct;

static ModbusdataStruct ModbusdataObject;


// Modbus register types  and registeradresses


  enum Modbustype_t {  Modbus_short, Modbus_ushort, Modbus_int, Modbus_uint, Modbus_float, Modbus_undef};
  enum Modbusmsgcmd_t { Modbus_READ, Modbus_RW, Modbus_UNDEF };

  struct Modbuslookup_t
    {
        int id;
        Modbusmsgcmd_t msg;
        Modbustype_t type;
        int reg;
        char* label;
        int phase;           // 0 = generic ,  4 = sum
        char* friendlyname;
        char* unit;
        uint16_t Modbus_short;
        float Modbus_float;
    };

#define MODBUSCOUNT 24

    Modbuslookup_t Modbusmap[MODBUSCOUNT] = {
        {  0, Modbus_READ  , Modbus_short, 0, "ErrCount", 0, "Count Read Errors", "" , 0 , 0 },
        {  1, Modbus_READ  , Modbus_float, 19000, "UL1N", 1, "Voltage L1-N", "V" , 0 , 0 },
        {  2, Modbus_READ  , Modbus_float, 19002, "UL2N", 2, "Voltage L2-N", "V" , 0 , 0 },
        {  3, Modbus_READ  , Modbus_float, 19004, "UL3N", 3, "Voltage L3-N", "V" , 0 , 0 },
        {  4, Modbus_READ  , Modbus_float, 19006, "UL1L2", 1, "Voltage L1-L2", "V" , 0 , 0 },
        {  5, Modbus_READ  , Modbus_float, 19008, "UL2L3", 2, "Voltage L2-L3", "V" , 0 , 0 },
        {  6, Modbus_READ  , Modbus_float, 19010, "UL3L1", 3, "Voltage L3-L1", "V" , 0 , 0 },
        {  7, Modbus_READ  , Modbus_float, 19012, "CurL1", 1, "Current L1", "A" , 0 , 0 },
        {  8, Modbus_READ  , Modbus_float, 19014, "CurL2", 2, "Current L2", "A" , 0 , 0 },
        {  9, Modbus_READ  , Modbus_float, 19016, "CurL3", 3, "Current L3", "A" , 0 , 0 },
        { 10, Modbus_READ  , Modbus_float, 19018, "CurVSum", 4, "Vector sum; IN=I1+I2+I3", "A" , 0 , 0 },
        { 11, Modbus_READ  , Modbus_float, 19020, "RlPwrL1", 1, "Real power L1", "W" , 0 , 0 },
        { 12, Modbus_READ  , Modbus_float, 19022, "RlPwrL2", 2, "Real power L2", "W" , 0 , 0 },
        { 13, Modbus_READ  , Modbus_float, 19024, "RlPwrL3", 3, "Real power L3", "W" , 0 , 0 },
        { 14, Modbus_READ  , Modbus_float, 19026, "RlPwrSum", 4, "Sum; Psum3=P1+P2+P3", "W" , 0 , 0 },
        { 15, Modbus_READ  , Modbus_float, 19050, "Freq", 0, "Measured frequency", "Hz" , 0 , 0 },
        { 16, Modbus_READ  , Modbus_float, 19054, "RlEnerL1", 1, "Real energy L1", "Wh" , 0 , 0 },
        { 17, Modbus_READ  , Modbus_float, 19056, "RlEnerL2", 2, "Real energy L2", "Wh" , 0 , 0 },
        { 18, Modbus_READ  , Modbus_float, 19058, "RlEnerL3", 3, "Real energy L3", "Wh" , 0 , 0 },
        { 19, Modbus_READ  , Modbus_float, 19060, "RlEnerSum", 4, "Real energy L1..L3", "Wh" , 0 , 0 },
        { 20, Modbus_READ  , Modbus_float, 19062, "RlEnerConsL1", 1, "Real energy Consumed L1", "Wh" , 0 , 0 },
        { 21, Modbus_READ  , Modbus_float, 19064, "RlEnerConsL2", 2, "Real energy Consumed L2", "Wh" , 0 , 0 },
        { 22, Modbus_READ  , Modbus_float, 19066, "RlEnerConsL3", 3, "Real energy Consumed L3", "Wh" , 0 , 0 },
        { 23, Modbus_READ  , Modbus_float, 19060, "RlEnerConsSum", 4, "Real energy Consumed Sum", "Wh" , 0 , 0 },

  };


bool cb(Modbus::ResultCode event, uint16_t transactionId, void* data) { // Callback to monitor errors
 #ifdef ESP8266
  if (event != 0) {
//    DebugTf("Modbus Request result: 0x%02X, Mem: %d\n", event, ESP.getFreeHeap());
    ModbusdataObject.ModbusErrors = ModbusdataObject.ModbusErrors + 1;
  }
#elif ESP32
  if (event != 0) {
    DebugTf("Modbus Request result: 0x%02X, Mem: %d\n", event, ESP.getFreeHeap());
    ModbusdataObject.ModbusErrors = ModbusdataObject.ModbusErrors + 1;
  }
#else
   if (event != 0) {
      DebugTf("Modbus Request result: 0x");
      DebugTf(event, HEX);
      ModbusdataObject.ModbusErrors = ModbusdataObject.ModbusErrors + 1;
   }
#endif

ModbusdataObject.LastResult = event ;
//  if (event != 0) {
//    Debugln("Modbus LastResult not 0") ;
//  } else {
//    Debugln("Modbus LastResult OK") ;
//  }
  return true;
}


/***************************************************************************
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to permit
* persons to whom the Software is furnished to do so, subject to the
* following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT
* OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
* THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
***************************************************************************/

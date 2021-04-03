/*
***************************************************************************
**  Program  : Header file: ModbusStuff.h
**  Version 1.7.0
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
#define MODBUS_RX D7 // RX  D7 = GPIO 13
#define MODBUS_TX D5   // TX  D5 = GPIO 14
#define MODBUS_RXTX D6 // TX Enable D6 = GPIO 12 , use in Modbus.begin(*Serial,RXTX_PIN)

#define MODBUSCOUNT 40  // max number of registers in config file


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

struct DaytimemapStruct_t
    {
      uint16_t day = 0;
      uint16_t starthour = 0;
      uint16_t startmin = 0;
      uint16_t endhour = 0;
      uint16_t endmin = 0;
    };
DaytimemapStruct_t* Daytimemap;

#define MODBUS_COMMAND_TOPIC "command"

        typedef struct {
	uint16_t 	LastResult = 0;
  uint16_t  ModbusErrors = 0;
  uint16_t  NumberRegisters = 0;
} ModbusdataStruct;


// #include <new>
static ModbusdataStruct ModbusdataObject;


// Modbus register types  and registeradresses

enum Modbusoper_t { Modbus_READ, Modbus_RW, Modbus_UNDEF };
static const char *Modbusoper_str[] = {"Modbus_READ", "Modbus_RW", "Modbus_UNDEF"} ; 
enum Modbusformat_t { Modbus_short, Modbus_ushort, Modbus_int, Modbus_uint, Modbus_float, Modbus_undef };
static const char *Modbusformat_str[] = {"Modbus_short", "Modbus_ushort", "Modbus_int", "Modbus_uint", "Modbus_float", "Modbus_undef"};

struct Modbuslookup_t
    {
        uint16_t id;
        Modbusoper_t oper;  // command RD or RW
        Modbusformat_t regformat;  // type of data
        uint16_t address;  //register address
        uint16_t phase;           // 0 = generic ,  4 = sum
        int16_t Modbus_short;
        uint16_t Modbus_ushort;
        int32_t Modbus_int;
        uint32_t Modbus_uint;
        float Modbus_float;
        float factor;
        uint16_t mqenable;
        char* label;
        char* friendlyname;
        char* unit;
        char* devclass;
    };

Modbuslookup_t* Modbusmap;


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

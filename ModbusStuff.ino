/*
***************************************************************************
**  Program  : ModbusStuff
**  Version 1.0.0
**
**  Copyright (c) 2021 Rob Roos
**     based on Framework ESP8266 from Willem Aandewiel and modifications
**     from Robert van Breemen
**
**  TERMS OF USE: MIT License. See bottom of file.
***************************************************************************
*/
//====================[ HandleModbus ]====================
/*
** This is the core of the Modbus firmware.
** This code basically reads from serial, connected to the Modbus hardware, and
** processes each Modbus request.
**
*/

// Setup ModbusRTU and Software Serial

SoftwareSerial S(MODBUS_RX, MODBUS_TX);
ModbusRTU mb;

float f32(uint16_t u1, uint16_t u2)
{
  long int num = u1 << 16 | u2;
  float numf;
  memcpy(&numf, &num, 4);
  return numf;
}

//============== Setup Modbus ======================================
// SoftwareSerial S(D1, D2, false, 256);

 // receivePin, transmitPin, inverse_logic, bufSize, isrBufSize
 // connect RX to D2 (GPIO4), TX to D1 (GPIO5)

void setupModbus()
{

    Debugf("Init Serial with baudrate:\r\n");
    Debugln(settingModbusBaudrate);

    #if defined(ESP8266)

      S.begin(settingModbusBaudrate, SWSERIAL_8N1, MODBUS_RX, MODBUS_TX);
      mb.begin(&S,MODBUS_RXTX);
    #elif defined(ESP32)
      Serial1.begin(settingModbusBaudrate, SERIAL_8N1);
      mb.begin(&Serial1,MODBUS_RXTX);
    #else
      Serial1.begin(settingModbusBaudrate, SERIAL_8N1);
      mb.begin(&Serial1,MODBUS_RXTX);
      mb.setBaudrate(settingModbusBaudrate);
    #endif

    mb.master();
    Debugf("Modbus Serial init completed\r\n");
}



//============ Read register functions (short )

int16_t Modbus_ReadShort(uint16_t readreg) {
//   Debugf("Modbus ReadShort started\r\n");
  uint16_t shortres[2];
  if (!mb.slave()) {    // Check if no transaction in progress

    mb.readHreg(settingModbusSlaveAdr, readreg, shortres, 1, cb); // Send Read Hreg from Modbus Server
    while(mb.slave()) { // Check if transaction is active
      delay(10);
      mb.task();
    }

//   Debugf("Modbus ReadShort Result: 0x%02X , \r\n",ModbusdataObject.LastResult);
   if (ModbusdataObject.LastResult == 0) {

//      for (int i = 0 ; i < 1 ; i++) {
//          Debugf("Reg: %d, Val: %d \r\n", i+readreg, shortres[i]);
//      }
     } else {
          DebugTf("Modbus ReadShort Reg: %d , Result: 0x%02X \r\n",readreg, ModbusdataObject.LastResult);
      }
  }  else {
      DebugTln("Error: Modbus Read while transaction active");
      ModbusdataObject.LastResult = 99 ;
    }

// Debugf("Modbus ReadShort ended\r\n");
 return shortres[0];
}


//============ Read register functions (Float )

float Modbus_ReadFloat(uint16_t readreg) {
//   Debugf("Modbus ReadFloat started\r\n");

  uint16_t floatres[2];
  float returnfloat;

  if (!mb.slave()) {      // Check if no transaction in progress

    mb.readHreg(settingModbusSlaveAdr, readreg, floatres, 2, cb); // Send Read Hreg from Modbus Server
    while(mb.slave()) { // Check if transaction is active
      delay(10);
      mb.task();
//      Debugf("Modbus ReadFloat delay\r\n");
    }

//   Debugf("Modbus ReadFloat Result: 0x%02X , \r\n",ModbusdataObject.LastResult);

   if (ModbusdataObject.LastResult == 0) {

//      for (int i = 0 ; i < 2 ; i++) {
//          Debugf("Reg: %d, Val: %d \r\n", i+readreg, floatres[i]);
//        }
      returnfloat = f32(floatres[0],floatres[1]) ;
      } else {
         DebugTf("Modbus ReadFloat Reg: %d , Result: 0x%02X , \r\n",readreg, ModbusdataObject.LastResult);
      }

   } else {
      DebugTln("Error: Modbus Read while transaction active");
      ModbusdataObject.LastResult = 99 ;
   }

// Debugf("Modbus ReadFloat ended \r\n");
 return returnfloat;
}


void readModbus()
{
    float TempFloat ;
    int16_t TempShort ;
    bool Noerror = true ;

//    Debugf("readModbus started\r\n");

    for (int i = 1; i < MODBUSCOUNT ; i++) {
       if (settingModbusSinglephase == 0 || Modbusmap[i].phase == 0 || Modbusmap[i].phase == 1) {
          switch (Modbusmap[i].type) {
            case Modbus_short:
               TempShort = Modbus_ReadShort(Modbusmap[i].reg) ;

              if (ModbusdataObject.LastResult == 0) {
                 Modbusmap[i].Modbus_short = TempShort ;
               }
               break;
            case Modbus_ushort:
              DebugTf("Not implemented %d = %s \r\n", i, Modbusmap[i].label) ;
              break;
            case Modbus_int:
              DebugTf("Not implemented %d = %s \r\n", i, Modbusmap[i].label) ;
              break;
            case Modbus_uint:
              DebugTf("Not implemented %d = %s \r\n", i, Modbusmap[i].label) ;
              break;
            case Modbus_float:
              TempFloat = Modbus_ReadFloat(Modbusmap[i].reg) ;

              if (ModbusdataObject.LastResult == 0) {
                 Modbusmap[i].Modbus_float = TempFloat ;
               }
               break;
            case Modbus_undef:
              DebugTf("Error undef type %d = %s \r\n", i, Modbusmap[i].label) ;
              break;
            default:
              DebugTf("Error undef type %d = %s \r\n", i, Modbusmap[i].label) ;
              break;
          }
          if (ModbusdataObject.LastResult != 0) { Noerror = false ;  }
       }
//      doBackgroundTasks();
        yield();
    }

    if (!Noerror) { DebugTf("readModbus ended with error Mem: %d \r\n",ESP.getFreeHeap()); }
}

void readModbusSetup()
{

  Debugf("readModbus2 started\r\n");


  uint16_t offset = 0;
  uint16_t count = 8;
  uint16_t resval[8];  // set count value !!

 if (!mb.slave()) {    // Check if no transaction in progress
   // mb.readHreg(SLAVE_ID, FIRST_REG, res, REG_COUNT, cb); // Send Read Hreg from Modbus Server
   mb.readHreg(settingModbusSlaveAdr, offset, resval, count, cb); // Send Read Hreg from Modbus Server

   while(mb.slave()) { // Check if transaction is active
 //    Debugf("readModbus slave true\r\n");
     mb.task();
     delay(10);
   }
   Debugf("readModbus2 result\r\n");
   for (int i = 0 ; i < count ; i++) {
      Debugf("Reg: %d, Val: %d \r\n", i+offset, resval[i]);
      // Debugln(resval[i]);
   }
 }
 Debugf("readModbus ended \r\n");
}


int sendModbus(const char* buf, int len)
 {
   //Just send the buffer to Modbus when the Serial interface is available
   // Needs to be Mudbus RTU !!!
   //

   DebugTf("sendModbus len: [%s] buf: [%s]\r\n",len,buf);
 //  if (Serial) {
 //    //check the write buffer
 //    Debugf("Serial Write Buffer space = [%d] - needed [%d]\r\n",Serial.availableForWrite(), (len+2));
 //    DebugT("Sending to Serial [");
 //    for (int i = 0; i < len; i++) {
 //      Debug((char)buf[i]);
 //    }
 //    Debug("] ("); Debug(len); Debug(")"); Debugln();
 //
 //    if (Serial.availableForWrite()>= (len+2)) {
 //      //write buffer to serial
 //      Serial.write(buf, len);
 //      // Serial.write("PS=0\r\n");
 //      Serial.write('\r');
 //      Serial.write('\n');
 //    } else Debugln("Error: Write buffer not big enough!");
 //  } else Debugln("Error: Serial device not found!");
 }




//====================[ functions for REST API ]====================
String getModbusValue(int modbusreg)
{

   return "Modbus not implemented yet!";
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

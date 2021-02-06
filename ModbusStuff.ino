/*
***************************************************************************
**  Program  : ModbusStuff
**  Version 1.1.1
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

void toMQTT_float(int id)
{
  //function to push float data to MQTT
  float _value = round(Modbusmap[id].Modbus_float * 100.0) / 100.0; // round float 2 digits, like this: x.xx
 
  char _msg[15]{0};
  dtostrf(_value, 3, 2, _msg);
  // DebugTf("To MQTT_float %s %s %s\r\n", Modbusmap[id].label, _msg, Modbusmap[id].unit);
  //SendMQTT
  sendMQTTData(Modbusmap[id].label, _msg);
  
}
void toMQTT_short(int id)
{
  //function to push float data to MQTT
  int16_t _value = Modbusmap[id].Modbus_short;

  char _msg[15]{0};
  itoa(_value, _msg, 10);
  // DebugTf("To MQTT_short %s %s %s\r\n", Modbusmap[id].label, _msg, Modbusmap[id].unit);
  //SendMQTT
  sendMQTTData(Modbusmap[id].label, _msg);
}

void Modbus2MQTT() {
  for (int i = 1; i <= ModbusdataObject.NumberRegisters; i++)
  {
    if (Modbusmap[i].mqenable == 1) {
      if (settingModbusSinglephase == 0 || Modbusmap[i].phase == 0 || Modbusmap[i].phase == 1 || Modbusmap[i].phase == 4)
      {
        switch (Modbusmap[i].regformat)
        {
        case Modbus_short:
          toMQTT_short(i);
          break;
        case Modbus_ushort:
          DebugTf("MQTT Not implemented for %d = %s \r\n", i, Modbusmap[i].label);
          break;
        case Modbus_int:
          DebugTf("MQTT Not implemented for %d = %s \r\n", i, Modbusmap[i].label);
          break;
        case Modbus_uint:
          DebugTf("MQTT Not implemented for %d = %s \r\n", i, Modbusmap[i].label);
          break;
        case Modbus_float:
          toMQTT_float(i);
          break;
        case Modbus_undef:
          DebugTf("MQTT Not implemented for %d = %s \r\n", i, Modbusmap[i].label);
          break;
        default:
          DebugTf("MQTT Error undef type %d = %s \r\n", i, Modbusmap[i].label);
          break;
        }
      }
    }   
  }

  
}

void readModbus()
{
    float TempFloat ;
    int16_t TempShort ;
    bool Noerror = true ;

//    Debugf("readModbus started\r\n");

    for (int i = 1; i <= ModbusdataObject.NumberRegisters ; i++) {
       if (settingModbusSinglephase == 0 || Modbusmap[i].phase == 0 || Modbusmap[i].phase == 1 || Modbusmap[i].phase == 4) {
          switch (Modbusmap[i].regformat) {
            case Modbus_short:
               TempShort = Modbus_ReadShort(Modbusmap[i].address) ;

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
              TempFloat = Modbus_ReadFloat(Modbusmap[i].address) ;

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

 // =============TO DO==============================================================================
 //===========================================================================================


 void doInitModbusMap()
 {

   const char* cfgFilename = "/Modbusmap.cfg";
   // Configuration file to map modbus registers
   // file format is CSV , all values as string without "" ,  fields are
   // reg = register address
   // format = (Modbus_short, Modbus_ushort, Modbus_int, Modbus_uint, Modbus_float, Modbus_undef)  (Only short and float implemented in rel 1)
   // operation = (Modbus_READ, Modbus_RW, Modbus_UNDEF)  (Only Modbus_READ implemented in rel 1)
   // label = Short label string
   // friendlyname = string in UI
   // unit = V, A, wH etc.
   // phase = 1,2,3 or 0 for generic and 4 for sum
   // factor = multiplication/division factor to apply. Specify 1 for no conversion , eg. 1000 for Wh to kWh
   // mqenable = set to 1 to enable sending to MQTT
   // reg, format, operation, label, friendlyname, unit, phase, factor, mqenable
   // 19000, Modbus_short, Modbus_READ, UL1N, Voltage L1-N, V, 1, 1, 1

   Modbusmap  = new  Modbuslookup_t[MODBUSCOUNT];

   int id = 0;
   int Index1, Index2, Index3, Index4, Index5, Index6, Index7, Index8 ;
   String sReg;
   String sFormat;
   String sOper;
   String sLabel;
   String sName;
   String sUnit;
   String sPhase;
   String sFactor;
   String sMQEnable;

   String stName;
   String stLabel;
   String stUnit;


   File fh; //filehandle
   //Let's open the Modbus config file
   SPIFFS.begin();
   if (SPIFFS.exists(cfgFilename))
   {
     fh = SPIFFS.open(cfgFilename, "r");
     if (fh) {
       //Lets go read the config and store in modbusmap line by line
       while(fh.available() && id <= MODBUSCOUNT)
       {  //read file line by line, split and send to MQTT (topic, msg)
          String sLine = fh.readStringUntil('\n');

          if (sLine.startsWith("//") )
          {
            DebugTf("Either comment or invalid config line: [%s]\r\n", sLine.c_str());
          } else {
            DebugTf("sline[%s]\r\n", sLine.c_str());
            // reg, format, type, label, friendlyname, unit, phase
            Index1 = sLine.indexOf(',');
            Index2 = sLine.indexOf(',', Index1 + 1);
            Index3 = sLine.indexOf(',', Index2 + 1);
            Index4 = sLine.indexOf(',', Index3 + 1);
            Index5 = sLine.indexOf(',', Index4 + 1);
            Index6 = sLine.indexOf(',', Index5 + 1);
            Index7 = sLine.indexOf(',', Index6 + 1);
            Index8 = sLine.indexOf(',', Index7 + 1);

            sReg     = sLine.substring(0, Index1);
            sFormat  = sLine.substring(Index1 + 1, Index2);
            sOper    = sLine.substring(Index2 + 1, Index3);
            sLabel   = sLine.substring(Index3 + 1, Index4);
            sName    = sLine.substring(Index4 + 1, Index5);
            sUnit    = sLine.substring(Index5 + 1, Index6);
            sPhase   = sLine.substring(Index6 + 1, Index7);
            sFactor  = sLine.substring(Index7 + 1, Index8);
            sMQEnable = sLine.substring(Index8 + 1);
            sReg.trim();
            sFormat.trim();
            sOper.trim();
            sLabel.trim();
            sName.trim();
            sUnit.trim();
            sPhase.trim();
            sFactor.trim();
            sMQEnable.trim();

          //  DebugTf("sReg[%s], sFormat[%s], sRegoper[%s], sLabel[%s], sName[%s], sUnit[%s], sPhase[%s], sFactor[%s], sMQEnable[%s]\r\n", sReg.c_str(), sFormat.c_str(), sOper.c_str(), sLabel.c_str(), sName.c_str(), sUnit.c_str(), sPhase.c_str(),sFactor.c_str(),sMQEnable.c_str());
          //  delay(10);

            id++;

            Modbusmap[id].id = id ;
            if (sOper == "Modbus_READ" ) {
                Modbusmap[id].oper = Modbus_READ ;
//                DebugTln("Modbus_READ detected");
            } else if (sOper == "Modbus_RW" ) {
                Modbusmap[id].oper = Modbus_RW ;
            } else {
              Modbusmap[id].oper = Modbus_UNDEF ;
//              DebugTln("Modbus_UNDEF detected");
            }

            if (sFormat == "Modbus_short" ) {
                Modbusmap[id].regformat = Modbus_short ;
//                DebugTln("Modbus_Short detected");
            } else if (sFormat == "Modbus_float" ) {
                Modbusmap[id].regformat = Modbus_float ;
            } else {
              Modbusmap[id].regformat= Modbus_undef ;
//              DebugTln("Modbus_undef detected");
            }

            // assign values to Modbusmap
             Modbusmap[id].address = sReg.toInt() ;
             Modbusmap[id].phase = sPhase.toInt() ;
             Modbusmap[id].factor = sFactor.toFloat();
             Modbusmap[id].mqenable = sMQEnable.toInt();

             char *clabel = new char[sLabel.length() + 1];
             char *cname = new char[sName.length() + 1];
             char *cunit = new char[sUnit.length() + 1];
             strcpy(clabel, sLabel.c_str());
             strcpy(cname, sName.c_str());
             strcpy(cunit, sUnit.c_str());
             Modbusmap[id].label = clabel ;
             Modbusmap[id].friendlyname = cname ;
             Modbusmap[id].unit = cunit;
            // delete [] clabel;  // do not delete, is required during runtime !
            // delete [] cname;
            // delete [] cunit;

             Modbusmap[id].Modbus_short = 9999;
             Modbusmap[id].Modbus_float = 9999;
          }

       } // while available()

       ModbusdataObject.NumberRegisters = id ;
       DebugTf("Number of Modbus registers initialized:, %d \r\n", ModbusdataObject.NumberRegisters);
       fh.close();

     }
   }
 }

void printModbusmap() {
  DebugTf("printModbusmap begin for: %d, records", ModbusdataObject.NumberRegisters) ;
  for (int i = 1; i < ModbusdataObject.NumberRegisters ; i++) {
    // Check if multiphase, if singlephase (1) then onlys show generic (0) or phase 1.

       DebugTf("Record: %d, id %d, oper: %d, format: %d ", i , Modbusmap[i].id, Modbusmap[i].oper, Modbusmap[i].regformat);
       DebugTf("Address: %d, phase: %d, Valuefloat %f \r\n", Modbusmap[i].address ,Modbusmap[i].phase, Modbusmap[i].Modbus_float);
       DebugTf("Label: %s, Friendlyname %s, Unit: %s ", Modbusmap[i].label, Modbusmap[i].friendlyname, Modbusmap[i].unit);
       DebugTf("Factor: %f, MQEnable %d \r\n", Modbusmap[i].factor, Modbusmap[i].mqenable, Modbusmap[i].unit);
  }
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

/*
***************************************************************************
**  Program  : ModbusStuff
**  Version 1.7.5
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


// Convert 2 16 bit registers to float 32bit value
float cf32(uint16_t u1, uint16_t u2)
{
  long int num = u1 << 16 | u2;
  float numf;
  memcpy(&numf, &num, 4);
  return numf;
}

// Convert 2 16 bit registers to int 32bit value
int32_t cfint32(uint16_t u1, uint16_t u2)
{
  long int num = u1 << 16 | u2;
  int32_t numint;
  memcpy(&numint, &num, 4);
  return numint;
}

// Convert 2 16 bit registers to uint 32bit value
uint32_t cfuint32(uint16_t u1, uint16_t u2)
{
  long int num = u1 << 16 | u2;
  uint32_t numuint;
  memcpy(&numuint, &num, 4);
  return numuint;
}

//============== Setup Modbus ======================================
// SoftwareSerial S(D1, D2, false, 256);

 // receivePin, transmitPin, inverse_logic, bufSize, isrBufSize
 // connect RX to D2 (GPIO4), TX to D1 (GPIO5)

void setupModbus()
{

  if (bDebugMBmsg) Debugf("Init Serial with baudrate:\r\n");
  if (bDebugMBmsg) Debugln(settingModbusBaudrate);

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
void waitMBslave() {
    while (mb.slave())
    { // Check if Modbus transaction is active, then wait
      delay(10);
      mb.task();    
    }
}


void MBtestreadHreg(uint16_t MBmapidx, uint16_t MBaddress, uint16_t value[], uint16_t numregs) {
    DebugTf("Args MBtestreadHreg MBmapidx[%d], MBaddress[%d] , .address[%d], numregs[%d]\r\n ",MBmapidx, MBaddress, Modbusmap[MBmapidx].address,numregs);
    ModbusdataObject.LastResult = 0;
    switch (Modbusmap[MBmapidx].regformat)
    {
    case Modbus_short:
      if (numregs != 1) { DebugTln("ERROR numregs read for short should be 1"); break; }
       switch (Modbusmap[MBmapidx].address) 
       {
       case 1: 
       value[0] = 0x04d2; 
       break; // 1234
       case 2: 
       value[0] = 0xfb2e; 
       break; // -1234 
       default : 
       value[0] = 1111; 
       }
      break; 
    case Modbus_ushort:
      if (numregs != 1) { DebugTln("ERROR numregs read for ushort should be 1"); break; }
      switch (Modbusmap[MBmapidx].address)
      {
      case 3:  
      value[0] = 0x04d2; // 1234 
      break;
      case 4:  
      value[0] = 0xfb2e; // 64302
      break;
      default: 
      value[0] = 2222;    
      }
      break;
    case Modbus_int:
      if (numregs != 2) { DebugTln("ERROR numregs read for int should be 2"); break; }
      switch (Modbusmap[MBmapidx].address)
      {
      case 5:  
        value[0] = 0x0000; 
        value[1] = 0x04d2; // 1234 
        break;
      case 6:  
        value[0] = 0xffff; 
        value[1] = 0xfb2e; // -1234
        break;
      default: 
        value[0] = 0; 
        value[1] = 3333; 
      }
      break;
    case Modbus_uint:
      if (numregs != 2) { DebugTln("ERROR numregs read for uint should be 2"); break;}
      switch (Modbusmap[MBmapidx].address)
      {
      case 7:  
        value[0] = 0x0000;
        value[1] = 0x04d2; // 1234 
        break;
      case 8: 
        value[0] = 0xffff; 
        value[1] = 0xfb2e; // 4294966062
        break;
      default: 
        value[0] = 0; 
        value[1] = 4444; 
      }
      break;
    case Modbus_float:
      if (numregs != 2) { DebugTln("ERROR numregs read for float should be 2"); }
      switch (Modbusmap[MBmapidx].address)
      {
      case 9:  
        value[0] = 0x47f1; 
        value[1] = 0x2000;  // 123456
        break;
      case 10: 
        value[0] = 0xc7f1; 
        value[1] = 0x2000; 
        break;
      default: 
        value[0] = 0x4759; 
        value[1] = 0x0300; // 55555
      }
      break;
    case Modbus_string:
      // todo 
        for (int i = 0 ; i < numregs ; i++) {
          value[i] = 0x424d;  // MB 
        }
     break;
    }
    // DebugTf("########## MBmapidx[%d] Value0[%d] value1[]\r\n",MBmapidx,value[0],value[1]) ;
}

//============ Read register functions V2 (short )
bool Modbus_Read_short(uint16_t i)
{
  uint16_t shortres[2];
  

  bool tempError = false;

  if (!mb.slave())  
  { 
    if (!bDebugMBlogic) { 
      mb.readHreg(settingModbusSlaveAdr, Modbusmap[i].address, shortres, 1, cb); // Send Read Hreg from Modbus Server
      waitMBslave();
    } else MBtestreadHreg(i, Modbusmap[i].address, shortres, 1 ) ;

    if (ModbusdataObject.LastResult == 0)
    {
      //  determine value from multiple registers for (int i = 0 ; i < 1 ; i++) { Debugf("Reg: %d, Val: %d \r\n", i+readreg, shortres[i]); }
      mb_convert.mb_uint16 = shortres[0] ;
      if (bDebugMBlogic) DebugTf("shortres[%d] md_uint16 [%d] md_int16 [%d]\r\n",shortres[0], mb_convert.mb_uint16, mb_convert.mb_int16 );
      // Convert when factor is set other than 1
      if (Modbusmap[i].factor != 1)
        Modbusmap[i].Modbus_short = round(mb_convert.mb_int16 * Modbusmap[i].factor);
      else
        Modbusmap[i].Modbus_short = mb_convert.mb_int16;
      if (bDebugMBmsg)
        DebugTf("Modbus Read short Reg:[%d] , Result:[%d]\r\n", Modbusmap[i].address, Modbusmap[i].Modbus_short);
      if (settingMQTTenable && Modbusmap[i].mqenable == 1)
      {
        toMQTT_short(i);
      }
    }
    else
    {
      DebugTf("Modbus Read short Reg:[%d] , Result: 0x%02X \r\n", Modbusmap[i].address, ModbusdataObject.LastResult);
      tempError = true;
    }
  }
  else
  {
    // this should never happen
    DebugTln("Error: Modbus Read while transaction active");
    ModbusdataObject.LastResult = 99;
  }

  return tempError;
}

//============ Read register functions V2 (ushort )
bool Modbus_Read_ushort(uint16_t i)
{
  uint16_t ushortres[2];
  bool tempError = false;

  if (!mb.slave())
  { 

    if (!bDebugMBlogic) { 
      mb.readHreg(settingModbusSlaveAdr, Modbusmap[i].address, ushortres, 1, cb); // Send Read Hreg from Modbus Server
      waitMBslave();
    } else  MBtestreadHreg(i, Modbusmap[i].address, ushortres, 1 ) ;

    if (ModbusdataObject.LastResult == 0)
    {
      //  determine value from multiple registers for (int i = 0 ; i < 1 ; i++) { Debugf("Reg: %d, Val: %d \r\n", i+readreg, shortres[i]); }
      if (bDebugMBlogic) DebugTf("ushortres0[%d] \r\n",ushortres[0]);
      // Convert when factor is set other than 1
      if (Modbusmap[i].factor != 1)
        Modbusmap[i].Modbus_ushort = round(ushortres[0] * Modbusmap[i].factor);
      else
        Modbusmap[i].Modbus_ushort = ushortres[0];
      if (bDebugMBmsg)
        DebugTf("Modbus Read ushort Reg:[%d] , Result:[%u]\r\n", Modbusmap[i].address, Modbusmap[i].Modbus_ushort);
      if (settingMQTTenable && Modbusmap[i].mqenable == 1)
      {
        toMQTT_ushort(i);
      }
    }
    else
    {
      DebugTf("Modbus Read ushort Reg:[%d] , Result: 0x%02X \r\n", Modbusmap[i].address, ModbusdataObject.LastResult);
      tempError = true;
    }
  }
  else
  {
    // this should never happen
    DebugTln("Error: Modbus Read while transaction active");
    ModbusdataObject.LastResult = 99;
  }

  return tempError;
}

//============ Read register functions V2 (int)
bool Modbus_Read_int(uint16_t i)
{
  uint16_t intres[2];
  bool tempError = false;
  int32_t tempint = 0;

  if (!mb.slave())
  { 
    if (!bDebugMBlogic) { 
      mb.readHreg(settingModbusSlaveAdr, Modbusmap[i].address, intres, 2, cb); // Send Read Hreg from Modbus Server
      waitMBslave();
    } else  MBtestreadHreg(i, Modbusmap[i].address, intres, 2 ) ;
    
    if (ModbusdataObject.LastResult == 0)
    {
      //  determine value from multiple registers for (int i = 0 ; i < 1 ; i++) { Debugf("Reg: %d, Val: %d \r\n", i+readreg, shortres[i]); }
      tempint = cfint32(intres[0], intres[1]);
      if (bDebugMBlogic) DebugTf("intres0[%d] intres1[%d] tempint[%d]\r\n",intres[0], intres[1], tempint );
      // Convert when factor is set other than 1
      if (Modbusmap[i].factor != 1)
          Modbusmap[i].Modbus_int = round(tempint * Modbusmap[i].factor);
      else
        Modbusmap[i].Modbus_int = tempint;
      if (bDebugMBmsg)
        DebugTf("Modbus Read int Reg:[%d] , Result:[%d]\r\n", Modbusmap[i].address, Modbusmap[i].Modbus_int);
      if (settingMQTTenable && Modbusmap[i].mqenable == 1)
      {
        toMQTT_int(i);
      }
    }
    else
    {
      DebugTf("Modbus Read int Reg:[%d] , Result: 0x%02X \r\n", Modbusmap[i].address, ModbusdataObject.LastResult);
      tempError = true;
    }
  }
  else
  {
    // this should never happen
    DebugTln("Error: Modbus Read while transaction active");
    ModbusdataObject.LastResult = 99;
  }

  return tempError;
}

//============ Read register functions V2 (uint)
bool Modbus_Read_uint(uint16_t i)
{
  uint16_t uintres[2];
  bool tempError = false;
  uint32_t tempuint = 0;

  if (!mb.slave())
  {     
    if (!bDebugMBlogic) { 
      mb.readHreg(settingModbusSlaveAdr, Modbusmap[i].address, uintres, 2, cb); // Send Read Hreg from Modbus Server
      waitMBslave();
    } else  MBtestreadHreg(i, Modbusmap[i].address, uintres, 2 ) ;
    
    if (ModbusdataObject.LastResult == 0)
    {
      //  determine value from multiple registers for (int i = 0 ; i < 1 ; i++) { Debugf("Reg: %d, Val: %d \r\n", i+readreg, shortres[i]); }
      tempuint = cfuint32(uintres[0], uintres[1]);
      if (bDebugMBlogic) DebugTf("uintres0[%d] uintres1[%d] tempuint[%d]\r\n",uintres[0], uintres[1], tempuint );
      // Convert when factor is set other than 1
      if (Modbusmap[i].factor != 1)
        Modbusmap[i].Modbus_uint = round(tempuint * Modbusmap[i].factor);
      else
        Modbusmap[i].Modbus_uint = tempuint;
      if (bDebugMBmsg)
        DebugTf("Modbus Read uint Reg:[%d] , Result:[%u]\r\n", Modbusmap[i].address, Modbusmap[i].Modbus_uint);

      if (settingMQTTenable && Modbusmap[i].mqenable == 1)
      {
        toMQTT_uint(i);
      }
    }
    else
    {
      DebugTf("Modbus Read uint Reg:[%d] , Result: 0x%02X \r\n", Modbusmap[i].address, ModbusdataObject.LastResult);
      tempError = true;
    }
  }
  else
  {
    // this should never happen
    DebugTln("Error: Modbus Read while transaction active");
    ModbusdataObject.LastResult = 99;
  }

  return tempError;
}

//============ Read register functions V2 (float
bool Modbus_Read_float(uint16_t i)
{
  uint16_t floatres[2];
  bool tempError = false;
  float tempfloat = 0;

  if (!mb.slave())
  { 
    if (!bDebugMBlogic) { 
      mb.readHreg(settingModbusSlaveAdr, Modbusmap[i].address, floatres, 2, cb); // Send Read Hreg from Modbus Server
      waitMBslave();
    } else  MBtestreadHreg(i, Modbusmap[i].address, floatres, 2 ) ;
    
    if (ModbusdataObject.LastResult == 0)
    {
      //  determine value from multiple registers for (int i = 0 ; i < 1 ; i++) { Debugf("Reg: %d, Val: %d \r\n", i+readreg, shortres[i]); }
      tempfloat = cf32(floatres[0], floatres[1]);
      if (bDebugMBlogic) DebugTf("floatres0[%d] floatres1[%d] tempfloat[%f]\r\n",floatres[0], floatres[1], tempfloat );
      // Convert when factor is set other than 1
      if (Modbusmap[i].factor != 1)
        Modbusmap[i].Modbus_float = tempfloat * Modbusmap[i].factor;
      else
        Modbusmap[i].Modbus_float = tempfloat;
      if (bDebugMBmsg)
        DebugTf("Modbus Read float Reg:[%d] , Result:[%f]\r\n", Modbusmap[i].address, Modbusmap[i].Modbus_float);

      if (settingMQTTenable && Modbusmap[i].mqenable == 1)
      {
        toMQTT_float(i);
      }
    }
    else
    {
      DebugTf("Modbus Read float Reg:[%d] , Result: 0x%02X \r\n", Modbusmap[i].address, ModbusdataObject.LastResult);
      tempError = true;
    }
  }
  else
  {
    // this should never happen
    DebugTln("Error: Modbus Read while transaction active");
    ModbusdataObject.LastResult = 99;
  }

  return tempError;
}

//============ Read register functions V2 (string)
bool Modbus_Read_string(uint16_t i)
{
  bool tempError = false;
  uint16_t numregs = Modbusmap[i].formatstringlen / 2 ;    
  // union defined static in .h 
  // static union {
  //     uint16_t mb_charregs[16];
  //     char     mb_charconv[32];
  // }  mb_reg2char ;

  if (bDebugMBlogic) DebugTf("Modbus_Read_string Number of registers[%d]\r\n",numregs);

  if (!mb.slave())  
  { 
    if (!bDebugMBlogic) { 
      mb.readHreg(settingModbusSlaveAdr, Modbusmap[i].address, mb_reg2char.mb_charregs, numregs, cb); // Send Read Hreg from Modbus Server
      waitMBslave();
    } else MBtestreadHreg(i, Modbusmap[i].address, mb_reg2char.mb_charregs, numregs ) ;

    if (ModbusdataObject.LastResult == 0)
    {
      //  determine value from multiple registers for (int r = 0 ; r < count ; r++) { Debugf("Reg: %d, Val: %d \r\n", r+readreg, shortres[r]); }
      char *ctemp = new char[Modbusmap[i].formatstringlen + 1];
      if (settingModbusByteswap) {
        for (int r = 0; r < Modbusmap[i].formatstringlen ; r=r+2) {
          ctemp[r] = mb_reg2char.mb_charconv[r+1] ;
          ctemp[r+1] = mb_reg2char.mb_charconv[r] ;
        }
        ctemp[Modbusmap[i].formatstringlen] = '\0' ; // set end of string
      } else  strCopy(ctemp, Modbusmap[i].formatstringlen ,mb_reg2char.mb_charconv); // compiler adds end of string
      Modbusmap[i].Modbus_string = ctemp ;
      delete ctemp;

      if (bDebugMBmsg)
        DebugTf("Modbus Read string Reg:[%d] , Result:[%s]\r\n", Modbusmap[i].address, CSTR(Modbusmap[i].Modbus_string));
      if (settingMQTTenable && Modbusmap[i].mqenable == 1)
      {
        toMQTT_string(i);
      }
    }
    else
    {
      DebugTf("Modbus Read string Reg:[%d] , Result: 0x%02X \r\n", Modbusmap[i].address, ModbusdataObject.LastResult);
      tempError = true;
    }
  }
  else
  {
    // this should never happen
    DebugTln("Error: Modbus Read while transaction active");
    ModbusdataObject.LastResult = 99;
  }

  return tempError;
}



// MQTT send functions

void toMQTT_short(int id)
{
  //function to push short data to MQTT
  int16_t _value = Modbusmap[id].Modbus_short;
  char _msg[15]{0};
  itoa(_value, _msg, 10);
  if (bDebugMBmsg)
    DebugTf("To MQTT_short %s %s %s\r\n", Modbusmap[id].label, _msg, Modbusmap[id].unit);
  //SendMQTT
  sendMQTTData(Modbusmap[id].label, _msg);
}

void toMQTT_ushort(int id)
{
  //function to push ushort data to MQTT
  uint16_t _value = Modbusmap[id].Modbus_ushort;
  char _msg[15]{0};
  itoa(_value, _msg, 10);
  if (bDebugMBmsg)
    DebugTf("To MQTT_short %s %s %s\r\n", Modbusmap[id].label, _msg, Modbusmap[id].unit);
  //SendMQTT
  sendMQTTData(Modbusmap[id].label, _msg);
}

void toMQTT_int(int id)
{
  //function to push int data to MQTT
  int32_t _value = Modbusmap[id].Modbus_int;
  char _msg[15]{0};
  itoa(_value, _msg, 10);
  if (bDebugMBmsg)
    DebugTf("To MQTT_short %s %s %s\r\n", Modbusmap[id].label, _msg, Modbusmap[id].unit);
  //SendMQTT
  sendMQTTData(Modbusmap[id].label, _msg);
}

void toMQTT_uint(int id)
{
  //function to push uint data to MQTT
  uint32_t _value = Modbusmap[id].Modbus_uint;
  char _msg[15]{0};
  itoa(_value, _msg, 10);
  if (bDebugMBmsg)
    DebugTf("To MQTT_short %s %s %s\r\n", Modbusmap[id].label, _msg, Modbusmap[id].unit);
  //SendMQTT
  sendMQTTData(Modbusmap[id].label, _msg);
}

void toMQTT_float(int id)
{
  //function to push float data to MQTT
  float _value = round(Modbusmap[id].Modbus_float * 100.0) / 100.0; // round float 2 digits, like this: x.xx

  char _msg[15]{0};
  dtostrf(_value, 3, 2, _msg);
  if (bDebugMBmsg) DebugTf("To MQTT_float %s %s %s\r\n", Modbusmap[id].label, _msg, Modbusmap[id].unit);
  //SendMQTT
  sendMQTTData(Modbusmap[id].label, _msg);

}

void toMQTT_string(int id)
{
  //function to push string data to MQTT

  if (bDebugMBmsg) DebugTf("To MQTT_string %s %s %s\r\n", Modbusmap[id].label, CSTR(Modbusmap[id].Modbus_string));
  //SendMQTT
  sendMQTTData(Modbusmap[id].label, CSTR(Modbusmap[id].Modbus_string));

}

void Modbus2MQTT() {
  for (int i = 1; i <= ModbusdataObject.NumberRegisters; i++)
  {
    if (settingMQTTenable && Modbusmap[i].mqenable == 1)
    {
      if (!settingModbusSinglephase || Modbusmap[i].phase == 0 || Modbusmap[i].phase == 1 || Modbusmap[i].phase == 4)
      {
        switch (Modbusmap[i].regformat)
        {
        case Modbus_short:
          toMQTT_short(i);  break;
        case Modbus_ushort: 
          toMQTT_ushort(i); break;
        case Modbus_int:
          toMQTT_int(i);    break;
        case Modbus_uint:
          toMQTT_uint(i);   break;
        case Modbus_float:
          toMQTT_float(i);  break;
        case Modbus_string:
          toMQTT_string(i);  break;
        case Modbus_undef:
          DebugTf("ERROR: MQTT Not implemented for %d = %s \r\n", i, Modbusmap[i].label); break;
        default:
          DebugTf("ERROR: MQTT Error undef type %d = %s \r\n", i, Modbusmap[i].label); break;
        }
      }
    }
  }


}


// Read all configured modbus registers

void readModbus()
{
  uint16_t countError = 0;

  if (settingLEDblink)
    blinkLEDnow(LED1);

  for (int i = 1; i <= ModbusdataObject.NumberRegisters; i++)
  {
    if (!settingModbusSinglephase || Modbusmap[i].phase == 0 || Modbusmap[i].phase == 1 || Modbusmap[i].phase == 4)
    {
      switch (Modbusmap[i].regformat)
      {
      case Modbus_short:
        if (Modbus_Read_short(i))  countError++;  break;
      case Modbus_ushort:
        if (Modbus_Read_ushort(i)) countError++;  break;
      case Modbus_int:
        if (Modbus_Read_int(i))    countError++;  break;
      case Modbus_uint:
        if (Modbus_Read_uint(i))  countError++;  break;
      case Modbus_float:
        if (Modbus_Read_float(i))  countError++;  break;
      case Modbus_string:
        if (Modbus_Read_string(i))  countError++;  break;
      case Modbus_undef:
        DebugTf("ERROR: undef type %d = %s \r\n", i, Modbusmap[i].label);
        break;
      default:
        DebugTf("ERROR: undef type %d = %s \r\n", i, Modbusmap[i].label);
        break;
      }
    }
    // yield();
  }

  if (countError > 0)
  {
    DebugTf("readModbus ended with errors count[%d] FreeHeap:[%d] \r\n", countError, ESP.getFreeHeap());
  }
  if (settingLEDblink)
    blinkLEDnow(LED1);
}

void readModbusSetup()
{

  if (bDebugMBmsg) Debugf("readModbus2 started\r\n");

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
 if (bDebugMBmsg) Debugf("readModbus ended \r\n");
}


int sendModbus(const char* buf, int len)
 {
   //Just send the buffer to Modbus when the Serial interface is available
   // Needs to be Mudbus RTU !!!
   //

   if (bDebugMBmsg)  DebugTf("sendModbus len: [%s] buf: [%s]\r\n", len, buf);
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

 void doInitDaytimemap()
 {

   const char *cfgFilename = "/Daytimemap.cfg";
   // Comment lines start with # or //
   // 
  
   Daytimemap = new DaytimemapStruct_t[8]; // 7 days, but days start at 1 , need 8 slots therefore

   int id = 1;
   int daynum = 0;
   int Index1, Index2, Index3, Index4;
   String sDay;
   String sStarthour;
   String sStartmin;
   String sEndhour;
   String sEndmin;

   File fh; //filehandle
   //Let's open the Modbus config file
   LittleFS.begin();
   if (LittleFS.exists(cfgFilename))
   {
     fh = LittleFS.open(cfgFilename, "r");
     if (fh)
     {
       //Lets go read the config and store in modbusmap line by line
       while (fh.available() && id <= 7)
       { //read file line by line, split and send to MQTT (topic, msg)
         String sLine = fh.readStringUntil('\n');

         if (sLine.startsWith("#") || sLine.startsWith("//"))
         {
           if (bDebugMBmsg)  DebugTf("Either comment or invalid config line: [%s]\r\n", sLine.c_str());
         }
         else
         {
           if (bDebugMBmsg)  DebugTf("sline[%s]\r\n", sLine.c_str());
           // #file format is CSV, all values as string without "", fields are
           // #day, starthour, startmin, endhour, endmin
           // #day = daynumber, Numeric representation of the day of the week(1 = Sunday)
           // #starthour = start hour of relay on
           // #startmin = start min of relay on
           // #endhour = end hour of relay off
           // #endmin = end minute of relay off

           Index1 = sLine.indexOf(',');
           Index2 = sLine.indexOf(',', Index1 + 1);
           Index3 = sLine.indexOf(',', Index2 + 1);
           Index4 = sLine.indexOf(',', Index3 + 1);
           if (bDebugMBmsg)
             DebugTf("Index1[%d],Index2[%d],Index3[%d],Index4[%d]\r\n", Index1, Index2, Index3, Index4);

           if (Index4 <= 0)
           {
             if (bDebugMBmsg)
             {
               DebugTf("Index1[%d],Index2[%d],Index3[%d],Index4[%d]\r\n", Index1, Index2, Index3, Index4);
               DebugTln("ERROR: Missing parameters in Daytimemap, skip line");
             }
             break;
           }

           sDay = sLine.substring(0, Index1);
           sStarthour = sLine.substring(Index1 + 1, Index2);
           sStartmin = sLine.substring(Index2 + 1, Index3);
           sEndhour = sLine.substring(Index3 + 1, Index4);
           sEndmin = sLine.substring(Index4 + 1);
           
           sDay.trim();
           sStarthour.trim();
           sStartmin.trim();
           sEndhour.trim();
           sEndmin.trim();

          if (bDebugMBmsg) { DebugTf("sDay[%s], sStarthour[%s], sStartmin[%s], sEndhour[%s], sEndmin[%s]\r\n", sDay.c_str(), sStarthour.c_str(), sStartmin.c_str(), sEndhour.c_str(), sEndmin.c_str());
              delay(10); 
          }

           daynum = sDay.toInt();
           if (daynum >7 || daynum != id ) {
              DebugTf("DAYNUM NOT IN SEQUENCE sDay[%s], sStarthour[%s], sStartmin[%s], sEndhour[%s], sEndmin[%s]\r\n", sDay.c_str(), sStarthour.c_str(), sStartmin.c_str(), sEndhour.c_str(), sEndmin.c_str());
           }
           else
           {
             Daytimemap[daynum].day = sDay.toInt();
             Daytimemap[daynum].starthour = sStarthour.toInt();
             Daytimemap[daynum].startmin = sStartmin.toFloat();
             Daytimemap[daynum].endhour = sEndhour.toInt();
             Daytimemap[daynum].endmin = sEndmin.toInt();
           }
           id++;
         }

       } // while available()
       if (bDebugMBmsg) DebugTf("Number of Daytimemap registers initialized:, %d \r\n", id-1);
       fh.close();
     }
   }
 }
 //===========================================================================================
 void printDaytimemap()
 {
   if (bDebugMBmsg) DebugTf("printDaytimemap begin for: %d, records \r\n",7);
   for (int i = 1; i <= 7; i++)
   {
     DebugTf("Day: %s, starttime: %02d:%02d, endtime: %02d:%02d \r\n", dayStr(Daytimemap[i].day).c_str(), Daytimemap[i].starthour, Daytimemap[i].startmin, Daytimemap[i].endhour, Daytimemap[i].endmin);
   }
  Debugln();
  checkactivateRelay(false) ;

 }

 void doInitModbusMap()
 {
   String cfgFilename = "/" + settingModbusCfgfile ;
  //  const char* cfgFilename = "/" + settingModbusCfgfile;
  if (bDebugMBlogic) cfgFilename = "/Modbusmaptest.cfg" ;
  DebugTf("Modbusmap config file[%s]\r\n",CSTR(settingModbusCfgfile));

   // Comment lines start with # or //
   // Configuration file to map modbus registers
   // file format is CSV , all values as string without "" ,  fields are
   // reg = register address
   // format = (Modbus_short, Modbus_ushort, Modbus_int, Modbus_uint, Modbus_float, Modbus_string[length], Modbus_undef)  
   // operation = (Modbus_READ, Modbus_RW, Modbus_UNDEF)  (Only Modbus_READ implemented in rel 1)
   // label = Short label string
   // friendlyname = string in UI
   // deviceclass = See Homeassistant https://www.home-assistant.io/integrations/sensor/#device-class 
   // unit = V, A, wH etc.
   // phase = 1,2,3 or 0 for generic and 4 for sum
   // factor = multiplication/division factor to apply. Specify 1 for no conversion , eg. 1000 for Wh to kWh
   // mqenable = set to 1 to enable sending to MQTT
   // reg, format, operation, label, friendlyname, deviceclass, unit, phase, factor, mqenable
   // 19000, Modbus_short, Modbus_READ, UL1N, Voltage L1-N, voltage, V, 1, 1, 1

   Modbusmap  = new  Modbuslookup_t[MODBUSCOUNT];

   int id = 0;
   int Index1, Index2, Index3, Index4, Index5, Index6, Index7, Index8, Index9;
   int Formatcnt = 0;
   String sReg;
   String sFormat;
   String sFormatcnt;
   String sOper;
   String sLabel;
   String sName;
   String sDevclass;
   String sUnit;
   String sPhase;
   String sFactor;
   String sMQEnable;

   String stName;
   String stLabel;
   String stUnit;


   File fh; //filehandle
   //Let's open the Modbus config file
   LittleFS.begin();
   if (LittleFS.exists(cfgFilename))
   {
     fh = LittleFS.open(cfgFilename, "r");
     if (fh) {
       //Lets go read the config and store in modbusmap line by line
       while(fh.available() && id <= MODBUSCOUNT)
       {  //read file line by line, split and send to MQTT (topic, msg)
          String sLine = fh.readStringUntil('\n');

          if (sLine.startsWith("#") || sLine.startsWith("//") )
          {
            if (bDebugMBmsg) DebugTf("INFO: Either comment or invalid config line: [%s]\r\n", sLine.c_str());
          } else {             
            if (bDebugMBmsg) DebugTf("sline[%s]\r\n", sLine.c_str());
            // reg, format, type, label, friendlyname, devclass, unit, phase, mqenable
            Index1 = sLine.indexOf(',');
            Index2 = sLine.indexOf(',', Index1 + 1);
            Index3 = sLine.indexOf(',', Index2 + 1);
            Index4 = sLine.indexOf(',', Index3 + 1);
            Index5 = sLine.indexOf(',', Index4 + 1);
            Index6 = sLine.indexOf(',', Index5 + 1);
            Index7 = sLine.indexOf(',', Index6 + 1);
            Index8 = sLine.indexOf(',', Index7 + 1);
            Index9 = sLine.indexOf(',', Index8 + 1);
            if (bDebugMBmsg)  DebugTf("Index1[%d],Index2[%d],Index3[%d],Index4[%d],Index5[%d],Index6[%d],Index7[%d],Index8[%d],Index9[%d]\r\n", Index1, Index2, Index3, Index4, Index5, Index6, Index7, Index8, Index9);

            if (Index9 <= 0)
            {
              if (bDebugMBmsg) {
                DebugTf("Index1[%d],Index2[%d],Index3[%d],Index4[%d],Index5[%d],Index6[%d],Index7[%d],Index8[%d],Index9[%d]\r\n", Index1, Index2, Index3, Index4, Index5, Index6, Index7, Index8, Index9);
                DebugTln("ERROR: Missing parameters in config line, skip line");
              }
              break;
            }
            Formatcnt = 0; // set Modbus_string lenght to 0
            sReg       = sLine.substring(0, Index1);
            sFormat    = sLine.substring(Index1 + 1, Index2);
            sOper      = sLine.substring(Index2 + 1, Index3);
            sLabel     = sLine.substring(Index3 + 1, Index4);
            sName      = sLine.substring(Index4 + 1, Index5);
            sDevclass  = sLine.substring(Index5 + 1, Index6);
            sUnit      = sLine.substring(Index6 + 1, Index7);
            sPhase     = sLine.substring(Index7 + 1, Index8);
            sFactor    = sLine.substring(Index8 + 1, Index9);
            sMQEnable  = sLine.substring(Index9 + 1);
            sReg.trim();
            sFormat.trim();
            sOper.trim();
            sLabel.trim();
            sName.trim();
            sDevclass.trim();
            sUnit.trim();
            sPhase.trim();
            sFactor.trim();
            sMQEnable.trim();
            if (bDebugMBmsg) {
               DebugTf("sReg[%s], sFormat[%s], sRegoper[%s], sLabel[%s], sName[%s], sDeviceclass[%s] , sUnit[%s], sPhase[%s], sFactor[%s], sMQEnable[%s]\r\n", sReg.c_str(), sFormat.c_str(), sOper.c_str(), sLabel.c_str(), sName.c_str(), sDevclass.c_str(), sUnit.c_str(), sPhase.c_str(),sFactor.c_str(),sMQEnable.c_str());
               delay(10);
            }
            id++;

            Modbusmap[id].id = id ;
            if (sOper == "Modbus_READ" )          { Modbusmap[id].oper = Modbus_READ ; } 
            else if (sOper == "Modbus_RW" )       { Modbusmap[id].oper = Modbus_RW ;   } 
            else {
              Modbusmap[id].oper = Modbus_UNDEF ;
              if (bDebugMBmsg) DebugTln("WARNING: Not read or RW,  Modbus_UNDEF");
            }

            if (sFormat == "Modbus_short" )       { Modbusmap[id].regformat = Modbus_short ;  }
            else if (sFormat == "Modbus_ushort")  { Modbusmap[id].regformat = Modbus_ushort;  }            
            else if (sFormat == "Modbus_int")     { Modbusmap[id].regformat = Modbus_int;     }
            else if (sFormat == "Modbus_uint")    { Modbusmap[id].regformat = Modbus_uint;    }
            else if (sFormat == "Modbus_float")   { Modbusmap[id].regformat = Modbus_float;   }
            else if (sFormat.startsWith("Modbus_string")) {
                    Index1 = sFormat.indexOf('[');
                    Index2 = sFormat.indexOf(']', Index1 + 1);
                    sFormatcnt = sFormat.substring(Index1 + 1, Index2);
                    if (Index2 > 0) {
                        Modbusmap[id].regformat = Modbus_string;
                        Formatcnt = sFormatcnt.toInt();
                        if (bDebugMBmsg) DebugTf("Modbus_string detected, sFormat[%s], sFormatcnt[%s], length[%d]\r\n",CSTR(sFormat), CSTR(sFormatcnt),Formatcnt);
                        if (Formatcnt > 64 || Formatcnt == 0) {
                          DebugTf("sReg[%s], sFormat[%s], sRegoper[%s], sLabel[%s], sName[%s], sDeviceclass[%s] , sUnit[%s], sPhase[%s], sFactor[%s], sMQEnable[%s]\r\n", sReg.c_str(), sFormat.c_str(), sOper.c_str(), sLabel.c_str(), sName.c_str(), sDevclass.c_str(), sUnit.c_str(), sPhase.c_str(),sFactor.c_str(),sMQEnable.c_str());
                          DebugTf("ERROR Modbus_string length[%d] outside allowed range\r\n",Formatcnt);
                          Formatcnt = 0;
                          Modbusmap[id].regformat = Modbus_undef; 
                    
                        }
                        else if ((Formatcnt % 2) != 0) {
                          DebugTf("sReg[%s], sFormat[%s], sRegoper[%s], sLabel[%s], sName[%s], sDeviceclass[%s] , sUnit[%s], sPhase[%s], sFactor[%s], sMQEnable[%s]\r\n", sReg.c_str(), sFormat.c_str(), sOper.c_str(), sLabel.c_str(), sName.c_str(), sDevclass.c_str(), sUnit.c_str(), sPhase.c_str(),sFactor.c_str(),sMQEnable.c_str());
                          DebugTf("ERROR Modbus_string length[%d] not even\r\n",Formatcnt);
                          Formatcnt = 0;
                          Modbusmap[id].regformat = Modbus_undef;
                        
                          } 
                    } else { 
                      DebugTf("WARNING: Modbus_string defined without lenght") ; 
                      Modbusmap[id].regformat = Modbus_undef;
                       
                      } 
                } 
            else  {  Modbusmap[id].regformat = Modbus_undef;
                  if (bDebugMBmsg)  DebugTln("WARNING:  Modbus_undef detected");
                  }
            }
            // assign values to Modbusmap
            Modbusmap[id].address = sReg.toInt();
            Modbusmap[id].phase = sPhase.toInt();
            // Modbusmap[id].factor = sFactor.toFloat();
            // also get negative sign converted of factor, this seems not to work with toFloat(). 
            char tempChar[sFactor.length()+1];
            sFactor.toCharArray(tempChar,sizeof(tempChar));  
            Modbusmap[id].factor = atof(tempChar);
            Modbusmap[id].mqenable = sMQEnable.toInt();
            Modbusmap[id].formatstringlen = Formatcnt;
            char *clabel = new char[sLabel.length() + 1];
            char *cname = new char[sName.length() + 1];
            char *cdevclass = new char[sDevclass.length() + 1];
            char *cunit = new char[sUnit.length() + 1];
            strcpy(clabel, sLabel.c_str());
            strcpy(cname, sName.c_str());
            strcpy(cdevclass, sDevclass.c_str());
            strcpy(cunit, sUnit.c_str());

            Modbusmap[id].label = clabel;
            Modbusmap[id].friendlyname = cname;
            Modbusmap[id].devclass = cdevclass;
            Modbusmap[id].unit = cunit;
            // delete [] clabel;  // do not delete objects, are still required during runtime !
            // delete [] cmbstring;
            // delete [] cname;
            // delete [] cdevclass;
            // delete [] cunit;
            switch (Modbusmap[id].regformat) {
              case Modbus_short:    Modbusmap[id].Modbus_short  = 9999; break;
              case Modbus_ushort:   Modbusmap[id].Modbus_ushort = 9999; break;
              case Modbus_int:      Modbusmap[id].Modbus_int =  9999;   break;
              case Modbus_uint:     Modbusmap[id].Modbus_uint = 9999;   break;
              case Modbus_float:    Modbusmap[id].Modbus_float = 9999;  break;
              case Modbus_string:   
                  Modbusmap[id].Modbus_string.reserve(Formatcnt) ;                         
                  Modbusmap[id].Modbus_string = "NaN";  
                break;
            }

       } // while available()

       ModbusdataObject.NumberRegisters = id ;
       if (bDebugMBmsg)  DebugTf("INFO: Number of Modbus registers initialized:, %d \r\n", ModbusdataObject.NumberRegisters);
       fh.close();

     }
   } else DebugTf("ERROR, Modbus configfile[%s] doesnt exist.\r\n",CSTR(cfgFilename)) ;
 }

// help routines 
String getStringForModbusoper(int enum_val) {
  String tmp(Modbusoper_str[enum_val]);
  return tmp;
}
String getStringForModbusformat(int enum_val)
{
  String tmp(Modbusformat_str[enum_val]);
  return tmp;
}

void printModbusmapln(int16_t i) {
    Debugf("Id[%d] Reg[%d] Oper[%s] Format[%s] ", Modbusmap[i].id, Modbusmap[i].address, CSTR(getStringForModbusoper(Modbusmap[i].oper)), CSTR(getStringForModbusformat(Modbusmap[i].regformat)));
    switch (Modbusmap[i].regformat)     {
    case Modbus_short:       Debugf("Value[%d]", Modbusmap[i].Modbus_short);  break;
    case Modbus_ushort:      Debugf("Value[%d]", Modbusmap[i].Modbus_ushort); break;
    case Modbus_int:         Debugf("Value[%d]", Modbusmap[i].Modbus_int);    break;
    case Modbus_uint:        Debugf("Value[%d]", Modbusmap[i].Modbus_uint);   break;
    case Modbus_float:       Debugf("Value[%f]", Modbusmap[i].Modbus_float);  break;
    case Modbus_string:      Debugf("Length[%d] Value[%s]",Modbusmap[i].formatstringlen, CSTR(Modbusmap[i].Modbus_string));  break;
    case Modbus_undef:       DebugTf("ERROR: undef type %d = %s \r\n", i, Modbusmap[i].label); break;
    default:                 DebugTf("ERROR: undef type %d = %s \r\n", i, Modbusmap[i].label); break;
    } 
  Debugf("  Label[%s] Name[%s] Phase[%d] ",  Modbusmap[i].label, Modbusmap[i].friendlyname, Modbusmap[i].phase);
  Debugf("Devclass[%s] Unit[%s] Factor[%f] MQEnable[%d]\r\n", Modbusmap[i].devclass, Modbusmap[i].unit, Modbusmap[i].factor, Modbusmap[i].mqenable);

}

void printModbusmap() {
  Debugf("printModbusmap begin for: %d, records\r\n", ModbusdataObject.NumberRegisters) ;
  for (int i = 1; i <= ModbusdataObject.NumberRegisters ; i++) {
    printModbusmapln(i);
  }
}



//====================[ functions for REST API ]====================
String getModbusValue(int modbusreg)
{

   return "Modbus not implemented yet!";
}

// Relay functions



void checkactivateRelay(bool activaterelay)
{
  int16_t dagcurmin, dagstartmin, dagendmin = 0;
  if (settingTimebasedSwitch && settingNTPenable)
  {

    DebugTf("Schedule for today: %s, starttime: %02d:%02d, endtime: %02d:%02d \r\n", dayStr(Daytimemap[weekday()].day).c_str(), Daytimemap[weekday()].starthour, Daytimemap[weekday()].startmin, Daytimemap[weekday()].endhour, Daytimemap[weekday()].endmin);

    if (tempsettingRelayOn && activaterelay) {
      DebugTln("Warning: Relay Temporary On until next on cycle"); 
      setRelay(RELAYON); 
    }
    dagcurmin = hour() * 60 + minute();
    dagstartmin = Daytimemap[weekday()].starthour * 60 + Daytimemap[weekday()].startmin;
    dagendmin = Daytimemap[weekday()].endhour * 60 + Daytimemap[weekday()].endmin ;

    if (dagstartmin < dagendmin)
    {
      if (dagcurmin >= dagstartmin && dagcurmin < dagendmin)
      {
        DebugTf("Tijd:%02d:%02d Binnen tijdslot, set relay on\r\n", hour(), minute());
        if (tempsettingRelayOn)    {        
          tempsettingRelayOn = false ; // Turn the temporary on switch of at next cycle
          DebugTln("Info: Relay Temporary On switch turned off"); 
        }
        if (activaterelay && statusRelay == RELAYOFF) {
          setRelay(RELAYON); 
        }  
      }
      else
      {
        DebugTf("Tijd:%02d:%02d Buiten tijdslot, set relay off\r\n", hour(), minute());
        if ((activaterelay && statusRelay == RELAYON) && !tempsettingRelayOn) setRelay(RELAYOFF); 
      }
    }
    else
    {
      if (dagcurmin >= dagstartmin || dagcurmin < dagendmin)
      {
        DebugTf("Tijd:%02d:%02d Binnen tijdslot, set relay on\r\n", hour(), minute());
        if (tempsettingRelayOn)    {        
          tempsettingRelayOn = false ; // Turn the temporary on switch of at next cycle
          DebugTln("Info: Relay Temporary On switch turned off"); 
        }
        if (activaterelay && statusRelay == RELAYOFF) { 
          setRelay(RELAYON);
        }  
      }
      else
      {
        DebugTf("Tijd:%02d:%02d Buiten tijdslot, set relay off\r\n", hour(), minute());
        if ((activaterelay && statusRelay == RELAYON) && !tempsettingRelayOn) setRelay(RELAYOFF); 
      }
    }
  }
  if (settingRelayAllwaysOnSwitch) {
    DebugTln("WARNING, Relay set to ON");
    DebugTln("WARNING, Relay set to ON");
    setRelay(RELAYON) ;
  }
  if (activaterelay) DebugTf("statusRelay[%d]\r\n", statusRelay);
}

void setRelay(uint8_t status)
{
  pinMode(RELAY, OUTPUT);
  digitalWrite(RELAY, status);
  if (bDebugMBmsg)  DebugTf("INFO: Relay set to %d \r\n", status);
  statusRelay = status;
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

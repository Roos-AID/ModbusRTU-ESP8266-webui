/*
***************************************************************************
**  Program  : jsonStuff
**  Version 1.11.0
**
**  Copyright (c) 2023 Rob Roos
**     based on Framework ESP8266 from Willem Aandewiel and modifications
**     from Robert van Breemen
**
**  TERMS OF USE: MIT License. See bottom of file.
***************************************************************************
*/
static int iIdentlevel = 0;
char sBeforenext[10] ="";
bool bFirst = true; 


//=======================================================================
void sendStartJsonObj(const char *objName)
{
  char sBuff[50] = "";
  sBeforenext[0]='\0';

  if (strlen(objName)==0){  
    snprintf(sBuff, sizeof(sBuff), "{\r\n");
  }else {
    snprintf(sBuff, sizeof(sBuff), "{\"%s\":[\r\n", objName);
  }
  httpServer.sendHeader("Access-Control-Allow-Origin", "*");
  httpServer.setContentLength(CONTENT_LENGTH_UNKNOWN);
  httpServer.send(200, "application/json", sBuff);
  iIdentlevel++;
  bFirst = true;

} // sendStartJsonObj()

//=======================================================================

void sendIdent(){
  for (int i = iIdentlevel; i >0; i--){
    httpServer.sendContent("  ");
  }
}  //sendIdent()
//=======================================================================
void sendBeforenext(){
  if (!bFirst){ 
    httpServer.sendContent(",\r\n");
  }
  bFirst = false;
} //sendBeforenext()

void sendEndJsonObj(const char *objName)
{
  iIdentlevel--;
  if (strlen(objName)==0){  
    httpServer.sendContent("\r\n}\r\n");
  } else {
    httpServer.sendContent("\r\n]}\r\n");
  }

} // sendEndJsonObj()

//=======================================================================
void sendNestedJsonObj(const char *cName, const char *cValue)
{
  char jsonBuff[JSON_BUFF_MAX] = "";

  snprintf(jsonBuff, sizeof(jsonBuff), "{\"name\": \"%s\", \"value\": \"%s\"}", cName, cValue);

  sendBeforenext();
  sendIdent();
  httpServer.sendContent(jsonBuff);

} // sendNestedJsonObj(*char, *char)


//=======================================================================
void sendNestedJsonObj(const char *cName, String sValue)
{
  char jsonBuff[JSON_BUFF_MAX] = "";

  if (sValue.length() > (JSON_BUFF_MAX - 65) )
  {
    DebugTf(PSTR("[2] sValue.length() [%d]\r\n"), sValue.length());
  }

  snprintf(jsonBuff, sizeof(jsonBuff), "{\"name\": \"%s\", \"value\": \"%s\"}", cName, sValue.c_str());

  sendBeforenext();
  sendIdent();
  httpServer.sendContent(jsonBuff);

} // sendNestedJsonObj(*char, String)


//=======================================================================
void sendNestedJsonObj(const char *cName, int32_t iValue)
{
  char jsonBuff[200] = "";

  snprintf(jsonBuff, sizeof(jsonBuff), "{\"name\": \"%s\", \"value\": %d}", cName, iValue);

  sendBeforenext();
  sendIdent();
  httpServer.sendContent(jsonBuff);

} // sendNestedJsonObj(*char, int)

//=======================================================================
void sendNestedJsonObj(const char *cName, uint32_t uValue)
{
  char jsonBuff[200] = "";

  snprintf(jsonBuff, sizeof(jsonBuff), "{\"name\": \"%s\", \"value\": %u}", cName, uValue);

  sendBeforenext();
  sendIdent();
  httpServer.sendContent(jsonBuff);

} // sendNestedJsonObj(*char, uint)


//=======================================================================
void sendNestedJsonObj(const char *cName, float fValue)
{
  char jsonBuff[200] = "";

  snprintf(jsonBuff, sizeof(jsonBuff), "{\"name\": \"%s\", \"value\": %.3f}", cName, fValue);

  sendBeforenext();
  sendIdent();
  httpServer.sendContent(jsonBuff);

} // sendNestedJsonObj(*char, float)

//============= build Modbusmonitor string ========================
void sendJsonModbusmonObj(const char *cName, const char *cValue, const char *cUnit)
{
  char jsonBuff[JSON_BUFF_MAX] = "";

  snprintf(jsonBuff, sizeof(jsonBuff), "{\"name\": \"%s\", \"value\": \"%s\", \"unit\": \"%s\"}"
                                      , cName, cValue, cUnit);

  sendBeforenext();
  sendIdent();
  httpServer.sendContent(jsonBuff);

} // sendJsonModbusmonObj(*char, *char, *char)

//=======================================================================
void sendJsonModbusmonObj(const char *cName, int32_t iValue, const char *cUnit)
{
  char jsonBuff[200] = "";

  snprintf(jsonBuff, sizeof(jsonBuff), "{\"name\": \"%s\", \"value\": %d, \"unit\": \"%s\"}"
                                      , cName, iValue, cUnit);

  sendBeforenext();
  sendIdent();
  httpServer.sendContent(jsonBuff);

} // sendJsonModbusmonObj(*char, int, *char)

//=======================================================================
void sendJsonModbusmonObj(const char *cName, uint32_t uValue, const char *cUnit)
{
  char jsonBuff[200] = "";

  snprintf(jsonBuff, sizeof(jsonBuff), "{\"name\": \"%s\", \"value\": %u, \"unit\": \"%s\"}"
                                      , cName, uValue, cUnit);

  sendBeforenext();
  sendIdent();
  httpServer.sendContent(jsonBuff);

} // sendNestedJsonObj(*char, uint, *char)


//=======================================================================
void sendJsonModbusmonObj(const char *cName, float fValue, const char *cUnit)
{
  char jsonBuff[200] = "";

  snprintf(jsonBuff, sizeof(jsonBuff), "{\"name\": \"%s\", \"value\": %.2f, \"unit\": \"%s\"}"
                                      , cName, fValue, cUnit);

  sendBeforenext();
  sendIdent();
  httpServer.sendContent(jsonBuff);

} // sendJsonModbusmonObj(*char, float, *char)


//=======================================================================
// ************ function to build Json Settings string ******************
//=======================================================================
void sendJsonSettingObj(const char *cName, float fValue, const char *fType, int minValue, int maxValue)
{
  char jsonBuff[200] = "";

  snprintf(jsonBuff, sizeof(jsonBuff), "{\"name\": \"%s\", \"value\": %.3f, \"type\": \"%s\", \"min\": %d, \"max\": %d}"
                                      , cName, fValue, fType, minValue, maxValue);

  sendBeforenext();
  sendIdent();
  httpServer.sendContent(jsonBuff);

} // sendJsonSettingObj(*char, float, *char, int, int)


//=======================================================================
void sendJsonSettingObj(const char *cName, float fValue, const char *fType, int minValue, int maxValue, int decPlaces)
{
  char jsonBuff[200] = "";

  switch(decPlaces) {
    case 0:
      snprintf(jsonBuff, sizeof(jsonBuff), "{\"name\": \"%s\", \"value\": %.0f, \"type\": \"%s\", \"min\": %d, \"max\": %d}"
                                      , cName, fValue, fType, minValue, maxValue);
      break;
    case 2:
      snprintf(jsonBuff, sizeof(jsonBuff), "{\"name\": \"%s\", \"value\": %.2f, \"type\": \"%s\", \"min\": %d, \"max\": %d}"
                                      , cName, fValue, fType, minValue, maxValue);
      break;
    case 5:
      snprintf(jsonBuff, sizeof(jsonBuff), "{\"name\": \"%s\", \"value\": %.5f, \"type\": \"%s\", \"min\": %d, \"max\": %d}"
                                      , cName, fValue, fType, minValue, maxValue);
      break;
    default:
      snprintf(jsonBuff, sizeof(jsonBuff), "{\"name\": \"%s\", \"value\": %f, \"type\": \"%s\", \"min\": %d, \"max\": %d}"
                                      , cName, fValue, fType, minValue, maxValue);

  }

  sendBeforenext();
  sendIdent();
  httpServer.sendContent(jsonBuff);

} // sendJsonSettingObj(*char, float, *char, int, int, int)

//=======================================================================
void sendJsonSettingObj(const char *cName, bool bValue, const char *bType)
{
  char jsonBuff[200] = "";

  snprintf(jsonBuff, sizeof(jsonBuff), "{\"name\": \"%s\", \"value\":\"%s\", \"type\": \"%s\"}"
                                    , cName, CBOOLEAN(bValue), bType);

  sendBeforenext();
  sendIdent();
  httpServer.sendContent(jsonBuff);

} // sendJsonSettingObj(*char, bool, *char)
//=======================================================================
void sendJsonSettingObj(const char *cName, int iValue, const char *iType, int minValue, int maxValue, int stepValue)
{
  char jsonBuff[200] = "";

  snprintf(jsonBuff, sizeof(jsonBuff), "{\"name\": \"%s\", \"value\": %d, \"type\": \"%s\", \"min\": %d, \"max\": %d, \"step\": %d}" 
                                      , cName, iValue, iType, minValue, maxValue, stepValue);

  sendBeforenext();
  sendIdent();
  httpServer.sendContent(jsonBuff);

} // sendJsonSettingObj(*char, int, *char, int, int, int)


//=======================================================================
void sendJsonSettingObj(const char *cName, const char *cValue, const char *sType, int maxLen)
{
  char jsonBuff[200] = "";

  snprintf(jsonBuff, sizeof(jsonBuff), "{\"name\": \"%s\", \"value\":\"%s\", \"type\": \"%s\", \"maxlen\": %d}"
                                      , cName, cValue, sType, maxLen);

  sendBeforenext();
  sendIdent();
  httpServer.sendContent(jsonBuff);

} // sendJsonSettingObj(*char, *char, *char, int, int)




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
****************************************************************************
*/

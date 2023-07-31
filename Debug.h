/*
***************************************************************************
**  Program  : Debug.h, part of ModbusRTU-webui
**  Version 1.11.0
**
**  Copyright (c) 2023 Rob Roos
**     based on Framework ESP8266 from Willem Aandewiel and modifications
**     from Robert van Breemen
**
**  TERMS OF USE: MIT License. See bottom of file.
***************************************************************************
*
*      Deze versie is voor Serial debug plus telnet.
*
*/

#ifndef DEBUG_H
#define DEBUG_H

// Setup the debug port during setup phase
#define DEBUG_PORT Serial

//DEBUG_ESP_PORT setup means the Core ESP Arduino can also output debug info
//To use this: Tools -> Core Debug Level --> <none, error, warning, info, debug, verbose>
#ifndef DEBUG_ESP_PORT
      #define DEBUG_ESP_PORT DEBUG_PORT   
#endif      

#ifdef DEBUG_ESP_PORT
  #define DEBUG_MSG(...) DEBUG_ESP_PORT.printf( __VA_ARGS__ )
  //#define DEBUG_MSG(...) Debugf( __VA_ARGS__ )
  #else
  #define DEBUG_MSG(...)
#endif

/*---- start macro's ------------------------------------------------------------------*/

#define Debug(...)      ({ Serial.print(__VA_ARGS__);         \
                           TelnetStream.print(__VA_ARGS__);   \
                        })
#define Debugln(...)    ({ Serial.println(__VA_ARGS__);       \
                           TelnetStream.println(__VA_ARGS__); \
                        })
#define Debugf(...)     ({ Serial.printf(__VA_ARGS__);        \
                           TelnetStream.printf(__VA_ARGS__);  \
                        })
#define DebugFlush()    ({ Serial.flush(); \
                           TelnetStream.flush(); \
                        })


#define DebugT(...)     ({ _debugBOL(__FUNCTION__, __LINE__);     \
                           Debug(__VA_ARGS__);                    \
                        })
#define DebugTln(...)   ({ _debugBOL(__FUNCTION__, __LINE__);     \
                           Debugln(__VA_ARGS__);                  \
                        })
#define DebugTf(...)    ({ _debugBOL(__FUNCTION__, __LINE__);     \
                           Debugf(__VA_ARGS__);                   \
                        })

/*---- einde macro's ------------------------------------------------------------------*/

// needs #include <TelnetStream.h>       // Version 0.0.1 - https://github.com/jandrassy/TelnetStream

//#include <sys/time.h>
// #include <time.h>
// extern "C" int clock_gettime(clockid_t unused, struct timespec *tp);

void _debugBOL(const char *fn, int line)
{
   char _bol[128];
   // This commented out code is using mix of system time and acetime to print, but it will not work on microsecond level correctly
   // // //calculate fractional seconds to millis fraction
   // double fractional_seconds;
   // int microseconds;
   // struct timespec tp;   //to enable clock_gettime()  
   // clock_gettime(CLOCK_REALTIME, &tp); 
   // fractional_seconds = (double) tp.tv_nsec;
   // fractional_seconds /= 1e3;
   // fractional_seconds = round(fractional_seconds);
   // microseconds = (int) fractional_seconds;
     
   /* snprintf(_bol, sizeof(_bol), "%02d:%02d:%02d.%06d (%7u|%6u) %-12.12s(%4d): ", \
                 hour(), minute(), second(), microseconds, \
                 ESP.getFreeHeap(), ESP.getMaxFreeBlockSize(),\
                 fn, line);
   */
                 
   //Alternative based on localtime function
   timeval now;
   //struct tm *tod;
   gettimeofday(&now, nullptr);
   //tod = localtime(&now.tv_sec);

   /*
   snprintf(_bol, sizeof(_bol), "%02d:%02d:%02d.%06d (%7u|%6u) %-12.12s(%4d): ", \
                  tod->tm_hour, tod->tm_min, tod->tm_sec, (int)now.tv_usec, \
                  ESP.getFreeHeap(), ESP.getMaxFreeBlockSize(),\
                  fn, line);
   */

   TimeZone myTz =  timezoneManager.createForZoneName(CSTR(settingNTPtimezone));
   ZonedDateTime myTime = ZonedDateTime::forUnixSeconds64(time(nullptr), myTz);
   
   //DebugTf(PSTR("%02d:%02d:%02d %02d-%02d-%04d\r\n"), myTime.hour(), myTime.minute(), myTime.second(), myTime.day(), myTime.month(), myTime.year());

   snprintf(_bol, sizeof(_bol), "%02d:%02d:%02d.%06d (%7u|%6u) %-12.12s(%4d): ", \
                  myTime.hour(), myTime.minute(), myTime.second(), (int)now.tv_usec, \
                  ESP.getFreeHeap(), ESP.getMaxFreeBlockSize(),\
                  fn, line);

   TelnetStream.print (_bol);
}
#endif // DEBUG_H

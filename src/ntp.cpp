#include <Arduino.h>
#include "ntp.h"
#include "action.h"

time_t now; //NTP returned date
struct tm *timeCur;                 // the structure tm holds time information in a more convenient way
time_t lastNTPRefresh;

String ntpTime(){
  return ((timeCur->tm_hour<10)?"0":"")+(String)timeCur->tm_hour+":"+((timeCur->tm_min<10)?"0":"")+(String)timeCur->tm_min;
}

String ntpDate(){
  return (String)(timeCur->tm_year+1900)+"/"+((timeCur->tm_mon<9)?"0":"")+(String)(timeCur->tm_mon+1)+"/"+((timeCur->tm_mday<10)?"0":"")+(String)timeCur->tm_mday;
}

String ntpHHMM(){
  return ((timeCur->tm_hour<10)?"0":"")+(String)timeCur->tm_hour+((timeCur->tm_min<10)?"0":"")+(String)timeCur->tm_min;
}

void setupNtp(){
    configTime(mySettings.timezone, mySettings.ntpServer);
    now = time(nullptr);
    // create some delay before printing
    Serial.print("NTP: ");
    delay(500);
    while (now < 1510592825)
    {
        digitalWrite(PIN_LED, LOW);
        Serial.print(".");
        delay(500);
        time(&now);
        digitalWrite(PIN_LED, HIGH);
    }
    timeCur = localtime(&now);
    lastNTPRefresh=now;
    logActions(" OK ("+ntpTime()+")");
}
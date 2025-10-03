#include <Arduino.h>

extern time_t now; //NTP returned date
extern struct tm *timeCur;                 // the structure tm holds time information in a more convenient way
extern time_t lastNTPRefresh;
#define NTP_REFRESH_INTERVAL 60*60*24

String ntpTime();
String ntpDate();
String ntpHHMM(); //used for cron check
void setupNtp();
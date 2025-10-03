#include <Arduino.h>

#define STUCK_MOTOR_TIMEOUT 2000 //after X milliseconds without switch state changed, motor is considered stuck and feed return error
//input
#define SW_MOTOR D5
#define SW_WIFI D6
#define SW_FEED D7
//output
#define PIN_LED D0
#define PIN_PLAY D1
#define PIN_MOTOR D2

#define SETTINGS_FILE "/setting.json"
#define LOGFILE "/actions.log"

#ifndef STRUCT_SETTINGS
    #define STRUCT_SETTINGS
    struct settings {
        int feedTime = 3000;//duration in seconds for motor to feed
        boolean ledStatus = HIGH;//keep led always on
        char schedules[10][5] ={"----", "----", "----", "----", "----", "----", "----", "----", "----", "----"}; //10 slot format HHMM for automatic food delivery
        char login[100] = "admin"; //basic auth login/password(hashed/salted)
        char password[100] = "admin"; //admin:admin by default
        char hash[100] = "07bfef0990bb212b72d41a3d20619d75"; //admin:admin by default used for Digest, but bugged on IOS
        char timezone[50] = "CET-1CEST,M3.5.0,M10.5.0/3"; //currenlty set to Paris, list: https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
        char ntpServer[100] = "pool.ntp.org";
    };
#endif
extern struct settings mySettings;

extern String lastCronExec;
extern boolean feedPressed;
extern boolean wifiPressed;

void setupAction();
boolean feedCat();
boolean playAudio();
void IRAM_ATTR buttonFeed();
void IRAM_ATTR buttonWiFi();
boolean logActions(String action);
String readFile(String filePath);
boolean clearLogs();
boolean purgeLogs();
size_t LittleFSFilesize(const char* filename);
String listAllFilesInDir(String dir_path);
boolean readSettings();
boolean saveSettings();
void resetSettings();
void cron();

#include <Arduino.h>
#include "LittleFS.h"
#include "action.h"
#include "wifi.h"
#include <ArduinoJson.h>

struct settings mySettings;
String lastCronExec = "XXXX";
boolean feedPressed=false;
boolean wifiPressed=false;

void setupAction(){
    pinMode(PIN_MOTOR, OUTPUT);
    pinMode(PIN_LED, OUTPUT);
    pinMode(PIN_PLAY, OUTPUT);
    pinMode(SW_MOTOR, INPUT);
    pinMode(SW_FEED, INPUT);
    pinMode(SW_WIFI, INPUT);
    attachInterrupt(SW_FEED, buttonFeed, RISING);
    attachInterrupt(SW_WIFI, buttonWiFi, RISING);
    digitalWrite(PIN_MOTOR, LOW);
    digitalWrite(PIN_PLAY, LOW);

    if(!LittleFS.begin()){
        LittleFS.format();
        Serial.println("An Error has occurred while mounting LittleFS");
    }else{
        Serial.println("LittleFS mounted");
    }

    if (! LittleFS.exists(LOGFILE)) {
        File logfile = LittleFS.open(LOGFILE, "w+");
        if (logfile) {
            logfile.println(ntpDate()+" "+ntpTime()+" logfile created");
            logfile.close();
        } else {
            Serial.println("Error: cannot create logfile");
        }
    }
    if (! LittleFS.exists(SETTINGS_FILE)) {
        Serial.println("Save default settings");
        saveSettings();
    }else{
        readSettings();
    }
}



boolean feedCat(){
  unsigned long initMotor = millis();
  unsigned long timeout = STUCK_MOTOR_TIMEOUT;
  unsigned long switchUpdate = initMotor;
  
  boolean switchStatus = digitalRead(SW_MOTOR);
  boolean sw;
  boolean successStatus=true;
  digitalWrite(PIN_MOTOR, HIGH);
  
  while(mySettings.feedTime+initMotor > millis()){
    sw=digitalRead(SW_MOTOR);
    if (sw == switchStatus && millis() > (switchUpdate+timeout)){
        logActions("Motor stuck !");
        successStatus=false;
        break;
    }else if(sw != switchStatus){
        Serial.println("Motor switch updated");
        switchStatus=sw;
        switchUpdate=millis();
    }
    delay(50);
  }
  digitalWrite(PIN_MOTOR, LOW);
  return successStatus;
}

boolean playAudio(){
    digitalWrite(PIN_PLAY, HIGH);
    delay(100);
    digitalWrite(PIN_PLAY, LOW);
    return true;
}

void IRAM_ATTR buttonFeed(){
    Serial.println("Feed button pressed");
    digitalWrite(PIN_MOTOR, LOW);
    //pressed for at least 200ms
    unsigned long buttonPressed = millis();
    while(buttonPressed+100 > millis()){
        if(!digitalRead(SW_FEED)){
            Serial.println("not enough, ignoring it");
            return;
        }
    }
    logActions("Feed button action");
    feedPressed=true;
    return;
}

void IRAM_ATTR buttonWiFi(){
    Serial.println("WiFi button pressed");
    digitalWrite(PIN_MOTOR, LOW);
    //pressed for at least 5s
    unsigned long buttonPressed = millis();
    while(buttonPressed+8000 > millis()){
        if(!digitalRead(SW_WIFI)){
            //patch broken feed button
            if(buttonPressed+100 < millis()){
                Serial.println("Patch, feed cat");
                feedPressed=true;
            }
            Serial.println("not enough, ignoring it");
            return;
        }
    }
    logActions("WiFi button action");
    wifiPressed=true;
}

boolean logActions(String action){
    Serial.println(action);
    File logfile = LittleFS.open(LOGFILE, "a"); 
    //LittleFS.truncate(10);
    if (logfile) {
      logfile.println(ntpDate()+" "+ntpTime()+" "+action);
      logfile.close();
      return true;
    } else {
      Serial.println(F("Cannot access logfile"));
    }
    return false;
}

String readFile(String filePath){
    File file = LittleFS.open(filePath, "r");
    if(!file){
        Serial.println("File missing: "+filePath);
        return "File missing: "+filePath;
    }
    String output="";
    while(file.available()){
     output += file.readString();
    }
    file.close();
    return output;
}

boolean clearLogs(){
    File file = LittleFS.open(LOGFILE, "r");
    return LittleFS.remove(LOGFILE);
}

boolean purgeLogs(size_t max){
    if(LittleFSFilesize(LOGFILE) > max){
        logActions("Purge logfile");
        File logfile = LittleFS.open(LOGFILE, "r");
        File logfile2 = LittleFS.open(".tmp", "a"); 
        if(!logfile || !logfile2){
            logActions("Log file missing");
        }
        String output="";
        while(logfile.available() && LittleFSFilesize(".tmp") < max){
            logfile2.println(logfile.readString());
        }
        logfile.close();
        logfile2.close();
        LittleFS.remove(LOGFILE);
        LittleFS.rename(".tmp", LOGFILE);
        logActions("New log size:"+LittleFSFilesize(LOGFILE));
        return true;
    }
    return false;
}

size_t LittleFSFilesize(String filename) {
  auto file = LittleFS.open(filename, "r");
  size_t filesize = file.size();
  // Don't forget to clean up!
  file.close();
  return filesize;
}

String listAllFilesInDir(String dir_path){
	Dir dir = LittleFS.openDir(dir_path);
    String output="";
	while(dir.next()) {
		if (dir.isFile()) {
			// print file names
			output+=dir_path + dir.fileName()+"\t"+LittleFSFilesize(dir_path + dir.fileName())+"\n";
		}
		if (dir.isDirectory()) {
			// print directory names
			output+=(dir_path + dir.fileName() + "/\n");
			// recursive file listing inside new directory
			output+=listAllFilesInDir(dir_path + dir.fileName() + "/");
		}
	}
    return output;
}

boolean readSettings(){
    DynamicJsonDocument doc(512);
    DeserializationError error = deserializeJson(doc, readFile(SETTINGS_FILE));
    if (error) {
        logActions(F("deserializeJson() failed: "));
        logActions(error.f_str());
        saveSettings();
        return false;
    }
    mySettings.feedTime=doc["feedTime"];
    mySettings.ledStatus=doc["ledStatus"];
    for(unsigned int i=0; i<sizeof(mySettings.schedules)/5; i++){
        strncpy(mySettings.schedules[i], doc["schedules"][i], sizeof(doc["schedules"][i]));
    }
    if(doc["login"] != NULL ) strncpy(mySettings.login, doc["login"], sizeof(mySettings.login));
    if(doc["password"] != NULL ) strncpy(mySettings.password, doc["password"], sizeof(mySettings.password));
    if(doc["hash"] != NULL ) strncpy(mySettings.hash, doc["hash"], sizeof(mySettings.hash));
    if(doc["timezone"] != NULL ) strncpy(mySettings.timezone, doc["timezone"], sizeof(mySettings.timezone));
    if(doc["ntpServer"] != NULL ) strncpy(mySettings.ntpServer, doc["ntpServer"], sizeof(mySettings.ntpServer));
    return true;
    
}

boolean saveSettings(){
    File file = LittleFS.open(SETTINGS_FILE, "w"); 
    if (file) {
      DynamicJsonDocument docJson(512);
      docJson["feedTime"]=mySettings.feedTime;
      docJson["ledStatus"]=mySettings.ledStatus;
      for(unsigned int i=0; i<sizeof(mySettings.schedules)/5; i++){
        docJson["schedules"][i]=mySettings.schedules[i];
      }
      docJson["login"]=mySettings.login;
      docJson["password"]=mySettings.password;
      docJson["hash"]=mySettings.hash;

      docJson["timezone"]=mySettings.timezone;
      docJson["ntpServer"]=mySettings.ntpServer;

      serializeJson(docJson, file);
      file.close();
      return true;
    } else {
      Serial.println("Cannot save settings");
    }
    return false;
}

void resetSettings(){
    logActions("Reset Settings");
    LittleFS.remove(SETTINGS_FILE);
    logActions("WiFi reset");
    digitalWrite(PIN_LED, HIGH); //switch off led
    WiFi.mode(WIFI_STA);
    WiFi.persistent(true);
    //LittleFS.format();
    wifiManager.resetSettings();
    delay(3000);
    ESP.restart();
    //WiFi.softAPIP();
}


void cron(){
    if(!lastCronExec.equals(ntpHHMM())){
        lastCronExec=ntpHHMM();
        //Serial.println("Cron check:"+(String)HHMM);
        for(unsigned int i=0; i<sizeof(mySettings.schedules)/5; i++){
            //Serial.print((String)mySettings.schedules[i]+"=="+lastCronExec+"? ");
            if(lastCronExec.equals(mySettings.schedules[i])){
                //Serial.println("Yes");
                boolean result=feedCat();
                logActions("Cron exec: feed="+(String)(result?"success":"failure"));
            }else{
                //Serial.println("No");
            }
        }
    }
    
}
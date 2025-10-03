#include <Arduino.h>
#include "LittleFS.h"
#include <ArduinoJson.h>
#include "wifi.h"
#include "ntp.h"
#include "action.h"

//---------------------------------------
//     HTTP handlers
//---------------------------------------
void handle_index(){
  session_authenticated();
  server.send(200, "text/html", readFile("/static/index.html"));
}

void handle_status(){
  session_authenticated();
  FSInfo fs_info;
  LittleFS.info(fs_info);

  int quality;
  int8_t RSSI = WiFi.RSSI();
  if (RSSI <= -100) {
    quality = 0;
  } else if (RSSI >= -50) {
    quality = 100;
  } else {
    quality = 2 * (RSSI + 100);
  }
  
  DynamicJsonDocument docJson(200);
  docJson["SSID"]=WiFi.SSID();
  docJson["Signal"]=(String)quality+"%";
  docJson["IP"]=WiFi.localIP().toString();
  docJson["heap"]=ESP.getFreeHeap();
  docJson["vcc"]=ESP.getVcc();
  docJson["fsSize"]=(fs_info.totalBytes/1024);
  docJson["fsOcc"]=(fs_info.usedBytes/1024);
  docJson["date"]=ntpDate();
  docJson["time"]=ntpTime();
  String json;
  serializeJson(docJson, json);

  server.send(200, "text/html", json);
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}

void handle_Feed(){
  session_authenticated();
  logActions("Feed button pressed online");
  server.send(200, "text/html", (String)feedCat());
}

//return motor switch status
void handle_switch(){
  session_authenticated();
  server.send(200, "text/html", (String)digitalRead(SW_MOTOR));
}

void handle_play(){
  session_authenticated();
  logActions("Audio play (online)");
  server.send(200, "text/html", (String)playAudio());
}

void handle_readLogs(){
  session_authenticated();
  server.send(200, "text/html", readFile(LOGFILE));
}

void handle_readSettings(){
  session_authenticated();
  server.send(200, "text/html", readFile(SETTINGS_FILE));
}

void handle_clearLogs(){
  session_authenticated();
  server.send(200, "text/html", (String)clearLogs());
  logActions("Log cleared");
}

void handle_list(){
  session_authenticated();
  server.send(200, "text/html", listAllFilesInDir("/"));
}

void handle_reboot(){
  session_authenticated();
  server.send(200, "text/html", (String)1);
  logActions("Reboot from web");
  ESP.restart();
}

void handle_GetSchedules(){
  session_authenticated();
  DynamicJsonDocument docJson(250);
  for(unsigned int i=0; i<sizeof(mySettings.schedules)/5; i++){
    docJson["schedules"][i]=mySettings.schedules[i];
  }
  String json;
  serializeJson(docJson, json);
  server.send(200, "text/html", json);
}

void handle_SetSchedules(){
  session_authenticated();
  int i = atoi(server.pathArg(0).c_str());
  const char* schedule = server.pathArg(1).c_str();
  if(i>=0 && i<=9){
    strncpy(mySettings.schedules[i], schedule, sizeof(mySettings.schedules[i]));
    logActions("Updated slot "+String(i)+": "+String(schedule));
    saveSettings();
  }
  handle_GetSchedules();
}

void handle_SetAuth(){
    String login = server.pathArg(0);
    String password = server.pathArg(1);
    String hash=generateHash(login, password);
    if(sizeof(hash) > sizeof(mySettings.hash)){
      logActions("Password too long, not updated");
    }
    
    strncpy(mySettings.login, login.c_str(), sizeof(mySettings.login));
    strncpy(mySettings.password, password.c_str(), sizeof(mySettings.password));
    strncpy(mySettings.hash, hash.c_str(), sizeof(mySettings.hash));
    logActions("Password Updated");
    saveSettings();
    
    handle_readSettings();
}

void handle_SetLed(){
    mySettings.ledStatus=atoi(server.pathArg(0).c_str());
    logActions("Led status: "+mySettings.ledStatus);
    saveSettings();
    handle_readSettings();
}

void handle_SetFeedTime(){
    mySettings.feedTime=atoi(server.pathArg(0).c_str());
    logActions("Feed time: "+mySettings.feedTime);
    saveSettings();
    handle_readSettings();
}


void handle_SetNtpTimezone(){
    strncpy(mySettings.timezone, server.pathArg(0).c_str(), sizeof(mySettings.timezone));
    logActions("NTP timezone: "+String(mySettings.timezone));
    saveSettings();
    handle_readSettings();
}

void handle_SetNtpServer(){
    strncpy(mySettings.ntpServer, server.pathArg(0).c_str(), sizeof(mySettings.ntpServer));
    logActions("NTP server: "+String(mySettings.ntpServer));
    saveSettings();
    handle_readSettings();
}
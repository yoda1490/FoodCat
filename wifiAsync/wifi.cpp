#include <Arduino.h>
#include "wifi.h"
#include "ntp.h"
#include "action.h"

WiFiManager wifiManager;
#ifdef WEBSSL
  #include "ssl.h" //cert & priv key TODO: read from littleFS
  AsyncWebServer server(443);
#else
  AsyncWebServer server(80);
#endif



unsigned long previousWebTime = 0; // Previous time
const long timeoutTime = 2000; // Define timeout time in milliseconds (example: 2000ms = 2s)
String header; // Variable to store the HTTP request




void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("WiFi client fail");
  Serial.println(WiFi.softAPIP());
  Serial.println(myWiFiManager->getConfigPortalSSID());
}


void setupWiFi(){
  //Serial.print("WiFi: ");
  wifiManager.setConfigPortalTimeout(180);
  wifiManager.setDebugOutput(false);
  digitalWrite(PIN_LED, HIGH);
  wifiManager.autoConnect();
  wifiManager.setAPCallback(configModeCallback);
  
  
  if(WiFi.status() != WL_CONNECTED){
    Serial.println("WiFi Fail");
  }else{
    digitalWrite(PIN_LED, LOW);
    Serial.print("\nWiFi connected, ");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP().toString());
  }

  #ifdef WEBSSL
    server.getServer().setRSACert(new BearSSL::X509List(serverCert), new BearSSL::PrivateKey(serverKey));
  #endif

  
  server.on("/", HTTP_GET, handle_index);
  server.on("/settings", HTTP_GET, handle_readSettings);

  //static files
  server.serveStatic("/", LittleFS, "/static/");

  //actions
  server.on("/feed", HTTP_GET, handle_Feed);
  server.on("/play", HTTP_GET, handle_play);
  server.on("/clearlogs", HTTP_GET, handle_clearLogs);
  server.on("/reboot", HTTP_GET, handle_reboot);
  
  //status
  server.on("/switch", HTTP_GET, handle_switch);
  server.on("/list", HTTP_GET, handle_list); //list LittleFS files

  server.onNotFound(handle_NotFound);

  server.begin();
  digitalWrite(PIN_LED, LOW);
}


String generateHash(String login, String password){
    //user:realm:md5(user:realm:pass)
    return ESP8266WebServer::credentialHash(login, STR_REALM, password);
}

bool session_authenticated(AsyncWebServerRequest *request) {
  if (request->authenticate(mySettings.hash.c_str())) {
    return true;
  } else {
    //Serial.println("Not authenticated. Requesting credentials.");
    request->requestAuthentication();
    Serial.println(mySettings.login+" authentication request.");
    logActions(mySettings.login+" authentication request.");
    return false;
  }
}



//---------------------------------------
//     HTTP handlers
//---------------------------------------
void handle_index(AsyncWebServerRequest *request){
  session_authenticated();
  String html="";
  // Display the HTML web page
  html+="<!DOCTYPE html><html>";
  html+="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
  html+="<link rel=\"icon\" href=\"data:,\">";
  html+="<link rel=\"stylesheet\" href=\"cat.css\">";
  html+="<script type=\"text/javascript\" src=\"cat.js\"></script>";
  html+="</head>";
  
  // Web Page Heading
  html+="<body><h1>Miaou</h1>";
  
  html+="<div id=\"blockActions\">";
  html+="<button class=\"button\" id=\"buttonMotory\" onclick=\"buttonPress('feed')\">Feed</button>";
  html+="<br/>";
  html+="<button class=\"button\" id=\"buttonPlay\" onclick=\"buttonPress('play')\">Play record</button>";
  html+="</div>";

  html+="<div id=\"blockSchedules\">";
  html+="<h2>Schedules</h2>";
  html+="<form id=\"schedules\">";
  
   for(unsigned int i=1; i<=sizeof(mySettings.schedules)/5; i++){
    html+="<label for=\"scheduleh-"+(String)i+"\">schedule "+i+": </label>";
    html+="<select name=\"scheduleh-"+(String)i+"\" id=\"scheduleh-"+(String)i+"\">";
    html+="<option title=\"disabled\" value=\"\"></option>";
    for(unsigned int s=0; s<24; s++){
      html+="<option value=\""+(String)s+"\">"+(String)s+"</option>";
    }
    
    html+="</select>";
    html+="<select name=\"schedulem-"+(String)i+"\" id=\"schedulem-"+(String)i+"\">";
    html+="<option title=\"disabled\" value=\"\"></option>";
    for(unsigned int s=0; s<60; s+=10){
      html+="<option value=\""+(String)s+"\">"+(String)s+"</option>";
    }
    html+="</select>";
    html+="<button onclick=saveSchedule("+(String)i+");>save</button>"; 
    html+="<br/>";
  } 
  html+="</form></div>";

  html+="<div id=\"blockSetting\">";
  html+="<h2>Settings</h2>";
  html+="<form id=\"Settings\" action=\"#\" onsubmit=\"saveSettings()\">";
  html+="<label>Login: <input type=\"text\" name=\"login\" value=\""+mySettings.login+"\"/></label><br/>";
  html+="<label>Password: <input type=\"text\" required name=\"password\" value=\"\"/></label><br/>";
  html+="<input type=\"submit\"/>";
  html+="</form>";
  html+="<a href=\"/logs\" target=\"_blank\">Read logs</a>&nbsp;&nbsp;&nbsp;&nbsp;";
  html+="<button onclick=\"buttonPress('clearLogs')\">Clear logs</button>";
  html+="</div>";

  int quality;
  int8_t RSSI = WiFi.RSSI();
  if (RSSI <= -100) {
    quality = 0;
  } else if (RSSI >= -50) {
    quality = 100;
  } else {
    quality = 2 * (RSSI + 100);
  }
  html+="<div id=\"blockWiFi\">";
  html+="<h2>Netwok infos</h2>";
  html+="<p>SSID: " + WiFi.SSID() + "</p>";
  html+="<p>Signal: " + (String)quality+"%</p>";
  html+="<p>IP: " + WiFi.localIP().toString()+"</p>";
  html+="</div>";

  html+="<div id=\"blockNTP\">";
  html+="<h2>"+ntpDate()+" "+ntpTime()+"</h2>";

  html+="</div></body></html>";

  request->send(200, "text/html", html);
  
}


void handle_NotFound(AsyncWebServerRequest *request){
  request->send(404, "text/plain", "Not found");
}

void handle_Feed(AsyncWebServerRequest *request){
  logActions("Feed button pressed online");
  request->send(200, "text/html", (String)feedCat());
}

//return motor switch status
void handle_switch(AsyncWebServerRequest *request){
  request->send(200, "text/html", (String)digitalRead(SW_MOTOR));
}


void handle_play(AsyncWebServerRequest *request){
  logActions("Audio play (online)");
  request->send(200, "text/html", (String)playAudio());
}


void handle_readSettings(AsyncWebServerRequest *request){
  request->send(200, "text/html", readFile(SETTINGS_FILE));
}

void handle_clearLogs(AsyncWebServerRequest *request){
  request->send(200, "text/html", (String)clearLogs());
  logActions("Log cleared");
}

void handle_list(AsyncWebServerRequest *request){
  request->send(200, "text/html", listAllFilesInDir("/"));
}

void handle_reboot(AsyncWebServerRequest *request){
  request->send(200, "text/html", (String)1);
  logActions("Reboot from web");
  ESP.restart();
}
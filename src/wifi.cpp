#include <Arduino.h>
#include <ArduinoJson.h>
#include "wifi.h"
#include "ntp.h"
#include "action.h"
#include "handlers.h"

WiFiManager wifiManager;
#ifdef WEBSSL
  #include "ssl.h" //cert & priv key TODO: read from littleFS
  ESP8266WebServerSecure server(443);
#else
  ESP8266WebServer server(80);
#endif

unsigned long previousWebTime = 0; // Previous time
const long timeoutTime = 2000; // Define timeout time in milliseconds (example: 2000ms = 2s)
String header; // Variable to store the HTTP request




void configModeCallback (WiFiManager *myWiFiManager) {
  logActions("WiFi client fail");
  logActions(WiFi.softAPIP().toString());
  logActions(myWiFiManager->getConfigPortalSSID());
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
    Serial.print("WiFi connected, IP address:");
    Serial.println(WiFi.localIP());
  }

  #ifdef WEBSSL
    server.getServer().setRSACert(new BearSSL::X509List(serverCert), new BearSSL::PrivateKey(serverKey));
  #endif

  server.begin();

  //content serve

  server.serveStatic("/static", LittleFS, "/static");

  //Dynamic
  server.on("/", handle_index);
  server.on("/logs", handle_readLogs);
  server.on("/settings", handle_readSettings);
  server.on("/schedules", handle_GetSchedules);
  server.on(UriRegex("^\\/schedules\\/([0-9])\\/([0-9]{4}|----)$"), handle_SetSchedules);
  server.on(UriRegex("\\/settings\\/auth/([a-zA-Z_]{5,100}):(.{5,100})"), handle_SetAuth);
  server.on(UriRegex("\\/settings\\/led/([01])"), handle_SetLed);
  server.on(UriRegex("\\/settings\\/feedTime/([1-9][0-9]{3})"), handle_SetFeedTime);
  server.on(UriRegex("\\/settings\\/ntp/timezone/([A-Z0-9,.<>/+-]+)$"), handle_SetNtpTimezone);
  server.on(UriRegex("\\/settings\\/ntp/server/([a-zA-Z0-9./_-]+)$"), handle_SetNtpServer);
  

  //actions
  server.on("/feed", handle_Feed);
  server.on("/play", handle_play);
  server.on("/clearlogs", handle_clearLogs);
  server.on("/reboot", handle_reboot);
  
  //status
  server.on("/switch", handle_switch);
  server.on("/status", handle_status);
  server.on("/list", handle_list); //list LittleFS files

  server.onNotFound(handle_NotFound);
  digitalWrite(PIN_LED, LOW);
}


String generateHash(String login, String password){
    return ESP8266WebServer::credentialHash(login, STR_REALM, password);
}



bool session_authenticated() {
  //if (server.authenticateDigest(mySettings.login, mySettings.hash)) {
  if (server.authenticate(mySettings.login, mySettings.password)) {
    return true;
  } else {
    //server.requestAuthentication(DIGEST_AUTH, STR_REALM, STR_AUTH_FAILED);
    server.requestAuthentication(BASIC_AUTH, STR_REALM, STR_AUTH_FAILED);
    logActions(String(mySettings.login) + " authentication request.");
    return false;
  }
}




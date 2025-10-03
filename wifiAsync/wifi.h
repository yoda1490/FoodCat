#include <Arduino.h>

#include <LittleFS.h>
#include "ntp.h"
#include "action.h"

#include <WiFiManager.h>

//#define WEBSSL
#define WIFI_ASYNC

#ifdef WIFI_ASYNC
    #define WEBSERVER_H
    #include <ESPAsyncTCP.h>
    #include <ESPAsyncWebServer.h>
#else
    #ifdef WEBSSL
        extern AsyncWebServer server;
    #else
        extern AsyncWebServer server;
    #endif
#endif
 

#define STR_REALM "global"
#define STR_AUTH_FAILED   "User authentication has failed."


extern WiFiManager wifiManager;
extern unsigned long previousWebTime ; // Previous time
extern const long timeoutTime ; // Define timeout time in milliseconds (example: 2000ms = 2s)
extern String header; // Variable to store the HTTP request


void configModeCallback (WiFiManager *myWiFiManager);
void setupWiFi();

bool session_authenticated();
String generateHash(String login, String password);
    

#ifdef WIFI_ASYNC
    void handle_index(AsyncWebServerRequest *request);
    void handle_NotFound(AsyncWebServerRequest *request);
    void handle_Feed(AsyncWebServerRequest *request);
    void handle_switch(AsyncWebServerRequest *request);
    void handle_play(AsyncWebServerRequest *request);
    void handle_readLogs(AsyncWebServerRequest *request);
    void handle_readSettings(AsyncWebServerRequest *request);
    void handle_clearLogs(AsyncWebServerRequest *request);
    void handle_list(AsyncWebServerRequest *request);
    void handle_reboot(AsyncWebServerRequest *request);
#else
    void handle_index();
    void handle_NotFound();
    void handle_Feed();
    void handle_switch();
    void handle_js();
    void handle_css();
    void handle_play();
    void handle_readLogs();
    void handle_readSettings();
    void handle_clearLogs();
    void handle_list();
    void handle_reboot();
#endif

#include <Arduino.h>
#include <WiFiManager.h> 
#include "ntp.h"
#include "action.h"

//#define WEBSSL

#ifdef WEBSSL
    #include <ESP8266WebServerSecure.h>
    extern ESP8266WebServerSecure server;
#else
    #include <ESP8266WebServer.h> 
    extern ESP8266WebServer server;
#endif

#include <uri/UriBraces.h>
#include <uri/UriRegex.h>

#define STR_REALM "global"
#define STR_AUTH_FAILED   "User authentication has failed."


extern WiFiManager wifiManager;
extern unsigned long previousWebTime ; // Previous time
extern const long timeoutTime ; // Define timeout time in milliseconds (example: 2000ms = 2s)
extern String header; // Variable to store the HTTP request


void configModeCallback (WiFiManager *myWiFiManager);
void setupWiFi();
String generateHash(String login, String password);
bool session_authenticated();



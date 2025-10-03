#include <Arduino.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "wifi.h"
#include "ntp.h"

#define SERIAL_SPEED 9600


ADC_MODE(ADC_VCC);


void setup() {
  Serial.begin(SERIAL_SPEED);
  setupAction();
  
  digitalWrite(PIN_LED, LOW); //led on
  setupWiFi();
  setupNtp();
  //OTA firmaware updates
  //ArduinoOTA.setHostname("...");
  ArduinoOTA.begin();
  ArduinoOTA.setPassword("ESP12F");


  logActions("System booted");

  digitalWrite(PIN_LED, !mySettings.ledStatus); //led status, LOW=ON
}

void loop() {
  time(&now);
  timeCur = localtime(&now);
  
  #ifndef WIFI_ASYNC
    server.handleClient();
  #endif

  if(feedPressed){
    feedPressed=false;
    feedCat();
  }
  if(wifiPressed){
    wifiPressed=false;
    resetSettings();
  }

  if(now > lastNTPRefresh+NTP_REFRESH_INTERVAL){
    logActions("Refreshing NTP");
    now = time(nullptr);
    lastNTPRefresh=now;
  }

  cron();

  if(WiFi.status() != WL_CONNECTED){
    digitalWrite(PIN_LED, HIGH); //switch off led
    logActions((String)WiFi.status());
    delay(200);
    digitalWrite(PIN_LED, LOW); //switch off led
    delay(200);
    digitalWrite(PIN_LED, HIGH); //switch off led
    delay(200);
  }else if(digitalRead(PIN_LED) == mySettings.ledStatus){
    digitalWrite(PIN_LED, !mySettings.ledStatus); //LOW=ON
  }
  
  ArduinoOTA.handle(); 
  delay(50);
}
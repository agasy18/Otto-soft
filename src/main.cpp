#include <Otto.h>

#include <Arduino.h>

#include <ESP8266WiFi.h>

#include <ESP8266mDNS.h>

#include <WiFiUdp.h>

#include <ArduinoOTA.h>

//create wifi_pass.h or define those here
// #define STASSID "ssid"
// #define STAPSK "pass"

#ifndef STASSID
#include "wifi_pass.h"
#endif


const char * ssid = STASSID;
const char * password = STAPSK;

Otto Otto;

#define LeftLeg 0 // left leg pin, servo[0]
#define RightLeg 2 // right leg pin, servo[1]
#define LeftFoot 12 // left foot pin, servo[2]
#define RightFoot 13 // right foot pin, servo[3]
#define Buzzer 5 //buzzer pin

long ultrasound_distance_1() {
  long duration, distance;
  digitalWrite(4, LOW);
  delayMicroseconds(2);
  digitalWrite(4, HIGH);
  delayMicroseconds(10);
  digitalWrite(4, LOW);
  duration = pulseIn(14, HIGH);
  distance = duration / 58;
  return distance;
}

void initOTA() {
  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("myesp8266");

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

}

void setup() {
  Serial.begin(9600);
  Serial.println("Booting");
  Otto.init(LeftLeg, RightLeg, LeftFoot, RightFoot, false, Buzzer);
  Otto.setTrims(101 - 90, 88 - 90, 104 - 90, 88 - 90);

  pinMode(4, OUTPUT);
  pinMode(14, INPUT);
  Otto.home();

  Otto.playGesture(OttoLove);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

}

int tryToConnectToWifi = 0;

int tick = 0;

void loop() {
  if (ultrasound_distance_1() < 15) {
    Otto.playGesture(OttoConfused);
    for (int count = 0; count < 2; count++) {
      Otto.walk(1, 1000, -1); // BACKWARD
    }
    for (int count = 0; count < 4; count++) {
      Otto.turn(1, 1000, 1); // LEFT
    }
  }
  Otto.walk(1, 1000, 1); // FORWARD

  if (tryToConnectToWifi != -1) {
    tryToConnectToWifi++;
    if (tryToConnectToWifi % 30 == 0) {
      if (WiFi.waitForConnectResult(1000) == WL_CONNECTED) {
        initOTA();
        tryToConnectToWifi = -1;
        Otto.playGesture(OttoVictory);
      } else if (tryToConnectToWifi > 100) {
        Serial.println("Connection Failed!");
        Otto.playGesture(OttoFail);
        tryToConnectToWifi = 0;
        ESP.restart();
      } else {
        Otto.playGesture(OttoWave);
      }
    }
  } else {
    ArduinoOTA.handle();
  }
  tick++;
}
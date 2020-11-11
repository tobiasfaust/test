/*#include <Wire.h>  
#include "PCF8574.h"
#include "WEMOS_Motor.h"
#include "SSD1306Wire.h"
#include <ArduinoJson.h>
#include <i2cdetect.h>
#include <DNSServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266mDNS.h>
#include <PubSubClient.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include "uptime.h"
#include "i2cdetect.h"
*/ 
#include "OW2408.h"  // https://github.com/queezythegreat/arduino-ds2408
                                   // https://github.com/PaulStoffregen/OneWire

ow2408* ds2408test = NULL;

void setup() {
  Serial.begin(9600);
  // put your setup code here, to run once: test
  ds2408test = new ow2408();
  ds2408test->setDebugMode(5);
  ds2408test->init(5);
  ds2408test->setOn(0); //Port 0 Device 0
  ds2408test->setOn(1);
  ds2408test->setOn(2);
  ds2408test->setOn(3);
  ds2408test->setOn(4);
  ds2408test->setOn(5);
  ds2408test->setOn(6);
  ds2408test->setOn(7);
  //ds2408test->setOn(8);
  //ds2408test->setOn(15);
  
  delay(1000);
  ds2408test->setOff(0);
  ds2408test->setOff(1);
  ds2408test->setOff(2);
  ds2408test->setOff(3);
  ds2408test->setOff(4);
  ds2408test->setOff(5);
  ds2408test->setOff(6);
  ds2408test->setOff(7);
  //ds2408test->setOff(8);
  //ds2408test->setOff(15);
}

void loop() {
  // put your main code here, to run repeatedly:

}

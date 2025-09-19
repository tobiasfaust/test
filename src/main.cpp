#include "flowercare.h"
#include <Arduino.h>
#include <ArduinoJson.h>

FlowerCare* flowerCare = nullptr;

void flowerCareGetValuesCallback(JsonDocument& json) {
  Serial.printf("FlowerCare data: %s\n", json.as<String>().c_str());
}

void flowerCareOnScanEndCallback() {
  Serial.println("FlowerCare scan ended");
}

void setup() {
  Serial.begin(115200);
  Serial.println("");
  Serial.println("ready");

  Serial.println("Starting FlowerCare");
  flowerCare = new FlowerCare();
  flowerCare->onValues(flowerCareGetValuesCallback);
  flowerCare->onScanEnd(flowerCareOnScanEndCallback);

  Serial.println("Starting BLE scan");
  flowerCare->ScanBLE();
  
  Serial.println("Setup finished");
}

void loop() {
  flowerCare->loop();
}

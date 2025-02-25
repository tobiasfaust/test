#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#include "flowercare.h"

unsigned long lastMillis = 0;
FlowerCare* flowerCare = nullptr;

void flowerCareCallbackGetValues(JsonDocument& json) {
  // sending over MQTT
  serializeJson(json, Serial); Serial.println();
}

void setup() {
  Serial.begin(115200);
  
  Serial.println("Starting FlowerCare");
  flowerCare = new FlowerCare();
  flowerCare->init(); // Scan BLE for devices
  flowerCare->setCb2getValues(flowerCareCallbackGetValues);
  
  const std::vector<FlowerCareDevice>* devices = flowerCare->getDevices();
}

void loop() {
  flowerCare->loop();

  if (millis() - lastMillis >= 1000) {
    lastMillis = millis();
    unsigned long uptime = millis() / 1000;
    unsigned int hours = uptime / 3600;
    unsigned int minutes = (uptime % 3600) / 60;
    unsigned int seconds = uptime % 60;
    Serial.printf("uptime: %02d:%02d:%02d\n", hours, minutes, seconds);
  }
}


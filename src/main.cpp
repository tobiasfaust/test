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

void logN(const int loglevel, const char* format, ...) {
  va_list args;
  va_start(args, format);
  char buffer[256];
  vsnprintf(buffer, sizeof(buffer), format, args);
  Serial.printf("[Log %d] ", loglevel);
  Serial.println(buffer);
  va_end(args);
}

void setup() {
  Serial.begin(115200);
  
  logN(1, "Starting FlowerCare");
  flowerCare = new FlowerCare();
  
  // define Callbacks
  flowerCare->onValues(flowerCareCallbackGetValues);
  flowerCare->onLog(logN);
  flowerCare->onScanEnd([]() {
    logN(1, "Scan ended");
    const String value = "c4:7c:8d:64:42:d0";
    flowerCare->addDevice(NimBLEAddress((std::string)value.c_str(), 0)); // per default active
    const FlowerCareDevice* device = flowerCare->getDevice(NimBLEAddress((std::string)value.c_str(), 0));
    logN(1, "FlowerCareDevice %s created", device->address.toString().c_str());
  });

  flowerCare->ScanBLE(); // Scan BLE for devices

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
    logN(1, "uptime: %02d:%02d:%02d", hours, minutes, seconds);
  }
}


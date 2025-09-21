#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#include "ethernet.h"

ethernet* LAN;

void setup() {
  Serial.begin(115200);
  LAN = new ethernet();
}

void loop() {
  static unsigned long lastCheck = 0;
  if (millis() - lastCheck >= 10000) {
    lastCheck = millis();
    String content;
    if (LAN->http_get(content, "https://www.google.de")) {
      Serial.println("still Online");
    } else {
      Serial.println("Offline");
    }
  }
}


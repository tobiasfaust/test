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

}
## TestRepo
![Build&Deploy](https://github.com/tobiasfaust/test/actions/workflows/BuildAndDeploy.yml/badge.svg)
[![license](https://img.shields.io/badge/Licence-GNU%20v3.0-green)](https://github.com/desktop/desktop/blob/master/LICENSE)
![ESP32 Architecture](https://img.shields.io/badge/Architecture-ESP32-blue)
![ESP8266 Architecture](https://img.shields.io/badge/Architecture-ESP8266-blue)
![GitHub release (latest by date including pre-releases)](https://img.shields.io/github/v/release/tobiasfaust/test?include_prereleases&style=plastic)
![GitHub All Releases](https://img.shields.io/github/downloads/tobiasfaust/test/total?style=plastic)

# FlowerCare

FlowerCare ist eine Arduino-Bibliothek zur Abfrage von Xiaomi FlowerCare Bluetooth-Sensoren. Diese Bibliothek ermöglicht es, Sensordaten wie Temperatur, Feuchtigkeit, Helligkeit und Bodenfruchtbarkeit zu lesen und zu verarbeiten.

## Installation

1. Kopiere die Dateien `flowercare.cpp` und `flowercare.h` in das `src`-Verzeichnis deines Projekts.
2. Füge die erforderlichen Bibliotheken in deiner `platformio.ini` hinzu:

```ini
[env:esp32]
platform = espressif32
board = esp32dev
framework = arduino
lib_deps =
    bblanchon/ArduinoJson @ ^6.18.5
    h2zero/NimBLE-Arduino @ ^1.3.1
```


## Verwendung
### Setup
In der setup-Funktion initialisieren wir die FlowerCare-Instanz und definieren die Callbacks:

```cpp
#include <Arduino.h>
#include "flowercare.h"

FlowerCare* flowerCare = nullptr;

void flowerCareCallbackGetValues(JsonDocument& json) {
  // Senden der Daten über MQTT oder Serial
  serializeJson(json, Serial);
  Serial.println();
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
  
  // Definiere Callbacks
  flowerCare->onValues(flowerCareCallbackGetValues);
  flowerCare->onLog(logN);
  flowerCare->onScanEnd([]() {
    logN(1, "Scan ended");
  });

  flowerCare->ScanBLE(); // Starte BLE-Scan
}
```
## Loop
In der loop-Funktion rufen wir die loop-Methode der FlowerCare-Instanz auf, um die Sensordaten regelmäßig zu aktualisieren:

```cpp
unsigned long lastMillis = 0;

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
```

## API
### Flowercare
* <code>FlowerCare()</code>: Konstruktor, initialisiert die FlowerCare-Instanz.
* <code>void loop()</code>: Muss in der loop-Funktion des Arduino-Sketches aufgerufen werden.
* <code>void ScanBLE()</code>: Startet den BLE-Scan nach FlowerCare-Geräten.
* <code>void addDevice(NimBLEAddress address)</code>: Fügt ein Gerät zur Liste hinzu.
* <code>void setActive(String macaddress, bool active)</code>: Setzt den aktiven Zustand eines Geräts.
* <code>const FlowerCareDevice* getDevice(NimBLEAddress address)</code>: Gibt das Gerät mit der angegebenen Adresse zurück.
* <code>const std::vector<FlowerCareDevice>* getDevices() const</code>: Gibt alle Geräte in einem Vektor zurück.
* <code>const bool& getIsScanActive() const</code>: Gibt den aktiven Zustand des Scans zurück.
* <code>void onValues(std::function<void(JsonDocument&)> callback)</code>: Setzt den Callback für das Abrufen der Werte.
* <code>void onLog(std::function<void(int, const char*, va_list)> onlogCallback)</code>: Setzt den Callback für das Logging.
* <code>void onScanEnd(std::function<void()> OnScanEndCallback)</code>: Setzt den Callback für das Ende des Scans.

### FlowerCareDevice
* <code>NimBLEAddress address</code>: Die Adresse des Geräts.
* <code>bool active</code>: Der aktive Zustand des Geräts.
* <code>int battery</code>: Der Batteriestand des Geräts.
* <code>int brightness</code>: Die Helligkeit.
* <code>float temperature</code>: Die Temperatur.
* <code>int moisture</code>: Die Feuchtigkeit.
* <code>int fertility</code>: Die Bodenfruchtbarkeit.
* <code>String firmwareVersion</code>: Die Firmware-Version.
* <code>unsigned long lastLiveDataUpdate</code>: Der Zeitpunkt des letzten Live-Daten-Updates.
* <code>unsigned long lastBatteryUpdate</code>: Der Zeitpunkt des letzten Batterie-Updates.
* <code>uint8_t failedReads</code>: Die Anzahl der fehlgeschlagenen Lesevorgänge.

## Beispiel
Hier ist ein vollständiges Beispiel, das zeigt, wie die FlowerCare-Bibliothek verwendet werden kann:

```cpp
#include <Arduino.h>
#include "flowercare.h"

FlowerCare* flowerCare = nullptr;

void flowerCareCallbackGetValues(JsonDocument& json) {
  serializeJson(json, Serial);
  Serial.println();
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
  
  flowerCare->onValues(flowerCareCallbackGetValues);
  flowerCare->onLog(logN);
  flowerCare->onScanEnd([]() {
    logN(1, "Scan ended");
  });

  flowerCare->ScanBLE();
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
```
## Lizenz
Dieses Projekt ist unter der GNU v3.0 Lizenz lizenziert. Weitere Informationen finden Sie in der LICENSE.


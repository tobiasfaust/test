#include "flowercare.h"

FlowerCare::FlowerCare() : previousMillis(0), scanCallbacksInstance(*this) {
    NimBLEDevice::init("");
    pBLEScan = NimBLEDevice::getScan();
    pBLEScan->setScanCallbacks(&scanCallbacksInstance, false); // Set the callback for when devices are discovered, no duplicates.
    pBLEScan->setActiveScan(true);
}

void FlowerCare::init() {
    ScanBLE();
}

void FlowerCare::ScanBLE() {
    bool scanStarted = pBLEScan->start(5000, false);
    if (scanStarted) {
        Serial.printf("Scanning for FlowerCare devices...\n");
    }
}

void FlowerCare::addDevice(NimBLEAddress address) {
    for (auto& device : devices) {
        if (device.address == address) {
            return;
        }
    }
    Serial.printf("Adding device: %s\n", address.toString().c_str());
    devices.emplace_back(address);
}

void FlowerCare::ReadSensor(FlowerCareDevice& device, bool getBatteryLevel) {
    NimBLEClient* pClient = NimBLEDevice::createClient();
    Serial.printf("Connecting to %s for updating data (%d bytes free Heap)\n", device.address.toString().c_str(), ESP.getFreeHeap());
    if (pClient->connect(device.address)) {
        NimBLERemoteService* pRemoteService = pClient->getService(NimBLEUUID("00001204-0000-1000-8000-00805f9b34fb"));
        if (pRemoteService) {
            // get battery data
            this ->updateBatteryLevel(device, pRemoteService);
            // Send real-time data read request
            this->updateDeviceData(device, pRemoteService);
        } else {
            Serial.printf("Failed to get service from %s\n", device.address.toString().c_str());
        }
    } else {
        Serial.printf("Failed to connect to %s for updating live data\n", device.address.toString().c_str());
    }
    NimBLEDevice::deleteClient(pClient);
}

void FlowerCare::updateDeviceData(FlowerCareDevice& device, NimBLERemoteService* pRemoteService) {
    NimBLERemoteCharacteristic* pWriteCharacteristic = pRemoteService->getCharacteristic(NimBLEUUID("00001a00-0000-1000-8000-00805f9b34fb"));
    delay(500);
    if (pWriteCharacteristic) {
        uint8_t requestData[2] = {0xA0, 0x1F};
        if (pWriteCharacteristic->writeValue(requestData, 2, true)) {
            Serial.printf("Sent real-time data read request to %s\n", device.address.toString().c_str());
                    
            NimBLERemoteCharacteristic* pReadCharacteristic = pRemoteService->getCharacteristic(NimBLEUUID("00001a01-0000-1000-8000-00805f9b34fb"));
            if (pReadCharacteristic) {
                device.lastLiveDataUpdate = millis();
                std::string value = pReadCharacteristic->readValue();
                const char* val = value.c_str();
                this->printDebugHexValue(val, 16);
                        
                device.temperature = (float)(val[0] | (val[1] << 8)) / 10.0;
                device.moisture = val[7];
                device.brightness = val[3] | (val[4] << 8) | (val[5] << 16) | (val[6] << 24);
                device.fertility = val[8] | (val[9] << 8);

                Serial.printf("Temperature: %.1f\n", device.temperature);
                Serial.printf("Brightness: %d\n", device.brightness);
                Serial.printf("Moisture: %d\n", device.moisture);
                Serial.printf("Fertility: %d\n", device.fertility);
            } else {
                Serial.printf("Failed to get characteristics for reading from %s\n", device.address.toString().c_str());
            }
        } else {
            Serial.printf("Failed to send real-time data read request to %s\n", device.address.toString().c_str());
        }
    } else {
        Serial.printf("Failed to get characteristics for writing from %s\n", device.address.toString().c_str());
    }
}

void FlowerCare::updateBatteryLevel(FlowerCareDevice& device, NimBLERemoteService* pRemoteService) {
    // Read battery level and firmware version
    NimBLERemoteCharacteristic* pBatteryCharacteristic = pRemoteService->getCharacteristic(NimBLEUUID("00001a02-0000-1000-8000-00805f9b34fb"));
    if (pBatteryCharacteristic) {
        device.lastBatteryUpdate = millis();
        std::string value = pBatteryCharacteristic->readValue();
        const char* val = value.c_str();
        this->printDebugHexValue(val, 7);
      
        if (value.length() >= 4) {
            // Convert hex string to battery level and firmware version
            device.battery = (uint8_t)value[0]; 
            device.firmwareVersion = &value[2];
            Serial.printf("Battery: %d%% , Firmware version: %s\n", device.battery, device.firmwareVersion.c_str());
        }
    } else {
        Serial.printf("Failed to get characteristics from %s\n", device.address.toString().c_str());
    }
}

void FlowerCare::printDebugHexValue(const char* value, int len) {
  Serial.printf("DEBUG: Value length n = %d, Hex: ", len);
  for (int i = 0; i < len; i++) {
    Serial.printf("%02x ", (int)value[i]); 
  }
  Serial.println(" ");
}


void FlowerCare::loop() {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= 2000) { // check every second to do a job
        previousMillis = currentMillis;
        
        for (auto& device : devices) {
            if (device.lastLiveDataUpdate == 0 || currentMillis - device.lastLiveDataUpdate >= LiveDataInterval) {                
                if (millis() - device.lastBatteryUpdate >= this->batteryInterval) {
                    ReadSensor(device, true);
                } else {
                    ReadSensor(device, false);
                }
                break; // only one device per loop
            }
        }
    }
}
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
    bool scanStarted = pBLEScan->start(10000, false);
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

void FlowerCare::setCb2getValues(void (*callback)(JsonDocument&)) {
    cb2getValues = callback;
}

void FlowerCare::ReadSensor(FlowerCareDevice& device, bool getBatteryLevel) {
    bool success = false;
    NimBLEClient* pClient = NimBLEDevice::createClient();
    Serial.printf("Connecting to %s for updating data (%d bytes free Heap)\n", device.address.toString().c_str(), ESP.getFreeHeap());
    if (pClient->connect(device.address)) {
        NimBLERemoteService* pRemoteService = pClient->getService(NimBLEUUID("00001204-0000-1000-8000-00805f9b34fb"));
        if (pRemoteService) {
            JsonDocument json;
            json["address"] = device.address.toString();
            // get battery data
            success = this ->updateBatteryLevel(json, device, pRemoteService);
            // Send real-time data read request
            success = this->updateDeviceData(json, device, pRemoteService);
            // Send data to callback function, if defined
            //serializeJson(json, Serial); Serial.println();
            if (cb2getValues) {
                cb2getValues(json);
            }
        } else {
            Serial.printf("Failed to get service from %s\n", device.address.toString().c_str());
        }
    } else {
        Serial.printf("Failed to connect to %s for updating live data\n", device.address.toString().c_str());
    }

    NimBLEDevice::deleteClient(pClient);

    if (!success) {
        device.failedReads++;
        device.lastLiveDataUpdate = millis();
        if (device.failedReads >= this->maxFailedReads) {
            device.active = false;
            Serial.printf("Marking device %s as inactive\n", device.address.toString().c_str());
        }
    } else {
        device.failedReads = 0;
    }
}

bool FlowerCare::updateDeviceData(JsonDocument& json, FlowerCareDevice& device, NimBLERemoteService* pRemoteService) {
    bool ret = false;
    NimBLERemoteCharacteristic* pWriteCharacteristic = pRemoteService->getCharacteristic(NimBLEUUID("00001a00-0000-1000-8000-00805f9b34fb"));
    delay(500);
    if (pWriteCharacteristic) {
        uint8_t requestData[2] = {0xA0, 0x1F};
        if (pWriteCharacteristic->writeValue(requestData, 2, true)) {
            //Serial.printf("Sent real-time data read request to %s\n", device.address.toString().c_str());
                    
            NimBLERemoteCharacteristic* pReadCharacteristic = pRemoteService->getCharacteristic(NimBLEUUID("00001a01-0000-1000-8000-00805f9b34fb"));
            if (pReadCharacteristic) {
                device.lastLiveDataUpdate = millis();
                std::string value = pReadCharacteristic->readValue();
                const char* val = value.c_str();
                //this->printDebugHexValue(val, 16);
                        
                device.temperature = (float)(val[0] | (val[1] << 8)) / 10.0;
                device.moisture = val[7];
                device.brightness = val[3] | (val[4] << 8) | (val[5] << 16) | (val[6] << 24);
                device.fertility = val[8] | (val[9] << 8);

                json["temperature"] = device.temperature;
                json["moisture"] = device.moisture;
                json["brightness"] = device.brightness;
                json["fertility"] = device.fertility;

                ret = true;

            } else {
                Serial.printf("Failed to get characteristics for reading from %s\n", device.address.toString().c_str());
            }
        } else {
            Serial.printf("Failed to send real-time data read request to %s\n", device.address.toString().c_str());
        }
    } else {
        Serial.printf("Failed to get characteristics for writing from %s\n", device.address.toString().c_str());
    }
    return ret;
}

bool FlowerCare::updateBatteryLevel(JsonDocument& json, FlowerCareDevice& device, NimBLERemoteService* pRemoteService) {
    // Read battery level and firmware version
    bool ret = false;
    NimBLERemoteCharacteristic* pBatteryCharacteristic = pRemoteService->getCharacteristic(NimBLEUUID("00001a02-0000-1000-8000-00805f9b34fb"));
    if (pBatteryCharacteristic) {
        device.lastBatteryUpdate = millis();
        std::string value = pBatteryCharacteristic->readValue();
        const char* val = value.c_str();
        //this->printDebugHexValue(val, 7);
      
        if (value.length() >= 4) {
            // Convert hex string to battery level and firmware version
            device.battery = (uint8_t)value[0]; 
            device.firmwareVersion = &value[2];

            json["battery"] = device.battery;
            json["firmwareVersion"] = device.firmwareVersion;

            ret = true;
        }
    } else {
        Serial.printf("Failed to get characteristics from %s\n", device.address.toString().c_str());
    }
    return ret;
}

void FlowerCare::printDebugHexValue(const char* value, int len) {
  Serial.printf("DEBUG: Value length n = %d, Hex: ", len);
  for (int i = 0; i < len; i++) {
    Serial.printf("%02x ", (int)value[i]); 
  }
  Serial.println(" ");
}

void FlowerCare::setActive(String macaddress, bool active) {
    for (auto& device : devices) {
        if (device.address.toString() == macaddress.c_str()) {
            device.active = active;
            Serial.printf("Setting device %s to active: %d\n", macaddress.c_str(), active);
            return;
        }
    }
}

void FlowerCare::loop() {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= 2000) { // check every second to do a job
        previousMillis = currentMillis;
        
        for (auto& device : devices) {
            if (device.active && (device.lastLiveDataUpdate == 0 || currentMillis - device.lastLiveDataUpdate >= this->LiveDataInterval)) {                
                if (millis() - device.lastBatteryUpdate >= this->batteryInterval) {
                    this->ReadSensor(device, true);
                } else {
                    this->ReadSensor(device, false);
                }
                break; // only one device per loop
            }
        }
    }
}
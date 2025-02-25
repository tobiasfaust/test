#ifndef FLOWERCARE_H
#define FLOWERCARE_H

#include <vector>
#include <NimBLEDevice.h>
#include <NimBLEUtils.h>
#include <NimBLEScan.h>
#include <NimBLEAdvertisedDevice.h>
#include <Arduino.h>
#include <ArduinoJson.h>

class FlowerCareDevice {
 public:
    NimBLEAddress address;
    bool active;
    int battery;
    int brightness;
    float temperature;
    int moisture;
    int fertility;
    String firmwareVersion;
    unsigned long lastLiveDataUpdate;
    unsigned long lastBatteryUpdate;
    uint8_t failedReads;

    FlowerCareDevice(NimBLEAddress addr) : 
        address(addr),
        active(true),
        battery(0),
        brightness(0),
        temperature(0.0),
        moisture(0),
        fertility(0),
        firmwareVersion(""),
        lastLiveDataUpdate(0),
        lastBatteryUpdate(0),
        failedReads(0)
        {}
};

class FlowerCare {
  public:
    FlowerCare();
    void init();
    void loop();
    void setCb2getValues(void (*callback)(JsonDocument&));
    void setCb2log(void (*callback)(const int, const char*, ...));
    void setActive(String macaddress, bool active); // mac like: c4:7c:8d:64:42:d0

    const std::vector<FlowerCareDevice>* getDevices() const { return &devices; }

  protected:
    void addDevice(NimBLEAddress address);
    
  private:

    NimBLEScan* pBLEScan;
    std::vector<FlowerCareDevice> devices;

    unsigned long previousMillis;
    const unsigned long LiveDataInterval = 1 * 60 * 1000; // 5 minutes
    const unsigned long batteryInterval =  1 * 60 * 1000; // 1 hour
    const uint8_t maxFailedReads = 5; // Number of failed continously reads before marking device as inactive

    class scanCallbacks : public NimBLEScanCallbacks {
        public:
            scanCallbacks(FlowerCare& flowerCare) : flowerCare(flowerCare) {}
    
            /** Initial discovery, advertisement data only. */
            void onDiscovered(const NimBLEAdvertisedDevice* advertisedDevice) override {
                if (advertisedDevice->haveServiceUUID() && advertisedDevice->getServiceUUID().equals(NimBLEUUID("0000fe95-0000-1000-8000-00805f9b34fb"))) {
                    flowerCare.addDevice(advertisedDevice->getAddress());
                }
            }
    
        private:
            FlowerCare& flowerCare;
    };
    
    scanCallbacks scanCallbacksInstance;
    void (*cb2getValues)(JsonDocument&) = nullptr;
    void (*cb2log)(const int, const char*, ...) = nullptr;

    void ScanBLE();
    void ReadSensor(FlowerCareDevice& device, bool getBatteryLevel = false);
    bool updateDeviceData(JsonDocument& json, FlowerCareDevice& device, NimBLERemoteService* pRemoteService);
    bool updateBatteryLevel(JsonDocument& json, FlowerCareDevice& device, NimBLERemoteService* pRemoteService);

    void printDebugHexValue(const char* value, int len);
};

#endif // FLOWERCARE_H
#ifndef FLOWERCARE_H
#define FLOWERCARE_H

#include <vector>
#include <NimBLEDevice.h>
#include <NimBLEUtils.h>
#include <NimBLEScan.h>
#include <NimBLEAdvertisedDevice.h>
#include <Arduino.h>

#include <ReactESP.h>

class FlowerCareDevice {
 public:
    NimBLEAddress address;
    int battery;
    int brightness;
    float temperature;
    int moisture;
    int fertility;
    String firmwareVersion;

    FlowerCareDevice(NimBLEAddress addr) : address(addr), battery(0), brightness(0), temperature(0.0), moisture(0), fertility(0), firmwareVersion("") {}
};

class FlowerCare {
  public:
    FlowerCare();
    void init();
    void loop();

  protected:
    void addDevice(NimBLEAddress address);
    
  private:
    reactesp::EventLoop event_loop;

    NimBLEScan* pBLEScan;
    std::vector<FlowerCareDevice> devices;

    unsigned long previousMillis;
    unsigned long previousBatteryMillis;
    const unsigned long LiveDataInterval = 1 * 60 * 1000; // 5 minutes
    const unsigned long batteryInterval =  1 * 60 * 1000; // 1 hour

    class scanCallbacks : public NimBLEScanCallbacks {
        public:
            scanCallbacks(FlowerCare& flowerCare) : flowerCare(flowerCare) {}
    
            /** Initial discovery, advertisement data only. */
            void onDiscovered(const NimBLEAdvertisedDevice* advertisedDevice) override {
                if (advertisedDevice->haveServiceUUID() && advertisedDevice->getServiceUUID().equals(NimBLEUUID("0000fe95-0000-1000-8000-00805f9b34fb"))) {
                    flowerCare.addDevice(advertisedDevice->getAddress());
                }
            }
    
            /** onScanEnd */
            //void onScanEnd(const NimBLEScanResults& results, int reason) override {
            //    flowerCare.ReadSensors();
                //flowerCare.ReadBatteryLevels();
            //}
    
        private:
            FlowerCare& flowerCare;
    };
    
    scanCallbacks scanCallbacksInstance;

    void ScanBLE();
    void ReadSensors();
    void ReadSensor(FlowerCareDevice& device, bool getBatteryLevel = false);
    void updateDeviceData(FlowerCareDevice& device, NimBLERemoteService* pRemoteService);
    void updateBatteryLevel(FlowerCareDevice& device, NimBLERemoteService* pRemoteService);

    void printDebugHexValue(const char* value, int len);
};

#endif // FLOWERCARE_H
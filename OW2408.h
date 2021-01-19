#ifndef OW2408_H
#define OW2408_H

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#include "DS2408.h"     // https://github.com/queezythegreat/arduino-ds2408

class ow2408 {
  
  public:
    ow2408();
    void init(uint8_t pin);
    bool setOn(uint8_t port); 
    bool setOff(uint8_t port);
    void setDebugMode(uint8_t debugmode);
    const uint8_t& GetCountDevices() const {return device_count;}
    
  private:
    DS2408* ow;
    Devices devices;
    uint8_t device_count;
    uint8_t debugmode;

    void findDevices();
    void setup_devices();
    String print_device(uint8_t index);
    void print_byte(uint8_t data); // nur test
    bool handlePort(uint8_t port, bool state);
    
};

#endif
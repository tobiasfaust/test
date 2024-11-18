#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#ifdef ESP32
  #include <WiFi.h>
  #include <Preferences.h>
  #define WIFI_OPEN WIFI_AUTH_OPEN
  
  Preferences preferences;
#else
  #include <ESP8266WiFi.h>
  #include <EEPROM.h>
  #define WIFI_OPEN ENC_TYPE_NONE
  #define EEPROM_SIZE 96
#endif

#include <Esp.h>
#include <improv.h>

//ethernet* LAN;

// https://github.com/jnthas/improv-wifi-demo/blob/main/src/esp32-wifiimprov/esp32-wifiimprov.ino
//*** Improv
#define MAX_ATTEMPTS_WIFI_CONNECTION 20
uint8_t x_buffer[255];
uint8_t x_position = 0;
#ifndef LED_BUILTIN
  #define LED_BUILTIN 2
#endif
//*** Improv

String Myssid;
String Mypassword;

void blink_led(int d, int times) {
  for (int j=0; j<times; j++){
    digitalWrite(LED_BUILTIN, HIGH);
    delay(d);
    digitalWrite(LED_BUILTIN, LOW);
    delay(d);
  }
  
}

bool connectWifi(std::string ssid, std::string password) {
  uint8_t count = 0;

  WiFi.begin(ssid.c_str(), password.c_str());

  while (WiFi.status() != WL_CONNECTED) {
    blink_led(500, 1);

    if (count > MAX_ATTEMPTS_WIFI_CONNECTION) {
      WiFi.disconnect();
      return false;
    }
    count++;
  }

  return true;
}


// *** Improv

std::vector<std::string> getLocalUrl() {
  return {
    // URL where user can finish onboarding or use device
    // Recommended to use website hosted by device
    String("http://" + WiFi.localIP().toString()).c_str()
  };
}

void onErrorCallback(improv::Error err) {
  blink_led(2000, 3);
}

void set_state(improv::State state) {  
  
  std::vector<uint8_t> data = {'I', 'M', 'P', 'R', 'O', 'V'};
  data.resize(11);
  data[6] = improv::IMPROV_SERIAL_VERSION;
  data[7] = improv::TYPE_CURRENT_STATE;
  data[8] = 1;
  data[9] = state;

  uint8_t checksum = 0x00;
  for (uint8_t d : data)
    checksum += d;
  data[10] = checksum;

  Serial.write(data.data(), data.size());
}


void send_response(std::vector<uint8_t> &response) {
  std::vector<uint8_t> data = {'I', 'M', 'P', 'R', 'O', 'V'};
  data.resize(9);
  data[6] = improv::IMPROV_SERIAL_VERSION;
  data[7] = improv::TYPE_RPC_RESPONSE;
  data[8] = response.size();
  data.insert(data.end(), response.begin(), response.end());

  uint8_t checksum = 0x00;
  for (uint8_t d : data)
    checksum += d;
  data.push_back(checksum);

  Serial.write(data.data(), data.size());
}

void set_error(improv::Error error) {
  std::vector<uint8_t> data = {'I', 'M', 'P', 'R', 'O', 'V'};
  data.resize(11);
  data[6] = improv::IMPROV_SERIAL_VERSION;
  data[7] = improv::TYPE_ERROR_STATE;
  data[8] = 1;
  data[9] = error;

  uint8_t checksum = 0x00;
  for (uint8_t d : data)
    checksum += d;
  data[10] = checksum;

  Serial.write(data.data(), data.size());
}



void getAvailableWifiNetworks() {
  int networkNum = WiFi.scanNetworks();

  for (int id = 0; id < networkNum; ++id) { 
    std::vector<uint8_t> data = improv::build_rpc_response(
            improv::GET_WIFI_NETWORKS, {WiFi.SSID(id), String(WiFi.RSSI(id)), (WiFi.encryptionType(id) == WIFI_OPEN ? "NO" : "YES")}, false);
    send_response(data);
    delay(1);
  }
  //final response
  std::vector<uint8_t> data =
          improv::build_rpc_response(improv::GET_WIFI_NETWORKS, std::vector<std::string>{}, false);
  send_response(data);
}

bool onCommandCallback(improv::ImprovCommand cmd) {

  switch (cmd.command) {
    case improv::Command::GET_CURRENT_STATE:
    {
      if ((WiFi.status() == WL_CONNECTED)) {
        set_state(improv::State::STATE_PROVISIONED);
        std::vector<uint8_t> data = improv::build_rpc_response(improv::GET_CURRENT_STATE, getLocalUrl(), false);
        send_response(data);

      } else {
        set_state(improv::State::STATE_AUTHORIZED);
      }
      
      break;
    }

    case improv::Command::WIFI_SETTINGS:
    {
      if (cmd.ssid.length() == 0) {
        set_error(improv::Error::ERROR_INVALID_RPC);
        break;
      }
     
      set_state(improv::STATE_PROVISIONING);
      
      Myssid = String(cmd.ssid.c_str());
      Mypassword = String(cmd.password.c_str());

      if (connectWifi(cmd.ssid, cmd.password)) {

        blink_led(100, 3);
        
        //TODO: Persist credentials here

        set_state(improv::STATE_PROVISIONED);        
        std::vector<uint8_t> data = improv::build_rpc_response(improv::WIFI_SETTINGS, getLocalUrl(), false);
        send_response(data);

        //server.begin();

      } else {
        set_state(improv::STATE_STOPPED);
        set_error(improv::Error::ERROR_UNABLE_TO_CONNECT);
      }
      
      break;
    }

    case improv::Command::GET_DEVICE_INFO:
    {
      std::vector<std::string> infos = {
        // Firmware name
        "ImprovWiFiDemo",
        // Firmware version
        "1.0.0",
        // Hardware chip/variant
        "ESP32",
        // Device name
        "SimpleWebServer"
      };
      std::vector<uint8_t> data = improv::build_rpc_response(improv::GET_DEVICE_INFO, infos, false);
      send_response(data);
      break;
    }

    case improv::Command::GET_WIFI_NETWORKS:
    {
      getAvailableWifiNetworks();
      break;
    }

    default: {
      set_error(improv::ERROR_UNKNOWN_RPC);
      return false;
    }
  }

  return true;
}

#ifdef ESP32
void saveWiFiCredentials(const char* ssid, const char* password) {
  preferences.begin("wifi", false);
  preferences.putString("ssid", ssid);
  preferences.putString("password", password);
  preferences.end();
  Serial.println("WiFi credentials saved to NVS");
}

void loadWiFiCredentials(String &ssid, String &password) {
  preferences.begin("wifi", true);
  ssid = preferences.getString("ssid", "");
  password = preferences.getString("password", "");
  preferences.end();
  Serial.println("WiFi credentials loaded from NVS");
}
#else
void saveWiFiCredentials(const char* ssid, const char* password) {
  EEPROM.begin(EEPROM_SIZE);
  for (int i = 0; i < 32; ++i) {
    EEPROM.write(i, ssid[i]);
  }
  for (int i = 0; i < 64; ++i) {
    EEPROM.write(32 + i, password[i]);
  }
  EEPROM.commit();
  Serial.println("WiFi credentials saved to EEPROM");
}

void loadWiFiCredentials(String &ssid, String &password) {
  char ssidArr[32];
  char passwordArr[64];
  EEPROM.begin(EEPROM_SIZE);
  for (int i = 0; i < 32; ++i) {
    ssidArr[i] = EEPROM.read(i);
  }
  for (int i = 0; i < 64; ++i) {
    passwordArr[i] = EEPROM.read(32 + i);
  }
  ssid = String(ssidArr);
  password = String(passwordArr);
  Serial.println("WiFi credentials loaded from EEPROM");
}
#endif

void setup() {
  //Serial.begin(115200);
  //LAN = new ethernet();

  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);

  loadWiFiCredentials(Myssid, Mypassword);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  
  //TODO: Try to connect here if credentials are available
  if (Myssid.length() > 0 && Mypassword.length() > 0) {
    WiFi.begin(Myssid.c_str(), Mypassword.c_str());
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
      Serial.println("WiFi Failed!");
    } else {
      Serial.println("WiFi Connected!");
    }
  }



  blink_led(100, 5); 
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    //wifi_handle_request();
  }

  if (Serial.available() > 0) {
    uint8_t b = Serial.read();

    if (parse_improv_serial_byte(x_position, b, x_buffer, onCommandCallback, onErrorCallback)) {
      x_buffer[x_position++] = b;      
    } else {
      x_position = 0;
    }
  }
}


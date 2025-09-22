#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#include "ethernet.h"
#include <PubSubClient.h>

ethernet* LAN;
WiFiClient espClient;
PubSubClient mqttClient(espClient);

bool ConnectStatusWifi = false;
IPAddress ipadresse = IPAddress(0, 0, 0, 0);

void WifiOnEvent(WiFiEvent_t event) {
    Serial.printf("[WiFi-event] event: %d\n", event);

    switch (event) {
        case ARDUINO_EVENT_WIFI_READY:
            Serial.printf("WiFi interface ready\n");
            break;
        case ARDUINO_EVENT_WIFI_SCAN_DONE:
            Serial.printf("Completed scan for access points\n");
            break;
        case ARDUINO_EVENT_WIFI_STA_START:
            Serial.printf("WiFi client started\n");
            break;
        case ARDUINO_EVENT_WIFI_STA_STOP:
            Serial.printf("WiFi clients stopped\n");
            break;
        case ARDUINO_EVENT_WIFI_STA_CONNECTED:
            Serial.printf("Connected to access point\n");
            break;
        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            Serial.printf("Disconnected from WiFi access point\n");
            ConnectStatusWifi = false;
            break;
        case ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE:
            Serial.printf("Authentication mode of access point has changed\n");
            break;
        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            Serial.printf("WiFi connected with local IP: %s\n", WiFi.localIP().toString().c_str());
            ipadresse = WiFi.localIP();
            ConnectStatusWifi = true;
            break;
        case ARDUINO_EVENT_WIFI_STA_LOST_IP:
            Serial.printf("Lost IP address and IP address is reset to 0\n");
            ConnectStatusWifi = false;
            ipadresse = IPAddress(0, 0, 0, 0);
            break;
        case ARDUINO_EVENT_WPS_ER_SUCCESS:
            Serial.printf("WiFi Protected Setup (WPS): succeeded in enrollee mode\n");
            break;
        case ARDUINO_EVENT_WPS_ER_FAILED:
            Serial.printf("WiFi Protected Setup (WPS): failed in enrollee mode\n");
            break;
        case ARDUINO_EVENT_WPS_ER_TIMEOUT:
            Serial.printf("WiFi Protected Setup (WPS): timeout in enrollee mode\n");
            break;
        case ARDUINO_EVENT_WPS_ER_PIN:
            Serial.printf("WiFi Protected Setup (WPS): pin code in enrollee mode\n");
            break;
        case ARDUINO_EVENT_WIFI_AP_START:
            Serial.printf("WiFi access point started\n");
            break;
        case ARDUINO_EVENT_WIFI_AP_STOP:
            Serial.printf("WiFi access point  stopped\n");
            break;
        case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
            Serial.printf("Client connected\n");
            break;
        case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
            Serial.printf("Client disconnected\n");
            break;
        case ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED:
            Serial.printf("Assigned IP address to client\n");
            break;
        case ARDUINO_EVENT_WIFI_AP_PROBEREQRECVED:
            Serial.printf("Received probe request\n");
            break;
        case ARDUINO_EVENT_WIFI_AP_GOT_IP6:
            Serial.printf("AP IPv6 is preferred\n");
            break;
        case ARDUINO_EVENT_WIFI_STA_GOT_IP6:
            Serial.printf("STA IPv6 is preferred\n");
            break;
        case ARDUINO_EVENT_ETH_GOT_IP6:
            Serial.printf("Ethernet IPv6 is preferred\n");
            break;
        case ARDUINO_EVENT_ETH_START:
            Serial.printf("Ethernet started\n");
             break;
            break;
        case ARDUINO_EVENT_ETH_STOP:
            Serial.printf("Ethernet stopped\n");
            break;
        case ARDUINO_EVENT_ETH_CONNECTED:
            Serial.printf("Ethernet connected\n");
            break;
        case ARDUINO_EVENT_ETH_DISCONNECTED:
            Serial.printf("Ethernet disconnected\n");
            ConnectStatusWifi = false;
            ipadresse = IPAddress(0, 0, 0, 0);
            break;
        case ARDUINO_EVENT_ETH_GOT_IP:
            if (!ConnectStatusWifi) {
              Serial.printf("ETH MAC: %s, IPv4: %s, %s, Mbps: %d\n",
                ETH.macAddress().c_str(),
                ETH.localIP().toString().c_str(),
                (ETH.fullDuplex()?"FULL_DUPLEX":"HALF_DUPLEX"),
                ETH.linkSpeed());
              ipadresse = ETH.localIP();
              ConnectStatusWifi = true;
            }
            break;
        default: break;
    }
}

void setup() {
  Serial.begin(115200);
  WiFi.onEvent(WifiOnEvent);
  LAN = new ethernet();
  mqttClient.setServer("192.168.10.10", 1883);
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

    if (mqttClient.connected()) {
      Serial.println("Publishing message to MQTT");
      mqttClient.publish("test/topic", "hello world");
    } else {
      if (ConnectStatusWifi) {
        if (mqttClient.connect("ESP32Client")) {
          Serial.println("MQTT connected");
        } else {
          Serial.print("MQTT failed, rc=");
          Serial.print(mqttClient.state());
          Serial.println(" try again in 10 seconds");
        }
      }
    }
  }

  if (mqttClient.connected()) {
    mqttClient.loop();
  }
}


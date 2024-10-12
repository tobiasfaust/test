//#include "arduino.h"
#include "ethernet.h"

ethernet::ethernet(): eth_connected(false) {
  // To be called before ETH.begin()
  WiFi.onEvent(std::bind(&ethernet::ETH_event, this, std::placeholders::_1));

  //bool begin(uint8_t phy_addr=ETH_PHY_ADDR, int power=ETH_PHY_POWER, int mdc=ETH_PHY_MDC, int mdio=ETH_PHY_MDIO,
  //           eth_phy_type_t type=ETH_PHY_TYPE, eth_clock_mode_t clk_mode=ETH_CLK_MODE);
  //ETH.begin(ETH_PHY_ADDR, ETH_PHY_POWER, ETH_PHY_MDC, ETH_PHY_MDIO, ETH_PHY_TYPE, ETH_CLK_MODE);
  
  //ETH.begin(ETH_PHY_ADDR, ETH_PHY_POWER);
  ETH.begin(1, 16, 23, 18, ETH_PHY_LAN8720, ETH_CLOCK_GPIO0_IN);
 
  // Static IP, leave without this line to get IP via DHCP
  //bool config(IPAddress local_ip, IPAddress gateway, IPAddress subnet, IPAddress dns1 = 0, IPAddress dns2 = 0);
  //ETH.config(myIP, myGW, mySN, myDNS);

}

void ethernet::ETH_waitForConnect()
{
  while (!this->eth_connected)
    delay(100);
    //yield();
}

bool ethernet::ETH_isConnected()
{
  return eth_connected;
}

void ethernet::ETH_event(WiFiEvent_t event)
{
  switch (event)
  {
      //#if USING_CORE_ESP32_CORE_V200_PLUS
#if ( ( defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 2) ) && ( ARDUINO_ESP32_GIT_VER != 0x46d5afb1 ) )
    // For breaking core v2.0.0
    // Why so strange to define a breaking enum arduino_event_id_t in WiFiGeneric.h
    // compared to the old system_event_id_t, now in tools/sdk/esp32/include/esp_event/include/esp_event_legacy.h
    // You can preserve the old enum order and just adding new items to do no harm
    case ARDUINO_EVENT_ETH_START:
      Serial.println(F("\nETH Started"));
      //set eth hostname here
      ETH.setHostname("WT32-ETH01");
      break;

    case ARDUINO_EVENT_ETH_CONNECTED:
      Serial.println(F("ETH Connected"));
      break;

    case ARDUINO_EVENT_ETH_GOT_IP:
      if (!eth_connected)
      {
        Serial.printf("ETH MAC: %s, IPv4: %s, %s, Mbps: %d\n", 
          ETH.macAddress().c_str(), 
          ETH.localIP().toString().c_str(),
          (ETH.fullDuplex()?"FULL_DUPLEX":"HALF_DUPLEX"),
          ETH.linkSpeed()
         );

        eth_connected = true;
      }

      break;

    case ARDUINO_EVENT_ETH_DISCONNECTED:
      Serial.println("ETH Disconnected");
      eth_connected = false;
      break;

    case ARDUINO_EVENT_ETH_STOP:
      Serial.println("\nETH Stopped");
      eth_connected = false;
      break;

#else

    // For old core v1.0.6-
    // Core v2.0.0 defines a stupid enum arduino_event_id_t, breaking any code for WT32_ETH01 written for previous core
    // Why so strange to define a breaking enum arduino_event_id_t in WiFiGeneric.h
    // compared to the old system_event_id_t, now in tools/sdk/esp32/include/esp_event/include/esp_event_legacy.h
    // You can preserve the old enum order and just adding new items to do no harm
    case SYSTEM_EVENT_ETH_START:
      ET_LOG(F("\nETH Started"));
      //set eth hostname here
      ETH.setHostname("WT32-ETH01");
      break;

    case SYSTEM_EVENT_ETH_CONNECTED:
      ET_LOG(F("ETH Connected"));
      break;

    case SYSTEM_EVENT_ETH_GOT_IP:
      if (!WT32_ETH01_eth_connected)
      {
        ET_LOG3(F("ETH MAC: "), ETH.macAddress(), F(", IPv4: "), ETH.localIP());

        if (ETH.fullDuplex())
        {
          ET_LOG0(F("FULL_DUPLEX, "));
        }
        else
        {
          ET_LOG0(F("HALF_DUPLEX, "));
        }

        ET_LOG1(ETH.linkSpeed(), F("Mbps"));

        WT32_ETH01_eth_connected = true;
      }

      break;

    case SYSTEM_EVENT_ETH_DISCONNECTED:
      ET_LOG("ETH Disconnected");
      WT32_ETH01_eth_connected = false;
      break;

    case SYSTEM_EVENT_ETH_STOP:
      ET_LOG("\nETH Stopped");
      WT32_ETH01_eth_connected = false;
      break;
#endif

    default:
      break;
  }
}
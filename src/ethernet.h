#ifndef ETHERNET_H
#define ETHERNET_H

// defined here before #include <ETH.h> to override

// I²C-address of Ethernet PHY (0 or 1 for LAN8720, 31 for TLK110)
#ifndef ETH_PHY_ADDR
//  #define ETH_PHY_ADDR        1
#endif

// Type of the Ethernet PHY (LAN8720 or TLK110)
//typedef enum { ETH_PHY_LAN8720, ETH_PHY_TLK110, ETH_PHY_RTL8201, ETH_PHY_DP83848, ETH_PHY_DM9051, ETH_PHY_KSZ8081, ETH_PHY_MAX } eth_phy_type_t;

#ifndef ETH_PHY_TYPE
//  #define ETH_PHY_TYPE    ETH_PHY_LAN8720
#endif

// Pin# of the enable signal for the external crystal oscillator (-1 to disable for internal APLL source)
#ifndef ETH_PHY_POWER
//  #define ETH_PHY_POWER  16
#endif

// Pin# of the I²C clock signal for the Ethernet PHY
#ifndef ETH_PHY_MDC
//  #define ETH_PHY_MDC     23
#endif

// Pin# of the I²C IO signal for the Ethernet PHY
#ifndef ETH_PHY_MDIO
//  #define ETH_PHY_MDIO    18
#endif

/*
  //typedef enum { ETH_CLOCK_GPIO0_IN, ETH_CLOCK_GPIO0_OUT, ETH_CLOCK_GPIO16_OUT, ETH_CLOCK_GPIO17_OUT } eth_clock_mode_t;
  ETH_CLOCK_GPIO0_IN   - default: external clock from crystal oscillator
  ETH_CLOCK_GPIO0_OUT  - 50MHz clock from internal APLL output on GPIO0 - possibly an inverter is needed for LAN8720
  ETH_CLOCK_GPIO16_OUT - 50MHz clock from internal APLL output on GPIO16 - possibly an inverter is needed for LAN8720
  ETH_CLOCK_GPIO17_OUT - 50MHz clock from internal APLL inverted output on GPIO17 - tested with LAN8720
*/
#ifndef ETH_CLK_MODE
//  #define ETH_CLK_MODE    ETH_CLOCK_GPIO0_IN  //  ETH_CLOCK_GPIO17_OUT
#endif

#include <ETH.h>
#include <WiFi.h> 
#include <vector>

class ethernet {

  typedef struct {
    String name; 
    uint8_t PHY_ADDR;
    int PHY_POWER; 
    int PHY_MDC;
    int PHY_MDIO; 
    eth_phy_type_t  PHY_TYPE;
    eth_clock_mode_t CLK_MODE;
  } eth_shield_t;

  std::vector<eth_shield_t> lan_shields = {{"WT32_ETH01", 1, 16, 23, 18, ETH_PHY_LAN8720, ETH_CLOCK_GPIO0_IN}, 
                                            {"test", 1, 16, 23, 18, ETH_PHY_LAN8720, ETH_CLOCK_GPIO0_IN}};

  public:
                ethernet();
    void        ETH_waitForConnect();
    bool        ETH_isConnected();

  private:
    
    bool        eth_connected;
    void        ETH_event(WiFiEvent_t event);

};

#endif
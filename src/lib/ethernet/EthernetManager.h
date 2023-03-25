// ethernet manager, used by the webserver and ethernet serial IP
#pragma once

#include "../../Common.h"

#ifndef OPERATIONAL_MODE
#define OPERATIONAL_MODE OFF
#endif

#if OPERATIONAL_MODE == ETHERNET_W5100 || OPERATIONAL_MODE == ETHERNET_W5500

#include "EthernetManager.defaults.h"

#ifdef ESP8266
  #ifndef ETHERNET_W5500
    #error "The ESP8266 Ethernet option supports the W5500 only"
  #endif
  #include <Ethernet2.h>  // https://github.com/adafruit/Ethernet2
#else
  #include <EthernetX.h>
#endif

#define EthernetSettingsSize 128
typedef struct EthernetSettings {
  char masterPassword[40];
  unsigned char mac[6];
  bool dhcpEnabled;
  IPAddress target, ip, dns, gw, sn;
} EthernetSettings;

class EthernetManager {
  public:
    bool init();
    void restart();
    void writeSettings();

    EthernetSettings settings = {
      PASSWORD_DEFAULT,
      MAC,
      STA_DHCP_ENABLED,
      STA_TARGET_IP_ADDR,
      STA_IP_ADDR, STA_GW_ADDR, STA_GW_ADDR, STA_SN_MASK
    };

    bool active = false;
  private:
};

extern EthernetManager ethernetManager;

#endif

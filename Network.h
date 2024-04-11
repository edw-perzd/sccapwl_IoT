
#ifndef Network_H_
#define Network_H_

#include <WiFi.h>
#include <WiFiAP.h>
#include <WiFiClient.h>
#include <WiFiGeneric.h>
#include <WiFiMulti.h>
#include <WiFiSTA.h>
#include <WiFiScan.h>
#include <WiFiServer.h>
#include <WiFiType.h>
#include <WiFiUdp.h>
class Network{
  private:
  friend void WiFiEventConnected(WiFiEvent_t event, WiFiEventInfo_t info);
  friend void WiFiEventGoIP(WiFiEvent_t event, WiFiEventInfo_t info);
  friend void WiFiEventDisconnected(WiFiEvent_t event, WiFiEventInfo_t info);

  public:
   Network();
   void initWiFi();
};
#endif
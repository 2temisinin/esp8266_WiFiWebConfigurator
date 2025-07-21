#ifndef WIFI
#define WIFI

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

extern ESP8266WebServer server;

void wifiConfiguratorBegin(const char *ap_ssid, const char *ap_password);
void wifiConfiguratorLoop();
void tryReconnect();
void setupWebServer();
bool wifiConnect();

#endif // WIFI_H
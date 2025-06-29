#ifndef WIFI_WEB_CONFIGURATOR_H
#define WIFI_WEB_CONFIGURATOR_H

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

extern ESP8266WebServer server;

void wifiConfiguratorBegin(const char *ap_ssid, const char *ap_password);
void wifiConfiguratorLoop();

#endif 
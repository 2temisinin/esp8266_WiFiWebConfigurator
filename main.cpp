#include "WiFiWebConfigurator.h"

WiFiWebConfigurator wifi("ESP8266", "12345678");

void setup()
{
    wifi.begin();
}

void loop()
{
    wifi.loop();
}
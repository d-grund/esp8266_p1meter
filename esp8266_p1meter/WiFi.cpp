#include "settings.h"
// **********************************
// * WIFI                           *
// **********************************
// * Initiate WIFI client
WiFiClient espClient;
// * WiFiManager local initialization. Maybe once its business is done, there is no need to keep it around
WiFiManager wifiManager;
// * Initiate WiFI server
String htmlHead = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>";
String htmlBody = "<p><h>LOGS:</h></p>";
String htmlTail = "</html>\r\n\r\n";
WiFiServer server(80);

bool shouldSaveConfig = false;
// * Callback notifying us of the need to save config
void save_wifi_config_callback ()
{
    Serial.println(F("Should save config"));
    shouldSaveConfig = true;
}

// * Gets called when WiFiManager enters configuration mode
void configModeCallback(WiFiManager *myWiFiManager)
{
    Serial.println(F("Entered config mode"));
    Serial.println(WiFi.softAPIP());

    // * If you used auto generated SSID, print it
    Serial.println(myWiFiManager->getConfigPortalSSID());

    // * Entered config mode, make led toggle faster
    ticker.attach(0.2, tick);
}
// * Gets called when MQTT does not connect, maybe user/password changed
void enterConfigMode()
{
    wifiManager.resetSettings();
    Serial.println(F("Entered config mode"));
    Serial.println(WiFi.softAPIP());

    // * If you used auto generated SSID, print it
    Serial.println(wifiManager.getConfigPortalSSID());

    // * Entered config mode, make led toggle faster
    ticker.attach(0.2, tick);
    
    wifiManager.startConfigPortal();
}

void server_println(String message)
{
    if(htmlBody.length() > 20*80)
    {
        htmlBody.clear();
        htmlBody+= "<p><h>LOGS:</h></p>";

    }
    htmlBody+= "<p> " + message + "</p>";
}

void server_sendto_client ( WiFiClient *client)
{
    client->print(htmlHead);
    client->print(htmlBody);
    client->print(htmlTail);
}
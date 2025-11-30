#include "settings.h"
// **********************************
// * WIFI                           *
// **********************************
// * Initiate WIFI client
WiFiClient espClient;
// * WiFiManager local initialization. Maybe once its business is done, there is no need to keep it around
WiFiManager wifiManager;
// * Initiate WiFI server
String htmlHead = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html><head><meta http-equiv=\"refresh\" content=\"30\"></head><body>";
String htmlBody = "<h1>LOGS:</h1>";
String htmlTail = "</body></html>\r\n\r\n";
WiFiServer server(80);

// * Message buffer to store last 80 messages
#define MESSAGE_BUFFER_SIZE 80
String messageBuffer[MESSAGE_BUFFER_SIZE];
int messageBufferIndex = 0;

bool shouldSaveConfig = false;
// * Callback notifying us of the need to save config
void save_wifi_config_callback ()
{
    server_println(F("Should save config"));
    shouldSaveConfig = true;
}

// * Gets called when WiFiManager enters configuration mode
void configModeCallback(WiFiManager *myWiFiManager)
{
    server_println(F("Entered config mode"));
    server_println(WiFi.softAPIP().toString());

    // * If you used auto generated SSID, print it
    server_println(myWiFiManager->getConfigPortalSSID());

    // * Entered config mode, make led toggle faster
    ticker.attach(0.2, tick);
}
// * Gets called when MQTT does not connect, maybe user/password changed
void enterConfigMode()
{
    wifiManager.resetSettings();
    server_println(F("Entered config mode"));
    server_println(WiFi.softAPIP().toString());

    // * If you used auto generated SSID, print it
    server_println(wifiManager.getConfigPortalSSID());

    // * Entered config mode, make led toggle faster
    ticker.attach(0.2, tick);
    
    wifiManager.startConfigPortal();
}

void server_print(String message)
{
    // Print to serial (without newline)
    Serial.print(message);
}

void server_println(String message)
{
    // Print to serial (with newline)
    Serial.println(message);
    
    // Add message to circular buffer
    messageBuffer[messageBufferIndex] = message;
    messageBufferIndex = (messageBufferIndex + 1) % MESSAGE_BUFFER_SIZE;
}

void server_sendto_client ( WiFiClient *client)
{
    // Build HTML body from message buffer
    htmlBody = "<h1>LOGS:</h1><div style=\"font-family: monospace; white-space: pre-wrap;\">";
    
    // Display messages in order, starting from the oldest
    for (int i = 0; i < MESSAGE_BUFFER_SIZE; i++) {
        int index = (messageBufferIndex + i) % MESSAGE_BUFFER_SIZE;
        if (messageBuffer[index].length() > 0) {
            htmlBody += messageBuffer[index] + "\n";
        }
    }
    
    htmlBody += "</div>";
    
    // Send response with proper error handling
    if (client && client->connected())
    {
        size_t sent = client->print(htmlHead);
        sent += client->print(htmlBody);
        sent += client->print(htmlTail);
        client->flush();
    }
}
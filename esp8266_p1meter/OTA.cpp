#define OTA
#include "settings.h"
// **********************************
// * Setup OTA                      *
// **********************************

void setup_ota()
{
    server_println(F("Arduino OTA activated."));

    ArduinoOTA.onStart([]()
    {
        server_println(F("Arduino OTA: Start"));
    });

    
    ArduinoOTA.onError([](int error,const char *msg)
    {
        server_println("Arduino OTA Error[" + String(error) + "]: " + String(msg));
    });

    ArduinoOTA.begin(WiFi.localIP(),HOSTNAME,OTA_PASSWORD,InternalStorage);
    server_println(F("Arduino setup OTA finished"));
}

void ota_poll()
{
    ArduinoOTA.poll();
}

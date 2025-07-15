#define OTA
#include "settings.h"
// **********************************
// * Setup OTA                      *
// **********************************

void setup_ota()
{
    Serial.println(F("Arduino OTA activated."));

    ArduinoOTA.onStart([]()
    {
        Serial.println(F("Arduino OTA: Start"));
    });

    
    ArduinoOTA.onError([](int error,const char *msg)
    {
        Serial.printf("Arduino OTA Error[%u]: ", error);
        Serial.println(msg);
    });

    ArduinoOTA.begin(WiFi.localIP(),HOSTNAME,OTA_PASSWORD,InternalStorage);
    Serial.println(F("Arduino setup OTA finished"));
}

void ota_poll()
{
    ArduinoOTA.poll();
}

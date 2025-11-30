#define OTA
#include "settings.h"

void setup_ota()
{
    server_println(F("OTA Activated"));
    
    // Port defaults to 8266
    ArduinoOTA.setPort(8266);
    
    // Hostname defaults to esp8266-[ChipID]
    ArduinoOTA.setHostname(HOSTNAME);
    
    // No authentication by default
    ArduinoOTA.setPassword(OTA_PASSWORD);
    
    // Password can be set with it's md5 value as well
    // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
    // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");
    
    ArduinoOTA.onStart([]() {
        server_println(F("OTA Start"));
    });
    
    ArduinoOTA.onEnd([]() {
        server_println(F("OTA End"));
    });
    
    ArduinoOTA.onError([](ota_error_t error) {
        server_println("OTA Error");
    });
    
    ArduinoOTA.begin();
    server_println(F("OTA Setup Finished"));
}

void ota_poll()
{
    ArduinoOTA.handle();
}

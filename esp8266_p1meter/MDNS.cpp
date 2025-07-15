
#define mDNS
#include "settings.h"
// **********************************
// * Setup MDNS discovery service   *
// **********************************

void setup_mdns()
{
    Serial.println(F("Starting MDNS responder service"));

    bool mdns_result = MDNS.begin(HOSTNAME);
    if (mdns_result)
    {
        Serial.println(F("Adding MDNS service"));

        MDNS.addService("http", "tcp", 80);
    }
}
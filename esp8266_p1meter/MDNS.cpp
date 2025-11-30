
#define mDNS
#include "settings.h"
// **********************************
// * Setup MDNS discovery service   *
// **********************************

void setup_mdns()
{
    server_println(F("Starting MDNS responder service"));

    bool mdns_result = MDNS.begin(HOSTNAME);
    if (mdns_result)
    {
        server_println(F("Adding MDNS service"));

        MDNS.addService("http", "tcp", 80);
    }
}
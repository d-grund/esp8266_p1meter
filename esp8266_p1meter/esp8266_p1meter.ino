#define MAIN
// * Include settings
#include "settings.h"





// **********************************
// * Setup Main                     *
// **********************************

void setup()
{
    // * Configure EEPROM
    EEPROM.begin(512);

    // Setup a hw serial connection for communication with the P1 meter and logging (not using inversion)
    Serial.begin(BAUD_RATE, SERIAL_8N1, SERIAL_FULL);
    Serial.println("");
    Serial.println("Swapping UART0 RX to inverted");
    Serial.flush();

    // Invert the RX serialport by setting a register value, this way the TX might continue normally allowing the serial monitor to read println's
    USC0(UART0) = USC0(UART0) | BIT(UCRXI);
    Serial.println("Serial port is ready to recieve.");

    // * Set led pin as output
    pinMode(LED_BUILTIN, OUTPUT);

    // * Start ticker with 0.5 because we start in AP mode and try to connect
    ticker.attach(0.6, tick);

    // * Get MQTT Server settings
    String settings_available = read_eeprom(134, 1);

    if (settings_available == "1")
    {
        read_eeprom(0, 64).toCharArray(MQTT_HOST, 64);   // * 0-63
        read_eeprom(64, 6).toCharArray(MQTT_PORT, 6);    // * 64-69
        read_eeprom(70, 32).toCharArray(MQTT_USER, 32);  // * 70-101
        read_eeprom(102, 32).toCharArray(MQTT_PASS, 32); // * 102-133
    }

    WiFiManagerParameter CUSTOM_MQTT_HOST("host", "MQTT hostname", MQTT_HOST, 64);
    WiFiManagerParameter CUSTOM_MQTT_PORT("port", "MQTT port",     MQTT_PORT, 6);
    WiFiManagerParameter CUSTOM_MQTT_USER("user", "MQTT user",     MQTT_USER, 32);
    WiFiManagerParameter CUSTOM_MQTT_PASS("pass", "MQTT pass",     MQTT_PASS, 32);

    
    // * Reset settings - uncomment for testing
    if(settings_available == "0") wifiManager.resetSettings();

    // * Set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
    wifiManager.setAPCallback(configModeCallback);

    // * Set timeout
    wifiManager.setConfigPortalTimeout(WIFI_TIMEOUT);

    // * Set save config callback
    wifiManager.setSaveConfigCallback(save_wifi_config_callback);

    // * Add all your parameters here
    wifiManager.addParameter(&CUSTOM_MQTT_HOST);
    wifiManager.addParameter(&CUSTOM_MQTT_PORT);
    wifiManager.addParameter(&CUSTOM_MQTT_USER);
    wifiManager.addParameter(&CUSTOM_MQTT_PASS);
        
    // * Set WifiHostName
    WiFi.setHostname(HOSTNAME);

    // * Fetches SSID and pass and tries to connect
    // * Reset when no connection after 10 seconds
    if(settings_available == "0")
    {
        enterConfigMode();
    }
    else if (!wifiManager.autoConnect())
    {
        Serial.println(F("Failed to connect to WIFI and hit timeout"));

        // * Reset and try again, or maybe put it to deep sleep
        ESP.reset();
        delay(WIFI_TIMEOUT);
    }

    // * Read updated parameters
    strcpy(MQTT_HOST, CUSTOM_MQTT_HOST.getValue());
    strcpy(MQTT_PORT, CUSTOM_MQTT_PORT.getValue());
    strcpy(MQTT_USER, CUSTOM_MQTT_USER.getValue());
    strcpy(MQTT_PASS, CUSTOM_MQTT_PASS.getValue());

    // * Save the custom parameters to FS
    if (shouldSaveConfig)
    {
        Serial.println(F("Saving WiFiManager config"));

        write_eeprom(0, 64, MQTT_HOST);   // * 0-63
        write_eeprom(64, 6, MQTT_PORT);   // * 64-69
        write_eeprom(70, 32, MQTT_USER);  // * 70-101
        write_eeprom(102, 32, MQTT_PASS); // * 102-133
        write_eeprom(134, 1, "1");        // * 134 --> always "1"
        EEPROM.commit();
    }

    // * If you get here you have connected to the WiFi
    Serial.println(F("Connected to WIFI..."));

    // * Keep LED on
    ticker.detach();
    digitalWrite(LED_BUILTIN, LOW);


    // * Configure OTA
    setup_ota();

    // * Startup MDNS Service
    setup_mdns();

    // * Attach MQTT client
    setup_mqqt();
 
    // * Setup MQTT
    Serial.printf("MQTT connecting to: %s:%s\n", MQTT_HOST, MQTT_PORT);
    server_println("MQTT connecting to: " + String(MQTT_HOST) + String( MQTT_PORT));
    IPAddress resolvedIP;
    if(!WiFi.hostByName(MQTT_HOST, resolvedIP))
    {
      Serial.printf("Unable to resolve host, connect using name: %s\n",MQTT_HOST);
      mqtt_client.setServer(MQTT_HOST, atoi(MQTT_PORT));
    }
    else
    {
      mqtt_client.setServer(resolvedIP, atoi(MQTT_PORT));
    }

}

// **********************************
// * Loop                           *
// **********************************

void loop()
{
    ota_poll();
    MDNS.update();
    long now = millis();
    if(server.hasClient())
    {
        WiFiClient client = server.available();
        // Read the first line of HTTP request
        String req = client.readStringUntil('\r');
        // First line of HTTP request looks like "GET /path HTTP/1.1"
        // Retrieve the "/path" part by finding the spaces
        int addr_start = req.indexOf(' ');
        int addr_end = req.indexOf(' ', addr_start + 1);
        req = req.substring(addr_start + 1, addr_end);
        client.flush();
        if (req == "/Logs")
        {
            server_sendto_client (&client);

        }
        else
        {
            client.print("HTTP/1.1 404 Not Found\r\n\r\n");
        }
    }


    if ( !mqtt_client.connected())
    {
        if (now - LAST_RECONNECT_ATTEMPT > 5000 )
        {
            LAST_RECONNECT_ATTEMPT = now;
            MQTT_RECONNECT_TRIES++;
            int reconnectStatus = mqtt_reconnect();

            if (reconnectStatus == 0)
            {
                LAST_RECONNECT_ATTEMPT = 0;
                MQTT_RECONNECT_TRIES = 0;
            }
            else
            {
                if(reconnectStatus ==5 )
                {
                  Serial.println(" Access denied connecting to MQTT entering config mode");
                  enterConfigMode();
                }
                
                delay(5000);

            }
        }
    }
    else
    {
        mqtt_client.loop();
    }
    
    if (now - LAST_UPDATE_SENT > UPDATE_INTERVAL) {
        read_p1_hardwareserial();
    }
}

#define MQTT
#include "settings.h"

// **********************************
// * MQTT                           *
// **********************************
// * Initiate MQTT client
PubSubClient mqtt_client;
// * To be filled with EEPROM data
char MQTT_HOST[64] = "";
char MQTT_PORT[6]  = "";
char MQTT_USER[32] = "";
char MQTT_PASS[32] = "";
int MQTT_RECONNECT_TRIES = 0;
mqtt_state_t mqtt_connection_state = MQTT_STATE_DISCONNECTED;
// * MQTT Last reconnection counter
long LAST_RECONNECT_ATTEMPT = 0;
long LAST_UPDATE_SENT = 0;

void setup_mqqt()
{
    mqtt_client.setClient(espClient);
}

// * Send a message to a broker topic
void send_mqtt_message(const char *topic, char *payload)
{
    //server_println("MQTT Outgoing on " + String(topic) + ": " + String(payload));

    bool result = mqtt_client.publish(topic, payload, false);

    if (!result)
    {
        server_println("MQTT publish to topic " + String(topic) + " failed");
    }
}

// * Reconnect to MQTT server and subscribe to in and out topics
int mqtt_reconnect()
{

    server_println("MQTT connection attempt " + String(MQTT_RECONNECT_TRIES) + " / " + String(MQTT_MAX_RECONNECT_TRIES) + " ...");

    // * Attempt to connect
    if (mqtt_client.connect(HOSTNAME, MQTT_USER, MQTT_PASS))
    {
        server_println(F("MQTT connected!"));

        // * Once connected, publish an announcement...
        char *message = new char[16 + strlen(HOSTNAME) + 1];
        strcpy(message, "p1 meter alive: ");
        strcat(message, HOSTNAME);
        mqtt_client.publish("hass/status", message);
        server_println("MQTT root topic: " + String(MQTT_ROOT_TOPIC));
    }
    else
    {
        int state = mqtt_client.state();
        server_println("MQTT Connection failed: rc=" + String(state));
        return state;

    }

    return 0;
}

// * Get human-readable MQTT error message
const char* mqtt_error_string(int error_code)
{
    switch(error_code) {
        case 0: return "Connected";
        case 1: return "Server unavailable";
        case 2: return "Invalid client ID";
        case 3: return "Server unavailable";
        case 4: return "Bad username or password";
        case 5: return "Not authorized";
        case -1: return "Socket connection failed";
        case -2: return "Socket connection lost";
        case -3: return "Connect failed";
        case -4: return "Disconnected";
        default: return "Unknown error";
    }
}

// * Enhanced MQTT connection handling with exponential backoff
void handle_mqtt_connection(unsigned long now)
{
    if (mqtt_client.connected()) {
        if (mqtt_connection_state != MQTT_STATE_CONNECTED) {
            mqtt_connection_state = MQTT_STATE_CONNECTED;
            MQTT_RECONNECT_TRIES = 0;
        }
        mqtt_client.loop();
        return;
    }

    // Not connected - try to reconnect with exponential backoff
    if (mqtt_connection_state == MQTT_STATE_CONNECTED) {
        // We were connected but now disconnected - log it
        server_println("MQTT: Connection lost");
        mqtt_connection_state = MQTT_STATE_DISCONNECTED;
    }

    // Check if we've exceeded max retries
    if (MQTT_RECONNECT_TRIES >= MQTT_MAX_RECONNECT_TRIES) {
        mqtt_connection_state = MQTT_STATE_MAX_RETRIES_EXCEEDED;
        
        // Only log periodically to avoid spamming serial output
        static unsigned long last_max_retries_log = 0;
        if (now - last_max_retries_log > 60000) {
            server_println("MQTT: Max reconnection attempts (" + String(MQTT_MAX_RECONNECT_TRIES) + ") exceeded");
            last_max_retries_log = now;
        }
        return;
    }

    // Calculate exponential backoff: 5s, 10s, 20s, 40s, ... capped at 5 minutes
    unsigned long backoff = (1UL << MQTT_RECONNECT_TRIES) * MQTT_BACKOFF_BASE;
    if (backoff > MQTT_BACKOFF_MAX) {
        backoff = MQTT_BACKOFF_MAX;
    }

    if (now - LAST_RECONNECT_ATTEMPT > backoff) {
        LAST_RECONNECT_ATTEMPT = now;
        MQTT_RECONNECT_TRIES++;
        mqtt_connection_state = MQTT_STATE_CONNECTING;

        server_println("MQTT: Reconnect attempt " + String(MQTT_RECONNECT_TRIES) + "/" + String(MQTT_MAX_RECONNECT_TRIES) + " (backoff: " + String(backoff) + " ms)");

        int status = mqtt_reconnect();

        if (status == 0) {
            mqtt_connection_state = MQTT_STATE_CONNECTED;
            MQTT_RECONNECT_TRIES = 0;
            server_println("MQTT: Successfully reconnected!");
        } else {
            // Determine connection state based on error code
            if (status == 4 || status == 5) {
                mqtt_connection_state = MQTT_STATE_AUTH_FAILED;
                server_println("MQTT: Authentication failed (" + String(mqtt_error_string(status)) + ") - entering config mode");
                enterConfigMode();
            } else {
                mqtt_connection_state = MQTT_STATE_DISCONNECTED;
                server_println("MQTT: Connection error - " + String(mqtt_error_string(status)) + " (code " + String(status) + ")");
            }
        }
    }
}

void send_metric(String name, long metric)
{
    //server_println("Sending metric to broker: " + name + "=" + String(metric));

    char output[10];
    ltoa(metric, output, sizeof(output));

    String topic = String(MQTT_ROOT_TOPIC) + "/" + name;
    send_mqtt_message(topic.c_str(), output);
}

void send_data_to_broker()
{
    for(const auto& pair : P1Values) {
        send_metric(String(pair.first.c_str()), pair.second);
    }
    
}

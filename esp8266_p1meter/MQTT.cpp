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
int MQTT_RECONNECT_TRIES=0;
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
    Serial.printf("MQTT Outgoing on %s: ", topic);
    Serial.println(payload);

    bool result = mqtt_client.publish(topic, payload, false);

    if (!result)
    {
        Serial.printf("MQTT publish to topic %s failed\n", topic);
    }
}

// * Reconnect to MQTT server and subscribe to in and out topics
int mqtt_reconnect()
{

    Serial.printf("MQTT connection attempt %d / %d ...\n", MQTT_RECONNECT_TRIES, MQTT_MAX_RECONNECT_TRIES);

    // * Attempt to connect
    if (mqtt_client.connect(HOSTNAME, MQTT_USER, MQTT_PASS))
    {
        Serial.println(F("MQTT connected!"));
        server_println("MQTT connected!");

        // * Once connected, publish an announcement...
        char *message = new char[16 + strlen(HOSTNAME) + 1];
        strcpy(message, "p1 meter alive: ");
        strcat(message, HOSTNAME);
        mqtt_client.publish("hass/status", message);
        Serial.printf("MQTT root topic: %s\n", MQTT_ROOT_TOPIC);
    }
    else
    {
        int state = mqtt_client.state();
        Serial.print(F("MQTT Connection failed: rc="));
        Serial.println(state);
        return state;

    }

    return 0;
}

void send_metric(String name, long metric)
{
    Serial.print(F("Sending metric to broker: "));
    Serial.print(name);
    Serial.print(F("="));
    Serial.println(metric);

    char output[10];
    ltoa(metric, output, sizeof(output));

    String topic = String(MQTT_ROOT_TOPIC) + "/" + name;
    send_mqtt_message(topic.c_str(), output);
}

void send_data_to_broker()
{
    send_metric("consumption_low_tarif", CONSUMPTION_LOW_TARIF);
    send_metric("consumption_high_tarif", CONSUMPTION_HIGH_TARIF);
    send_metric("returndelivery_low_tarif", RETURNDELIVERY_LOW_TARIF);
    send_metric("returndelivery_high_tarif", RETURNDELIVERY_HIGH_TARIF);
    send_metric("actual_consumption", ACTUAL_CONSUMPTION);
    send_metric("actual_returndelivery", ACTUAL_RETURNDELIVERY);

    send_metric("l1_instant_power_usage", L1_INSTANT_POWER_USAGE);
    send_metric("l2_instant_power_usage", L2_INSTANT_POWER_USAGE);
    send_metric("l3_instant_power_usage", L3_INSTANT_POWER_USAGE);
    send_metric("l1_instant_power_current", L1_INSTANT_POWER_CURRENT);
    send_metric("l2_instant_power_current", L2_INSTANT_POWER_CURRENT);
    send_metric("l3_instant_power_current", L3_INSTANT_POWER_CURRENT);
    send_metric("l1_voltage", L1_VOLTAGE);
    send_metric("l2_voltage", L2_VOLTAGE);
    send_metric("l3_voltage", L3_VOLTAGE);
    
    send_metric("gas_meter_m3", GAS_METER_M3);

    send_metric("actual_tarif_group", ACTUAL_TARIF);
    send_metric("short_power_outages", SHORT_POWER_OUTAGES);
    send_metric("long_power_outages", LONG_POWER_OUTAGES);
    send_metric("short_power_drops", SHORT_POWER_DROPS);
    send_metric("short_power_peaks", SHORT_POWER_PEAKS);
}

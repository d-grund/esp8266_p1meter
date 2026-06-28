// **********************************
// * Settings                       *
// **********************************
#ifndef SETTINGS_H
#define SETTINGS_H


#include <FS.h>
#include <EEPROM.h>
#include <DNSServer.h>
#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <WiFiManager.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>
#include <Time.h>


#ifdef OTA
#include <ArduinoOTA.h>
#endif

// * led blinker library
extern Ticker ticker;
// * WIFI client
extern WiFiClient espClient;
// *  WiFi Server
extern WiFiServer server;
// * WiFiManager local initialization. Maybe once its business is done, there is no need to keep it around
extern WiFiManager wifiManager;
// * MQTT client
extern PubSubClient mqtt_client;

// * Ticker
void tick();
// * MQTT
void setup_mqqt();
int mqtt_reconnect();
void send_data_to_broker();
// * MDNS
void setup_mdns();
// * OTA
void setup_ota();
void ota_poll();
// * P1
unsigned int CRC16(unsigned int crc, unsigned char *buf, int len);
bool isNumber(char *res, int len);
int FindCharInArrayRev(char array[], char c, int len);
long getValue(char *buffer, int maxlen, char startchar, char endchar);
bool decode_telegram(int len);
void processLine(int len, long now);
void read_p1_hardwareserial(long now);
//WiFi
void enterConfigMode();
void save_wifi_config_callback ();
void configModeCallback(WiFiManager *myWiFiManager);
void server_sendto_client ( WiFiClient *client);
void server_println(String message);
void server_print(String message);
// EEPROM
String read_eeprom(int offset, int len);
void write_eeprom(int offset, int len, String value);


// Update treshold in milliseconds, messages will only be sent on this interval
#define UPDATE_INTERVAL 30000  // 30 seconds
//#define UPDATE_INTERVAL 300000 // 5 minutes

// * Baud rate for both hardware and software 
#define BAUD_RATE 115200

// The used serial pins, note that this can only be UART0, as other serial port doesn't support inversion
// By default the UART0 serial will be used. These settings displayed here just as a reference. 
// #define SERIAL_RX RX
// #define SERIAL_TX TX

// * The hostname of our little creature
#define HOSTNAME "p1meter"

// * The password used for OTA
#define OTA_PASSWORD "admin"
#ifdef MAIN
// * Wifi timeout in milliseconds
#define WIFI_TIMEOUT 30000
#endif

// * MQTT network settings
#define MQTT_MAX_RECONNECT_TRIES 20
#define MQTT_BACKOFF_BASE 5000        // Base backoff time in milliseconds (5 seconds)
#define MQTT_BACKOFF_MAX 300000       // Maximum backoff time in milliseconds (5 minutes)
// * MQTT root topic
#define MQTT_ROOT_TOPIC "sensors/power/p1meter"

// * MQTT connection state enumeration (renamed to avoid conflict with PubSubClient macros)
typedef enum {
    MQTT_STATE_DISCONNECTED = 0,
    MQTT_STATE_CONNECTING = 1,
    MQTT_STATE_CONNECTED = 2,
    MQTT_STATE_AUTH_FAILED = 3,
    MQTT_STATE_MAX_RETRIES_EXCEEDED = 4
} mqtt_state_t;

// * MQTT Last reconnection counter
extern long LAST_RECONNECT_ATTEMPT;
extern long LAST_UPDATE_SENT;
extern int MQTT_RECONNECT_TRIES;
extern mqtt_state_t mqtt_connection_state;

#if defined(MAIN) || defined(MQTT)
// * To be filled with EEPROM data
extern char MQTT_HOST[64];
extern char MQTT_PORT[6];
extern char MQTT_USER[32];
extern char MQTT_PASS[32];
#endif

// * Max telegram length
#define P1_MAXLINELENGTH 1050
// * Set to store received telegram
extern char telegram[P1_MAXLINELENGTH];
extern int telegramRead;  // Count number of telegrams read

// * Set to store the data values read
#include <iostream>
#include <map>
extern std::map<std::string,long> P1Values;



// ******************************************
// * Callback for saving WIFI config        *
// ******************************************
extern bool shouldSaveConfig;

// ******************************************
// * MQTT Function Declarations             *
// ******************************************
void handle_mqtt_connection(unsigned long now);
const char* mqtt_error_string(int error_code);


#endif

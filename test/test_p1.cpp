#include <unity.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <map>


extern std::map<std::string,long> P1Values;
unsigned int CRC16(unsigned int crc, unsigned char *buf, int len);
bool isNumber(char *res, int len);
int FindCharInArrayRev(char array[], char c, int len);
long getValue(char *buffer, int maxlen, char startchar, char endchar);
bool decode_telegram(int len);
void processLine(int len);
void read_p1_hardwareserial();
long LAST_UPDATE_SENT;
// ========================================
// * ARDUINO FRAMEWORK STUBS
// ========================================
extern "C" {
    void setup() {}
    void loop() {}
}

// ========================================
// * MOCK FLASH STRING HELPER
// ========================================
typedef const char __FlashStringHelper;

// ========================================
// * MOCK SERIAL CLASS
// ========================================
class MockSerial {
public:
    void print(const char* str) {}
    void print(char c) {}
    void println(const char* str) {}
    bool available() { return false; }
    int readBytesUntil(char terminator, char* buffer, size_t length) { return 0; }
};

MockSerial Serial;

// ========================================
// * MOCK FUNCTIONS
// ========================================
void yield() {}

unsigned long millis() {
    return 0;
}

void send_data_to_broker() {
}

// Mock ESP functions
class MockESP {
public:
    void wdtDisable() {}
    void wdtEnable(int timeout) {}
} ESP;

#define F(x) (const __FlashStringHelper*)(x)

// ========================================
// * DEFINE P1 CONSTANTS
// ========================================
#define P1
#define P1_MAXLINELENGTH 1050

// ========================================
// * INCLUDE P1.CPP (without #include "settings.h")
// ========================================
#define SETTINGS_H
#include <../esp8266_p1meter/P1.cpp>


// ========================================
// * UNIT TESTS WITH REAL TELEGRAMS
// ========================================

void test_crc16_basic() {
    unsigned char data[] = {0x01, 0x02, 0x03};
    unsigned int crc = CRC16(0x0000, data, 3);
    TEST_ASSERT_NOT_EQUAL(0, crc);
}

void test_find_char_in_array_rev() {
    char array[] = "Hello World!";
    int pos = FindCharInArrayRev(array, 'l', strlen(array));
    TEST_ASSERT_EQUAL(9, pos);
}

void test_find_char_in_array_rev_not_found() {
    char array[] = "Hello";
    int pos = FindCharInArrayRev(array, 'x', strlen(array));
    TEST_ASSERT_EQUAL(-1, pos);
}

void test_is_number_valid() {
    char num[] = "123.45";
    TEST_ASSERT_TRUE(isNumber(num, 6));
}

void test_is_number_invalid() {
    char num[] = "12A.45";
    TEST_ASSERT_FALSE(isNumber(num, 6));
}

void test_is_number_empty() {
    char num[] = "";
    TEST_ASSERT_TRUE(isNumber(num, 0));
}

void test_parse_consumption_low_tarif() {
    memset(telegram, 0, sizeof(telegram));
    strcpy(telegram, "1-0:1.8.1(000992.992*kWh)");
    int len = strlen(telegram);
    decode_telegram(len);
    TEST_ASSERT_EQUAL(992991, P1Values["consumption_low_tarif"]);
}

void test_parse_consumption_high_tarif() {
    memset(telegram, 0, sizeof(telegram));
    strcpy(telegram, "1-0:1.8.2(000560.157*kWh)");
    int len = strlen(telegram);
    decode_telegram(len);
    TEST_ASSERT_EQUAL(560157, P1Values["consumption_high_tarif"]);
}

void test_parse_actual_consumption() {
    memset(telegram, 0, sizeof(telegram));
    strcpy(telegram, "1-0:1.7.0(00.424*kW)");
    int len = strlen(telegram);
    decode_telegram(len);
    TEST_ASSERT_EQUAL(423,  P1Values["actual_consumption"]);
}

void test_parse_actual_return_delivery() {
    memset(telegram, 0, sizeof(telegram));
    strcpy(telegram, "1-0:2.7.0(00.000*kW)");
    int len = strlen(telegram);
    decode_telegram(len);
    TEST_ASSERT_EQUAL(0, P1Values["actual_returndelivery"]);
}

void test_parse_l1_instant_power() {
    memset(telegram, 0, sizeof(telegram));
    strcpy(telegram, "1-0:21.7.0(00.378*kW)");
    int len = strlen(telegram);
    decode_telegram(len);
    TEST_ASSERT_EQUAL(378, P1Values["l1_instant_power_usage"]);
}

void test_parse_l2_instant_power() {
    memset(telegram, 0, sizeof(telegram));
    strcpy(telegram, "1-0:41.7.0(00.500*kW)");
    int len = strlen(telegram);
    decode_telegram(len);
    TEST_ASSERT_EQUAL(500, P1Values["l2_instant_power_usage"]);
}

void test_parse_l3_instant_power() {
    memset(telegram, 0, sizeof(telegram));
    strcpy(telegram, "1-0:61.7.0(00.625*kW)");
    int len = strlen(telegram);
    decode_telegram(len);
    TEST_ASSERT_EQUAL(625, P1Values["l3_instant_power_usage"]);
}

void test_parse_l1_instant_current() {
    memset(telegram, 0, sizeof(telegram));
    strcpy(telegram, "1-0:31.7.0(002*A)");
    int len = strlen(telegram);
    decode_telegram(len);
    TEST_ASSERT_EQUAL(2000, P1Values["l1_instant_power_current"]);
}

void test_parse_l2_instant_current() {
    memset(telegram, 0, sizeof(telegram));
    strcpy(telegram, "1-0:51.7.0(003*A)");
    int len = strlen(telegram);
    decode_telegram(len);
    TEST_ASSERT_EQUAL(3000, P1Values["l2_instant_power_current"]);
}

void test_parse_l3_instant_current() {
    memset(telegram, 0, sizeof(telegram));
    strcpy(telegram, "1-0:71.7.0(001*A)");
    int len = strlen(telegram);
    decode_telegram(len);
    TEST_ASSERT_EQUAL(1000, P1Values["l3_instant_power_current"]);
}

void test_parse_l1_voltage() {
    memset(telegram, 0, sizeof(telegram));
    strcpy(telegram, "1-0:32.7.0(232.0*V)");
    int len = strlen(telegram);
    decode_telegram(len);
    TEST_ASSERT_EQUAL(232000, P1Values["l1_voltage"]);
}

void test_parse_l2_voltage() {
    memset(telegram, 0, sizeof(telegram));
    strcpy(telegram, "1-0:52.7.0(232.5*V)");
    int len = strlen(telegram);
    decode_telegram(len);
    TEST_ASSERT_EQUAL(232500, P1Values["l2_voltage"]);
}

void test_parse_l3_voltage() {
    memset(telegram, 0, sizeof(telegram));
    strcpy(telegram, "1-0:72.7.0(233.0*V)");
    int len = strlen(telegram);
    decode_telegram(len);
    TEST_ASSERT_EQUAL(233000, P1Values["l3_voltage"]);
}

void test_parse_gas_meter() {
    memset(telegram, 0, sizeof(telegram));
    strcpy(telegram, "0-1:24.2.1(150531200000S)(00811.923*m3)");
    int len = strlen(telegram);
    decode_telegram(len);
    TEST_ASSERT_EQUAL(811923, P1Values["gas_meter_m3"]);
}

void test_parse_actual_tariff() {
    memset(telegram, 0, sizeof(telegram));
    strcpy(telegram, "0-0:96.14.0(0001)");
    int len = strlen(telegram);
    decode_telegram(len);
    TEST_ASSERT_EQUAL(1, P1Values["actual_tarif"]);
}

void test_parse_short_power_outages() {
    memset(telegram, 0, sizeof(telegram));
    strcpy(telegram, "0-0:96.7.21(00003)");
    int len = strlen(telegram);
    P1Values["short_power_outages"] = 999;  // Set to known value before parsing
    decode_telegram(len);
    TEST_ASSERT_EQUAL(3, P1Values["short_power_outages"]);
}

void test_parse_long_power_outages() {
    memset(telegram, 0, sizeof(telegram));
    strcpy(telegram, "0-0:96.7.9(00001)");
    int len = strlen(telegram);
     P1Values["long_power_outages"] = 999;  // Set to known value before parsing
    decode_telegram(len);
    TEST_ASSERT_EQUAL(1,  P1Values["long_power_outages"]);
}

void test_parse_short_power_drops() {
    memset(telegram, 0, sizeof(telegram));
    strcpy(telegram, "1-0:32.32.0(00000)");
    int len = strlen(telegram);
     P1Values["short_power_drops"] = 999;  // Set to known value before parsing
    decode_telegram(len);
    TEST_ASSERT_EQUAL(0,  P1Values["short_power_drops"]);
}

void test_parse_short_power_peaks() {
    memset(telegram, 0, sizeof(telegram));
    strcpy(telegram, "1-0:32.36.0(00005)");
    int len = strlen(telegram);
    P1Values["short_power_peaks"] = 999;  // Set to known value before parsing
    decode_telegram(len);
    TEST_ASSERT_EQUAL(5, P1Values["short_power_peaks"]);
}

void test_parse_return_delivery_low_tarif() {
    memset(telegram, 0, sizeof(telegram));
    strcpy(telegram, "1-0:2.8.1(000100.500*kWh)");
    int len = strlen(telegram);
    decode_telegram(len);
    TEST_ASSERT_EQUAL(100500, P1Values["returndelivery_low_tarif"]);
}

void test_parse_return_delivery_high_tarif() {
    memset(telegram, 0, sizeof(telegram));
    strcpy(telegram, "1-0:2.8.2(000050.250*kWh)");
    int len = strlen(telegram);
    decode_telegram(len);
    TEST_ASSERT_EQUAL(50250, P1Values["returndelivery_high_tarif"]);
}

// ========================================
// * TEST SETUP AND TEARDOWN
// ========================================

void setUp(void) {
    memset(telegram, 0, sizeof(telegram));
    currentCRC = 0;
    LAST_UPDATE_SENT = 0;
}

void tearDown(void) {
}

// ========================================
// * TEST RUNNER
// ========================================

int main(int argc, char **argv) {
    UNITY_BEGIN();
    
    RUN_TEST(test_crc16_basic);
    RUN_TEST(test_find_char_in_array_rev);
    RUN_TEST(test_find_char_in_array_rev_not_found);
    RUN_TEST(test_is_number_valid);
    RUN_TEST(test_is_number_invalid);
    RUN_TEST(test_is_number_empty);
    RUN_TEST(test_parse_consumption_low_tarif);
    RUN_TEST(test_parse_consumption_high_tarif);
    RUN_TEST(test_parse_actual_consumption);
    RUN_TEST(test_parse_return_delivery_low_tarif);
    RUN_TEST(test_parse_return_delivery_high_tarif);
    RUN_TEST(test_parse_actual_return_delivery);
    RUN_TEST(test_parse_l1_instant_power);
    RUN_TEST(test_parse_l2_instant_power);
    RUN_TEST(test_parse_l3_instant_power);
    RUN_TEST(test_parse_l1_instant_current);
    RUN_TEST(test_parse_l2_instant_current);
    RUN_TEST(test_parse_l3_instant_current);
    RUN_TEST(test_parse_l1_voltage);
    RUN_TEST(test_parse_l2_voltage);
    RUN_TEST(test_parse_l3_voltage);
    RUN_TEST(test_parse_gas_meter);
    RUN_TEST(test_parse_actual_tariff);
    RUN_TEST(test_parse_short_power_outages);
    RUN_TEST(test_parse_long_power_outages);
    RUN_TEST(test_parse_short_power_drops);
    RUN_TEST(test_parse_short_power_peaks);
    
    return UNITY_END();
}

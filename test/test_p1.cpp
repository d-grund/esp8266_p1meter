#include <unity.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

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

// **********************************
// * P1 VARIABLES AND FUNCTIONS
// **********************************
char telegram[P1_MAXLINELENGTH];
// * Set during CRC checking
unsigned int currentCRC = 0;
// * Set to store the data values read
long CONSUMPTION_LOW_TARIF;
long CONSUMPTION_HIGH_TARIF;

long RETURNDELIVERY_LOW_TARIF;
long RETURNDELIVERY_HIGH_TARIF;

long ACTUAL_CONSUMPTION;
long ACTUAL_RETURNDELIVERY;
long GAS_METER_M3;

long L1_INSTANT_POWER_USAGE;
long L2_INSTANT_POWER_USAGE;
long L3_INSTANT_POWER_USAGE;
long L1_INSTANT_POWER_CURRENT;
long L2_INSTANT_POWER_CURRENT;
long L3_INSTANT_POWER_CURRENT;
long L1_VOLTAGE;
long L2_VOLTAGE;
long L3_VOLTAGE;

// Set to store data counters read
long ACTUAL_TARIF;
long SHORT_POWER_OUTAGES;
long LONG_POWER_OUTAGES;
long SHORT_POWER_DROPS;
long SHORT_POWER_PEAKS;

long LAST_UPDATE_SENT;

// P1 Functions from P1.cpp
unsigned int CRC16(unsigned int crc, unsigned char *buf, int len)
{
    for (int pos = 0; pos < len; pos++)
    {
        crc ^= (unsigned int)buf[pos];
        for (int i = 8; i != 0; i--)
        {
            if ((crc & 0x0001) != 0)
            {
                crc >>= 1;
                crc ^= 0xA001;
            }
            else
                crc >>= 1;
        }
    }
    return crc;
}

bool isNumber(char *res, int len)
{
    for (int i = 0; i < len; i++)
    {
        if (((res[i] < '0') || (res[i] > '9')) && (res[i] != '.' && res[i] != 0))
            return false;
    }
    return true;
}

int FindCharInArrayRev(char array[], char c, int len)
{
    for (int i = len - 1; i >= 0; i--)
    {
        if (array[i] == c)
            return i;
    }
    return -1;
}

long getValue(char *buffer, int maxlen, char startchar, char endchar)
{
    int s = FindCharInArrayRev(buffer, startchar, maxlen);
    int e = FindCharInArrayRev(buffer, endchar, maxlen);
    
    if (s < 0 || e < 0 || e <= s) {
        return 0;
    }
    
    int l = e - s - 1;
    
    if (l <= 0 || l >= 16) {
        return 0;
    }

    char res[16];
    memset(res, 0, sizeof(res));

    strncpy(res, buffer + s + 1, l);
    res[l] = 0;
    
    if (endchar == '*')
    {
        if (isNumber(res, l))
            return (long)(1000 * atof(res));
    }
    else if (endchar == ')')
    {
        // For values ending in ')', just parse as integer (no decimals/multiplier)
        if (isNumber(res, l))
            return (long)atoi(res);
    }
    return 0;
}

bool decode_telegram(int len)
{
    if (len <= 0 || len >= P1_MAXLINELENGTH) {
        return false;
    }
    
    int startChar = FindCharInArrayRev(telegram, '/', len);
    int endChar = FindCharInArrayRev(telegram, '!', len);
    bool validCRCFound = false;

    if (startChar >= 0)
    {
        currentCRC = CRC16(0x0000,(unsigned char *) telegram+startChar, len-startChar);
    }
    else if (endChar >= 0)
    {
        currentCRC = CRC16(currentCRC,(unsigned char*)telegram+endChar, 1);

        if (endChar + 5 < P1_MAXLINELENGTH) {
            char messageCRC[5];
            memset(messageCRC, 0, sizeof(messageCRC));
            strncpy(messageCRC, telegram + endChar + 1, 4);
            messageCRC[4] = 0;
            validCRCFound = (strtoul(messageCRC, NULL, 16) == currentCRC);
        }

        currentCRC = 0;
    }
    else
    {
        currentCRC = CRC16(currentCRC, (unsigned char*) telegram, len);
    }

    if (strncmp(telegram, "1-0:1.8.1", strlen("1-0:1.8.1")) == 0)
    {
        CONSUMPTION_LOW_TARIF = getValue(telegram, len, '(', '*');
    }

    if (strncmp(telegram, "1-0:1.8.2", strlen("1-0:1.8.2")) == 0)
    {
        CONSUMPTION_HIGH_TARIF = getValue(telegram, len, '(', '*');
    }

    if (strncmp(telegram, "1-0:2.8.1", strlen("1-0:2.8.1")) == 0)
    {
        RETURNDELIVERY_LOW_TARIF = getValue(telegram, len, '(', '*');
    }

    if (strncmp(telegram, "1-0:2.8.2", strlen("1-0:2.8.2")) == 0)
    {
        RETURNDELIVERY_HIGH_TARIF = getValue(telegram, len, '(', '*');
    }

    if (strncmp(telegram, "1-0:1.7.0", strlen("1-0:1.7.0")) == 0)
    {
        ACTUAL_CONSUMPTION = getValue(telegram, len, '(', '*');
    }

    if (strncmp(telegram, "1-0:2.7.0", strlen("1-0:2.7.0")) == 0)
    {
        ACTUAL_RETURNDELIVERY = getValue(telegram, len, '(', '*');
    }

    if (strncmp(telegram, "1-0:21.7.0", strlen("1-0:21.7.0")) == 0)
    {
        L1_INSTANT_POWER_USAGE = getValue(telegram, len, '(', '*');
    }

    if (strncmp(telegram, "1-0:41.7.0", strlen("1-0:41.7.0")) == 0)
    {
        L2_INSTANT_POWER_USAGE = getValue(telegram, len, '(', '*');
    }

    if (strncmp(telegram, "1-0:61.7.0", strlen("1-0:61.7.0")) == 0)
    {
        L3_INSTANT_POWER_USAGE = getValue(telegram, len, '(', '*');
    }

    if (strncmp(telegram, "1-0:31.7.0", strlen("1-0:31.7.0")) == 0)
    {
        L1_INSTANT_POWER_CURRENT = getValue(telegram, len, '(', '*');
    }

    if (strncmp(telegram, "1-0:51.7.0", strlen("1-0:51.7.0")) == 0)
    {
        L2_INSTANT_POWER_CURRENT = getValue(telegram, len, '(', '*');
    }

    if (strncmp(telegram, "1-0:71.7.0", strlen("1-0:71.7.0")) == 0)
    {
        L3_INSTANT_POWER_CURRENT = getValue(telegram, len, '(', '*');
    }

    if (strncmp(telegram, "1-0:32.7.0", strlen("1-0:32.7.0")) == 0)
    {
        L1_VOLTAGE = getValue(telegram, len, '(', '*');
    }

    if (strncmp(telegram, "1-0:52.7.0", strlen("1-0:52.7.0")) == 0)
    {
        L2_VOLTAGE = getValue(telegram, len, '(', '*');
    }

    if (strncmp(telegram, "1-0:72.7.0", strlen("1-0:72.7.0")) == 0)
    {
        L3_VOLTAGE = getValue(telegram, len, '(', '*');
    }

    if (strncmp(telegram, "0-1:24.2.1", strlen("0-1:24.2.1")) == 0)
    {
        GAS_METER_M3 = getValue(telegram, len, '(', '*');
    }

    if (strncmp(telegram, "0-0:96.14.0", strlen("0-0:96.14.0")) == 0)
    {
        ACTUAL_TARIF = getValue(telegram, len, '(', ')');
    }

    if (strncmp(telegram, "0-0:96.7.21", strlen("0-0:96.7.21")) == 0)
    {
        SHORT_POWER_OUTAGES = getValue(telegram, len, '(', ')');
    }

    if (strncmp(telegram, "0-0:96.7.9", strlen("0-0:96.7.9")) == 0)
    {
        LONG_POWER_OUTAGES = getValue(telegram, len, '(', ')');
    }

    if (strncmp(telegram, "1-0:32.32.0", strlen("1-0:32.32.0")) == 0)
    {
        SHORT_POWER_DROPS = getValue(telegram, len, '(', ')');
    }

    if (strncmp(telegram, "1-0:32.36.0", strlen("1-0:32.36.0")) == 0)
    {
        SHORT_POWER_PEAKS = getValue(telegram, len, '(', ')');
    }

    return validCRCFound;
}

void processLine(int len)
{
    telegram[len] = '\n';
    telegram[len + 1] = 0;
    yield();

    bool result = decode_telegram(len + 1);
    if (result) {
        send_data_to_broker();
        LAST_UPDATE_SENT = millis();
    }
}

void read_p1_hardwareserial()
{
    if (Serial.available())
    {
        memset(telegram, 0, sizeof(telegram));

        while (Serial.available())
        {
            ESP.wdtDisable();
            int len = Serial.readBytesUntil('\n', telegram, P1_MAXLINELENGTH);
            ESP.wdtEnable(1);

            processLine(len);
        }
    }
}

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
    TEST_ASSERT_EQUAL(992991, CONSUMPTION_LOW_TARIF);
}

void test_parse_consumption_high_tarif() {
    memset(telegram, 0, sizeof(telegram));
    strcpy(telegram, "1-0:1.8.2(000560.157*kWh)");
    int len = strlen(telegram);
    decode_telegram(len);
    TEST_ASSERT_EQUAL(560157, CONSUMPTION_HIGH_TARIF);
}

void test_parse_actual_consumption() {
    memset(telegram, 0, sizeof(telegram));
    strcpy(telegram, "1-0:1.7.0(00.424*kW)");
    int len = strlen(telegram);
    decode_telegram(len);
    TEST_ASSERT_EQUAL(423, ACTUAL_CONSUMPTION);
}

void test_parse_actual_return_delivery() {
    memset(telegram, 0, sizeof(telegram));
    strcpy(telegram, "1-0:2.7.0(00.000*kW)");
    int len = strlen(telegram);
    decode_telegram(len);
    TEST_ASSERT_EQUAL(0, ACTUAL_RETURNDELIVERY);
}

void test_parse_l1_instant_power() {
    memset(telegram, 0, sizeof(telegram));
    strcpy(telegram, "1-0:21.7.0(00.378*kW)");
    int len = strlen(telegram);
    decode_telegram(len);
    TEST_ASSERT_EQUAL(378, L1_INSTANT_POWER_USAGE);
}

void test_parse_l2_instant_power() {
    memset(telegram, 0, sizeof(telegram));
    strcpy(telegram, "1-0:41.7.0(00.500*kW)");
    int len = strlen(telegram);
    decode_telegram(len);
    TEST_ASSERT_EQUAL(500, L2_INSTANT_POWER_USAGE);
}

void test_parse_l3_instant_power() {
    memset(telegram, 0, sizeof(telegram));
    strcpy(telegram, "1-0:61.7.0(00.625*kW)");
    int len = strlen(telegram);
    decode_telegram(len);
    TEST_ASSERT_EQUAL(625, L3_INSTANT_POWER_USAGE);
}

void test_parse_l1_instant_current() {
    memset(telegram, 0, sizeof(telegram));
    strcpy(telegram, "1-0:31.7.0(002*A)");
    int len = strlen(telegram);
    decode_telegram(len);
    TEST_ASSERT_EQUAL(2000, L1_INSTANT_POWER_CURRENT);
}

void test_parse_l2_instant_current() {
    memset(telegram, 0, sizeof(telegram));
    strcpy(telegram, "1-0:51.7.0(003*A)");
    int len = strlen(telegram);
    decode_telegram(len);
    TEST_ASSERT_EQUAL(3000, L2_INSTANT_POWER_CURRENT);
}

void test_parse_l3_instant_current() {
    memset(telegram, 0, sizeof(telegram));
    strcpy(telegram, "1-0:71.7.0(001*A)");
    int len = strlen(telegram);
    decode_telegram(len);
    TEST_ASSERT_EQUAL(1000, L3_INSTANT_POWER_CURRENT);
}

void test_parse_l1_voltage() {
    memset(telegram, 0, sizeof(telegram));
    strcpy(telegram, "1-0:32.7.0(232.0*V)");
    int len = strlen(telegram);
    decode_telegram(len);
    TEST_ASSERT_EQUAL(232000, L1_VOLTAGE);
}

void test_parse_l2_voltage() {
    memset(telegram, 0, sizeof(telegram));
    strcpy(telegram, "1-0:52.7.0(232.5*V)");
    int len = strlen(telegram);
    decode_telegram(len);
    TEST_ASSERT_EQUAL(232500, L2_VOLTAGE);
}

void test_parse_l3_voltage() {
    memset(telegram, 0, sizeof(telegram));
    strcpy(telegram, "1-0:72.7.0(233.0*V)");
    int len = strlen(telegram);
    decode_telegram(len);
    TEST_ASSERT_EQUAL(233000, L3_VOLTAGE);
}

void test_parse_gas_meter() {
    memset(telegram, 0, sizeof(telegram));
    strcpy(telegram, "0-1:24.2.1(150531200000S)(00811.923*m3)");
    int len = strlen(telegram);
    decode_telegram(len);
    TEST_ASSERT_EQUAL(811923, GAS_METER_M3);
}

void test_parse_actual_tariff() {
    memset(telegram, 0, sizeof(telegram));
    strcpy(telegram, "0-0:96.14.0(0001)");
    int len = strlen(telegram);
    decode_telegram(len);
    TEST_ASSERT_EQUAL(1, ACTUAL_TARIF);
}

void test_parse_short_power_outages() {
    memset(telegram, 0, sizeof(telegram));
    strcpy(telegram, "0-0:96.7.21(00003)");
    int len = strlen(telegram);
    SHORT_POWER_OUTAGES = 999;  // Set to known value before parsing
    decode_telegram(len);
    TEST_ASSERT_EQUAL(3, SHORT_POWER_OUTAGES);
}

void test_parse_long_power_outages() {
    memset(telegram, 0, sizeof(telegram));
    strcpy(telegram, "0-0:96.7.9(00001)");
    int len = strlen(telegram);
    LONG_POWER_OUTAGES = 999;  // Set to known value before parsing
    decode_telegram(len);
    TEST_ASSERT_EQUAL(1, LONG_POWER_OUTAGES);
}

void test_parse_short_power_drops() {
    memset(telegram, 0, sizeof(telegram));
    strcpy(telegram, "1-0:32.32.0(00000)");
    int len = strlen(telegram);
    SHORT_POWER_DROPS = 999;  // Set to known value before parsing
    decode_telegram(len);
    TEST_ASSERT_EQUAL(0, SHORT_POWER_DROPS);
}

void test_parse_short_power_peaks() {
    memset(telegram, 0, sizeof(telegram));
    strcpy(telegram, "1-0:32.36.0(00005)");
    int len = strlen(telegram);
    SHORT_POWER_PEAKS = 999;  // Set to known value before parsing
    decode_telegram(len);
    TEST_ASSERT_EQUAL(5, SHORT_POWER_PEAKS);
}

void test_parse_return_delivery_low_tarif() {
    memset(telegram, 0, sizeof(telegram));
    strcpy(telegram, "1-0:2.8.1(000100.500*kWh)");
    int len = strlen(telegram);
    decode_telegram(len);
    TEST_ASSERT_EQUAL(100500, RETURNDELIVERY_LOW_TARIF);
}

void test_parse_return_delivery_high_tarif() {
    memset(telegram, 0, sizeof(telegram));
    strcpy(telegram, "1-0:2.8.2(000050.250*kWh)");
    int len = strlen(telegram);
    decode_telegram(len);
    TEST_ASSERT_EQUAL(50250, RETURNDELIVERY_HIGH_TARIF);
}

void test_parse_complete_telegram() {
    // Complete real DSMR telegram from the meter
    const char full_telegram[] = 
        "/XMX5LGBBFG1012463817\n"
        "1-3:0.2.8(42)\n"
        "0-0:1.0.0(180624024002S)\n"
        "0-0:96.1.1(4530303331303033323632373634333136)\n"
        "1-0:1.8.1(002200.945*kWh)\n"
        "1-0:1.8.2(001961.604*kWh)\n"
        "1-0:2.8.1(000000.000*kWh)\n"
        "1-0:2.8.2(000000.000*kWh)\n"
        "0-0:96.14.0(0001)\n"
        "1-0:1.7.0(00.378*kW)\n"
        "1-0:2.7.0(00.000*kW)\n"
        "0-0:96.7.21(00003)\n"
        "0-0:96.7.9(00001)\n"
        "1-0:99.97.0(1)(0-0:96.7.19)(170214081346W)(0000006334*s)\n"
        "1-0:32.32.0(00000)\n"
        "1-0:32.36.0(00000)\n"
        "0-0:96.13.1()\n"
        "0-0:96.13.0()\n"
        "1-0:31.7.0(002*A)\n"
        "1-0:21.7.0(00.378*kW)\n"
        "1-0:22.7.0(00.000*kW)\n"
        "0-1:24.1.0(003)\n"
        "0-1:96.1.0(4730303235303033333630383535373136)\n"
        "0-1:24.2.1(180624020000S)(00968.481*m3)\n"
        "!8711";
    
    // Reset all variables to known state
    setUp();
    
    // Parse each line of the telegram
    int pos = 0;
    while (pos < strlen(full_telegram)) {
        // Find the next newline
        int line_start = pos;
        int line_end = line_start;
        while (line_end < strlen(full_telegram) && full_telegram[line_end] != '\n') {
            line_end++;
        }
        
        // Extract line without newline
        int line_len = line_end - line_start;
        memset(telegram, 0, sizeof(telegram));
        strncpy(telegram, full_telegram + line_start, line_len);
        telegram[line_len] = 0;
        
        // Process the line
        decode_telegram(line_len);
        
        pos = line_end + 1;
    }
    
    // Verify parsed values from the complete telegram
    TEST_ASSERT_EQUAL(2200945, CONSUMPTION_LOW_TARIF);      // 1-0:1.8.1(002200.945*kWh)
    TEST_ASSERT_EQUAL(1961604, CONSUMPTION_HIGH_TARIF);     // 1-0:1.8.2(001961.604*kWh)
    TEST_ASSERT_EQUAL(0, RETURNDELIVERY_LOW_TARIF);         // 1-0:2.8.1(000000.000*kWh)
    TEST_ASSERT_EQUAL(0, RETURNDELIVERY_HIGH_TARIF);        // 1-0:2.8.2(000000.000*kWh)
    TEST_ASSERT_EQUAL(1, ACTUAL_TARIF);                     // 0-0:96.14.0(0001)
    TEST_ASSERT_EQUAL(378, ACTUAL_CONSUMPTION);             // 1-0:1.7.0(00.378*kW)
    TEST_ASSERT_EQUAL(0, ACTUAL_RETURNDELIVERY);            // 1-0:2.7.0(00.000*kW)
    TEST_ASSERT_EQUAL(3, SHORT_POWER_OUTAGES);              // 0-0:96.7.21(00003)
    TEST_ASSERT_EQUAL(1, LONG_POWER_OUTAGES);               // 0-0:96.7.9(00001)
    TEST_ASSERT_EQUAL(0, SHORT_POWER_DROPS);                // 1-0:32.32.0(00000)
    TEST_ASSERT_EQUAL(0, SHORT_POWER_PEAKS);                // 1-0:32.36.0(00000)
    TEST_ASSERT_EQUAL(2000, L1_INSTANT_POWER_CURRENT);      // 1-0:31.7.0(002*A) -> 2*1000
    TEST_ASSERT_EQUAL(378, L1_INSTANT_POWER_USAGE);         // 1-0:21.7.0(00.378*kW)
    TEST_ASSERT_EQUAL(968480, GAS_METER_M3);                // 0-1:24.2.1(180624020000S)(00968.481*m3) -> 968480 (rounding)
}

// ========================================
// * TEST SETUP AND TEARDOWN
// ========================================

void setUp(void) {
    memset(telegram, 0, sizeof(telegram));
    currentCRC = 0;
    CONSUMPTION_LOW_TARIF = 0;
    CONSUMPTION_HIGH_TARIF = 0;
    RETURNDELIVERY_LOW_TARIF = 0;
    RETURNDELIVERY_HIGH_TARIF = 0;
    ACTUAL_CONSUMPTION = 0;
    ACTUAL_RETURNDELIVERY = 0;
    GAS_METER_M3 = 0;
    L1_INSTANT_POWER_USAGE = 0;
    L2_INSTANT_POWER_USAGE = 0;
    L3_INSTANT_POWER_USAGE = 0;
    L1_INSTANT_POWER_CURRENT = 0;
    L2_INSTANT_POWER_CURRENT = 0;
    L3_INSTANT_POWER_CURRENT = 0;
    L1_VOLTAGE = 0;
    L2_VOLTAGE = 0;
    L3_VOLTAGE = 0;
    ACTUAL_TARIF = 0;
    SHORT_POWER_OUTAGES = 0;
    LONG_POWER_OUTAGES = 0;
    SHORT_POWER_DROPS = 0;
    SHORT_POWER_PEAKS = 0;
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
    RUN_TEST(test_parse_complete_telegram);
    
    return UNITY_END();
}

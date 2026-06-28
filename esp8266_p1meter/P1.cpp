#define P1
#include "settings.h"

// **********************************
// * P1                             *
// **********************************
char telegram[P1_MAXLINELENGTH];
// * Set during CRC checking
unsigned int currentCRC = 0;
//count number of telegrams read
int telegramRead=0;
// * Set to store the data values read
std::map<std::string,long> P1Values = {
   {"consumption_low_tarif", 0},
   {"consumption_high_tarif", 0},
  
   {"returndelivery_low_tarif", 0},
   {"returndelivery_high_tarif", 0},
  
   {"actual_consumption", 0},
   {"actual_returndelivery", 0},
   {"actual_tarif_group", 0},
 
   {"l1_instant_power_usage", 0},
   {"l2_instant_power_usage", 0},
   {"l3_instant_power_usage", 0},
   {"l1_instant_power_current", 0},
   {"l2_instant_power_current", 0},
   {"l3_instant_power_current", 0},
 
   {"l1_voltage", 0},
   {"l2_voltage", 0},
   {"l3_voltage", 0},
    
   {"gas_meter_m3", 0},

   {"short_power_outages", 0},
   {"long_power_outages", 0},
   {"short_power_drops", 0},
   {"short_power_peaks", 0}
};

unsigned int CRC16(unsigned int crc, unsigned char *buf, int len)
{
	for (int pos = 0; pos < len; pos++)
    {
		crc ^= (unsigned int)buf[pos];    // * XOR byte into least sig. byte of crc
                                          // * Loop over each bit
        for (int i = 8; i != 0; i--)
        {
            // * If the LSB is set
            if ((crc & 0x0001) != 0)
            {
                // * Shift right and XOR 0xA001
                crc >>= 1;
				crc ^= 0xA001;
			}
            // * Else LSB is not set
            else
                // * Just shift right
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
    // Find start and end delimiters with full buffer search
    // (prevents missing delimiters near buffer end)
    int s = FindCharInArrayRev(buffer, startchar, maxlen);
    int e = FindCharInArrayRev(buffer, endchar, maxlen);
    
    // Validate delimiters found and in correct order
    if (s < 0 || e < 0 || e <= s) {
        return 0;
    }
    
    int l = e - s - 1;
    
    // Validate extracted string length
    if (l <= 0 || l >= 16) {
        return 0;
    }

    char res[16];
    memset(res, 0, sizeof(res));

    strncpy(res, buffer + s + 1, l);
    res[l] = 0;
    
    if (endchar == '*')
    {
        // Values with units (decimal): multiply by 1000 for integer precision
        if (isNumber(res, l))
            return (long)(1000 * atof(res));
    }
    else if (endchar == ')')
    {
        // Integer-only values: parse directly without float conversion
        if (isNumber(res, l))
            return (long)atoi(res);
    }
    return 0;
}

bool decode_telegram(int len)
{
    // Validate input length to prevent buffer overruns
    if (len <= 0 || len >= P1_MAXLINELENGTH) {
        return false;
    }
    
    int startChar = FindCharInArrayRev(telegram, '/', len);
    int endChar = FindCharInArrayRev(telegram, '!', len);
    bool validCRCFound = false;

    if (startChar >= 0)
    {
        // Start of telegram: initialize CRC calculation
        currentCRC = CRC16(0x0000,(unsigned char *) telegram+startChar, len-startChar);
    }
    else if (endChar >= 0)
    {
        // End of telegram: finalize CRC and validate
        currentCRC = CRC16(currentCRC,(unsigned char*)telegram+endChar, 1);

        // Check buffer bounds before reading CRC
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
        // Middle of telegram: continue CRC calculation
        currentCRC = CRC16(currentCRC, (unsigned char*) telegram, len);
    }

    if (strncmp(telegram, "1-0:1.8.1", strlen("1-0:1.8.1")) == 0)
    {
        P1Values["consumption_low_tarif"] = getValue(telegram, len, '(', '*');
    }

    if (strncmp(telegram, "1-0:1.8.2", strlen("1-0:1.8.2")) == 0)
    {
        P1Values["consumption_high_tarif"] = getValue(telegram, len, '(', '*');
    }

    if (strncmp(telegram, "1-0:2.8.1", strlen("1-0:2.8.1")) == 0)
    {
       P1Values["returndelivery_low_tarif"] = getValue(telegram, len, '(', '*');
    }

    if (strncmp(telegram, "1-0:2.8.2", strlen("1-0:2.8.2")) == 0)
    {
        P1Values["returndelivery_high_tarif"] = getValue(telegram, len, '(', '*');
    }

    if (strncmp(telegram, "1-0:1.7.0", strlen("1-0:1.7.0")) == 0)
    {
        P1Values["actual_consumption"] = getValue(telegram, len, '(', '*');
    }

    if (strncmp(telegram, "1-0:2.7.0", strlen("1-0:2.7.0")) == 0)
    {
        P1Values["actual_returndelivery"] = getValue(telegram, len, '(', '*');
    }

    if (strncmp(telegram, "1-0:21.7.0", strlen("1-0:21.7.0")) == 0)
    {
        P1Values["l1_instant_power_usage"] = getValue(telegram, len, '(', '*');
    }

    if (strncmp(telegram, "1-0:41.7.0", strlen("1-0:41.7.0")) == 0)
    {
        P1Values["l2_instant_power_usage"] = getValue(telegram, len, '(', '*');
    }

    if (strncmp(telegram, "1-0:61.7.0", strlen("1-0:61.7.0")) == 0)
    {
        P1Values["l3_instant_power_usage"] = getValue(telegram, len, '(', '*');
    }

    if (strncmp(telegram, "1-0:31.7.0", strlen("1-0:31.7.0")) == 0)
    {
        P1Values["l1_instant_power_current"] = getValue(telegram, len, '(', '*');
    }

    if (strncmp(telegram, "1-0:51.7.0", strlen("1-0:51.7.0")) == 0)
    {
        P1Values["l2_instant_power_current"] = getValue(telegram, len, '(', '*');
    }

    if (strncmp(telegram, "1-0:71.7.0", strlen("1-0:71.7.0")) == 0)
    {
        P1Values["l3_instant_power_current"] = getValue(telegram, len, '(', '*');
    }

    if (strncmp(telegram, "1-0:32.7.0", strlen("1-0:32.7.0")) == 0)
    {
        P1Values["l1_voltage"] = getValue(telegram, len, '(', '*');
    }

    if (strncmp(telegram, "1-0:52.7.0", strlen("1-0:52.7.0")) == 0)
    {
        P1Values["l2_voltage"] = getValue(telegram, len, '(', '*');
    }

    if (strncmp(telegram, "1-0:72.7.0", strlen("1-0:72.7.0")) == 0)
    {
        P1Values["l3_voltage"] = getValue(telegram, len, '(', '*');
    }

    if (strncmp(telegram, "0-1:24.2.1", strlen("0-1:24.2.1")) == 0)
    {
        P1Values["gas_meter_m3"] = getValue(telegram, len, '(', '*');
    }

    if (strncmp(telegram, "0-0:96.14.0", strlen("0-0:96.14.0")) == 0)
    {
        P1Values["actual_tarif"] = getValue(telegram, len, '(', ')');
    }

    if (strncmp(telegram, "0-0:96.7.21", strlen("0-0:96.7.21")) == 0)
    {
        P1Values["short_power_outages"] = getValue(telegram, len, '(', ')');
    }

    if (strncmp(telegram, "0-0:96.7.9", strlen("0-0:96.7.9")) == 0)
    {
        P1Values["long_power_outages"] = getValue(telegram, len, '(', ')');
    }

    if (strncmp(telegram, "1-0:32.32.0", strlen("1-0:32.32.0")) == 0)
    {
        P1Values["short_power_drops"] = getValue(telegram, len, '(', ')');
    }

    if (strncmp(telegram, "1-0:32.36.0", strlen("1-0:32.36.0")) == 0)
    {
        P1Values["short_power_peaks"] = getValue(telegram, len, '(', ')');
    }

    return validCRCFound;
}

void processLine(int len, long now) 
{
    telegram[len] = '\n';
    telegram[len + 1] = 0;
    yield();

    bool result = decode_telegram(len + 1);
    if (result && (now - LAST_UPDATE_SENT >= UPDATE_INTERVAL)) {
        send_data_to_broker();
        LAST_UPDATE_SENT = millis();
    }
}

void read_p1_hardwareserial(long now)
{
    if (Serial.available())
    {
        memset(telegram, 0, sizeof(telegram));
        telegramRead++;  // Increment telegram read count

        while (Serial.available())
        {
            ESP.wdtDisable();
            int len = Serial.readBytesUntil('\n', telegram, P1_MAXLINELENGTH);
            ESP.wdtEnable(1);

            processLine(len, now);
        }
    }
}

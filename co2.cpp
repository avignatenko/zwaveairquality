
#include "co2.h"

#include "HardwareSerial.h"

word s_co2 = 400; // ppm

HardwareSerial &s_co2_serial = Serial0;

word getCO2()
{
    return s_co2;
}

void setupCO2()
{
    s_co2_serial.begin(9600);
}

char getCheckSum(uint8_t *packet)
{
    uint8_t checksum = 0;

    for (uint8_t i = 1; i < 8; i++)
        checksum += packet[i];

    checksum = 0xff - checksum;
    checksum = checksum + 1;
    return checksum;
}

void updateCO2(bool firstTime)
{
#if SERIAL_LOGS
    Serial.print("CO2 ");
    Serial.print(s_co2);
    Serial.print(" ");
    Serial.println();
#endif

    // send data request
    uint8_t bufferOut[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
    s_co2_serial.write(bufferOut, 9);

    // read co2 concentraction reply
    uint8_t bufferIn[9];
    byte read = s_co2_serial.readBytes(bufferIn, 9);
    if (read < 9)
    {
#if SERIAL_LOGS
        Serial.println("No answer");
#endif
    }
    else // parse answer
    {
        bool ok = true;
        ok = ok && (bufferIn[0] == 0xFF);
        ok = ok && (bufferIn[1] == 0x86);

#if SERIAL_LOGS
        Serial.print("Ok id: ");
        Serial.println(ok);
#endif

        byte c1 = bufferIn[2];
        byte c2 = bufferIn[3];

        word value = c1 * 256 + c2;

        ok = ok && (getCheckSum(bufferIn) == bufferIn[8]);

#if SERIAL_LOGS
        Serial.print("Ok checksum: ");
        Serial.println(ok);
#endif

        if (ok)
        {
            s_co2 = value;
        }
        else
        {
            s_co2 = 0;
#if SERIAL_LOGS
            Serial.println("Wrong answer");
#endif
        }
    }
}

bool reportCO2Updates(bool firstTime)
{
}
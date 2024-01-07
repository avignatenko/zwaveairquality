
#include "co2.h"

#include "HardwareSerial.h"

const byte co2_hd_pin = 6;

const int CALIBRATION_TIME = 8 * 1000; // 8 seconds
bool s_inCalibration = false;
dword s_calibrationStartedTime = 0;

word s_co2 = 0; // ppm

word s_co2LastReported = 0;
word s_co2_interval = 60 * 20; // 20 mins default, min 30 seconds
word s_co2_threshold = 50;
unsigned long s_lastReportedTimeCO2 = 0;

HardwareSerial &s_co2_serial = Serial0;

word getCO2()
{
    return s_co2;
}

void setupCO2()
{
    s_co2_serial.begin(9600);
    // set hd pin
    pinMode(co2_hd_pin, HIGH);
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

void triggerCalibration()
{

    if (s_inCalibration)
        return;

#if SERIAL_LOGS
    Serial.println("Trigger calibration");
#endif

    // set hd pin to LOW and start timer
    pinMode(co2_hd_pin, LOW);

    s_calibrationStartedTime = millis();

    s_inCalibration = true;
}

void updateCalibration()
{
    if (!s_inCalibration)
        return;

    unsigned long delta = millis() - s_calibrationStartedTime;

#if SERIAL_LOGS
    Serial.print("Calibration update: ");
    Serial.println(delta);
#endif

    if (delta > CALIBRATION_TIME)
    {
        s_inCalibration = false;
        s_calibrationStartedTime = 0;
    }
}

void updateCO2(bool firstTime)
{
    updateCalibration();

    // read everything which might stay there
    while (s_co2_serial.available())
        s_co2_serial.read();

    // send data request
    uint8_t bufferOut[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
    s_co2_serial.write(bufferOut, 9);

    // read co2 concentraction reply
    uint8_t bufferIn[9];
    byte read = s_co2_serial.readBytes(bufferIn, 9);
    if (read != 9)
    {
#if SERIAL_LOGS
        Serial.println("No answer");
#endif
        s_co2 = 0;

        // read everything else
        while (s_co2_serial.available())
            s_co2_serial.read();

        return;
    }

    // parse answer

    bool ok = true;
    ok = ok && (bufferIn[0] == 0xFF);
    ok = ok && (bufferIn[1] == 0x86);

#if SERIAL_LOGS
    Serial.print("Ok id: ");
    Serial.println(ok);
#endif
    if (!ok)
    {
        s_co2 = 1;
        return;
    }

    byte c1 = bufferIn[2];
    byte c2 = bufferIn[3];

    word value = (word)c1 * 256 + (word)c2;

    ok = ok && (getCheckSum(bufferIn) == bufferIn[8]);

#if SERIAL_LOGS
    Serial.print("Bytes: ");
    Serial.print(bufferIn[4]);
    Serial.print(" ");
    Serial.print(bufferIn[5]);
    Serial.print(" ");
    Serial.print(bufferIn[6]);
    Serial.print(" ");
    Serial.println(bufferIn[7]);

    Serial.print("Ok checksum: ");
    Serial.println(ok);
#endif

    if (!ok)
    {
#if SERIAL_LOGS
        Serial.println("Wrong answer");
#endif

        s_co2 = 2;
        return;
    }

    s_co2 = value;
}

bool reportCO2Updates(bool firstTime)
{
#if SERIAL_LOGS
    Serial.print("CO2 ");
    Serial.print(s_co2);
    Serial.print(" ");
    Serial.println();
#endif

    if (s_co2 < 100)
        return false; // wrong value, let's skip it

    unsigned long curMillis = millis();

    bool reportCO2 = (abs(s_co2 - s_co2LastReported) > s_co2_threshold);
    bool timePassedCO2 = (curMillis - s_lastReportedTimeCO2 > (unsigned long)s_co2_interval * 1000);

    if (firstTime || reportCO2 || timePassedCO2)
    {
        zunoSendReport(CHANNEL_CO2);
        s_co2LastReported = s_co2;
        s_lastReportedTimeCO2 = curMillis;

#if SERIAL_LOGS
        Serial.print("CO2 update sent, because: ");
        Serial.print(reportCO2);
        Serial.print(" ");
        Serial.print(timePassedCO2);
        Serial.println();
#endif

        return true;
    }

    return false;
}

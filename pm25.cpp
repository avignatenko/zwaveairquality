#include "pm25.h"

PM25Task::PM25Task(SerialEx& serial, uint8_t reportChannel1, uint8_t reportChannel2, uint8_t reportChannel3)
    : Task(2000),
      serial_(serial),
      reportChannel1_(reportChannel1),
      reportChannel2_(reportChannel2),
      reportChannel3_(reportChannel3)
{
}

unsigned char FucCheckSum(unsigned char* i, unsigned char ln)
{
    unsigned char j, tempq = 0;
    i += 1;
    for (j = 0; j < (ln - 2); j++)
    {
        tempq += *i;
        i++;
    }
    tempq = (~tempq) + 1;
    return (tempq);
}

void PM25Task::requestData()
{
    HardwareSerial& serial = serial_.captureSerial();

    // clear everything which stays in the buffer, be prepared to correct answer
    while (serial.available()) serial.read();

    byte buffer[9] = {0xff, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
    for (int i = 0; i < 9; ++i) serial.write(buffer[i]);
}

PM25Task::Reply PM25Task::receiveData()
{
    HardwareSerial& serial = serial_.captureSerial();

    uint8_t buffer[9];
    int read = serial.readBytes(buffer, 9);
    if (read == 0) return REPLY_NO_ANSWER;

    if (read != 9)
    {
        // read everything else
        while (serial.available()) serial.read();

        return REPLY_WRONG_LENGTH;
    }

#if SERIAL_LOGS
    Serial.print("PM2.5: Read bytes: ");

    for (int i = 0; i < read; ++i)
    {
        Serial.print(buffer[i]);
        Serial.print(" ");
    }
    Serial.println();
#endif

    bool ok = true;
    ok = ok && (buffer[0] == 0xFF);
    ok = ok && (buffer[1] == 0x86);
    if (!ok) return REPLY_WRONG_ID;

#if SERIAL_LOGS
    Serial.print("PM2.5: Check ");
    Serial.println(buffer[8]);
#endif

    ok = ok && (FucCheckSum(buffer, 9) == buffer[8]);

    if (!ok) return REPLY_WRONG_CHECKSUM;

    pm2d5_ = buffer[2] * 256 + buffer[3];
    pm10_ = buffer[4] * 256 + buffer[5];
    pm1d0_ = buffer[6] * 256 + buffer[7];

#if SERIAL_LOGS
    Serial.print("PM2.5: PM2.5 ");
    Serial.print(pm2d5_);
    Serial.print(" PM10: ");
    Serial.print(pm10_);
    Serial.print(" PM1.0: ");
    Serial.println(pm1d0_);
#endif

    // read everything else just in case
    while (serial.available()) serial.read();

    return REPLY_OK;
}

void PM25Task::setQAMode()
{
    HardwareSerial& serial = serial_.captureSerial();

    byte buffer[9] = {0xff, 0x01, 0x78, 0x41, 0x00, 0x00, 0x00, 0x00, 0x46};
    for (int i = 0; i < 9; ++i) serial.write(buffer[i]);
}

void PM25Task::setup()
{
#if SERIAL_LOGS
    Serial.println("PM2.5: setup start: ");
#endif

    serial_.begin(9600);

    setQAMode();

#if SERIAL_LOGS
    Serial.println("PM2.5: setup end");
#endif

    updateInternal(true);
}

uint16_t PM25Task::getPM2d5()
{
    return pm2d5_;
}

uint16_t PM25Task::getPM10()
{
    return pm10_;
}

uint16_t PM25Task::getPM1d0()
{
    return pm1d0_;
}

void PM25Task::update()
{
    updateInternal();
}

void PM25Task::updateInternal(bool firstTime)
{
    update(firstTime);
    reportUpdates(firstTime);
}

void PM25Task::update(bool firstTime)
{
    requestData();
    Reply reply = receiveData();

    if (reply != REPLY_OK)
    {
#if SERIAL_LOGS
        Serial.print("PM2.5: Error reading PM2.5: ");
        Serial.print(reply);
        Serial.println();
#endif
        pm2d5_ = (uint16_t)~0 - (uint16_t)reply;
        return;
    }
}

bool PM25Task::reportUpdates(bool firstTime)
{
    if (pm2d5_ <= -1) return false;  // wrong value, let's skip it

    unsigned long curMillis = millis();

    bool reportPM2d5 = (abs(pm2d5_ - pm2d5LastReported_) > pm2d5Threshold_);
    bool timePassedPM2d5 = (curMillis - lastReportedTimePM2d5_ > (unsigned long)pm2d5Interval_ * 1000);

    if (firstTime || reportPM2d5 || timePassedPM2d5)
    {
        zunoSendReport(reportChannel1_);
        zunoSendReport(reportChannel2_);
        zunoSendReport(reportChannel3_);
        pm2d5LastReported_ = pm2d5_;
        lastReportedTimePM2d5_ = curMillis;

#if SERIAL_LOGS
        Serial.print("PM2.5: update sent, because: ");
        Serial.print(reportPM2d5);
        Serial.print(" ");
        Serial.print(timePassedPM2d5);
        Serial.println();
#endif

        return true;
    }

    return false;
}

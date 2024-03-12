#include "co.h"

COTask::COTask(SerialEx& serial) : Task(2000), serial_(serial) {}

unsigned char FucCheckSumCO(unsigned char* i, unsigned char ln)
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

void COTask::requestData()
{
    HardwareSerial& serial = serial_.captureSerial();

    // clear everything which stays in the buffer, be prepared to correct answer
    while (serial.available()) serial.read();

    byte buffer[9] = {0xff, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
    for (int i = 0; i < 9; ++i) serial.write(buffer[i]);
}

COTask::Reply COTask::receiveData()
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
    Serial.print("CO: Read bytes: ");

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
    Serial.print("CO: Check ");
    Serial.println(buffer[8]);
#endif

    ok = ok && (FucCheckSumCO(buffer, 9) == buffer[8]);

    if (!ok) return REPLY_WRONG_CHECKSUM;

    uint8_t coHigh = buffer[2];
    uint8_t coLow = buffer[3];

    // check sensor failure
    bool failed = (coHigh >> 7);
    if (failed)
    {
#if SERIAL_LOGS
        Serial.print("CO: ");
        Serial.println("sensor failed");
#endif
        return REPLY_SENSOR_FAILED;
    }

    co_ = (coHigh & 0x1F) * 256 + coLow;

#if SERIAL_LOGS
    Serial.print("CO: x10 ");
    Serial.println(co_);
#endif

    // read everything else just in case
    while (serial.available()) serial.read();

    return REPLY_OK;
}

void COTask::setQAMode()
{
    HardwareSerial& serial = serial_.captureSerial();

    byte buffer[9] = {0xff, 0x01, 0x78, 0x41, 0x00, 0x00, 0x00, 0x00, 0x46};
    for (int i = 0; i < 9; ++i) serial.write(buffer[i]);
}

void COTask::setup()
{
#if SERIAL_LOGS
    Serial.println("CO: setup start: ");
#endif

    serial_.begin(9600);

    setQAMode();

#if SERIAL_LOGS
    Serial.println("CO: setup end");
#endif
}

uint16_t COTask::getCO()
{
    return co_;
}

void COTask::update()
{
    requestData();
    Reply reply = receiveData();

    if (reply != REPLY_OK)
    {
#if SERIAL_LOGS
        Serial.print("CO: Error reading CO: ");
        Serial.print(reply);
        Serial.println();
#endif
        co_ = (uint16_t)~0 - (uint16_t)reply;
        return;
    }
}
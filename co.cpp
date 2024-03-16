#include "co.h"

COTask::COTask(SerialEx& serial) : Task(5000), serial_(serial) {}

void COTask::requestData()
{
    sendCommand(serial_, WINSEN_REQUEST_DATA_COMMAND);
}

WinsenReply COTask::receiveData()
{
    uint8_t buffer[6];
    WinsenReply reply = readReply(serial_, WINSEN_REQUEST_DATA_COMMAND, buffer);
    if (reply != REPLY_OK) return reply;

    uint8_t coHigh = buffer[0];
    uint8_t coLow = buffer[1];

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

    return REPLY_OK;
}

void COTask::setQAMode()
{
    sendCommand(serial_, WINSEN_SET_QAMODE_COMMAND, WINSEN_SET_QAMODE_COMMAND_ARG);
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
    WinsenReply reply = receiveData();

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
#include "pm25.h"

PM25Task::PM25Task(SerialEx& serial, uint8_t reportChannel1, uint8_t reportChannel2, uint8_t reportChannel3)
    : Task(5000),
      serial_(serial),
      reportChannel1_(reportChannel1),
      reportChannel2_(reportChannel2),
      reportChannel3_(reportChannel3)
{
}

void PM25Task::requestData()
{
    sendCommand(serial_, WINSEN_REQUEST_DATA_COMMAND);
}

WinsenReply PM25Task::receiveData()
{
    uint8_t buffer[6];
    WinsenReply reply = readReply(serial_, WINSEN_REQUEST_DATA_COMMAND, buffer);
    if (reply != REPLY_OK) return reply;

    pm2d5_ = buffer[0] * 256 + buffer[1];
    pm10_ = buffer[2] * 256 + buffer[3];
    pm1d0_ = buffer[4] * 256 + buffer[5];

#if SERIAL_LOGS
    Serial.print("PM2.5: PM2.5 ");
    Serial.print(pm2d5_);
    Serial.print(" PM10: ");
    Serial.print(pm10_);
    Serial.print(" PM1.0: ");
    Serial.println(pm1d0_);
#endif

    return REPLY_OK;
}

void PM25Task::setQAMode()
{
    sendCommand(serial_, WINSEN_SET_QAMODE_COMMAND, WINSEN_SET_QAMODE_COMMAND_ARG);
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
    WinsenReply reply = receiveData();

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

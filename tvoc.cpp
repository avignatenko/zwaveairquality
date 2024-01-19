#include "tvoc.h"

TVOCTask::TVOCTask(uint8_t pin) : Task(2000), pin_(pin) {}

void TVOCTask::setup()
{
    pinMode(pin_, INPUT);

    updateInternal(true);
}

byte TVOCTask::get()
{
    return tvocLevel_;
}

byte TVOCTask::getPercent()
{
    return get() * 10;  // map [0 .. 10 ] -> [0 .. 100]
}

void TVOCTask::updateTVOC(bool firstTime)
{
    // wait for the next HIGH pulse
    DWORD duration = pulseIn(pin_, HIGH, 200000);  // wait for 200 ms
    if (duration == 0)                             // no changes, just read the pin
    {
        byte level = digitalRead(pin_);
        tvocLevel_ = (level > 0 ? 10 : 0);
        return;
    }

    tvocLevel_ = (duration / 1000 + 10) / 10;

#if SERIAL_LOGS
    Serial.print("TVOC: raw ");
    Serial.println(tvocLevel_);
#endif

    if (tvocLevel_ > 10) tvocLevel_ = 10;
}

bool TVOCTask::reportTVOCUpdates(bool firstTime)
{
    unsigned long curMillis = millis();

    bool reportTVOC = (abs(tvocLevel_ - tvocLastReported_) > tvocThreshold_);
    bool timePassedTVOC = (curMillis - lastReportedTimeTVOC_ > (unsigned long)tvocInterval_ * 1000);

    if (firstTime || reportTVOC || timePassedTVOC)
    {
        zunoSendReport(CHANNEL_TVOC);
        tvocLastReported_ = tvocLevel_;
        lastReportedTimeTVOC_ = curMillis;

#if SERIAL_LOGS
        Serial.print("TVOC: update sent, because: ");
        Serial.print(reportTVOC);
        Serial.print(" ");
        Serial.print(timePassedTVOC);
        Serial.println();
#endif

        return true;
    }

    return false;
}

void TVOCTask::updateInternal(bool firstTime)
{
    updateTVOC(firstTime);
    reportTVOCUpdates(firstTime);
}

void TVOCTask::update()
{
    updateInternal();
}

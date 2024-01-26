#pragma once

#include "common.h"
#include "tasks.h"

#include "serialex.h"

class CO2Task : public Task
{
public:
    CO2Task(SerialEx& serial, uint8_t pinHd, uint8_t reportChannel, uint8_t configChannel);

    // returns CO2 as PPM value
    // value <= 100 means error (or still in pre-heat)
    word getCO2();

    void setup();

    void triggerCalibration();
    void enableAutoCalibration(bool enable);

    void updateFromCFGParams();
protected:
    void update() override;

private:
    bool updateCalibration();
    bool updatePreheat();
    void sendCommand(uint8_t command, uint8_t arg = 0x00);

    enum Reply
    {
        REPLY_OK = 0,
        REPLY_NO_ANSWER,
        REPLY_WRONG_LENGTH,
        REPLY_WRONG_ID,
        REPLY_WRONG_CHECKSUM
    };

    Reply readReply(uint8_t command, uint8_t bufferOut[6]);

    void updateInternal(bool firstTime = false);
    void update(bool firstTime = false);
    bool reportUpdates(bool firstTime = false);

private:
    SerialEx& serial_;
    const uint8_t pinHd_;

    const int PREHEAT_DURATION = 90 * 1000;  // 1.5 min
    const int CALIBRATION_TIME = 8 * 1000;   // 8 seconds

    bool preheat_ = false;
    dword preheatStartedTime_ = 0;

    bool inCalibration_ = false;
    dword calibrationStartedTime_ = 0;

    word co2_ = 0;  // ppm

    word co2LastReported_ = 0;
    word co2Interval_ = 60 * 20;  // 20 mins default, min 30 seconds
    word co2Threshold_ = 50;
    unsigned long lastReportedTimeCO2_ = 0;

    const uint8_t reportChannel_;
    const uint8_t configChannel_;
};

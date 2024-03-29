#pragma once

#include "common.h"
#include "serialex.h"
#include "tasks.h"
#include "winsenutils.h"

class PM25Task : public Task
{
public:
    PM25Task(SerialEx& serial, uint8_t reportChannel1, uint8_t reportChannel2, uint8_t reportChannel3);

    void setup();

    uint16_t getPM2d5();
    uint16_t getPM10();
    uint16_t getPM1d0();

protected:
    void update() override;

private:
    void setQAMode();

    WinsenReply receiveData();
    void requestData();

    void updateInternal(bool firstTime = false);
    void update(bool firstTime = false);
    bool reportUpdates(bool firstTime = false);

   bool updatePreheat();

private:
    SerialEx& serial_;

    uint16_t pm2d5_ = 0;
    uint16_t pm10_ = 0;
    uint16_t pm1d0_ = 0;

    word pm2d5LastReported_ = 0;
    word pm2d5Interval_ = 60 * 20;  // 20 mins default, min 30 seconds
    word pm2d5Threshold_ = 5;
    unsigned long lastReportedTimePM2d5_ = 0;

    const uint8_t reportChannel1_;
    const uint8_t reportChannel2_;
    const uint8_t reportChannel3_;

    const int PREHEAT_DURATION = 60 * 1000;  // 1 min
    bool preheat_ = false;
    dword preheatStartedTime_ = 0;
};

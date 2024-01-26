#pragma once

#include "common.h"
#include "tasks.h"


class TVOCTask : public Task
{
public:
    TVOCTask(uint8_t pin, uint8_t tvocUpdateChannel);

    void setup();

    // returns TVOC as 0 .. 10 value
    byte get();

    // returns TVOS as 0 .. 100 percent
    byte getPercent();


protected:
    void update() override;

private:
    void updateInternal(bool firstTime = false);
    void updateTVOC(bool firstTime = false);
    bool reportTVOCUpdates(bool firstTime = false);

private:
    const uint8_t pin_;
    const uint8_t tvocUpdateChannel_;

    byte tvocLevel_ = 0;
    word tvocLastReported_ = 0;
    word tvocInterval_ = 60 * 20;  // 20 mins default, min 30 seconds
    word tvocThreshold_ = 0;
    unsigned long lastReportedTimeTVOC_ = 0;
};

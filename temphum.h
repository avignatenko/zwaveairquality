#pragma once

#include "common.h"
#include "tasks.h"
#include "temphumsensor.h"

class TempHumTask : public Task
{
public:
    TempHumTask(TempHumSensor& sensor);

    // returns temp (degrees Celcius) * 10 as two bytes
    word getTemperature();
    // returns humidity (percent) * 10 as two bytes
    word getHumidity();

    void updateTempHumFromCFGParams();

    void setup();

protected:
    void update() override;

private:
    bool reportTempUpdates(bool firstTime = false);
    bool reportHumUpdates(bool firstTime = false);
    void updateInternal(bool firstTime = false);

private:
    TempHumSensor& sensor_;

    word humidityLastReported_ = 0;
    unsigned long lastReportedTimeHumidity_ = 0;
    word humThreshold_ = 5;
    word humCorrect_ = 0;

    word temperatureLastReported_ = 0;
    unsigned long lastReportedTimeTemperature_ = 0;
    word tempThreshold_ = 2;
    word tempCorrect_ = 0;

    word tempHumInterval_ = 60 * 20;  // 20 mins default, min 30 seconds
};

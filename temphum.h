#pragma once

#include "tasks.h"
#include "common.h"
#include "temphumsensor.h"

class TempHumTask : public Task
{
public:
    TempHumTask(TempHumSensor& sensor) : Task(2000), sensor_(sensor) {}

    // returns temp (degrees Celcius) * 10 as two bytes
    word getTemperature();
    // returns humidity (percent) * 10 as two bytes
    word getHumidity();

    void updateTempHumFromCFGParams();

    void setup();

protected:
    void update();

private:
    bool reportTempUpdates(bool firstTime = false);
    bool reportHumUpdates(bool firstTime = false);
    void updateInternal(bool firstTime = false);
private:
    TempHumSensor& sensor_;

    word s_humidityLastReported = 0;
    unsigned long s_lastReportedTimeHumidity = 0;
    word s_hum_threshold = 5;
    word s_hum_correct = 0;

    word s_temperatureLastReported = 0;
    unsigned long s_lastReportedTimeTemperature = 0;
    word s_temp_threshold = 2;
    word s_temp_correct = 0;

    word s_temp_hum_interval = 60 * 20;  // 20 mins default, min 30 seconds
};

#pragma once

#include "common.h"
#include "tasks.h"
#include "temphum.h"

class TempHumTask : public Task
{
public:
    TempHumTask(TempHumSensor& sensor, const TempHum::Config& config, const TempHum::Report& report)
        : Task(5000), sensor_(sensor, config, report)
    {
    }

    // returns temp (degrees Celcius) * 10 as two bytes
    word getTemperature() { return sensor_.getTemperature(); }

    // returns humidity (percent) * 10 as two bytes
    word getHumidity() { return sensor_.getHumidity(); }

    void updateTempHumFromCFGParams() { return sensor_.updateTempHumFromCFGParams(); }

    void setup() { return sensor_.setup(); }

protected:
    void update() override { sensor_.update();}

private:
    TempHum sensor_;
};

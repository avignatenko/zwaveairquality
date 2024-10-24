#pragma once

#include "common.h"
#include "temphumsensor.h"

class TempHum
{
public:
    struct Config
    {
        uint8_t tempHumIntervalChannel;
        uint8_t tempThresholdChannel;
        uint8_t humThresholdChannel;
        uint8_t tempCorrectChannel;
        uint8_t humCorrectChannel;
    };

    struct Report
    {
        uint8_t tempReportChannel;
        uint8_t humReportChannel;
    };

    TempHum(TempHumSensor& sensor, const Config& config, const Report& report);

    // returns temp (degrees Celcius) * 10 as two bytes
    word getTemperature();
    // returns humidity (percent) * 10 as two bytes
    word getHumidity();

    void updateTempHumFromCFGParams();

    void setup(bool firstTimeUpdate = true);
    void update();

    void updateSensorValues();
    void reportUpdates(bool firstTime = false);

private:
    bool reportTempUpdates(bool firstTime = false);
    bool reportHumUpdates(bool firstTime = false);
    void updateInternal(bool firstTime = false);

private:
    Config config_;
    Report report_;
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


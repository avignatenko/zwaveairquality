#pragma once

#include "common.h"

#include "Wire.h"
#include "SHTSensor.h"

#include "temphumsensor.h"


class SensirionSensor : public TempHumSensor
{
public:
    SensirionSensor(uint8_t sclPin, uint8_t sdaPin);

    // returns temp (degrees Celcius)
    float getTemperatureInternal() override;

    // returns humidity (percent)
    float getHumidityInternal() override;

    void setup() override;
    void update() override;

private:
    SHTSensor sht_;

    float humidity_ = 0;
    float temperature_ = 0;
};

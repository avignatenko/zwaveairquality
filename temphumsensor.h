#pragma once

#include "common.h"

class TempHumSensor
{
public:
   
    // returns temp (degrees Celcius)
    virtual float getTemperatureInternal() = 0;

    // returns humidity (percent)
    virtual float getHumidityInternal() = 0;

    virtual void setup() = 0;
    virtual void update() = 0;
};
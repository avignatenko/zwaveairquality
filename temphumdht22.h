#pragma once

#include "common.h"

#include "ZUNO_DHT.h"

#include "temphumsensor.h"

class DHT22Sensor: public TempHumSensor
{
public:
     
    DHT22Sensor(uint8_t pin);
    
    // returns temp (degrees Celcius)
    float getTemperatureInternal() override;

    // returns humidity (percent)
    float getHumidityInternal() override;

    void setup() override;
    void update() override;

private:

    DHT dht22_;

    float humidity_ = 0;
    float temperature_ = 0;
};

#pragma once

#include "common.h"
#include "luxsensor.h"

class LuxTEMT6000 : public LuxSensor
{
public:
    LuxTEMT6000(uint8_t pin);

    // returns luminance in lux
    float getLuminance() override;

    void setup() override;
    void update() override;

private:
    uint8_t pin_;
    float luminance_ = 0;
};

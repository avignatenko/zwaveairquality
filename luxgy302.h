#pragma once

#include "common.h"
#include "luxsensor.h"
#include "BH1750.h"
#include "Wire.h"

class LuxGY302 : public LuxSensor
{
public:
    LuxGY302(TwoWire& wire, uint8_t addr);

    // returns luminance in lux
    float getLuminance() override;

    void setup() override;
    void update() override;

private:
    float luminance_ = 0;

    TwoWire& wire_;
    uint8_t addr_;
    BH1750 sensor_;
};

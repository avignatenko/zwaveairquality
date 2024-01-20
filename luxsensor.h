#pragma once

#include "common.h"

class LuxSensor
{
public:
   
    // returns luminance in lux
    virtual float getLuminance() = 0;

    virtual void setup() = 0;
    virtual void update() = 0;
};
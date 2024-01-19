#pragma once

#include "common.h"
#include "tasks.h"

class LuxTask : public Task
{
public:
    LuxTask(uint8_t pin);

    void setup();
    uint16_t getLuminance();

protected:
    void update() override;

private:
    uint8_t pin_;

    uint16_t lux_ = 0;
};

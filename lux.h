#pragma once

#include "common.h"
#include "tasks.h"
#include "luxsensor.h"

class LuxTask : public Task
{
public:
    LuxTask(LuxSensor& sensor);

    void setup();
    float getLuminance();

protected:
    void update() override;

private:
    LuxSensor& sensor_;
};

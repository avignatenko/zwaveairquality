#pragma once

#include "common.h"
#include "tasks.h"

class PM25Task : public Task
{
public:
    PM25Task(HardwareSerial& serial);

    void setup();
   
    uint16_t getPM2_5();

protected:
    void update() override;

private:
    HardwareSerial& serial_;

    uint16_t pm2_5_ = 0;
};

#pragma once

#include "common.h"
#include "serialex.h"
#include "tasks.h"

class PM25Task : public Task
{
public:
    PM25Task(SerialEx& serial);

    void setup();

    uint16_t getPM2d5();
    uint16_t getPM10();
    uint16_t getPM1d0();

protected:
    void update() override;

private:
    void setQAMode();

    enum Reply
    {
        REPLY_OK = 0,
        REPLY_NO_ANSWER,
        REPLY_WRONG_LENGTH,
        REPLY_WRONG_ID,
        REPLY_WRONG_CHECKSUM
    };

    Reply receiveData();
    void requestData();

private:
    SerialEx& serial_;

    uint16_t pm2d5_ = 0;
    uint16_t pm10_ = 0;
    uint16_t pm1d0_ = 0;
};

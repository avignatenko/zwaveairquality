#pragma once

#include "common.h"
#include "serialex.h"
#include "tasks.h"

class COTask : public Task
{
public:
    COTask(SerialEx& serial);

    void setup();

    // return ppm * 10
    uint16_t getCO();

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
        REPLY_WRONG_CHECKSUM,
        REPLY_SENSOR_FAILED
    };

    Reply receiveData();
    void requestData();

private:
    SerialEx& serial_;

    uint16_t co_ = 0; // ppm * 10
};

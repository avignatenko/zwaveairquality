#pragma once

#include "common.h"
#include "serialex.h"
#include "tasks.h"
#include "winsenutils.h"

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

    WinsenReply receiveData();
    void requestData();

private:
    SerialEx& serial_;

    uint16_t co_ = 0; // ppm * 10
};

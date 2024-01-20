#include "serialex.h"

SerialEx::SerialEx(uint8_t channel, SerialData& data) : channel_(channel), data_(data) {}

void SerialEx::activate()
{
    data_.activate(channel_);
}

HardwareSerial& SerialEx::captureSerial()
{
    activate();
    return data_.serial_;
}

void SerialEx::begin(unsigned long baud)
{
    data_.begin(baud);
}

void SerialEx::end()
{
    data_.end();
}
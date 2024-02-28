#pragma once

#include "HardwareSerial.h"
#include "common.h"

struct SerialData
{
    SerialData(uint8_t s0pin, uint8_t s1pin, HardwareSerial& serial, uint8_t rxpin = 0, uint8_t txpin = 0)
        : s0pin_(s0pin), s1pin_(s1pin), serial_(serial), rxpin_(rxpin), txpin_(txpin)
    {
        pinMode(s0pin_, OUTPUT);
        pinMode(s1pin_, OUTPUT);
    }

    void activate(uint8_t channel)
    {
        if (activeChannel_ == channel) return;  // already active

#if SERIAL_LOGS
        Serial.print("Serial: activate channel ");
        Serial.println(channel);
#endif

        // read everything left before switching
        while (serial_.available()) serial_.read();

        // switch!
        digitalWrite(s0pin_, channel % 2);
        digitalWrite(s1pin_, channel / 2);

        // read everything left after switching
        while (serial_.available()) serial_.read();

        activeChannel_ = channel;
    }

    void begin(unsigned long baud)
    {
        if (!started_ || (started_ && baud_ != baud))
        {
            if (started_) serial_.end();
            if (rxpin_ == 0 || txpin_ == 0)
                serial_.begin(baud);
            else
            {
#if SERIAL_LOGS
                Serial.print("SerialData: Starting with custom rx, tx: ");
                Serial.print(rxpin_);
                Serial.print(" ");
                Serial.print(txpin_);
                Serial.println();
#endif
                serial_.begin(baud, SERIAL_8N1, rxpin_, txpin_);
            }
        }

        started_ = true;
        baud_ = baud;
    }

    void end()
    {
        serial_.end();
        started_ = false;
        baud_ = 0;
    }

    uint8_t s0pin_;
    uint8_t s1pin_;

    HardwareSerial& serial_;
    uint8_t rxpin_;
    uint8_t txpin_;
    bool started_ = false;
    unsigned long baud_ = 0;
    uint8_t activeChannel_ = ~0;
};

class SerialEx
{
public:
    SerialEx(uint8_t channel, SerialData& data);

    void begin(unsigned long baud);
    void end();

    HardwareSerial& captureSerial();

private:
    void activate();

private:
    uint8_t channel_;
    SerialData& data_;
};

extern SerialEx SerialEx01;
extern SerialEx SerialEx02;
extern SerialEx SerialEx03;
extern SerialEx SerialEx04;
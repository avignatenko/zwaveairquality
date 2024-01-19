#include "lux.h"

LuxTask::LuxTask(uint8_t pin) : Task(10000), pin_(pin) {}

void LuxTask::setup()
{
    pinMode(pin_, INPUT);  // define pin as input
}

uint16_t LuxTask::getLuminance() { return lux_;}

void LuxTask::update()
{
    int rawValue = 0;
    const int TOTAL_AVERAGE = 10;
    for (int i = 0; i < TOTAL_AVERAGE; ++i)
    {
        rawValue += analogRead(pin_);
        delay(2);
    }
    rawValue /= TOTAL_AVERAGE;

#if SERIAL_LOGS
    Serial.print("Lux: raw value: ");
    Serial.println(rawValue);
#endif

    // get from TEMP6000 sensor
    float volts = rawValue * 3.0 / 2047.0;
    float amps = volts / 10000.0;  // across 10,000 Ohms (series resistor)
    float microamps = amps * 1000000;
    float lux = microamps * 2.0;

#if SERIAL_LOGS
    Serial.print("Lux: ");
    Serial.println((uint16_t)lux);
#endif

    lux_ = (uint16_t)lux;
    
}
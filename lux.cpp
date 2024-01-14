#include "lux.h"

uint16_t getLuminance()
{
    // get from TEMP6000 sensor
    float volts = analogRead(TEMPT6000_PIN) * 3.0 / 2047.0;
    float amps = volts / 10000.0; // across 10,000 Ohms (series resistor)
    float microamps = amps * 1000000;
    float lux = microamps * 2.0;

    return (uint16_t)lux;
}
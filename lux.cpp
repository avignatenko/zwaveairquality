#include "lux.h"

void setupLuxSensor()
{
    pinMode(TEMPT6000_PIN, INPUT); // define pin as input
}

uint16_t getLuminance()
{
    int rawValue = 0;
    const int TOTAL_AVERAGE = 10;
    for (int i = 0; i < TOTAL_AVERAGE; ++i)
    {
        rawValue += analogRead(TEMPT6000_PIN);
        delay(2);
    }
    rawValue /= TOTAL_AVERAGE;

#if SERIAL_LOGS
    Serial.print("Lux raw value: ");
    Serial.println(rawValue);
#endif

    // get from TEMP6000 sensor
    float volts = rawValue * 3.0 / 2047.0;
    float amps = volts / 10000.0; // across 10,000 Ohms (series resistor)
    float microamps = amps * 1000000;
    float lux = microamps * 2.0;

    return (uint16_t)lux;
}

#include "co2.h"

#include "HardwareSerial.h"

word s_co2 = 400; // ppm

HardwareSerial &s_co2_serial = Serial0;

word getCO2()
{
    return s_co2;
}

void setupCO2()
{
    s_co2_serial.begin(9600);
}

void updateCO2(bool firstTime)
{
#if SERIAL_LOGS
    Serial.print("CO2 ");
    Serial.print(s_co2);
    Serial.print(" ");
    Serial.println();
#endif
}

bool reportCO2Updates(bool firstTime)
{
}
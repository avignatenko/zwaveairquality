#include "temphumsensirion.h"

SensirionSensor::SensirionSensor(TwoWire& wire) : wire_(wire) {}

float SensirionSensor::getTemperatureInternal()
{
    return temperature_;
}

float SensirionSensor::getHumidityInternal()
{
    return humidity_;
}

void SensirionSensor::setup()
{
#if SERIAL_LOGS
    Serial.println("SHT: setup started");
#endif

    if (sht_.init(wire_))
    {
#if SERIAL_LOGS
        Serial.println("SHT: init success");
#endif
    }
    else
    {
#if SERIAL_LOGS
        Serial.println("SHT: init failed");
#endif
    }
    sht_.setAccuracy(SHTSensor::SHT_ACCURACY_MEDIUM);  // only supported by SHT3x
}

void SensirionSensor::update()
{
    if (sht_.readSample())
    {
        humidity_ = sht_.getHumidity();
        temperature_ = sht_.getTemperature();
    }
    else
    {
        humidity_ = -100;
        temperature_ = -100;

#if SERIAL_LOGS
        Serial.println("SHT: Read Error!");
#endif
    }
}

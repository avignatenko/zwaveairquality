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
        Serial.print("Found sensor: ");
        Serial.println(sht_.mSensorType);
#endif
    }
    else
    {
#if SERIAL_LOGS
        Serial.println("SHT: init failed");
#endif
    }
}

void SensirionSensor::update()
{
    if (sht_.readSample())
    {
        humidity_ = sht_.getHumidity();
        temperature_ = sht_.getTemperature();

        errorsNum_ = 0;
    }
    else
    {
        ++errorsNum_;

        if (errorsNum_ > ReportErrorThreshold)
        {
            humidity_ = -100;
            temperature_ = -100;
        }
#if SERIAL_LOGS
        Serial.print("SHT: Read Error! Errors: ");
        Serial.println(errorsNum_);
#endif
    }
}

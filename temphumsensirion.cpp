#include "temphumsensirion.h"

#ifdef SENSIRION_DHT_SENSOR

#include "SHTSensor.h"

SHTSensor sht;

float s_humidity = 0;
float s_temperature = 0;

float getTemperatureInternal()
{
    return s_temperature;
}

float getHumidityInternal()
{
    return s_humidity;
}

void setupTempHumSensor()
{
    Wire.begin();

    if (sht.init())
    {
#if SERIAL_LOGS
        Serial.println("SHT init success");
#endif
    }
    else
    {
#if SERIAL_LOGS
        Serial.println("SHT init failed");
#endif
    }
    sht.setAccuracy(SHTSensor::SHT_ACCURACY_HIGH); // only supported by SHT3x
}

void updateTempHumSensor()
{
    if (sht.readSample())
    {
        s_humidity = sht.getHumidity();
        s_temperature = sht.getTemperature();
    }
    else
    {
        s_humidity = -100;
        s_temperature = -100;

#if SERIAL_LOGS
        Serial.println("SHT Read Error!");
#endif
    }
}


#endif
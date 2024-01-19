#include "temphumsensirion.h"

#ifdef SENSIRION_DHT_SENSOR

#include "SHTSensor.h"

SHTSensor sht;

const uint8_t sht_scl_pin = 10;
const uint8_t sht_sda_pin = 11;

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
    Wire.begin(0, sht_scl_pin, sht_sda_pin);

    if (sht.init())
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
    sht.setAccuracy(SHTSensor::SHT_ACCURACY_MEDIUM); // only supported by SHT3x
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
        Serial.println("SHT: Read Error!");
#endif
    }
}


#endif
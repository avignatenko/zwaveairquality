#include "temphumdht22.h"

#ifdef DHT22_SENSOR

#include "ZUNO_DHT.h"

#define PIN_DHT 17

// temp & humidity sensor (DHT22)
DHT dht22_sensor(PIN_DHT, DHT22);

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
    dht22_sensor.begin();
}

void updateTempHumSensor()
{
    byte result;
    result = dht22_sensor.read(true);

    if (result == ZunoErrorOk)
    {
        s_humidity = dht22_sensor.readHumidity();
        s_temperature = dht22_sensor.readTemperature() ;
    }
    else
    {

        s_humidity = -100;
        s_temperature = -100;
    }
}

#endif
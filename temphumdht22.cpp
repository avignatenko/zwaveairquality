#include "temphumdht22.h"

DHT22Sensor::DHT22Sensor(uint8_t pin) : dht22_(pin, DHT22) {}

float DHT22Sensor::getTemperatureInternal()
{
    return temperature_;
}

float DHT22Sensor::getHumidityInternal()
{
    return humidity_;
}

void DHT22Sensor::setup()
{
    dht22_.begin();
}

void DHT22Sensor::update()
{
    byte result;
    result = dht22_.read(true);

    if (result == ZunoErrorOk)
    {
        humidity_ = dht22_.readHumidity();
        temperature_ = dht22_.readTemperature();
    }
    else
    {
        humidity_ = -100;
        temperature_ = -100;
    }
}

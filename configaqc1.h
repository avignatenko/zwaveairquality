#pragma once

#include "Wire.h"

// choose temp & humidity sensor
#define SENSIRION_DHT_SENSOR 1
// #define DHT22_SENSOR 1

#ifdef SENSIRION_DHT_SENSOR
TwoWire& dhtWire = Wire0;
#endif

#ifdef DHT22_SENSOR
const uint8_t DHT22_PIN = 17;
#endif

enum
{
    CONFIG_TEMPERATURE_THRESHOLD_DEGREES = 64,
    CONFIG_HUMIDITY_THRESHOLD_PERCENT,
    CONFIG_TEMPERATURE_CORRECTION_DEGREES,
    CONFIG_HUMIDITY_CORRECTION_PERCENT
};

enum
{
    CHANNEL_DUMMY_SWITCH = 1,
    CHANNEL_TEMPERATURE,
    CHANNEL_HUMIDITY,
};

const uint8_t WIRE_0_SCL_PIN = 10;
const uint8_t WIRE_0_SDA_PIN = 11;

const uint8_t SHT_POWER_PIN = 9;
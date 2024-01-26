#pragma once

// choose temp & humidity sensor
#define SENSIRION_DHT_SENSOR 1
//#define DHT22_SENSOR 1

enum
{
  CONFIG_TEMPERATURE_HUMIDITY_INTERVAL_SEC = 64,
  CONFIG_TEMPERATURE_THRESHOLD_DEGREES,
  CONFIG_HUMIDITY_THRESHOLD_PERCENT,
  CONFIG_TEMPERATURE_CORRECTION_DEGREES,
  CONFIG_HUMIDITY_CORRECTION_PERCENT
};

#define CHANNEL_TEMPERATURE 1
#define CHANNEL_HUMIDITY 2
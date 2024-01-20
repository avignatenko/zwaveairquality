#pragma once

#include "Arduino.h"

#define SERIAL_LOGS 1

// choose temp & humidity sensor
#define SENSIRION_DHT_SENSOR 1
//#define DHT22_SENSOR 1

#define LUX_GY302 1
//#define LUX_TEMT6000 1

enum
{
  CONFIG_TEMPERATURE_HUMIDITY_INTERVAL_SEC = 64,
  CONFIG_TEMPERATURE_THRESHOLD_DEGREES,
  CONFIG_HUMIDITY_THRESHOLD_PERCENT,
  CONFIG_TEMPERATURE_CORRECTION_DEGREES,
  CONFIG_HUMIDITY_CORRECTION_PERCENT,
  CONFIG_CO2_START_CALIBRATION,
  CONFIG_AUTO_NIGHT_MODE,
  CONFIG_NIGHT_MODE_LUMINANCE,
  CONFIG_NIGHT_MODE_HYSTERESIS
};

#define CHANNEL_DEFAULT 1
#define CHANNEL_NIGHT_MODE 2
#define CHANNEL_TEMPERATURE 3
#define CHANNEL_HUMIDITY 4
#define CHANNEL_CO2 5
#define CHANNEL_TVOC 6
#define CHANNEL_PM2_2 7

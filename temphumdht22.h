#pragma once

#include "common.h"

#ifdef DHT22_SENSOR
// returns temp (degrees Celcius)
float getTemperatureInternal();

// returns humidity (percent) 
float getHumidityInternal();

void setupTempHumSensor();
void updateTempHumSensor();

#endif
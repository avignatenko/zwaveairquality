#pragma once

#include "common.h"

#ifdef SENSIRION_DHT_SENSOR
// returns temp (degrees Celcius) * 10 as two bytes
word getTemperature();

// returns humidity (percent) * 10 as two bytes
word getHumidity();

void setupTempHumSensor();
void updateTempHumSensor();
void updateTempHumFromCFGParams();

bool reportTempUpdates(bool firstTime = false);
bool reportHumUpdates(bool firstTime = false);
#endif
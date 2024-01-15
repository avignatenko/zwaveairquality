#pragma once

#include "common.h"

#if defined SENSIRION_DHT_SENSOR
#include "temphumsensirion.h"
#elif defined DHT22_SENSOR 
#include "temphumdht22.h"
#endif

// returns temp (degrees Celcius) * 10 as two bytes
word getTemperature();
// returns humidity (percent) * 10 as two bytes
word getHumidity();

void updateTempHumFromCFGParams();

bool reportTempUpdates(bool firstTime = false);
bool reportHumUpdates(bool firstTime = false);

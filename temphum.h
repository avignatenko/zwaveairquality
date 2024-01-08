#pragma once

#include "common.h"

// returns temp (degrees Celcius) * 10 as two bytes
word getTemperature();

// returns humidity (percent) * 10 as two bytes
word getHumidity();

void setupDHT();
void updateDHT();
void updateTempHumFromCFGParams();

bool reportTempUpdates(bool firstTime = false);
bool reportHumUpdates(bool firstTime = false);
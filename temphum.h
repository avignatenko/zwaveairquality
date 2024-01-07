#pragma once

#include "common.h"

word getTemperature();
word getHumidity();

void setupDHT();
void updateDHT();
void updateTempHumFromCFGParams();

bool reportTempUpdates(bool firstTime = false);
bool reportHumUpdates(bool firstTime = false);
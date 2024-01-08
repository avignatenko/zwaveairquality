#pragma once

#include "common.h"

// returns CO2 as PPM value
word getCO2();

void setupCO2();
void updateCO2(bool firstTime = false);
bool reportCO2Updates(bool firstTime = false);

void triggerCO2Calibration();
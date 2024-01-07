#pragma once

#include "common.h"

word getCO2();
void setupCO2();
void updateCO2(bool firstTime = false);
bool reportCO2Updates(bool firstTime = false);
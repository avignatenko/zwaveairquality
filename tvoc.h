#pragma once

#include "common.h"

// returns TVOC as 0 .. 10 value 
byte getTVOC();

// returns TVOS as 0 .. 100 percent
byte getTVOCPercent();

void setupTVOC();
void updateTVOC(bool firstTime = false);
bool reportTVOCUpdates(bool firstTime = false);
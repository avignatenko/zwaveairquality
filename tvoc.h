#pragma once

#include "common.h"

byte getTVOC();
void setupTVOC();
void updateTVOC(bool firstTime = false);
bool reportTVOCUpdates(bool firstTime = false);
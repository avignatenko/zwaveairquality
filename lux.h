#pragma once

#include "common.h"

const int TEMPT6000_PIN = A1;

void setupLuxSensor();
uint16_t getLuminance();

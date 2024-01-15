#pragma once

#include "common.h"

#if defined SENSIRION_DHT_SENSOR
#include "temphumsensirion.h"
#elif defined DHT22_SENSOR 
#include "temphumdht22.h"
#endif


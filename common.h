#pragma once

#include "Arduino.h"

#define SERIAL_LOGS 1

#define ZUNO_SENSOR_MULTILEVEL_TEMPERATURE_2(GETTER)                                                 \
    ZUNO_SENSOR_MULTILEVEL(ZUNO_SENSOR_MULTILEVEL_TYPE_TEMPERATURE, SENSOR_MULTILEVEL_SCALE_CELSIUS, \
                           SENSOR_MULTILEVEL_SIZE_TWO_BYTES, SENSOR_MULTILEVEL_PRECISION_ONE_DECIMAL, GETTER)

#define ZUNO_SENSOR_MULTILEVEL_HUMIDITY_2(GETTER)                                                                   \
    ZUNO_SENSOR_MULTILEVEL(ZUNO_SENSOR_MULTILEVEL_TYPE_RELATIVE_HUMIDITY, SENSOR_MULTILEVEL_SCALE_PERCENTAGE_VALUE, \
                           SENSOR_MULTILEVEL_SIZE_TWO_BYTES, SENSOR_MULTILEVEL_PRECISION_ONE_DECIMAL, GETTER)

#define ZUNO_SENSOR_MULTILEVEL_CO2_LEVEL_2(GETTER)                                                           \
    ZUNO_SENSOR_MULTILEVEL(ZUNO_SENSOR_MULTILEVEL_TYPE_CO2_LEVEL, SENSOR_MULTILEVEL_SCALE_PARTS_PER_MILLION, \
                           SENSOR_MULTILEVEL_SIZE_TWO_BYTES, SENSOR_MULTILEVEL_PRECISION_ZERO_DECIMALS, GETTER)

#define ZUNO_SENSOR_MULTILEVEL_GP_PM2_5_LEVEL(GETTER)                                                     \
    ZUNO_SENSOR_MULTILEVEL(ZUNO_SENSOR_MULTILEVEL_TYPE_GENERAL_PURPOSE_VALUE,                             \
                           SENSOR_MULTILEVEL_SCALE_DIMENSIONLESS_VALUE, SENSOR_MULTILEVEL_SIZE_TWO_BYTES, \
                           SENSOR_MULTILEVEL_PRECISION_ZERO_DECIMALS, GETTER)

#define ZUNO_SENSOR_MULTILEVEL_GP_VOC_PERCENT(GETTER)                                                 \
    ZUNO_SENSOR_MULTILEVEL(ZUNO_SENSOR_MULTILEVEL_TYPE_GENERAL_PURPOSE_VALUE,                         \
                           SENSOR_MULTILEVEL_SCALE_PERCENTAGE_VALUE, SENSOR_MULTILEVEL_SIZE_ONE_BYTE, \
                           SENSOR_MULTILEVEL_PRECISION_ZERO_DECIMALS, GETTER)

                           
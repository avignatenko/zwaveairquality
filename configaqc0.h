#pragma once

#include "Wire.h"

// general components

// choose temp & humidity sensor
#define SENSIRION_DHT_SENSOR 1
// #define DHT22_SENSOR 1

#ifdef SENSIRION_DHT_SENSOR
TwoWire& dhtWire = Wire0;
#endif

#ifdef DHT22_SENSOR
const uint8_t DHT22_PIN = 17;
#endif

#define LUX_GY302 1
// #define LUX_TEMT6000 1

#ifdef LUX_GY302
const uint8_t BH1750_ADDRESS = 0x23;
TwoWire& luxWire = Wire0;
#endif

#ifdef LUX_TEMT6000
const uint8_t TEMT6000_PIN = 4;
#endif

// Z-Wave
enum
{
    CONFIG_TEMPERATURE_HUMIDITY_INTERVAL_SEC = 64,
    CONFIG_TEMPERATURE_THRESHOLD_DEGREES,
    CONFIG_HUMIDITY_THRESHOLD_PERCENT,
    CONFIG_TEMPERATURE_CORRECTION_DEGREES,
    CONFIG_HUMIDITY_CORRECTION_PERCENT,
    CONFIG_CO2_START_CALIBRATION,
    CONFIG_AUTO_NIGHT_MODE,
    CONFIG_NIGHT_MODE_LUMINANCE,
    CONFIG_NIGHT_MODE_HYSTERESIS
};

enum
{
    CHANNEL_DEFAULT = 1,
    CHANNEL_NIGHT_MODE,
    CHANNEL_TEMPERATURE,
    CHANNEL_HUMIDITY,
    CHANNEL_CO2,
    CHANNEL_TVOC,
    CHANNEL_PM2_2,
};

// Wire (I2C)
const uint8_t WIRE_0_SCL_PIN = 10;
const uint8_t WIRE_0_SDA_PIN = 11;

// Serial multiplexor
const uint8_t SERIAL_MULTIPLEXOR_S0_PIN = 12;
const uint8_t SERIAL_MULTIPLEXOR_S1_PIN = 14;
HardwareSerial& multiplexorSerial = Serial0;

// Serials
extern SerialEx SerialEx00;
extern SerialEx SerialEx01;
extern SerialEx SerialEx02;
extern SerialEx SerialEx03;

// TVOC
const uint8_t TVOC_PIN = 9;

// CO2
SerialEx& co2Serial = SerialEx00;
const uint8_t CO2_HD_PIN = 6;

// PM2.5

SerialEx& pm25Serial = SerialEx01;

// Display

HardwareSerial& displaySerial = Serial1;
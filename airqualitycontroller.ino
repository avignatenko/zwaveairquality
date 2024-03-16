
#include "EEPROM.h"

#include "co2.h"
#include "lux.h"
#include "nextion.h"
#include "pm25.h"
#include "co.h"
#include "serialex.h"
#include "temphum.h"
#include "tvoc.h"

#include "Tasks.h"
#include "common.h"
#include "configaqc0.h"

ZUNO_ENABLE(MODERN_MULTICHANNEL);  // No clusters, the first channel is mapped to NIF only

SerialData serialData0(SERIAL_MULTIPLEXOR_S0_PIN, SERIAL_MULTIPLEXOR_S1_PIN, multiplexorSerial, SERIAL_MULTIPLEXOR_RX_PIN, SERIAL_MULTIPLEXOR_TX_PIN);

SerialEx SerialEx00(0, serialData0);
SerialEx SerialEx01(1, serialData0);
SerialEx SerialEx02(2, serialData0);
SerialEx SerialEx03(3, serialData0);

#if defined SENSIRION_DHT_SENSOR

#include "temphumsensirion.h"
SensirionSensor sensor(dhtWire);

#elif defined DHT22_SENSOR

#include "temphumdht22.h"
DHT22Sensor sensor(DHT22_PIN);

#endif

#if defined LUX_GY302

#include "luxgy302.h"
LuxGY302 luxSensor(luxWire, BH1750_ADDRESS);

#elif if defined LUX_TEMT6000

#include "luxtemt6000.h"
LuxTEMT6000 luxSensor(TEMT6000_PIN);

#endif

TempHumTask tempHumTask(sensor,
                        TempHumTask::Config{CONFIG_TEMPERATURE_HUMIDITY_INTERVAL_SEC,
                                            CONFIG_TEMPERATURE_THRESHOLD_DEGREES, CONFIG_HUMIDITY_THRESHOLD_PERCENT,
                                            CONFIG_TEMPERATURE_CORRECTION_DEGREES, CONFIG_HUMIDITY_CORRECTION_PERCENT},
                        TempHumTask::Report{CHANNEL_TEMPERATURE, CHANNEL_HUMIDITY});

TVOCTask tvocTask(TVOC_PIN, CHANNEL_TVOC);
CO2Task co2Task(co2Serial, CO2_HD_PIN, CHANNEL_CO2, CONFIG_CO2_START_CALIBRATION);
LuxTask luxTask(luxSensor);
PM25Task pm25Task(pm25Serial, CHANNEL_PM2_5, CHANNEL_PM10, CHANNEL_PM1d0);
COTask coTask(coSerial);

DisplayTask displayTask(DisplayTask::Tasks{tempHumTask, tvocTask, co2Task, luxTask, pm25Task, coTask},
                        DisplayTask::Config{CONFIG_AUTO_NIGHT_MODE, CONFIG_NIGHT_MODE_LUMINANCE,
                                            CONFIG_NIGHT_MODE_HYSTERESIS},
                        displaySerial, DISPLAY_RX_PIN, DISPLAY_TX_PIN);

// need to use this due to ZUNO preprocessor behaviour

byte getDisplayEnabled1()
{
    return displayTask.enabled();
}
void setDisplayEnabled1(byte value)
{
    return displayTask.enable(value);
}

byte getDisplayBrightness1()
{
    return displayTask.getDayBrightness();
}
void setDisplayBrightness1(byte value)
{
    return displayTask.setDayBrightness(value);
}

word getTemperature1()
{
    return tempHumTask.getTemperature();
}
word getHumidity1()
{
    return tempHumTask.getHumidity();
}
word getCO21()
{
    return co2Task.getCO2();
}

word getCO()
{
    return coTask.getCO();
}

byte getTVOCPercent1()
{
    return tvocTask.getPercent();
}

byte getPM2d5()
{
    return pm25Task.getPM2d5();
}

byte getPM10()
{
    return pm25Task.getPM10();
}

byte getPM1d0()
{
    return pm25Task.getPM1d0();
}

byte getDisplayNightMode1()
{
    return displayTask.getNightMode();
}

void setNightMode1(byte newValue)
{
    displayTask.setNightMode(newValue);
}

ZUNO_SETUP_CHANNELS(ZUNO_SWITCH_BINARY(getDisplayEnabled1, setDisplayEnabled1),
                    ZUNO_SWITCH_MULTILEVEL(getDisplayBrightness1, setDisplayBrightness1),
                    ZUNO_SWITCH_BINARY(getDisplayNightMode1, setNightMode1),
                    ZUNO_SENSOR_MULTILEVEL_TEMPERATURE_2(getTemperature1),
                    ZUNO_SENSOR_MULTILEVEL_HUMIDITY_2(getHumidity1), 
                    ZUNO_SENSOR_MULTILEVEL_CO2_LEVEL_2(getCO21),
                    ZUNO_SENSOR_MULTILEVEL_CO_LEVEL(getCO),
                    ZUNO_SENSOR_MULTILEVEL_GP_VOC_PERCENT(getTVOCPercent1),
                    ZUNO_SENSOR_MULTILEVEL_GP_PM2d5_LEVEL(getPM2d5),
                    ZUNO_SENSOR_MULTILEVEL_GP_PM2d5_LEVEL(getPM10), // fixme: not supported yet?
                    ZUNO_SENSOR_MULTILEVEL_GP_PM2d5_LEVEL(getPM1d0)); // fixme: not supported yet?

ZUNO_SETUP_CONFIGPARAMETERS(ZUNO_CONFIG_PARAMETER("Temperature and humidity update period (sec)", 30, 86400, 1800),
                            ZUNO_CONFIG_PARAMETER_1B("Temperature update threshold", 1, 255, 2),
                            ZUNO_CONFIG_PARAMETER_1B("Humidity update threshold", 1, 255, 5),
                            ZUNO_CONFIG_PARAMETER_1B("Temperature correction (deg * 10 + 100)", 0, 200, 100),
                            ZUNO_CONFIG_PARAMETER_1B("Humidity correction (% * 10 + 100)", 0, 200, 100),
                            ZUNO_CONFIG_PARAMETER_1B("Trigger CO2 calibration (set to 1)", 0, 1, 0),
                            ZUNO_CONFIG_PARAMETER_1B("Auto night mode (set to 1)", 0, 1, 0),
                            ZUNO_CONFIG_PARAMETER_1B("Night mode luminance (lux)", 0, 100, 10),
                            ZUNO_CONFIG_PARAMETER_1B("Night mode hysteresis (lux)", 0, 50, 5));

void updateFromCFGParams()
{
    tempHumTask.updateTempHumFromCFGParams();
    displayTask.updateFromCFGParams();
    co2Task.updateFromCFGParams();
}

void configParameterChanged(byte param, uint32_t value)
{
    zunoSaveCFGParam(param, value);
    updateFromCFGParams();
}

ZUNO_SETUP_CFGPARAMETER_HANDLER(configParameterChanged);

void setupI2C()
{
    Serial.print("Main: wire init start");

    int result = Wire.begin(0, WIRE_0_SCL_PIN, WIRE_0_SDA_PIN);
#if SERIAL_LOGS
    Serial.print("Main: wire init done");
    Serial.print(result);
#endif
}

void setup()
{
#if SERIAL_LOGS
    Serial.begin(115200);
#endif

    // i2c init with Wire0
    setupI2C();

    updateFromCFGParams();

    tempHumTask.setup();
    displayTask.setup();
    tvocTask.setup();
    co2Task.setup();
    luxTask.setup();
    pm25Task.setup();
    coTask.setup();
}

void loop()
{
    TaskManager.update();
}
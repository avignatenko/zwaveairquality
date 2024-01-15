
#include "EEPROM.h"

#include "temphum.h"
#include "tvoc.h"
#include "co2.h"
#include "nextion.h"
#include "lux.h"

#include "common.h"

word s_pm25 = 5; // ppm

#define ZUNO_SENSOR_MULTILEVEL_TEMPERATURE_2(GETTER) \
  ZUNO_SENSOR_MULTILEVEL(ZUNO_SENSOR_MULTILEVEL_TYPE_TEMPERATURE, SENSOR_MULTILEVEL_SCALE_CELSIUS, SENSOR_MULTILEVEL_SIZE_TWO_BYTES, SENSOR_MULTILEVEL_PRECISION_ONE_DECIMAL, GETTER)

#define ZUNO_SENSOR_MULTILEVEL_HUMIDITY_2(GETTER) \
  ZUNO_SENSOR_MULTILEVEL(ZUNO_SENSOR_MULTILEVEL_TYPE_RELATIVE_HUMIDITY, SENSOR_MULTILEVEL_SCALE_PERCENTAGE_VALUE, SENSOR_MULTILEVEL_SIZE_TWO_BYTES, SENSOR_MULTILEVEL_PRECISION_ONE_DECIMAL, GETTER)

#define ZUNO_SENSOR_MULTILEVEL_CO2_LEVEL_2(GETTER) \
  ZUNO_SENSOR_MULTILEVEL(ZUNO_SENSOR_MULTILEVEL_TYPE_CO2_LEVEL, SENSOR_MULTILEVEL_SCALE_PARTS_PER_MILLION, SENSOR_MULTILEVEL_SIZE_TWO_BYTES, SENSOR_MULTILEVEL_PRECISION_ZERO_DECIMALS, GETTER)

#define ZUNO_SENSOR_MULTILEVEL_GP_PM2_5_LEVEL(GETTER) \
  ZUNO_SENSOR_MULTILEVEL(ZUNO_SENSOR_MULTILEVEL_TYPE_GENERAL_PURPOSE_VALUE, SENSOR_MULTILEVEL_SCALE_DIMENSIONLESS_VALUE, SENSOR_MULTILEVEL_SIZE_TWO_BYTES, SENSOR_MULTILEVEL_PRECISION_ZERO_DECIMALS, GETTER)

#define ZUNO_SENSOR_MULTILEVEL_GP_VOC_PERCENT(GETTER) \
  ZUNO_SENSOR_MULTILEVEL(ZUNO_SENSOR_MULTILEVEL_TYPE_GENERAL_PURPOSE_VALUE, SENSOR_MULTILEVEL_SCALE_PERCENTAGE_VALUE, SENSOR_MULTILEVEL_SIZE_ONE_BYTE, SENSOR_MULTILEVEL_PRECISION_ZERO_DECIMALS, GETTER)

ZUNO_ENABLE(
    MODERN_MULTICHANNEL // No clusters, the first channel is mapped to NIF only
);

// need to use this due to ZUNO preprocessor behaviour

byte getDisplayBrightness1() { return getDisplayBrightness(); }
void setDisplayBrightness1(byte value) { return setDisplayBrightness(value); }

word getTemperature1() { return getTemperature(); }
word getHumidity1() { return getHumidity(); }
word getCO21() { return getCO2(); }
byte getTVOCPercent1() { return getTVOCPercent(); }

byte getDisplayNightMode1() { return getDisplayNightMode(); }
void setNightMode1(byte newValue) { setDisplayNightMode(newValue); }

ZUNO_SETUP_CHANNELS(
    ZUNO_SWITCH_MULTILEVEL(getDisplayBrightness1, setDisplayBrightness1),
    ZUNO_SWITCH_BINARY(getDisplayNightMode1, setNightMode1),
    ZUNO_SENSOR_MULTILEVEL_TEMPERATURE_2(getTemperature1),
    ZUNO_SENSOR_MULTILEVEL_HUMIDITY_2(getHumidity1),
    ZUNO_SENSOR_MULTILEVEL_CO2_LEVEL_2(getCO21),
    ZUNO_SENSOR_MULTILEVEL_GP_VOC_PERCENT(getTVOCPercent1),
    ZUNO_SENSOR_MULTILEVEL_GP_PM2_5_LEVEL(s_pm25));

ZUNO_SETUP_CONFIGPARAMETERS(
    ZUNO_CONFIG_PARAMETER("Temperature and humidity update period (sec)", 30, 86400, 1800),
    ZUNO_CONFIG_PARAMETER_1B("Temperature update threshold", 1, 255, 2),
    ZUNO_CONFIG_PARAMETER_1B("Humidity update threshold", 1, 255, 5),
    ZUNO_CONFIG_PARAMETER_1B("Temperature correction (deg * 10 + 100)", 0, 200, 100),
    ZUNO_CONFIG_PARAMETER_1B("Humidity correction (% * 10 + 100)", 0, 200, 100),
    ZUNO_CONFIG_PARAMETER_1B("Trigger CO2 calibration (set to 1)", 0, 1, 0),
    ZUNO_CONFIG_PARAMETER_1B("Auto night mode (set to 1)", 0, 1, 0),
    ZUNO_CONFIG_PARAMETER_1B("Night mode luminance (lux)", 0, 100, 10),
    ZUNO_CONFIG_PARAMETER_1B("Night mode hysteresis (lux)", 0, 50, 5));

ZUNO_SETUP_CFGPARAMETER_HANDLER(configParameterChanged2);

void updateFromCFGParams()
{
  updateTempHumFromCFGParams();
  updateDisplayFromCFGParams();
}

void configParameterChanged2(byte param, uint32_t value)
{
  if (param == CONFIG_CO2_START_CALIBRATION && value == 1)
  {
    triggerCO2Calibration();
    return;
  }

  zunoSaveCFGParam(param, value);
  updateFromCFGParams();
}

void reportUpdates(bool firstTime = false)
{
#if SERIAL_LOGS
  Serial.println("reportUpdates called");
#endif

  if (reportTempUpdates(firstTime))
    return;

  if (reportHumUpdates(firstTime))
    return;

  if (reportTVOCUpdates(firstTime))
    return;

  if (reportCO2Updates(firstTime))
    return;
}

void setup()
{

#if SERIAL_LOGS
  Serial.begin(115200);
#endif

  updateFromCFGParams();

  setupDisplay();
  setupTempHumSensor();
  setupTVOC();
  setupCO2();
  setupLuxSensor();

  updateTempHumSensor();
  updateTVOC(true); // first time
  updateCO2(true);

  reportUpdates(true); // first time

  updateDisplay();
}

void loop()
{

  updateTempHumSensor();
  updateTVOC();
  updateCO2();

  reportUpdates();

  updateDisplay();

  delay(2000);
}
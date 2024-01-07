
#include "EEPROM.h"

#include "temphum.h"
#include "tvoc.h"
#include "common.h"

#include "HardwareSerial.h"
#include "EasyNextionLibrary.h" // Include EasyNextionLibrary

EasyNex s_display(Serial1); // Create an object of EasyNex class with the name < myNex >
                            // Set as parameter the Hardware Serial you are going to use

byte s_displayBrightness = 100;
word s_co2 = 400; // ppm
word s_pm25 = 5;  // ppm

#define ZUNO_SENSOR_MULTILEVEL_TEMPERATURE_2(GETTER) ZUNO_SENSOR_MULTILEVEL(ZUNO_SENSOR_MULTILEVEL_TYPE_TEMPERATURE, SENSOR_MULTILEVEL_SCALE_CELSIUS, SENSOR_MULTILEVEL_SIZE_TWO_BYTES, SENSOR_MULTILEVEL_PRECISION_ONE_DECIMAL, GETTER)
#define ZUNO_SENSOR_MULTILEVEL_HUMIDITY_2(GETTER) ZUNO_SENSOR_MULTILEVEL(ZUNO_SENSOR_MULTILEVEL_TYPE_RELATIVE_HUMIDITY, SENSOR_MULTILEVEL_SCALE_PERCENTAGE_VALUE, SENSOR_MULTILEVEL_SIZE_TWO_BYTES, SENSOR_MULTILEVEL_PRECISION_ONE_DECIMAL, GETTER)
#define ZUNO_SENSOR_MULTILEVEL_CO2_LEVEL_2(GETTER) ZUNO_SENSOR_MULTILEVEL(ZUNO_SENSOR_MULTILEVEL_TYPE_CO2_LEVEL, SENSOR_MULTILEVEL_SCALE_PARTS_PER_MILLION, SENSOR_MULTILEVEL_SIZE_TWO_BYTES, SENSOR_MULTILEVEL_PRECISION_ZERO_DECIMALS, GETTER)
#define ZUNO_SENSOR_MULTILEVEL_PM2_5_LEVEL(GETTER) ZUNO_SENSOR_MULTILEVEL(ZUNO_SENSOR_MULTILEVEL_TYPE_GENERAL_PURPOSE_VALUE, SENSOR_MULTILEVEL_SCALE_PARTS_PER_MILLION, SENSOR_MULTILEVEL_SIZE_TWO_BYTES, SENSOR_MULTILEVEL_PRECISION_ZERO_DECIMALS, GETTER)
#define ZUNO_SENSOR_MULTILEVEL_VOLATILE_ORGANIC_COMPOUND(GETTER) ZUNO_SENSOR_MULTILEVEL(ZUNO_SENSOR_MULTILEVEL_TYPE_VOLATILE_ORGANIC_COMPOUND, SENSOR_MULTILEVEL_SCALE_DIMENSIONLESS_VALUE, SENSOR_MULTILEVEL_SIZE_ONE_BYTE, SENSOR_MULTILEVEL_PRECISION_ZERO_DECIMALS, GETTER)

ZUNO_ENABLE(
    MODERN_MULTICHANNEL // No clusters, the first channel is mapped to NIF only
);

ZUNO_SETUP_CHANNELS(
    ZUNO_SWITCH_MULTILEVEL(getDisplayBrightness, setDisplayBrightness),
    ZUNO_SENSOR_MULTILEVEL_TEMPERATURE_2(getTemperature),
    ZUNO_SENSOR_MULTILEVEL_HUMIDITY_2(getHumidity),
    ZUNO_SENSOR_MULTILEVEL_CO2_LEVEL_2(s_co2),
    ZUNO_SENSOR_MULTILEVEL_VOLATILE_ORGANIC_COMPOUND(getTVOC),
    ZUNO_SENSOR_MULTILEVEL_PM2_5_LEVEL(s_pm25));

ZUNO_SETUP_CONFIGPARAMETERS(
    ZUNO_CONFIG_PARAMETER("Temperature and humidity update period (sec)", 30, 86400, 1800),
    ZUNO_CONFIG_PARAMETER_1B("Temperature update threshold", 1, 255, 2),
    ZUNO_CONFIG_PARAMETER_1B("Humidity update threshold", 1, 255, 5),
    ZUNO_CONFIG_PARAMETER_1B("Temperature correction (deg * 10 + 100)", 0, 200, 100),
    ZUNO_CONFIG_PARAMETER_1B("Humidity correction (% * 10 + 100)", 0, 200, 100));

ZUNO_SETUP_CFGPARAMETER_HANDLER(configParameterChanged2);

void updateFromCFGParams()
{
  updateTempHumFromCFGParams();
}

void configParameterChanged2(byte param, uint32_t value)
{
  zunoSaveCFGParam(param, value);
  updateFromCFGParams();
}

BYTE getDisplayBrightness()
{
  return s_displayBrightness;
}

void setDisplayBrightness(BYTE newValue)
{
  s_displayBrightness = newValue;
  s_display.writeNum("dim", s_displayBrightness);
}

void setupDisplay()
{
  s_display.begin(9600);
  delay(500); // Wait for Nextion to start
}

int colorFromSeverity(int severity)
{
  switch (severity)
  {
  case 0:
    return 1469; // blue
  case 1:
    return 38592; // green
  case 2:
    return 65248; // yellow
  case 3:
    return 64512; // amber
  case 4:
    return 63495; // red
  default:
    return 0;
  }
}

int temperatureToSeverity(int temp)
{
  if (temp < 23)
    return 0; // blue
  if (temp < 25)
    return 1; // green
  if (temp < 27)
    return 2; // yellow
  return 4;   // red, no amber
}

int humidityToSeverity(int humidity)
{
  if (humidity < 30)
    return 4; // red
  if (humidity < 40)
    return 2; // yellow
  if (humidity < 60)
    return 1; // green
  return 2;
}

int tvocToSeverity(int tvoc)
{
  if (tvoc <= 1)
    return 1;
  if (tvoc <= 4)
    return 2;
  if (tvoc <= 7)
    return 3;
  return 4;
}

void updateTemperatureDisplay()
{
  s_display.writeStr("vis temp_disp,1");
  s_display.writeNum("temp_disp.val", getTemperature());
  s_display.writeNum("temp_disp.bco", colorFromSeverity(temperatureToSeverity(getTemperature() / 10)));
}

void updateHumidityDisplay()
{
  s_display.writeStr("vis hum_disp,1");
  s_display.writeNum("hum_disp.val", getHumidity());
  s_display.writeNum("hum_disp.bco", colorFromSeverity(humidityToSeverity(getHumidity() / 10)));
}

void updateTVOCDisplay()
{
  s_display.writeStr("vis co_disp,1");
  s_display.writeNum("co_disp.val", getTVOC());
  s_display.writeNum("co_disp.bco", colorFromSeverity(tvocToSeverity(getTVOC())));
}

void reportUpdates(bool firstTime = false)
{
#if SERIAL_LOGS
  Serial.println("reportUpdates called");
#endif

  updateTemperatureDisplay();
  updateHumidityDisplay();
  updateTVOCDisplay();

  if (reportTempUpdates(firstTime))
    return;

  if (reportHumUpdates(firstTime))
    return;

  if (reportTVOCUpdates(firstTime))
    return;
}

void setup()
{

#if SERIAL_LOGS
  Serial.begin(115200);
#endif

  updateFromCFGParams();

  setupDisplay();
  setupDHT();
  setupTVOC();

  updateDHT();
  updateTVOC(true);    // first time

  reportUpdates(true); // first time
}

void loop()
{

  updateDHT();
  updateTVOC();
  reportUpdates();

  delay(2000);
}
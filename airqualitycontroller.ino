
#include "EEPROM.h"

#include "co2.h"
#include "lux.h"
#include "nextion.h"
#include "pm25.h"
#include "serialex.h"
#include "temphum.h"
#include "tvoc.h"

#include "Tasks.h"
#include "common.h"

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

ZUNO_ENABLE(MODERN_MULTICHANNEL  // No clusters, the first channel is mapped to NIF only
);

SerialData serialData0(12, 14, Serial0);

SerialEx SerialEx00(0, serialData0);
SerialEx SerialEx01(1, serialData0);
SerialEx SerialEx02(2, serialData0);
SerialEx SerialEx03(3, serialData0);

const uint8_t WIRE_0_SCL_PIN = 10;
const uint8_t WIRE_0_SDA_PIN = 11;

#if defined SENSIRION_DHT_SENSOR

#include "temphumsensirion.h"
SensirionSensor sensor(Wire0);

#elif defined DHT22_SENSOR

#include "temphumdht22.h"
DHT22Sensor sensor(17);

#endif

#if defined LUX_GY302

#include "luxgy302.h"
LuxGY302 luxSensor(Wire0, 0x23);

#elif if defined LUX_TEMT6000

#include "luxtemt6000.h"
LuxTEMT6000 luxSensor(4);

#endif

TempHumTask tempHumTask(sensor);
TVOCTask tvocTask(9);
CO2Task co2Task(SerialEx00, 6);
LuxTask luxTask(luxSensor);
PM25Task pm25Task(SerialEx01);  // !!!!! which serial

DisplayTask displayTask(tempHumTask, tvocTask, co2Task, luxTask, pm25Task, Serial1);

// need to use this due to ZUNO preprocessor behaviour

byte getDisplayBrightness1()
{
    return displayTask.getBrightness();
}
void setDisplayBrightness1(byte value)
{
    return displayTask.setBrightness(value);
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
byte getTVOCPercent1()
{
    return tvocTask.getPercent();
}

byte getPM2d5()
{
    return pm25Task.getPM2d5();
}

byte getDisplayNightMode1()
{
    return displayTask.getNightMode();
}

void setNightMode1(byte newValue)
{
    displayTask.setNightMode(newValue);
}

ZUNO_SETUP_CHANNELS(ZUNO_SWITCH_MULTILEVEL(getDisplayBrightness1, setDisplayBrightness1),
                    ZUNO_SWITCH_BINARY(getDisplayNightMode1, setNightMode1),
                    ZUNO_SENSOR_MULTILEVEL_TEMPERATURE_2(getTemperature1),
                    ZUNO_SENSOR_MULTILEVEL_HUMIDITY_2(getHumidity1), ZUNO_SENSOR_MULTILEVEL_CO2_LEVEL_2(getCO21),
                    ZUNO_SENSOR_MULTILEVEL_GP_VOC_PERCENT(getTVOCPercent1),
                    ZUNO_SENSOR_MULTILEVEL_GP_PM2_5_LEVEL(getPM2d5));

ZUNO_SETUP_CONFIGPARAMETERS(ZUNO_CONFIG_PARAMETER("Temperature and humidity update period (sec)", 30, 86400, 1800),
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
    tempHumTask.updateTempHumFromCFGParams();
    displayTask.updateFromCFGParams();
    co2Task.updateFromCFGParams();
}

void configParameterChanged2(byte param, uint32_t value)
{
    zunoSaveCFGParam(param, value);
    updateFromCFGParams();
}

void setup()
{
#if SERIAL_LOGS
    Serial.begin(115200);
#endif

    // i2c init with Wire0

    Serial.print("Main: wire init");

    int result = Wire.begin(0, WIRE_0_SCL_PIN, WIRE_0_SDA_PIN);
#if SERIAL_LOGS
    Serial.print("SHT: wire init ");
    Serial.print(result);
#endif

    updateFromCFGParams();

    tempHumTask.setup();
    displayTask.setup();
    tvocTask.setup();
    co2Task.setup();
    luxTask.setup();
    pm25Task.setup();
}

void loop()
{
    TaskManager.update();
}
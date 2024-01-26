
#include "EEPROM.h"

#include "temphum.h"

#include "Tasks.h"
#include "common.h"
#include "configaqc1.h"

const uint8_t WIRE_0_SCL_PIN = 10;
const uint8_t WIRE_0_SDA_PIN = 11;

#if defined SENSIRION_DHT_SENSOR

#include "temphumsensirion.h"
SensirionSensor sensor(Wire0);

#elif defined DHT22_SENSOR

#include "temphumdht22.h"
DHT22Sensor sensor(17);

#endif

TempHumTask tempHumTask(sensor,
                        TempHumTask::Config{CONFIG_TEMPERATURE_HUMIDITY_INTERVAL_SEC,
                                            CONFIG_TEMPERATURE_THRESHOLD_DEGREES, CONFIG_HUMIDITY_THRESHOLD_PERCENT,
                                            CONFIG_TEMPERATURE_CORRECTION_DEGREES, CONFIG_HUMIDITY_CORRECTION_PERCENT},
                        TempHumTask::Report{CHANNEL_TEMPERATURE, CHANNEL_HUMIDITY});

// need to use this due to ZUNO preprocessor behaviour

word getTemperature1()
{
    return tempHumTask.getTemperature();
}
word getHumidity1()
{
    return tempHumTask.getHumidity();
}

ZUNO_SETUP_CHANNELS(ZUNO_SENSOR_MULTILEVEL_TEMPERATURE_2(getTemperature1),
                    ZUNO_SENSOR_MULTILEVEL_HUMIDITY_2(getHumidity1));

ZUNO_SETUP_CONFIGPARAMETERS(ZUNO_CONFIG_PARAMETER("Temperature and humidity update period (sec)", 30, 86400, 1800),
                            ZUNO_CONFIG_PARAMETER_1B("Temperature update threshold", 1, 255, 2),
                            ZUNO_CONFIG_PARAMETER_1B("Humidity update threshold", 1, 255, 5),
                            ZUNO_CONFIG_PARAMETER_1B("Temperature correction (deg * 10 + 100)", 0, 200, 100),
                            ZUNO_CONFIG_PARAMETER_1B("Humidity correction (% * 10 + 100)", 0, 200, 100));


void updateFromCFGParams()
{
    tempHumTask.updateTempHumFromCFGParams();
}

void configParameterChanged(uint8_t param, uint32_t value)
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
}

void loop()
{
    TaskManager.update();
}
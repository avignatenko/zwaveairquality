
ZUNO_ENABLE(MODERN_MULTICHANNEL MODERN_MULTICHANNEL_S2 MODERN_MULTICHANNEL_S2_ALWAYS);  // No clusters, the first channel is mapped to NIF only

#include "configaqc1.h"

#include "EEPROM.h"

#include "temphum.h"

#include "Tasks.h"
#include "common.h"

#if defined SENSIRION_DHT_SENSOR
#include "temphumsensirion.h"
#elif defined DHT22_SENSOR
#include "temphumdht22.h"
#endif

#if defined SENSIRION_DHT_SENSOR
SensirionSensor sensor(dhtWire);
#elif defined DHT22_SENSOR
DHT22Sensor sensor(DHT22_PIN);
#endif

TempHumTask tempHumTask(sensor,
                        TempHumTask::Config{0,
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


byte dummyVar = 0;
ZUNO_SETUP_CHANNELS(ZUNO_SWITCH_BINARY(dummyVar, NULL),
                    ZUNO_SENSOR_MULTILEVEL_TEMPERATURE_2(getTemperature1),
                    ZUNO_SENSOR_MULTILEVEL_HUMIDITY_2(getHumidity1));


ZUNO_SETUP_CONFIGPARAMETERS(ZUNO_CONFIG_PARAMETER_1B("Temperature update threshold", 1, 255, 2),
                            ZUNO_CONFIG_PARAMETER_1B("Humidity update threshold", 1, 255, 5),
                            ZUNO_CONFIG_PARAMETER_1B("Temperature correction (deg * 10 + 100)", 0, 200, 100),
                            ZUNO_CONFIG_PARAMETER_1B("Humidity correction (% * 10 + 100)", 0, 200, 100));

#define SLEEP_MODE SLEEP_MODE_EM4                       // You can select from SLEEP_MODE_EM2, SLEEP_MODE_EM4
ZUNO_SETUP_SLEEPING_MODE(ZUNO_SLEEPING_MODE_SLEEPING);  // It's sleeping device

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
    Serial.print("Main: wire init done: ");
    Serial.println(result);
#endif
}

void setup()
{
#if SERIAL_LOGS
    Serial.begin(115200);

    Serial.print("Main: wakeup reason:");
    Serial.println(zunoGetWakeReason(), HEX);
#endif

    // send power to SHT sensor
    pinMode(SHT_POWER_PIN, OUTPUT);
    digitalWrite(SHT_POWER_PIN, HIGH);
    delay(2); // let SHT sensor start

    // i2c init with Wire0
    setupI2C();

    updateFromCFGParams();

    tempHumTask.setup();

    zunoSendWakeUpNotification();
}

void loop()
{
    if (zunoIsSleepLocked())
    {  // Does sleep latch is locked?
        // Here we do all sleep uninterruptable logic
        // Report all the channels
        TaskManager.update();

        // turn off sht
        digitalWrite(SHT_POWER_PIN, LOW);

#if SERIAL_LOGS
        Serial.print("Going to sleep");
#endif

        zunoSendDeviceToSleep(SLEEP_MODE);  // This just says I am ready but it doesn't stop usercode momentally &
                                            // completely User sleep latch is opened
    }

}
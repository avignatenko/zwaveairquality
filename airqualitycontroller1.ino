
// ZUNO_ENABLE(MODERN_MULTICHANNEL MODERN_MULTICHANNEL_S2
//                 MODERN_MULTICHANNEL_S2_ALWAYS);  // No clusters, the first channel is mapped to NIF only

#include "configaqc1.h"

#include "EEPROM.h"
#include "common.h"
#include "temphum.h"
#include "temphumsensirion.h"

// need to use this due to ZUNO preprocessor behaviour

byte dummyVar = 0;

// ZUNO_ENABLE(LOGGING_DBG LOGGING_UART=Serial);

TempHum::StorageAddr s_addr{0, 0 + sizeof(dword)};
bool needUpdateSensors = false;

dword getTemperature()
{
    return TempHum::getTemperatureFromStorage(s_addr);
}

dword getHumidity()
{
    return TempHum::getHumidityFromStorage(s_addr);
}

ZUNO_SETUP_SLEEPING_MODE(ZUNO_SLEEPING_MODE_SLEEPING);  // It's sleeping device

ZUNO_SETUP_CHANNELS(ZUNO_SWITCH_BINARY(dummyVar, NULL), ZUNO_SENSOR_MULTILEVEL_TEMPERATURE_2(getTemperature()),
                    ZUNO_SENSOR_MULTILEVEL_HUMIDITY_2(getHumidity()));

ZUNO_SETUP_CONFIGPARAMETERS(ZUNO_CONFIG_PARAMETER_1B("Temperature update threshold", 1, 255, 2),
                            ZUNO_CONFIG_PARAMETER_1B("Humidity update threshold", 1, 255, 5),
                            ZUNO_CONFIG_PARAMETER_1B("Temperature correction (deg * 10 + 100)", 0, 200, 100),
                            ZUNO_CONFIG_PARAMETER_1B("Humidity correction (% * 10 + 100)", 0, 200, 100));

void setupI2C()
{
#if SERIAL_LOGS
    Serial.println("Main: wire init start");
#endif

    int result = Wire.begin(0, WIRE_0_SCL_PIN, WIRE_0_SDA_PIN);

#if SERIAL_LOGS
    Serial.print("Main: wire init done: ");
    Serial.println(result);
#endif
}

void initAndReport(bool firstTime = false)
{
    // send power to SHT sensor
    pinMode(SHT_POWER_PIN, OUTPUT);
    digitalWrite(SHT_POWER_PIN, HIGH);
    delay(2);  // let SHT sensor start

    // i2c init with Wire0
    setupI2C();

    SensirionSensor sensor(dhtWire);

    TempHum tempHum(sensor,
                    TempHum::Config{0, CONFIG_TEMPERATURE_THRESHOLD_DEGREES, CONFIG_HUMIDITY_THRESHOLD_PERCENT,
                                    CONFIG_TEMPERATURE_CORRECTION_DEGREES, CONFIG_HUMIDITY_CORRECTION_PERCENT},
                    TempHum::Report{CHANNEL_TEMPERATURE, CHANNEL_HUMIDITY}, &s_addr);

    tempHum.setup(false);  // this will also update values, but no report

    tempHum.updateSensorValues();

    bool reported = tempHum.reportUpdates(firstTime);
    if (reported) zunoSendWakeUpNotification();

    // turn off sht
    digitalWrite(SHT_POWER_PIN, LOW);

    // zunoSendWakeUpNotification();

    // zunoSendDeviceToSleep(SLEEP_MODE_EM4);

#if SERIAL_LOGS
    Serial.println("initAndReport: Done");
#endif
}

// core will call this function every time zuno wakes from EM2 mode
// instead of setup()
void _wakeHandler(void)
{
#if SERIAL_LOGS
    Serial.print("EM2 Handler: time: ");
    Serial.println(millis());
    Serial.print("EM2 Handler: wakeup reason:");
    Serial.println(zunoGetWakeReason(), HEX);
#endif

    if (zunoGetWakeReason() == ZUNO_WAKEUP_REASON_PIN || zunoGetWakeReason() == ZUNO_WAKEUP_REASON_WUT_EM4 ||
        zunoGetWakeReason() == ZUNO_WAKEUP_REASON_WUT_EM2 || zunoGetWakeReason() == ZUNO_WAKEUP_REASON_EXT_EM2 ||
        zunoGetWakeReason() == ZUNO_WAKEUP_REASON_EXT_EM4)
    {
        needUpdateSensors = true;
    }
}

void setup()
{
    // will not work without, why???
    Serial.begin(115200);

#if SERIAL_LOGS
    Serial.begin(115200);

    Serial.print("Setup: time: ");
    Serial.println(millis());
    Serial.print("Setup: wakeup reason:");
    Serial.println(zunoGetWakeReason(), HEX);
#endif

    if (zunoGetWakeReason() == ZUNO_WAKEUP_REASON_PIN || zunoGetWakeReason() == ZUNO_WAKEUP_REASON_WUT_EM4 ||
        zunoGetWakeReason() == ZUNO_WAKEUP_REASON_WUT_EM2 || zunoGetWakeReason() == ZUNO_WAKEUP_REASON_EXT_EM2 ||
        zunoGetWakeReason() == ZUNO_WAKEUP_REASON_EXT_EM4)
    {
        needUpdateSensors = true;
    }

    // setup handler for EM2 mode only
    zunoAttachSysHandler(ZUNO_HANDLER_WUP, 0, (void*)&_wakeHandler);
}

void loop()
{
    if (zunoIsSleepLocked())
    {
        if (needUpdateSensors)
        {
            Serial.println("Need update sensors");
            bool firstTime = (zunoGetWakeReason() == ZUNO_WAKEUP_REASON_PIN);
            initAndReport(firstTime);

            needUpdateSensors = false;
        }

#if SERIAL_LOGS
        Serial.println("loop: Device to sleep");
#endif
#define WAKEUP_CUSTOM_INTERVAL 300  // Every 300 seconds
        zunoSetCustomWUPTimer(WAKEUP_CUSTOM_INTERVAL);

        zunoSendDeviceToSleep(SLEEP_MODE_EM4);  // This just says I am ready but it doesn't stop usercode momentally &
    }
}
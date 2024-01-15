#include "temphumsensirion.h"

#ifdef SENSIRION_DHT_SENSOR

#include "SHTSensor.h"

SHTSensor sht;

word s_humidity = 0;
word s_humidityLastReported = 0;
unsigned long s_lastReportedTimeHumidity = 0;
word s_hum_threshold = 5;
word s_hum_correct = 0;

word s_temperature = 0;
word s_temperatureLastReported = 0;
unsigned long s_lastReportedTimeTemperature = 0;
word s_temp_threshold = 2;
word s_temp_correct = 0;

word s_temp_hum_interval = 60 * 20; // 20 mins default, min 30 seconds

word getTemperature()
{
    return s_temperature;
}

word getHumidity()
{
    return s_humidity;
}

void updateTempHumFromCFGParams()
{
    s_temp_hum_interval = zunoLoadCFGParam(CONFIG_TEMPERATURE_HUMIDITY_INTERVAL_SEC);
    s_temp_threshold = zunoLoadCFGParam(CONFIG_TEMPERATURE_THRESHOLD_DEGREES);
    s_hum_threshold = zunoLoadCFGParam(CONFIG_HUMIDITY_THRESHOLD_PERCENT);
    s_temp_correct = zunoLoadCFGParam(CONFIG_TEMPERATURE_CORRECTION_DEGREES);
    s_hum_correct = zunoLoadCFGParam(CONFIG_HUMIDITY_CORRECTION_PERCENT);
}

void setupTempHumSensor()
{
    Wire.begin();

    if (sht.init())
    {
#if SERIAL_LOGS
        Serial.println("SHT init success");
#endif
    }
    else
    {
#if SERIAL_LOGS
        Serial.println("SHT init failed");
#endif
    }
    sht.setAccuracy(SHTSensor::SHT_ACCURACY_HIGH); // only supported by SHT3x
}

void updateTempHumSensor()
{
    if (sht.readSample())
    {
        s_humidity = (sht.getHumidity() * 10) + (s_hum_correct - 100);
        s_temperature = (sht.getTemperature() * 10) + (s_temp_correct - 100);
    }
    else
    {
        s_humidity = -100 * 10;
        s_temperature = -100 * 10;

#if SERIAL_LOGS
        Serial.println("SHT Read Error!");
#endif
    }
}

bool reportTempUpdates(bool firstTime)
{
    unsigned long curMillis = millis();

#if SERIAL_LOGS
    Serial.print("Temp ");
    Serial.print(s_temperature);
    Serial.print(" ");
    Serial.println(s_temperatureLastReported);
#endif

    bool reportTemperature = (abs(s_temperature - s_temperatureLastReported) > s_temp_threshold);
    bool timePassedTemperature = (curMillis - s_lastReportedTimeTemperature > (unsigned long)s_temp_hum_interval * 1000);

    if (firstTime || reportTemperature || timePassedTemperature)
    {
        zunoSendReport(CHANNEL_TEMPERATURE);
        s_temperatureLastReported = s_temperature;
        s_lastReportedTimeTemperature = curMillis;

#if SERIAL_LOGS
        Serial.print("Temp update sent, because: ");
        Serial.print(reportTemperature);
        Serial.print(" ");
        Serial.print(timePassedTemperature);
        Serial.println();
#endif

        return true;
    }

    return false;
}

bool reportHumUpdates(bool firstTime)
{
    unsigned long curMillis = millis();

    bool reportHumidity = (abs(s_humidity - s_humidityLastReported) > s_hum_threshold);
    bool timePassedHumidity = (curMillis - s_lastReportedTimeHumidity > (unsigned long)s_temp_hum_interval * 1000);

    if (firstTime || reportHumidity || timePassedHumidity)
    {
        zunoSendReport(CHANNEL_HUMIDITY);
        s_humidityLastReported = s_humidity;
        s_lastReportedTimeHumidity = curMillis;

#if SERIAL_LOGS
        Serial.print("Hum update sent, because: ");
        Serial.print(reportHumidity);
        Serial.print(" ");
        Serial.print(timePassedHumidity);
        Serial.println();
#endif

        return true;
    }

    return false;
}
#endif
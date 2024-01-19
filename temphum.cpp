#include "temphum.h"

word s_humidityLastReported = 0;
unsigned long s_lastReportedTimeHumidity = 0;
word s_hum_threshold = 5;
word s_hum_correct = 0;

word s_temperatureLastReported = 0;
unsigned long s_lastReportedTimeTemperature = 0;
word s_temp_threshold = 2;
word s_temp_correct = 0;

word s_temp_hum_interval = 60 * 20; // 20 mins default, min 30 seconds

// returns temp (degrees Celcius) * 10 as two bytes
word getTemperature()
{
    return round(getTemperatureInternal() * 10) + (s_temp_correct - 100);
}

// returns humidity (percent) * 10 as two bytes
word getHumidity()
{
    return round(getHumidityInternal() * 10) + (s_hum_correct - 100);
}

bool reportTempUpdates(bool firstTime)
{
    unsigned long curMillis = millis();

#if SERIAL_LOGS
    Serial.print("Temp: ");
    Serial.print(getTemperatureInternal(), 2);
    Serial.print(" ");
    Serial.print(getTemperature());
    Serial.print(" ");
    Serial.print(s_temperatureLastReported);
    Serial.print(" ");
    Serial.println(s_temp_correct);
#endif

    bool reportTemperature = (abs(getTemperature() - s_temperatureLastReported) > s_temp_threshold);
    bool timePassedTemperature = (curMillis - s_lastReportedTimeTemperature > (unsigned long)s_temp_hum_interval * 1000);

    if (firstTime || reportTemperature || timePassedTemperature)
    {
        zunoSendReport(CHANNEL_TEMPERATURE);
        s_temperatureLastReported = getTemperature();
        s_lastReportedTimeTemperature = curMillis;

#if SERIAL_LOGS
        Serial.print("Temp: update sent, because: ");
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
#if SERIAL_LOGS
    Serial.print("Hum: ");
    Serial.print(getHumidityInternal(), 2);
    Serial.print(" ");
    Serial.print(getHumidity());
    Serial.print(" ");
    Serial.print(s_humidityLastReported);
    Serial.print(" ");
    Serial.println(s_hum_correct);
#endif

    unsigned long curMillis = millis();

    bool reportHumidity = (abs(getHumidity() - s_humidityLastReported) > s_hum_threshold);
    bool timePassedHumidity = (curMillis - s_lastReportedTimeHumidity > (unsigned long)s_temp_hum_interval * 1000);

    if (firstTime || reportHumidity || timePassedHumidity)
    {
        zunoSendReport(CHANNEL_HUMIDITY);
        s_humidityLastReported = getHumidity();
        s_lastReportedTimeHumidity = curMillis;

#if SERIAL_LOGS
        Serial.print("Hum: update sent, because: ");
        Serial.print(reportHumidity);
        Serial.print(" ");
        Serial.print(timePassedHumidity);
        Serial.println();
#endif

        return true;
    }

    return false;
}

void updateTempHumFromCFGParams()
{
    s_temp_hum_interval = zunoLoadCFGParam(CONFIG_TEMPERATURE_HUMIDITY_INTERVAL_SEC);
    s_temp_threshold = zunoLoadCFGParam(CONFIG_TEMPERATURE_THRESHOLD_DEGREES);
    s_hum_threshold = zunoLoadCFGParam(CONFIG_HUMIDITY_THRESHOLD_PERCENT);
    s_temp_correct = zunoLoadCFGParam(CONFIG_TEMPERATURE_CORRECTION_DEGREES);
    s_hum_correct = zunoLoadCFGParam(CONFIG_HUMIDITY_CORRECTION_PERCENT);
}

#include "temphum.h"

void TempHumTask::setup()
{
    sensor_.setup();
    updateInternal(true);  // first time update
}

// returns temp (degrees Celcius) * 10 as two bytes
word TempHumTask::getTemperature()
{
    return round(sensor_.getTemperatureInternal() * 10) + (s_temp_correct - 100);
}

// returns humidity (percent) * 10 as two bytes
word TempHumTask::getHumidity()
{
    return round(sensor_.getHumidityInternal() * 10) + (s_hum_correct - 100);
}

bool TempHumTask::reportTempUpdates(bool firstTime)
{
    unsigned long curMillis = millis();

#if SERIAL_LOGS
    Serial.print("Temp: ");
    Serial.print(sensor_.getTemperatureInternal(), 2);
    Serial.print(" ");
    Serial.print(getTemperature());
    Serial.print(" ");
    Serial.print(s_temperatureLastReported);
    Serial.print(" ");
    Serial.println(s_temp_correct);
#endif

    bool reportTemperature = (abs(getTemperature() - s_temperatureLastReported) > s_temp_threshold);
    bool timePassedTemperature =
        (curMillis - s_lastReportedTimeTemperature > (unsigned long)s_temp_hum_interval * 1000);

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

bool TempHumTask::reportHumUpdates(bool firstTime)
{
#if SERIAL_LOGS
    Serial.print("Hum: ");
    Serial.print(sensor_.getHumidityInternal(), 2);
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

void TempHumTask::updateTempHumFromCFGParams()
{
    s_temp_hum_interval = zunoLoadCFGParam(CONFIG_TEMPERATURE_HUMIDITY_INTERVAL_SEC);
    s_temp_threshold = zunoLoadCFGParam(CONFIG_TEMPERATURE_THRESHOLD_DEGREES);
    s_hum_threshold = zunoLoadCFGParam(CONFIG_HUMIDITY_THRESHOLD_PERCENT);
    s_temp_correct = zunoLoadCFGParam(CONFIG_TEMPERATURE_CORRECTION_DEGREES);
    s_hum_correct = zunoLoadCFGParam(CONFIG_HUMIDITY_CORRECTION_PERCENT);
}

void TempHumTask::updateInternal(bool firstTime)
{
    Serial.println("TempHum: update started");
    sensor_.update();

    reportTempUpdates(firstTime);
    reportHumUpdates(firstTime);
}

void TempHumTask::update()
{
    updateInternal();
}

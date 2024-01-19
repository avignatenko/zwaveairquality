#include "temphum.h"

TempHumTask::TempHumTask(TempHumSensor& sensor) : Task(2000), sensor_(sensor) {}

void TempHumTask::setup()
{
    sensor_.setup();
    updateInternal(true);  // first time update
}

// returns temp (degrees Celcius) * 10 as two bytes
word TempHumTask::getTemperature()
{
    return round(sensor_.getTemperatureInternal() * 10) + (tempCorrect_ - 100);
}

// returns humidity (percent) * 10 as two bytes
word TempHumTask::getHumidity()
{
    return round(sensor_.getHumidityInternal() * 10) + (humCorrect_ - 100);
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
    Serial.print(temperatureLastReported_);
    Serial.print(" ");
    Serial.println(tempCorrect_);
#endif

    bool reportTemperature = (abs(getTemperature() - temperatureLastReported_) > tempThreshold_);
    bool timePassedTemperature = (curMillis - lastReportedTimeTemperature_ > (unsigned long)tempHumInterval_ * 1000);

    if (firstTime || reportTemperature || timePassedTemperature)
    {
        zunoSendReport(CHANNEL_TEMPERATURE);
        temperatureLastReported_ = getTemperature();
        lastReportedTimeTemperature_ = curMillis;

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
    Serial.print(humidityLastReported_);
    Serial.print(" ");
    Serial.println(humCorrect_);
#endif

    unsigned long curMillis = millis();

    bool reportHumidity = (abs(getHumidity() - humidityLastReported_) > humThreshold_);
    bool timePassedHumidity = (curMillis - lastReportedTimeHumidity_ > (unsigned long)tempHumInterval_ * 1000);

    if (firstTime || reportHumidity || timePassedHumidity)
    {
        zunoSendReport(CHANNEL_HUMIDITY);
        humidityLastReported_ = getHumidity();
        lastReportedTimeHumidity_ = curMillis;

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
    tempHumInterval_ = zunoLoadCFGParam(CONFIG_TEMPERATURE_HUMIDITY_INTERVAL_SEC);
    tempThreshold_ = zunoLoadCFGParam(CONFIG_TEMPERATURE_THRESHOLD_DEGREES);
    humThreshold_ = zunoLoadCFGParam(CONFIG_HUMIDITY_THRESHOLD_PERCENT);
    tempCorrect_ = zunoLoadCFGParam(CONFIG_TEMPERATURE_CORRECTION_DEGREES);
    humCorrect_ = zunoLoadCFGParam(CONFIG_HUMIDITY_CORRECTION_PERCENT);
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

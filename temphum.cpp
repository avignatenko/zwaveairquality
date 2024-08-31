#include "temphum.h"

TempHumTask::TempHumTask(TempHumSensor& sensor, const Config& config, const Report& report)
    : Task(5000), sensor_(sensor), config_(config), report_(report)
{
}

void TempHumTask::setup()
{
    sensor_.setup();
    updateInternal(true);  // first time update
}

// returns temp (degrees Celcius) * 10 as two bytes
word TempHumTask::getTemperature()
{
    return round((sensor_.getTemperatureInternal() + (tempCorrect_ - 100) / 10.0) * 10);
}

// returns humidity (percent) * 10 as two bytes
word TempHumTask::getHumidity()
{
    return round((sensor_.getHumidityInternal() + (humCorrect_ - 100) / 5.0) * 10);
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
        zunoSendReport(report_.tempReportChannel);
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
        zunoSendReport(report_.humReportChannel);
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
#if SERIAL_LOGS
    Serial.println("TempHum: update config started");
#endif
    tempHumInterval_ = zunoLoadCFGParam(config_.tempHumIntervalChannel);
    tempThreshold_ = zunoLoadCFGParam(config_.tempThresholdChannel);
    humThreshold_ = zunoLoadCFGParam(config_.humThresholdChannel);
    tempCorrect_ = zunoLoadCFGParam(config_.tempCorrectChannel);
    humCorrect_ = zunoLoadCFGParam(config_.humCorrectChannel);
}

void TempHumTask::updateInternal(bool firstTime)
{
#if SERIAL_LOGS
    Serial.println("TempHum: update started");
#endif

    sensor_.update();

    reportTempUpdates(firstTime);
    reportHumUpdates(firstTime);
}

void TempHumTask::update()
{
    updateInternal();
}

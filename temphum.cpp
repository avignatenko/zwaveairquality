#include "temphum.h"
#include "EEPROM.h"

word TempHum::getTemperatureFromStorage(const StorageAddr& addr)
{
    dword buffer;
    EEPROM.get(addr.tempAddr, &buffer, sizeof(dword));
    return buffer;
}

word TempHum::getHumidityFromStorage(const StorageAddr& addr)
{
    dword buffer;
    EEPROM.get(addr.humAddr, &buffer, sizeof(dword));
    return buffer;
}

TempHum::TempHum(TempHumSensor& sensor, const Config& config, const Report& report, const StorageAddr* storageAddr)
    : sensor_(sensor), config_(config), report_(report), storageAddr_(storageAddr)
{
    if (storageAddr_)
    {
        temperatureLastReported_ = getTemperatureFromStorage(*storageAddr_);
        humidityLastReported_ = getHumidityFromStorage(*storageAddr_);

#if SERIAL_LOGS
        Serial.print("Read defaults from EEPRO: ");
        Serial.print(temperatureLastReported_);
        Serial.print(" ");
        Serial.println(humidityLastReported_);
#endif
    }
}

void TempHum::setup(bool firstTimeUpdate)
{
    sensor_.setup();
    updateTempHumFromCFGParams();
    if (firstTimeUpdate) updateInternal(true);  // first time update
}

// returns temp (degrees Celcius) * 10 as two bytes
word TempHum::getTemperature()
{
    return round((sensor_.getTemperatureInternal() + (tempCorrect_ - 100) / 10.0) * 10);
}

// returns humidity (percent) * 10 as two bytes
word TempHum::getHumidity()
{
    return round((sensor_.getHumidityInternal() + (humCorrect_ - 100) / 5.0) * 10);
}

bool TempHum::reportTempUpdates(bool firstTime)
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
    bool timePassedTemperature =
        tempHumInterval_ > 0 ? (curMillis - lastReportedTimeTemperature_ > (unsigned long)tempHumInterval_ * 1000)
                             : false;

    if (firstTime || reportTemperature || timePassedTemperature)
    {
        zunoSendReport(report_.tempReportChannel);
        temperatureLastReported_ = getTemperature();
        lastReportedTimeTemperature_ = curMillis;

#if SERIAL_LOGS
        Serial.print("Temp: update sent, because: ");
        Serial.print(firstTime);
        Serial.print(" ");
        Serial.print(reportTemperature);
        Serial.print(" ");
        Serial.print(timePassedTemperature);
        Serial.println();
#endif

        if (storageAddr_) EEPROM.put(storageAddr_->tempAddr, &temperatureLastReported_, sizeof(dword));
        return true;
    }

    return false;
}
bool TempHum::reportHumUpdates(bool firstTime)
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
    bool timePassedHumidity =
        tempHumInterval_ > 0 ? (curMillis - lastReportedTimeHumidity_ > (unsigned long)tempHumInterval_ * 1000) : false;

    if (firstTime || reportHumidity || timePassedHumidity)
    {
        zunoSendReport(report_.humReportChannel);
        humidityLastReported_ = getHumidity();
        lastReportedTimeHumidity_ = curMillis;

#if SERIAL_LOGS
        Serial.print("Hum: update sent, because: ");
        Serial.print(firstTime);
        Serial.print(" ");
        Serial.print(reportHumidity);
        Serial.print(" ");
        Serial.print(timePassedHumidity);
        Serial.println();
#endif

        if (storageAddr_) EEPROM.put(storageAddr_->humAddr, &humidityLastReported_, sizeof(dword));
        return true;
    }

    return false;
}

void TempHum::updateTempHumFromCFGParams()
{
#if SERIAL_LOGS
    Serial.println("TempHum: update config started");
#endif
    tempHumInterval_ = config_.tempHumIntervalChannel > 0 ? zunoLoadCFGParam(config_.tempHumIntervalChannel) : 0;
    tempThreshold_ = zunoLoadCFGParam(config_.tempThresholdChannel);
    humThreshold_ = zunoLoadCFGParam(config_.humThresholdChannel);
    tempCorrect_ = zunoLoadCFGParam(config_.tempCorrectChannel);
    humCorrect_ = zunoLoadCFGParam(config_.humCorrectChannel);
}

void TempHum::updateSensorValues()
{
    sensor_.update();
}

bool TempHum::reportUpdates(bool firstTime)
{
    bool reportedTemp = reportTempUpdates(firstTime);
    bool reportedHum = reportHumUpdates(firstTime);
    return reportedHum || reportedHum;
}

void TempHum::updateInternal(bool firstTime)
{
#if SERIAL_LOGS
    Serial.println("TempHum: update started");
#endif

    updateSensorValues();
    reportUpdates(firstTime);
}

void TempHum::update()
{
    updateInternal();
}

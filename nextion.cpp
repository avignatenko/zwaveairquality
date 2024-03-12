#include "nextion.h"

DisplayTask::DisplayTask(const Tasks& tasks, const Config& config, HardwareSerial& serial, uint8_t rxpin, uint8_t txpin)
    : Task(1000), tasks_(tasks), config_(config), display_(serial), serial_(serial), rxpin_(rxpin), txpin_(txpin)
{
}

void DisplayTask::setBrightness(byte newValue)
{
    display_.writeNum("dim", newValue);
}

byte DisplayTask::getDayBrightness()
{
    return displayDayBrightness_;
}

void DisplayTask::setDayBrightness(byte newValue)
{
    displayDayBrightness_ = newValue;
    if (!getNightMode()) setBrightness(displayDayBrightness_);
}

void DisplayTask::setDayMode()
{
    display_.writeNum("col_s0_bkg", 1469);
    display_.writeNum("col_s0_txt", 65535);
    display_.writeNum("col_s1_bkg", 38592);
    display_.writeNum("col_s1_txt", 65535);
    display_.writeNum("col_s2_bkg", 65248);
    display_.writeNum("col_s2_txt", 65535);
    display_.writeNum("col_s3_bkg", 64512);
    display_.writeNum("col_s3_txt", 65535);
    display_.writeNum("col_s4_bkg", 63495);
    display_.writeNum("col_s4_txt", 65535);
    setBrightness(displayDayBrightness_);
}

void DisplayTask::setNightMode()
{
    display_.writeNum("col_s0_bkg", 0);
    display_.writeNum("col_s0_txt", 1469);
    display_.writeNum("col_s1_bkg", 0);
    display_.writeNum("col_s1_txt", 55280);
    display_.writeNum("col_s2_bkg", 0);
    display_.writeNum("col_s2_txt", 65248);
    display_.writeNum("col_s3_bkg", 0);
    display_.writeNum("col_s3_txt", 64512);
    display_.writeNum("col_s4_bkg", 0);
    display_.writeNum("col_s4_txt", 63495);
    setBrightness(displayNightBrightness_);
}

byte DisplayTask::getNightMode()
{
    return nightMode_;
}
void DisplayTask::setNightMode(byte val)
{
    if (nightMode_ == val) return;  // no change

    if (val)
        setNightMode();
    else
        setDayMode();
    nightMode_ = val;

#if SERIAL_LOGS
    Serial.print("Nextion: Updated display night node: ");
    Serial.println(nightMode_);
#endif
}

void DisplayTask::setup()
{
    if (rxpin_ == 0 || txpin_ == 0)
    {
        display_.begin(9600);
    }
    else
    {
#if SERIAL_LOGS
        Serial.print("Nextion: Starting with custom rx, tx: ");
        Serial.print(rxpin_);
        Serial.print(" ");
        Serial.print(txpin_);
        Serial.println();
#endif
        serial_.begin(9600, SERIAL_8N1, rxpin_, txpin_);
        ;
        display_.beginLazy();
    }

    delay(500);  // Wait for Nextion to start
    setNightMode(0);
}

int temperatureToSeverity(int temp)
{
    if (temp < 22) return 0;  // blue
    if (temp < 25) return 1;  // green
    if (temp < 27) return 2;  // yellow
    return 4;                 // red, no amber
}

int humidityToSeverity(int humidity)
{
    if (humidity < 30) return 4;  // red
    if (humidity < 40) return 2;  // yellow
    if (humidity < 60) return 1;  // green
    return 2;
}

int tvocToSeverity(int tvoc)
{
    if (tvoc <= 1) return 1;
    if (tvoc <= 4) return 2;
    if (tvoc <= 7) return 3;
    return 4;
}

int co2ToSeverity(int co2)
{
    if (co2 <= 800) return 1;
    if (co2 <= 1100) return 2;
    if (co2 <= 2000) return 3;
    return 4;
}

int coToSeverity(float co)
{
    if (co < 10) return 1;
    if (co < 30) return 2;
    if (co < 100) return 3;
    return 4;
}

int pm25ToSeverity(int pm25)
{
    if (pm25 <= 12) return 1;
    if (pm25 <= 35) return 2;
    if (pm25 <= 50) return 3;
    return 4;
}

void DisplayTask::updateTemperatureDisplay()
{
    display_.writeNum("temp", tasks_.tempHumTask.getTemperature());
    display_.writeNum("temp_severity", temperatureToSeverity(tasks_.tempHumTask.getTemperature() / 10));
}

void DisplayTask::updateHumidityDisplay()
{
    display_.writeNum("hum", tasks_.tempHumTask.getHumidity());
    display_.writeNum("hum_severity", humidityToSeverity(tasks_.tempHumTask.getHumidity() / 10));
}

void DisplayTask::updateTVOCDisplay()
{
    display_.writeNum("tvoc", tasks_.tvocTask.get());
    display_.writeNum("tvoc_severity", tvocToSeverity(tasks_.tvocTask.get()));
}

void DisplayTask::updateCO2Display()
{
    display_.writeNum("co2", tasks_.co2Task.getCO2());
    display_.writeNum("co2_severity", co2ToSeverity(tasks_.co2Task.getCO2()));
}

void DisplayTask::updatePM25Display()
{
    display_.writeNum("pm25", tasks_.pm25.getPM2d5());
    display_.writeNum("pm25_severity", pm25ToSeverity(tasks_.pm25.getPM2d5()));
}

void DisplayTask::updateCODisplay()
{
    display_.writeNum("co", tasks_.co.getCO()  / 10.0);
    display_.writeNum("co_severity", coToSeverity(tasks_.co.getCO() / 10.0));
}

void DisplayTask::updateNightMode()
{
    if (!isAutoNightMode())
    {
#if SERIAL_LOGS
        Serial.println("Nextion: Auto night mode is off, skipping");
#endif

        return;
    }
    const uint16_t luminance = tasks_.lux.getLuminance();

    const uint16_t histLowerBound = nightModeLuminance_ - nightModeLuminanceHysteresis;
    const uint16_t histUpperBound = nightModeLuminance_ + nightModeLuminanceHysteresis;

    if (luminance > histLowerBound && luminance < histUpperBound) return;  // no change, hysteresis in effect

    bool isNight = (tasks_.lux.getLuminance() <= histLowerBound);
    setNightMode(isNight);
}

void DisplayTask::updateFromCFGParams()
{
    autoNightMode_ = zunoLoadCFGParam(config_.nightModeAutoChannel);
    nightModeLuminance_ = zunoLoadCFGParam(config_.nightModeLuminanceChannel);
    nightModeLuminanceHysteresis = zunoLoadCFGParam(config_.nightModeHysteresisChannel);

    if (nightModeLuminance_ < nightModeLuminanceHysteresis) nightModeLuminance_ = nightModeLuminanceHysteresis;

#if SERIAL_LOGS
    Serial.print("Nextion: Updated display params: ");
    Serial.print(autoNightMode_);
    Serial.print(" ");
    Serial.print(nightModeLuminance_);
    Serial.print(" ");
    Serial.println(nightModeLuminanceHysteresis);
#endif
}

void DisplayTask::enable(bool enable)
{
#if SERIAL_LOGS
    Serial.print("Nextion: enabled status: ");
    Serial.print(enable);
    Serial.println();
#endif

    enabled_ = enable;
    display_.writeNum("sleep", enabled_ ? 0 : 1);
}

bool DisplayTask::enabled()
{
    return enabled_;
}

void DisplayTask::update()
{
#if SERIAL_LOGS
    Serial.println("Nextion: Update started");
#endif

    updateNightMode();
    updateTemperatureDisplay();
    updateHumidityDisplay();
    updateTVOCDisplay();
    updateCO2Display();
    updatePM25Display();
    updateCODisplay();
}

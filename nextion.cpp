#include "nextion.h"



DisplayTask::DisplayTask(TempHumTask& tempHumTask, TVOCTask& tvocTask, CO2Task& co2Task, LuxTask& lux, HardwareSerial& serial)
    : Task(1000), tempHumTask_(tempHumTask), tvocTask_(tvocTask), co2Task_(co2Task), lux_(lux), display_(serial)
{
}

byte DisplayTask::getBrightness()
{
    return displayBrightness_;
}

void DisplayTask::setBrightness(byte newValue)
{
    displayBrightness_ = newValue;
    display_.writeNum("dim", displayBrightness_);
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
    setBrightness(100);
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
    setBrightness(5);
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
    display_.begin(9600);
    delay(500);  // Wait for Nextion to start
    setNightMode(0);
}

int temperatureToSeverity(int temp)
{
    if (temp < 23) return 0;  // blue
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

void DisplayTask::updateTemperatureDisplay()
{
    display_.writeNum("temp", tempHumTask_.getTemperature());
    display_.writeNum("temp_severity", temperatureToSeverity(tempHumTask_.getTemperature() / 10));
}

void DisplayTask::updateHumidityDisplay()
{
    display_.writeNum("hum", tempHumTask_.getHumidity());
    display_.writeNum("hum_severity", humidityToSeverity(tempHumTask_.getHumidity() / 10));
}

void DisplayTask::updateTVOCDisplay()
{
    display_.writeNum("tvoc", tvocTask_.get());
    display_.writeNum("tvoc_severity", tvocToSeverity(tvocTask_.get()));
}

void DisplayTask::updateCO2Display()
{
    display_.writeNum("co2", co2Task_.getCO2());
    display_.writeNum("co2_severity", co2ToSeverity(co2Task_.getCO2()));
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
    const uint16_t luminance = lux_.getLuminance();

    const uint16_t histLowerBound = nightModeLuminance_ - nightModeLuminanceHysteresis;
    const uint16_t histUpperBound = nightModeLuminance_ + nightModeLuminanceHysteresis;

    if (luminance > histLowerBound && luminance < histUpperBound) return;  // no change, hysteresis in effect

    bool isNight = (lux_.getLuminance() <= histLowerBound);
    setNightMode(isNight);
}

void DisplayTask::updateFromCFGParams()
{
    autoNightMode_ = zunoLoadCFGParam(CONFIG_AUTO_NIGHT_MODE);
    nightModeLuminance_ = zunoLoadCFGParam(CONFIG_NIGHT_MODE_LUMINANCE);
    nightModeLuminanceHysteresis = zunoLoadCFGParam(CONFIG_NIGHT_MODE_HYSTERESIS);

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
}
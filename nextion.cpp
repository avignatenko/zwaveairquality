#include "nextion.h"

#include "co2.h"
#include "lux.h"
#include "temphum.h"
#include "tvoc.h"

DisplayTask::DisplayTask(TempHumTask& tempHumTask, HardwareSerial& serial)
    : Task(1000), tempHumTask_(tempHumTask), display_(serial)
{
}

byte DisplayTask::getDisplayBrightness()
{
    return s_displayBrightness;
}

void DisplayTask::setDisplayBrightness(byte newValue)
{
    s_displayBrightness = newValue;
    display_.writeNum("dim", s_displayBrightness);
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
    setDisplayBrightness(100);
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
    setDisplayBrightness(5);
}

byte DisplayTask::getDisplayNightMode()
{
    return s_nightMode;
}
void DisplayTask::setDisplayNightMode(byte val)
{
    if (s_nightMode == val) return;  // no change

    if (val)
        setNightMode();
    else
        setDayMode();
    s_nightMode = val;

#if SERIAL_LOGS
    Serial.print("Nextion: Updated display night node: ");
    Serial.println(s_nightMode);
#endif
}

void DisplayTask::setup()
{
    display_.begin(9600);
    delay(500);  // Wait for Nextion to start
    setDisplayNightMode(0);
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
    display_.writeNum("tvoc", getTVOC());
    display_.writeNum("tvoc_severity", tvocToSeverity(getTVOC()));
}

void DisplayTask::updateCO2Display()
{
    display_.writeNum("co2", getCO2());
    display_.writeNum("co2_severity", co2ToSeverity(getCO2()));
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
    const uint16_t luminance = getLuminance();

    const uint16_t histLowerBound = s_night_mode_luminance - s_night_mode_luminance_hysteresis;
    const uint16_t histUpperBound = s_night_mode_luminance + s_night_mode_luminance_hysteresis;

    if (luminance > histLowerBound && luminance < histUpperBound) return;  // no change, hysteresis in effect

    bool isNight = (getLuminance() <= histLowerBound);
    setDisplayNightMode(isNight);
}

void DisplayTask::updateDisplayFromCFGParams()
{
    s_auto_night_mode = zunoLoadCFGParam(CONFIG_AUTO_NIGHT_MODE);
    s_night_mode_luminance = zunoLoadCFGParam(CONFIG_NIGHT_MODE_LUMINANCE);
    s_night_mode_luminance_hysteresis = zunoLoadCFGParam(CONFIG_NIGHT_MODE_HYSTERESIS);

    if (s_night_mode_luminance < s_night_mode_luminance_hysteresis)
        s_night_mode_luminance = s_night_mode_luminance_hysteresis;

#if SERIAL_LOGS
    Serial.print("Nextion: Updated display params: ");
    Serial.print(s_auto_night_mode);
    Serial.print(" ");
    Serial.print(s_night_mode_luminance);
    Serial.print(" ");
    Serial.println(s_night_mode_luminance_hysteresis);
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
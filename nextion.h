#pragma once

#include "common.h"
#include "tasks.h"

#include "temphum.h"

#include "EasyNextionLibrary.h"
#include "HardwareSerial.h"

class TempHumTask;

class DisplayTask : public Task
{
public:
    DisplayTask(TempHumTask& tempHumTask, HardwareSerial& serial);

    byte getDisplayBrightness();
    void setDisplayBrightness(byte newValue);
    byte getDisplayNightMode();
    void setDisplayNightMode(byte val);
    void updateDisplayFromCFGParams();

    void setup();

protected:
    void update();

private:
    bool isAutoNightMode() { return s_auto_night_mode; }
    uint16_t getNightModeLuminance() { return s_night_mode_luminance; }

    void setDayMode();
    void setNightMode();

    void updateTemperatureDisplay();
    void updateHumidityDisplay();
    void updateTVOCDisplay();
    void updateCO2Display();
    void updateNightMode();
private:
    TempHumTask& tempHumTask_;
    EasyNex display_;

    byte s_displayBrightness = 100;
    bool s_nightMode = false;

    bool s_auto_night_mode = false;
    uint16_t s_night_mode_luminance = 0;
    uint16_t s_night_mode_luminance_hysteresis =
        5;  // day mode if lum >= s_night_mode_luminance + s_night_mode_luminance_hysteresis
            // night mode if lum <= s_night_mode_luminance - s_night_mode_luminance_hysteresis
};

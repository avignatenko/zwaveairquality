#pragma once

#include "common.h"
#include "tasks.h"

#include "temphum.h"
#include "tvoc.h"
#include "co2.h"

#include "EasyNextionLibrary.h"
#include "HardwareSerial.h"

class TempHumTask;

class DisplayTask : public Task
{
public:
    DisplayTask(TempHumTask& tempHumTask, TVOCTask& tvocTask, CO2Task& co2Task, HardwareSerial& serial);

    byte getBrightness();
    void setBrightness(byte newValue);
    byte getNightMode();
    void setNightMode(byte val);
    void updateFromCFGParams();

    void setup();

protected:
    void update();

private:
    bool isAutoNightMode() { return autoNightMode_; }
    uint16_t getNightModeLuminance() { return nightModeLuminance_; }

    void setDayMode();
    void setNightMode();

    void updateTemperatureDisplay();
    void updateHumidityDisplay();
    void updateTVOCDisplay();
    void updateCO2Display();
    void updateNightMode();

private:
    TempHumTask& tempHumTask_;
    TVOCTask& tvocTask_;
    CO2Task& co2Task_;

    EasyNex display_;

    byte displayBrightness_ = 100;
    bool nightMode_ = false;

    bool autoNightMode_ = false;
    uint16_t nightModeLuminance_ = 0;
    uint16_t nightModeLuminanceHysteresis =
        5;  // day mode if lum >= nightModeLuminance_ + nightModeLuminanceHysteresis
            // night mode if lum <= nightModeLuminance_ - nightModeLuminanceHysteresis
};

#pragma once

#include "common.h"
#include "tasks.h"

#include "co.h"
#include "co2.h"
#include "lux.h"
#include "pm25.h"
#include "temphum.h"
#include "tvoc.h"

#include "EasyNextionLibrary.h"
#include "HardwareSerial.h"

class TempHumTask;

class DisplayTask : public Task
{
public:
    struct Tasks
    {
        TempHumTask& tempHumTask;
        TVOCTask& tvocTask;
        CO2Task& co2Task;
        LuxTask& lux;
        PM25Task& pm25;
        COTask& co;
    };

    struct Config
    {
        uint8_t nightModeAutoChannel;
        uint8_t nightModeLuminanceChannel;
        uint8_t nightModeHysteresisChannel;
    };

    DisplayTask(const Tasks& tasks, const Config& config, HardwareSerial& serial, uint8_t rxpin = 0, uint8_t txpin = 0);

    byte getDayBrightness();
    void setDayBrightness(byte newValue);

    byte getNightMode();
    void setNightMode(byte val);
    void updateFromCFGParams();

    // enable/display display (enabled by default)
    void enable(bool enable);
    bool enabled();

    void setup();

protected:
    void update() override;

private:
    void setBrightness(byte newValue);

    bool isAutoNightMode() { return autoNightMode_; }
    uint16_t getNightModeLuminance() { return nightModeLuminance_; }

    void setDayMode();
    void setNightMode();

    void updateTemperatureDisplay();
    void updateHumidityDisplay();
    void updateTVOCDisplay();
    void updateCO2Display();
    void updatePM25Display();
    void updateCODisplay();
    void updateNightMode();

private:
    Tasks tasks_;
    Config config_;

    bool enabled_ = true;

    EasyNex display_;
    HardwareSerial& serial_;
    uint8_t rxpin_;
    uint8_t txpin_;

    byte displayDayBrightness_ = 100;
    const byte displayNightBrightness_ = 5;
    bool nightMode_ = false;

    bool autoNightMode_ = false;
    uint16_t nightModeLuminance_ = 0;
    uint16_t nightModeLuminanceHysteresis =
        5;  // day mode if lum >= nightModeLuminance_ + nightModeLuminanceHysteresis
            // night mode if lum <= nightModeLuminance_ - nightModeLuminanceHysteresis
};

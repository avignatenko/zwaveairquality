#include "nextion.h"

#include "HardwareSerial.h"
#include "EasyNextionLibrary.h" // Include EasyNextionLibrary

#include "temphum.h"
#include "tvoc.h"
#include "co2.h"

EasyNex s_display(Serial1); // Create an object of EasyNex class with the name < myNex >
                            // Set as parameter the Hardware Serial you are going to use

byte s_displayBrightness = 100;

byte getDisplayBrightness()
{
    return s_displayBrightness;
}

void setDisplayBrightness(byte newValue)
{
    s_displayBrightness = newValue;
    s_display.writeNum("dim", s_displayBrightness);
}

void setupDisplay()
{
    s_display.begin(9600);
    delay(500); // Wait for Nextion to start

    // setup colors
}

int colorFromSeverity(int severity)
{
    switch (severity)
    {
    case 0:
        return 1469; // blue
    case 1:
        return 38592; // green
    case 2:
        return 65248; // yellow
    case 3:
        return 64512; // amber
    case 4:
        return 63495; // red
    default:
        return 0;
    }
}

int temperatureToSeverity(int temp)
{
    if (temp < 23)
        return 0; // blue
    if (temp < 25)
        return 1; // green
    if (temp < 27)
        return 2; // yellow
    return 4;     // red, no amber
}

int humidityToSeverity(int humidity)
{
    if (humidity < 30)
        return 4; // red
    if (humidity < 40)
        return 2; // yellow
    if (humidity < 60)
        return 1; // green
    return 2;
}

int tvocToSeverity(int tvoc)
{
    if (tvoc <= 1)
        return 1;
    if (tvoc <= 4)
        return 2;
    if (tvoc <= 7)
        return 3;
    return 4;
}

int co2ToSeverity(int co2)
{
    if (co2 <= 800)
        return 1;
    if (co2 <= 1100)
        return 2;
    if (co2 <= 2000)
        return 3;
    return 4;
}

void updateTemperatureDisplay()
{
    s_display.writeNum("temp", getTemperature());
    s_display.writeNum("temp_severity", temperatureToSeverity(getTemperature() / 10));
}

void updateHumidityDisplay()
{
    s_display.writeNum("hum", getHumidity());
    s_display.writeNum("hum_severity", humidityToSeverity(getHumidity() / 10));
}

void updateTVOCDisplay()
{
    s_display.writeNum("tvoc", getTVOCPercent());
    s_display.writeNum("tvoc_severity", tvocToSeverity(getTVOC()));
}

void updateCO2Display()
{
    s_display.writeNum("co2", getCO2());
    s_display.writeNum("co2_severity", co2ToSeverity(getCO2()));
}

void updateDisplay()
{
    updateTemperatureDisplay();
    updateHumidityDisplay();
    updateTVOCDisplay();
    updateCO2Display();
}
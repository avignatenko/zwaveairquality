#include "nextion.h"
#include "common.h"

#include "HardwareSerial.h"
#include "EasyNextionLibrary.h" // Include EasyNextionLibrary

EasyNex s_display(Serial1); // Create an object of EasyNex class with the name < myNex >
                            // Set as parameter the Hardware Serial you are going to use

byte s_displayBrightness = 100;

BYTE getDisplayBrightness()
{
    return s_displayBrightness;
}

void setDisplayBrightness(BYTE newValue)
{
    s_displayBrightness = newValue;
    s_display.writeNum("dim", s_displayBrightness);
}

void setupDisplay()
{
    s_display.begin(9600);
    delay(500); // Wait for Nextion to start
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

void updateTemperatureDisplay()
{
    s_display.writeStr("vis temp_disp,1");
    s_display.writeNum("temp_disp.val", getTemperature());
    s_display.writeNum("temp_disp.bco", colorFromSeverity(temperatureToSeverity(getTemperature() / 10)));
}

void updateHumidityDisplay()
{
    s_display.writeStr("vis hum_disp,1");
    s_display.writeNum("hum_disp.val", getHumidity());
    s_display.writeNum("hum_disp.bco", colorFromSeverity(humidityToSeverity(getHumidity() / 10)));
}

void updateTVOCDisplay()
{
    s_display.writeStr("vis co_disp,1");
    s_display.writeNum("co_disp.val", getTVOC());
    s_display.writeNum("co_disp.bco", colorFromSeverity(tvocToSeverity(getTVOC())));
}

void updateDisplay()
{
    updateTemperatureDisplay();
    updateHumidityDisplay();
    updateTVOCDisplay();
}
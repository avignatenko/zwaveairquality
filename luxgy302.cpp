#include "luxgy302.h"
#include "BH1750.h"

LuxGY302::LuxGY302(TwoWire& wire, uint8_t addr) : wire_(wire), addr_(addr) {}

void LuxGY302::setup()
{
    // begin returns a boolean that can be used to detect setup problems.
    bool result = sensor_.begin(BH1750::CONTINUOUS_HIGH_RES_MODE_2, addr_, &wire_);

#if SERIAL_LOGS
    if (result)
        Serial.println("BH1750: begin OK");
    else
        Serial.println("BH1750: Error initialising");
#endif
}

float LuxGY302::getLuminance()
{
    return luminance_;
}

void LuxGY302::update()
{
    if (sensor_.measurementReady())
    {
        float lux = sensor_.readLightLevel();
#if SERIAL_LOGS
        Serial.print("BH1750 result: ");
        Serial.print(lux);
        Serial.println(" lx");
#endif
        luminance_ = lux;
    }
}
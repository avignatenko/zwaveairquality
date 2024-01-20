#include "luxgy302.h"

#include "ZUNO_BH1750.h"

LuxGY302::LuxGY302(TwoWire& wire) : wire_(wire) {}

void LuxGY302::setup()
{
    sensor_.begin();
}

float LuxGY302::getLuminance()
{
    return luminance_;
}

void LuxGY302::update()
{
   
    uint16_t lux = sensor_.readLightLevel();
#if SERIAL_LOGS
    Serial.print("Lux: ");
    Serial.println(lux);
#endif

    luminance_ = lux;
}
#include "lux.h"

LuxTask::LuxTask(LuxSensor& sensor) : Task(10000), sensor_(sensor) {}

void LuxTask::setup()
{
    sensor_.setup();
}

float LuxTask::getLuminance()
{
    return sensor_.getLuminance();
}

void LuxTask::update()
{
    sensor_.update();
}

#include "co2.h"

CO2Task::CO2Task(SerialEx& serial, uint8_t pinHd, uint8_t reportChannel, uint8_t configChannel)
    : Task(2000), serial_(serial), pinHd_(pinHd), configChannel_(configChannel), reportChannel_(reportChannel)
{
}

word CO2Task::getCO2()
{
    return co2_;
}

void CO2Task::updateFromCFGParams()
{
    int value = zunoLoadCFGParam(configChannel_);
    if (value == 1 && !inCalibration_)
    {
        triggerCalibration();
        zunoSaveCFGParam(configChannel_, 0);
    }
}

void CO2Task::setup()
{
    serial_.begin(9600);

    // set hd pin
    pinMode(pinHd_, OUTPUT);
    digitalWrite(pinHd_, HIGH);

    // enable pre-heat
    preheat_ = true;
    preheatStartedTime_ = millis();

    // disable auto-calibration (todo: move to properties)
    enableAutoCalibration(false);

    updateInternal(true);
}

char getCheckSum(uint8_t* packet)
{
    uint8_t checksum = 0;

    for (uint8_t i = 1; i < 8; i++) checksum += packet[i];

    checksum = 0xff - checksum;
    checksum = checksum + 1;
    return checksum;
}

void CO2Task::triggerCalibration()
{
#if SERIAL_LOGS
    Serial.println("CO2: Trigger calibration entered");
#endif

    if (inCalibration_) return;

    // set hd pin to LOW and start time
    digitalWrite(pinHd_, LOW);

    calibrationStartedTime_ = millis();

    inCalibration_ = true;

#if SERIAL_LOGS
    Serial.println("CO2: Calibration started");
#endif
}

// returns true if still in calibration mode
bool CO2Task::updateCalibration()
{
    if (!inCalibration_) return false;

    unsigned long delta = millis() - calibrationStartedTime_;

#if SERIAL_LOGS
    Serial.print("CO2: Calibration update: ");
    Serial.println(delta);
#endif

    if (delta > CALIBRATION_TIME)
    {
#if SERIAL_LOGS
        Serial.print("CO2: Calibration finished ");
#endif
        // set hd pin to HIGH and stop timer
        digitalWrite(pinHd_, HIGH);
        inCalibration_ = false;
        calibrationStartedTime_ = 0;

        return false;
    }

    return true;
}

// returns true if still in pre-heat
bool CO2Task::updatePreheat()
{
    if (!preheat_) return false;

    dword curPreheatDuration = millis() - preheatStartedTime_;

    if (curPreheatDuration > PREHEAT_DURATION)
    {
#if SERIAL_LOGS
        Serial.println("CO2: Preheat finished");
#endif
        preheat_ = false;
        preheatStartedTime_ = 0;

        return false;
    }

#if SERIAL_LOGS
    Serial.println("CO2: No CO2 value, in preheat");
    Serial.print("CO2: Preheat remaining: ");
    Serial.println(PREHEAT_DURATION - curPreheatDuration);
#endif

    return true;  // still in pre-heat
}

void CO2Task::sendCommand(uint8_t command, uint8_t arg)
{
    HardwareSerial& serial = serial_.captureSerial();

    // read everything which might stay there
    while (serial.available()) serial.read();

    // send data request
    uint8_t bufferOut[9] = {0xFF, 0x01, command, arg, 0x00, 0x00, 0x00, 0x00, 0x00};
    bufferOut[8] = getCheckSum(bufferOut);
    serial.write(bufferOut, 9);
}

CO2Task::Reply CO2Task::readReply(uint8_t command, uint8_t bufferOut[6])
{
    HardwareSerial& serial = serial_.captureSerial();

    // read co2 concentraction reply
    uint8_t bufferIn[9];
    byte read = serial.readBytes(bufferIn, 9);
    if (read == 0) return REPLY_NO_ANSWER;

    if (read != 9)
    {
        // read everything else
        while (serial.available()) serial.read();

        return REPLY_WRONG_LENGTH;
    }

    // parse answer

    bool ok = true;
    ok = ok && (bufferIn[0] == 0xFF);
    ok = ok && (bufferIn[1] == command);

    if (!ok) return REPLY_WRONG_ID;

    ok = ok && (getCheckSum(bufferIn) == bufferIn[8]);

    if (!ok) return REPLY_WRONG_CHECKSUM;

    memcpy(bufferOut, bufferIn + 2, 6);

    // read everything else just in case
    while (serial.available()) serial.read();

    return REPLY_OK;
}

void CO2Task::update(bool firstTime)
{
    // return if still in pre-heat
    if (updatePreheat())
    {
        co2_ = 0;
        return;
    }

    // return if in calibration process
    if (updateCalibration())
    {
        // return, but keep co2 as before
        return;
    }

    // Read CO2 concentration
    const uint8_t READ_CO2_COMMAND = 0x86;
    sendCommand(READ_CO2_COMMAND);

    // read co2 concentraction reply

    uint8_t bufferIn[6];
    Reply reply = readReply(READ_CO2_COMMAND, bufferIn);

    if (reply != REPLY_OK)
    {
#if SERIAL_LOGS
        Serial.print("CO2: Error reading CO2: ");
        Serial.print(reply);
        Serial.println();
#endif
        co2_ = (word)reply;
        return;
    }

    byte c1 = bufferIn[0];
    byte c2 = bufferIn[1];

    word value = (word)c1 * 256 + (word)c2;

    co2_ = value;
}

void CO2Task::enableAutoCalibration(bool enable)
{
    const uint8_t ENABLE_AUTO_CALIBRATION_COMMAND = 0x79;
    sendCommand(ENABLE_AUTO_CALIBRATION_COMMAND, enable ? 0xA0 : 0x00);

#if SERIAL_LOGS
    Serial.print("CO2: Sent auto calibration enable: ");
    Serial.print(enable);
    Serial.println();
#endif
}

bool CO2Task::reportUpdates(bool firstTime)
{
#if SERIAL_LOGS
    Serial.print("CO2: ");
    Serial.print(co2_);
    Serial.print(" ");
    Serial.println();
#endif

    if (co2_ <= 100) return false;  // wrong value, let's skip it

    unsigned long curMillis = millis();

    bool reportCO2 = (abs(co2_ - co2LastReported_) > co2Threshold_);
    bool timePassedCO2 = (curMillis - lastReportedTimeCO2_ > (unsigned long)co2Interval_ * 1000);

    if (firstTime || reportCO2 || timePassedCO2)
    {
        zunoSendReport(reportChannel_);
        co2LastReported_ = co2_;
        lastReportedTimeCO2_ = curMillis;

#if SERIAL_LOGS
        Serial.print("CO2: update sent, because: ");
        Serial.print(reportCO2);
        Serial.print(" ");
        Serial.print(timePassedCO2);
        Serial.println();
#endif

        return true;
    }

    return false;
}

void CO2Task::updateInternal(bool firstTime)
{
    update(firstTime);
    reportUpdates(firstTime);
}
void CO2Task::update()
{
    updateInternal();
}
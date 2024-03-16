#pragma once

const uint8_t WINSEN_REQUEST_DATA_COMMAND = 0x86;
const uint8_t WINSEN_SET_QAMODE_COMMAND = 0x78;
const uint8_t WINSEN_SET_QAMODE_COMMAND_ARG = 0x41;

inline char getCheckSum(uint8_t* packet)
{
    uint8_t checksum = 0;

    for (uint8_t i = 1; i < 8; i++) checksum += packet[i];

    checksum = 0xff - checksum;
    checksum = checksum + 1;
    return checksum;
}

inline void sendCommand(SerialEx& serialEx, uint8_t command = 0x0, uint8_t arg = 0x0)
{
    HardwareSerial& serial = serialEx.captureSerial();

    // read everything which might stay there
    while (serial.available()) serial.read();

    // send data request
    uint8_t bufferOut[9] = {0xFF, 0x01, command, arg, 0x00, 0x00, 0x00, 0x00, 0x00};
    bufferOut[8] = getCheckSum(bufferOut);
    serial.write(bufferOut, 9);
}

enum WinsenReply
{
    REPLY_OK = 0,
    REPLY_NO_ANSWER,
    REPLY_WRONG_LENGTH,
    REPLY_WRONG_ID,
    REPLY_WRONG_CHECKSUM,
    REPLY_SENSOR_FAILED
};

inline WinsenReply readReply(SerialEx& serialEx, uint8_t command, uint8_t bufferOut[6])
{
    HardwareSerial& serial = serialEx.captureSerial();

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

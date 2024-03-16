#pragma once

inline char getCheckSum(uint8_t* packet)
{
    uint8_t checksum = 0;

    for (uint8_t i = 1; i < 8; i++) checksum += packet[i];

    checksum = 0xff - checksum;
    checksum = checksum + 1;
    return checksum;
}

inline void sendCommand(SerialEx& serialEx, uint8_t command, uint8_t arg)
{
    HardwareSerial& serial = serialEx.captureSerial();

    // read everything which might stay there
    while (serial.available()) serial.read();

    // send data request
    uint8_t bufferOut[9] = {0xFF, 0x01, command, arg, 0x00, 0x00, 0x00, 0x00, 0x00};
    bufferOut[8] = getCheckSum(bufferOut);
    serial.write(bufferOut, 9);
}
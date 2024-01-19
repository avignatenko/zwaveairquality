#include "pm25.h"

PM25Task::PM25Task(HardwareSerial& serial) : Task(2000), serial_(serial) {}

#if 0
uint8_t PM25_TX = 12;
uint8_t PM25_RX = 14;

SoftwareSerial1 s_pm2_5_Serial(PM25_TX, PM25_RX);

unsigned char FucCheckSum(unsigned char *i, unsigned char ln) {
  unsigned char j, tempq = 0; i += 1;
  for (j = 0; j < (ln - 2); j++)
  {
    tempq += *i;
    i++;
  }
  tempq = (~tempq) + 1;
  return (tempq);
}

void requestData()
{
  byte buffer[9] = { 0xff, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
  for (int i = 0; i < 9; ++i) s_pm2_5_Serial.write(buffer[i]);
  
}

void receiveData()
{
  uint8_t buffer[9];
  int read = s_pm2_5_Serial.readBytes(buffer, 9);

#if SERIAL_LOGS
  Serial.print("PM2.5 Read bytes: ");
  Serial.println(read);


  for (int i = 0; i < 9; ++i)
  {
    Serial.print(buffer[i]);
    Serial.print(" ");
  }
  Serial.println();
#endif

  int check = FucCheckSum(buffer, 9);

#if SERIAL_LOGS
  Serial.print("PM2.5 Check: ");
  Serial.println(check);
#endif
}


void setQAMode()
{
  byte buffer[9] = { 0xff, 0x01, 0x78, 0x41, 0x00, 0x00, 0x00, 0x00, 0x46};
  for (int i = 0; i < 9; ++i) s_pm2_5_Serial.write(buffer[i]);
}

#endif
void PM25Task::setup()
{
#if SERIAL_LOGS
    Serial.println("PM2.5: setup start: ");
#endif
    // s_pm2_5_Serial.begin(9600);
    //  setQAMode();

#if SERIAL_LOGS
    Serial.println("PM2.5: setup end");
#endif
}
uint16_t PM25Task::getPM2_5()
{
    return pm2_5_;
}

void PM25Task::update()
{
    pm2_5_ += 1;
    if (pm2_5_ > 100) pm2_5_ = 0;

    // requestData();
    // receiveData();
}
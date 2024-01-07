#include "tvoc.h"

s_pin TVOC_A_PIN = 9;

byte s_tvoc_level = 0;
word s_tvocLastReported = 0;
word s_temp_tvoc_interval = 60 * 20; // 20 mins default, min 30 seconds
word s_tvoc_threshold = 0;
unsigned long s_lastReportedTimeTVOC = 0;


void setupTVOC()
{
  pinMode(TVOC_A_PIN, INPUT);
}

byte getTVOC()
{
    return s_tvoc_level;
}

void updateTVOC(bool firstTime)
{
  // wait for the next HIGH pulse
  DWORD duration = pulseIn(TVOC_A_PIN, HIGH, 200000); // wait for 200 ms
  if (duration == 0)                                  // no changes, just read the pin
  {
    byte level = digitalRead(TVOC_A_PIN);
    s_tvoc_level = (level > 0 ? 10 : 0);
    return;
  }

  s_tvoc_level = (duration / 1000 + 10) / 10;
  if (s_tvoc_level > 10)
    s_tvoc_level = 10;
}

bool reportTVOCUpdates(bool firstTime)
{
unsigned long curMillis = millis();

  bool reportTVOC = (abs(s_tvoc_level - s_tvocLastReported) > s_tvoc_threshold);
  bool timePassedTVOC = (curMillis - s_lastReportedTimeTVOC > (unsigned long)s_temp_tvoc_interval * 1000);

  if (firstTime || reportTVOC || timePassedTVOC)
  {
    zunoSendReport(CHANNEL_TVOC);
    s_tvocLastReported = s_tvoc_level;
    s_lastReportedTimeTVOC = curMillis;

#if SERIAL_LOGS
    Serial.print("TVOC update sent, because: ");
    Serial.print(reportTVOC);
    Serial.print(" ");
    Serial.print(timePassedTVOC);
    Serial.println();
#endif

    return true;
  }

  return false;
}
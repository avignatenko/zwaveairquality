
#include "ZUNO_DHT.h"
#include "EEPROM.h"

#include "HardwareSerial.h"
#include "EasyNextionLibrary.h" // Include EasyNextionLibrary

EasyNex s_display(Serial1); // Create an object of EasyNex class with the name < myNex >
                            // Set as parameter the Hardware Serial you are going to use

s_pin TVOC_A_PIN = 9;

#define SERIAL_LOGS 1

#define CHANNEL_DEFAULT 1
#define CHANNEL_TEMPERATURE 2
#define CHANNEL_HUMIDITY 3
#define CHANNEL_CO2 4
#define CHANNEL_TVOC 5

word s_humidity = 0;
word s_temperature = 0;
byte s_displayBrightness = 100;
word s_co2 = 400; // ppm
byte s_tvoc_level = 0;
word s_pm25 = 5; // ppm

#define ZUNO_SENSOR_MULTILEVEL_TEMPERATURE_2(GETTER) ZUNO_SENSOR_MULTILEVEL(ZUNO_SENSOR_MULTILEVEL_TYPE_TEMPERATURE, SENSOR_MULTILEVEL_SCALE_CELSIUS, SENSOR_MULTILEVEL_SIZE_TWO_BYTES, SENSOR_MULTILEVEL_PRECISION_ONE_DECIMAL, GETTER)
#define ZUNO_SENSOR_MULTILEVEL_HUMIDITY_2(GETTER) ZUNO_SENSOR_MULTILEVEL(ZUNO_SENSOR_MULTILEVEL_TYPE_RELATIVE_HUMIDITY, SENSOR_MULTILEVEL_SCALE_PERCENTAGE_VALUE, SENSOR_MULTILEVEL_SIZE_TWO_BYTES, SENSOR_MULTILEVEL_PRECISION_ONE_DECIMAL, GETTER)
#define ZUNO_SENSOR_MULTILEVEL_CO2_LEVEL_2(GETTER) ZUNO_SENSOR_MULTILEVEL(ZUNO_SENSOR_MULTILEVEL_TYPE_CO2_LEVEL, SENSOR_MULTILEVEL_SCALE_PARTS_PER_MILLION, SENSOR_MULTILEVEL_SIZE_TWO_BYTES, SENSOR_MULTILEVEL_PRECISION_ZERO_DECIMALS, GETTER)
#define ZUNO_SENSOR_MULTILEVEL_PM2_5_LEVEL(GETTER) ZUNO_SENSOR_MULTILEVEL(ZUNO_SENSOR_MULTILEVEL_TYPE_GENERAL_PURPOSE_VALUE, SENSOR_MULTILEVEL_SCALE_PARTS_PER_MILLION, SENSOR_MULTILEVEL_SIZE_TWO_BYTES, SENSOR_MULTILEVEL_PRECISION_ZERO_DECIMALS, GETTER)
#define ZUNO_SENSOR_MULTILEVEL_VOLATILE_ORGANIC_COMPOUND(GETTER) ZUNO_SENSOR_MULTILEVEL(ZUNO_SENSOR_MULTILEVEL_TYPE_VOLATILE_ORGANIC_COMPOUND, SENSOR_MULTILEVEL_SCALE_DIMENSIONLESS_VALUE, SENSOR_MULTILEVEL_SIZE_ONE_BYTE, SENSOR_MULTILEVEL_PRECISION_ZERO_DECIMALS, GETTER)


ZUNO_ENABLE(
    MODERN_MULTICHANNEL // No clusters, the first channel is mapped to NIF only
);

ZUNO_SETUP_CHANNELS(
    ZUNO_SWITCH_MULTILEVEL(getDisplayBrightness, setDisplayBrightness),
    ZUNO_SENSOR_MULTILEVEL_TEMPERATURE_2(s_temperature),
    ZUNO_SENSOR_MULTILEVEL_HUMIDITY_2(s_humidity),
    ZUNO_SENSOR_MULTILEVEL_CO2_LEVEL_2(s_co2),
    ZUNO_SENSOR_MULTILEVEL_VOLATILE_ORGANIC_COMPOUND(s_tvoc_level),
    ZUNO_SENSOR_MULTILEVEL_PM2_5_LEVEL(s_pm25));

enum
{
  CONFIG_TEMPERATURE_HUMIDITY_INTERVAL_SEC = 64,
  CONFIG_TEMPERATURE_THRESHOLD_DEGREES,
  CONFIG_HUMIDITY_THRESHOLD_PERCENT,
  CONFIG_TEMPERATURE_CORRECTION_DEGREES,
  CONFIG_HUMIDITY_CORRECTION_PERCENT
};

ZUNO_SETUP_CONFIGPARAMETERS(
    ZUNO_CONFIG_PARAMETER("Temperature and humidity update period (sec)", 30, 86400, 1800),
    ZUNO_CONFIG_PARAMETER_1B("Temperature update threshold", 1, 255, 2),
    ZUNO_CONFIG_PARAMETER_1B("Humidity update threshold", 1, 255, 5),
    ZUNO_CONFIG_PARAMETER_1B("Temperature correction (deg * 10 + 100)", 0, 200, 100),
    ZUNO_CONFIG_PARAMETER_1B("Humidity correction (% * 10 + 100)", 0, 200, 100));

ZUNO_SETUP_CFGPARAMETER_HANDLER(configParameterChanged2);

#define PIN_DHT 03

// temp & humidity sensor (DHT22)
DHT dht22_sensor(PIN_DHT, DHT22);

word s_humidityLastReported = 0;
word s_temperatureLastReported = 0;
unsigned long s_lastReportedTimeTemperature = 0;
unsigned long s_lastReportedTimeHumidity = 0;

word s_temp_hum_interval = 60 * 20; // 20 mins default, min 30 seconds
word s_temp_threshold = 2;
word s_hum_threshold = 5;

word s_temp_correct = 0;
word s_hum_correct = 0;


word s_tvocLastReported = 0;
word s_temp_tvoc_interval = 60 * 20; // 20 mins default, min 30 seconds
word s_tvoc_threshold = 0;
unsigned long s_lastReportedTimeTVOC = 0;

void updateFromCFGParams()
{
  s_temp_hum_interval = zunoLoadCFGParam(CONFIG_TEMPERATURE_HUMIDITY_INTERVAL_SEC);
  s_temp_threshold = zunoLoadCFGParam(CONFIG_TEMPERATURE_THRESHOLD_DEGREES);
  s_hum_threshold = zunoLoadCFGParam(CONFIG_HUMIDITY_THRESHOLD_PERCENT);
  s_temp_correct = zunoLoadCFGParam(CONFIG_TEMPERATURE_CORRECTION_DEGREES);
  s_hum_correct = zunoLoadCFGParam(CONFIG_HUMIDITY_CORRECTION_PERCENT);
}

void configParameterChanged2(byte param, uint32_t value)
{
  zunoSaveCFGParam(param, value);
  updateFromCFGParams();
}

BYTE getDisplayBrightness()
{
  return s_displayBrightness;
}

void setDisplayBrightness(BYTE newValue)
{
  s_displayBrightness = newValue;
  s_display.writeNum("dim", s_displayBrightness);
}

void setupDHT()
{
  dht22_sensor.begin();
}

void updateDHT()
{
  byte result;
  result = dht22_sensor.read(true);

  if (result == ZunoErrorOk)
  {
    s_humidity = (dht22_sensor.readHumidity() * 10) + (s_hum_correct - 100);
    s_temperature = (dht22_sensor.readTemperature() * 10) + (s_temp_correct - 100);
  }
  else
  {

    s_humidity = -100 * 10;
    s_temperature = -100 * 10;
  }
}

void setupTVOC()
{
  pinMode(TVOC_A_PIN, INPUT);
}

void updateTVOC(bool firstTime = false)
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
  return 4;   // red, no amber
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
  s_display.writeNum("temp_disp.val", s_temperature);
  s_display.writeNum("temp_disp.bco", colorFromSeverity(temperatureToSeverity(s_temperature / 10)));
}

void updateHumidityDisplay()
{
  s_display.writeStr("vis hum_disp,1");
  s_display.writeNum("hum_disp.val", s_humidity);
  s_display.writeNum("hum_disp.bco", colorFromSeverity(humidityToSeverity(s_humidity / 10)));
}

void updateTVOCDisplay()
{
  s_display.writeStr("vis co_disp,1");
  s_display.writeNum("co_disp.val", s_tvoc_level);
  s_display.writeNum("co_disp.bco", colorFromSeverity(tvocToSeverity(s_tvoc_level)));
}

void reportUpdates(bool firstTime = false)
{

#if SERIAL_LOGS
  Serial.println("reportUpdates called");
#endif

#if SERIAL_LOGS
  unsigned long curMillis = millis();

  Serial.print("Temp ");
  Serial.print(s_temperature);
  Serial.print(" ");
  Serial.println(s_temperatureLastReported);
#endif

  updateTemperatureDisplay();
  updateHumidityDisplay();
  updateTVOCDisplay();

  bool reportTemperature = (abs(s_temperature - s_temperatureLastReported) > s_temp_threshold);
  bool timePassedTemperature = (curMillis - s_lastReportedTimeTemperature > (unsigned long)s_temp_hum_interval * 1000);

  if (firstTime || reportTemperature || timePassedTemperature)
  {
    zunoSendReport(CHANNEL_TEMPERATURE);
    s_temperatureLastReported = s_temperature;
    s_lastReportedTimeTemperature = curMillis;

#if SERIAL_LOGS
    Serial.print("Temp update sent, because: ");
    Serial.print(reportTemperature);
    Serial.print(" ");
    Serial.print(timePassedTemperature);
    Serial.println();
#endif

    return;
  }

  bool reportHumidity = (abs(s_humidity - s_humidityLastReported) > s_hum_threshold);
  bool timePassedHumidity = (curMillis - s_lastReportedTimeHumidity > (unsigned long)s_temp_hum_interval * 1000);

  if (firstTime || reportHumidity || timePassedHumidity)
  {
    zunoSendReport(CHANNEL_HUMIDITY);
    s_humidityLastReported = s_humidity;
    s_lastReportedTimeHumidity = curMillis;

#if SERIAL_LOGS
    Serial.print("Hum update sent, because: ");
    Serial.print(reportHumidity);
    Serial.print(" ");
    Serial.print(timePassedHumidity);
    Serial.println();
#endif

    return;
  }

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

    return;
  }
}

void setup()
{

#if SERIAL_LOGS
  Serial.begin(115200);
#endif

  updateFromCFGParams();

  setupDisplay();
  setupDHT();
  setupTVOC();

  updateDHT();
  updateTVOC(true);    // first time\

  reportUpdates(true); // first time
}

void loop()
{

  updateDHT();
  updateTVOC();
  reportUpdates();

  delay(2000);
}
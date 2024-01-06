
#include "ZUNO_DHT.h"
#include "EEPROM.h"

#include "HardwareSerial.h"
#include "EasyNextionLibrary.h" // Include EasyNextionLibrary

EasyNex s_display(Serial1); // Create an object of EasyNex class with the name < myNex >
                            // Set as parameter the Hardware Serial you are going to use

#define SERIAL_LOGS 1

#define CHANNEL_DEFAULT 1
#define CHANNEL_TEMPERATURE 2
#define CHANNEL_HUMIDITY 3

word s_humidity = 0;
word s_temperature = 0;
byte s_switchDefault = 0;

#define ZUNO_SENSOR_MULTILEVEL_TEMPERATURE_2(GETTER) ZUNO_SENSOR_MULTILEVEL(ZUNO_SENSOR_MULTILEVEL_TYPE_TEMPERATURE, SENSOR_MULTILEVEL_SCALE_CELSIUS, SENSOR_MULTILEVEL_SIZE_TWO_BYTES, SENSOR_MULTILEVEL_PRECISION_ONE_DECIMAL, GETTER)
#define ZUNO_SENSOR_MULTILEVEL_HUMIDITY_2(GETTER) ZUNO_SENSOR_MULTILEVEL(ZUNO_SENSOR_MULTILEVEL_TYPE_RELATIVE_HUMIDITY, SENSOR_MULTILEVEL_SCALE_PERCENTAGE_VALUE, SENSOR_MULTILEVEL_SIZE_TWO_BYTES, SENSOR_MULTILEVEL_PRECISION_ONE_DECIMAL, GETTER)

ZUNO_ENABLE(
    MODERN_MULTICHANNEL // No clusters, the first channel is mapped to NIF only
);

ZUNO_SETUP_CHANNELS(
    ZUNO_SWITCH_BINARY(s_switchDefault, NULL),
    ZUNO_SENSOR_MULTILEVEL_TEMPERATURE_2(s_temperature),
    ZUNO_SENSOR_MULTILEVEL_HUMIDITY_2(s_humidity));

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

void setupDisplay()
{
  s_display.begin(9600);
  delay(500); // Wait for Nextion to start
}

void updateDisplay()
{
  s_display.writeStr("vis temp_disp,1");
  s_display.writeStr("vis hum_disp,1");
  s_display.writeNum("temp_disp.val", s_temperature);
  s_display.writeNum("hum_disp.val", s_humidity);
}

void resetLastReportedData()
{

  unsigned long curMillis = millis();

  s_humidityLastReported = s_humidity;
  s_temperatureLastReported = s_temperature;
  s_lastReportedTimeTemperature = curMillis;
  s_lastReportedTimeHumidity = curMillis;
}

void reportUpdates()
{

  updateDisplay();

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

  bool reportTemperature = (abs(s_temperature - s_temperatureLastReported) > s_temp_threshold);
  bool timePassedTemperature = (curMillis - s_lastReportedTimeTemperature > (unsigned long)s_temp_hum_interval * 1000);

  if (reportTemperature || timePassedTemperature)
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

  if (reportHumidity || timePassedHumidity)
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
}

void setup()
{

#if SERIAL_LOGS
  Serial.begin(115200);
#endif

  updateFromCFGParams();

  setupDisplay();
  setupDHT();
  updateDHT();
  resetLastReportedData();
}

void loop()
{

  updateDHT();
  reportUpdates();

  delay(2000);
}
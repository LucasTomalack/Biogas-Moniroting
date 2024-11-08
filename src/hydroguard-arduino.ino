#include <Wire.h>
#include <SSD1306Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <ph4502c_sensor.h>

#include "loramac.h"
#include "boards.h"

#include <ArduinoJson.h>
#include <CayenneLPP.h>

#define OLED_ADDR 0x3c
#define BMP280_ADDR 0x76

#define PH4502C_TEMPERATURE_PIN 34
#define PH4502C_PH_PIN 35
#define PH4502C_PH_TRIGGER_PIN 14
#define PH4502C_CALIBRATION 12.92f
#define PH4502C_READING_INTERVAL 100
#define PH4502C_READING_COUNT 100
#define ADC_RESOLUTION 4096.0f

// Seção Ritter Milligascounter
#define AnalogRitter 36
#define TRS_GAS_PIN 15
volatile unsigned long int contadorGiros = 0;
float volumeGas = 0.0;
#define VOLUME_GIRO 3.05 // Supondo que seja para o Ritter especificado 3ml por bascula
volatile bool bascula = false;

unsigned long previousMillis1 = 0;
const unsigned int interval1 = 30; // 1 second

unsigned long previousMillis2 = 0;
const unsigned int interval2 = 1000; // 1 seconds

SSD1306Wire display(OLED_ADDR, I2C_SDA, I2C_SCL);
Adafruit_BMP280 bmp;
PH4502C_Sensor ph4502c(
    PH4502C_PH_PIN,
    PH4502C_TEMPERATURE_PIN,
    PH4502C_CALIBRATION,
    PH4502C_READING_INTERVAL,
    PH4502C_READING_COUNT,
    ADC_RESOLUTION);

void handleInterruptGas()
{
  static unsigned long lastInterrupTime = 0;
  unsigned long interruptTime = millis();
  int noise = 1000;

  if (digitalRead(TRS_GAS_PIN) == LOW && interruptTime - lastInterrupTime > noise)
  {
    contadorGiros++;
    bascula = true;
    lastInterrupTime = interruptTime;
  }
}

void setup()
{
  initBoard();

  Serial.begin(115200);
  delay(5000);
  display.init();
  display.flipScreenVertically();
  display.setColor(WHITE);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);

  bmp.begin(BMP280_ADDR);
  bmp.setSampling(
      Adafruit_BMP280::MODE_NORMAL,
      Adafruit_BMP280::SAMPLING_X2,
      Adafruit_BMP280::SAMPLING_X16,
      Adafruit_BMP280::FILTER_X16,
      Adafruit_BMP280::STANDBY_MS_500);

  pinMode(TRS_GAS_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(TRS_GAS_PIN), handleInterruptGas, FALLING);

  ph4502c.init();
  delay(1500);
  setupLMIC();

  // previousMillis1 = millis() + (interval1 * 2);
}

CayenneLPP lpp(51);

void loop() {
  if (bascula) {
    volumeGas = contadorGiros * VOLUME_GIRO;
    bascula = false;
  }
  
  if (millis() - previousMillis2 >= interval2) {
    previousMillis2 = millis();

    // Leitura dos sensores
    float bmp_temperature = bmp.readTemperature();
    float bmp_pressure = bmp.readPressure() / 100.0;  // Converte Pa para hPa
    float bmp_altitude = bmp.readAltitude(1013.25);

    float ph4502c_ph = ph4502c.read_ph_level();
    float ph4502c_temperature = ph4502c.read_temp();

    DynamicJsonDocument payload(256);

    // bmp data
    JsonObject bmp_data = payload.createNestedObject("bmp");
    bmp_data["temperature"] = bmp_temperature;
    bmp_data["pressure"] = bmp_pressure;
    bmp_data["altitude"] = bmp_altitude;

    // ph4502c data
    JsonObject ph4502c_data = payload.createNestedObject("ph4502c");
    ph4502c_data["ph"] = ph4502c_ph;
    ph4502c_data["temperature"] = ph4502c_temperature;

    // trs data
    JsonObject trs_data = payload.createNestedObject("trs");
    trs_data["data"] = volumeGas;

    // Serializar o JSON para uma string
    std::string jsonString;
    serializeJson(payload, jsonString);

    // Imprimir o JSON no Serial Monitor para depuração
    // Serial.print(F("Sending: "));
    // serializeJsonPretty(payload, Serial);
    // Serial.println();

    // Alocar memória para o buffer de bytes
    uint8_t* txBuffer = (uint8_t*)malloc(jsonString.length());
    if (!txBuffer) {
      Serial.println(F("Erro de alocação de memória!"));
      return;
    }

    // Copiar os dados da string para o buffer de bytes
    memcpy(txBuffer, jsonString.c_str(), jsonString.length());

    txBufferLen = jsonString.length();

    updateLMICBuffer(txBuffer, txBufferLen);

    loopLMIC();

    free(txBuffer);

    // Exibir no display (opcional)
    display.clear();
    display.drawString(0, 10, "Temp: " + String(bmp_temperature, 2) + " ºC");
    display.drawString(0, 20, "Pressão: " + String(bmp_pressure, 2) + " hPa");
    display.drawString(0, 30, "Altitude: " + String(bmp_altitude, 2) + " m");
    display.drawString(0, 40, "pH: " + String(ph4502c_ph, 2));
    display.drawString(0, 50, "Vol. Gas: " + String(volumeGas, 2) + " ml");
    display.display();
  }
}

void checkForTilt()
{
  if (millis() - previousMillis1 > interval1)
  {
    if (bascula)
    {                             // Sua função de detecção de basculada
      previousMillis1 = millis(); // Atualiza o tempo da última basculada
      volumeGas = contadorGiros * VOLUME_GIRO;
      // Serial.print("Volume medido: ");
      // Serial.print(volumeGas);
      // Serial.println(" ml");
    }
  }
}
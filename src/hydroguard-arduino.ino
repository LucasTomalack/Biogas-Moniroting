#include <Wire.h>
#include <SSD1306Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <ph4502c_sensor.h>

#include "loramac.h"
#include "boards.h"

#include <ArduinoJson.h>

#define OLED_ADDR 0x3c
#define BMP280_ADDR 0x76

#define PH4502C_TEMPERATURE_PIN 34
#define PH4502C_PH_PIN 35
#define PH4502C_PH_TRIGGER_PIN 14 
#define PH4502C_CALIBRATION 12.92f
#define PH4502C_READING_INTERVAL 100
#define PH4502C_READING_COUNT 100
#define ADC_RESOLUTION 4096.0f

#define TRS_GAS_PIN 15

volatile int contadorGiros = 0;
float volumeGas = 0.0;
#define VOLUME_GIRO 3.0 //Supondo que seja para o Ritter especificado 3ml por bascula

volatile unsigned long int counter = 0;
const float averageGas = 0.1998f;

unsigned long previousMillis1 = 0;
const unsigned int interval1 = 10;  // 0.1 second

unsigned long previousMillis2 = 0;
const unsigned int interval2 = 1000;   // 1 seconds


SSD1306Wire display(OLED_ADDR, I2C_SDA, I2C_SCL);
Adafruit_BMP280 bmp;
PH4502C_Sensor ph4502c(
  PH4502C_PH_PIN,
  PH4502C_TEMPERATURE_PIN,
  PH4502C_CALIBRATION,
  PH4502C_READING_INTERVAL,
  PH4502C_READING_COUNT,
  ADC_RESOLUTION
);

void setup() {
  initBoard();

  Serial.begin(115200);
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
    Adafruit_BMP280::STANDBY_MS_500
  );

  pinMode(TRS_GAS_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(TRS_GAS_PIN), contarBascula, FALLING);
  
  ph4502c.init();
  delay(1500);
  setupLMIC();

  previousMillis1 = millis()+(interval1*2);
  
}

void loop() {

  if (millis() - previousMillis1 >= interval1) {
    previousMillis1 = millis();
      volumeGas = contadorGiros * VOLUME_GIRO;
      int trs_left = analogRead(TRS_GAS_PIN); 

    if (trs_left==0){
      counter++;
    }
  }

  if (millis() - previousMillis2 >= interval2) {
    previousMillis2 = millis();

    float bmp_temperature = bmp.readTemperature();
    float bmp_pressure = bmp.readPressure();
    float bmp_altitude = bmp.readAltitude(1013.25);

    float ph4502c_ph = ph4502c.read_ph_level();
    float ph4502c_temperature = ph4502c.read_temp();

    double trs_left = counter * averageGas;

    // Reseta o contador para evitar overflow
    if (counter >= 429000000){
      counter = 0;
    }

    StaticJsonDocument<4096> payload;

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
    trs_data["data"] = trs_left;
    
    // NOTE: This is not safe at all and its ugly
    txBufferLen = measureJson(payload);
    serializeJson(payload, txBuffer, txBufferLen);

    display.clear();

    display.drawString(0, 10, "Bmp Temperature: " + String(bmp_temperature) + "ºC");
    display.drawString(0, 20, "Bmp Pressure: " + String(bmp_pressure) + " Pa");
    display.drawString(0, 30, "Bmp Altitude: " + String(bmp_altitude) + " m");

    display.drawString(0, 40, "Ph4502c Ph: " + String(ph4502c_ph));

    //display.drawString(0, 50, "Trs Left: " + String(trs_left,5));

    display.drawString(0, 50, "Vol Ind: " + String(volumeGas,5));

    display.display();

    loopLMIC();
  }
}

void contarBascula(){
  contadorGiros++;
}
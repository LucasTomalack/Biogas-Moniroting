#include <Arduino.h>

#include <lmic.h>

#include <cstdint>
#include <hal/hal.h>

#include "boards.h"
// #include "loramac.h"

#define PROGMEM
#define LMIC_DEBUG_LEVEL = 1
#define CFG_au915

#define LORA_GAIN 20

// LoRaWAN NwkSKey, network session key
// This should be in big-endian (aka msb).
// cbb26990ffbccf7313672a5fa3ec20e3
static const PROGMEM u1_t NWKSKEY[16] = {0x28, 0xF6, 0xFA, 0xA2, 0x63, 0xA8,
                                         0x61, 0x73, 0x95, 0xC3, 0xAA, 0x06,
                                         0x3B, 0x9A, 0x41, 0xCE};

// LoRaWAN AppSKey, application session key
// This should also be in big-endian (aka msb).
static const u1_t PROGMEM APPSKEY[16] = {0xFE, 0x93, 0xBF, 0x1D, 0x7A, 0x50,
                                         0x5B, 0x9A, 0x2B, 0xF2, 0x27, 0x0F,
                                         0xD1, 0xAE, 0x62, 0xF2};

// LoRaWAN end-device address (DevAddr)
// See http://thethingsnetwork.org/wiki/AddressSpace
// The library converts the address to network byte order as needed, so this
// should be in big-endian (aka msb) too.
static const u4_t DEVADDR = 0x260D550F;

void loopLMIC(void) { os_runloop_once(); }

// These callbacks are only used in over-the-air activation, so they are
// left empty here (we cannot leave them out completely unless
// DISABLE_JOIN is set in arduino-lmic/project_config/lmic_project_config.h,
// otherwise the linker will complain).
void os_getArtEui(u1_t *buf){};
void os_getDevEui(u1_t *buf){};
void os_getDevKey(u1_t *buf){};

uint8_t txBuffer[4069];
int txBufferLen = 0;
void do_send(osjob_t *j) {
  if (LMIC.opmode & OP_TXRXPEND) {
    Serial.println(F("OP_TXRXPEND, not sending"));
  } else {
    LMIC_setTxData2(1, txBuffer, sizeof(uint8_t) * txBufferLen, 0);
    Serial.println(F("Packet queued"));
  }
}

static osjob_t sendjob;

const unsigned TX_INTERVAL = 900;

const lmic_pinmap lmic_pins = {
    .nss = RADIO_CS_PIN,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = RADIO_RST_PIN,
    .dio = {RADIO_DIO0_PIN, RADIO_DIO1_PIN, RADIO_BUSY_PIN}};

void onEvent(ev_t ev) {
  Serial.print(os_getTime());
  Serial.print(": ");
  switch (ev) {
  case EV_SCAN_TIMEOUT:
    Serial.println(F("EV_SCAN_TIMEOUT"));
    break;
  case EV_BEACON_FOUND:
    Serial.println(F("EV_BEACON_FOUND"));
    break;
  case EV_BEACON_MISSED:
    Serial.println(F("EV_BEACON_MISSED"));
    break;
  case EV_BEACON_TRACKED:
    Serial.println(F("EV_BEACON_TRACKED"));
    break;
  case EV_JOINING:
    Serial.println(F("EV_JOINING"));
    break;
  case EV_JOINED:
    Serial.println(F("EV_JOINED"));
    break;
  case EV_RFU1:
    Serial.println(F("EV_RFU1"));
    break;
  case EV_JOIN_FAILED:
    Serial.println(F("EV_JOIN_FAILED"));
    break;
  case EV_REJOIN_FAILED:
    Serial.println(F("EV_REJOIN_FAILED"));
    break;
  case EV_TXCOMPLETE:
    Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
    if (LMIC.txrxFlags & TXRX_ACK)
      Serial.println(F("Received ack"));
    if (LMIC.dataLen) {
      Serial.println(F("Received "));
      Serial.println(LMIC.dataLen);
      Serial.println(F(" bytes of payload"));
    }
    os_setTimedCallback(&sendjob, os_getTime() + sec2osticks(TX_INTERVAL),
                        do_send);
    break;
  case EV_LOST_TSYNC:
    Serial.println(F("EV_LOST_TSYNC"));
    break;
  case EV_RESET:
    Serial.println(F("EV_RESET"));
    break;
  case EV_RXCOMPLETE:
    Serial.println(F("EV_RXCOMPLETE"));
    break;
  case EV_LINK_DEAD:
    Serial.println(F("EV_LINK_DEAD"));
    break;
  case EV_LINK_ALIVE:
    Serial.println(F("EV_LINK_ALIVE"));
    break;
  case ECANCELED:
    Serial.println(F("ECANCELED"));
    break;
  default:
    Serial.print(F("Unknown event: "));
    Serial.println((unsigned)ev);
    break;
  }
}


void setupLMIC() {

  os_init();
  LMIC_reset();

#ifdef PROGMEM
  uint8_t appskey[sizeof(APPSKEY)];
  uint8_t nwkskey[sizeof(NWKSKEY)];
  memcpy_P(appskey, APPSKEY, sizeof(APPSKEY));
  memcpy_P(nwkskey, NWKSKEY, sizeof(NWKSKEY));
  LMIC_setSession(0x13, DEVADDR, nwkskey, appskey);
#else
  LMIC_setSession(0x13, DEVADDR, NWKSKEY, APPSKEY);
#endif

#if defined(CFG_eu868)
  LMIC_setupChannel(0, 868100000, DR_RANGE_MAP(DR_SF10, DR_SF7),
                    BAND_CENTI); // g-band
  LMIC_setupChannel(1, 868300000, DR_RANGE_MAP(DR_SF10, DR_SF7B),
                    BAND_CENTI); // g-band
  LMIC_setupChannel(2, 868500000, DR_RANGE_MAP(DR_SF10, DR_SF7),
                    BAND_CENTI); // g-band
  LMIC_setupChannel(3, 867100000, DR_RANGE_MAP(DR_SF10, DR_SF7),
                    BAND_CENTI); // g-band
  LMIC_setupChannel(4, 867300000, DR_RANGE_MAP(DR_SF10, DR_SF7),
                    BAND_CENTI); // g-band
  LMIC_setupChannel(5, 867500000, DR_RANGE_MAP(DR_SF10, DR_SF7),
                    BAND_CENTI); // g-band
  LMIC_setupChannel(6, 867700000, DR_RANGE_MAP(DR_SF10, DR_SF7),
                    BAND_CENTI); // g-band
  LMIC_setupChannel(7, 867900000, DR_RANGE_MAP(DR_SF10, DR_SF7),
                    BAND_CENTI); // g-band
  LMIC_setupChannel(8, 868800000, DR_RANGE_MAP(DR_FSK, DR_FSK),
                    BAND_MILLI); // g2-band
#elif defined(CFG_us915) || defined(CFG_au915)
  LMIC_selectSubBand(1);

#elif defined(CFG_as923)
#elif defined(CFG_kr920)
#elif defined(CFG_in866)
#else
#error Region not supported
#endif
  LMIC_setLinkCheckMode(0);
  LMIC.dn2Dr = DR_SF9;
  LMIC_setDrTxpow(DR_SF10, LORA_GAIN);
  do_send(&sendjob);
}

// void setup() {
//   // Inicializando os comandos no TTGO lora32 (placa usada para ensino na
//   // unioeste).
//   Serial.begin(115200);
//   // led.begin();
//   // led.setFont(uu8x8_font_chroma48medium8_r);
//   Serial.println("Iniciando");
//   setupLoRaWAN();
// }

// void loop() {
//   // unsigned long currentTime= millis();
//   // if (currentTime - displayTime > 5000) {
//   //   displayValues();
//   //   displayTime= currentTime;
//   // }
//   // Serial.println("loop");
//
//   os_runloop_once();
//
//   /* if (millis() > 5000 && gps.charsProcessed() < 10)
//      Serial.println(F("No GPS data received: check wiring"));*/
// }

// void displayValues() {
//   // tempo de espera entre cada leitura.
//   // Ler o valor da temperatura e imprimir esse valor no monitor serial.
//   //led.clear();
//
//   sensors_event_t event;
//   dht.temperature().getEvent(&event);
//   if (isnan(event.temperature)) {
//   //  led.drawString(0,0,"Erro ao ler a temperatura!"); //Se o sensor não
//   conseguir ler a temperatura ele imprimirá "Erro ao ler a temperatura".
//     Serial.println("Erro sensor");
//   }
//   else {
//     led.drawString(0,0,"T: ");
//     dtostrf(event.temperature,4,2, tempdht);
//     Serial.println(tempdht);
//     Serial.println(event.temperature);
//     led.drawString(5,0, tempdht);
//     led.drawString(10,0,"°C");
//   }
//    // Get humidity event and print its value.
//   dht.humidity().getEvent(&event);
//   if (isnan(event.relative_humidity)) {
//     Serial.println(F("Erro ao ler a umidade!")); //Se o sensor não conseguir
//     ler a umidade ele imprimirá "Erro ao ler a umidade"
//   }
//   else {
//    led.drawString(0,5,"U:");
//    dtostrf(event.relative_humidity,4,2, hum);
//        Serial.println(hum);
//     led.drawString(2,5, hum);
//     led.drawString(9,5,"%");
//
//   }

// }

#pragma once

#include <stdint.h>
#include <lmic.h>

extern uint8_t txBuffer[4096];
extern int txBufferLen;

void setupLMIC(void);
void loopLMIC(void);
void updateLMICBuffer(uint8_t* buffer, int bufferLen);

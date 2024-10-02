#pragma once

extern uint8_t txBuffer[4096];
extern int txBufferLen;

void setupLMIC(void);
void loopLMIC(void);

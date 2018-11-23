#ifndef TIMESTAMP_H
#define TIMESTAMP_H

#include <inttypes.h>
#include <stm32f4xx_hal.h>

ErrorStatus timestamp_init(void);

uint32_t tic();
uint32_t toc(uint32_t tic);

//void delay_us(uint32_t us);

void vHookSetupTimerForRunTimeStats(void);
uint32_t vHookGetRunTimerCounterValue(void);

#endif // TIMESTAMP_H

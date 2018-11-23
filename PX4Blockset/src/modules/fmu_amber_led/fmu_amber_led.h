#ifndef FMU_AMBER_LED_H
#define FMU_AMBER_LED_H 

#include "stm32f4xx_hal.h"

/**
*	initialize hardware for led control
*/
void fmu_amber_led_init(void);

/**
*	set led state (on/off) to value
*	val => 0  -> led off
*	val  > 0  -> led on
*/
void fmu_amber_led_set(uint32_t val);

/**
*	switch the led state
*/
void fmu_amber_led_toggle(void);

#endif // FMU_AMBER_LED_H

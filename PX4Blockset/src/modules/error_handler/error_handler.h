#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"
#include <defines.h>
#include <fmu_amber_led.h>

/*
 * Function for catches custom errors.
 * param val: 	0 - lets fmu led blink with 1Hz
 * 				x - lets fmu led blink x time with 400ms
 * 					and with additional pause of 3 seconds between blink series
 */
void error_handler(uint32_t val);

#endif // ERROR_HANDLER_H

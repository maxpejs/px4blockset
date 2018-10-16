/****************************************************************************
*
*   Copyright (C) 2016, Max Pejs <max.pejs@googlemail.com>
*	All rights reserved.
*
* 	Redistribution and use in source and binary forms, with or without 
*	modification, are permitted provided that the following conditions 
*	are met:
*
*	1. 	Redistributions of source code must retain the above copyright 
*		notice, this list of conditions and the following disclaimer.
*
*	2. 	Redistributions in binary form must reproduce the above copyright 
*		notice, this list of conditions and the following disclaimer in 
*		the documentation and/or other materials provided with the 
*		distribution.
*
*	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
*	"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
*	LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
*	FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
*	COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
*	INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
*	BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
*	OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED 
*	AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
*	OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF 
*	THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH 
*	DAMAGE.
*****************************************************************************/
  
/* @file driver for pwm aux out ports */
 
#ifndef PWM_AUX_OUT_H
#define PWM_AUX_OUT_H

#include <stm32f4xx_hal.h>
#include <defines.h>
#include <inttypes.h>
#include <comm_itf.h>

#define MAX_AUX_OUT_SERVO_CNT 		6
#define PWM_AUX_MIN_RATE 			20
#define PWM_AUX_MAX_RATE 			400
#define PWM_AUX_OUT_IMPULSE_MIN		90
#define PWM_AUX_OUT_IMPULSE_MAX		2500



/**
 * Initializes the ports and timers for aux pwms of pixhawk. There are two group of aux pwm ports:
 * 1st Group: ports 1-4
 * 2nd Group: ports 5-6
 * It is possible to set the pwm frequency for whole group, but not(!) for single pwm port
 *
 * Parameter:
 * 				- pwm_freqs:	Array with two(!) elements, contains pwm frequency values. Values will be limited
 * 								inside the function by the range 20Hz-400Hz
 */
void px4_pwm_aux_out_init(uint16_t * pwm_freqs);

/**
 * Writes new impulse values to pwm ports
 *
 * Parameter: 
 * impulse_values:	Array with six(!) elements, contains pwm impulse values in µs.
 */
void px4_pwm_aux_out_write_impulse(uint16_t * impulse_values);

/**
*	Returns current runtime of setting registers
*/
uint32_t px4_pwm_aux_out_getruntime(void);

#endif // PWM_AUX_OUT_H

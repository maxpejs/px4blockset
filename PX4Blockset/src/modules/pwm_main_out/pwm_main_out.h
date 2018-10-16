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

/* @file module for writing the impulse values to pwm main out ports of pixhawk */

#ifndef PWM_MAIN_OUT_H
#define PWM_MAIN_OUT_H

#include "pxio_driver.h"
#include "defines.h"
#include "timestamp.h"

// pwm main output
#define MAX_MAIN_OUT_SERVO_CNT 		8
#define PWM_MAIN_OUT_IMPULSE_MIN 	90 	 /* Lowest minimum PWM in us */
#define PWM_MAIN_OUT_IMPULSE_MAX 	2500 /* Highest maximum PWM in us */

/**
*	settings for initialization of main pwm output ports
*/
typedef struct
{
	uint16_t def_pwm_rate;	// Default pwm rate
	
	uint16_t alt_pwm_rate; 	// Alternate pwm rate
	
	uint16_t pwm_mask;		/* Mask for switching between default and alternate pwm rates
							 * 		Not every mask value is valid. Possible values are
							 * 		-0b00000011 channels 1 and 2 to alternate pwm rate, all other to default rate
							 * 		-0b00001100 channels 3 and 4 to alternate pwm rate, all other to default rate
							 * 		-0b11110000 channels 5 to  8 to alternate pwm rate, all other to default rate 
							 */
}pwm_main_out_settings_st;

/**
*	data struct for setting main out ports
*/
typedef struct
{
	uint16_t values[MAX_MAIN_OUT_SERVO_CNT];	// Array with ipulse values
	uint8_t safetyoff;							// SafetyOff flag
	uint8_t arm;								// Arming flag
}pwm_main_out_data_st;

/**
*	Initialization routine for pwm main channels of pixhawk.
*/
void px4_pwm_main_out_init(pwm_main_out_settings_st * settings);

/**
*	Set new values
*/
void px4_pwm_main_out_set(pwm_main_out_data_st * data);

/**
*	Set pins to given pwm rates
*/
void px4_pwm_main_out_update(void);

/**
*	Returns current runtime of tx command for set pwm impulse values to pxio
*/
uint32_t px4_pwm_main_out_getruntime(void);


#endif // PWM_MAIN_OUT_H

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
#ifndef COLOR_POWER_LED_H
#define COLOR_POWER_LED_H

#include <i2c_drv.h>
#include <defines.h>
#include <comm_itf.h>

#define COLOR_LED_CLK_SPEED			200000		// 200khz is overclocked!, originaly clock is 100khz
#define COLOR_LED_I2C_DEV_ADDR  	0x55<<1
#define COLOR_LED_I2C_ITF   		PX4_I2C2

#define BLUE_REG			0x81	// Blue, no auto-increment
#define GREEN_REG			0x82	// Green, no auto-increment
#define RED_REG				0x83	// Red, no auto-increment
#define SUB_ADDR_SETTINGS	0x84	// setting address, no auto-increment
#define SET_ENABLE_DATA		0x02
#define SET_SHDN 			0x01

typedef struct
{
	uint32_t r;
	uint32_t g;
	uint32_t b;
} color_led_data_st;

/**
 *	Initializes the module and the rgb driver chip.
 */
void px4_color_power_led_init();

/**
 *	Stores internal the given color channel values. The values will be calculated
 *	to pwm impuses by formula impulse = value * (16/256).
 *	Values below 16 are not applicable.
 */
void px4_color_power_led_set(uint32_t r, uint32_t g, uint32_t b);

/**
 *	Task for triggering the I2C-command to sets rgb led to internal stored channel values.
 */
void px4_color_power_led_update();

/**
 *	Returns current runtime of tx command
 */
uint32_t px4_color_power_led_getruntime(void);

#endif // COLOR_POWER_LED_H

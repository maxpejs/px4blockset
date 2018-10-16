/****************************************************************************
*
*   Copyright (C) 2017, Max Pejs <max.pejs@googlemail.com>
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
#ifndef I2C_DRV_H
#define I2C_DRV_H

#include <stm32f4xx_hal.h>
#include <defines.h>
#include <macros.h>
#include <inttypes.h>
#include <comm_itf.h>

#define PX4_I2C1	0
#define PX4_I2C2	1

/**
 *
 */
uint8_t px4_i2c_drv_init(uint32_t i2c_id);

/**
 *
 */
uint8_t px4_i2c_drv_transmit(uint32_t i2c_id, uint16_t I2C_DEVICE_ADDRESS, uint8_t * txBuff, uint8_t size);

/**
 *
 */
uint8_t px4_i2c_drv_receive(uint32_t i2c_id, uint16_t i2c_address, uint8_t * rxBuff, uint8_t size);

/**
 *
 */
void px4_i2c_drv_set_clock_speed(uint32_t spi_id, uint32_t speed);


#endif	// I2C_DRV_H

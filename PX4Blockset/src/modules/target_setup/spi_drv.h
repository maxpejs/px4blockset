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
#ifndef SPI_DRV_H
#define SPI_DRV_H

#include <stm32f4xx_hal.h>
#include <defines.h>
#include <macros.h>
#include <inttypes.h>
#include <comm_itf.h>

#define PX4_SPI1	0
#define PX4_SPI2	1

#define SPI_DRV_PRESCALER_2		SPI_BAUDRATEPRESCALER_2
#define SPI_DRV_PRESCALER_4		SPI_BAUDRATEPRESCALER_4
#define SPI_DRV_PRESCALER_8		SPI_BAUDRATEPRESCALER_8
#define SPI_DRV_PRESCALER_16	SPI_BAUDRATEPRESCALER_16
#define SPI_DRV_PRESCALER_32	SPI_BAUDRATEPRESCALER_32
#define SPI_DRV_PRESCALER_64	SPI_BAUDRATEPRESCALER_64
#define SPI_DRV_PRESCALER_128	SPI_BAUDRATEPRESCALER_128
#define SPI_DRV_PRESCALER_256	SPI_BAUDRATEPRESCALER_256



/**
 *
 */
uint8_t px4_spi_drv_init(uint32_t spi_id);

/**
 *
 */
uint8_t px4_spi_drv_transmit(uint32_t spi_id, uint8_t * txBuff, uint8_t * rxBuff, uint8_t size);



/**
 *
 */
void px4_spi_drv_set_clock_speed(uint32_t spi_id, uint32_t speed);




#endif	// SPI_DRV_H

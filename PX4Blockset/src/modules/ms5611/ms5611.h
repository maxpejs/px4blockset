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
  
/* @file driver for ms5611 (barometer) */

#ifndef MS5611_H
#define MS5611_H 

#include <comm_itf.h>
#include <defines.h>
#include <inttypes.h>
#include <spi_drv.h>
#include <error_handler.h>
#include <timestamp.h>

#define MS5611_MSG_BUFFERSIZE 12 + 1

#define MS5611_ENA       HAL_GPIO_WritePin(GPIOD, GPIO_PIN_7, RESET)
#define MS5611_DISA      HAL_GPIO_WritePin(GPIOD, GPIO_PIN_7, SET)

// registers
#define CMD_MS5611_RESET 		0x1E
#define REG_MS5611_PROM_BASE 	0xA0	// 	PROM memory base address

#define REG_MS5611_PROM_C1  	REG_MS5611_PROM_BASE | (1 << 1)		// 	C1 coefficient
#define REG_MS5611_PROM_C2 		REG_MS5611_PROM_BASE | (2 << 1)		// 	C2 coefficient
#define REG_MS5611_PROM_C3 		REG_MS5611_PROM_BASE | (3 << 1)		// 	C3 coefficient
#define REG_MS5611_PROM_C4 		REG_MS5611_PROM_BASE | (4 << 1)		// 	C4 coefficient
#define REG_MS5611_PROM_C5 		REG_MS5611_PROM_BASE | (5 << 1)		// 	C5 coefficient
#define REG_MS5611_PROM_C6 		REG_MS5611_PROM_BASE | (6 << 1)		// 	C6 coefficient

// ....
#define REG_MS5611_D1_OSR_256 	0x40
#define REG_MS5611_D1_OSR_512 	0x42
#define REG_MS5611_D1_OSR_1024 	0x44
#define REG_MS5611_D1_OSR_2048 	0x46
#define REG_MS5611_D1_OSR_4096 	0x48

// ....
#define REG_MS5611_D2_OSR_256	0x50
#define REG_MS5611_D2_OSR_512 	0x52
#define REG_MS5611_D2_OSR_1024 	0x54
#define REG_MS5611_D2_OSR_2048	0x56
#define REG_MS5611_D2_OSR_4096 	0x58

/**
*	measurement data storage struct
*/
typedef struct
{
	float 		baroValue;		// pressure (mbar)
	float 		temp;			// temperature (celcius)
	uint32_t	isNew;			// flag shows if data set contains new values
}ms5611_data_st;

/**
* TODO
*/
void px4_ms5611_init(void);

/**
* TODO
*/
void px4_ms5611_get(ms5611_data_st * data);

/**
 * TODO
 */
void px4_ms5611_update(void const * argv);

#endif // MS5611_H

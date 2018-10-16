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
#ifndef PXIO_DRIVER_H
#define PXIO_DRIVER_H

#include <stm32f4xx_hal.h>
#include <defines.h>
#include <error_handler.h>
#include <protocol.h>
#include <inttypes.h>
#include <string.h>
#include <comm_itf.h>
#include <timestamp.h>

#define DMA_TXRX_TIMEOUT		2000	// 2ms
#define TRANSMIT_TRIES_CNT		2
#define FMU_IO_BAUDRATE 		1500000
#define MAX_TRANSFER 			PKT_MAX_REGS * 2

/**
 * initialization of serial interface fo communication with pxio
 */
void pxio_driver_init(void);

/**
 * modifies single register values of pxio
 */
int32_t pxio_driver_reg_mod(uint8_t page, uint8_t offset, uint16_t clearbits, uint16_t setbits);

/**
 * reads register values of pxio
 */
int32_t pxio_driver_reg_get(uint8_t page, uint8_t offset, uint16_t *values, unsigned num_values);

/**
 * writes new values to pxio register
 */
int32_t pxio_driver_reg_set(uint8_t page, uint8_t offset, uint16_t *values, unsigned num_values);

#endif // PXIO_DRIVER_H



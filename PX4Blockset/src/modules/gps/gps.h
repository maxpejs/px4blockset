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

#ifndef GPS_H
#define GPS_H

#include "stm32f4xx_hal.h"
#include <defines.h>
#include <inttypes.h>
#include <error_handler.h>
#include <comm_itf.h>
#include <timestamp.h>
#include <gps_rmc_parcer.h>

#define RECEIVE_TIMEOUT				1500000	// timeout im µs after that the rmc data set to invalid if no new sentence are incoming
#define RMC_DATA_UPDATE_FREQ		1		// update frequence of NMEA 0183 RMC

#define RMC_SENTENCE_HEADER			"$GPRMC"

#define GPS_SENTENCE_BUFF_SIZE		90
#define GPS_RING_BUFF_SIZE			500
#define GPS_RX_PACKAGE_LENGTH		50	// buffer length for single interrupt

#if GPS_RING_BUFF_SIZE % GPS_RX_PACKAGE_LENGTH != 0
#error "Wrong size of GPS_RING_BUFF_SIZE or GPS_RX_PACKAGE_LENGTH. Schould be: GPS_RING_BUFF_SIZE % GPS_RX_PACKAGE_LENGTH = 0"
#endif

/**
*	TODO
*/
void px4_gps_init(uint32_t baud);

/**
*	TODO
*/
void px4_gps_get(gps_rmc_packet_st * pData);

/**
*	TODO
*/
void px4_gps_get_raw(uint8_t * buff);

/**
*	TODO
*/
void px4_gps_update(void);

/**
*	TODO
*/
void px4_gps_rx_complete_event(void);

#endif // GPS_H

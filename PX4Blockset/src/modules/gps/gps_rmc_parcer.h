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
#ifndef GPS_RMC_PARCER_H
#define GPS_RMC_PARCER_H

#define SPEED_FACTOR_KNOTS_TO_MS	0.51444f // Speed converting factor knots->m/s

#include <inttypes.h>
#include <timestamp.h>
#include <utilities.h>
#include <string.h>
#include <stdlib.h>

/**
* The struct with data of the NMEA0183 RMC sentence format
*/
typedef struct
{
	uint8_t  hour;
	uint8_t  Minute;
	uint8_t  Second;
	uint8_t  Day;
	uint8_t  Month;
	uint8_t  Year;
	uint8_t  Valid;			    // A - Data valid, V - data not valid
	float  	 Latitude;			// latitude
	float  	 Longitude;			// longitude
	float    Ground_speed_ms;	// speed over ground (m/s) * 10^-2
	float 	 Ground_course;		// course over ground in degrees * 10^-2
	float  	 MagVar;			// magnetic variation, East(+)/West(-) in degrees
	uint8_t  Quality;
}gps_rmc_packet_st;

void parse_nmea_rmc_sentence(uint8_t * rxMsgBuffer, gps_rmc_packet_st * rmcData);

#endif // GPS_RMC_PARCER_H

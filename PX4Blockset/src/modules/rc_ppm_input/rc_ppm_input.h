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

/* @file module for receiving rc input data from rc transmitter conneted to pixhawk (pxio) */

#ifndef RC_PPM_INPUT_H
#define RC_PPM_INPUT_H 

#include "pxio_driver.h"
#include "timestamp.h"

#define MAX_RC_CHANNEL_CNT 		18
#define CHANNEL_PART 			10

/**
 * TODO
 */
typedef struct
{
	uint16_t  channels[MAX_RC_CHANNEL_CNT];
	uint8_t   channel_cnt;
}rc_ppm_input_data_st;


/**
* TODO
*/
void px4_rc_ppm_input_init(void);

/**
* TODO
*/
void px4_rc_ppm_input_get(rc_ppm_input_data_st * data);

/**
 * TODO
 */
void px4_rc_ppm_input_task_function(void const * argv);


/**
 * TODO
 */
uint32_t px4_rc_ppm_input_getruntime(void);

#endif // RC_PPM_INPUT_H

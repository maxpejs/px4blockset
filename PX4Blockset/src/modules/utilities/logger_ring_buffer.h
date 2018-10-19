/****************************************************************************
*
*   Copyright (C) 2018, Max Pejs <max.pejs@googlemail.com>
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
#ifndef LOGGER_RING_BUFFER_H
#define LOGGER_RING_BUFFER_H
#include <inttypes.h>

#define SIGNAL_MAX_CNT 		10
#define RING_BUFF_SIZE		200

typedef struct
{
	uint32_t timestamp;
	float val[SIGNAL_MAX_CNT];
}data_st;

typedef struct
{
	uint32_t read;
	uint32_t write;
	data_st buff[RING_BUFF_SIZE];
}ring_buff_data_st;

/**
*	TODO
*/
uint32_t ring_buffer_full(ring_buff_data_st * b);

/**
*	TODO
*/
uint32_t ring_buffer_empty(ring_buff_data_st * b);

/**
*	TODO
*/
uint32_t ring_buffer_free_space(ring_buff_data_st * b);

/**
*	TODO
*/
uint32_t ring_buffer_count(ring_buff_data_st * b);



#endif // LOGGER_RING_BUFFER_H

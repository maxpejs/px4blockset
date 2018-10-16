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
#ifndef CPULOAD_H
#define CPULOAD_H

#include "stm32f4xx_hal.h"
#include "inttypes.h"
#include "timestamp.h"

/**
*	TODO
*/
void cpu_load_init(uint32_t sTime_ms);

/**	
*	TODO
*/
void cpu_load_start_meas(void);

/**
*	TODO
*/
void cpu_load_stop_meas(void);

/**
*	TODO
*/
uint32_t cpu_load_get_curr_cpu_load(void);

/**
* TODO
*/
uint32_t cpu_load_get_max_cpu_load(void);

#endif // CPULOAD_H
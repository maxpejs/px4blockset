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
#ifndef TASKS_H
#define TASKS_H


#include <mpu6000.h>
#include <ms5611.h>
#include <hmc5883.h>
#include <gps.h>
#include <fmu_amber_led.h>
#include <rc_ppm_input.h>
#include <pwm_aux_out.h>
#include <pwm_main_out.h>
#include <color_power_led.h>
#include <sd_card_logger.h>

/**
*
*/
void px4_tasks_read_inputs();

/**
*
*/
void px4_tasks_write_outputs();


#endif // TASKS_H

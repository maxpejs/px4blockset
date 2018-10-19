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
#include "hmc5883.h"

static uint32_t  _module_state 		= DISABLE;
static uint32_t _hmc5883_runtime	= 0;

static hmc5883_data_st 		_hmc5883_data_storage;
static hmc5883_settings_st 	_settings;

static uint32_t hmc5883_enable(void);
static uint32_t _get_scale(void);

void px4_hmc5883_init(hmc5883_settings_st * in_settings)
{
	// copy settings to internal var
	memcpy(&_settings, in_settings, sizeof(hmc5883_settings_st));
	memset(&_hmc5883_data_storage, 0, sizeof(_hmc5883_data_storage));

	px4_i2c_drv_init(HMC5883_I2C_ITF);
	px4_i2c_drv_set_clock_speed(HMC5883_I2C_ITF, HMC5883_CLOCK_SPEED);

	if (hmc5883_enable() == SUCCESS)
	{
		_module_state = ENABLE;
		px4debug(eHMC5883, "hmc5883 init ok\r\n");
	}
	else
	{
		px4debug(eHMC5883,"err at hmc5883 init!\r\n");
	}
}

void px4_hmc5883_update()
{
	uint32_t start = tic();

	if (_module_state == DISABLE)
	{
		return;
	}

	uint8_t msg[1] = { HMC5883_REG_MAG_OUT_X_HB };
	int result = px4_i2c_drv_transmit(HMC5883_I2C_ITF, HMC5883_I2C_DEVICE_ADDRESS, msg, sizeof(msg));

	if (result == ERROR)
	{
		px4debug(eHMC5883,"err tx cmd at compass module!\r\n");
		return;
	}

	uint8_t rx[6] = { 0 };
	result = px4_i2c_drv_receive(HMC5883_I2C_ITF, HMC5883_I2C_DEVICE_ADDRESS, rx, sizeof(rx));

	if (result == ERROR)
	{
		px4debug(eHMC5883, "err get compass data!\r\n");
		return;
	}

	float scale = (float)_get_scale();
	_hmc5883_data_storage.magX = (float)TWO_UINT8_TO_INT16(rx[0], rx[1]) / scale;
	_hmc5883_data_storage.magZ = (float)TWO_UINT8_TO_INT16(rx[2], rx[3]) / scale;
	_hmc5883_data_storage.magY = (float)TWO_UINT8_TO_INT16(rx[4], rx[5]) / scale;

	_hmc5883_runtime = toc(start);
}

static uint32_t _get_scale(void)
{
	switch(_settings.magRange)
	{
	case HMC5883_RANGE_0_88:	return 1370;
	case HMC5883_RANGE_1_3:		return 1090;
	case HMC5883_RANGE_1_9:		return 820;
	case HMC5883_RANGE_2_5:		return 660;
	case HMC5883_RANGE_4_0:		return 440;
	case HMC5883_RANGE_4_7:		return 390;
	case HMC5883_RANGE_5_6:		return 330;
	case HMC5883_RANGE_8_1:		return 230;
	default:					return 1090;	// range +/- 1,3Ga is default chip setting
	}
}


void px4_hmc5883_get(hmc5883_data_st * data)
{
	memcpy(data, &_hmc5883_data_storage, sizeof(hmc5883_data_st));
}

static uint32_t hmc5883_enable(void)
{
	uint8_t msg[2] = {0};
	uint32_t result = SUCCESS;

	// set sample rate & averaging
	msg[0] = HMC5883_REG_CONFA;
	msg[1] = 0x18;	// 75Hz & average on 1 sample (no average filter)
	result = px4_i2c_drv_transmit(HMC5883_I2C_ITF, HMC5883_I2C_DEVICE_ADDRESS, msg, sizeof(msg));

	// set range
	msg[0] = HMC5883_REG_CONFB;
	msg[1] = _settings.magRange;
	result &= px4_i2c_drv_transmit(HMC5883_I2C_ITF, HMC5883_I2C_DEVICE_ADDRESS, msg, sizeof(msg));


	msg[0] = HMC5883_REG_MODE;
	msg[1] = 0;
	result &= px4_i2c_drv_transmit(HMC5883_I2C_ITF, HMC5883_I2C_DEVICE_ADDRESS, msg, sizeof(msg));

	if (result == ERROR)
		px4debug(eHMC5883, "err set regs!\r\n");

	return result;
}

uint32_t px4_hmc5883_getruntime(void)
{
	return _hmc5883_runtime;
}

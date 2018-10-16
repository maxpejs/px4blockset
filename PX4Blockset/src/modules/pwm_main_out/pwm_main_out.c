#include "pwm_main_out.h"
#include <pxio_driver.h>
#include <defines.h>
#include <timestamp.h>
#include <macros.h>

static uint32_t _safety_off_last_val 	= 0;
static uint32_t _arm_state_last_val 		= 0;
static uint32_t _main_out_runtime		= 0;
static uint32_t _main_out_max_runtime	= 0;


static pwm_main_out_data_st 			_pwm_output_data[2];
static int32_t  _idxsection  			= -1;

static uint32_t  _module_ready 			= DISABLE;

static int32_t internal_reg_mod(uint8_t page, uint8_t offset, uint16_t clearbits, uint16_t setbits);
static int32_t internal_reg_set(uint8_t page, uint8_t offset, uint16_t *values, unsigned num_values);

static void set_arm_state(uint8_t ena)
{
	// inform pxio about the arm state switching
	int result;

	if (ena)
	{
		result = pxio_driver_reg_mod(PX4IO_PAGE_SETUP, PX4IO_P_SETUP_ARMING, 0, PX4IO_P_SETUP_ARMING_FMU_ARMED);
	}
	else
	{
		result = pxio_driver_reg_mod(PX4IO_PAGE_SETUP, PX4IO_P_SETUP_ARMING, PX4IO_P_SETUP_ARMING_FMU_ARMED, 0);
	}

	// store new value only if tx command was successful
	// otherwise try once again on next function call
	if (result == SUCCESS)
	{
		_arm_state_last_val = ena;
	}
	else
	{
		debug_print_string("change pwm arm err!\r\n");
	}
}

static void set_safety_off_state(uint8_t ena)
{
	// inform pxio about the safety off switching

	int result;
	if (ena)
	{
		result = pxio_driver_reg_mod(PX4IO_PAGE_STATUS, PX4IO_P_STATUS_FLAGS, 0, PX4IO_P_STATUS_FLAGS_SAFETY_OFF);
	}
	else
	{
		result = pxio_driver_reg_mod(PX4IO_PAGE_STATUS, PX4IO_P_STATUS_FLAGS, PX4IO_P_STATUS_FLAGS_SAFETY_OFF, 0);
	}

	// store new value only if tx command was successful
	// otherwise try once again on next function call
	if (result == SUCCESS)
	{
		_safety_off_last_val = ena;
	}
	else
	{
		debug_print_string("change safety off err!\r\n");
	}
}

void px4_pwm_main_out_init(pwm_main_out_settings_st * settings)
{
	memset(&_pwm_output_data, 0, sizeof(_pwm_output_data));
	pxio_driver_init();

	if (internal_reg_mod(PX4IO_PAGE_SETUP, PX4IO_P_SETUP_ARMING, 0, PX4IO_P_SETUP_ARMING_RC_HANDLING_DISABLED) != SUCCESS)
	{
		debug_print_string("reg mod PX4IO_P_SETUP_ARMING_RC_HANDLING_DISABLED err\r\n");
	}

	if (internal_reg_mod(PX4IO_PAGE_SETUP, PX4IO_P_SETUP_ARMING, 0, PX4IO_P_SETUP_ARMING_IO_ARM_OK) != SUCCESS)
	{
		debug_print_string("reg mod PX4IO_P_SETUP_ARMING_IO_ARM_OK err\r\n");
	}

	/* SET PWM DEFAULT RATE */
	if (internal_reg_set(PX4IO_PAGE_SETUP, PX4IO_P_SETUP_PWM_DEFAULTRATE, &(settings->def_pwm_rate), 1) != SUCCESS)
	{
		debug_print_string("set def pwm rate err\r\n");
	}

	/* SET PWM ALTERNATE RATE */
	if (internal_reg_set(PX4IO_PAGE_SETUP, PX4IO_P_SETUP_PWM_ALTRATE, &(settings->alt_pwm_rate), 1) != SUCCESS)
	{
		debug_print_string("set alt pwm rate err\r\n");
	}

	/* SET PWM RATE BITMASK*/
	if (internal_reg_set(PX4IO_PAGE_SETUP, PX4IO_P_SETUP_PWM_RATES, &(settings->pwm_mask), 1) != SUCCESS)
	{
		debug_print_string("set pwm mask err\r\n");
	}

	set_safety_off_state(DISABLE);
	set_arm_state(DISABLE);

	_module_ready = ENABLE;

	debug_print_string("pwm_main_out init ok\r\n");
}

void px4_pwm_main_out_set(pwm_main_out_data_st * new_data)
{
	// init on first call
	if(_idxsection == -1)
	{
		_idxsection = 0;
	}
	
	memcpy(&_pwm_output_data[_idxsection], new_data, sizeof(pwm_main_out_data_st));
}

void px4_pwm_main_out_task_function(void const * argv)
{
	uint32_t start = tic();

	if (_module_ready == DISABLE)
	{
		return;
	}
	
	if(_idxsection == -1)
	{
		// no data to write on output
		return;
	}
	
	// get index from last updated storage bank
	uint32_t temp_idx = _idxsection;
	
	// switch index to other storage bank
	_idxsection = (_idxsection + 1) % 2;

	if (_safety_off_last_val != _pwm_output_data[temp_idx].safetyoff)
	{
		set_safety_off_state(_pwm_output_data[temp_idx].safetyoff); // safety off state change detected
	}

	if (_arm_state_last_val != _pwm_output_data[temp_idx].arm)
	{
		set_arm_state(_pwm_output_data[temp_idx].arm); // arm state change detected
	}

	for (int i = 0; i < MAX_MAIN_OUT_SERVO_CNT; i++)
	{
		if (_pwm_output_data[temp_idx].values[i] > PWM_MAIN_OUT_IMPULSE_MAX)
			_pwm_output_data[temp_idx].values[i] = PWM_MAIN_OUT_IMPULSE_MAX;

		if (_pwm_output_data[temp_idx].values[i] < PWM_MAIN_OUT_IMPULSE_MIN)
			_pwm_output_data[temp_idx].values[i] = PWM_MAIN_OUT_IMPULSE_MIN;
	}

	// send PWM values to PXIO as impulse duration in �s for given pwm channels
	if (pxio_driver_reg_set(PX4IO_PAGE_DIRECT_PWM, 0, &(_pwm_output_data[temp_idx].values[0]), MAX_MAIN_OUT_SERVO_CNT) != SUCCESS)
	{
		debug_print_string("set direct main out pwms err \r\n");
	}
	
	_main_out_runtime = toc(start);
	
	if(_main_out_runtime > _main_out_max_runtime)
	{
		_main_out_max_runtime = _main_out_runtime;
	}
}

static int32_t internal_reg_mod(uint8_t page, uint8_t offset, uint16_t clearbits, uint16_t setbits)
{
	int32_t tries = 3;
	int32_t ret;
	
	do
	{
		ret = pxio_driver_reg_mod(page, offset, clearbits, setbits);
		if(ret == SUCCESS)
		{
			return ret;
		}
		else
		{
			HAL_Delay(10);
		}
	}while(tries-- > 0);
	
	return ret;
} 

static int32_t internal_reg_set(uint8_t page, uint8_t offset, uint16_t *values, unsigned num_values)
{
	int32_t tries = 3;
	int32_t ret;
	
	do
	{
		ret = pxio_driver_reg_set(page, offset, values, num_values);
		if(ret == SUCCESS)
		{
			return ret;
		}
		else
		{
			HAL_Delay(100);
		}
	}while(tries-- > 0);

	return ret;
} 

uint32_t px4_pwm_main_out_getruntime(void)
{
	return _main_out_runtime;
}


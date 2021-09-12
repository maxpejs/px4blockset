#include "pwm_main_out.h"
#include <pxio_driver.h>
#include <defines.h>
#include <timestamp.h>
#include <macros.h>

static uint32_t safetyoff_last_val 	= 0;
static uint32_t _arm_state_last_val = 0;
static pwm_main_out_data_st 		pwm_output_data[2];
static int32_t  storage_idx  		= 0;

static uint32_t  module_ready 		= DISABLE;

static int32_t internal_reg_mod(uint8_t page, uint8_t offset, uint16_t clearbits, uint16_t setbits);
static int32_t internal_reg_set(uint8_t page, uint8_t offset, uint16_t *values, unsigned num_values);

static void set_arm_state(uint8_t ena)
{
	// if value didn't changed, nothing to do here
	if (_arm_state_last_val == ena)
	{
		return;
	}

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
		px4debug("change pwm arm err!\n");
	}
}

static void set_safetyoff_state(uint8_t ena)
{
	// if value didn't changed, nothing to do here
	if (safetyoff_last_val == ena)
	{
		return;
	}

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
		safetyoff_last_val = ena;
	}
	else
	{
		px4debug("change safety off err!\n");
	}
}

void px4_pwm_main_out_init(pwm_main_out_settings_st * settings)
{
	memset(&pwm_output_data, 0, sizeof(pwm_output_data));
	pxio_driver_init();

	if (internal_reg_mod(PX4IO_PAGE_SETUP, PX4IO_P_SETUP_ARMING, 0, PX4IO_P_SETUP_ARMING_RC_HANDLING_DISABLED) != SUCCESS)
	{
		px4debug("reg mod HANDLING_DISABLED err\n");
	}

	if (internal_reg_mod(PX4IO_PAGE_SETUP, PX4IO_P_SETUP_ARMING, 0, PX4IO_P_SETUP_ARMING_IO_ARM_OK) != SUCCESS)
	{
		px4debug("reg mod ARM_OK err\n");
	}

	/* SET PWM DEFAULT RATE */
	if (internal_reg_set(PX4IO_PAGE_SETUP, PX4IO_P_SETUP_PWM_DEFAULTRATE, &(settings->def_pwm_rate), 1) != SUCCESS)
	{
		px4debug("set def pwm rate err\n");
	}

	/* SET PWM ALTERNATE RATE */
	if (internal_reg_set(PX4IO_PAGE_SETUP, PX4IO_P_SETUP_PWM_ALTRATE, &(settings->alt_pwm_rate), 1) != SUCCESS)
	{
		px4debug("set alt pwm rate err\n");
	}

	/* SET PWM RATE BITMASK*/
	if (internal_reg_set(PX4IO_PAGE_SETUP, PX4IO_P_SETUP_PWM_RATES, &(settings->pwm_mask), 1) != SUCCESS)
	{
		px4debug("set pwm mask err\n");
	}

	set_safetyoff_state(DISABLE);
	set_arm_state(DISABLE);

	module_ready = ENABLE;

	px4debug("pwm_main_out init ok\n");
}

void px4_pwm_main_out_set(pwm_main_out_data_st * new_data)
{
	memcpy(&pwm_output_data[storage_idx], new_data, sizeof(pwm_main_out_data_st));
}

void px4_pwm_main_out_update()
{
	if (module_ready == DISABLE)
	{
		return;
	}
	
	// get index from last updated storage bank
	uint32_t idx = storage_idx;

	// switch index to other storage bank
	storage_idx = (storage_idx + 1) % 2;

	// handle safety off state change
	set_safetyoff_state(pwm_output_data[idx].safetyoff);

	// handle arm state change
	set_arm_state(pwm_output_data[idx].arm);


	for (int i = 0; i < MAX_MAIN_OUT_SERVO_CNT; i++)
	{
		if (pwm_output_data[idx].values[i] > PWM_MAIN_OUT_IMPULSE_MAX)
			pwm_output_data[idx].values[i] = PWM_MAIN_OUT_IMPULSE_MAX;

		if (pwm_output_data[idx].values[i] < PWM_MAIN_OUT_IMPULSE_MIN)
			pwm_output_data[idx].values[i] = PWM_MAIN_OUT_IMPULSE_MIN;
	}

	// send PWM values to PXIO as impulse duration in µs for given pwm channels
	if (pxio_driver_reg_set(PX4IO_PAGE_DIRECT_PWM, 0, &(pwm_output_data[idx].values[0]), MAX_MAIN_OUT_SERVO_CNT) != SUCCESS)
	{
		px4debug("set direct main out pwms err \n");
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

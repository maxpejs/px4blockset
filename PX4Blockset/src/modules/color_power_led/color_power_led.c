#include "color_power_led.h"

static uint32_t _module_state = DISABLE;
static uint32_t _RGB[3] = {0, 0, 0};
static uint32_t _new_value = 1;

static uint32_t _color_power_led_runtime		= 0;
static uint32_t _color_power_led_max_runtime	= 0;

static int color_power_led_enable(void);

void px4_color_power_led_init()
{
	px4_i2c_drv_init(COLOR_LED_I2C_ITF);
	px4_i2c_drv_set_clock_speed(COLOR_LED_I2C_ITF, COLOR_LED_CLK_SPEED);

	if (color_power_led_enable() == SUCCESS)
	{
		_module_state = ENABLE;
		debug_print_string("color power led init ok\r\n");
	}
	else
	{
		debug_print_string("err at color power led init\r\n");
	}
}

void px4_color_power_led_update()
{
	uint64_t start = tic();

	if (_module_state == DISABLE)
	{
		// module not used, nothing to do
		return;
	}


	if (_new_value)
	{
		uint32_t _RGB_scale[3];

		for (int i = 0; i < 3; i++)
		{
			_RGB_scale[i] = (_RGB[i] * 16) / 256;
		}

		uint8_t msg[6] = { 	SUB_ADDR_PWM0, (uint8_t)_RGB_scale[2],
							SUB_ADDR_PWM1, (uint8_t)_RGB_scale[1],
							SUB_ADDR_PWM2, (uint8_t)_RGB_scale[0] };

		int result = px4_i2c_drv_transmit(COLOR_LED_I2C_ITF, COLOR_LED_I2C_DEV_ADDR, msg, sizeof(msg));

		// if no errors reset, otherwise try to set color on next call
		if (result != ERROR)
		{
			_new_value = 0;
			_color_power_led_runtime = (uint32_t)toc(start);
		}
		else
		{
			debug_print_string("err set rgb color\r\n");
		}

		if(_color_power_led_runtime > _color_power_led_max_runtime)
			_color_power_led_max_runtime = _color_power_led_runtime;
	}
}

void px4_color_power_led_set(uint32_t r, uint32_t g, uint32_t b)
{
	if (_RGB[0] == r && _RGB[1] == g && _RGB[2] == b)
	{
		// no changes since last call
		return;
	}
	else
	{
		_RGB[0] = r;
		_RGB[1] = g;
		_RGB[2] = b;
		_new_value = 1;
	}
}

int color_power_led_enable(void)
{
	uint8_t msg[2] = { SUB_ADDR_SETTINGS, SET_ENABLE_DATA|SET_SHDN };
	return px4_i2c_drv_transmit(COLOR_LED_I2C_ITF, COLOR_LED_I2C_DEV_ADDR, msg, sizeof(msg));
}

uint32_t px4_color_power_led_getruntime(void)
{
	return _color_power_led_runtime;
}

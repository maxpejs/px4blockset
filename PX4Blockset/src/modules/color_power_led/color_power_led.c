#include "color_power_led.h"

static uint32_t _module_state = DISABLE;
static color_led_data_st _RGB_out, _RGB_last;

static uint32_t _runtime		= 0;
static uint32_t _max_runtime	= 0;

static int color_power_led_enable(void);

void px4_color_power_led_init()
{
	px4_i2c_drv_init(COLOR_LED_I2C_ITF);
	px4_i2c_drv_set_clock_speed(COLOR_LED_I2C_ITF, COLOR_LED_CLK_SPEED);

	memset(&_RGB_out, 0, sizeof(_RGB_out));

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

static int cnt = 0;

void px4_color_power_led_update(void const * argv)
{
	UNUSED(argv);

	px4debug(eCOLORLED, "px4_color_power_led_update: %d", cnt++);

	uint32_t start = tic();

	if (_module_state == DISABLE)
	{
		// nothing to do
		return;
	}

	// copy to avoid inconsistency data
	color_led_data_st temp;
	memcpy(&temp, &_RGB_out, sizeof(color_led_data_st));

	if (temp.r == _RGB_last.r && temp.g == _RGB_last.g && temp.b == _RGB_last.b)
	{
		// same color already set last call, so not necessary to do it once again (reduce runtime)
		return;
	}

	uint8_t msg[6] = { BLUE_REG, (uint8_t)temp.b, GREEN_REG, (uint8_t)temp.g, RED_REG, (uint8_t)temp.r };

	int result = px4_i2c_drv_transmit(COLOR_LED_I2C_ITF, COLOR_LED_I2C_DEV_ADDR, msg, sizeof(msg));

	// if no errors mark as successful, otherwise try to set color on next call
	if (result != ERROR)
	{
		memcpy(&_RGB_last, &temp, sizeof(color_led_data_st));
		_runtime = toc(start);
	}
	else
	{
		debug_print_string("err set rgb color\r\n");
	}

	if (_runtime > _max_runtime)
		_max_runtime = _runtime;
}

void px4_color_power_led_set(uint32_t r, uint32_t g, uint32_t b)
{
	_RGB_out.r = (r >> 4); // rescale to 16/256
	_RGB_out.g = (g >> 4);
	_RGB_out.b = (b >> 4);
}

int color_power_led_enable(void)
{
	uint8_t msg[2] = { SUB_ADDR_SETTINGS, SET_ENABLE_DATA|SET_SHDN };
	return px4_i2c_drv_transmit(COLOR_LED_I2C_ITF, COLOR_LED_I2C_DEV_ADDR, msg, sizeof(msg));
}

uint32_t px4_color_power_led_getruntime(void)
{
	return _runtime;
}

#include "color_power_led.h"

static uint32_t module_state = DISABLE;
static color_led_data_st rgb_color[2], rgb_last;
static uint32_t storage_idx;

int color_power_led_enable(void)
{
	uint8_t msg[2] = { SUB_ADDR_SETTINGS, SET_ENABLE_DATA|SET_SHDN };
	return px4_i2c_drv_transmit(COLOR_LED_I2C_ITF, COLOR_LED_I2C_DEV_ADDR, msg, sizeof(msg));
}

void px4_color_power_led_init()
{
	px4_i2c_drv_init(COLOR_LED_I2C_ITF);
	px4_i2c_drv_set_clock_speed(COLOR_LED_I2C_ITF, COLOR_LED_CLK_SPEED);

	memset(&rgb_color, 0, sizeof(rgb_color));
	memset(&rgb_last,  0, sizeof(rgb_last));

	if (color_power_led_enable() == SUCCESS)
	{
		module_state = ENABLE;
		px4debug("color power led init ok \n");
	}
	else
	{
		px4debug("err at color power led init\n");
	}
}

void px4_color_power_led_update()
{
	if (module_state == DISABLE)
	{
		// nothing to do
		return;
	}

	// switch the storage bank
	uint32_t idx  = storage_idx;
	storage_idx =  (storage_idx + 1) % 2;

	if(	rgb_color[idx].r == rgb_last.r &&
		rgb_color[idx].g == rgb_last.g &&
		rgb_color[idx].b == rgb_last.b)
	{
		// no updates in color
		return;
	}

	uint8_t msg[6] = {  BLUE_REG, 	(uint8_t)rgb_color[idx].b,
						GREEN_REG, 	(uint8_t)rgb_color[idx].g,
						RED_REG, 	(uint8_t)rgb_color[idx].r
					 };

	int result = px4_i2c_drv_transmit(COLOR_LED_I2C_ITF, COLOR_LED_I2C_DEV_ADDR, msg, sizeof(msg));

	// if no errors mark as successful, otherwise try to set color on next call
	if (result != ERROR)
	{
		rgb_last.r = rgb_color[idx].r;
		rgb_last.g = rgb_color[idx].g;
		rgb_last.b = rgb_color[idx].b;
	}
	else
	{
		px4debug("err set rgb color\n");
	}
}

void px4_color_power_led_set(uint32_t r, uint32_t g, uint32_t b)
{
	// rescale from 0...255 to 0...16
	rgb_color[storage_idx].r = (r >> 4);
	rgb_color[storage_idx].g = (g >> 4);
	rgb_color[storage_idx].b = (b >> 4);
}


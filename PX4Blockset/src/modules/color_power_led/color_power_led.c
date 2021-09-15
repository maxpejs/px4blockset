#include "color_power_led.h"

uint32_t module_state = DISABLE;
color_led_data_st rgb[2];
uint32_t storage_idx;
uint32_t updated;

uint8_t i2c_msg[6] = { BLUE_REG, 0U, GREEN_REG, 0U, RED_REG, 0U };

int color_power_led_enable(void)
{
	uint8_t msg[2] = { SUB_ADDR_SETTINGS, SET_ENABLE_DATA | SET_SHDN };
	return px4_i2c_drv_transmit(COLOR_LED_I2C_ITF, COLOR_LED_I2C_DEV_ADDR, msg, sizeof(msg));
}

void px4_color_power_led_init()
{
	px4_i2c_drv_init(COLOR_LED_I2C_ITF);
	px4_i2c_drv_set_clock_speed(COLOR_LED_I2C_ITF, COLOR_LED_CLK_SPEED);

	memset(&rgb, 0, sizeof(rgb));
	storage_idx = 0;
	updated = 0;

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
	if (module_state == DISABLE || !updated)
	{
		// nothing to do
		return;
	}

	if (rgb[0].r == rgb[1].r && rgb[0].g == rgb[1].g && rgb[0].b == rgb[1].b)
	{
		// nothing changed since last call, so nothing to do
		return;
	}

	updated = 0;

	// switch the storage bank
	uint32_t idx = storage_idx;
	storage_idx = (storage_idx + 1) % 2;

	i2c_msg[1] = (uint8_t) rgb[idx].b;
	i2c_msg[3] = (uint8_t) rgb[idx].g;
	i2c_msg[5] = (uint8_t) rgb[idx].r;

	int result = px4_i2c_drv_transmit(COLOR_LED_I2C_ITF, COLOR_LED_I2C_DEV_ADDR, i2c_msg, sizeof(i2c_msg));

	// if no errors mark as successful, otherwise try to set color on next call
	if (result == ERROR)
	{
		px4debug("err set rgb color\n");
	}
}

void px4_color_power_led_set(uint32_t r, uint32_t g, uint32_t b)
{
	// rescale from 0...255 to 0...15, divide by 16 = 2^4 => shift 4 bits to right
	rgb[storage_idx].r = (r >> 4);
	rgb[storage_idx].g = (g >> 4);
	rgb[storage_idx].b = (b >> 4);
	updated = 1;
}


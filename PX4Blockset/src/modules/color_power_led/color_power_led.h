#ifndef COLOR_POWER_LED_H
#define COLOR_POWER_LED_H

#include <i2c_drv.h>
#include <defines.h>
#include <comm_itf.h>

#define COLOR_LED_CLK_SPEED			200000		// 200khz is overclocked!, originaly clock is 100khz
#define COLOR_LED_I2C_DEV_ADDR  	0x55<<1
#define COLOR_LED_I2C_ITF   		PX4_I2C2

#define BLUE_REG			0x81	// Blue, no auto-increment
#define GREEN_REG			0x82	// Green, no auto-increment
#define RED_REG				0x83	// Red, no auto-increment
#define SUB_ADDR_SETTINGS	0x84	// setting address, no auto-increment
#define SET_ENABLE_DATA		0x02
#define SET_SHDN 			0x01

typedef struct
{
	uint32_t r;
	uint32_t g;
	uint32_t b;
} color_led_data_st;

/**
 *	Initializes the module and the rgb driver chip.
 */
void px4_color_power_led_init();

/**
 *	Stores internal the given color channel values. The values will be calculated
 *	to pwm impulses by formula IMPULSE = VALUE * (16/256).
 *	Values below 16 are not applicable.
 *
 *	@param r,b,g	red, green, blue component of 8-Bit color
 */
void px4_color_power_led_set(uint32_t r, uint32_t g, uint32_t b);

/**
 *	Task for triggering the I2C-command to sets rgb led to internal stored channel values.
 *
 *	@param	pointer to argument vector
 */
void px4_color_power_led_update();

#endif // COLOR_POWER_LED_H

#ifndef I2C_DRV_H
#define I2C_DRV_H

#include <stm32f4xx_hal.h>
#include <defines.h>
#include <macros.h>
#include <inttypes.h>
#include <comm_itf.h>

#define PX4_I2C1	0
#define PX4_I2C2	1

uint8_t px4_i2c_drv_init(uint32_t i2c_id);

uint8_t px4_i2c_drv_transmit(uint32_t i2c_id, uint16_t I2C_DEVICE_ADDRESS, uint8_t * txBuff, uint8_t size);

uint8_t px4_i2c_drv_receive(uint32_t i2c_id, uint16_t i2c_address, uint8_t * rxBuff, uint8_t size);

void px4_i2c_drv_set_clock_speed(uint32_t spi_id, uint32_t speed);


#endif	// I2C_DRV_H

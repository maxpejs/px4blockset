#ifndef SPI_DRV_H
#define SPI_DRV_H

#include <stm32f4xx_hal.h>
#include <defines.h>
#include <macros.h>
#include <inttypes.h>
#include <comm_itf.h>

#define PX4_SPI1	0
#define PX4_SPI2	1

#define SPI_DRV_PRESCALER_2		SPI_BAUDRATEPRESCALER_2
#define SPI_DRV_PRESCALER_4		SPI_BAUDRATEPRESCALER_4
#define SPI_DRV_PRESCALER_8		SPI_BAUDRATEPRESCALER_8
#define SPI_DRV_PRESCALER_16	SPI_BAUDRATEPRESCALER_16
#define SPI_DRV_PRESCALER_32	SPI_BAUDRATEPRESCALER_32
#define SPI_DRV_PRESCALER_64	SPI_BAUDRATEPRESCALER_64
#define SPI_DRV_PRESCALER_128	SPI_BAUDRATEPRESCALER_128
#define SPI_DRV_PRESCALER_256	SPI_BAUDRATEPRESCALER_256



/**
 *
 */
uint8_t px4_spi_drv_init(uint32_t spi_id);

/**
 *
 */
uint8_t px4_spi_drv_transmit(uint32_t spi_id, uint8_t * txBuff, uint8_t * rxBuff, uint8_t size);



/**
 *
 */
void px4_spi_drv_set_clock_speed(uint32_t spi_id, uint32_t speed);




#endif	// SPI_DRV_H

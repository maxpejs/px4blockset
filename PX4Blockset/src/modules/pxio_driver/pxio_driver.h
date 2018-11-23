#ifndef PXIO_DRIVER_H
#define PXIO_DRIVER_H

#include <stm32f4xx_hal.h>
#include <defines.h>
#include <error_handler.h>
#include <protocol.h>
#include <inttypes.h>
#include <string.h>
#include <comm_itf.h>
#include <timestamp.h>

#define DMA_TXRX_TIMEOUT		2000	// 2ms
#define TRANSMIT_TRIES_CNT		2
#define FMU_IO_BAUDRATE 		1500000
#define MAX_TRANSFER 			PKT_MAX_REGS * 2

/**
 * initialization of serial interface fo communication with pxio
 */
void pxio_driver_init(void);

/**
 * modifies single register values of pxio
 */
int32_t pxio_driver_reg_mod(uint8_t page, uint8_t offset, uint16_t clearbits, uint16_t setbits);

/**
 * reads register values of pxio
 */
int32_t pxio_driver_reg_get(uint8_t page, uint8_t offset, uint16_t *values, unsigned num_values);

/**
 * writes new values to pxio register
 */
int32_t pxio_driver_reg_set(uint8_t page, uint8_t offset, uint16_t *values, unsigned num_values);

#endif // PXIO_DRIVER_H



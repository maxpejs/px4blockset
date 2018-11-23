/* @file driver for ms5611 (barometer) */

#ifndef MS5611_H
#define MS5611_H 

#include <comm_itf.h>
#include <defines.h>
#include <inttypes.h>
#include <spi_drv.h>
#include <error_handler.h>
#include <timestamp.h>

#define MS5611_MSG_BUFFERSIZE 12 + 1

#define MS5611_ENA       HAL_GPIO_WritePin(GPIOD, GPIO_PIN_7, RESET)
#define MS5611_DISA      HAL_GPIO_WritePin(GPIOD, GPIO_PIN_7, SET)

// registers
#define CMD_MS5611_RESET 		0x1E
#define REG_MS5611_PROM_BASE 	0xA0	// 	PROM memory base address

#define REG_MS5611_PROM_C1  	REG_MS5611_PROM_BASE | (1 << 1)		// 	C1 coefficient
#define REG_MS5611_PROM_C2 		REG_MS5611_PROM_BASE | (2 << 1)		// 	C2 coefficient
#define REG_MS5611_PROM_C3 		REG_MS5611_PROM_BASE | (3 << 1)		// 	C3 coefficient
#define REG_MS5611_PROM_C4 		REG_MS5611_PROM_BASE | (4 << 1)		// 	C4 coefficient
#define REG_MS5611_PROM_C5 		REG_MS5611_PROM_BASE | (5 << 1)		// 	C5 coefficient
#define REG_MS5611_PROM_C6 		REG_MS5611_PROM_BASE | (6 << 1)		// 	C6 coefficient

// ....
#define REG_MS5611_D1_OSR_256 	0x40
#define REG_MS5611_D1_OSR_512 	0x42
#define REG_MS5611_D1_OSR_1024 	0x44
#define REG_MS5611_D1_OSR_2048 	0x46
#define REG_MS5611_D1_OSR_4096 	0x48

// ....
#define REG_MS5611_D2_OSR_256	0x50
#define REG_MS5611_D2_OSR_512 	0x52
#define REG_MS5611_D2_OSR_1024 	0x54
#define REG_MS5611_D2_OSR_2048	0x56
#define REG_MS5611_D2_OSR_4096 	0x58

/**
*	measurement data storage struct
*/
typedef struct
{
	float 		baroValue;		// pressure (mbar)
	float 		temp;			// temperature (celcius)
	uint32_t	isNew;			// flag shows if data set contains new values
}ms5611_data_st;

/**
* TODO
*/
void px4_ms5611_init(void);

/**
* TODO
*/
void px4_ms5611_get(ms5611_data_st * data);

/**
 * TODO
 */
void px4_ms5611_update();

#endif // MS5611_H

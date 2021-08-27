/* driver for mpu6000 */

#ifndef MPU6000_H
#define MPU6000_H 

#include <spi_drv.h>
#include <defines.h>
#include <macros.h>
#include <inttypes.h>
#include <comm_itf.h>
#include <error_handler.h>
#include <timestamp.h>

#define MPU_MSG_BUFFERSIZE 15

#define MPU6000_ENA       HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, RESET)
#define MPU6000_DISA      HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, SET)

#define MPU6000_CFG		    	0x1A

#define DLPF_CFG_256HZ         	0x00
#define DLPF_CFG_188HZ         	0x01
#define DLPF_CFG_98HZ          	0x02
#define DLPF_CFG_42HZ          	0x03
#define DLPF_CFG_20HZ          	0x04
#define DLPF_CFG_10HZ         	0x05
#define DLPF_CFG_5HZ          	0x06

// registers
#define MPU6000_SMPLRT_DIV	   	0x19
#define MPU6000_GYRO_CONFIG	   	0x1B
#define MPU6000_ACCEL_CONFIG  	0x1C

#define ACCEL_XOUT_H 			0x3B
#define ACCEL_XOUT_L 			0x3C
#define ACCEL_YOUT_H 			0x3D
#define ACCEL_YOUT_L 			0x3E
#define ACCEL_ZOUT_H 			0x3F
#define ACCEL_ZOUT_L    		0x40
#define TEMP_OUT_H	    		0x41
#define TEMP_OUT_L	    		0x42
#define GYRO_XOUT_H	    		0x43
#define GYRO_XOUT_L	    		0x44
#define GYRO_YOUT_H	    		0x45
#define GYRO_YOUT_L	     		0x46
#define GYRO_ZOUT_H	    		0x47
#define GYRO_ZOUT_L	    		0x48

#define USER_CTRL	    		0x6A
#define PWR_MGMT_1	    		0x6B
#define PWR_MGMT_2	    		0x6C

#define SIGNAL_PATH_RESET		0x68

#define WHOAMI		    		0x75
#define WHOAMI_MPU6000_ID		0x68

// register bits
#define BIT_SLEEP				0x40
#define BIT_H_RESET				0x80
#define BITS_CLKSEL				0x07
#define MPU_CLK_SEL_PLLGYROX	0x01
#define MPU_CLK_SEL_PLLGYROZ	0x03
#define MPU_EXT_SYNC_GYROX		0x02

#define BITS_FS_250DPS          0x00
#define BITS_FS_500DPS          0x08
#define BITS_FS_1000DPS         0x10
#define BITS_FS_2000DPS         0x18
#define BITS_FS_2G              0x00
#define BITS_FS_4G              0x08
#define BITS_FS_8G              0x10
#define BITS_FS_16G             0x18

#define BIT_I2C_IF_DIS       	0x10
#define BIT_READ_REG 			0x80

#define BIT_GYRO_ACCEL_TEMP_RESET 	0x07

/**
 * 	struct with settings for mpu600 setup
 */
typedef struct
{
	uint32_t accel_range;	// data range for accelerometer. Use defines, e.g. symbol BITS_FS_2G for +-2g
	uint32_t gyro_range;	// data range for gyroscope. Use defines e.g. symbol BITS_FS_250DPS for +-250°/s
	uint32_t dlpf_cfg;		// cutoff frequency for low pass filter setting. Use defines, e.g. BITS_DLPF_CFG_5HZ
	uint32_t smplrt_cfg;// Divider for measurement calculation ratio inside mpu6000. 0x00 means full speed => 8kHz for accel + 1kHz for gyro

	float scale_accel_x;	// Correction factor for acceleration along x axis
	float scale_accel_y;	// ... y axis
	float scale_accel_z;	// ... z axis

	float offset_accel_x;	// Offset for acceleration along x axis
	float offset_accel_y;	// ... y axis
	float offset_accel_z;	// ... z axis
} mpu6000_settings_st;

/**
 *	measurement data storage struct
 */
typedef struct
{
	float accel_x;	// acceleration along x axis
	float accel_y;
	float accel_z;
	float temp;		// temperature in Celcius
	float gyro_x;	// angular speed in °/s around x axis
	float gyro_y;
	float gyro_z;
} mpu6000_data_st;

/**
 * initialize module
 */
void px4_mpu6000_init(mpu6000_settings_st * in_settings);

/**
 * get last data received from mpu6000 chip
 */
void px4_mpu6000_get(mpu6000_data_st * data);

/**
 * 	function for reading the new values from mpu6000 sensor over spi
 */
void px4_mpu6000_update();

#endif // MPU6000_H

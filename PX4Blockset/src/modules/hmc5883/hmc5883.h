/* driver for hmc5883 (compass) */

#ifndef HMC5883_H
#define HMC5883_H

#include <i2c_drv.h>
#include <defines.h>
#include <comm_itf.h>
#include <macros.h>

#define HMC5883_I2C_ITF   				PX4_I2C1
#define HMC5883_CLOCK_SPEED				400000
#define HMC5883_I2C_DEVICE_ADDRESS  	0x1E<<1

#define HMC5883_REG_CONFA 				0x00
#define HMC5883_REG_CONFB 				0x01
#define HMC5883_REG_MODE 				0x02
#define HMC5883_REG_MAG_OUT_X_HB       	0x03

/**
 * enum for gain settings
 */
typedef enum
{
	HMC5883_RANGE_0_88 = 0x00,  // +/- 0.88
	HMC5883_RANGE_1_3  = 0x20,  // +/- 1.3
	HMC5883_RANGE_1_9  = 0x40,  // +/- 1.9
	HMC5883_RANGE_2_5  = 0x60,  // +/- 2.5
	HMC5883_RANGE_4_0  = 0x80,  // +/- 4.0
	HMC5883_RANGE_4_7  = 0xA0,  // +/- 4.7
	HMC5883_RANGE_5_6  = 0xC0,  // +/- 5.6
	HMC5883_RANGE_8_1  = 0xE0   // +/- 8.1
} e_hmc5883_mag_range_t;

/**
* 	struct with settings for hmc5883 chip setup
 */
typedef struct
{
	e_hmc5883_mag_range_t magRange;
} hmc5883_settings_st;

/**
 *	Struct with compass measurements
 */
typedef struct
{
	float 		magX;	// magnitude along x axis
	float 		magY;	// ... y axis
	float 		magZ;	// ... z axis
	uint32_t 	isNew;
}hmc5883_data_st;


/**
 *	Initializes the module driver and the compass chip
 */
void px4_hmc5883_init(hmc5883_settings_st * in_settings);

/**
*
*/
void px4_hmc5883_get(hmc5883_data_st * data);

/**
*
*/
void px4_hmc5883_update();


#endif // HMC5883_H

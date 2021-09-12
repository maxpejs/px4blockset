/* module for receiving rc input data from rc transmitter conneted to pixhawk (pxio) */
#pragma once

#include "pxio_driver.h"
#include "timestamp.h"

#define RC_MAX_CHANNELS 18

/**
 * TODO
 */
typedef struct
{
	uint16_t  channels[RC_MAX_CHANNELS];
	size_t 	  rc_failsafe;
	size_t    rc_lost;
	size_t    rc_lost_frame_count;
	size_t    rc_total_frame_count;
	size_t    channel_cnt;
}rc_ppm_data_st;


/**
* TODO
*/
void px4_rc_ppm_init(void);

/**
* TODO
*/
void px4_rc_ppm_get(rc_ppm_data_st * data);

/**
 * TODO
 */
void px4_rc_ppm_update();

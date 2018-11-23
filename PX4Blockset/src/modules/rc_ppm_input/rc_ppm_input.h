/* module for receiving rc input data from rc transmitter conneted to pixhawk (pxio) */

#ifndef RC_PPM_INPUT_H
#define RC_PPM_INPUT_H 

#include "pxio_driver.h"
#include "timestamp.h"

#define MAX_RC_CHANNEL_CNT 		18
#define CHANNEL_PART 			10

/**
 * TODO
 */
typedef struct
{
	uint16_t  channels[MAX_RC_CHANNEL_CNT];
	uint8_t   channel_cnt;
}rc_ppm_input_data_st;


/**
* TODO
*/
void px4_rc_ppm_input_init(void);

/**
* TODO
*/
void px4_rc_ppm_input_get(rc_ppm_input_data_st * data);

/**
 * TODO
 */
void px4_rc_ppm_input_update();


#endif // RC_PPM_INPUT_H

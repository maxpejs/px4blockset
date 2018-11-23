#ifndef SD_CARD_LOGGER_H
#define SD_CARD_LOGGER_H

#include <stm32f4xx_hal.h>
#include <defines.h>
#include <string.h>
#include <timestamp.h>
#include <comm_itf.h>


/**
* 	TODO
*/
void px4_sd_card_logger_init();


/**
 *	TODO
 */
uint32_t px4_sd_card_logger_add_new_logger(uint32_t sample_time, uint32_t sig_cnt, char * file_name);

/**
 *
 */
void px4_sd_card_logger_add_val(uint32_t id, float * values);

/**
*	TODO
*/
void px4_sd_card_logger_task(void);

/**
 * TODO
 */
void px4_sd_card_logger_add_user_cmd(const char * cmd);

#endif // SD_CARD_LOGGER_H









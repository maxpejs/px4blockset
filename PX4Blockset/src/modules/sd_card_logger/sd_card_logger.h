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
 *
 */
void px4_sd_card_logger_add_val(float * values);

/**
*	TODO
*/
void px4_sd_card_logger_task(void);

/**
 * TODO
 */
void px4_sd_card_logger_add_user_cmd(const char * cmd);

#endif // SD_CARD_LOGGER_H









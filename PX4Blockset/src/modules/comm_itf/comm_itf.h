#ifndef COMM_ITF_H
#define COMM_ITF_H

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cmsis_os.h>

#include <stm32f4xx_hal.h>
#include <defines.h>
#include <error_handler.h>
#include <cpu_load.h>
#include <mpu6000.h>
#include <hmc5883.h>
#include <gps.h>
#include <rc_ppm_input.h>
#include <pwm_main_out.h>
#include <pwm_aux_out.h>
#include <color_power_led.h>
#include <signal_output.h>
#include <sd_card_logger.h>
#include <ms5611.h>
#include <utilities.h>
#include <tasks.h>

/* regular functions for printing any pixhawk sensor informations like calibration info */
void comm_itf_init();
void comm_itf_rx_complete_event(void);
void comm_itf_task_function();

/**
 *
 */
void px4debug(eTaskID id, char * MESSAGE, ...);

/* print functions */
void comm_itf_print_string(const char * str);


#endif // COMM_ITF_H

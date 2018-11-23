/* driver for pwm aux out ports */
 
#ifndef PWM_AUX_OUT_H
#define PWM_AUX_OUT_H

#include <stm32f4xx_hal.h>
#include <defines.h>
#include <inttypes.h>
#include <comm_itf.h>

#define MAX_AUX_OUT_SERVO_CNT 		6
#define PWM_AUX_MIN_RATE 			20
#define PWM_AUX_MAX_RATE 			400
#define PWM_AUX_OUT_IMPULSE_MIN		90
#define PWM_AUX_OUT_IMPULSE_MAX		2500

/**
 * Initializes the ports and timers for aux pwms of pixhawk. There are two group of aux pwm ports:
 * 1st Group: ports 1-4
 * 2nd Group: ports 5-6
 * It is possible to set the pwm frequency for whole group, but not(!) for single pwm port
 *
 * Parameter:
 * 				- pwm_freqs:	Array with two(!) elements, contains pwm frequency values. Values will be limited
 * 								inside the function by the range 20Hz-400Hz
 */
void px4_pwm_aux_out_init(uint16_t * pwm_freqs);

/**
 * Writes new impulse values to pwm ports
 *
 * Parameter: 
 * impulse_values:	Array with six(!) elements, contains pwm impulse values in µs.
 */
void px4_pwm_aux_out_set(uint16_t * impulse_values);

/**
*	Provides pwm impulses stored by "px4_pwm_aux_out_write_impulse" to hardware
*/
void px4_pwm_aux_out_update();

#endif // PWM_AUX_OUT_H

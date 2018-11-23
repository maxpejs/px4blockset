/* module for writing the impulse values to pwm main out ports of pixhawk */

#ifndef PWM_MAIN_OUT_H
#define PWM_MAIN_OUT_H

#include <inttypes.h>

// pwm main output
#define MAX_MAIN_OUT_SERVO_CNT 		8
#define PWM_MAIN_OUT_IMPULSE_MIN 	90 	 /* Lowest minimum PWM in us */
#define PWM_MAIN_OUT_IMPULSE_MAX 	2500 /* Highest maximum PWM in us */

/**
*	settings for initialization of main pwm output ports
*/
typedef struct
{
	uint16_t def_pwm_rate;	// Default pwm rate
	
	uint16_t alt_pwm_rate; 	// Alternate pwm rate
	
	uint16_t pwm_mask;		/* Mask for switching between default and alternate pwm rates
							 * 		Not every mask value is valid. Possible values are
							 * 		-0b00000011 channels 1 and 2 to alternate pwm rate, all other to default rate
							 * 		-0b00001100 channels 3 and 4 to alternate pwm rate, all other to default rate
							 * 		-0b11110000 channels 5 to  8 to alternate pwm rate, all other to default rate 
							 */
}pwm_main_out_settings_st;

/**
*	data struct for setting main out ports
*/
typedef struct
{
	uint16_t values[MAX_MAIN_OUT_SERVO_CNT];	// Array with ipulse values
	uint8_t safetyoff;							// SafetyOff flag
	uint8_t arm;								// Arming flag
}pwm_main_out_data_st;

/**
*	Initialization routine for pwm main channels of pixhawk.
*/
void px4_pwm_main_out_init(pwm_main_out_settings_st * settings);

/**
*	Set new values
*/
void px4_pwm_main_out_set(pwm_main_out_data_st * data);

/**
*	Set pins to given pwm rates
*/
void px4_pwm_main_out_update();


#endif // PWM_MAIN_OUT_H

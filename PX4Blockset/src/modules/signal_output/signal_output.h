#ifndef SIGNAL_OUTPUT_H
#define SIGNAL_OUTPUT_H
#include <inttypes.h>

/**
*	TODO
*/
void px4_signal_output_init(uint32_t sample_time_ms, uint32_t sig_cnt);

/**
*	TODO
*/
void px4_signal_output_set(float * values);

/**
*	TODO
*/
void px4_signal_output_task(void);

/**
*	TODO
*/
void px4_signal_output_set_log(uint8_t state);

#endif // SIGNAL_OUTPUT_H

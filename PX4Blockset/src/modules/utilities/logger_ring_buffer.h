#ifndef LOGGER_RING_BUFFER_H
#define LOGGER_RING_BUFFER_H
#include <inttypes.h>

#define SIGNAL_MAX_CNT 		10
#define RING_BUFF_SIZE		200

typedef struct
{
	uint32_t timestamp;
	float val[SIGNAL_MAX_CNT];
}log_dataset_st;

typedef struct
{
	uint32_t 		read;
	uint32_t 		write;
	log_dataset_st 	buff[RING_BUFF_SIZE];
}ring_buff_data_st;

/**
*	TODO
*/
uint32_t ring_buffer_full(ring_buff_data_st * b);

/**
*	TODO
*/
uint32_t ring_buffer_empty(ring_buff_data_st * b);

/**
*	TODO
*/
uint32_t ring_buffer_free_space(ring_buff_data_st * b);

/**
*	TODO
*/
uint32_t ring_buffer_count(ring_buff_data_st * b);



#endif // LOGGER_RING_BUFFER_H

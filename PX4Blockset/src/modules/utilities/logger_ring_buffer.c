#include "logger_ring_buffer.h"

uint32_t ring_buffer_full(ring_buff_data_st * b)
{
	return ((b->write + 1) % RING_BUFF_SIZE) == b->read;
}

uint32_t ring_buffer_empty(ring_buff_data_st * b)
{
	return b->read == b->write;
}

uint32_t ring_buffer_free_space(ring_buff_data_st * b)
{
	return (RING_BUFF_SIZE + b->read - b->write - 1) % RING_BUFF_SIZE;
}

uint32_t ring_buffer_count(ring_buff_data_st * b)
{
	return (RING_BUFF_SIZE + b->write - b->read ) % RING_BUFF_SIZE;
}

uint32_t ring_buffer_add(ring_buff_data_st * b)
{
	return (RING_BUFF_SIZE + b->write - b->read ) % RING_BUFF_SIZE;
}

uint32_t ring_buffer_get(ring_buff_data_st * b)
{
	return (RING_BUFF_SIZE + b->write - b->read ) % RING_BUFF_SIZE;
}

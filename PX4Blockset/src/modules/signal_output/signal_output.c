#include <timestamp.h>
#include "signal_output.h"
#include <defines.h>
#include <comm_itf.h>
#include <logger_ring_buffer.h>

#define SIGNAL_MAX_CNT 		10

static uint32_t _sig_cnt 		= 0U;
static uint32_t _sample_time_us	= 0U;
static uint32_t _log_enabled 	= 0U;
static uint64_t _tick_last_call = 0U;
static uint32_t _runtime		= 0U;
static uint32_t _sample_cnt		= 0U;

static uint32_t  _module_state 	= DISABLE;

static ring_buff_data_st rbuff;

void px4_signal_output_init(uint32_t sample_time_ms, uint32_t sig_cnt)
{
	_sig_cnt = (sig_cnt > SIGNAL_MAX_CNT) ? SIGNAL_MAX_CNT : sig_cnt;
	_sample_time_us = sample_time_ms * 1000;
	memset(&rbuff, 0 , sizeof(ring_buff_data_st));
	
	_module_state = ENABLE;
}

void px4_signal_output_set(float * values)
{
	uint32_t ts = tic();
	
	if(!_log_enabled)
	{
		// signal log is deactivated, nothing to do
		return;
	}
	
	// check if signal plot cycle time elapsed
	if (ts  < (_sample_time_us * _sample_cnt))
	{
		return;
	}
	
	// Workaround! Init counter of samples for exact timing for 
	// adding values according to sampling time
	if(_sample_cnt == 0)
	{
		_sample_cnt = ts / _sample_time_us;
	}
	
	_sample_cnt++;
	
	if (_ring_buffer_free_space(&rbuff) > 0)
	{
		memcpy(&(rbuff.buff[rbuff.write].val), values, sizeof(float) * _sig_cnt);
		rbuff.buff[rbuff.write].timestamp = ts;

		_tick_last_call = rbuff.buff[rbuff.write].timestamp;
		rbuff.write++;
		rbuff.write %= RING_BUFF_SIZE;
	}
	else
	{
		debug_print_string("signal logger. not enough space in buffer\r\n");
	}
}

void px4_signal_output_task(void)
{
	uint64_t timestampt = tic();

	if (_module_state == DISABLE)
	{
		return;
	}

	if (_ring_buffer_empty(&rbuff))
	{
		return; // no values to plot
	}

	while(!_ring_buffer_empty(&rbuff))
	{
		uint32_t tm = rbuff.buff[rbuff.read].timestamp;
	
		char arr[30];
		snprintf(arr, sizeof(arr), "t(s):%d.%03d", (int) (tm / 1000000), (int) ((tm / 1000) % 1000));
		comm_itf_print_string(arr);

		for (uint32_t i = 0; i < _sig_cnt; i++)
		{
			// sprintf(arr, " [%d]:%f\n", (int)i+1, arr_copy[i]);
			// comm_itf_print_string(arr);

			// TODO: use sprintf instead
			comm_itf_print_string(" [");
			comm_itf_print_int(i + 1);
			comm_itf_print_string("]:");
			comm_itf_print_float(rbuff.buff[rbuff.read].val[i]);
		}
		
		comm_itf_print_string("\r\n");
		rbuff.read++;
		rbuff.read %= RING_BUFF_SIZE;
	}

	_runtime = (uint32_t)toc(timestampt);
}

// TODO: strange section
void px4_signal_output_set_log(uint8_t state)
{
	if(state == ENABLE)
	{
		_log_enabled = !_log_enabled;
	}
	else
	{
		_log_enabled = state;
	}
}

uint32_t px4_signal_output_getruntime(void)
{
	return _runtime;
}





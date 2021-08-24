#include <timestamp.h>
#include "signal_output.h"
#include <defines.h>
#include <comm_itf.h>
#include <logger_ring_buffer.h>

static uint32_t _sig_cnt 		= 0U;
static uint32_t _sample_time_us	= 0U;
static uint32_t _log_enabled 	= 0U;
static uint64_t _tick_last_call = 0U;
static uint32_t _sample_cnt		= 0U;

static uint32_t  _module_state 	= DISABLE;

static ring_buff_data_st rbuff;

void px4_signal_output_init(uint32_t sample_time_ms, uint32_t sig_cnt)
{
	_sig_cnt = (sig_cnt > SIGNAL_MAX_CNT) ? SIGNAL_MAX_CNT : sig_cnt;
	_sample_time_us = sample_time_ms * 1000;
	memset(&rbuff, 0 , sizeof(ring_buff_data_st));
	
	px4debug(eCOMMITF, "signal logger init ok \r\n");
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
	
	if (ring_buffer_free_space(&rbuff) > 0)
	{
		memcpy(&(rbuff.buff[rbuff.write].val), values, sizeof(float) * _sig_cnt);
		rbuff.buff[rbuff.write].timestamp = ts;

		_tick_last_call = rbuff.buff[rbuff.write].timestamp;
		rbuff.write++;
		rbuff.write %= RING_BUFF_SIZE;
	}
	else
	{
		px4debug(eCOMMITF, "signal logger. not enough space in buffer\r\n");
	}
}

void px4_signal_output_task(void)
{
	if (_module_state == DISABLE || ring_buffer_empty(&rbuff))
	{
		return;
	}

	while (!ring_buffer_empty(&rbuff))
	{
		uint32_t tm = rbuff.buff[rbuff.read].timestamp;

		px4debug(eCOMMITF, "t(s):%d.%03d", (tm / 1000000), ((tm / 1000) % 1000));

		for (uint32_t i = 0; i < _sig_cnt; i++)
		{
			px4debug(eCOMMITF, " [%d]:%f", (i + 1), rbuff.buff[rbuff.read].val[i]);
		}

		px4debug(eCOMMITF, "\r\n");
		rbuff.read++;
		rbuff.read %= RING_BUFF_SIZE;
	}
}

void px4_signal_output_set_log(uint8_t state)
{
	if(state == ENABLE)
	{
		_log_enabled = !_log_enabled; // ENABLE is use for toggle
	}
	else
	{
		_log_enabled = state;
	}
}


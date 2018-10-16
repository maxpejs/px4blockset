#include "signal_output.h"

#define SIGNAL_MAX_CNT 		10

static float arr[SIGNAL_MAX_CNT];

static uint32_t _sig_cnt 		= 0U;
static uint32_t _sample_time	= 0U;
static uint32_t _log_enabled 	= 0U;
static uint64_t _tick_last_call = 0U;
static uint32_t _runtime		= 0;

static uint32_t  _module_state 	= DISABLE;

void px4_signal_output_init(uint32_t sample_time, uint32_t sig_cnt)
{
	_sig_cnt = (sig_cnt > SIGNAL_MAX_CNT) ? SIGNAL_MAX_CNT : sig_cnt;
	_sample_time = sample_time * 1000;
	_module_state = ENABLE;
}

void px4_signal_output_set(float * values)
{
	memcpy(arr, values, sizeof(float) * _sig_cnt);
}

void px4_signal_output_task(void)
{
	uint64_t timestampt = tic();

	if (_module_state == DISABLE)
	{
		return;
	}

	if(!_log_enabled)
	{
		// signal log is deactivated, nothing to do
		return;
	}

	// check if cycle time elapsed
	if ((uint32_t)toc(_tick_last_call) < _sample_time)
	{
		return;
	}

	_tick_last_call = timestampt;

	/* copy data into temp array to keep time for minimize time inside critical section */
	float arr_copy[SIGNAL_MAX_CNT];
	
	memcpy(arr_copy, arr, sizeof(float) * _sig_cnt);

	char arr[30];
	snprintf(arr, sizeof(arr), "t(s):%d.%03d", (int) (_tick_last_call / 1000000), (int) ((_tick_last_call / 1000) % 1000));
	comm_itf_print_string(arr);

	for (uint32_t i = 0; i < _sig_cnt; i++)
	{
		// sprintf(arr, " [%d]:%f\n", (int)i+1, arr_copy[i]);
		// comm_itf_print_string(arr);

		// TODO: use sprintf instead
		comm_itf_print_string(" [");
		comm_itf_print_int(i + 1);
		comm_itf_print_string("]:");
		comm_itf_print_float(arr_copy[i]);


	}
	comm_itf_print_string("\r\n");

	_runtime = (uint32_t)toc(timestampt);
}

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





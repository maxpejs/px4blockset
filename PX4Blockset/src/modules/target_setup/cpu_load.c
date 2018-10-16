#include "cpu_load.h"

static uint64_t _start 			= 0;
static uint32_t _sampleTime_us 	= 0;
static uint32_t _filt_depth 	= 100;
static float _cpu_load_max 		= 0.f;
static float _cpu_load_filt 	= 0.f;
static float _cpu_load_act 		= 0.f;

void cpu_load_init(uint32_t sampleTime_ms)
{
	_sampleTime_us = sampleTime_ms * 1000U;
	timestamp_init();
}

void cpu_load_start_meas(void)
{
	_start = tic();
}

void cpu_load_stop_meas(void)
{
	uint32_t runtime = (uint32_t)toc(_start);
	
	_cpu_load_act = (float)(( (float)runtime * 100.0f) / (float)_sampleTime_us); // cpu usage in %
	
	if(_cpu_load_act > _cpu_load_max)
		_cpu_load_max = _cpu_load_act;
	
	_cpu_load_filt = ((_cpu_load_filt * (_filt_depth - 1)) + _cpu_load_act) / _filt_depth;
}

uint32_t cpu_load_get_curr_cpu_load(void)
{
	return (uint32_t)_cpu_load_filt;
}

uint32_t cpu_load_get_max_cpu_load(void)
{
	return (uint32_t)_cpu_load_max;
}

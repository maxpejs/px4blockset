#include "cpu_load.h"
#include "string.h"

static uint32_t _filt_depth 		= 20;
static float _cpu_load_max 			= 0.f;
static float _cpu_load_filt 		= 0.f;
static float _cpu_load_act 			= 0.f;

void cpu_load_update(void const * argv)
{
	TaskStatus_t val[10];
	uint32_t total = 1;
	UBaseType_t size = uxTaskGetSystemState((TaskStatus_t * const ) &val, 10, &total);

	uint32_t cpuload = 0;

	for (uint32_t i = 0; i < size; i++)
	{
		if(strcmp(val[i].pcTaskName, "IDLE") == 0)
		{
			continue; // skip
		}
		// debug_print_int(val[i].xTaskNumber);
		// debug_print_int(val[i].usStackHighWaterMark);
		cpuload += val[i].ulRunTimeCounter;
	}

	_cpu_load_act = ((uint64_t) cpuload * 100) / total;

	_cpu_load_filt = ((_cpu_load_filt * (_filt_depth - 1)) + _cpu_load_act) / _filt_depth;

	if (_cpu_load_filt > _cpu_load_max)
		_cpu_load_max = _cpu_load_filt;
}

uint32_t cpu_load_get_curr_cpu_load(void)
{
	return (uint32_t)_cpu_load_filt;
}

uint32_t cpu_load_get_max_cpu_load(void)
{
	return (uint32_t)_cpu_load_max;
}

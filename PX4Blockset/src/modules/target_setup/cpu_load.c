#include "cpu_load.h"
#include "string.h"

static uint32_t _cpu_load_max = 0;
static uint32_t _cpu_load_act = 0;

void cpu_load_update()
{
	static uint32_t last_summ_RunTimeCounter = 0;
	static uint32_t last_total = 0;


	TaskStatus_t val[10];
	uint32_t total = 1;
	UBaseType_t size = uxTaskGetSystemState((TaskStatus_t * const ) &val, 10, &total);

	uint32_t cpuload = 0;

	for (uint32_t i = 0; i < size; i++)
	{
		if(strcmp(val[i].pcTaskName, "IDLE") == 0)
			continue; // skip

		cpuload += val[i].ulRunTimeCounter;
	}

	_cpu_load_act = ((uint64_t) (cpuload - last_summ_RunTimeCounter) * 100) / (total - last_total);

	if (_cpu_load_act > _cpu_load_max)
		_cpu_load_max = _cpu_load_act;

	last_total = total;
	last_summ_RunTimeCounter = cpuload;
}

uint32_t cpu_load_get_curr_cpu_load(void)
{
	return _cpu_load_act;
}

uint32_t cpu_load_get_max_cpu_load(void)
{
	return _cpu_load_max;
}

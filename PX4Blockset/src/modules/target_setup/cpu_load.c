#include "cpu_load.h"
#include "string.h"

static uint32_t cpu_load_max = 0;
static uint32_t cpu_load_act = 0;

void cpu_load_update()
{
	static uint32_t last_cpu_load = 0;
	static uint32_t last_os_runtime_total = 0;

	TaskStatus_t val[10];
	uint32_t os_runtime_total = 1;
	UBaseType_t size = uxTaskGetSystemState((TaskStatus_t * const ) &val, 10, &os_runtime_total);

	uint32_t cpu_load = 0;

	for (uint32_t i = 0; i < size; i++)
	{
		if(strcmp(val[i].pcTaskName, "IDLE") == 0)
		{
			continue; // skip
		}

		cpu_load += val[i].ulRunTimeCounter;
	}

	cpu_load_act = ((cpu_load - last_cpu_load) * 100) / (os_runtime_total - last_os_runtime_total);

	if (cpu_load_act > cpu_load_max)
		cpu_load_max = cpu_load_act;

	last_os_runtime_total = os_runtime_total;
	last_cpu_load = cpu_load;
}

uint32_t cpu_load_get_curr_cpu_load(void)
{
	return cpu_load_act;
}

uint32_t cpu_load_get_max_cpu_load(void)
{
	return cpu_load_max;
}

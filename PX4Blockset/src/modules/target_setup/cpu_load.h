#ifndef CPULOAD_H
#define CPULOAD_H

#include "inttypes.h"
#include <cmsis_os.h>

/**
*	TODO
*/
void cpu_load_update();

/**
*	TODO
*/
uint32_t cpu_load_get_curr_cpu_load(void);

/**
* TODO
*/
uint32_t cpu_load_get_max_cpu_load(void);

#endif // CPULOAD_H


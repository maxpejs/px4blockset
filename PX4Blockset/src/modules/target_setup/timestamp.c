#include "timestamp.h"

static uint32_t fac = 0;

void timestamp_init(void)
{
	 fac = HAL_RCC_GetSysClockFreq()/1000000;
}

uint64_t calc_on_overrun(uint64_t tic, uint64_t toc)
{
	uint64_t tic_us = tic % 1000;
	uint64_t toc_us = toc % 1000;
	
	uint64_t tic_ms = tic / 1000;
	uint64_t toc_ms = toc / 1000;
	
	uint64_t ret = ((uint64_t)UINT32_MAX - tic_ms) + toc_ms;

	ret += (1000 - tic_us) + toc_us;
	
	return ret;
}

uint64_t getCurrentTimeStamp_uS()
{
	uint64_t ms = HAL_GetTick();	// milliseconds since cpu start
	uint64_t us = (SysTick->LOAD - SysTick->VAL) / fac; // elapsed time in 탎econds since new millisecond has begun
	
	// return timestamp in 탎econds since cpu start
	return (ms * 1000U) + us;
}

// returns timestamp in 탎econds
uint64_t tic()
{
	return getCurrentTimeStamp_uS();
}

// calculates difference in 탎econds from given tic
uint64_t toc(uint64_t tic)
{
	uint64_t toc = getCurrentTimeStamp_uS();

	if(tic <= toc)
		return toc-tic;
	else
		return calc_on_overrun(tic, toc); // occurs once after ~49 days runtime
}

void delay_us(uint32_t us)
{
	uint64_t start = tic();

	do
	{}
	while((uint32_t)toc(start) < us);
}

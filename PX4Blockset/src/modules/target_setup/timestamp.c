#include "timestamp.h"

static uint32_t RTOS_Timer_offset = 0;
static TIM_HandleTypeDef TimHandleTimeStamp;

ErrorStatus timestamp_init(void)
{
	RCC_ClkInitTypeDef clkconfig;
	uint32_t uwTimclock, uwAPB1Prescaler = 0U;
	uint32_t uwPrescalerValue = 0U;
	uint32_t pFLatency;

	__HAL_RCC_TIM5_CLK_ENABLE();

	HAL_RCC_GetClockConfig(&clkconfig, &pFLatency);

	uwAPB1Prescaler = clkconfig.APB1CLKDivider;

	if (uwAPB1Prescaler == RCC_HCLK_DIV1)
	{
		uwTimclock = HAL_RCC_GetPCLK1Freq();
	}
	else
	{
		uwTimclock = 2 * HAL_RCC_GetPCLK1Freq();
	}

	/* Compute the prescaler value to have TIM6 counter clock equal to 1MHz (count every 1µs) */
	uwPrescalerValue = (uint32_t) ((uwTimclock / 1000000U) - 1U);

	/* Initialize TIM6 */
	TimHandleTimeStamp.Instance = TIM5;
	TimHandleTimeStamp.Init.Period = UINT32_MAX;
	TimHandleTimeStamp.Init.Prescaler = uwPrescalerValue;
	TimHandleTimeStamp.Init.ClockDivision = 0;
	TimHandleTimeStamp.Init.CounterMode = TIM_COUNTERMODE_UP;

	if (HAL_TIM_Base_Init(&TimHandleTimeStamp) != HAL_OK)
	{
		/* Start the TIM time Base generation in interrupt mode */
		return ERROR;
	}

	if (HAL_TIM_Base_Start(&TimHandleTimeStamp) != HAL_OK)
	{
		return ERROR;
	}

	/* Return function status */
	return SUCCESS;

}

// returns current time since cpu start
uint32_t tic()
{
	return TimHandleTimeStamp.Instance->CNT;
}

// calculates difference in µseconds from given tic
uint32_t toc(uint32_t t1)
{
	uint32_t ret = 0;
	uint32_t t2 = TimHandleTimeStamp.Instance->CNT;

	if(t1 <= t2)
		ret = t2 - t1;
	else
		ret = (UINT32_MAX - t2) + t1; // occurs after 1,19 hours

	return ret;
}

/************************/
/* RTOS Hooks */
void vHookSetupTimerForRunTimeStats(void)
{
	RTOS_Timer_offset = tic();
}

uint32_t vHookGetRunTimerCounterValue(void)
{
	return tic() - RTOS_Timer_offset;
}

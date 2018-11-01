#include "pwm_aux_out.h"

static float 	_impulse_fac[2];
static uint8_t  _module_ready = DISABLE;
static uint16_t _impulse_values[6];

TIM_HandleTypeDef TimHandler1, TimHandler4;

void px4_pwm_aux_out_init(uint16_t * pwm_freqs)
{
	memset(_impulse_values, 0, sizeof(_impulse_values));

	uint32_t prescalers[2];
	uint32_t periods[2];

	// limit pwm rates
	for (int i = 0; i < 2; i++)
	{
		pwm_freqs[i] = (pwm_freqs[i] > PWM_AUX_MAX_RATE) ? PWM_AUX_MAX_RATE : pwm_freqs[i];
		pwm_freqs[i] = (pwm_freqs[i] < PWM_AUX_MIN_RATE) ? PWM_AUX_MIN_RATE : pwm_freqs[i];

		// calc prescaler so that we can use as much range of period as possible
		// large period range means better pwm pulse accuracy
		prescalers[i] = (HAL_RCC_GetSysClockFreq() / (i + 1)) / ((uint32_t) pwm_freqs[i] * 65535);
		periods[i] 	  = (HAL_RCC_GetSysClockFreq() / (i + 1)) / ((uint32_t) pwm_freqs[i] * (prescalers[i] + 1));

		// calc factor for calculating register compare value from pulse
		_impulse_fac[i] = (float) (pwm_freqs[i] * periods[i]) / 1e6f;
	}

	__HAL_RCC_GPIOE_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();

	__HAL_RCC_TIM1_CLK_ENABLE();
	__HAL_RCC_TIM4_CLK_ENABLE();

	// init aux port pins 1-4
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin 		= GPIO_PIN_9 | GPIO_PIN_11 | GPIO_PIN_13 | GPIO_PIN_14;
	GPIO_InitStruct.Mode 		= GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull 		= GPIO_PULLDOWN;
	GPIO_InitStruct.Speed 		= GPIO_SPEED_HIGH;
	GPIO_InitStruct.Alternate 	= GPIO_AF1_TIM1;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9 | GPIO_PIN_11 | GPIO_PIN_13 | GPIO_PIN_14, GPIO_PIN_RESET);

	// init aux port pins 5-6
	GPIO_InitStruct.Pin 		= GPIO_PIN_13 | GPIO_PIN_14;
	GPIO_InitStruct.Alternate 	= GPIO_AF2_TIM4;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13 | GPIO_PIN_14, GPIO_PIN_RESET);

	TimHandler1.Instance 			= TIM1;
	TimHandler1.Init.CounterMode 	= TIM_COUNTERMODE_UP;
	TimHandler1.Init.ClockDivision 	= TIM_CLOCKDIVISION_DIV1;
	TimHandler1.Init.Prescaler 		= prescalers[0];
	TimHandler1.Init.Period 		= periods[0];

	TimHandler4.Instance 			= TIM4;
	TimHandler4.Init.CounterMode 	= TIM_COUNTERMODE_UP;
	TimHandler4.Init.ClockDivision 	= TIM_CLOCKDIVISION_DIV1;
	TimHandler4.Init.Prescaler 		= prescalers[1];
	TimHandler4.Init.Period 		= periods[1];

	// init clock output compare struct
	TIM_OC_InitTypeDef oc_config;
	oc_config.OCMode 		= TIM_OCMODE_PWM1;
	oc_config.OCPolarity 	= TIM_OCPOLARITY_HIGH;
	oc_config.OCFastMode 	= TIM_OCFAST_DISABLE;
	oc_config.OCNPolarity 	= TIM_OCNPOLARITY_HIGH;
	oc_config.OCNIdleState 	= TIM_OCNIDLESTATE_RESET;
	oc_config.OCIdleState 	= TIM_OCIDLESTATE_RESET;
	oc_config.Pulse 		= 0;

	HAL_TIM_PWM_Init(&TimHandler1);
	HAL_TIM_PWM_Init(&TimHandler4);

	HAL_TIM_PWM_ConfigChannel(&TimHandler1, &oc_config, TIM_CHANNEL_4);
	HAL_TIM_PWM_ConfigChannel(&TimHandler1, &oc_config, TIM_CHANNEL_3);
	HAL_TIM_PWM_ConfigChannel(&TimHandler1, &oc_config, TIM_CHANNEL_2);
	HAL_TIM_PWM_ConfigChannel(&TimHandler1, &oc_config, TIM_CHANNEL_1);
	HAL_TIM_PWM_ConfigChannel(&TimHandler4, &oc_config, TIM_CHANNEL_2);
	HAL_TIM_PWM_ConfigChannel(&TimHandler4, &oc_config, TIM_CHANNEL_3);

	HAL_TIM_PWM_Start(&TimHandler1, TIM_CHANNEL_4);	// aux1
	HAL_TIM_PWM_Start(&TimHandler1, TIM_CHANNEL_3);	// aux2
	HAL_TIM_PWM_Start(&TimHandler1, TIM_CHANNEL_2);	// aux3
	HAL_TIM_PWM_Start(&TimHandler1, TIM_CHANNEL_1);	// aux4

	HAL_TIM_PWM_Start(&TimHandler4, TIM_CHANNEL_2);	// aux5
	HAL_TIM_PWM_Start(&TimHandler4, TIM_CHANNEL_3);	// aux6

	_module_ready = ENABLE;
	px4debug(ePWM_AUX, "pwm_aux_out init ok\r\n");
}

void px4_pwm_aux_out_set(uint16_t * impulse_values)
{
	memcpy(&_impulse_values, impulse_values, sizeof(_impulse_values));
}

void px4_pwm_aux_out_update()
{
	if (_module_ready == DISABLE)
	{
		return;
	}

	for (int i = 0; i < MAX_AUX_OUT_SERVO_CNT; i++)
	{
		if (_impulse_values[i] > PWM_AUX_OUT_IMPULSE_MAX)
		{
			_impulse_values[i] = PWM_AUX_OUT_IMPULSE_MAX;
		}

		if (_impulse_values[i] < PWM_AUX_OUT_IMPULSE_MIN)
		{
			_impulse_values[i] = PWM_AUX_OUT_IMPULSE_MIN;
		}
	}

	// update period value
	TIM1->CCR4 = _impulse_values[0] * _impulse_fac[0];
	TIM1->CCR3 = _impulse_values[1] * _impulse_fac[0];
	TIM1->CCR2 = _impulse_values[2] * _impulse_fac[0];
	TIM1->CCR1 = _impulse_values[3] * _impulse_fac[0];
	TIM4->CCR2 = _impulse_values[4] * _impulse_fac[1];
	TIM4->CCR3 = _impulse_values[5] * _impulse_fac[1];
}

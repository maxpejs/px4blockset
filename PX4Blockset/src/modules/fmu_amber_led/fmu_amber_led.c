#include "fmu_amber_led.h"

void fmu_amber_led_init(void)
{
	__HAL_RCC_GPIOE_CLK_ENABLE();
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin 	= GPIO_PIN_12;
	GPIO_InitStruct.Mode 	= GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull 	= GPIO_PULLUP;
	GPIO_InitStruct.Speed 	= GPIO_SPEED_FAST;

	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

	fmu_amber_led_set(GPIO_PIN_RESET);
}

void fmu_amber_led_set(uint32_t val)
{
	/* inverted logic at fmu led amber */
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_12, val == 0 ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void fmu_amber_led_toggle()
{
	HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_12);
}

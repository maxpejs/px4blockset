#include "error_handler.h"

static uint32_t _module_state = DISABLE;

void error_handler_init()
{
	fmu_amber_led_init();
	_module_state = ENABLE;
}

void error_handler(uint32_t val)
{
	if(_module_state == DISABLE)
	{
		error_handler_init();
	}

	if (val == 0)
	{
		while (1)
		{
			fmu_amber_led_toggle();
			HAL_Delay(500);
		}
	}
	else
	{
		while (1)
		{
			fmu_amber_led_set(GPIO_PIN_RESET);
			for (uint32_t i = 0; i < (val * 2); i++)
			{
				fmu_amber_led_toggle();
				HAL_Delay(200);
			}
			HAL_Delay(3000);
		}
	}
}

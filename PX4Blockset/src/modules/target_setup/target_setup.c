#include "target_setup.h"
#include "stm32f4xx_hal.h"
#include "error_handler.h"
#include "comm_itf.h"
#include <fmu_amber_led.h>
#include <spi_drv.h>

static void _set_vector_table(uint32_t NVIC_VectTab, uint32_t Offset);
static void _set_clock(void);
static void _setup_gpio(GPIO_TypeDef * port, uint16_t pin, uint32_t mode, uint32_t pull, uint32_t speed, uint8_t alternate, uint8_t pin_state);

void HAL_MspInit(void);

#define NONE 	0

void px4_target_setup_init()
{
	// vector table offset caused by bootloader
	_set_vector_table(FLASH_BASE, 0x4000);

	// initialize STM HAL drivers
	HAL_Init();

	// set internal clock to 168MHz
	_set_clock();
}

static void _set_vector_table(uint32_t NVIC_VectTab, uint32_t Offset)
{ 
	/* Check the parameters */
    assert_param(IS_NVIC_VECTTAB(NVIC_VectTab));
    assert_param(IS_NVIC_OFFSET(Offset));  
    SCB->VTOR = NVIC_VectTab | (Offset & (uint32_t)0x1FFFFF80);
}

static void _set_clock()
{
	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	RCC_OscInitTypeDef RCC_OscInitStruct;

	/* Enable Power Control clock */
	__HAL_RCC_PWR_CLK_ENABLE();

	/* The voltage scaling allows optimizing the power consumption when the device is
	clocked below the maximum system frequency, to update the voltage scaling value
	regarding system frequency refer to product datasheet. */
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	/* Enable HSE Oscillator and activate PLL with HSE as source */
	RCC_OscInitStruct.OscillatorType 	= RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState 			= RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState 		= RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource 	= RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM 			= 24;
	RCC_OscInitStruct.PLL.PLLN 			= 336;	// 360 for 180MHz
	RCC_OscInitStruct.PLL.PLLP 			= RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ 			= 7;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) 
    {
		error_handler(0);
	}

	/* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 clocks dividers */
	RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |
		RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK) 
    {
		error_handler(0);
	}

    // HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000U);
}

void HAL_MspInit(void)
{
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();

	// activate power source for 5V peripherie
	_setup_gpio(GPIOA, GPIO_PIN_8, GPIO_MODE_OUTPUT_PP, GPIO_PULLDOWN, GPIO_SPEED_FAST, NONE, GPIO_PIN_RESET);

	// activate power source for 3.3V peripherie
	_setup_gpio(GPIOE, GPIO_PIN_3, GPIO_MODE_OUTPUT_PP, GPIO_PULLDOWN, GPIO_SPEED_FAST, NONE, GPIO_PIN_SET);

	// config pin VDD_BRICK_VALID
	_setup_gpio(GPIOB, GPIO_PIN_5, GPIO_MODE_INPUT, GPIO_PULLUP, GPIO_SPEED_FAST, NONE, NONE);

	// config pin VDD_SERVO_VALID
	_setup_gpio(GPIOB, GPIO_PIN_7, GPIO_MODE_INPUT, GPIO_PULLUP, GPIO_SPEED_FAST, NONE, NONE);

	// config pin VDD_5V_HIPOWER_OC
	_setup_gpio(GPIOE, GPIO_PIN_10, GPIO_MODE_INPUT, GPIO_PULLUP, GPIO_SPEED_FAST, NONE, NONE);

	// config pin VDD_5V_PERIPH_OC
	_setup_gpio(GPIOE, GPIO_PIN_15, GPIO_MODE_INPUT, GPIO_PULLUP, GPIO_SPEED_FAST, NONE, NONE);

	// cofig SPI1
	px4_spi_drv_init(PX4_SPI1);
	_setup_gpio(GPIOA, GPIO_PIN_5,  GPIO_MODE_AF_PP, GPIO_PULLUP, GPIO_SPEED_FAST, GPIO_AF5_SPI1, GPIO_PIN_RESET);	// SCK
	_setup_gpio(GPIOA, GPIO_PIN_6,  GPIO_MODE_AF_PP, GPIO_PULLUP, GPIO_SPEED_FAST, GPIO_AF5_SPI1, GPIO_PIN_RESET);	// MISO
	_setup_gpio(GPIOA, GPIO_PIN_7,  GPIO_MODE_AF_PP, GPIO_PULLUP, GPIO_SPEED_FAST, GPIO_AF5_SPI1, GPIO_PIN_RESET);	// MOSI
	_setup_gpio(GPIOD, GPIO_PIN_15, GPIO_MODE_AF_PP, GPIO_PULLUP, GPIO_SPEED_FAST, GPIO_AF15_EVENTOUT, GPIO_PIN_RESET);	// DRDY

	/*
	 * config CS-pins from sensors and deactivate all sensors, by seting CS-pins to HIGH
	 */

	// deactivate L3GD20/L3GD20H (gyro)
	_setup_gpio(GPIOC, GPIO_PIN_13, GPIO_MODE_OUTPUT_PP, GPIO_PULLUP, GPIO_SPEED_FAST, NONE, GPIO_PIN_SET);

	// deactivate LSM303D (Accel + Mag)
	_setup_gpio(GPIOC, GPIO_PIN_15, GPIO_MODE_OUTPUT_PP, GPIO_PULLUP, GPIO_SPEED_FAST, NONE, GPIO_PIN_SET);

	// deactivate MS5611-01BA (Baro)
	_setup_gpio(GPIOD, GPIO_PIN_7,  GPIO_MODE_OUTPUT_PP, GPIO_PULLUP, GPIO_SPEED_FAST, NONE, GPIO_PIN_SET);

	// deactivate Sensor MPU6000 (Accel + Gyro)
	_setup_gpio(GPIOC, GPIO_PIN_2,  GPIO_MODE_OUTPUT_PP, GPIO_PULLUP, GPIO_SPEED_FAST, NONE, GPIO_PIN_SET);

	// config I2C1
	// I2C SCL
	_setup_gpio(GPIOB, GPIO_PIN_8, GPIO_MODE_AF_OD, GPIO_PULLDOWN, GPIO_SPEED_FAST, GPIO_AF4_I2C1, GPIO_PIN_RESET);
	// I2C SDA
	_setup_gpio(GPIOB, GPIO_PIN_9, GPIO_MODE_AF_OD, GPIO_PULLDOWN, GPIO_SPEED_FAST, GPIO_AF4_I2C1, GPIO_PIN_RESET);
	
	// config I2C2
	// I2C SCL
	_setup_gpio(GPIOB, GPIO_PIN_10, GPIO_MODE_AF_OD, GPIO_PULLDOWN, GPIO_SPEED_FAST, GPIO_AF4_I2C2, GPIO_PIN_RESET);
	// I2C SDA
	_setup_gpio(GPIOB, GPIO_PIN_11, GPIO_MODE_AF_OD, GPIO_PULLDOWN, GPIO_SPEED_FAST, GPIO_AF4_I2C2, GPIO_PIN_RESET);

	// config uart pins ... 
	// comming soon

	fmu_amber_led_init();
}

static void _setup_gpio(GPIO_TypeDef * port, uint16_t pin, uint32_t mode, uint32_t pull, uint32_t speed, uint8_t alternate, uint8_t pin_state)
{
	GPIO_InitTypeDef GPIO_InitStruct =	{ pin, mode, pull, speed, alternate };
	HAL_GPIO_Init(port, &GPIO_InitStruct);
	if (mode != GPIO_MODE_INPUT)
	{
		HAL_GPIO_WritePin(port, pin, pin_state);
	}
}

void print_system_info()
{
	px4debug(eNONE, "******************************************\r\n");
	px4debug(eNONE, "*** System Info ***\r\n");
	px4debug(eNONE, "PX4-Blockset v. 1.2\r\n");
	px4debug(eNONE, "FreeRTOS     v. 9.0.0\r\n");
	px4debug(eNONE, "FatFs        v. R0.13c\r\n");
	px4debug(eNONE, "STM32F4_HAL  v. 1.19.0\r\n");
	px4debug(eNONE, "******************************************\r\n");
}

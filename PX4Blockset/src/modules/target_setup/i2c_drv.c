#include "spi_drv.h"

#define CNT_I2C_ITF		4

static I2C_HandleTypeDef I2C_Handles[CNT_I2C_ITF];

static uint32_t _init_flag[CNT_I2C_ITF] = { DISABLE };

uint8_t px4_i2c_drv_init(uint32_t i2c_id)
{
	if (i2c_id >= CNT_I2C_ITF)
	{
		return ERROR;
	}

	if (_init_flag[i2c_id] == ENABLE)
	{
		return SUCCESS; // current i2c device channel is already enabled
	}

	switch (i2c_id)
	{
	case PX4_I2C1:
		__HAL_RCC_I2C1_CLK_ENABLE();
		I2C_Handles[i2c_id].Instance = I2C1;
		break;

	case PX4_I2C2:
		__HAL_RCC_I2C2_CLK_ENABLE();
		I2C_Handles[i2c_id].Instance = I2C2;
		break;

	default:
		return ERROR;
	}

	I2C_Handles[i2c_id].Init.AddressingMode 	= I2C_ADDRESSINGMODE_7BIT;
	I2C_Handles[i2c_id].Init.OwnAddress1 		= 0x00;
	I2C_Handles[i2c_id].Init.OwnAddress2 		= 0x00;
	I2C_Handles[i2c_id].Init.DualAddressMode 	= I2C_DUALADDRESS_DISABLE;
	I2C_Handles[i2c_id].Init.GeneralCallMode 	= I2C_GENERALCALL_DISABLE;
	I2C_Handles[i2c_id].Init.DutyCycle 			= I2C_DUTYCYCLE_16_9;
	I2C_Handles[i2c_id].Init.ClockSpeed 		= 100000;	// init clock 100kHz
	I2C_Handles[i2c_id].Init.NoStretchMode 		= I2C_NOSTRETCH_DISABLE;

	if (HAL_I2C_Init(&I2C_Handles[i2c_id]) != HAL_OK)
	{
		debug_print_string("HAL_I2C_Init error! I2C: ");
		debug_print_int(i2c_id);
		debug_print_string("\r\n");
		error_handler(0);
	}

	_init_flag[i2c_id] = ENABLE;

	return SUCCESS;
}

uint8_t px4_i2c_drv_transmit(uint32_t i2c_id, uint16_t i2c_address, uint8_t * txBuff, uint8_t size)
{
	HAL_StatusTypeDef ret;
	uint8_t tries = 3;

	do
	{
		ret = HAL_I2C_Master_Transmit(&I2C_Handles[i2c_id], i2c_address, txBuff, size, 10);

		if (ret != HAL_OK)
		{
			if (HAL_I2C_GetError(&I2C_Handles[i2c_id]) != HAL_I2C_ERROR_AF)
			{
				debug_print_string("HAL_I2C_Master_Transmit err!\r\n");
				return ERROR;
			}
		}
	} while ((ret != HAL_OK) && tries-- > 0);

	return (ret == HAL_OK) ? SUCCESS : ERROR;
}

uint8_t px4_i2c_drv_receive(uint32_t i2c_id, uint16_t i2c_address, uint8_t * rxBuff, uint8_t size)
{
	HAL_StatusTypeDef ret = HAL_OK;
	uint8_t tries = 3;

	do
	{
		ret = HAL_I2C_Master_Receive(&I2C_Handles[i2c_id], i2c_address, rxBuff, size, 10);

		if (ret != HAL_OK)
		{

			if (HAL_I2C_GetError(&I2C_Handles[i2c_id]) != HAL_I2C_ERROR_AF)
			{
				debug_print_string("HAL_I2C_Master_Receive err!\r\n");
				return ERROR;
			}
		}
	} while ((ret != HAL_OK) && tries-- > 0);

	return (ret == HAL_OK) ? SUCCESS : ERROR;
}

void px4_i2c_drv_set_clock_speed(uint32_t i2c_id, uint32_t clock_speed)
{
	if (i2c_id > 3)
	{
		return;
	}

	I2C_Handles[i2c_id].Init.ClockSpeed = clock_speed;

	if (HAL_I2C_Init(&I2C_Handles[i2c_id]) != HAL_OK)
	{
		debug_print_string("HAL_I2C_Init error! I2C: ");
		debug_print_int(i2c_id);
		debug_print_string("\r\n");
		error_handler(0);
	}
}

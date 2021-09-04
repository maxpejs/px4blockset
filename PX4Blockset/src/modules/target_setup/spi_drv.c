#include "spi_drv.h"

#define CNT_SPI_ITF		6

static SPI_HandleTypeDef SPI_Handles[CNT_SPI_ITF];

static uint32_t _init_flag[CNT_SPI_ITF] =
{ DISABLE, DISABLE, DISABLE, DISABLE, DISABLE, DISABLE };

uint8_t px4_spi_drv_init(uint32_t spi_id)
{
	if (spi_id >= CNT_SPI_ITF)
	{
		return ERROR;
	}

	if (_init_flag[spi_id] == ENABLE)
	{
		return SUCCESS; // spi already enabled
	}

	SPI_HandleTypeDef * SpiHandle = &SPI_Handles[spi_id];

	switch (spi_id)
	{
	case PX4_SPI1:
		__HAL_RCC_SPI1_CLK_ENABLE();
		SpiHandle->Instance = SPI1;
		break;

	default:
		return ERROR;
	}

	SpiHandle->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
	SpiHandle->Init.Direction = SPI_DIRECTION_2LINES;
	SpiHandle->Init.CLKPhase = SPI_PHASE_2EDGE;
	SpiHandle->Init.CLKPolarity = SPI_POLARITY_HIGH;
	SpiHandle->Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	SpiHandle->Init.CRCPolynomial = 7;
	SpiHandle->Init.DataSize = SPI_DATASIZE_8BIT;
	SpiHandle->Init.FirstBit = SPI_FIRSTBIT_MSB;
	SpiHandle->Init.NSS = SPI_NSS_SOFT;
	SpiHandle->Init.TIMode = SPI_TIMODE_DISABLE;
	SpiHandle->Init.Mode = SPI_MODE_MASTER;

	if (HAL_SPI_Init(SpiHandle) != HAL_OK)
	{
		px4debug("HAL_SPI_Init error!\r\n");
		error_handler(0);
	}

	_init_flag[spi_id] = ENABLE;

	return SUCCESS;
}

uint8_t px4_spi_drv_transmit(uint32_t spi_id, uint8_t * txBuff, uint8_t * rxBuff, uint8_t size)
{
	uint8_t ret = HAL_ERROR;

	ret = HAL_SPI_TransmitReceive(&SPI_Handles[spi_id], txBuff, rxBuff, size, 10);

	if (ret == HAL_OK)
	{
		return SUCCESS;
	}
	else
	{
		px4debug("spi transmit error!\r\n");
		return ERROR;
	}
}

void px4_spi_drv_set_clock_speed(uint32_t spi_id, uint32_t prescaler)
{
	if (spi_id >= CNT_SPI_ITF)
	{
		return;
	}
	SPI_Handles[spi_id].Init.BaudRatePrescaler = prescaler;

	if (HAL_SPI_Init(&SPI_Handles[spi_id]) != HAL_OK)
	{
		px4debug("HAL_SPI_Init error!\r\n");
		error_handler(0);
	}
}

#include "pxio_driver.h"

static struct IOPacket 		dmaTxBuffer;
static struct IOPacket 		dmaRxBuffer;
static uint8_t				module_state = DISABLE;

UART_HandleTypeDef 			PXIO_UART;
static DMA_HandleTypeDef 	hdma_rx;
static DMA_HandleTypeDef 	hdma_tx;

static int32_t read(uint8_t page, uint8_t offset, uint16_t * data, uint8_t count);
static int32_t write(uint8_t page, uint8_t offset, uint16_t * values, uint8_t count);
static int32_t com_complete();

void pxio_driver_init(void)
{
	if(module_state == DISABLE)
	{
		memset(&dmaTxBuffer, 0, sizeof(dmaTxBuffer));
		memset(&dmaRxBuffer, 0, sizeof(dmaRxBuffer));

		__HAL_RCC_GPIOC_CLK_ENABLE();
		__HAL_RCC_USART6_CLK_ENABLE();
		__HAL_RCC_DMA2_CLK_ENABLE();

		PXIO_UART.Instance 			= USART6;
		PXIO_UART.Init.BaudRate 	= FMU_IO_BAUDRATE;
		PXIO_UART.Init.WordLength 	= UART_WORDLENGTH_8B;
		PXIO_UART.Init.StopBits 	= UART_STOPBITS_1;
		PXIO_UART.Init.Parity 		= UART_PARITY_NONE;
		PXIO_UART.Init.HwFlowCtl 	= UART_HWCONTROL_NONE;
		PXIO_UART.Init.Mode 		= UART_MODE_TX_RX;
		PXIO_UART.Init.OverSampling = UART_OVERSAMPLING_16;

		if (HAL_UART_Init(&PXIO_UART) != HAL_OK)
		{
			px4debug(eDRV, "pxio drv HAL_UART_Init error! \r\n");
			error_handler(0);
		}

		// UART TX and RX GPIO pin configuration
		GPIO_InitTypeDef GPIO_InitStruct;
		GPIO_InitStruct.Pin 	  = GPIO_PIN_6|GPIO_PIN_7;
		GPIO_InitStruct.Mode 	  = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull 	  = GPIO_NOPULL;
		GPIO_InitStruct.Speed 	  = GPIO_SPEED_FAST;
		GPIO_InitStruct.Alternate = GPIO_AF8_USART6;
		HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

		hdma_tx.Instance 					= DMA2_Stream7;
		hdma_tx.Init.Channel 				= DMA_CHANNEL_5;
		hdma_tx.Init.Direction 				= DMA_MEMORY_TO_PERIPH;
		hdma_tx.Init.PeriphInc 				= DMA_PINC_DISABLE;
		hdma_tx.Init.MemInc 				= DMA_MINC_ENABLE;
		hdma_tx.Init.PeriphDataAlignment 	= DMA_PDATAALIGN_BYTE;
		hdma_tx.Init.MemDataAlignment 		= DMA_MDATAALIGN_BYTE;
		hdma_tx.Init.Mode 					= DMA_NORMAL;
		hdma_tx.Init.Priority 				= DMA_PRIORITY_LOW;
		hdma_tx.Init.FIFOMode 				= DMA_FIFOMODE_DISABLE;
		hdma_tx.Init.FIFOThreshold 			= DMA_FIFO_THRESHOLD_FULL;
		hdma_tx.Init.MemBurst 				= DMA_MBURST_INC4;
		hdma_tx.Init.PeriphBurst 			= DMA_PBURST_INC4;
		if (HAL_DMA_Init(&hdma_tx) != HAL_OK)
		{
			px4debug(eDRV, "pxio drv HAL_DMA_Init tx error! \r\n");
			error_handler(0);
		}

		__HAL_LINKDMA(&PXIO_UART, hdmatx, hdma_tx);

		hdma_rx.Instance 					= DMA2_Stream2;
		hdma_rx.Init.Channel 				= DMA_CHANNEL_5;
		hdma_rx.Init.Direction 				= DMA_PERIPH_TO_MEMORY;
		hdma_rx.Init.PeriphInc 				= DMA_PINC_DISABLE;
		hdma_rx.Init.MemInc 				= DMA_MINC_ENABLE;
		hdma_rx.Init.PeriphDataAlignment 	= DMA_PDATAALIGN_BYTE;
		hdma_rx.Init.MemDataAlignment 		= DMA_MDATAALIGN_BYTE;
		hdma_rx.Init.Mode 					= DMA_NORMAL;
		hdma_rx.Init.Priority 				= DMA_PRIORITY_HIGH;
		hdma_rx.Init.FIFOMode 				= DMA_FIFOMODE_DISABLE;
		hdma_rx.Init.FIFOThreshold 			= DMA_FIFO_THRESHOLD_FULL;
		hdma_rx.Init.MemBurst 				= DMA_MBURST_INC4;
		hdma_rx.Init.PeriphBurst 			= DMA_PBURST_INC4;

		if (HAL_DMA_Init(&hdma_rx) != HAL_OK)
		{
			px4debug(eDRV, "pxio drv HAL_DMA_Init rx error! \r\n");
			error_handler(0);
		}
		__HAL_LINKDMA(&PXIO_UART, hdmarx, hdma_rx);

		/*## Configure interrupts for DMA #########################################*/
		/* NVIC configuration for DMA transfer complete interrupt (USARTx_TX) */
		HAL_NVIC_SetPriority(DMA2_Stream7_IRQn, PXIO_ITF_DMA_IRQ_PRIO, PXIO_ITF_DMA_TX_IRQ_SUB_PRIO);
		HAL_NVIC_EnableIRQ(DMA2_Stream7_IRQn);

		/* NVIC configuration for DMA transfer complete interrupt (USARTx_RX) */
		HAL_NVIC_SetPriority(DMA2_Stream2_IRQn, PXIO_ITF_DMA_IRQ_PRIO, PXIO_ITF_DMA_RX_IRQ_SUB_PRIO);
		HAL_NVIC_EnableIRQ(DMA2_Stream2_IRQn);

		/* NVIC configuration for USART TC interrupt */
		HAL_NVIC_SetPriority(USART6_IRQn, PXIO_ITF_USART_IRQ_PRIO, 0);
		HAL_NVIC_EnableIRQ(USART6_IRQn);
		
		module_state = ENABLE;
		px4debug(eDRV, "pxio_driver init ok\r\n");

		uint16_t reg_val = 0;
		int16_t ret, tries = 5;

		// test if pxio communication succeed
		do
		{
			// try to get some data from PXIO
			ret = pxio_driver_reg_get(PX4IO_PAGE_STATUS, PX4IO_P_STATUS_FLAGS, &reg_val, 1);

			// if register read failed, wait for a while, pxio needs time to start up
			if (ret != SUCCESS)
			{
				HAL_Delay(500);
			}

		} while ((ret != SUCCESS) && (--tries > 0));

		if (tries == 0)
		{
			px4debug(eDRV, "no communication to pxio\r\n");
		}
		else
		{
			px4debug(eDRV, "communication with pxio is established\r\n");
		}
	}
}

int32_t pxio_driver_reg_mod(uint8_t page, uint8_t offset, uint16_t clearbits, uint16_t setbits)
{
	int ret;
	uint16_t value;
	
	ret = pxio_driver_reg_get(page, offset, &value, 1);

	if (ret != SUCCESS)
	{
		px4debug(eDRV, "reg mod get err\r\n");
		return ret;
	}
	
	value &= ~clearbits;
	value |= setbits;
	
	ret = pxio_driver_reg_set(page, offset, &value, 1);

	if (ret != SUCCESS)
	{
		px4debug(eDRV, "reg mod set err\r\n");
		return ret;
	}
	return ret; 
}

int32_t pxio_driver_reg_get(uint8_t page, uint8_t offset, uint16_t *values, unsigned num_values)
{
	if (num_values > ((MAX_TRANSFER) / sizeof(*values)))
	{
		return ERROR;
	}

	int ret = read(page, offset, values, num_values);

	if (ret != (int) num_values)
	{
		px4debug(eDRV, "reg get err\r\n");
		return ERROR;
	}
	return SUCCESS;
}

int32_t pxio_driver_reg_set(uint8_t page, uint8_t offset, uint16_t * values, unsigned num_values)
{
	if (num_values > ((MAX_TRANSFER) / sizeof(*values)))
	{
		return ERROR;
	}
	
	int ret = write(page, offset, values, num_values);

	if (ret != (int)num_values) 
	{
		px4debug(eDRV, "reg set err\r\n");
		return ERROR;
	}
	return SUCCESS;
}

int32_t write(uint8_t page, uint8_t offset, uint16_t * values, uint8_t count)
{
	if (count > PKT_MAX_REGS)
		return ERROR;

	int result;
	int tries = TRANSMIT_TRIES_CNT;
	do
	{
		memset(&dmaTxBuffer, 0, sizeof(dmaTxBuffer));
		dmaTxBuffer.count_code = count | PKT_CODE_WRITE;
		dmaTxBuffer.page = page;
		dmaTxBuffer.offset = offset;
		memcpy((void*) &dmaTxBuffer.regs[0], (void*) values, count*2);

		for (unsigned i = count; i < PKT_MAX_REGS; i++)
		{
			dmaTxBuffer.regs[i] = 0x55aa;
		}

		result = com_complete();

	} while (result != SUCCESS && tries-- > 0);

	if (result == SUCCESS)
	{
		if (PKT_CODE(dmaRxBuffer) != PKT_CODE_ERROR)
		{
			result = count;
		}
		else
		{
			px4debug(eDRV, "wrt pck err\r\n");
			result = ERROR;
		}
	}
	
	return result;
}

int32_t read(uint8_t page, uint8_t offset, uint16_t * values, uint8_t count)
{
	if (count > PKT_MAX_REGS)
	{
		return ERROR;
	}

	int result;
	int tries = TRANSMIT_TRIES_CNT;
	do
	{
		memset(&dmaTxBuffer, 0, sizeof(dmaTxBuffer));
		dmaTxBuffer.count_code = count | PKT_CODE_READ;
		dmaTxBuffer.page = page;
		dmaTxBuffer.offset = offset;

		result = com_complete();
		
	} while (result != SUCCESS && tries-- > 0);

	if (result == SUCCESS)
	{
		if (PKT_CODE(dmaRxBuffer) == PKT_CODE_ERROR)
		{
			px4debug(eDRV, "pck code err\r\n");
			result = ERROR;
		}
		else if (PKT_COUNT(dmaRxBuffer) != count) /* compare register counts */
		{
			px4debug(eDRV, "recv cnt err\r\n");
			result = ERROR;
		}
		else // read was successful
		{
			// no errors detected, copy the result
			memcpy((void *) values, (void *) (&dmaRxBuffer.regs[0]), (2 * count));
			result = count;
		}
	}

	return result;
}

int32_t com_complete()
{
	int result = SUCCESS;

	dmaTxBuffer.crc = 0;
	dmaTxBuffer.crc = crc_packet(&dmaTxBuffer);

	// init Rx buffer with sample 0xFF
	memset(&dmaRxBuffer, 0xFF, sizeof(dmaRxBuffer));

	// load rx dma interrupt
	if (HAL_UART_Receive_DMA(&PXIO_UART, (uint8_t*) &dmaRxBuffer, sizeof(dmaRxBuffer)) != HAL_OK)
	{
		px4debug(eDRV, "setup rx dma err\r\n");
	}

	// load tx dma and fires
	if (HAL_UART_Transmit_DMA(&PXIO_UART, (uint8_t*) &dmaTxBuffer, PKT_SIZE(dmaTxBuffer)) != HAL_OK)
	{
		px4debug(eDRV, "setup tx dma err\r\n");
	}

	//	// wait until rx transaction starts
	//	while (__HAL_UART_GET_FLAG(&PXIO_UART, UART_FLAG_IDLE) && (timer < DMA_TXRX_TIMEOUT))
	//		timer = toc(start);
	//
	//	if (timer >= DMA_TXRX_TIMEOUT)
	//	{
	//		px4debug(eDRV, "dma timeout on start \r\n");
	//	}
	//
	//	// wait until rx transaction is over
	//	while (!__HAL_UART_GET_FLAG(&PXIO_UART, UART_FLAG_IDLE) && (timer < DMA_TXRX_TIMEOUT))
	//		timer = toc(start);


	// packet code is one of the first received bytes
	// wait until packet code incomes
	
	uint64_t start = tic();
	uint32_t timer = 0;
	
	uint8_t packet_code;
	
	// packed code is first byte we get received, so check cyclic the init values in packet code were overwritten
	do
	{
		packet_code = PKT_CODE(dmaRxBuffer);
		timer = toc(start);
	} while ((packet_code == (PKT_CODE_MASK & 0xFF)) && (timer < DMA_TXRX_TIMEOUT));

	// we received the packed code, so rx transaction already runs
	// wait until rx transaction is over by checking IDLE flag
	while (!__HAL_UART_GET_FLAG(&PXIO_UART, UART_FLAG_IDLE) && (timer < DMA_TXRX_TIMEOUT))
	{
		timer = toc(start);
	}

	// rx transaction is over, check that we received package correct, by checking crc
	uint8_t crc = dmaRxBuffer.crc;
	dmaRxBuffer.crc = 0;
	if ((crc != crc_packet(&dmaRxBuffer)) | (PKT_CODE(dmaRxBuffer) == PKT_CODE_CORRUPT))
	{
		px4debug(eDRV, "dma crc err\r\n");
		result = ERROR;
	}

	HAL_UART_DMAStop(&PXIO_UART);

	return result;
}

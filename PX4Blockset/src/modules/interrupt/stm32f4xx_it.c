/**
  ******************************************************************************
  * @file    stm32f4xx_it.c 
  * @author  MCD Application Team
  * @version V1.2.3
  * @date    29-January-2016
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2016 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_it.h"

/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/


extern UART_HandleTypeDef GpsUart;
extern UART_HandleTypeDef CommUart;
extern UART_HandleTypeDef PXIO_UART;

extern void error_handler(uint32_t);
extern void comm_itf_rx_complete_event(void);
extern void px4_gps_rx_complete_event(void);

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
	 while (1)
	  {
		  error_handler(0);
	  }
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
	debug_print_string("Hard Fault occured! \n");
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
	  error_handler(0);
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
	 while (1)
	  {
		  error_handler(0);
	  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
	 while (1)
	  {
		  error_handler(0);
	  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
	 while (1)
	  {
		  error_handler(0);
	  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
	 while (1)
	  {
		  error_handler(0);
	  }
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
	 while (1)
	  {
		  error_handler(0);
	  }
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
	 while (1)
	  {
		  error_handler(0);
	  }
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
  HAL_IncTick();
}

/******************************************************************************/
/*                 STM32F4xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f4xx.s).                                               */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/


/*********************************************************/
/* callback functions */
/*********************************************************/

// common callback function for all uart receive complete callbacks
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle)
{
	if (UartHandle->Instance == UART8)	// SERIAL4
	{
		comm_itf_rx_complete_event();
		return;
	}
	
	if (UartHandle->Instance == USART2)	// TELEM1
	{
		comm_itf_rx_complete_event();
		return;
	}

	if (UartHandle->Instance == UART4)	// GPS
	{
		px4_gps_rx_complete_event();
		return;
	}
}

/*********************************************************/
/* interrupt functions */
/*********************************************************/

// interrupt at USART6 (PXFMU <-> PXIO)
void USART6_IRQHandler(void)
{
	HAL_UART_IRQHandler(&PXIO_UART);
}

// PXIO rx dma event
void DMA2_Stream2_IRQHandler(void)
{
	HAL_DMA_IRQHandler(PXIO_UART.hdmarx);
}

// PXIO tx dma event
void DMA2_Stream7_IRQHandler(void)
{
	HAL_DMA_IRQHandler(PXIO_UART.hdmatx);
}

// interrupt at UART4 (GPS)
void UART4_IRQHandler(void)
{
	HAL_UART_IRQHandler(&GpsUart);
}

// interrupt at UART8 (SERIAL4)
void UART8_IRQHandler(void)
{
	HAL_UART_IRQHandler(&CommUart);
}

// interrupt at USART2 (TELEM1)
void USART2_IRQHandler(void)
{
	HAL_UART_IRQHandler(&CommUart);
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

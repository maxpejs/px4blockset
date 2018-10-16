#include "gps.h"

typedef struct
{
	uint8_t  buff[GPS_RING_BUFF_SIZE];
	uint32_t read;
	uint32_t write;
}ring_buffer;

static uint8_t  _parser_buff[GPS_SENTENCE_BUFF_SIZE];
static uint8_t  _parser_buffer_last[GPS_SENTENCE_BUFF_SIZE];
static uint8_t  _crc_idx 	 = 0;
static uint8_t  _recv_crc 	 = 0;
static uint32_t _time_rx_last_rmc_msg = 0;
static uint8_t  _module_state 	 	  = DISABLE;

static gps_rmc_packet_st _rmc_storage;
static ring_buffer 	  _rb;

UART_HandleTypeDef GpsUart;

static uint32_t _runtimes_arr[4]; // 0 - rmc string parsing time, 1 - update time, 2 - crc runtime, 3 - rx interrupt runtime

// internal functions
static 			uint8_t 	calculate_nmea_crc(uint8_t * buff);
static inline 	uint8_t 	ring_buffer_pop(ring_buffer * b);
static inline 	uint32_t 	ring_buffer_empty(ring_buffer * b);
static 			uint32_t 	ring_buffer_free_space(ring_buffer * b);
static 			void 		state_machine_process_new_byte(uint8_t rxChar);
static 			void 		gps_setup_uart(uint32_t baud);


/******* FOR TESTS *****************/
uint64_t it_runtime_total = 0;
uint32_t it_runtime_cnt = 0, it_runtime_cnt_last = 0;

typedef enum
{
	SYNC_SOS,		// wait for $ - start of sequence
	SYNC_HEADER,	// check what for sentence we are receiving
	GET_DATA,		// get NMEA data string
	CHK_SUM			// get checksum
} state;

static state 	_act_state = SYNC_SOS;
static uint8_t 	_char_idx  = 0;


void px4_gps_update(void)
{
	if (_module_state == DISABLE)
	{
		return;
	}

	uint8_t cnt = 0;
	uint64_t start = tic();

	// parse just some chars during single call
	while (!ring_buffer_empty(&_rb) && (cnt++ < 20))
	{
		state_machine_process_new_byte(ring_buffer_pop(&_rb));
	}

	// first check if there are new data from gps sensor
	if ((HAL_GetTick() - _time_rx_last_rmc_msg) > RECEIVE_TIMEOUT)
	{
		_rmc_storage.Valid = 0;
	}

	_runtimes_arr[1] = (uint32_t)toc(start);
}

void px4_gps_init(uint32_t baud)
{
	_crc_idx = 0;
	memset(&_rmc_storage, 0, sizeof(gps_rmc_packet_st));
	memset(&_rb, 0, sizeof(ring_buffer));
	memset(_parser_buff, 0, sizeof(_parser_buff));
	memset(_runtimes_arr, 0, sizeof(_runtimes_arr));

	gps_setup_uart(baud);

	// TODO setup gps chip using ublox protocol, only RMC sentense schould be sent
	_module_state = ENABLE;
	debug_print_string("gps init ok \r\n");
}

static void gps_setup_uart(uint32_t baud)
{
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_UART4_CLK_ENABLE();

	GpsUart.Instance 			= UART4;
	GpsUart.Init.BaudRate 		= baud;
	GpsUart.Init.WordLength 	= UART_WORDLENGTH_8B;
	GpsUart.Init.StopBits 		= UART_STOPBITS_1;
	GpsUart.Init.Parity 		= UART_PARITY_NONE;
	GpsUart.Init.HwFlowCtl 		= UART_HWCONTROL_NONE;
	GpsUart.Init.Mode 			= UART_MODE_TX_RX;
	GpsUart.Init.OverSampling 	= UART_OVERSAMPLING_16;
	if (HAL_UART_Init(&GpsUart) != HAL_OK)
	{
		debug_print_string("gps HAL_UART_Init err!\r\n");
		error_handler(0);
	}

	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin 		= GPIO_PIN_0 | GPIO_PIN_1;
	GPIO_InitStruct.Mode 		= GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull 		= GPIO_NOPULL;
	GPIO_InitStruct.Speed 		= GPIO_SPEED_FAST;
	GPIO_InitStruct.Alternate 	= GPIO_AF8_UART4;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	HAL_NVIC_SetPriority(UART4_IRQn, GPS_UART_IRQ_PRIO, 0);
	HAL_NVIC_EnableIRQ(UART4_IRQn);

	// start receiving
	if (HAL_UART_Receive_IT(&GpsUart, &(_rb.buff[_rb.write]), GPS_RX_PACKAGE_LENGTH)!= HAL_OK)
	{
		debug_print_string("gps HAL_UART_Receive_IT err!\r\n");
		error_handler(0);
	}

	// TODO set Delay to 10ms
	HAL_Delay(100);
}

static void state_machine_process_new_byte(uint8_t rxChar)
{
	switch (_act_state)
	{
	case SYNC_SOS:
		if (rxChar == '$')
		{
			// new sentence starts
			_char_idx = 0;
			_parser_buff[_char_idx++] = rxChar;
			_act_state = SYNC_HEADER;
		}
		break;

	case SYNC_HEADER:
		_parser_buff[_char_idx++] = rxChar;

		if (rxChar == ',')
		{
			// check if we are reading GPRMC - Sentence
			if (strncmp((const char*) _parser_buff, RMC_SENTENCE_HEADER, strlen(RMC_SENTENCE_HEADER)) == 0)
				_act_state = GET_DATA;
			else
				_act_state = SYNC_SOS;
		}
		break;

	case GET_DATA:
		_parser_buff[_char_idx++] = rxChar;

		// check if received end of nmea string
		if (rxChar == '*')
		{
			_crc_idx = 0;
			_act_state = CHK_SUM;
		}
		break;

	case CHK_SUM:
		_parser_buff[_char_idx++] = rxChar;
		_parser_buff[_char_idx] = 0; // terminate string
		
		if (_crc_idx == 0)
		{
			// high nibble
			_recv_crc = ascii_2_nibble(rxChar);
		}
		else if (_crc_idx == 1)
		{
			// low nibble
			_recv_crc = (_recv_crc << 4) | ascii_2_nibble(rxChar);

			// compare checksums, check if we received the sentence correctly
			if (_recv_crc == calculate_nmea_crc(_parser_buff))
			{
				// update receive time stamp
				_time_rx_last_rmc_msg = HAL_GetTick();
				uint64_t start = tic();
				parse_nmea_rmc_sentence(_parser_buff, &_rmc_storage);
				_runtimes_arr[0] = (uint32_t)toc(start);

				// TODO: use _char_idx for copy length
				memcpy(_parser_buffer_last, _parser_buff, GPS_SENTENCE_BUFF_SIZE);

				// update interrupt runtime
				// average interrupt runtime * interrupt call counter since last received rmc string
				_runtimes_arr[3] = ((uint32_t) it_runtime_total / it_runtime_cnt) * (it_runtime_cnt - it_runtime_cnt_last);
				it_runtime_cnt_last = it_runtime_cnt;
				/*******************************************************/
			}else
			{
				debug_print_string("rmc crc err \r\n");
			}

			_act_state = SYNC_SOS;
		}
		else
		{
			_act_state = SYNC_SOS;
		}
		_crc_idx++;
		break;
	default:
		break;
	}

	// avoid buffer overflow
	if (_char_idx >= GPS_SENTENCE_BUFF_SIZE)
	{
		_act_state = SYNC_SOS;
		return;
	}
}



void px4_gps_get(gps_rmc_packet_st * pData)
{
	memcpy(pData, &_rmc_storage, sizeof(gps_rmc_packet_st));
}

void px4_gps_get_raw(uint8_t * buff)
{
	memcpy(buff, _parser_buffer_last, GPS_SENTENCE_BUFF_SIZE);
}

void px4_gps_rx_complete_event()
{
	uint64_t start = tic();

	// debug_print_string("gps rx evt \r\n");
	// if there are enought free space in the ring buffer
	// move to next position, otherwise overwrite last position.
	// this is more secure, in worst case one sentence can get lost
	if (ring_buffer_free_space(&_rb) >= GPS_RX_PACKAGE_LENGTH)
	{
		_rb.write = (_rb.write + GPS_RX_PACKAGE_LENGTH) % GPS_RING_BUFF_SIZE;
	}
	else
	{
		debug_print_string("gps. not enought free space in ring buffer! \r\n");
	}

	// start receiving next part
	HAL_UART_Receive_IT(&GpsUart, &(_rb.buff[_rb.write]), GPS_RX_PACKAGE_LENGTH);

	it_runtime_total += toc(start);
	it_runtime_cnt++;
}

static uint8_t calculate_nmea_crc(uint8_t * buff)
{
	uint64_t start = tic();

	uint8_t idx = 1; // skip first char: '$'
	uint8_t ret = 0;

	do
	{
		if (buff[idx] != '*')
		{
			ret ^= buff[idx++];
		}
		else
		{
			break;
		}
	} while (idx < GPS_SENTENCE_BUFF_SIZE);

	_runtimes_arr[2] = (uint32_t) toc(start);

	return ret;
}


static inline uint8_t ring_buffer_pop(ring_buffer * b)
{
	uint8_t ret = b->buff[b->read++];
	b->read = b->read % GPS_RING_BUFF_SIZE;
	return ret;
}

static inline uint32_t ring_buffer_empty(ring_buffer * b)
{
	return b->read == b->write;
}

// TODO see implementation ring_buffer_free_space in sd card
static uint32_t ring_buffer_free_space(ring_buffer * b)
{
	if (b->read > b->write)
	{
		return b->read - b->write;
	}
	else
	{
		return (sizeof(b->buff)- b->write) + b->read - 1;
	}
}

uint32_t * px4_gps_getruntimes(void)
{
	return _runtimes_arr;
}

#include "gps.h"

typedef struct
{
	uint8_t buff[GPS_RING_BUFF_SIZE];
	uint32_t read;
	uint32_t write;
} ring_buffer;

static uint8_t _parser_buff[2][GPS_SENTENCE_BUFF_SIZE];
static uint8_t _rmc_raw_string[GPS_SENTENCE_BUFF_SIZE];

static gps_rmc_packet_st _rmc_storage[2];

static uint8_t _crc_idx = 0;
static uint8_t _recv_crc = 0;
static uint32_t _time_rx_last_rmc_msg = 0;
static uint8_t _module_state = DISABLE;

static ring_buffer _rb;

static uint32_t _storage_idx = 0;

UART_HandleTypeDef GpsUart;

// internal functions
static uint8_t calculate_nmea_crc(uint8_t * buff);
static inline uint8_t ring_buffer_pop(ring_buffer * b);
static inline uint32_t ring_buffer_empty(ring_buffer * b);
static uint32_t ring_buffer_free_space(ring_buffer * b);
static void state_machine_process_new_byte(uint8_t rxChar);
static void gps_setup_uart(uint32_t baud);

typedef enum
{
	SYNC_SOS,		// wait for $ - start of sequence
	SYNC_HEADER,	// check what for sentence we are receiving
	GET_DATA,		// get NMEA data string
	CHK_SUM			// get checksum
} state;

static state _act_state = SYNC_SOS;
static uint8_t _char_idx = 0;

void px4_gps_init(uint32_t baud)
{
	_crc_idx = 0;
	memset(&_rmc_storage, 0, sizeof(_rmc_storage));
	memset(&_rb, 0, sizeof(_rb));
	memset(_parser_buff, 0, sizeof(_parser_buff));
	memset(_rmc_raw_string, 0, sizeof(_rmc_raw_string));

	gps_setup_uart(baud);

	// TODO setup gps chip using ublox protocol, only RMC sentense should be sent
	_module_state = ENABLE;
	px4debug(eCOMMITF, "gps init ok \r\n");
}

static void gps_setup_uart(uint32_t baud)
{
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_UART4_CLK_ENABLE();

	GpsUart.Instance = UART4;
	GpsUart.Init.BaudRate = baud;
	GpsUart.Init.WordLength = UART_WORDLENGTH_8B;
	GpsUart.Init.StopBits = UART_STOPBITS_1;
	GpsUart.Init.Parity = UART_PARITY_NONE;
	GpsUart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	GpsUart.Init.Mode = UART_MODE_TX_RX;
	GpsUart.Init.OverSampling = UART_OVERSAMPLING_16;

	if (HAL_UART_Init(&GpsUart) != HAL_OK)
	{
		px4debug(eCOMMITF, "gps HAL_UART_Init err!\r\n");
		error_handler(0);
	}

	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
	GPIO_InitStruct.Alternate = GPIO_AF8_UART4;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	HAL_NVIC_SetPriority(UART4_IRQn, GPS_UART_IRQ_PRIO, 0);
	HAL_NVIC_EnableIRQ(UART4_IRQn);

	// start receiving
	if (HAL_UART_Receive_IT(&GpsUart, &(_rb.buff[_rb.write]), GPS_RX_PACKAGE_LENGTH) != HAL_OK)
	{
		px4debug(eCOMMITF, "gps HAL_UART_Receive_IT err!\r\n");
		error_handler(0);
	}
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
			_parser_buff[_storage_idx][_char_idx++] = rxChar;
			_act_state = SYNC_HEADER;
		}
		break;

	case SYNC_HEADER:
		_parser_buff[_storage_idx][_char_idx++] = rxChar;

		if (rxChar == ',')
		{
			// check if we are reading GPRMC - Sentence
			if (strncmp((const char*) _parser_buff[_storage_idx], RMC_HEADER, strlen(RMC_HEADER)) == 0)
				_act_state = GET_DATA;
			else
				_act_state = SYNC_SOS;
		}
		break;

	case GET_DATA:
		_parser_buff[_storage_idx][_char_idx++] = rxChar;

		// check if received end of nmea string
		if (rxChar == '*')
		{
			_crc_idx = 0;
			_act_state = CHK_SUM;
		}
		break;

	case CHK_SUM:
		_parser_buff[_storage_idx][_char_idx++] = rxChar;
		_parser_buff[_storage_idx][_char_idx] = 0; // terminate string

		if (_crc_idx == 0)
		{
			// high nibble
			_recv_crc = ascii_2_nibble(rxChar);
		}
		else if (_crc_idx == 1)
		{
			// low nibble
			_recv_crc = (_recv_crc << 4) | ascii_2_nibble(rxChar);

			// compare checksums, so check if we received the sentence correctly
			if (_recv_crc == calculate_nmea_crc(_parser_buff[_storage_idx]))
			{
				// update receive time stamp
				_time_rx_last_rmc_msg = tic();

				// parse the string to single data values
				parse_nmea_rmc_sentence(_parser_buff[_storage_idx], &_rmc_storage[_storage_idx]);

				// copy the raw string from the received buffer
				memcpy(_rmc_raw_string, _parser_buff[_storage_idx], strlen((const char*) _parser_buff[_storage_idx]) + 1);

				// clear received buffer
				memset(_parser_buff[_storage_idx], 0, sizeof(_parser_buff[_storage_idx]));

				// switch the storage bank
				_storage_idx = (_storage_idx + 1) % 2;

			}
			else
			{
				px4debug(eCOMMITF, "rmc crc err \r\n");
			}

			// reset state machine
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

void px4_gps_update(void)
{
	if (_module_state == DISABLE)
	{
		return;
	}

	// parse all chars received until yet
	while (!ring_buffer_empty(&_rb))
	{
		state_machine_process_new_byte(ring_buffer_pop(&_rb));
	}

	// first check if there are new data from gps sensor
	if (toc(_time_rx_last_rmc_msg) > GPS_RECEIVE_TIMEOUT)
	{
		// mark dataset for reading as invalid => is too old
		uint32_t id = (_storage_idx + 1) % 2;
		_rmc_storage[id].Valid = 0;
	}
}

void px4_gps_get(gps_rmc_packet_st * pData)
{
	uint32_t id = (_storage_idx + 1) % 2;
	memcpy(pData, &_rmc_storage[id], sizeof(gps_rmc_packet_st));
}

void px4_gps_get_raw(uint8_t * buff)
{
	memcpy(buff, _rmc_raw_string, strlen((const char*) _rmc_raw_string) + 1);
}

void px4_gps_rx_complete_event()
{
	// if there are enought free space in the ring buffer
	// move to next position, otherwise overwrite last position.
	// this is more secure, in worst case one sentence can get lost
	if (ring_buffer_free_space(&_rb) >= GPS_RX_PACKAGE_LENGTH)
	{
		_rb.write = (_rb.write + GPS_RX_PACKAGE_LENGTH) % GPS_RING_BUFF_SIZE;
	}
	else
	{
		px4debug(eCOMMITF, "gps. no space in rb\r\n");
	}

	// TODO check write limit on single operation (write pointer overflow)
	// reload interrupt for receiving next chars
	HAL_UART_Receive_IT(&GpsUart, &(_rb.buff[_rb.write]), GPS_RX_PACKAGE_LENGTH);
}

static uint8_t calculate_nmea_crc(uint8_t * buff)
{
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

	return ret;
}

static inline uint8_t ring_buffer_pop(ring_buffer * b)
{
	uint8_t ret = b->buff[b->read++];
	b->read = b->read % GPS_RING_BUFF_SIZE;	// avoid overflow
	return ret;
}

static inline uint32_t ring_buffer_empty(ring_buffer * b)
{
	return b->read == b->write;
}

// TODO see implementation ring_buffer_free_space in sd card
static uint32_t ring_buffer_free_space(ring_buffer * b)
{
	return (GPS_RING_BUFF_SIZE + b->read - b->write - 1) % GPS_RING_BUFF_SIZE;
}

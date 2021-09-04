#include "comm_itf.h"
#include <stdarg.h>

// Commands
#define MPU6000_TOGLE_LOG_ACC  		"log acc"
#define MPU6000_TOGLE_LOG_GYRO 		"log gyro"
#define MS5611_TOGLE_LOG_BARO 		"log baro"
#define HMC5883_TOGLE_LOG_MAG 		"log mag"
#define TARGET_TOGLE_CPU_LOAD 		"log cpu"
#define GPS_TOGLE_LOG_POSITION		"log pos"
#define GPS_TOGLE_LOG_RAW_RMC		"log rmc"
#define RC_INPUT_TOGLE_LOG_CH_VAL	"log rc"
#define COMMON_LOG_OFF				"log off"
#define COMMON_LOG_SIM_SIGNALS  	"log sim"
#define TASK_LOAD 					"taskload"
#define TOP 	 					"top"

#define SD_CARD_LIST_FILE  			"list "
#define SD_CARD_DEL_FILE  			"del "

#define RX_BUFFER_SIZE			50
#define PARSER_BUFFER_SIZE		20

typedef enum
{
	COMPLETED = 0U, NOT_COMPLETED = !COMPLETED
} completed_status;

/*
 * flags for enabling or disabling print infos
 */
typedef struct
{
	uint32_t mpu_acc;
	uint32_t mpu_gyro;
	uint32_t mag;
	uint32_t baro;
	uint32_t pwm_main;
	uint32_t pwm_aux;
	uint32_t cpu_load;
	uint32_t gps_pos;
	uint32_t gps_raw;
	uint32_t rc_input;
	uint32_t top;
} st_print_map_st;

/*
 * Forward Declarations
 */
static void process_received_cmd(void);
static void check_rx_buff(void);
static void print_help();
void print_task_load();

void process_cyclic_print(void);

/*
 * global variables
 */
static uint32_t _moule_state = DISABLE;
static uint32_t _cmd_received = NOT_COMPLETED;

static uint8_t _rx_buff[RX_BUFFER_SIZE];
static uint8_t _parser_buff[PARSER_BUFFER_SIZE];
static st_print_map_st _print_map;

static uint32_t _rx_idx = 0;
static uint32_t _parse_idx = 0;

UART_HandleTypeDef CommUart;

static TaskStatus_t last_task_state [20];
static uint64_t last_total_tick_counter;

void send_over_uart(const char * str)
{
	HAL_UART_Transmit(&CommUart, (uint8_t*) str, strlen(str), 100);
}

void comm_itf_print_string(const char * str)
{
	send_over_uart(str);
}

void comm_itf_init()
{
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_USART2_CLK_ENABLE();

	CommUart.Instance = USART2;
	CommUart.Init.BaudRate = 57600;
	CommUart.Init.WordLength = UART_WORDLENGTH_8B;
	CommUart.Init.StopBits = UART_STOPBITS_1;
	CommUart.Init.Parity = UART_PARITY_NONE;
	CommUart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	CommUart.Init.Mode = UART_MODE_TX_RX;
	CommUart.Init.OverSampling = UART_OVERSAMPLING_8;
	if (HAL_UART_Init(&CommUart) != HAL_OK)
	{
		error_handler(0);
	}

	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin = GPIO_PIN_5 | GPIO_PIN_6;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
	GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

	// setup interrupt
	HAL_NVIC_SetPriority(USART2_IRQn, COMM_ITF_UART_IRQ_PRIO, 0);
	HAL_NVIC_ClearPendingIRQ(USART2_IRQn);
	HAL_NVIC_EnableIRQ(USART2_IRQn);

	memset(_rx_buff, 0, sizeof(_rx_buff));
	memset(_parser_buff, 0, sizeof(_parser_buff));
	memset(&_print_map, DISABLE, sizeof(_print_map));
	memset(&last_task_state, 0, sizeof(last_task_state));

	if (HAL_UART_Receive_IT(&CommUart, &_rx_buff[0], RX_BUFFER_SIZE) != HAL_OK)
	{
		px4debug("comm itf HAL_UART_Receive_IT err!\r\n");
		error_handler(0);
	}

	_moule_state = ENABLE;

	px4debug("\n\n\ncomm module init ok \r\n");
}

static void check_rx_buff()
{
	while (_rx_buff[_rx_idx] != 0)
	{
		_parser_buff[_parse_idx] = _rx_buff[_rx_idx];	// copy char to parser buff
		_rx_buff[_rx_idx] = 0;							// delete copied char from rx buff

		_rx_idx++;
		_rx_idx %= RX_BUFFER_SIZE;			// avoid buffer access violation

		if (_parser_buff[_parse_idx] == 10)	// if 'NL' char received
		{
			_cmd_received = COMPLETED;		// mark  as "command string received completely"
			_parse_idx = 0; 				// reset index counter for receiving new command
			break;
		}
		_parse_idx++;
		_parse_idx %= PARSER_BUFFER_SIZE;	// avoid buffer access violation
	}
}

void comm_itf_task_function(void)
{
	if (_moule_state == DISABLE)
	{
		return;
	}

	// 1. process interaction with user
	check_rx_buff();

	if (_cmd_received == COMPLETED)
	{
		process_received_cmd();
		_cmd_received = NOT_COMPLETED;
	}

	// 2.
	process_cyclic_print();

	// 3.
	QueueHandle_t comm_itf_queue_handle = getQueueHandleByEnum(eCOMMITF);
	BaseType_t ret = pdFALSE;

	do
	{
		char *queueRecvString = NULL;
		ret = xQueueReceive(comm_itf_queue_handle, &queueRecvString, portTICK_PERIOD_MS * 10); // wait 10ms
		if (ret == pdTRUE && queueRecvString != NULL)
		{
			send_over_uart(queueRecvString);
			vPortFree(queueRecvString);
		}
		else
		{
			// comm_itf_print_string("queue is empty");
		}

	} while (ret == pdTRUE);
}

void process_cyclic_print(void)
{
	static uint32_t timer = 0;
	if (toc(timer) < 1000000)
	{
		return;
	}

	timer = tic();

	if (_print_map.mag)
	{
		hmc5883_data_st data;
		px4_hmc5883_get(&data);
		px4debug("mag: %f %f %f \r\n", data.magX, data.magY, data.magZ);
	}

	if (_print_map.mpu_acc)
	{
		mpu6000_data_st data;
		px4_mpu6000_get(&data);
		px4debug("mpu_acc: %f %f %f \r\n", data.accel_x, data.accel_y, data.accel_z);
	}

	if (_print_map.mpu_gyro)
	{
		mpu6000_data_st data;
		px4_mpu6000_get(&data);
		px4debug("mpu_gyro %f %f %f \r\n", data.gyro_x, data.gyro_y, data.gyro_z);
	}

	if (_print_map.cpu_load)
	{
		px4debug("cpu load:%d%% peak:%d%% \r\n", cpu_load_get_curr_cpu_load(), cpu_load_get_max_cpu_load());
	}

	if (_print_map.gps_pos)
	{
		gps_rmc_packet_st pack;
		px4_gps_get(&pack);
		px4debug("gps lat:%f lon:%f valid:%d \r\n", pack.Latitude, pack.Longitude, pack.Valid);
	}

	if (_print_map.gps_raw)
	{
		uint8_t tmp[GPS_SENTENCE_BUFF_SIZE];
		px4_gps_get_raw(tmp);
		px4debug("%s\r\n", tmp);
	}

	if (_print_map.baro)
	{
		ms5611_data_st data;
		px4_ms5611_get(&data);
		px4debug("baro: %f \r\n", data.baroValue);
	}

	if (_print_map.rc_input)
	{
		rc_ppm_input_data_st data;
		px4_rc_ppm_input_get(&data);

		if (data.channel_cnt > 0)
		{
			for (uint32_t i = 0; i < data.channel_cnt; i++)
			{
				px4debug("ch%d:%d", (i + 1), data.channels[i]);
			}
			px4debug("\r\n");
		}
		else
		{
			px4debug("no rc input data\r\n");
		}
	}
	if(_print_map.top)
	{
		print_task_load();
	}
}

void process_received_cmd(void)
{
	const char * buff = (const char*) _parser_buff;

	if (strncmp(buff, MPU6000_TOGLE_LOG_ACC, strlen(MPU6000_TOGLE_LOG_ACC)) == 0)
	{
		_print_map.mpu_acc = !_print_map.mpu_acc;
	}
	else if (strncmp(buff, HMC5883_TOGLE_LOG_MAG, strlen(HMC5883_TOGLE_LOG_MAG)) == 0)
	{
		_print_map.mag = !_print_map.mag;
	}
	else if (strncmp(buff, MPU6000_TOGLE_LOG_GYRO, strlen(MPU6000_TOGLE_LOG_GYRO)) == 0)
	{
		_print_map.mpu_gyro = !_print_map.mpu_gyro;
	}
	else if (strncmp(buff, TARGET_TOGLE_CPU_LOAD, strlen(TARGET_TOGLE_CPU_LOAD)) == 0)
	{
		_print_map.cpu_load = !_print_map.cpu_load;
	}
	else if (strncmp(buff, COMMON_LOG_OFF, strlen(COMMON_LOG_OFF)) == 0)
	{
		memset(&_print_map, DISABLE, sizeof(_print_map));
		px4_signal_output_set_log(DISABLE);
	}
	else if (strncmp(buff, GPS_TOGLE_LOG_POSITION, strlen(GPS_TOGLE_LOG_POSITION)) == 0)
	{
		_print_map.gps_pos = !_print_map.gps_pos;
	}
	else if (strncmp(buff, GPS_TOGLE_LOG_RAW_RMC, strlen(GPS_TOGLE_LOG_RAW_RMC)) == 0)
	{
		_print_map.gps_raw = !_print_map.gps_raw;
	}
	else if (strncmp(buff, RC_INPUT_TOGLE_LOG_CH_VAL, strlen(RC_INPUT_TOGLE_LOG_CH_VAL)) == 0)
	{
		_print_map.rc_input = !_print_map.rc_input;
	}
	else if (strncmp(buff, COMMON_LOG_SIM_SIGNALS, strlen(COMMON_LOG_SIM_SIGNALS)) == 0)
	{
		px4_signal_output_set_log(ENABLE);
	}
	else if (strncmp(buff, MS5611_TOGLE_LOG_BARO, strlen(MS5611_TOGLE_LOG_BARO)) == 0)
	{
		_print_map.baro = !_print_map.baro;
	}
	else if (strncmp(buff, SD_CARD_LIST_FILE, strlen(SD_CARD_LIST_FILE)) == 0)
	{
		px4_sd_card_logger_add_user_cmd(buff);
	}
	else if (strncmp(buff, SD_CARD_DEL_FILE, strlen(SD_CARD_DEL_FILE)) == 0)
	{
		px4_sd_card_logger_add_user_cmd(buff);
	}
	else if (strncmp(buff, TASK_LOAD, strlen(TASK_LOAD)) == 0)
	{
		print_task_load();
	}
	else if (strncmp(buff, TOP, strlen(TOP)) == 0)
	{
		_print_map.top = !_print_map.top;
	}
	else if (strncmp(buff, "help", 4) == 0)
	{
		print_help();
	}
	else
	{
		px4debug("UNKNOWN COMMAND! Type <help> for command info \r\n");
	}
}

void print_help()
{
	px4debug("=============================== \r\n");
	px4debug("= Follow commands are allowed = \r\n");
	px4debug("===============================\r\n");
	px4debug("\r\nCOMMANDS FOR SENSOR DATA \r\n");
	px4debug("------------------------ \r\n");
	px4debug("'log acc' 	- log acceleration data\r\n");
	px4debug("'log gyro' 	- log gyroscope sensor data\r\n");
	px4debug("'log baro' 	- log barometer data\r\n");
	px4debug("'log mag' 	- log compass data\r\n");
	px4debug("'log pos' 	- log gps position\r\n");
	px4debug("'log rmc' 	- log raw GPS-RMC-Sentence\r\n");
	px4debug("'log rc' 	- log received remote control values\r\n");
	px4debug("'log sim' 	- log values from signal logger\r\n");
	px4debug("\r\nCOMMANDS FOR SD CARD FILE MANAGEMENT\r\n");
	px4debug("------------------------------------\r\n");
	px4debug("'list all'        - list all existing logfile names\r\n");
	px4debug("'list <filename>' - list the logfile with name <filename> on console\r\n");
	px4debug("'del all'         - delete all log files\r\n");
	px4debug("'del <filename>'  - delete the logfile with name <filename>\r\n");
	px4debug("\r\nOTHER COMANDS\r\n");
	px4debug("-------------\r\n");
	px4debug("'log cpu'       - log cpu usage\r\n");
	px4debug("'log off'       - disable all logging\r\n");
	px4debug("'taskload'		- log task cpu usage since last taskload call\r\n");
	px4debug("'top'           - log cyclic task cpu usage\r\n");
	px4debug("===============================\r\n");
}

int32_t find_index_of_task(TaskStatus_t * t, const char * s, uint32_t size)
{

	for (uint32_t i = 0; i < size; i++)
		if (strcmp(t[i].pcTaskName, s) == 0)
			return i;

	return -1;
}

void print_task_load()
{
	static uint32_t firstcall = 1;

	TaskStatus_t val[20];
	uint32_t total = 1;
	UBaseType_t size = uxTaskGetSystemState((TaskStatus_t * const ) &val, 20, &total);

	if(firstcall)
	{
		firstcall = 0;
		memcpy(last_task_state, val, sizeof(val));
		last_total_tick_counter = total;
		return;
	}

	px4debug("\r\n\r\n");
	px4debug("=== TASK INFORMATION===\r\n");
	px4debug("----------------------- \r\n");
	px4debug("%-20s%-20s%-10s", "Task name", "|Stack watermark", "|CPU(%)\r\n");
	px4debug("--------------------------------------------------\r\n");

	// print new calculated values since last call
	for (unsigned int i = 0; i < size; i++)
	{
		int32_t ind = find_index_of_task((TaskStatus_t * const ) &last_task_state, val[i].pcTaskName, size);

		if(ind == -1)
			continue;

		px4debug("%-20s|%-20d|%-10d\r\n", val[i].pcTaskName, val[i].usStackHighWaterMark,
				((val[i].ulRunTimeCounter - last_task_state[ind].ulRunTimeCounter) * 100) / (total - last_total_tick_counter));
	}
	px4debug("--------------------------------------------------\r\n");

	px4debug("===== HEAP STATE =======\r\n");
	size_t fr = xPortGetFreeHeapSize();
	size_t mfr = xPortGetMinimumEverFreeHeapSize();

	px4debug("free: %d\r\n", fr);
	px4debug("min free: %d\r\n", mfr);

	px4debug("-------------------------\r\n");

	memcpy(last_task_state, val, sizeof(val));
	last_total_tick_counter = total;
}

void px4debug(char * MESSAGE, ...)
{
	va_list arg;
	va_start(arg, MESSAGE);

	int messageSize = 256;
	int cnt = -1;

	// if no sceduler is running (queue mechanism doesn't active yet)
	if (xTaskGetSchedulerState() != taskSCHEDULER_RUNNING)
	{
		// use dynamic strings
		char arr[256];
		cnt = vsnprintf(arr, messageSize, MESSAGE, arg);
		if (cnt == messageSize-1)
		{
			comm_itf_print_string("WARNING! px4debug string size limit reached \r\n");
		}
		comm_itf_print_string(arr);
	}
	else
	{
		// logging from regular modules and scheduler is already running
		// so create a message and send over queue to comm task
		char * pcStringToSend = (char *) pvPortMalloc(messageSize);
		cnt = vsnprintf(pcStringToSend, messageSize, MESSAGE, arg);
		if (cnt == messageSize)
		{
			px4debug("WARNING! px4debug string size limit reached \r\n");
		}

		QueueHandle_t msgQueue = getQueueHandleByEnum(eCOMMITF);
		if (msgQueue != NULL)
		{
			if ( xQueueSend(msgQueue, &pcStringToSend, 0) != pdTRUE)
			{
				// px4debug("Queue is full, caused by %d \r\n", id);
				// adding to queue failed, free memory
				vPortFree(pcStringToSend);
			}
			else
			{
				// comm_itf_print_string("Couldn't sent queue message \r\n");
			}
		}
	}
	va_end(arg);
}

void comm_itf_rx_complete_event()
{
	HAL_UART_Receive_IT(&CommUart, &_rx_buff[0], RX_BUFFER_SIZE); // Reload rx interrupt
}


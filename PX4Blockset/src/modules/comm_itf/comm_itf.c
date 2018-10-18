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
#define COMMON_LOG_RUNTIMES			"log runtime"
#define COMMON_LOG_SIM_SIGNALS  	"log sim"
#define TASK_LOAD 					"taskload"

#define SD_CARD_LIST_FILE  			"list "
#define SD_CARD_DEL_FILE  			"del "

#define RX_BUFFER_SIZE			50
#define PARSER_BUFFER_SIZE		20

typedef enum
{
  COMPLETED = 0U,
  NOT_COMPLETED = !COMPLETED
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
	uint32_t runtimes;
} st_print_map_st;

/*
 * Forward Declarations
 */
static void process_received_cmd(void);
static void check_rx_buff(void);
static void print_help();
void print_task_load();

void process_cyclic_print(void );


/*
 * global variables
 */
static uint32_t _moule_state  = DISABLE;
static uint32_t _cmd_received = NOT_COMPLETED;

static uint8_t _rx_buff[RX_BUFFER_SIZE];
static uint8_t _parser_buff[PARSER_BUFFER_SIZE];
static st_print_map_st _print_map;

static uint32_t _rx_idx = 0;
static uint32_t _parse_idx = 0;

UART_HandleTypeDef CommUart;

void send_over_uart(const char * str)
{
	HAL_UART_Transmit(&CommUart, (uint8_t*)str, strlen(str), 10);
}


void comm_itf_print_string(const char * str)
{
	send_over_uart(str);
	send_over_uart("\r\n");
}

void comm_itf_print_float(float val)
{
	char arr[30];
	my_ftoa(val, arr, 6);
	comm_itf_print_string(arr);
}

void comm_itf_print_int(int val)
{
	char arr[20];
	my_i32toa(val, arr);
	comm_itf_print_string(arr);
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
	CommUart.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_UART_Init(&CommUart) != HAL_OK)
	{
		debug_print_string("comm itf HAL_UART_Init err!\r\n");
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

	if (HAL_UART_Receive_IT(&CommUart, &_rx_buff[0], RX_BUFFER_SIZE) != HAL_OK)
	{
		debug_print_string("comm itf HAL_UART_Receive_IT err!\r\n");
		error_handler(0);
	}

	_moule_state = ENABLE;

	debug_print_string("\n\n\ncomm module init ok \r\n");
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

void comm_itf_task_function(void const * argv)
{
	if (_moule_state == DISABLE)
	{
		return;
	}

	// ===============================================
	// process interaction with user
	check_rx_buff();

	if (_cmd_received == COMPLETED)
	{
		process_received_cmd();
		_cmd_received = NOT_COMPLETED;
	}

	// ===============================================
	process_cyclic_print();

	// ===============================================
	// check logging queues from other tasks
	px4_task * task = (px4_task*) argv;

	QueueHandle_t nextQueueWithData = NULL;
	char *queueRecvString = NULL;

	do
	{
		nextQueueWithData = (QueueHandle_t) xQueueSelectFromSet(task->queueSet, 0);
		if (nextQueueWithData != NULL)
		{
			BaseType_t ok = errQUEUE_EMPTY;
			do
			{
				ok = xQueueReceive(nextQueueWithData, &queueRecvString, 0);

				if(ok == pdPASS)
				{
					comm_itf_print_string(queueRecvString);
					vPortFree(queueRecvString);
				}
				else
				{
					// comm_itf_print_string("queue is empty");
				}
			}
			while(ok == pdPASS);
		}
	} while (nextQueueWithData != NULL);
}

void process_cyclic_print(void)
{
	if (_print_map.mag)
	{
		hmc5883_data_st data;
		px4_hmc5883_get(&data);
		px4debug(eCOMMITF, "mag: %f %f %f", data.magX, data.magY, data.magZ);
	}

	if (_print_map.mpu_acc)
	{
		mpu6000_data_st data;
		px4_mpu6000_get(&data);
		px4debug(eCOMMITF, "mpu_acc: %f %f %f", data.accel_x, data.accel_y, data.accel_z);
	}

	if (_print_map.mpu_gyro)
	{
		mpu6000_data_st data;
		px4_mpu6000_get(&data);
		comm_itf_print_string("mpu_gyro ");
		comm_itf_print_float(data.gyro_x);
		comm_itf_print_string(" ");
		comm_itf_print_float(data.gyro_y);
		comm_itf_print_string(" ");
		comm_itf_print_float(data.gyro_z);
		comm_itf_print_string("\r\n");
	}

	if (_print_map.cpu_load)
	{
		comm_itf_print_string("cpu load:");
		comm_itf_print_int(cpu_load_get_curr_cpu_load());
		comm_itf_print_string("% peak:");
		comm_itf_print_int(cpu_load_get_max_cpu_load());
		comm_itf_print_string("%\r\n");
	}

	if (_print_map.gps_pos)
	{
		gps_rmc_packet_st pack;
		px4_gps_get(&pack);
		comm_itf_print_string("gps pos. lat: ");
		comm_itf_print_float(pack.Latitude);
		comm_itf_print_string(" lon: ");
		comm_itf_print_float(pack.Longitude);
		comm_itf_print_string(" valid:");
		comm_itf_print_int(pack.Valid);
		comm_itf_print_string("\r\n");
	}

	if (_print_map.gps_raw)
	{
		uint8_t tmp[GPS_SENTENCE_BUFF_SIZE];
		px4_gps_get_raw(tmp);
		comm_itf_print_string((const char *) tmp);
		comm_itf_print_string("\r\n");
	}

	if (_print_map.baro)
	{
		ms5611_data_st data;
		px4_ms5611_get(&data);
		comm_itf_print_string("baro: ");
		comm_itf_print_float(data.baroValue);
		comm_itf_print_string("\r\n");
	}

	if (_print_map.rc_input)
	{
		rc_ppm_input_data_st data;
		px4_rc_ppm_input_get(&data);

		if (data.channel_cnt > 0)
		{
			for (uint32_t i = 0; i < data.channel_cnt; i++)
			{
				comm_itf_print_string(" ch");
				comm_itf_print_int(i + 1);
				comm_itf_print_string(":");
				comm_itf_print_int(data.channels[i]);
			}
			comm_itf_print_string("\r\n");
		}
		else
		{
			comm_itf_print_string("no rc input data\r\n");
		}
	}

	if (_print_map.runtimes)
	{
		comm_itf_print_string("Runtimes (µs). ");

		comm_itf_print_string("app: ");
		comm_itf_print_int(app_runtime);

		comm_itf_print_string(" rc: ");
		comm_itf_print_int(px4_rc_ppm_input_getruntime());

		comm_itf_print_string(", main: ");
		comm_itf_print_int(px4_pwm_main_out_getruntime());

		comm_itf_print_string(", aux: ");
		comm_itf_print_int(px4_pwm_aux_out_getruntime());

		comm_itf_print_string(", mpu6000: ");
		comm_itf_print_int(px4_mpu6000_get_runtime());

		comm_itf_print_string(", ms5611: ");
		comm_itf_print_int(px4_ms5611_get_runtime());

		comm_itf_print_string(", hmc5883: ");
		comm_itf_print_int(px4_hmc5883_getruntime());

		comm_itf_print_string(", cLED: ");
		comm_itf_print_int(px4_color_power_led_getruntime());

		uint32_t * gpsruntimes = px4_gps_getruntimes();
		comm_itf_print_string(", gps parse: ");
		comm_itf_print_int(gpsruntimes[0]);

		comm_itf_print_string(", gps updt: ");
		comm_itf_print_int(gpsruntimes[1]);

		comm_itf_print_string(", gps crc: ");
		comm_itf_print_int(gpsruntimes[2]);

		comm_itf_print_string(", gps it: ");
		comm_itf_print_int(gpsruntimes[3]);

		comm_itf_print_string(", sig_log: ");
		comm_itf_print_int(px4_signal_output_getruntime());

		comm_itf_print_string(", sd_log: ");
		comm_itf_print_int(px4_sd_card_logger_getruntime());

		comm_itf_print_string("\r\n");
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
	else if (strncmp(buff, COMMON_LOG_RUNTIMES, strlen(COMMON_LOG_RUNTIMES)) == 0)
	{
		_print_map.runtimes = !_print_map.runtimes;
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
		px4_sd_card_logger_process_cmd(buff);
	}
	else if (strncmp(buff, SD_CARD_DEL_FILE, strlen(SD_CARD_DEL_FILE)) == 0)
	{
		px4_sd_card_logger_process_cmd(buff);
	}
	else if (strncmp(buff, TASK_LOAD, strlen(TASK_LOAD)) == 0)
	{
		print_task_load();
	}
	else if (strncmp(buff, "help", 4) == 0)
	{
		print_help();
	}
	else
	{
		comm_itf_print_string("UNKNOWN COMMAND! Type <help> for command info \r\n");
	}
}

void print_help()
{
	comm_itf_print_string("===============================\r\n");
	comm_itf_print_string("= Follow commands are allowed =\r\n");
	comm_itf_print_string("===============================\r\n");

	comm_itf_print_string("\r\nCOMMANDS FOR SENSOR DATA\r\n");
	comm_itf_print_string("------------------------\r\n");
	comm_itf_print_string("'log acc' 	- log acceleration data \r\n");
	comm_itf_print_string("'log gyro' 	- log gyroscope sensor data \r\n");
	comm_itf_print_string("'log baro' 	- log barometer data \r\n");
	comm_itf_print_string("'log mag' 	- log compass data \r\n");
	comm_itf_print_string("'log pos' 	- log gps position \r\n");
	comm_itf_print_string("'log rmc' 	- log raw GPS-RMC-Sentence \r\n");
	comm_itf_print_string("'log rc' 	- log received remote control values \r\n");
	comm_itf_print_string("'log sim' 	- log values from signal logger \r\n");

	comm_itf_print_string("\r\nCOMMANDS FOR SD CARD FILE MANAGEMENT\r\n");
	comm_itf_print_string("------------------------------------\r\n");
	comm_itf_print_string("'list all' 		 - list all existing logfile names \r\n");
	comm_itf_print_string("'list <filename>' - list the logfile with name <filename> on console\r\n");
	comm_itf_print_string("'del all' 		 - delete all log files \r\n");
	comm_itf_print_string("'del <filename>'  - delete the logfile with name <filename>\r\n");

	comm_itf_print_string("\r\nOTHER COMANDS\r\n");
	comm_itf_print_string("-------------\r\n");
	comm_itf_print_string("'log cpu' 		- log cpu usage \r\n");
	comm_itf_print_string("'log runtime'	- log calculated runtimes of all modules\r\n");
	comm_itf_print_string("'log off' 		- disable all logging \r\n");
	comm_itf_print_string("'taskload' 		- log task cpu usage since system start\r\n");
	comm_itf_print_string("'top' 			- log cyclic task cpu usage\r\n");

	comm_itf_print_string("===============================\r\n");
}

void print_task_load()
{
	TaskStatus_t val[10];
	uint32_t total = 1;
	UBaseType_t size = uxTaskGetSystemState((TaskStatus_t * const ) &val, 10, &total);

	debug_print_string("TASK INFORMATION\n-------------------------");
	debug_print_string("\nTask name \t Stack water mark \t %CPU \n");
	for (unsigned int i = 0; i < size; i++)
	{
		debug_print_string(val[i].pcTaskName);

		debug_print_string("\t");
		debug_print_int(val[i].usStackHighWaterMark);

		debug_print_string("\t");
		debug_print_int((val[i].ulRunTimeCounter * 100)/total);

		debug_print_string("\n");
	}
	debug_print_string("\n-------------------------\n");
}

/***********************************************************/
#ifdef DEBUG
void debug_print_int(int val)
{
	comm_itf_print_int(val);
}

void debug_print_float(float val)
{
	comm_itf_print_float(val);
}

void debug_print_string(const char * str)
{
	comm_itf_print_string(str);
}

void px4debug(eTaskID id, char * MESSAGE, ...)
{
	va_list arg;
	va_start(arg, MESSAGE);
	int messageSize = 100;
	char * pcStringToSend = (char *) pvPortMalloc(messageSize);
	int cnt = vsnprintf(pcStringToSend, messageSize, MESSAGE, arg);

	if (cnt == messageSize)
	{
		px4debug(id, "WARNING! px4debug string size limit reached");
	}

	if(xTaskGetSchedulerState() == taskSCHEDULER_RUNNING)
	{
		QueueHandle_t msgQueue = getHandleByEnum(id);
		if ( xQueueSend(msgQueue, &pcStringToSend, 0) != pdPASS)
		{
			// comm_itf_print_string("Queue is full");
			// free memory, because adding to queue was failed
			vPortFree(pcStringToSend);
		}
		else
		{
			// comm_itf_print_string("Queue add OK");
		}
	}
	else
	{
		comm_itf_print_string(pcStringToSend);
		// free memory, because printing directly to output
		vPortFree(pcStringToSend);
	}

	va_end(arg);
}

#else

void mycustomprint(QueueHandle_t msgQueue, char * MESSAGE, ...)
{
	UNUSED(msgQueue);
	UNUSED(MESSAGE);
}

void debug_print_int(int val)
{
	UNUSED(val);
}

void debug_print_float(float val)
{
	UNUSED(val);
}

void debug_print_string(const char * str)
{
	UNUSED(str);
}


#endif


//---------------------------------------------------------------------------------
void comm_itf_rx_complete_event()
{
	HAL_UART_Receive_IT(&CommUart, &_rx_buff[0], RX_BUFFER_SIZE); // Reload rx interrupt
}









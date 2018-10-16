#include "tasks.h"
#include "defines.h"
#include "mpu6000.h"
#include "ms5611.h"
#include "hmc5883.h"
#include "gps.h"
#include "fmu_amber_led.h"
#include "rc_ppm_input.h"
#include "pwm_aux_out.h"
#include "pwm_main_out.h"
#include "color_power_led.h"
#include "sd_card_logger.h"
#include "cpu_load.h"

#define MSG_QUEUE_SIZE	20

void Common_Task(void const * argv);

px4_task _tasklist[eMaxCount];
SemaphoreHandle_t _mpu6000Mutex, _pxioMutex;

static QueueSetHandle_t commQueueSet = NULL;

void px4_tasks_register_task(eTaskID id,  const char * name, callback_t func, uint32_t sampleTime, uint32_t stacksize, uint32_t taskPrio)
{
	_tasklist[id].taskID 		= id;
	_tasklist[id].taskFunction 	= func;
	_tasklist[id].stackSize 	= (stacksize == 0) ? configMINIMAL_STACK_SIZE : stacksize;
	_tasklist[id].taskPrio 		= taskPrio;
	_tasklist[id].sampleTime 	= sampleTime;

	_tasklist[id].msgQueue 		= xQueueCreate(MSG_QUEUE_SIZE, sizeof(char *));

	xQueueAddToSet(_tasklist[id].msgQueue, commQueueSet );

	memcpy(_tasklist[id].name, name, strlen(name));

	debug_print_string("Register task \"");
	debug_print_string(_tasklist[id].name);
	debug_print_string("\"\n");

	uint32_t ret = 0;

	switch (id)
	{

	case ePWM_MAIN:
		_tasklist[id].mutex = &_pxioMutex;

		break;

	case ePPM_INPUT:
		_tasklist[id].mutex = &_pxioMutex;
		break;

	case eMPU6000:
		_tasklist[id].mutex = &_mpu6000Mutex;
		break;

	case eMS5611:
		_tasklist[id].mutex = &_mpu6000Mutex;
		break;

	case eCOMMITF:
		_tasklist[id].queueSet = commQueueSet;
		break;

	default:
		_tasklist[id].mutex = NULL;
		break;
	}

	ret = xTaskCreate(	(TaskFunction_t) Common_Task,
						_tasklist[id].name,
						_tasklist[id].stackSize,
						(void*) &(_tasklist[id]),
						_tasklist[id].taskPrio,
						&(_tasklist[id].threadID)
					);

	if (!ret)
	{
		debug_print_string("error creating task\n");
	}
}

void Common_Task(void const * argv)
{
	px4_task * task = (px4_task*) argv;

	char * pcStringToSend = (char *) pvPortMalloc(100);
	snprintf(pcStringToSend, 100, "entry task \"%s\"\r\n", task->name);
	xQueueSend(task->msgQueue, &pcStringToSend, portMAX_DELAY);

	TickType_t xLastWakeTime = xTaskGetTickCount();

	uint32_t mutex_exist = task->mutex != NULL;
	uint32_t mutex_taken = DISABLE;

	// if mutex is available, use the half of the sample time as wait time for reserving the mutex
	uint32_t mutex_wait_time = (task->sampleTime > 1) ? (task->sampleTime / 2) : 1;

	while (1)
	{
		if (mutex_exist)
		{
			mutex_taken = xSemaphoreTake(*(task->mutex), mutex_wait_time);
		}

		if(mutex_exist & !mutex_taken)
		{
			debug_print_string("mutex not taken\n");
		}

		if ((mutex_exist & mutex_taken) || (!mutex_exist))
		{
			if (task->taskFunction != NULL)
			{
				task->taskFunction(argv);
			}
			if (mutex_exist)
			{
				xSemaphoreGive(*(task->mutex));
			}
		}
		osDelayUntil(&xLastWakeTime, task->sampleTime);
	}
}

void px4_tasks_initialize()
{
	memset(_tasklist, 0, sizeof(_tasklist));
	_mpu6000Mutex 	= xSemaphoreCreateMutex();
	_pxioMutex 		= xSemaphoreCreateMutex();

	commQueueSet = xQueueCreateSet(eMaxCount);

//	const size_t xMaxStringLength = 50;
//	BaseType_t xStringNumber = 0;
//	char *pcStringToSend;
//
//	pcStringToSend = ( char * ) prvGetBuffer( xMaxStringLength );
//	snprintf( pcStringToSend, xMaxStringLength, "String number %d\r\n", xStringNumber );
//
//	xQueueSend( xPointerQueue, /* The QueueHandle_t handle of the queue. */
//	&pcStringToSend, /* The address of the pointer that points to the buffer. */
//	portMAX_DELAY );


	debug_print_string("Tasks init ok\n");
}

void px4_tasks_run()
{
	debug_print_string("Start scheduler\n");
	vTaskStartScheduler();
}

// TODO Reaction on stack overflow?
void vApplicationStackOverflowHook(TaskHandle_t *pxTask, signed char *pcTaskName)
{
	debug_print_string("RTOS stack overflow by ");
	debug_print_string((const char *) pcTaskName);
	debug_print_string("\r\n");
}

void vApplicationMallocFailedHook(void)
{
	debug_print_string("RTOS malloc failed \n");
}


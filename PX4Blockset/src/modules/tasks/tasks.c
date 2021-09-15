#include "comm_itf.h"
#include "tasks.h"
#include "defines.h"

#define MSG_QUEUE_SIZE_DEFAULT	5

void Common_Task(void const * argv);

px4_task tasklist[eMaxCount];
SemaphoreHandle_t _spiMutex, _pxioMutex;

QueueHandle_t getQueueHandleByEnum(eTaskID id)
{
	return tasklist[id].msgQueue;
}

const char * getTaskNameByEnum(eTaskID id)
{
	switch (id)
	{
		case eCOMMITF: 		return "comm_itf"; 	 break;
		case eSDCARD: 		return "sdcard"; 	 break;
		case eCPU_LOAD: 	return "cpu_load"; 	 break;
		case eAPPL: 		return "appl"; 		 break;
		case ePWM_AUX: 		return "pwm_aux"; 	 break;
		case ePWM_MAIN: 	return "pwm_main"; 	 break;
		case eCOLORLED: 	return "color_led";  break;
		case ePPM_INPUT: 	return "ppm_input";  break;
		case eGPS: 			return "gps"; 		 break;
		case eHMC5883: 		return "hmc5883"; 	 break;
		case eMPU6000: 		return "mpu6000"; 	 break;
		case eMS5611: 		return "ms5611"; 	 break;
		case eSIGLOGGER: 	return "sig_logger"; break;
		default: 			return "unknown";
	}
}

void px4_tasks_register_task(eTaskID id, callback_t func, uint32_t sampleTimeMS, uint32_t stacksize, uint32_t taskPrio)
{
	tasklist[id].taskID 		= id;
	tasklist[id].taskFunction 	= func;
	tasklist[id].stackSize 	    = max(stacksize, configMINIMAL_STACK_SIZE);
	tasklist[id].taskPrio 		= taskPrio;
	tasklist[id].sampleTime 	= sampleTimeMS;

	const char * taskname = getTaskNameByEnum(id);
	size_t name_length = min(strlen(getTaskNameByEnum(id)) + 1, TASK_NAME_MAX_LENGTH);
	memcpy(tasklist[id].name, taskname, name_length);
	tasklist[id].name[TASK_NAME_MAX_LENGTH] = 0; // string termination

	px4debug("reg task \"%s\"\n", tasklist[id].name);

	uint32_t msgQueueSize = 0;

	// handle common mutexes
	switch (id)
	{
	case ePWM_MAIN:		tasklist[id].mutex = _pxioMutex; break;
	case ePPM_INPUT:	tasklist[id].mutex = _pxioMutex; break;
	case eMPU6000:		tasklist[id].mutex = _spiMutex;  break;
	case eMS5611:		tasklist[id].mutex = _spiMutex;  break;
	default:			tasklist[id].mutex = NULL;		  break;
	}

	// set different message queue size
	switch (id)
	{
	case eCOMMITF: 	msgQueueSize = 100; break;
	default: 		msgQueueSize = MSG_QUEUE_SIZE_DEFAULT;	break;
	}

	tasklist[id].msgQueue = xQueueCreate(msgQueueSize, sizeof(char*));

	uint32_t ret = xTaskCreate(	(TaskFunction_t) Common_Task,
								tasklist[id].name,
								tasklist[id].stackSize,
								(void*) &(tasklist[id]),
								tasklist[id].taskPrio,
								&(tasklist[id].threadID)
							);

	if (!ret)
	{
		px4debug("error creating task\n");
	}
}

void Common_Task(void const * argv)
{
	px4_task * task = (px4_task*) argv;

	px4debug("entry task \"%s\"\n", task->name);

	TickType_t xLastWakeTime = xTaskGetTickCount();

	uint32_t mutex_exist = task->mutex != NULL;
	uint32_t mutex_taken = DISABLE;

	// if mutex is available, use the half of the sample time as wait time for reserving the mutex, but at least 1 ms
	uint32_t mutex_wait_time = (task->sampleTime > 1) ? (task->sampleTime / 2) : 1;

	while (1)
	{
		if (mutex_exist)
		{
			mutex_taken = xSemaphoreTake(task->mutex, mutex_wait_time);
		}

		if(mutex_exist & !mutex_taken)
		{
			px4debug("mutex not taken. Task:%s\n", task->name);
		}

		if ((mutex_exist & mutex_taken) || (!mutex_exist))
		{
			if (task->taskFunction != NULL)
			{
				task->taskFunction(); // execute task function
			}
			if (mutex_exist)
			{
				xSemaphoreGive(task->mutex);
			}
		}
		osDelayUntil(&xLastWakeTime, task->sampleTime);
	}
}

void px4_tasks_initialize()
{
	memset(tasklist, 0, sizeof(tasklist));
	_spiMutex 		= xSemaphoreCreateMutex();
	_pxioMutex 		= xSemaphoreCreateMutex();

	if(_spiMutex == NULL || _pxioMutex == NULL)
	{
		px4debug("Tasks init error\n");
	}
	else
	{
		px4debug("Tasks init ok\n");
	}
}

void px4_tasks_run()
{
	px4debug("Start scheduler\n");
	vTaskStartScheduler();
}

// TODO Reaction on stack overflow?
void vApplicationStackOverflowHook(TaskHandle_t *pxTask, signed char *pcTaskName)
{
	px4debug("RTOS stack overflow caused by %s\n", pcTaskName);
}

// TODO Reaction on failed malloc?
void vApplicationMallocFailedHook(void)
{
	px4debug("RTOS malloc failed\n");
}


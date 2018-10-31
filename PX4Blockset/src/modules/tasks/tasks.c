#include "comm_itf.h"
#include "tasks.h"
#include "defines.h"

#define MSG_QUEUE_SIZE	20

void Common_Task(void const * argv);

px4_task _tasklist[eMaxCount];
SemaphoreHandle_t _spiMutex, _pxioMutex;

static QueueSetHandle_t commQueueSet = NULL;

QueueSetHandle_t px4_tasks_get_queueset(void)
{
	return commQueueSet;
}

QueueHandle_t getQueueHandleByEnum(eTaskID id)
{
	return _tasklist[id].msgQueue;
}

void px4_tasks_register_task(eTaskID id,  const char * name, callback_t func, uint32_t sampleTime, uint32_t stacksize, uint32_t taskPrio)
{
	_tasklist[id].taskID 		= id;
	_tasklist[id].taskFunction 	= func;
	_tasklist[id].stackSize 	= (stacksize < configMINIMAL_STACK_SIZE) ? configMINIMAL_STACK_SIZE : stacksize;
	_tasklist[id].taskPrio 		= taskPrio;
	_tasklist[id].sampleTime 	= sampleTime;
	_tasklist[id].msgQueue 		= xQueueCreate(MSG_QUEUE_SIZE, sizeof(char*));

	xQueueAddToSet(_tasklist[id].msgQueue, commQueueSet);
	memcpy(_tasklist[id].name, name, strlen(name));

	px4debug(eNONE, "Register task \"%s\"\r\n", _tasklist[id].name);

	uint32_t ret = 0;

	switch (id)
	{

	case ePWM_MAIN:
		_tasklist[id].mutex = _pxioMutex;

		break;

	case ePPM_INPUT:
		_tasklist[id].mutex = _pxioMutex;
		break;

	case eMPU6000:
		_tasklist[id].mutex = _spiMutex;
		break;

	case eMS5611:
		_tasklist[id].mutex = _spiMutex;
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
		px4debug(eNONE, "error creating task\r\n");
	}
}

void Common_Task(void const * argv)
{
	px4_task * task = (px4_task*) argv;

	px4debug(task->taskID, "entry task \"%s\"\r\n", task->name);

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
			px4debug(task->taskID, "mutex not taken\r\n");
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
	memset(_tasklist, 0, sizeof(_tasklist));
	_spiMutex 	= xSemaphoreCreateMutex();
	_pxioMutex 		= xSemaphoreCreateMutex();
	commQueueSet = xQueueCreateSet(eMaxCount * MSG_QUEUE_SIZE);

	if(commQueueSet == NULL || _spiMutex == NULL || _pxioMutex == NULL)
	{
		px4debug(eNONE, "Tasks init error\r\n");
	}
	else
	{
		px4debug(eNONE, "Tasks init ok\r\n");
	}

	_tasklist[eDRV].msgQueue 	= xQueueCreate(MSG_QUEUE_SIZE, sizeof(char*));
	xQueueAddToSet(_tasklist[eDRV].msgQueue, commQueueSet);

}

void px4_tasks_run()
{
	px4debug(eNONE, "Start scheduler\r\n");
	vTaskStartScheduler();
}

// TODO Reaction on stack overflow?
void vApplicationStackOverflowHook(TaskHandle_t *pxTask, signed char *pcTaskName)
{
	px4debug(eNONE, "RTOS stack overflow caused by %s\r\n", pcTaskName);
}

// TODO Reaction on failed malloc?
void vApplicationMallocFailedHook(void)
{
	px4debug(eNONE, "RTOS malloc failed\r\n");
}


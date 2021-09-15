/**
*   @author Max Pejs <max.pejs@googlemail.com>
*	@date 	2018
*/
#pragma once

#include <inttypes.h>
// #include <cmsis_os.h>


typedef void (* callback_t)();

#define DEFAULT_STACK_SIZE 		configMINIMAL_STACK_SIZE
#define TASK_NAME_MAX_LENGTH 	10

typedef enum
{
	eCPU_LOAD = 0,
	eAPPL,
	ePWM_AUX,
	ePWM_MAIN,
	eCOLORLED,
	ePPM_INPUT,		// = 5
	eGPS,
	eHMC5883,
	eMPU6000,
	eMS5611,
	eSDCARD,		// = 10
	eSIGLOGGER,
	eCOMMITF,
	eMaxCount,		// = 13
	eNONE			// is used for printing to console only still no schedule is running
}eTaskID;

typedef enum
{
	ePrioIdle  			= osPriorityIdle,
	ePrioLow 			= osPriorityLow,
	ePrioNormal 		= osPriorityNormal,
	ePrioHigh			= osPriorityHigh,
}eTaskPrio;


typedef struct
{
	osThreadId 	threadID;
	uint32_t 	stackSize;
	uint32_t 	taskPrio;
	char  		name[TASK_NAME_MAX_LENGTH + 1];
	uint32_t 	sampleTime;
	callback_t 	taskFunction;
	eTaskID 	taskID;
	SemaphoreHandle_t mutex;
	QueueSetHandle_t queueSet;
	QueueHandle_t msgQueue;
}px4_task;


void px4_tasks_initialize();

void px4_tasks_run();

void px4_tasks_register_task(eTaskID id, callback_t func, uint32_t sampleTimeMS, uint32_t stacksize, uint32_t taskPrio);

QueueHandle_t getQueueHandleByEnum(eTaskID id);

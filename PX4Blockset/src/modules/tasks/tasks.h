/**
*
*   @author Max Pejs <max.pejs@googlemail.com>
*	@date 	2018
*/
#ifndef TASKS_H
#define TASKS_H

#include <inttypes.h>
#include <cmsis_os.h>

/**
*
*/
typedef void (* callback_t)(void const * argv);

typedef enum
{
	eAPPL = 0,
	eAUX_PWM,
	ePWM_MAIN,
	eCOLORLED,
	ePPM_INPUT,
	eGPS,
	eHMC5883,
	eMPU6000,
	eMS5611,
	eSDCARD,
	eSIGLOGGER,
	eCOMMITF,
	eCPU_LOAD,
	eMaxCount
}eTaskID;

typedef enum
{
	ePrioIdle  			= osPriorityIdle,
	ePrioLow 			= osPriorityLow,
	ePrioBelowNormal	= osPriorityBelowNormal,
	ePrioNormal 		= osPriorityNormal,
	ePrioAboveNormal 	= osPriorityAboveNormal,
	ePrioHigh			= osPriorityHigh,
	ePrioRealTime		= osPriorityRealtime,
	ePrioError 			= osPriorityError
}eTaskPrio;


typedef struct
{
	osThreadId 	threadID;
	uint32_t 	stackSize;
	uint32_t 	taskPrio;
	char  		name[20];
	uint32_t 	sampleTime;
	callback_t 	taskFunction;
	eTaskID 	taskID;
	SemaphoreHandle_t * mutex;
	QueueHandle_t msgQueue;
	QueueSetHandle_t queueSet;
}px4_task;

/**
*
*/
void px4_tasks_initialize();


/**
*
*/
void px4_tasks_run();

/**
*
*/
void px4_tasks_register_task(eTaskID id, const char * name, callback_t func, uint32_t sampleTime, uint32_t stacksize, uint32_t taskPrio);

#endif // TASKS_H

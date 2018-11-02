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
typedef void (* callback_t)();

#define DEFAULT_STACK_SIZE configMINIMAL_STACK_SIZE

typedef enum
{
	eNONE = 0,	// is used for printing to console only still no schedule is running
	eDRV,		// enum defined only for logging low level driver messages (spi, i2c, pxio)
	eAPPL,
	ePWM_AUX,
	ePWM_MAIN,
	eCOLORLED,		// = 5
	ePPM_INPUT,
	eGPS,
	eHMC5883,
	eMPU6000,
	eMS5611,		// = 10
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
	char  		name[10];
	uint32_t 	sampleTime;
	callback_t 	taskFunction;
	eTaskID 	taskID;
	SemaphoreHandle_t mutex;
	QueueSetHandle_t queueSet;
	QueueHandle_t msgQueue;
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

QueueHandle_t getQueueHandleByEnum(eTaskID id);

QueueSetHandle_t px4_tasks_get_queueset(void);




#endif // TASKS_H

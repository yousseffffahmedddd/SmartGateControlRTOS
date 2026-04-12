#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "basic_io.h"
#include "TM4C123GH6PM.h"

#define QUEUE_LENGTH    10
#define QUEUE_ITEM_SIZE sizeof(int)

extern void PortF_Init(void);
extern void PortB_Init(void);
extern void vInputTask(void *pvParameters);
extern void vGateControlTask(void *pvParameters);
extern void vLEDControlTask(void *pvParameters);
extern void vSafetyTask(void *pvParameters);
extern void vOpenLimitTask(void *pvParameters);
extern void vCloseLimitTask(void *pvParameters);
	
QueueHandle_t xGateEventQueue;
SemaphoreHandle_t xGateStateMutex;
SemaphoreHandle_t xOpenLimitSem;
SemaphoreHandle_t xCloseLimitSem;
SemaphoreHandle_t xObstacleSem;

// timer handle
TimerHandle_t    xReverseTimer;

int main(void)
{
		/*
	Port F
		PF4 Security OPEN
		PF0 Security CLOSE (unlock needed)
		PF3 Green LED
		PF1 Red LED

	Port B
		PB0 Driver OPEN
		PB1 Driver CLOSE
		PB2 Open Limit
		PB3 Closed Limit
		PB4 Obstacle
		*/
		PortF_Init();
		PortB_Init();
		__asm("  CPSIE I");
	
		// mutex
		vSemaphoreCreateBinary(xGateStateMutex);
	
		// open limit semaphore
		vSemaphoreCreateBinary(xOpenLimitSem);
		xSemaphoreTake(xOpenLimitSem, 0);
		
		// close limit semaphore
		vSemaphoreCreateBinary(xCloseLimitSem);
		xSemaphoreTake(xCloseLimitSem, 0);
		
		// obstacle semaphore
		vSemaphoreCreateBinary(xObstacleSem);
		xSemaphoreTake(xObstacleSem, 0);

		// event queue
    xGateEventQueue = xQueueCreate(QUEUE_LENGTH, QUEUE_ITEM_SIZE);    
    if (xGateEventQueue == NULL)
    {
        /* Queue creation failed! */
        for (;;);
    }
		
		// priority from 1 to 5
    xTaskCreate(vInputTask, "Input Task", 240, NULL, 3, NULL);
		xTaskCreate(vGateControlTask, "Gate Control Task", 240, NULL, 2, NULL);
    xTaskCreate(vLEDControlTask, "LED Contorl Task", 240, NULL, 2, NULL);
		
		// semaphore-based tasks
    xTaskCreate(vSafetyTask, "Safety Task", 240, NULL, 5, NULL);
		xTaskCreate(vOpenLimitTask, "Open Limit Task", 240, NULL, 4, NULL);
    xTaskCreate(vCloseLimitTask, "Close Limit Task", 240, NULL, 4, NULL);

		// optional status task
		// xTaskCreate(vStatusTask, "Task", 240, NULL, 1, NULL);
    

		// initialize Timer Api
	/* 5. Create reverse timer — ONE-SHOT, 500ms */
    xReverseTimer = xTimerCreate(
        "RevTmr",               /* name, debug only          */
        pdMS_TO_TICKS(500),     /* period                    */
        pdFALSE,                /* one-shot (not auto-reload)*/
        NULL,                   /* timer ID, not needed      */
        vReverseTimerCb         /* callback function         */
    );


    vTaskStartScheduler();
    
    for (;;);
}
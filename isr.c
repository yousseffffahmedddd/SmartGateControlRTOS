#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* Bit-field encoding sent via task notification value.
   The Input Task reads the raw GPIO level on wake-up,
   so we only need to signal WHICH pin group fired. */
#define NOTIFY_DRV_OPEN   (1UL << 0)
#define NOTIFY_DRV_CLOSE  (1UL << 1)
#define NOTIFY_SEC_OPEN   (1UL << 2)
#define NOTIFY_SEC_CLOSE  (1UL << 3)

extern TaskHandle_t xInputTaskHandle;
extern SemaphoreHandle_t xOpenLimitSem;
extern SemaphoreHandle_t xCloseLimitSem;
extern SemaphoreHandle_t xObstacleSem;

static inline void GPIOIntClear(GPIOA_Type *port, uint8_t pins);
/*
USAGE EXAMPLE:

GPIOIntClear(GPIOF, (1 << 4));  // clear interrupt on PF4
*/


/* -- SECURITY OPEN and SECURITY CLOSE --------------------------------------------------- */
void GPIOF_Handler(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

		if(GPIOF->MIS & (1 << 4)){ // if interrupt caused by PF4 (SECURITY OPEN)
			GPIOIntClear(GPIOF, (1 << 4)); // clear interrupt on PF4
			xTaskNotifyFromISR(xInputTaskHandle,
												 NOTIFY_SEC_OPEN,
												 eSetBits,
												 &xHigherPriorityTaskWoken);
		}
	
		if(GPIOF->MIS & (1 << 0)){ // if interrupt caused by PF0 (SECURITY CLOSE)
			GPIOIntClear(GPIOF, (1 << 0)); // clear interrupt on PF0
			xTaskNotifyFromISR(xInputTaskHandle,
												 NOTIFY_SEC_CLOSE,
												 eSetBits,
												 &xHigherPriorityTaskWoken);
		}

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/*
// -- Driver CLOSE -------------------------------------------------- 
void GPIO_DriverClose_ISR(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    GPIOIntClear(DRV_CLOSE_PORT, DRV_CLOSE_PIN);

    xTaskNotifyFromISR(xInputTaskHandle,
                       NOTIFY_DRV_CLOSE,
                       eSetBits,
                       &xHigherPriorityTaskWoken);

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

// -- Security OPEN -------------------------------------------------
void GPIO_SecurityOpen_ISR(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    GPIOIntClear(SEC_OPEN_PORT, SEC_OPEN_PIN);

    xTaskNotifyFromISR(xInputTaskHandle,
                       NOTIFY_SEC_OPEN,
                       eSetBits,
                       &xHigherPriorityTaskWoken);

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

// -- Security CLOSE ------------------------------------------------
void GPIO_SecurityClose_ISR(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    GPIOIntClear(SEC_CLOSE_PORT, SEC_CLOSE_PIN);

    xTaskNotifyFromISR(xInputTaskHandle,
                       NOTIFY_SEC_CLOSE,
                       eSetBits,
                       &xHigherPriorityTaskWoken);

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

// -- OPEN Limit ------------------------------------------------
void GPIO_OpenLimit_ISR(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    GPIOIntClear(SEC_CLOSE_PORT, SEC_CLOSE_PIN);

    xTaskNotifyFromISR(xInputTaskHandle,
                       NOTIFY_SEC_CLOSE,
                       eSetBits,
                       &xHigherPriorityTaskWoken);

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

// -- CLOSE Limit ------------------------------------------------
void GPIO_CloseLimit_ISR(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    GPIOIntClear(SEC_CLOSE_PORT, SEC_CLOSE_PIN);

    xTaskNotifyFromISR(xInputTaskHandle,
                       NOTIFY_SEC_CLOSE,
                       eSetBits,
                       &xHigherPriorityTaskWoken);

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

// -- OBSTACLE ------------------------------------------------
void GPIO_Obstacle_ISR(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    GPIOIntClear(SEC_CLOSE_PORT, SEC_CLOSE_PIN);

    xTaskNotifyFromISR(xInputTaskHandle,
                       NOTIFY_SEC_CLOSE,
                       eSetBits,
                       &xHigherPriorityTaskWoken);

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
*/
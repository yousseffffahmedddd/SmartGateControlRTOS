#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "gpio_utility.h"
#include "rtos_resources.h"

/* Bit-field encoding sent via task notification value.
   The Input Task reads the raw GPIO level on wake-up,
   so we only need to signal WHICH pin group fired. */
#define NOTIFY_DRV_OPEN   (1UL << 0)
#define NOTIFY_DRV_CLOSE  (1UL << 1)
#define NOTIFY_SEC_OPEN   (1UL << 2)
#define NOTIFY_SEC_CLOSE  (1UL << 3)

/* ISR resources are defined in main.c */


// -- SECURITY OPEN and SECURITY CLOSE ---------------------------------------------------
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


// -- DRIVER OPEN and DRIVER CLOSE, OPEN LIMIT AND CLOSE LIMIT, OBSTACLE -------------------
void GPIOB_Handler(void)
{
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;

		if(GPIOB->MIS & (1 << 0)){ // if interrupt caused by PB0 (DRIVER OPEN)
			GPIOIntClear(GPIOB, (1 << 0)); // clear interrupt on PB0
			xTaskNotifyFromISR(xInputTaskHandle,
												 NOTIFY_DRV_OPEN,
												 eSetBits,
												 &xHigherPriorityTaskWoken);
		}
	
		if(GPIOB->MIS & (1 << 1)){ // if interrupt caused by PB1 (DRIVER CLOSE)
			GPIOIntClear(GPIOB, (1 << 1)); // clear interrupt on PB1
			xTaskNotifyFromISR(xInputTaskHandle,
												 NOTIFY_DRV_CLOSE,
												 eSetBits,
												 &xHigherPriorityTaskWoken);
		}
		
		if(GPIOB->MIS & (1 << 2)){ // if interrupt caused by PB2 (OPEN LIMIT)
			GPIOIntClear(GPIOB, (1 << 2)); // clear interrupt on PB2
			xSemaphoreGiveFromISR(xOpenLimitSem, &xHigherPriorityTaskWoken);
		}
	
		if(GPIOB->MIS & (1 << 3)){ // if interrupt caused by PB3 (CLOSE LIMIT)
			GPIOIntClear(GPIOB, (1 << 3)); // clear interrupt on PB3
			xSemaphoreGiveFromISR(xCloseLimitSem, &xHigherPriorityTaskWoken);
		}
		
		if(GPIOB->MIS & (1 << 4)){ // if interrupt caused by PB4 (OBSTACLE)
			GPIOIntClear(GPIOB, (1 << 4)); // clear interrupt on PB4
			xSemaphoreGiveFromISR(xObstacleSem, &xHigherPriorityTaskWoken);
		}

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
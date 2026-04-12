#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "basic_io.h"

#define QUEUE_LENGTH    5
#define QUEUE_ITEM_SIZE sizeof(int)
	
QueueHandle_t xQueue;
void TestTask(void *pvParameters);
SemaphoreHandle_t semaphore;
	
void PortF_Init(void)
{
    /* Enable clock for Port F */
		SYSCTL->RCGCGPIO = 1 << 5;
    volatile int delay = 1000;  // Simple delay for clock to stabilize
    while(delay--) {}

		GPIOF->DIR |= (1 << 1);
		GPIOF->DIR &= ~(1 << 4);
    GPIOF->DEN |= ((1 << 1) | (1 << 4));
		GPIOF->PUR |= (1 << 4);
			
    GPIOF->IS &= ~(1 << 4);     /* Edge-sensitive */
    GPIOF->IBE &= ~(1 << 4);    /* Single edge */
    GPIOF->IEV &= ~(1 << 4);    /* Falling edge (button press) */
    GPIOF->ICR |= (1 << 4);     /* Clear any prior interrupt */
    GPIOF->IM |= (1 << 4);      /* Unmask interrupt */
			
		NVIC_EnableIRQ(GPIOF_IRQn);
		// we have to lower priority of the ISR to make it compatible with FreeRTOS
		NVIC_SetPriority(GPIOF_IRQn, 6);
		__asm("  CPSIE I");
}

void GPIOIntClear(uint32_t port, uint8_t pins)
{
    if (port == 'A')
    {
				GPIOA->ICR = pins;
    }
    else if (port == 'B')
    {
        GPIOB->ICR = pins;
    }
    else if (port == 'C')
    {
        GPIOC->ICR = pins;
    }
    else if (port == 'D')
    {
        GPIOD->ICR = pins;
    }
    else if (port == 'E')
    {
        GPIOE->ICR = pins;
    }
    else if (port == 'F')
    {
        GPIOF->ICR = pins;
    }
}

void GPIOF_Handler(void){
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    GPIOIntClear('F', 1 << 4);

    // xTaskNotifyFromISR(xInputTaskHandle, NOTIFY_DRV_CLOSE, eSetBits, &xHigherPriorityTaskWoken);
		
		xSemaphoreGiveFromISR(semaphore, &xHigherPriorityTaskWoken);

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void TestTask(void *pvParameters){
	for(;;){
		xSemaphoreTake(semaphore, portMAX_DELAY);
		vPrintString("KILL ALL NIGGERS\n");
	}
}

int main(void)
{
		PortF_Init();
		vSemaphoreCreateBinary(semaphore);
		xSemaphoreTake(semaphore, 0); // make it EMPTY initially

    xQueue = xQueueCreate(QUEUE_LENGTH, QUEUE_ITEM_SIZE);
    
    if (xQueue == NULL)
    {
        /* Queue creation failed! */
        vPrintString("Could not create queue!\n");
        for (;;);
    }
    
    /* Create sender task */
    xTaskCreate(TestTask, "Task", 240, NULL, 1, NULL);
    
    vTaskStartScheduler();
    
    for (;;);
}
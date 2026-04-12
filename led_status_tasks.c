#include "FreeRTOS.h"
#include "led_status_tasks.h"
#include "gpio_utility.h"
#include "rtos_resources.h"

volatile LEDState_t gDesiredLED = LED_OFF;

void setLED(LEDState_t state)
{
    gDesiredLED = state;       /* written under caller's mutex - already safe */
    xSemaphoreGive(xLEDSem);  /* wake LED Task */
}

void vLEDControlTask(void *pvParameters)
{
    for(;;)
    {
        xSemaphoreTake(xLEDSem, portMAX_DELAY);

        /* gDesiredLED is stable - no mutex needed.
           Caller wrote it under the mutex before giving xLEDSem,
           and no other writer can touch it until they take the mutex again,
           which they can't until we're done since we don't hold it.        */
        switch(gDesiredLED)
        {
            case LED_GREEN:
                GPIOPinWrite(GPIOF_BASE, 1 << 3, 1);
                GPIOPinWrite(GPIOF_BASE, 1 << 1, 0);
                break;
            case LED_RED:
                GPIOPinWrite(GPIOF_BASE, 1 << 1, 1);
                GPIOPinWrite(GPIOF_BASE, 1 << 3, 0);
                break;
            case LED_OFF:
            default:
                GPIOPinWrite(GPIOF_BASE, 1 << 3, 0);
                GPIOPinWrite(GPIOF_BASE, 1 << 1, 0);
                break;
        }
    }
}
#include "FreeRTOS.h"
#include "semphr.h"
#include "timers.h"
#include "gate_control_task.h"
#include "led_status_tasks.h"
#include "rtos_resources.h"

// check for timing as gate mutex may made the immediate response to be delayed


// from main.c
extern SemaphoreHandle_t xGateStateMutex;
extern SemaphoreHandle_t xOpenLimitSem;
extern SemaphoreHandle_t xCloseLimitSem;
extern SemaphoreHandle_t xObstacleSem;
extern GateCtx_t gGate;
extern TimerHandle_t xReverseTimer;

// by ahmed and yousef
void vReverseTimerCb(TimerHandle_t xTimer)
{
    xSemaphoreTake(xGateStateMutex, portMAX_DELAY);
    if (gGate.state == GATE_REVERSING) {        /* safety check */
        gGate.state    = GATE_STOPPED_MIDWAY;
        gGate.autoMode = 0;
    }
    xSemaphoreGive(xGateStateMutex);
    setLED(LED_OFF);
}
// by ahmed and yousef
void vSafetyTask(void *pvParameters)
{
    for (;;)
    {
        xSemaphoreTake(xObstacleSem, portMAX_DELAY);

        xSemaphoreTake(xGateStateMutex, portMAX_DELAY);

        if (gGate.state == GATE_CLOSING)
        {
            // ignore queue events
            gGate.state    = GATE_REVERSING;
            gGate.autoMode = 0;
            xSemaphoreGive(xGateStateMutex);    /* release BEFORE side effects */
            setLED(LED_GREEN);
            xTimerStart(xReverseTimer,portMAX_DELAY);      /* fires vReverseTimerCb after 500ms */
        
            // accept queue events
            /*
            gGate.state    = GATE_REVERSING;
            gGate.autoMode = 0;
            setLED(LED_GREEN);
            vTaskDelay(pdMS_TO_TICKS(10000));
            if (gGate.state == GATE_REVERSING) {        // safety check
                gGate.state    = GATE_STOPPED_MIDWAY;
                gGate.autoMode = 0;
            }
            setLED(LED_OFF);
            xSemaphoreGive(xGateStateMutex);    // release BEFORE side effects
            */
        }
        else
        {
            xSemaphoreGive(xGateStateMutex);    /* TC-09: ignore, release immediately */
        }
    }
}

void vOpenLimitTask(void *pvParameters)
{
    for (;;)
    {
        xSemaphoreTake(xOpenLimitSem, portMAX_DELAY);

        xSemaphoreTake(xGateStateMutex, portMAX_DELAY);

        if (gGate.state == GATE_OPENING)
        {
            gGate.state    = GATE_IDLE_OPEN;
            gGate.autoMode = 0;
            xSemaphoreGive(xGateStateMutex);   /* release BEFORE side effects */

            setLED(LED_OFF);
        }
        else
        {
            xSemaphoreGive(xGateStateMutex);   /* TC-12: wrong limit, ignore */
        }
    }
}

void vCloseLimitTask(void *pvParameters)
{
    for (;;)
    {
        xSemaphoreTake(xCloseLimitSem, portMAX_DELAY);

        xSemaphoreTake(xGateStateMutex, portMAX_DELAY);

        if (gGate.state == GATE_CLOSING)
        {
            gGate.state    = GATE_IDLE_CLOSED;
            gGate.autoMode = 0;
            xSemaphoreGive(xGateStateMutex);   /* release BEFORE side effects */

            setLED(LED_OFF);
        }
        else
        {
            xSemaphoreGive(xGateStateMutex);   /* TC-12: wrong limit, ignore */
        }
    }
}

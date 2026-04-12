#include "FreeRTOS.h"
#include "gate_control_task.h"   /* GateCtx_t, GateState_t, setLED() */
#include "semphr.h"

// check for timing as gate mutex may made the immediate response to be delayed


// from main.c
extern SemaphoreHandle_t xGateStateMutex;
extern SemaphoreHandle_t xOpenLimitSem;
extern SemaphoreHandle_t xCloseLimitSem;
extern SemaphoreHandle_t xObstacleSem;
extern GateCtx_t gGateCtx;

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
            gGate.state    = GATE_REVERSING;
            gGate.autoMode = 0;
            xSemaphoreGive(xGateStateMutex);    /* release BEFORE side effects */

            setLED(LED_GREEN);
            xTimerStart(xReverseTimer, 0);      /* fires vReverseTimerCb after 500ms */
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
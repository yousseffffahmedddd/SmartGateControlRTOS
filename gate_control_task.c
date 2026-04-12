/* gate_control_task.c */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"
#include "gate_events.h"
#include "gate_control_task.h"
#include "led_status_tasks.h"
#include "rtos_resources.h"

GateCtx_t gGateCtx = { .state = GATE_IDLE_CLOSED, .autoMode = 0 };

/* -- FSM transition logic ------------------------------------------- */
static void handleEvent(GateEvent_t *pEvt)
{
    /* Acquire mutex before reading/writing shared gate context */
    xSemaphoreTake(xGateStateMutex, portMAX_DELAY);

    GateState_t currentState = gGateCtx.state;
    GateCommand_t cmd        = pEvt->cmd;
    PressType_t   press      = pEvt->pressType;
    CommandSource_t src      = pEvt->src;

    switch (currentState) {

    /* -- IDLE_CLOSED ----------------------------------------------- */
    case GATE_IDLE_CLOSED:
        if (cmd == CMD_OPEN) {
            gGateCtx.autoMode = (press == PRESS_TAP) ? 1u : 0u;
            gGateCtx.state    = GATE_OPENING;
						setLED(LED_GREEN);
        }
        /* CMD_CLOSE or CMD_STOP ? ignore, already closed */
        break;

    /* -- IDLE_OPEN ------------------------------------------------- */
    case GATE_IDLE_OPEN:
        if (cmd == CMD_CLOSE) {
            gGateCtx.autoMode = (press == PRESS_TAP) ? 1u : 0u;
            gGateCtx.state    = GATE_CLOSING;
						setLED(LED_RED);
        }
        /* CMD_OPEN or CMD_STOP ? ignore, already open */
        break;

    /* -- OPENING --------------------------------------------------- */
    case GATE_OPENING:
        if (cmd == CMD_STOP) {
            /* Conflicting buttons on same panel, or timer synthetic */
            gGateCtx.state    = GATE_STOPPED_MIDWAY;
            gGateCtx.autoMode = 0;
						setLED(LED_OFF);
        }
        else if (cmd == CMD_CLOSE) {
            if (gGateCtx.autoMode) {
                /* Auto mode: close command stops, does not reverse */
                gGateCtx.state    = GATE_STOPPED_MIDWAY;
                gGateCtx.autoMode = 0;
								setLED(LED_OFF);
            } else {
                /* Manual mode: reverse direction */
                gGateCtx.state = GATE_CLOSING;
								setLED(LED_RED);
            }
        }
        else if (cmd == CMD_OPEN && press == PRESS_RELEASE) {
            /* Manual button released mid-open */
            if (!gGateCtx.autoMode) {
                gGateCtx.state = GATE_STOPPED_MIDWAY;
								setLED(LED_OFF);
            }
            /* Auto mode: PRESS_RELEASE is ignored - motor keeps running */
        }
        /* CMD_OPEN PRESS_TAP/HOLD while already opening? ignore */
        break;

    /* -- CLOSING --------------------------------------------------- */
    case GATE_CLOSING:
        if (cmd == CMD_STOP) {
            gGateCtx.state    = GATE_STOPPED_MIDWAY;
            gGateCtx.autoMode = 0;
						setLED(LED_OFF);
        }
        else if (cmd == CMD_OPEN) {
            if (gGateCtx.autoMode) {
                gGateCtx.state    = GATE_STOPPED_MIDWAY;
                gGateCtx.autoMode = 0;
								setLED(LED_OFF);
            } else {
                gGateCtx.state = GATE_OPENING;
								setLED(LED_GREEN);
            }
        }
        else if (cmd == CMD_CLOSE && press == PRESS_RELEASE) {
            if (!gGateCtx.autoMode) {
                gGateCtx.state = GATE_STOPPED_MIDWAY;
								setLED(LED_OFF);
            }
        }
        /* CMD_CLOSE PRESS_TAP/HOLD while closing? ignore */
        break;

    /* -- STOPPED_MIDWAY -------------------------------------------- */
    case GATE_STOPPED_MIDWAY:
        if (cmd == CMD_OPEN && press != PRESS_RELEASE) {
            gGateCtx.autoMode = (press == PRESS_TAP) ? 1u : 0u;
            gGateCtx.state    = GATE_OPENING;
						setLED(LED_GREEN);
        }
        else if (cmd == CMD_CLOSE && press != PRESS_RELEASE) {
            gGateCtx.autoMode = (press == PRESS_TAP) ? 1u : 0u;
            gGateCtx.state    = GATE_CLOSING;
						setLED(LED_RED);
        }
        /* CMD_STOP or PRESS_RELEASE? stay stopped */
        break;

    default:
        break;
    }

    xSemaphoreGive(xGateStateMutex);
}

/* -- Gate Control Task entry point --------------------------------- */
void vGateControlTask(void *pvParameters)
{
    GateEvent_t evt;

    for (;;) {
        if (xQueueReceive(xGateEventQueue, &evt, portMAX_DELAY) == pdTRUE) {
            handleEvent(&evt);
        }
    }
}
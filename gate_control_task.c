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
#include "basic_io.h"

GateCtx_t gGate = { .state = GATE_IDLE_CLOSED, .autoMode = 0 };

 
    /* -- FSM transition logic ------------------------------------------- */

    // by ahmed and yousef: led after mutex release
    //add close on idle close and open on idle open
   static void handleEvent(GateEvent_t *pEvt)
{
    xSemaphoreTake(xGateStateMutex, portMAX_DELAY);

    GateState_t currentState = gGate.state;
    GateCommand_t cmd        = pEvt->cmd;
    PressType_t   press      = pEvt->pressType;
    (void)pEvt->src;

    switch (currentState) {

    case GATE_IDLE_CLOSED:
        if (cmd == CMD_OPEN) {
            gGate.autoMode = (press == PRESS_TAP) ? 1u : 0u;
            gGate.state    = GATE_OPENING;
        }
        else if (cmd==CMD_CLOSE){
                gGate.autoMode = (press == PRESS_TAP) ? 1u : 0u;
                gGate.state    = GATE_CLOSING;
        }
        break;

    case GATE_IDLE_OPEN:
        if (cmd == CMD_CLOSE) {
            gGate.autoMode = (press == PRESS_TAP) ? 1u : 0u;
            gGate.state    = GATE_CLOSING;
        }
        else if (cmd == CMD_OPEN){
                gGate.autoMode = (press == PRESS_TAP) ? 1u : 0u;
                gGate.state    = GATE_OPENING;
        }
        break;

    case GATE_OPENING:
        if (cmd == CMD_STOP) {
            gGate.state    = GATE_STOPPED_MIDWAY;
            gGate.autoMode = 0;
        }
        // else if (cmd == CMD_CLOSE) {
        //     if (gGate.autoMode) {
        //         gGate.state    = GATE_STOPPED_MIDWAY;
        //         gGate.autoMode = 0;
        //     } else {
        //         gGate.state = GATE_CLOSING;
        //     }
        // }
        else if (cmd == CMD_OPEN && press == PRESS_RELEASE) {
            if (!gGate.autoMode) {
                gGate.state = GATE_STOPPED_MIDWAY;
            }
        }
        break;

    case GATE_CLOSING:
        if (cmd == CMD_STOP) {
            gGate.state    = GATE_STOPPED_MIDWAY;
            gGate.autoMode = 0;
        }
        // else if (cmd == CMD_OPEN) {
        //     if (gGate.autoMode) {
        //         gGate.state    = GATE_STOPPED_MIDWAY;
        //         gGate.autoMode = 0;
        //     } else {
        //         gGate.state = GATE_OPENING;
        //     }
        // }
        else if (cmd == CMD_CLOSE && press == PRESS_RELEASE) {
            if (!gGate.autoMode) {
                gGate.state = GATE_STOPPED_MIDWAY;
            }
        }
        break;

    case GATE_STOPPED_MIDWAY:
        if (cmd == CMD_OPEN && press != PRESS_RELEASE) {
            gGate.autoMode = (press == PRESS_TAP) ? 1u : 0u;
            gGate.state    = GATE_OPENING;
        }
        else if (cmd == CMD_CLOSE && press != PRESS_RELEASE) {
            gGate.autoMode = (press == PRESS_TAP) ? 1u : 0u;
            gGate.state    = GATE_CLOSING;
        }
        break;

    default:
        break;
    }

    GateState_t newState = gGate.state;      /* snapshot before release */
    xSemaphoreGive(xGateStateMutex);         /* release mutex first     */

    /* LED update outside mutex — no priority inversion risk */
    if      (newState == GATE_OPENING)  setLED(LED_GREEN);
    else if (newState == GATE_CLOSING)  setLED(LED_RED);
    else if (newState==GATE_REVERSING) ;
    else  setLED(LED_OFF);
}


    /* -- Gate Control Task entry point --------------------------------- */
    void vGateControlTask(void *pvParameters)
    {
        GateEvent_t evt;

        for (;;) {
            if (xQueueReceive(xGateEventQueue, &evt, portMAX_DELAY) == pdTRUE) {
                vPrintString("Queue has received a task!\n");
                handleEvent(&evt);
            }
        }
    }
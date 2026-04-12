/* gate_control_task.c */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"
#include "gate_events.h"

/* -- Shared gate state ---------------------------------------------- */
typedef enum {
    GATE_IDLE_CLOSED = 0,
    GATE_IDLE_OPEN,
    GATE_OPENING,
    GATE_CLOSING,
    GATE_STOPPED_MIDWAY,
    GATE_REVERSING
} GateState_t;

typedef struct {
    GateState_t state;
    uint8_t     autoMode;   /* 1 = one-touch auto, 0 = manual hold */
} GateCtx_t;

static GateCtx_t gGate = { .state = GATE_IDLE_CLOSED, .autoMode = 0 };

extern QueueHandle_t   xGateEventQueue;
extern SemaphoreHandle_t xGateStateMutex;   /* created in main.c */

/* -- FSM transition logic ------------------------------------------- */
static void handleEvent(GateEvent_t *pEvt)
{
    /* Acquire mutex before reading/writing shared gate context */
    xSemaphoreTake(xGateStateMutex, portMAX_DELAY);

    GateState_t currentState = gGate.state;
    GateCommand_t cmd        = pEvt->cmd;
    PressType_t   press      = pEvt->pressType;
    CommandSource_t src      = pEvt->src;

    switch (currentState) {

    /* -- IDLE_CLOSED ----------------------------------------------- */
    case GATE_IDLE_CLOSED:
        if (cmd == CMD_OPEN) {
            gGate.autoMode = (press == PRESS_TAP) ? 1u : 0u;
            gGate.state    = GATE_OPENING;
            ledGreenOn();
        }
        /* CMD_CLOSE or CMD_STOP ? ignore, already closed */
        break;

    /* -- IDLE_OPEN ------------------------------------------------- */
    case GATE_IDLE_OPEN:
        if (cmd == CMD_CLOSE) {
            gGate.autoMode = (press == PRESS_TAP) ? 1u : 0u;
            gGate.state    = GATE_CLOSING;
            ledRedOn();
        }
        /* CMD_OPEN or CMD_STOP ? ignore, already open */
        break;

    /* -- OPENING --------------------------------------------------- */
    case GATE_OPENING:
        if (cmd == CMD_STOP) {
            /* Conflicting buttons on same panel, or timer synthetic */
            gGate.state    = GATE_STOPPED_MIDWAY;
            gGate.autoMode = 0;
            ledAllOff();
        }
        else if (cmd == CMD_CLOSE) {
            if (gGate.autoMode) {
                /* Auto mode: close command stops, does not reverse */
                gGate.state    = GATE_STOPPED_MIDWAY;
                gGate.autoMode = 0;
                ledAllOff();
            } else {
                /* Manual mode: reverse direction */
                gGate.state = GATE_CLOSING;
                ledRedOn();
            }
        }
        else if (cmd == CMD_OPEN && press == PRESS_RELEASE) {
            /* Manual button released mid-open */
            if (!gGate.autoMode) {
                gGate.state = GATE_STOPPED_MIDWAY;
                ledAllOff();
            }
            /* Auto mode: PRESS_RELEASE is ignored — motor keeps running */
        }
        /* CMD_OPEN PRESS_TAP/HOLD while already opening ? ignore */
        break;

    /* -- CLOSING --------------------------------------------------- */
    case GATE_CLOSING:
        if (cmd == CMD_STOP) {
            gGate.state    = GATE_STOPPED_MIDWAY;
            gGate.autoMode = 0;
            ledAllOff();
        }
        else if (cmd == CMD_OPEN) {
            if (gGate.autoMode) {
                gGate.state    = GATE_STOPPED_MIDWAY;
                gGate.autoMode = 0;
                ledAllOff();
            } else {
                gGate.state = GATE_OPENING;
                ledGreenOn();
            }
        }
        else if (cmd == CMD_CLOSE && press == PRESS_RELEASE) {
            if (!gGate.autoMode) {
                gGate.state = GATE_STOPPED_MIDWAY;
                ledAllOff();
            }
        }
        /* CMD_CLOSE PRESS_TAP/HOLD while closing ? ignore */
        break;

    /* -- STOPPED_MIDWAY -------------------------------------------- */
    case GATE_STOPPED_MIDWAY:
        if (cmd == CMD_OPEN && press != PRESS_RELEASE) {
            gGate.autoMode = (press == PRESS_TAP) ? 1u : 0u;
            gGate.state    = GATE_OPENING;
            ledGreenOn();
        }
        else if (cmd == CMD_CLOSE && press != PRESS_RELEASE) {
            gGate.autoMode = (press == PRESS_TAP) ? 1u : 0u;
            gGate.state    = GATE_CLOSING;
            ledRedOn();
        }
        /* CMD_STOP or PRESS_RELEASE ? stay stopped */
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
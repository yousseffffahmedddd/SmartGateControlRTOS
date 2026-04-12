#ifndef GATE_CONTROL_TASK_H
#define GATE_CONTROL_TASK_H

#include <stdint.h>

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

#endif
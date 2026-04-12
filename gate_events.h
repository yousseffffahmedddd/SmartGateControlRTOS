/* gate_events.h - shared between Input Task and Gate Control Task */
#ifndef GATE_EVENTS_H
#define GATE_EVENTS_H

#include <stdint.h>

/* Source of a command */
typedef enum {
    SRC_DRIVER   = 0,
    SRC_SECURITY = 1    /* higher priority */
} CommandSource_t;

/* Direction / action requested */
typedef enum {
    CMD_OPEN  = 0,
    CMD_CLOSE = 1,
    CMD_STOP  = 2    /* conflicting buttons on same panel */
} GateCommand_t;

/* Press type - distinguishes manual hold from one-touch tap */
typedef enum {
    PRESS_HOLD    = 0,   /* button still held after debounce window */
    PRESS_TAP     = 1,   /* button released within TAP_THRESHOLD_MS */
    PRESS_RELEASE = 2    /* button was released (ends a HOLD) */
} PressType_t;

/* The message sent through the inter-task queue */
typedef struct {
    GateCommand_t   cmd;
    CommandSource_t src;
    PressType_t     pressType;
} GateEvent_t;

#endif /* GATE_EVENTS_H */
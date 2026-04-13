/* led_status_tasks.h */
#ifndef LED_STATUS_TASKS_H
#define LED_STATUS_TASKS_H

typedef enum {
    LED_OFF   = 0,
    LED_GREEN = 1,
    LED_RED   = 2
} LEDState_t;

/* Defined in led_status_tasks.c. */
extern volatile LEDState_t gDesiredLED;

/* Call this while holding xGateStateMutex */
void setLED(LEDState_t state);

/* Task entry point passed to xTaskCreate in main.c. */
void vLEDControlTask(void *pvParameters);

#endif /* LED_STATUS_TASKS_H */
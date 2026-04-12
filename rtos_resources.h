#ifndef RTOS_RESOURCES_H
#define RTOS_RESOURCES_H

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

extern QueueHandle_t xGateEventQueue;
extern SemaphoreHandle_t xGateStateMutex;
extern SemaphoreHandle_t xOpenLimitSem;
extern SemaphoreHandle_t xCloseLimitSem;
extern SemaphoreHandle_t xObstacleSem;
extern SemaphoreHandle_t xLEDSem;
extern TaskHandle_t xInputTaskHandle;

#endif /* RTOS_RESOURCES_H */

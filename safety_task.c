#include "FreeRTOS.h"
#include "gate_control_task.h"   /* GateCtx_t, GateState_t, setLED() */
#include "semphr.h"

// from main.c
extern SemaphoreHandle_t xGateStateMutex;
extern SemaphoreHandle_t xOpenLimitSem;
extern SemaphoreHandle_t xCloseLimitSem;
extern SemaphoreHandle_t xObstacleSem;
extern GateCtx_t gGateCtx;

void vSafetyTask(void *pvParameters){
	xSemaphoreTake(xObstacleSem, portMAX_DELAY);
	
	/* Acquire mutex before inspecting/modifying gate state */
	xSemaphoreTake(xGateStateMutex, portMAX_DELAY);

	if (gGateCtx.state == GATE_CLOSING)
	{
			/* Enforce TC-07:
				 1. Stop immediately
				 2. Reverse (open) for 500 ms   ? Green LED ON
				 3. Stop in STOPPED_MIDWAY       ? handled by timer callback */
			gGateCtx.state    = GATE_REVERSING;
			gGateCtx.autoMode = 0;

			setLED(LED_GREEN);    /* setLED writes gDesiredLED + gives xLEDSem */

		// TODO: DELAY 500 MS
		// TODO: GO TO STOPPED_MIDWAY
	}
	/* TC-09: obstacle during OPENING or any stopped state ? ignore  */

	xSemaphoreGive(xGateStateMutex);
}
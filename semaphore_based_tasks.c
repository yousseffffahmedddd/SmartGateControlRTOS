#include "FreeRTOS.h"
#include "semphr.h"
#include "gate_control_task.h"
#include "led_status_tasks.h"
#include "rtos_resources.h"

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

void vOpenLimitTask(void *pvParameters){
	xSemaphoreTake(xOpenLimitSem, portMAX_DELAY);
	
	/* Acquire mutex before inspecting/modifying gate state */
	xSemaphoreTake(xGateStateMutex, portMAX_DELAY);

	// TODO: FSM concerned with open limit

	xSemaphoreGive(xGateStateMutex);
}

void vCloseLimitTask(void *pvParameters){
	xSemaphoreTake(xCloseLimitSem, portMAX_DELAY);
	
	/* Acquire mutex before inspecting/modifying gate state */
	xSemaphoreTake(xGateStateMutex, portMAX_DELAY);

	// TODO: FSM concerned with close limit

	xSemaphoreGive(xGateStateMutex);
}

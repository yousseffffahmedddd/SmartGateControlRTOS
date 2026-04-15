/* input_task.c */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "gate_events.h"
#include "gpio_utility.h"
#include "rtos_resources.h"

/* -- Tunables ------------------------------------------------------- */
#define DEBOUNCE_MS          20u    /* settle time after first edge      */
#define TAP_THRESHOLD_MS     300u   /* hold longer ? HOLD, shorter ? TAP */
#define NOTIFY_WAIT_MS       portMAX_DELAY

/* ISR notification bits (must match gate_isr.c) */
#define NOTIFY_DRV_OPEN   (1UL << 0)
#define NOTIFY_DRV_CLOSE  (1UL << 1)
#define NOTIFY_SEC_OPEN   (1UL << 2)
#define NOTIFY_SEC_CLOSE  (1UL << 3)

/* xGateEventQueue is defined in main.c */

/* -- Internal helpers ----------------------------------------------- */

/* Read the actual GPIO level (active-low buttons ? pressed = 0) */
static inline uint8_t btnPressed(uint32_t port, uint8_t pin)
{
    return (GPIOPinRead(port, pin) == 0) ? 1u : 0u;
}

/* Block until button is released or timeout; return ticks held */
static uint32_t waitForRelease(uint32_t port, uint8_t pin, uint32_t maxMs)
{
    const TickType_t maxTicks = pdMS_TO_TICKS(maxMs);
    const TickType_t start = xTaskGetTickCount();

    while (btnPressed(port, pin)) {
        if ((xTaskGetTickCount() - start) >= maxTicks) {
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }

    return (uint32_t)(xTaskGetTickCount() - start);
}

/* Classify press and enqueue event.
   Handles the conflicting-input case (OPEN + CLOSE on same panel). */
static void processPanel(uint32_t openPort, uint8_t openPin,
                         uint32_t closePort, uint8_t closePin,
                         CommandSource_t src)
{
    uint8_t openHeld  = btnPressed(openPort,  openPin);
    uint8_t closeHeld = btnPressed(closePort, closePin);

    GateEvent_t evt = { .src = src };

    /* Conflicting input - same panel pressing both */
    if (openHeld && closeHeld) {
        evt.cmd       = CMD_STOP;
        evt.pressType = PRESS_HOLD;
        xQueueSend(xGateEventQueue, &evt, 0);
        /* Wait until both released before continuing */
        while (btnPressed(openPort, openPin) || btnPressed(closePort, closePin)) {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
        return;
    }

    /* Single button: determine which one fired */
    GateCommand_t cmd;
    uint32_t port;
    uint8_t  pin;

    if (openHeld) {
        cmd  = CMD_OPEN;
        port = openPort;
        pin  = openPin;
    } else if (closeHeld) {
        cmd  = CMD_CLOSE;
        port = closePort;
        pin  = closePin;
    } else {
        return;   /* spurious wake - button already released */
    }

    /* Wait for release to classify tap vs hold */
    uint32_t heldTicks = waitForRelease(port, pin, TAP_THRESHOLD_MS + 100u);

    if (heldTicks >= pdMS_TO_TICKS(TAP_THRESHOLD_MS)) {
        /* -- HOLD: send PRESS_HOLD immediately, then PRESS_RELEASE on release */
        evt.cmd       = cmd;
        evt.pressType = PRESS_HOLD;
        xQueueSend(xGateEventQueue, &evt, 0);

        /* Wait until the button is fully released */
        while (btnPressed(port, pin)) {
            vTaskDelay(pdMS_TO_TICKS(10));
        }

        evt.pressType = PRESS_RELEASE;
        xQueueSend(xGateEventQueue, &evt, 0);
    } else {
        /* -- TAP: button released quickly - one-touch auto mode */
        evt.cmd       = cmd;
        evt.pressType = PRESS_TAP;
        xQueueSend(xGateEventQueue, &evt, 0);
    }
}

/* -- Input Task ----------------------------------------------------- */
void vInputTask(void *pvParameters)
{
    uint32_t ulNotifiedValue;

    for (;;) {
        /* Block until any ISR fires */
        xTaskNotifyWait(0x00,               /* don't clear on entry  */
                        0xFFFFFFFF,         /* clear all on exit     */
                        &ulNotifiedValue,
                        portMAX_DELAY);

        /* Debounce - let the lines settle */
        vTaskDelay(pdMS_TO_TICKS(DEBOUNCE_MS));

        /* -- Priority resolution --------------------------------------
           If security panel has fired we process it and ignore driver.
           If only driver fired, process driver.
           Both fired simultaneously? security wins (spec 4).          */

        uint8_t secActive = (ulNotifiedValue & (NOTIFY_SEC_OPEN | NOTIFY_SEC_CLOSE)) != 0;
        uint8_t drvActive = (ulNotifiedValue & (NOTIFY_DRV_OPEN | NOTIFY_DRV_CLOSE)) != 0;

        if (secActive) {
            /* Security panel takes priority - process it */
						/* How to use:
            processPanel(SEC_OPEN_PORT,  SEC_OPEN_PIN,
                         SEC_CLOSE_PORT, SEC_CLOSE_PIN,
                         SRC_SECURITY); */
						processPanel(GPIOF_BASE,  1 << 4,
												 GPIOF_BASE, 1 << 0,
												 SRC_SECURITY);
        }

        if (drvActive && !secActive) {
            /* Only process driver if security is not simultaneously active */
						/* How to use:
            processPanel(DRV_OPEN_PORT,  DRV_OPEN_PIN,
                         DRV_CLOSE_PORT, DRV_CLOSE_PIN,
                         SRC_DRIVER); */
					  processPanel(GPIOB_BASE,  1 << 0,
                         GPIOB_BASE, 1 << 1,
                         SRC_DRIVER);
        }
        /* If both panels active simultaneously and security already sent
           its command, driver input is intentionally dropped per spec. */
    }
}
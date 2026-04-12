#include "stdint.h"
#include "TM4C123GH6PM.h"

static inline uint32_t GPIOPinRead(uint32_t port, uint8_t pins)
{
    return *((volatile uint32_t *)(port + (pins << 2)));
}
/*
USAGE EXAMPLE:

uint32_t val = GPIOPinRead(GPIOF_BASE, (1 << 4));

if (val)
{
    // PF4 is HIGH
}
*/

static inline void GPIOPinWrite(uint32_t port, uint8_t pins, uint8_t value)
{
    *((volatile uint32_t *)(port + (pins << 2))) = value ? pins : 0;
}
/*
USAGE EXAMPLE:

GPIOPinWrite(GPIOF_BASE, (1 << 1), 1);   // PF1 HIGH
GPIOPinWrite(GPIOF_BASE, (1 << 1), 0);   // PF1 LOW
*/

static inline void GPIOIntClear(GPIOA_Type *port, uint8_t pins)
{
    port->ICR = pins;
}
/*
USAGE EXAMPLE:

GPIOIntClear(GPIOF, (1 << 4));  // clear interrupt on PF4
*/

void PortF_Init(void) // REMOVE LOCK
{
    SYSCTL->RCGCGPIO = (1 << 5);  
    // Enable clock for GPIO Port F (bit 5 = Port F)

    volatile int delay = 1000;
    while(delay--) {}
    // Small delay to allow clock to stabilize

    GPIOF->DIR |= ((1 << 1) | (1 << 3));
    // Set PF1 and PF3 as OUTPUT (LED pins)

    GPIOF->DIR &= ~((1 << 0) | (1 << 4));
    // Set PF4 as INPUT (switch)

    GPIOF->DEN |= ((1 << 1) | (1 << 3) | (1 << 4));
    // Enable digital function on PF1 and PF4

    GPIOF->PUR |= ((1 << 0) | (1 << 4));
    // Enable pull-up resistor on PF4 (button idle HIGH)

    GPIOF->IS &= ~((1 << 0) | (1 << 4));
    // Configure PF4 as edge-sensitive interrupt

    GPIOF->IBE &= ~((1 << 0) | (1 << 4));
    // Disable both-edge trigger (use single edge only)

    GPIOF->IEV &= ~((1 << 0) | (1 << 4));
    // Interrupt on falling edge (button press)

    GPIOF->ICR |= ((1 << 0) | (1 << 4));
    // Clear any pending interrupt on PF4

    GPIOF->IM |= ((1 << 0) | (1 << 4));
    // Unmask interrupt for PF4 (enable interrupt)

    NVIC_EnableIRQ(GPIOF_IRQn);
    // Enable GPIOF interrupt in NVIC

    NVIC_SetPriority(GPIOF_IRQn, 6);
    // Set interrupt priority (lower number = higher priority)
}

void PortB_Init(void)
{
    SYSCTL->RCGCGPIO |= (1 << 1);
    // Enable clock for GPIO Port B (bit 1)

    volatile int delay = 1000;
    while(delay--) {}
    // allow clock to stabilize

    GPIOB->DIR &= ~((1 << 0) | (1 << 1) | (1 << 2) |
                    (1 << 3) | (1 << 4));
    // Set PB0–PB4 as INPUTs

    GPIOB->DEN |= ((1 << 0) | (1 << 1) | (1 << 2) |
                   (1 << 3) | (1 << 4));
    // Enable digital function on PB0–PB4

    GPIOB->PUR |= ((1 << 0) | (1 << 1) | (1 << 2) |
                   (1 << 3) | (1 << 4));
    // Enable pull-up resistors on PB0–PB4

    GPIOB->IS &= ~((1 << 0) | (1 << 1) | (1 << 2) |
                   (1 << 3) | (1 << 4));
    // Make PB0–PB4 edge-sensitive interrupts

    GPIOB->IBE |= ((1 << 0) | (1 << 1));
    // Enable both-edge interrupt on PB0 and PB1

    GPIOB->IBE &= ~((1 << 2) | (1 << 3) | (1 << 4));
    // Disable both-edge on PB2–PB4 (single-edge mode)

    GPIOB->IEV &= ~((1 << 2) | (1 << 3) | (1 << 4));
    // Set PB2–PB4 interrupt to falling edge

    GPIOB->ICR |= ((1 << 0) | (1 << 1) | (1 << 2) |
                   (1 << 3) | (1 << 4));
    // Clear any pending interrupts

    GPIOB->IM |= ((1 << 0) | (1 << 1) | (1 << 2) |
                  (1 << 3) | (1 << 4));
    // Unmask interrupts for PB0–PB4

    NVIC_EnableIRQ(GPIOB_IRQn);
    // Enable GPIO Port B interrupt in NVIC

    NVIC_SetPriority(GPIOB_IRQn, 6);
    // Set interrupt priority
}
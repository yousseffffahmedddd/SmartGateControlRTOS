#ifndef GPIO_UTILITY_H
#define GPIO_UTILITY_H

#include <stdint.h>
#include "TM4C123GH6PM.h"

static inline uint32_t GPIOPinRead(uint32_t port, uint8_t pins)
{
    return *((volatile uint32_t *)(port + (pins << 2)));
}

static inline void GPIOPinWrite(uint32_t port, uint8_t pins, uint8_t value)
{
    *((volatile uint32_t *)(port + (pins << 2))) = value ? pins : 0;
}

static inline void GPIOIntClear(GPIOA_Type *port, uint8_t pins)
{
    port->ICR = pins;
}

void PortF_Init(void);
void PortB_Init(void);

#endif /* GPIO_UTILITY_H */

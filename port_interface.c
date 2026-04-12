#include "FreeRTOS.h"

void PortB_Init(void)
{
    /* Enable clock for Port F */
		SYSCTL->RCGCGPIO = 1 << 1;
    volatile int delay = 1000;  // Simple delay for clock to stabilize
    while(delay--) {}

		GPIOF->DIR |= (1 << 1);
		GPIOF->DIR &= ~(1 << 4);
    GPIOF->DEN |= ((1 << 1) | (1 << 4));
		GPIOF->PUR |= (1 << 4);
			
    GPIOF->IS &= ~(1 << 4);     /* Edge-sensitive */
    GPIOF->IBE &= ~(1 << 4);    /* Single edge */
    GPIOF->IEV &= ~(1 << 4);    /* Falling edge (button press) */
    GPIOF->ICR |= (1 << 4);     /* Clear any prior interrupt */
    GPIOF->IM |= (1 << 4);      /* Unmask interrupt */
			
		NVIC_EnableIRQ(GPIOB_IRQn);
		// we have to lower priority of the ISR to make it compatible with FreeRTOS
		NVIC_SetPriority(GPIOB_IRQn, 6);
}

void PortF_Init(void)
{
    /* Enable clock for Port F */
		SYSCTL->RCGCGPIO = 1 << 5;
    volatile int delay = 1000;  // Simple delay for clock to stabilize
    while(delay--) {}

		GPIOF->DIR |= (1 << 1);
		GPIOF->DIR &= ~(1 << 4);
    GPIOF->DEN |= ((1 << 1) | (1 << 4));
		GPIOF->PUR |= (1 << 4);
			
    GPIOF->IS &= ~(1 << 4);     /* Edge-sensitive */
    GPIOF->IBE &= ~(1 << 4);    /* Single edge */
    GPIOF->IEV &= ~(1 << 4);    /* Falling edge (button press) */
    GPIOF->ICR |= (1 << 4);     /* Clear any prior interrupt */
    GPIOF->IM |= (1 << 4);      /* Unmask interrupt */
			
		NVIC_EnableIRQ(GPIOF_IRQn);
		// we have to lower priority of the ISR to make it compatible with FreeRTOS
		NVIC_SetPriority(GPIOF_IRQn, 6);
}
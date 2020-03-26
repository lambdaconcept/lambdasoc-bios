#include <arch/irq.h>
#include <config.h>
#include <stdint.h>
#include <uart.h>

void isr(void);
void isr(void)
{
	uint32_t irqs = irq_pending() & irq_getmask();

	if (irqs & (1 << CONFIG_UART_IRQNO))
		uart_isr();
}

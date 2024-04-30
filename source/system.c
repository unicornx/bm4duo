#include <stdio.h>

#include "uart.h"
#include "pinmux.h"
#include "irq.h"

void system_init(void)
{
	pinmux_config(PINMUX_UART1);
	uart_init(1);
	
	irq_init();
	
	printf("\n");
}


#define ICACHE  (1 << 0)
#define DCACHE  (1 << 1)

#define MCOR_IC_SEL  (1 << 0)
#define MCOR_DC_SEL  (1 << 1)
#define MCOR_INV_SEL (1 << 4)
#define MCOR_CLR_SEL (1 << 5)

void dcache_disable(void)
{
    asm volatile("csrc mhcr, %0"::"rk"(DCACHE));
}

inline void dcache_enable(void)
{
    asm volatile("csrs mhcr, %0"::"rK"(DCACHE));
}

void inv_dcache_all(void)
{
    asm volatile("csrs mcor, %0"::"rk"(MCOR_DC_SEL | MCOR_INV_SEL));
}

void clean_dcache_all(void)
{
    asm volatile("csrs mcor, %0"::"rk"(MCOR_DC_SEL | MCOR_CLR_SEL));
}

inline void inv_clean_dcache_all(void)
{
    asm volatile("csrs mcor, %0"::"rk"(MCOR_DC_SEL | MCOR_INV_SEL | MCOR_CLR_SEL));
}

#ifndef __weak
#define __weak __attribute__((weak))
#endif


__weak void invalidate_dcache_range(unsigned long start, unsigned long size) {}
__weak void flush_dcache_range(unsigned long start, unsigned long size) {}
__weak void invalidate_icache_all(void) {}
__weak void invalidate_dcache_all(void) {}






#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "irq.h"
#include "riscv.h"
#include "mmio.h"
#include "mmap.h"

struct irq_action {
	irq_handler_t handler;
	unsigned long flags;
	unsigned int irqn;
	void *priv;
	char name[32];
};

static struct irq_action g_irq_action[256];

struct irq_chip {
	const char  *name;
	void        (*irq_mask)(int irq_num);
	void        (*irq_unmask)(int irq_num);
	int         (*irq_ack)(void);
	void        (*irq_set_priority)(int irq_num, int priority);
	void        (*irq_eoi)(int irq_num);
	void        (*irq_set_threshold)(uint32_t threshold);
};

static struct irq_chip sirq_chip;

static void plic_mask_irq( int irq_num)
{
	uint32_t mask = (irq_num);
	uint32_t value = 0;
	if(irq_num < 16)
	{
		printf("mask irq_num is %d\n",irq_num);
		return;
	}
	value = mmio_read_32(PLIC_ENABLE1+4*(mask/32));
	value &= ~(0x1 << (mask%32));
	mmio_write_32((PLIC_ENABLE1+(mask/32)*4),value);

}
static void plic_unmask_irq( int irq_num)
{
	uint32_t mask = (irq_num);
	uint32_t value=0;
	if(irq_num < 16)
	{
		printf("unmask irq_num is %d\n",irq_num);
		return;
	}
	//printf("irq_num unmask=%d\n",mask);
	value = mmio_read_32(PLIC_ENABLE1+4*(mask/32));
	value |= (0x1 << (mask%32));
	//printf("value=%x\n",value);
	mmio_write_32((PLIC_ENABLE1+(mask/32)*4),value);

}

static int plic_ack_irq()
{
	return mmio_read_32(PLIC_CLAIM);
}

static void plic_eoi_irq(int irq_num)
{
	mmio_write_32(PLIC_CLAIM, irq_num);
}

static void plic_set_priority_irq(int irq_num, int priority)
{
	//printf("plic_set_priority_irq addr(%x)=%d\n",PLIC_PRIORITY0 + irq_num*4,priority);
	mmio_write_32((PLIC_PRIORITY0 + irq_num*4), priority);
}

static void plic_set_threshold(uint32_t threshold)
{
	mmio_write_32((PLIC_THRESHOLD), threshold);
}

static struct irq_chip sirq_chip = {
	.name           = "RISCV PLIC",
	.irq_mask       = plic_mask_irq,
	.irq_unmask     = plic_unmask_irq,
	.irq_ack        = plic_ack_irq,
	.irq_set_priority = plic_set_priority_irq,
	.irq_eoi        = plic_eoi_irq,
	.irq_set_threshold = plic_set_threshold,
};

void cpu_enable_irqs(void)
{
    set_csr(mstatus, MSTATUS_MIE);
    set_csr(mie, MIP_MEIE);
}

void cpu_disable_irqs(void)
{
    clear_csr(mstatus, MSTATUS_MIE);
    clear_csr(mie, MIP_MEIE);
}

void irq_init(void)
{
	int i;
	// clear interrupt enable
	write_csr(mie, 0);
	// clear interrupt pending
	write_csr(mip, 0);

	// Clean the setting of all IRQ
	for (i = 0; i < 256 * 4; i = i + 4) {
		mmio_write_32(((uintptr_t)PLIC_PRIORITY0 + i), 0);
	}
	for (i = 0; i <= 256/32; i++) {
		mmio_write_32((PLIC_PENDING1  + i*4), 0);
		mmio_write_32((PLIC_ENABLE1  + i*4), 0);
	}
	memset(g_irq_action, 0, sizeof(struct irq_action)*256);
	sirq_chip.irq_set_threshold(0);
	cpu_enable_irqs();
}

int request_irq(unsigned int irqn, irq_handler_t handler, unsigned long flags,
		const char *name, void *priv)
{
	if ((irqn < 0) || (irqn >= NUM_IRQ))
		return -1;
	//printf("request_irq irqn=%d\n handler=%lx  name = %s\n",irqn,(long) handler,name);
	g_irq_action[irqn].handler = handler;
	if (name) {
		memcpy(g_irq_action[irqn].name, name, sizeof(g_irq_action[irqn].name));
		g_irq_action[irqn].name[sizeof(g_irq_action[irqn].name) - 1] = 0;
	}
	g_irq_action[irqn].irqn = irqn ; //- gic_data.hwirq_base;
	g_irq_action[irqn].flags = flags;
	g_irq_action[irqn].priv = priv;
	// set highest priority
	sirq_chip.irq_set_priority( irqn, 7);
	// unmask irq
	sirq_chip.irq_unmask( irqn);
	return 0;
}

void do_irq(void)
{
	int irqn;
	do{
		irqn = sirq_chip.irq_ack();
		if(g_irq_action[irqn].handler && irqn)
		{
//			printf("do_irq irqn=%d\n",irqn);
			g_irq_action[irqn].handler(g_irq_action[irqn].irqn, g_irq_action[irqn].priv);
		}
		else if(irqn)
			printf("g_irq_action[%i] NULL",irqn);
		else //plic_claim =0
			break;
		// clear plic pending
		sirq_chip.irq_eoi(irqn);
	}while(1);
	// clear external interrupt pending
	clear_csr(mip, MIP_MEIE);
}

void disable_irq(unsigned int irqn)
{
	sirq_chip.irq_mask( irqn);
}

void enable_irq(unsigned int irqn)
{
	sirq_chip.irq_unmask( irqn);
}
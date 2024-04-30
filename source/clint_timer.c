#include <stdint.h>

#include "config.h"
#include "riscv.h"
#include "mmap.h"
#include "types.h"

#include "clint_timer.h"

typedef struct {
	volatile uint32_t* timecmpl0;
	volatile uint32_t* timecmph0;
	volatile uint64_t* mtime;
	uint64_t smtime;
	uint64_t interval;
	void (*intr_handler)(void);
} timer_hls_t;

#define TIMER_HZ 1000

void riscv_start_timer(uint64_t interval);
void riscv_stop_timer(void);

timer_hls_t thls;
uint64_t riscv_get_timer_count()
{
	return thls.interval;
}

void riscv_start_timer(uint64_t interval)
{
	/* T-HEAD C906, CLINT */
	thls.timecmpl0 = (volatile uint32_t *)CLINT_TIMECMPL0;
	thls.timecmph0 = (volatile uint32_t *)CLINT_TIMECMPH0;

	thls.interval=0;

	CLINT_MTIME(thls.smtime);

	/* Set compare timer */
	*thls.timecmpl0 = (interval + thls.smtime) & 0xFFFFFFFF;
	*thls.timecmph0 = (interval + thls.smtime) >> 32;

	/* Enable timer interrupt */
	set_csr(mstatus, MSTATUS_MIE);
	set_csr(mie, MIP_MTIP);

	/* C906 mxstatus bit 17 for enable clint*/
//	set_csr(mxstatus, 17);
//	ptr= (unsigned int *) 0x74000000;
//	*ptr = 1;
//	printf("mxstatus=%lx\n",read_csr(mxstatus));
}

void riscv_stop_timer(void)
{
	/* Stop timer and disable timer interrupt */
	clear_csr(mie, MIP_MTIP);
	clear_csr(mip, MIP_MTIP);
}

void timer_interrupt(struct pt_regs *regs)
{
	uint64_t tval;
	if(thls.intr_handler)
        	thls.intr_handler();
	/* Clear timer interrupt pending */
	clear_csr(mip, MIP_MTIP);
	thls.interval++;
	/* Update timer for real interval, and not consider about overflow (64 bit) in this case*/
	tval = (thls.interval + 1) * CONFIG_CPU_TIMER_CLOCK / TIMER_HZ + thls.smtime;
	*thls.timecmpl0 = tval & 0xFFFFFFFF;
	*thls.timecmph0 = tval >> 32;
}


void clint_timer_init(void)
{
}

u64 clint_timer_get_tick(void)
{
	u64 cntpct=0;
	asm volatile("rdtime %0" : "=r" (cntpct));
	return cntpct;
}

void clint_timer_mdelay(u32 ms)
{
	u64 cnt = TIMER_CNT_MS * ms;
	u64 start_tick = clint_timer_get_tick();
	u64 exp_tick = start_tick + cnt;
	u64 val;
	do {
		val = clint_timer_get_tick();
	} while (val < exp_tick);
}

void clint_timer_udelay(u32 us)
{
	u64 cnt = TIMER_CNT_US * us;
	u64 start_tick = clint_timer_get_tick();
	u64 exp_cnt = start_tick + cnt;
	u64 val;
	do {
		val = clint_timer_get_tick();
	} while (val < exp_cnt);
}

static u64 timer_meter_start_cnt = 0;

void clint_timer_meter_start(void)
{
	timer_meter_start_cnt = clint_timer_get_tick();
}

u64 clint_timer_meter_get_tick(void)
{
	return (clint_timer_get_tick() - timer_meter_start_cnt);
}

u32 clint_timer_meter_get_ms(void)
{
	return (clint_timer_get_tick() - timer_meter_start_cnt) / TIMER_CNT_MS;
}

u32 clint_timer_meter_get_us(void)
{
	return (clint_timer_get_tick() - timer_meter_start_cnt) / TIMER_CNT_US;
}

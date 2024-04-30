#ifndef _CLINT_TIMER_H_
#define _CLINT_TIMER_H_

#include "config.h"
#include "types.h"

#define TIMER_CNT_US    (CONFIG_TIMER_FREQ / (1000 * 1000))
#define TIMER_CNT_MS    (CONFIG_TIMER_FREQ / (1000))

void clint_timer_init(void);

/* tick */
uint64_t clint_timer_get_tick(void);

/* delay */
void clint_timer_mdelay(uint32_t ms);
void clint_timer_udelay(uint32_t us);

/* meter */
void clint_timer_meter_start(void);
uint32_t clint_timer_meter_get_ms(void);
uint32_t clint_timer_meter_get_us(void);
uint64_t clint_timer_meter_get_tick(void);

#define mdelay(__ms)	clint_timer_mdelay(__ms)
#define udelay(__us)	clint_timer_udelay(__us)

#endif // _CLINT_TIMER_H_

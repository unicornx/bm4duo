#ifndef _SYSTEM_H_
#define _SYSTEM_H_

static inline void opdelay(unsigned int times)
{
	while (times--)
		__asm__ volatile("nop");
}

// cache operation api
extern void dcache_disable(void);
extern void dcache_enable(void);
extern void inv_dcache_all(void);
extern void clean_dcache_all(void);
extern void inv_clean_dcache_all(void);

extern void system_init(void);

#endif /* _SYSTEM_H_ */

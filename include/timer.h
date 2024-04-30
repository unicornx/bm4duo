#ifndef __TIMER_H__
#define __TIMER_H__

#define TIMER_IRQ(n)	(79 + n)

enum TIMER_ID {
	TIMER_ID0 = 0,
	TIMER_ID1,
	TIMER_ID2,
	TIMER_ID3,
	TIMER_ID4,
	TIMER_ID5,
	TIMER_ID6,
	TIMER_ID7,
	MAX_TIMER_ID,
};

struct timerN_regs {
	uint32_t TimerNLoadCount;
	uint32_t TimerNCurrentValue;
	uint32_t TimerNControlReg;
	uint32_t TimerNEOI;
	uint32_t TimerNIntStatus;
};

struct timer_reg {
	struct timerN_regs timern_regs[8];
	uint32_t TimersIntStatus;
	uint32_t TimersEOI;
	uint32_t TimersRawIntStatus;
};

#endif // __TIMER_H__
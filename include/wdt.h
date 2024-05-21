#ifndef __WATCHDOG_H__
#define __WATCHDOG_H__

#define WDT_CR		0x000	// Control register
#define WDT_TORR	0x004	// Timeout range register
#define WDT_CCVR	0x008	// Current counter value register
#define WDT_CRR		0x00c	// Counter restart register
#define WDT_STAT	0x010	// Interrupt status register
#define WDT_EOI		0x014	// Interrupt clear register
#define WDT_TOC		0x01C	// Time Out Count

// CR bits
#define WDT_CR_ENABLE		1 << 0
#define WDT_CR_RESPMODE_2PHASE	1 << 1  // 2 phases, first generate an interrupt
					// and if it is not cleared by the time,
					// a second timeout occurs then generate
					// a system reset
#define WDT_CR_RESET_PULSE_LEN_2	0 << 2
#define WDT_CR_RESET_PULSE_LEN_4	1 << 2
#define WDT_CR_RESET_PULSE_LEN_8	2 << 2
#define WDT_CR_RESET_PULSE_LEN_16	3 << 2
#define WDT_CR_RESET_PULSE_LEN_32	4 << 2
#define WDT_CR_RESET_PULSE_LEN_64	5 << 2
#define WDT_CR_RESET_PULSE_LEN_128	6 << 2
#define WDT_CR_RESET_PULSE_LEN_256	7 << 2
#define WDT_CR_TOR_MODE_1		1 << 6
#define WDT_CR_ITOR_MODE_1		1 << 7

#endif // __WATCHDOG_H__
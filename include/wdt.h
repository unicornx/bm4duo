#ifndef __WATCHDOG_H__
#define __WATCHDOG_H__

#define WDT_CR		0x000	// Control register
#define WDT_TORR	0x004	// Timeout range register
#define WDT_CCVR	0x008	// Current counter value register
#define WDT_CRR		0x00c	// Counter restart register
#define WDT_STAT	0x010	// Interrupt status register
#define WDT_EOI		0x014	// Interrupt clear register
#define WDT_TOC		0x01C	// Time Out Count

#endif // __WATCHDOG_H__
#ifndef __COMMON_H__
#define __COMMON_H__

#include <time.h>
#include <stdio.h>

#include "bits.h"
#include "system.h"
#include "clint_timer.h"
#include "mmap.h"
#include "mmio.h"
#include "top.h"
#include "irq.h"
#include "clock.h"

#include "uart.h"
#include "pwm.h"
#include "rtc.h"
#include "wdt.h"
#include "i2c.h"
#include "gpio.h"
#include "pinmux.h"
#include "adc.h"
#include "timer.h"


extern void test_gpio(int loop);
extern void test_clock();
extern void test_uart_irq();
extern void test_pwm();
extern void test_rtc();
extern void test_rtc_irq();
extern void test_wdt();

extern int test_sdhci_rw_registers();

extern int test_sdhci_detect_sdcard();

extern int mmc_identify_test();
extern int mmc_single_blk_rwe_test();

extern void test_adc();

extern void test_i2c();

extern void test_timer();

#endif // __COMMON_H__


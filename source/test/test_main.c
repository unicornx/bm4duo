/*
 * SPDX-License-Identifier: GPL-2.0+
 */

#include "common.h"

int testcase_main(void)
{
#if defined(CONFIG_TEST_GPIO)
	printf("====> Running GPIO example ......\n");
	test_gpio(30);

#elif defined(CONFIG_TEST_CLOCK)
	printf("====> Running CLOCK example ......\n");
	test_clock();

#elif defined(CONFIG_TEST_IRQ)
	printf("====> Running IRQ example ......\n");
	test_irq();

#elif defined(CONFIG_TEST_PWM)
	printf("====> Running PWM example ......\n");
	test_pwm();

#elif defined(CONFIG_TEST_RTC)
	printf("====> Running RTC example ......\n");
	test_rtc();

#elif defined(CONFIG_TEST_RTC_IRQ)
	printf("====> Running RTC-IRQ example ......\n");
	test_rtc_irq();

#elif defined(CONFIG_TEST_WDT)
	printf("====> Running WatchDog example ......\n");
	test_wdt();

#elif defined(CONFIG_TEST_ADC)
	printf("====> Running ADC example ......\n");
	test_adc();

#elif defined(CONFIG_TEST_I2C)
	printf("====> Running I2C example ......\n");
	test_i2c();

#elif defined(CONFIG_TEST_I2C_S)
	printf("====> Running I2C Slave example ......\n");
	test_i2c_slave();

#elif defined(CONFIG_TEST_TIMER)
	printf("====> Running TIMER example ......\n");
	test_timer();

#elif defined(CONFIG_TEST_SPI)
	printf("====> Running SPI example ......\n");
	test_spi();

#elif defined(CONFIG_TEST_LPC)
	printf("====> Running Low-Power-Consumption example ......\n");
	test_lpc();

#else
	printf("No test is selected, please double-check!\n");

#endif

	return 0;
}


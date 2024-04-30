#ifndef _MEMORYMAP_H_
#define _MEMORYMAP_H_

// TRM Page 25 地址空间映像

#define SEC_BASE            0x02000000
#define TOP_BASE            0x03000000 /* TOP_MISC */

#define SPACC_BASE          (SEC_BASE + 0x00060000)
#define TRNG_BASE           (SEC_BASE + 0x00070000)
#define SEC_DBG_I2C_BASE    (SEC_BASE + 0x00080000)
#define FAB_FIREWALL_BASE   (SEC_BASE + 0x00090000)
#define DDR_FIREWALL_BASE   (SEC_BASE + 0x000A0000)

#define PINMUX_BASE         (0x03001000)
#define CLKGEN_BASE         (0x03002000) // Page 67
#define RSTGEN_BASE         (0x03003000)
#define TEMPSEN_BASE        (TOP_BASE + 0xE0000)
#define WDT_BASE            (TOP_BASE + 0x11000)

#define TOP_USB_PHY_CTRSTS_REG	(TOP_BASE + 0x48)
#define TOP_DDR_ADDR_MODE_REG	(TOP_BASE + 0x64)
#define TOP_USB_CTRSTS_REG	(TOP_BASE + 0x38)

#define WATCHDOG0_BASE		0x03010000
#define WATCHDOG1_BASE		0x03011000
#define WATCHDOG2_BASE		0x03012000

#define REG_PWM0_BASE		0x03060000
#define REG_PWM1_BASE		0x03061000
#define REG_PWM2_BASE		0x03062000
#define REG_PWM3_BASE		0x03063000

#define REG_TIMER_BASE		0x030A0000

#define ADC_BASE		0x030F0000
#define PWR_ADC_BASE		0x0502C000

#define I2C0_BASE		0x04000000
#define I2C1_BASE		0x04010000
#define I2C2_BASE		0x04020000
#define I2C3_BASE		0x04030000
#define I2C4_BASE		0x04040000
#define RTCSYS_I2C_BASE		0x0502B000

#define UART0_BASE          0x04140000
#define UART1_BASE          0x04150000
#define UART2_BASE          0x04160000
#define UART3_BASE          0x04170000
#define UART4_BASE          0x041C0000
#define PWR_UART_BASE       0x05022000

#define GPIO0_BASE          0x03020000 // XGPIOA
#define GPIO1_BASE          0x03021000 // XGPIOB
#define GPIO2_BASE          0x03022000 // XGPIOC
#define GPIO3_BASE          0x03023000 // XGPIOD
#define PWR_GPIO_BASE       0x05021000
#define SRAM_BASE           0x0E000000


#define SD0_BASE	0x04310000
#define SD1_BASE	0x04320000

#define RTC_CTRL_REG_BASE	0x05025000
#define RTC_CORE_REG_BASE	0x05026000
#define RTC_MACRO_REG_BASE	0x05026400


/* RISC-V */
#define CLINT_BASE              0x74000000
#define PLIC_BASE               0x70000000

/* CLINT */
#define CLINT_TIMECMPL0         (CLINT_BASE + 0x4000)
#define CLINT_TIMECMPH0         (CLINT_BASE + 0x4004)

#define CLINT_MTIME(cnt)             asm volatile("csrr %0, time\n" : "=r"(cnt) :: "memory");

/* PLIC */
#define PLIC_PRIORITY0          (PLIC_BASE + 0x0)
#define PLIC_PRIORITY1          (PLIC_BASE + 0x4)
#define PLIC_PRIORITY2          (PLIC_BASE + 0x8)
#define PLIC_PRIORITY3          (PLIC_BASE + 0xc)
#define PLIC_PRIORITY4          (PLIC_BASE + 0x10)

#define PLIC_PENDING1           (PLIC_BASE + 0x1000)
#define PLIC_PENDING2           (PLIC_BASE + 0x1004)
#define PLIC_PENDING3           (PLIC_BASE + 0x1008)
#define PLIC_PENDING4           (PLIC_BASE + 0x100C)

#define PLIC_ENABLE1            (PLIC_BASE + 0x2000)
#define PLIC_ENABLE2            (PLIC_BASE + 0x2004)
#define PLIC_ENABLE3            (PLIC_BASE + 0x2008)
#define PLIC_ENABLE4            (PLIC_BASE + 0x200C)

#define PLIC_THRESHOLD          (PLIC_BASE + 0x200000)
#define PLIC_CLAIM              (PLIC_BASE + 0x200004)

#endif /* _MEMORYMAP_H_ */

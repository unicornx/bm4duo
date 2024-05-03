#ifndef __CLOCK_H__
#define __CLOCK_H__

#include "mmap.h"

#define REG_CLK_EN_0			(CLKGEN_BASE)
#define REG_CLK_EN_1			(CLKGEN_BASE + 0x004)
#define REG_CLK_EN_2			(CLKGEN_BASE + 0x008)
#define REG_CLK_EN_3			(CLKGEN_BASE + 0x00c)
#define REG_CLK_EN_4			(CLKGEN_BASE + 0x010)
#define REG_CLK_SEL_0			(CLKGEN_BASE + 0x020)
#define REG_CLK_BYP_0			(CLKGEN_BASE + 0x030)
#define REG_CLK_BYP_1			(CLKGEN_BASE + 0x034)
#define REG_DIV_CLK_A53_0		(CLKGEN_BASE + 0x040)
#define REG_DIV_CLK_CPU_AXI0		(CLKGEN_BASE + 0x048)
#define REG_DIV_CLK_TPU			(CLKGEN_BASE + 0x054)
#define REG_DIV_CLK_C906_0_0		(CLKGEN_BASE + 0x130)
#define REG_DIV_CLK_C906_0_1		(CLKGEN_BASE + 0x134)

#define CLK_PLL_BASE			(CLKGEN_BASE + 0x800)

#define CLK_EN_1_8_BIT	BIT(8) // clk_apb_pwm

#define CLK_EN_4_4_BIT	BIT(4) // clk_pwm_src

#endif // __CLOCK_H__
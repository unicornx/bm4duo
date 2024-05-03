// 演示系统时钟的设置
// 复用 GPIO 的用例，观察通过修改 CPU 的时钟频率对 LED 闪烁的快慢的影响

#include "common.h"

void test_clock()
{
	int32_t val;

	val = mmio_read_32(REG_CLK_SEL_0);

	// 默认情况下
	// REG_CLK_SEL_0 is 0x1800000
	// Select div_clk_c906_0_0 as clock source
	// REG_DIV_CLK_C906_0_0 is 0x10309
	// 0001-0000-0011-0000-1001
	// bit[0]: 1: De-assert Reset
	// bit[1]: 0: Low level of the clock is wider 
	// bit[2]: 0: Select initial value
	// bit[3]: 1: Select High Wide from this register
	// bit[9:8]: clk_src, 3, mpll
	// [20:16] Clock Divider Factor: 1
	printf("REG_CLK_SEL_0 is 0x%x\n", val);
	if (val & 1 << 23) {
		printf("Select div_clk_c906_0_0 as clock source\n");
		val = mmio_read_32(REG_DIV_CLK_C906_0_0);
		printf("REG_DIV_CLK_C906_0_0 is 0x%x\n", val);
	} else {
		printf("Select div_clk_c906_0_1 as clock source\n");
		val = mmio_read_32(REG_DIV_CLK_C906_0_1);
		printf("REG_DIV_CLK_C906_0_1 is 0x%x\n", val);
	}

	test_gpio(10);

	// 改变 div
	// FIXME: 需要搞清楚 High Wide Control 的概念
	// 以及为何无法设置 Divider Factor > 0xf
#if 0
	for (uint32_t i = 0xa; i < 0x1f; i++) {
		val &= ~(0x1f << 16);
		val |= i << 16;
		mmio_write_32(REG_DIV_CLK_C906_0_0, val);

		mdelay(10);
		val = mmio_read_32(REG_DIV_CLK_C906_0_0);
		printf("REG_DIV_CLK_C906_0_0 is 0x%x\n", val);
	}
#endif

	val &= ~(0x1f << 16);
	val |= 0xa << 16 | 1 << 2;
	mmio_write_32(REG_DIV_CLK_C906_0_0, val);

	mdelay(10);
	val = mmio_read_32(REG_DIV_CLK_C906_0_0);
	printf("REG_DIV_CLK_C906_0_0 is 0x%x\n", val);

	test_gpio(20);
}
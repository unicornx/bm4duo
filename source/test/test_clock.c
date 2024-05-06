// 演示系统时钟的设置
// 复用 GPIO 的用例，观察通过修改 CPU 的时钟频率对 LED 闪烁的快慢的影响
// 我们使用 Duo 的 C906 大核演示该用例。
// C906 大核默认使用 clk_c906_0，该时钟的默认 parent clock 为 mpll，默认频率为 850Mhz
// 同时该时钟支持 bypass 到 xtal(25Mhz)。对比切换前后 LED 的闪烁频率，会发现切换到
// xtal 后 LED 闪烁频率显著变慢。如果严格测量应该在 34 倍左右。

#include "common.h"

void test_clock()
{
	int32_t val;

	// 先采用默认 cpu 主频点灯
	printf("Lighting with default cpu frequency\n");
	test_gpio(10);

	// 切换 clk_c906_0, bypass 到 xtal
	val = mmio_read_32(REG_CLK_BYP_1);
	val |=  1 << 6;
	mmio_write_32(REG_CLK_BYP_1, val);

	// 等待切换完成
	mdelay(10);
	printf("Switched cpu clock to xtal.\n");

	printf("Lighting with cpu frequency bypass to xtal.\n");
	test_gpio(10);
}
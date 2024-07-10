#include "common.h"

// 本实验演示通过 PWM 产生方波驱动无源扬声器发出声音
// 参考【DUO256-PINOUT】，利用 GP4 的 PWM5 功能，duo 的引脚连接方式相同。
// 更多实验说明，参考 `docs/labs/pwm.md`。

#define REG_PWM_BASE		REG_PWM1_BASE
void test_pwm()
{
	uint32_t val;

	// 使用 GP4 引脚，选择 PWM5 功能
	PINMUX_CONFIG(SD1_D2, PWM_5);

#if 0
	// 打开时钟 Gate clk clk_apb_pwm & clk_pwm_src
	// 这两个 Gate clk 默认是开的。
	// 按照 TRM 描述: PWM 的时钟来源为 100MHz 和 148.5MHz 二选一，默认为 100MHz。 
	val = mmio_read_32(REG_CLK_EN_1);
	printf("REG_CLK_EN_1 = 0x%x\n", val);
	val |= CLK_EN_1_8_BIT;
	mmio_write_32(REG_CLK_EN_1, val);

	val = mmio_read_32(REG_CLK_EN_4);
	printf("REG_CLK_EN_4 = 0x%x\n", val);
	val |= CLK_EN_4_4_BIT;
	mmio_write_32(REG_CLK_EN_4, val);
#endif

	// TRM 要求：
	// When PWMMODE is set to 0, write bit n to 0 and then write 1 to start
	// PWMn output, until bit n is written to 0 to stop output. 
	// When PWMMODE is set to 1, write bit n to 1 to start PWMn output, 
	// and stop output automatically when the number of pulses output output
	// equals to the value of PCOUNTn.
	// 我们这里选择的是 PWMMODE 为 0，即持续输出模式，所以要先设置对应通道为 0，
	// 然后再最后设置对应通道为 1 来启动 PWM。
	val = mmio_read_32(REG_PWM_BASE + PWMSTART);
	val &= ~(1 << 1);
	mmio_write_32(REG_PWM_BASE + PWMSTART, val);

	int period, hlp	, pola;

	// 正常的人耳能听到 20 赫玆到 20,000 赫玆频率的声音。
	// PERIOD = 100MHz / Fpwm
	// 例1：取 Fpwm 为 2000Hz， 则 PERIOD=50000
	// 例2：取 Fpwm 为 5000Hz， 则 PERIOD=20000
	period = 50000;
	hlp = 37500;
	pola = 1;
	// 设置占空比
	// POLARITY[0] 的值为 0 时，HLPERIOD 是对应的低电平
	// POLARITY[0] 的值为 1 时，HLPERIOD 是对应的高电平
	// 所以这里占空比 DC = 37500/50000 = 75%
	// 反之，如果 period 和 hlp 值不变，pola 值变为 0，则占空比变为 25%
	mmio_write_32(REG_PWM_BASE + HLPERIOD1, hlp);
	mmio_write_32(REG_PWM_BASE + PERIOD1, period);

	// POLARITY.POLARITY: [3:0], 每一位对应四个 PWM controller中的一个
	// POLARITY[0] 对应 PWM0，依次类推。用于控制方波的极性
	// - 0：先低再高（默认）
	// - 1：先高再低
	val = mmio_read_32(REG_PWM_BASE + POLARITY);
	if (pola)
		val |= 1 << 1;
	else 
		val &= ~(1 << 1);
	mmio_write_32(REG_PWM_BASE + POLARITY, val);

	// POLARITY.PWMMODE : [11:8], 每一位对应四个 PWM controller 的一个
	// POLARITY[8] 对应 PWM0，依次类推，用于控制 PWM 的工作模式是
	// - 连续输出模式（Continuous Out） 方式（默认）
	// - 固定脉冲个数 （fixed output） 方式，需要设置 PCOUNT，达到设定的方波数后自动结束，
	//   状态寄存器 PWMDONE 由 0 转 1
	val = mmio_read_32(REG_PWM_BASE + PWMOE);
	val |= 1 << 1;
	mmio_write_32(REG_PWM_BASE + PWMOE, val);

	val = mmio_read_32(REG_PWM_BASE + PWMSTART);
	val |= 1 << 1;
	mmio_write_32(REG_PWM_BASE + PWMSTART, val);
}

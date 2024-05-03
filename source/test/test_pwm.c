#include "common.h"

// 本实验希望演示通过 PWM 产生方波驱动无源扬声器发生

#define REG_PWM_BASE		REG_PWM1_BASE
void test_pwm1()
{
	uint32_t val;

	PINMUX_CONFIG(SD1_D2, PWM_5);
	//PINMUX_CONFIG(SD1_GPIO1, PWM_10);

	val = mmio_read_32(0x0502703c);
	printf("IO = %x\n", val);
	//mmio_write_32(0x0502703c, 0x848);

#if 1
	// Enable Gate clk clk_apb_pwm
	//mmio_write_32(0x03002004, mmio_read_32(0x03002004) | (1 << 8));
	val = mmio_read_32(REG_CLK_EN_1);
	val |= CLK_EN_1_8_BIT;
	mmio_write_32(REG_CLK_EN_1, val);
	
	// Enable Gate Clock clk_pwm_src
	//mmio_write_32(0x03002010, mmio_read_32(0x03002010) | (1 << 4));
	val = mmio_read_32(REG_CLK_EN_4);
	val |= CLK_EN_4_4_BIT;
	mmio_write_32(REG_CLK_EN_4, val);
#endif

	// Page 90, clk_sel_0
	// 1: Select int_clk_pwm_src as clock source 0: Select int_clk_axi6 as clock source
	// FIXME: 从手册上看不出这里设置的什么意思？
	//mmio_write_32(0x03002020, 0xFFFFFEFF);
	mmio_write_32(0x03002020, 0x000F0009);
	val = mmio_read_32(0x03002120);
	printf("div_clk_pwm_src_0 is %x\n", val);

	// Page 592， PWM

	// 寄存器
	// POLARITY.POLARITY: [3:0], 每一位对应四个 PWM controller中的一个
	// POLARITY[0] 对应 PWM0，依次类推。用于控制方波的极性
	// - 0：先低再高（默认）
	// - 1：先高再低
	//
	// POLARITY.PWMMODE : [11:8], 每一位对应四个 PWM controller 的一个
	// POLARITY[8] 对应 PWM0，依次类推，用于控制 PWM 的工作模式是 
	// - 连续输出模式（Continuous Out） 方式（默认）
	// - 固定脉冲个数 （fixed output） 方式，需要设置 PCOUNT，达到设定的方波数后自动结束，状态寄存器 PWMDONE 由 0 转 1
	int period, hlp	, pola;

	period = 100;
	hlp = 60;
	pola = 0;

	mmio_write_32(REG_PWM_BASE + PWMSTART, 0x00);

//	mmio_write_32(REG_PWM_BASE + HLPERIOD0, period - hlp);
//	mmio_write_32(REG_PWM_BASE + PERIOD0, period);

	mmio_write_32(REG_PWM_BASE + HLPERIOD1, period - hlp);
	mmio_write_32(REG_PWM_BASE + PERIOD1, period);

//	mmio_write_32(REG_PWM_BASE + HLPERIOD2, period - hlp);
//	mmio_write_32(REG_PWM_BASE + PERIOD2, period);

//	mmio_write_32(REG_PWM_BASE + HLPERIOD3, period - hlp);
//	mmio_write_32(REG_PWM_BASE + PERIOD3, period);

	//mmio_write_32(REG_PWM_BASE + PWMOE, 0xf);//output
	val = mmio_read_32(REG_PWM_BASE + POLARITY);
	printf("POLARITY is %x\n", val);
	if (pola)
		val |= 0xf;
	else 
		val &= ~0xf;
	printf("2 POLARITY is %x\n", val);
	mmio_write_32(REG_PWM_BASE + POLARITY, val);
	//mmio_write_32(REG_PWM_BASE + PWMSTART, 0x0F);



	//mmio_write_32(REG_PWM_BASE + POLARITY, pola);
	//mmio_write_32(REG_PWM_BASE + POLARITY, (pola ? 0xf : 0x0));
	//mmio_write_32(REG_PWM_BASE + PWMOE, 0xf);//output
	//printf("Output %d%% duty cycle waveform, clk=pclk/%d\n", hlp * 100 / period, period);

	// TRM 要求：
	// When PWMMODE is set to 0, write bit n to 0 and then write 1 to start
	// PWMn output, until bit n is written to 0 to stop output. 
	// When PWMMODE is set to 1, write bit n to 1 to start PWMn output, 
	// and stop output automatically when the number of pulses output output
	// equals to the value of PCOUNTn.
	//mmio_write_32(REG_PWM_BASE + PWMSTART, 0x00);

	mmio_write_32(REG_PWM_BASE + PWMSTART, 0x0F);

	mmio_write_32(REG_PWM_BASE + PWMOE, 0xf);//output
}


void test_pwm3()
{
	uint32_t val;

	// https://milkv.io/docs/duo/getting-started/duo
	// 我们选用 Duo 的 GP4 引脚，选择 PWM5 功能
	PINMUX_CONFIG(SD1_D2, PWM_5);

	// 打开时钟 Gate clk clk_apb_pwm & clk_pwm_src
	// 这两个 Gate clk 默认是开的。
	// FIXME: 需要理清 pwm 的时钟 source
	// 按照 TRM 描述: PWM 的时钟来源为 100MHz 和 148.5MHz 二选一，默认为 100MHz。 
	// 但是还无法找到 TRM 中有关该两个时钟源从哪里来的
#if 0	
	val = mmio_read_32(CLK_GATE_CLK_EN_1);
	printf("REG_CLK_EN_1 = 0x%x\n", val);

	val = mmio_read_32(REG_CLK_EN_4);
	printf("REG_CLK_EN_4 = 0x%x\n", val);
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

	// 正常的人耳能聽到 20 赫玆到 20,000 赫玆頻率的聲音。
	// PERIOD = 100MHz / Fpwm
	// 取 Fpwm 为 2000Hz， PERIOD=50000
	// 取 Fpwm 为 5000Hz， PERIOD=20000
	period = 50000;
	hlp = 37500;
	//period = 20000;
	//hlp = 15000;
	pola = 1;

	mmio_write_32(REG_PWM_BASE + HLPERIOD1, hlp);
	mmio_write_32(REG_PWM_BASE + PERIOD1, period);
	val = mmio_read_32(REG_PWM_BASE + POLARITY);
	if (pola)
		val |= 1 << 1;
	else 
		val &= ~(1 << 1);
	mmio_write_32(REG_PWM_BASE + POLARITY, val);

	val = mmio_read_32(REG_PWM_BASE + PWMOE);
	val |= 1 << 1;
	mmio_write_32(REG_PWM_BASE + PWMOE, val);

	val = mmio_read_32(REG_PWM_BASE + PWMSTART);
	val |= 1 << 1;
	mmio_write_32(REG_PWM_BASE + PWMSTART, val);
}

void test_pwm2()
{
	uint32_t val;

	printf("\n----> 22222222\n");

	PINMUX_CONFIG(SD1_D2, PWM_5);
	PINMUX_CONFIG(SD1_D1, PWM_6);

	//PINMUX_CONFIG(SD1_GPIO1, PWM_10);

	val = mmio_read_32(REG_PWM_BASE + PWMOE);
	val |= 1 << 1;
	val |= 1 << 2;
	mmio_write_32(REG_PWM_BASE + PWMOE, val);



	val = mmio_read_32(REG_PWM_BASE + PWMSTART);
	val &= ~(1 << 1);
	val &= ~(1 << 2);
	mmio_write_32(REG_PWM_BASE + PWMSTART, val);

	mmio_write_32(REG_PWM_BASE + HLPERIOD1, 0x11);
	mmio_write_32(REG_PWM_BASE + PERIOD1, 0x22);
	
	mmio_write_32(REG_PWM_BASE + HLPERIOD2, 0x11);
	mmio_write_32(REG_PWM_BASE + PERIOD2, 0x22);

	val = mmio_read_32(REG_PWM_BASE + PWMSTART);
	val |= 1 << 1;
	val |= 1 << 2;
	mmio_write_32(REG_PWM_BASE + PWMSTART, val);
}

void test_pwm()
{
	test_pwm3();
}

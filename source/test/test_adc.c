#include "common.h"

// 演示通过检测可变电压输出，

// 使用 DUO 上默认的 ADC1

// 这个是最初的版本
void test_adc1()
{
	uint32_t val;
	uint32_t val_ch1, val_ch2, val_ch3;

	val = mmio_read_32(0x030010a8);
	printf("FMUX_GPIO_REG_IOCTRL_ADC1 = 0x%x\n", val);

	val = mmio_read_32(0x03001804);
	printf("IOBLK_G1_REG_ADC1 = 0x%x\n", val);

	val = mmio_read_32(ADC_BASE + SARADC_CTRL);
	printf("SARADC_CTRL = 0x%x\n", val);

	// read status
	val = mmio_read_32(ADC_BASE + SARADC_STATUS);
	printf("SARADC_STATUS = 0x%x\n", val);

	val |= (7 << 4) | 1;
	mmio_write_32(ADC_BASE + SARADC_CTRL, val);

	// read status
	while (1) {
		val = mmio_read_32(ADC_BASE + SARADC_STATUS);
		printf("SARADC_STATUS = 0x%x\n", val);

		val_ch1 = mmio_read_32(ADC_BASE + SARADC_CH1);
		val_ch2 = mmio_read_32(ADC_BASE + SARADC_CH2);
		val_ch3 = mmio_read_32(ADC_BASE + SARADC_CH3);
		printf("--> 0x%x, 0x%x, 0x%x\n", val_ch1, val_ch2, val_ch3);

		if (val == 0x700)
			break;
	}
}

void test_adc2()
{
	uint32_t val;
	uint32_t val_ch1;
	//uint32_t val_ch2;
	//uint32_t val_ch3;

	val = mmio_read_32(ADC_BASE + SARADC_CTRL);
	printf("SARADC_CTRL = 0x%x\n", val);

	// read status
	val = mmio_read_32(ADC_BASE + SARADC_STATUS);
	printf("SARADC_STATUS = 0x%x\n", val);

	// bit4，5 --> ch1
	// bit6 --> ch2
	// bit7 --> ch3
	val |= 0x2 << 4;
	mmio_write_32(ADC_BASE + SARADC_CTRL, val);

	val = mmio_read_32(ADC_BASE + SARADC_CYC_SET);
	printf("SARADC_CYC_SET = 0x%x\n", val);
	// default: 0xb130f
	// 1011-0001-0011-000-01111  和 TRM 一致
	// 控制器工作频率 12.5MHz; 这个参考 reg_saradc_cyc_clkdiv，感觉就是根据
	// 公式：freq = ip_clk/（1+clk_div）
	// 其中
	// ip_clk = 25Mhz
	// clk_div 就是寄存器 reg_saradc_cyc_clkdiv，默认值为 1
	// 所以 freq = 25Mhz / (1 +1) = 12.5Mhz
	// 扫描频率不能高于 320K/s;

	val |= (0x1f) | (0xf << 8) | (0xf << 16);
	mmio_write_32(ADC_BASE + SARADC_CYC_SET, val);
	//mmio_clrsetbits_32(ADC_BASE + SARADC_CYC_SET, 0xf000, 0xf << 12);
	//mmio_clrsetbits_32(ADC_BASE + SARADC_CYC_SET, 0xf000, 0x2 << 12);

	val = mmio_read_32(ADC_BASE + SARADC_CTRL);
	printf("SARADC_CTRL = 0x%x\n", val);
	val |= 1;
	mmio_write_32(ADC_BASE + SARADC_CTRL, val);

	mdelay(10);

#if 0
	while (1) {
		val = mmio_read_32(ADC_BASE + SARADC_STATUS);
		printf("SARADC_STATUS = 0x%x\n", val);

		val_ch1 = mmio_read_32(ADC_BASE + SARADC_CH1);
		val_ch2 = mmio_read_32(ADC_BASE + SARADC_CH2);
		val_ch3 = mmio_read_32(ADC_BASE + SARADC_CH3);
		printf("--> 0x%x, 0x%x, 0x%x\n", val_ch1, val_ch2, val_ch3);

		if (val == 0x700)
			break;
	}
#endif

	int count= 0;
	while (1) {
		val = mmio_read_32(ADC_BASE + SARADC_STATUS);
		
		val_ch1 = mmio_read_32(ADC_BASE + SARADC_CH1);
		
		count++;

		if (!(val & 1))
			break;
	}

	printf("--> count = %d, 0x%x\n", count, val_ch1);


#if 0
	while (1) {
		// check adc busy
		// 这种方式无法退出死循环
		while(mmio_read_32(ADC_BASE + SARADC_STATUS) & 0x200);

		val_ch1 = mmio_read_32(ADC_BASE + SARADC_CH1);
		val_ch2 = mmio_read_32(ADC_BASE + SARADC_CH2);
		val_ch3 = mmio_read_32(ADC_BASE + SARADC_CH3);
		printf("--> 0x%x, 0x%x, 0x%x\n", val_ch1, val_ch2, val_ch3);
	}
#endif


#if 0
	while (1) {
		// check adc busy
		while(mmio_read_32(ADC_BASE + SARADC_STATUS) & 0x1);
		val_ch1 = mmio_read_32(ADC_BASE + SARADC_CH1);
		val_ch2 = mmio_read_32(ADC_BASE + SARADC_CH2);
		val_ch3 = mmio_read_32(ADC_BASE + SARADC_CH3);
		printf("--> 0x%x, 0x%x, 0x%x\n", val_ch1, val_ch2, val_ch3);
	}
#endif
}

// 测试手动启动多次扫描，看看结果
#define NUM 100
void test_adc3()
{
	uint32_t val;
	uint32_t val_ch1[NUM];

	val = mmio_read_32(ADC_BASE + SARADC_CTRL);
	printf("SARADC_CTRL = 0x%x\n", val);

	// read status
	val = mmio_read_32(ADC_BASE + SARADC_STATUS);
	printf("SARADC_STATUS = 0x%x\n", val);

	// bit4，5 --> ch1
	// bit6 --> ch2
	// bit7 --> ch3
	val |= 0x2 << 4;
	mmio_write_32(ADC_BASE + SARADC_CTRL, val);

	val = mmio_read_32(ADC_BASE + SARADC_CYC_SET);
	printf("SARADC_CYC_SET = 0x%x\n", val);
	mmio_clrsetbits_32(ADC_BASE + SARADC_CYC_SET, 0xf000, 0x2 << 12);
	
	val = mmio_read_32(ADC_BASE + SARADC_CTRL);
	printf("SARADC_CTRL = 0x%x\n", val);
	val |= 1;
	mmio_write_32(ADC_BASE + SARADC_CTRL, val);

	mdelay(10);

	int i;
	for (i = 0; i < NUM; i++) {
		while (mmio_read_32(ADC_BASE + SARADC_STATUS) & 0x1);
		
		val_ch1[i] = mmio_read_32(ADC_BASE + SARADC_CH1);

		val = mmio_read_32(ADC_BASE + SARADC_CTRL);
		val |= 1;
		mmio_write_32(ADC_BASE + SARADC_CTRL, val);
	}

	for (i = 0; i < NUM; i++) {
		printf("--> sample [%d] = 0x%x\n", i, val_ch1[i]);
	}
}

void test_adc()
{
	test_adc3();
}


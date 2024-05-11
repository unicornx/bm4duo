#include "common.h"

// 演示通过 ADC 采样电压值
// 使用 DUO 上默认的 ADC1

#define NUM 10

void test_adc()
{
	uint32_t val;
	uint32_t val_ch1[NUM], sum;
	int num;
	float volage; 

	// 我们使用 DUO 上的 ADC1 管脚，这个管脚默认提供 ADC1 的功能，无需 select

	// saradc_ctrl.reg_saradc_sel[7:4]
	// 四个 bit 控制的 channel 对应如下：
	// bit4 --> ch1 没有 ch0，所以，bit4 实际上也是用于控制 ch1
	// bit5 --> ch1 -> ADC1
	// bit6 --> ch2 
	// bit7 --> ch3
	// 选择我们要采样的通道
	// 这里我们只选择 Ch1(ADC1)
	val = mmio_read_32(ADC_BASE + SARADC_CTRL);
	val |= 0x2 << 4;
	mmio_write_32(ADC_BASE + SARADC_CTRL, val);

  	// SoC 芯片 ADC1 引脚的参考电压是 1.5V，由于 Duo 针对 ADC1 引脚接了分压
	// 电阻，分压比为 1：2，因此板子级别的 GP26 引脚的电压输入范围为 0 ~ 3V。
  	// 寄存器 saradc_test
	// Reset Value: 0x0
	// bit[2]:0表示使用内部参考电压，1 表示使用外部参考电压。
	val = mmio_read_32(ADC_BASE + SARADC_TEST);
	val |= 0x1 << 2;
	mmio_write_32(ADC_BASE + SARADC_TEST, val);
	
	// 校正参考电压
	// 寄存器 saradc_trim
	// Reset Value: 0x0
	// bit[0:3]:用于调整 vref 的大小。
	val = mmio_read_32(ADC_BASE + SARADC_TRIM);
	val |= 0x4;
	mmio_write_32(ADC_BASE + SARADC_TRIM, val);
	
	// 开启总开关，总开关开启后，会对我们在前面打开的通道开始采样
	val = mmio_read_32(ADC_BASE + SARADC_CTRL);
	printf("SARADC_CTRL = 0x%x\n", val);
	val |= 1;
	mmio_write_32(ADC_BASE + SARADC_CTRL, val);

	// 采样 NUM 次
	int i;
	for (i = 0; i < NUM; i++) {
		// 轮询直到采样完成，till not busy
		while (mmio_read_32(ADC_BASE + SARADC_STATUS) & 0x1);
		
		// 读取采样结果
		val_ch1[i] = mmio_read_32(ADC_BASE + SARADC_CH1);

		// 发起下一次采样
		val = mmio_read_32(ADC_BASE + SARADC_CTRL);
		val |= 1;
		mmio_write_32(ADC_BASE + SARADC_CTRL, val);
	}

	// 打印所有的采样结果，并计算平均值
	// 因为分压电路的影响，我们计算得到的电压值 volage 理论上应该只有 P 点
	// 实际电压表测量值的一半。
	sum = 0;
	num = 0;
	for (i = 0; i < NUM; i++) {
		printf("--> saradc_ch1_result [%d] = 0x%x\n", i, val_ch1[i]);
		if (val_ch1[i] & (1 << 15)) {
			val = val_ch1[i] & 0xfff;
			sum = sum + val;
			num++;
		}
	}
	float average_val_ch1 = sum / num;
	printf("sampling: sum = %d, average is: %d\n", sum, (int)average_val_ch1);
	volage = 1.5 * average_val_ch1 / 4096;
	printf("Caculated Volage is: %.2fV\n", volage);
}

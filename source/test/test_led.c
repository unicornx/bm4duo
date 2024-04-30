#include "common.h"

// 这个实验是演示 GPIO
// 通过设置引脚的 pinmux 选择合适的功能，这里设置 GPIO
// 设置引脚的 pinctrl 控制引脚 GPIO 输出高电平和低电平来控制 LED 灯的开关

// FIXME: DUO256 上的 LED 引脚和 DUO 不同，目前只支持 DUO

void test_led()
{
	// 查看电路图，可知 duo 上连接 LED 的引脚是 PAD_AUD_AOUTR
	// 查看 CV180xb-Pinout-v1.xlsx
	// 该引脚默认 func select 为 3 : XGPIOC[24] (default)，所以 pinmux 设置可以省略
	// 且对应的 GPIO 是 GPIO3 的 bit 24
	uint32_t val = 0;
	// 设置 GPIO 管脚为输出模式 GPIO_SWPORTA_DDR
	val = mmio_read_32(0x03022004);
    	val |= 1 << 24;
	mmio_write_32(0x03022004, val);
	
	// 设置 GPIO 输出高电平， led 亮
	// GPIO_SWPORTA_DR
	val = mmio_read_32(GPIO2_BASE + GPIO_SWPORTA_DR);
    	val |= 1 << 24;
	mmio_write_32(GPIO2_BASE + GPIO_SWPORTA_DR, val);

	// 延迟
       	opdelay(25000000);

	// 设置 GPIO 输出低电平， led 灭
	val = mmio_read_32(GPIO2_BASE + GPIO_SWPORTA_DR);
	val &= ~(1 << 24);
	mmio_write_32(GPIO2_BASE + GPIO_SWPORTA_DR, val);
}
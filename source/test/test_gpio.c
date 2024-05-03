#include "common.h"

/*
 * 这个实验是演示 GPIO 的使用
 * 通过设置引脚的 pinmux 选择合适的功能，这里选择 GPIO 功能
 * 在设置引脚功能的基础上通过控制引脚，输出高电平和低电平来控制 LED 灯的开关
 * 
 * 为简化外部电路设计，本实验使用开发板上自带的 LED 实现一个简单的开关闪烁。
 * 
 * 注意 DUO256 和 DUO 开发板上 LED 引脚连接方式不同导致我们用不同函数实现
 */

static inline void delay(unsigned int times)
{
	while (times--)
		__asm__ volatile("nop");
}

static void test_pinmux_config()
{
#if defined(CONFIG_BOARD_DUO256)
	// 对于 Duo256：
	// 查看 【DUO256-SCHEMA】，可知 duo 上连接 LED 的引脚是 PWR_GPIO2
	// 查看 【SG2002-TRM】10.1.2 FMUX 寄存器描述，搜索 PWR_GPIO2
	// 该引脚默认 func select 为 0 : PWR_GPIO[2] (default)，
	// 所以 pinmux 设置可以省略，但我们这里还是配置一下，以防前面被人改过
	PINMUX_CONFIG(PWR_GPIO2, PWR_GPIO_2);

#elif defined(CONFIG_BOARD_DUO)

	// 对于 Duo
	// 查看【DUO-SCHEMA】，可知 duo 上连接 LED 的引脚是 PAD_AUD_AOUTR
	// 查看【CV180X-PIN】PAD_AUD_AOUTR 引脚默认 func select 为 3 : 
	// XGPIOC[24] (default)，所以 pinmux 设置可以省略
	// 但我们这里还是配置一下，以防前面被人改过
	PINMUX_CONFIG(PAD_AUD_AOUTR, XGPIOC_24);

#else
	#error "Unsupported board type!"
#endif
}

#if defined(CONFIG_BOARD_DUO256)

	// 参考 【SG2002-TRM】的 GPIO 章节可知，系统配置 4 组 GPIO 和
	// 1 组 No-die Domain 下的 GPIO RTCSYS_GPIO，
	// 每组 GPIO 提供 32 个可编程的输入输出管脚。
	// TRM 上的 RTCSYS_GPIO 也被称为 PWR_GPIO，
	// 所以 PWR_GPIO2 即 RTCSYS_GPIO 管理的 第 2 路信号
	// 第 2 路信号对应相应寄存器的 bit 2
	#define GPIO_BASE	PWR_GPIO_BASE
	#define GPIO_CHANNEL	2

#elif defined(CONFIG_BOARD_DUO)

	// 参考 【CV180X-TRM】的 GPIO 章节可知，系统配置 4 组 GPIO,
	// GPIO0 ~ GPIO3。每组 GPIO 提供 32 个可编程的输入输出管脚
	// TRM 上的 GPIO0 ~ GPIO3 也被称为 XGPIOA ~ XGPIOD，
	// 所以 XGPIOC 即 GPIO2，所以 XGPIOC[24] 即 GPIO2 管理的 第 24 路信号
	// 第 24 路信号对应相应寄存器的 bit 24
	#define GPIO_BASE	GPIO2_BASE
	#define GPIO_CHANNEL	24

#else

	#error "Unsupported board type!"

#endif

#define DELAY	2500000

void test_gpio(int loop)
{
	uint32_t val = 0;

	// 配置管脚复用，选择 GPIO 功能
	test_pinmux_config();

	// 配置 GPIO，设置为输出
	// 参考 TRM 的 GPIO 章节可知控制 GPIO 数据输入还是输出的寄存器是 GPIO_SWPORTA_DDR
	// 对应位设置为 1 表示输出
	val = mmio_read_32(GPIO_BASE + GPIO_SWPORTA_DDR);
    	val |= 1 << GPIO_CHANNEL;
	mmio_write_32(GPIO_BASE + GPIO_SWPORTA_DDR, val);

	// 循环点亮和熄灭 led，中间通过简单的空指令（nop）循环实现延时。
	while (loop) {
		// 设置 GPIO 输出高电平， led 亮
		val = mmio_read_32(GPIO_BASE + GPIO_SWPORTA_DR);
    		val |= 1 << GPIO_CHANNEL;
		mmio_write_32(GPIO_BASE + GPIO_SWPORTA_DR, val);

		// 延迟
	       	delay(DELAY);

		// 设置 GPIO 输出低电平， led 灭
		val = mmio_read_32(GPIO_BASE + GPIO_SWPORTA_DR);
		val &= ~(1 << GPIO_CHANNEL);
		mmio_write_32(GPIO_BASE + GPIO_SWPORTA_DR, val);

		// 延迟
	       	delay(DELAY);

		loop--;
	}
}

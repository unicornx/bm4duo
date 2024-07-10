/*
 * 本实验演示芯片低功耗（Low-Power-Consumption）的使用。
 *
 * SoC 集成了 CPU 核心以及众多 IP 模块，IP 模块包括 GPIO，PWM, I2C 等等。CPU 核心
 * 和这些 IP 模块子系统都支持独立设置进入和退出低功耗。
 *
 * 本实验演示 CPU 核心和 GPIO 模块中第一个 GPIO 控制器 GPIO0(XGPIOA) 的低功耗控
 * 制，其他的 IP 模块的低功耗操作类似。
 *
 * CPU 的睡眠利用 RISC-V ISA 提供的 wfi 指令。具体解释见函数 `cpu_sleep()`的注释。
 *
 * GPIO 外设模块的睡眠利用 reset 机制。具体解释见函数 `gpio0_reset()` 的注释。
 *
 * 本实验利用 Watchdog 定时器超时中断来唤醒 CPU。超时时间设置为 10 秒。具体执行
 * 流程解释如下：
 *
 * Step 1: 系统上电后，GPIO0 正常运行，此时测量 GPIO0 的 #15 通道（外部引脚 GP14,
 * 具体参考 DUO 开发板的引脚图 [1]/[2]）的电流会看到有电流输出。测量时可以使用万
 * 用表，切换至电流档，mA 为单位即可。duo256 上实测电流在大约 28mA。
 *
 * Step 2: 运行 5 秒后程序主动将 GPIO0 和 CPU 设置进入睡眠，此时 GPIO0 处于停止
 * 运行状态，即低功耗状态，此时测量引脚电流应该为 0，串口控制台也无输出打印。
 *
 * Step 3: 虽然 CPU 此时休眠，但外设 Watchdog 仍然正常运行。所以再经过 5 秒后，
 * Watchdog 超时定时器中断发生，该中断会唤醒 CPU。CPU 开始执行中断处理函数，我
 * 们在中断处理函数 `lpc_isr()` 中除了必需的喂狗动作（`wdt_feeddog()`）外，会唤
 * 醒 GPIO0。此时测量引脚电流又会看到电流输出。
 *
 * 以上 Step 1 ~ Step 3 循环执行。
 *
 * 除了直接测量引脚电流外也可以外接 LED，通过查看 LED 亮度来判断 GPIO0 的工作状态。
 *
 * [1]: https://milkv.io/docs/duo/getting-started/duo
 * [2]: https://milkv.io/docs/duo/getting-started/duo256m
 */

#include "common.h"

static void gpio0_init(int channel_no)
{
	// 高电平
	mmio_write_32(GPIO0_BASE + GPIO_SWPORTA_DR, 1 << channel_no);
	// 输出
	mmio_write_32(GPIO0_BASE + GPIO_SWPORTA_DDR, 1 << channel_no);
}

/*
 * @active: RESET_ACTIVE, else RESET_INACTIVE
 *
 * 针对 Reset 系统，TRM 手册上说 active low，即设置为 0 表示 assert，1 表示
 * deassert 所以：
 * 当我们设置 RESET 寄存器 bit 位为 0 时，即将复位信号保持低电平，该子系统或者
 * 模块被 assert，系统或者模块组件被强制停止，不执行任何操作，处于停止状态，
 * 功耗较低。反之如果我们设置寄存器 bit 位为 1，则该子系统或者模块被 deassert，
 * 复位信号被释放，系统开始正常运行，功耗较高。
 */
#define RESET_ACTIVE 1
#define RESET_INACTIVE 0
static void gpio0_reset(int active)
{
	uint32_t val;

	val = mmio_read_32(RSTGEN_BASE + SOFT_RSTN_1);
	// TRM 手册上说 active low, 即设置为 0 表示 assert，1 表示 deassert
	// 这个不要和函数参数 active 的取值搞混淆
	if (active)
		val &= ~(1 << OFFSET_REG_SOFT_RESET_X_GPIO0);
	else
		val |= (1 << OFFSET_REG_SOFT_RESET_X_GPIO0);
	mmio_write_32(RSTGEN_BASE + SOFT_RSTN_1, val);
}

static void gpio0_wakeup()
{
	gpio0_reset(RESET_INACTIVE);

	// GPIO 模块唤醒后原有的配置恢复为默认值，所以需要重新初始化一下
	gpio0_init(15);

	printf("===> GPIO0 is waken up!\n");
}

static void gpio0_sleep()
{
	printf("===> GPIO0 enter sleeping ...\n");

	gpio0_reset(RESET_ACTIVE);
}

extern void wdt_init(int32_t torr, int32_t itorr, int32_t toc);
extern void wdt_feeddog(void);

static int lpc_isr(int intrid, void *priv)
{
	printf("---> Watchdog timer expired!\n");
	printf("===> CPU is waken up, continue ...\n");

	wdt_feeddog();

	gpio0_wakeup();

	return 0;
}

static void cpu_sleep()
{
	/*
	 * 处理器指令架构本身并不会定义低功耗机制，但处理器架构中通常会定义一条
	 * 休眠指令。WFI (Wait For Interrupt）指令是 RISC-V 架构定义的一条休眠指
	 * 令。当处理器执行到 WFI 指令之后，将会停止执行当前的指令流，进入一种空
	 * 闲状态。这种空闲状态可以被称为 “休眠” 态, 直到处理器接收到中断，处理
	 * 器便被唤醒。处理器被唤醒后，如果中断被全局打开 (mstatus 寄存器的 MIE
	 * 域控制），则进入中断异常服务程序开始执行。
	 */
	printf("===> CPU enter sleeping, waiting for interrupt ...\n");
	__asm volatile("wfi");
}

void test_lpc()
{
	int i = 0;

	// 我们设置 Duo 开发板上的 GP15 引脚，这个引脚 Duo 和 Duo256 是相同的。
	// 该引脚默认功能即 GPIO，为保险起见，我们还是设置一下。
	PINMUX_CONFIG(SPK_EN, XGPIOA_15);

	gpio0_init(15);

	request_irq(WDT1_INTR, lpc_isr, 0, "WDT INT", 0);
	// 参考 source/test/test_wdt.c 中的注释，这里的含义是初始超时和后继超时
	// 时长都设定为 10 秒。
	wdt_init(11, 11, 64000);

	while (1) {
		// 每过 1 s 打印一下，方便观测我们 TOP 的工作顺序。
		printf("---> 1s delay\n");
		mdelay(1000);
		i++;
		if (i % 5 == 0) {
			gpio0_sleep();
			cpu_sleep();
			// 中断处理函数退出后，CPU 从睡眠中醒来会走到这里
			printf("Hallelu Yah! I'm alive!\n");
		}
	}
}

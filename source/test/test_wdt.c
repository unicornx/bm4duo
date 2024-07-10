/*
 * 本实验演示 WatchDog 的使用
 */

#include "common.h"

void wdt_feeddog()
{
#if 1
	// 设置 0x76 确保安全地重启 WDT 计数器
	// A restart also clears the WDT interrupt.
	mmio_write_32(WATCHDOG1_BASE + WDT_CRR, 0x76);
#else
	// 只要我们及时 clear 中断，同样可以达到喂狗的效果，虽然我们没有显式地
	// restart/reload 定时器，但超时后 timer 会自动执行该操作
	// 但这种喂狗方式只在中断上下文中有效，而主动通过 WDT_CRR 喂狗可以在
	// 任何场景下发生。
	mmio_read_32(WATCHDOG1_BASE + WDT_EOI);
#endif
	printf("Hallelu Yah! I'm alive!\n");
}

static int wdt_isr(int intrid, void *priv)
{
	printf("---> Watchdog timer expired!\n");

	wdt_feeddog();

	return 0;
}

/*
 * @torr: WDT_TORR.WDT_TORR
 * @itorr: WDT_TORR.WDT_ITORR
 * @toc: WDT_TOC
 * 本 demo 中 WDT_CR.TOR_MODE = WDT_CR.ITOR_MODE 固定为 1
 */
void wdt_init(int32_t torr, int32_t itorr, int32_t toc)
{
#if 0
	// 本段代码注释部分仅仅用于调试，打印了系统初始化阶段一些相关寄存器的内容

	int32_t val;

	// 系统控制器 sys_ctrl_reg.reg_sw_root_reset_en 的 bit[0] 用于控制 watchdog
	// 超时后是否触发系统复位
	// 由于 fsbl（裸机程序执行前执行的一小段初始化代码）目前默认设置了
	// RTC_CTRL0 的 bit[6] 即 hw_wdg_rst_en ，这会导致 watchdog 超时触发 RTC
	// 复位，进而导致系统复位。所以默认不设置 sys_ctrl_reg.reg_sw_root_reset_en
	// 的 bit[0], watchdog 超时后系统依然会复位。
	// mmio_write_32(TOP_BASE + TOP_SYS_CTRL_REG, 0x2);
	val = mmio_read_32(TOP_BASE + TOP_SYS_CTRL_REG);
	printf("TOP_SYS_CTRL_REG is 0x%x\n", val);
  
	// 系统控制器 top_wdt_ctrl
	// reg_wdt_rst_sys_en: [2:0]: 111, enable wdt0 ~ wdt2 to reset system
	// reg_wdt_rst_cpu_en: [6:4]: 111, enable wdt0~wdt2 to reset cpu
	// reg_wdt_clk_sel [10:8]:   0: xtal clock; 1: 32k clock
	val = mmio_read_32(TOP_BASE + TOP_TOP_WDT_CTRL);
	printf("TOP_TOP_WDT_CTRL is 0x%x\n", val);
	// mmio_write_32(TOP_BASE + TOP_TOP_WDT_CTRL, 0x77);
	// 自己实验效果，裸机阶段默认读出来是 0x7，也就是已经 enable wdt0~wdt2 to reset system

	val = mmio_read_32(WATCHDOG1_BASE + WDT_CR);
	printf("WDT_CR is 0x%x\n", val);
#endif

	// WDT_TORR/Timeout range register
	// WDT_TORR 寄存器上保存了两个 TimeOut Period 值, 简称 TOP，
	// 一个是 WDT_TORR.WDT_TORR（TOP）
	// 一个是 WDT_TORR.WDT_ITORR(Initial TOP）。
	// 发现第一次计数的超时时间取决于 WDT_TORR.WDT_ITORR，此后计数的超时时间
	// 取决于 WDT_TORR.WDT_TORR
	//
	// 注意这两个 TOP 的取值计算方法
	// WDT_TORR.WDT_TORR 分两种 mode，mode 的值由寄存器 WDT_CR.TOR_MODE 决定
	// WDT_TORR.WDT_ITORR 也分两种 mode，mode 的值由另外一个寄存器 WDT_CR.ITOR_MODE 决定
	// 大致的思路是，以 TOP 为例，ITOP 类似。
	// 如果 mode 为 0，则 T = 2^(16 + WDT_TORR)，
	// 如果 mode 为 1，则 T = WDT_TOC << (WDT_TORR + 1)
	// 也就是说 mode 为 0 时，T 只取决于 WDT_TORR 而且是 64k 的整数倍，最大不会超过 2^(16+15)
	// 如果 mode 为 1，则可以自己设置一个数（由另一个寄存器 WDT_TOC）指定，
	// 并左移 [0, 16] 位，即本身或者乘上 2^16 倍
	//
	mmio_write_32(WATCHDOG1_BASE + WDT_TORR, (itorr & 0xf) << 4 | (torr & 0xf));
  
	// WDT_TOC/Time Out Count
	mmio_write_32(WATCHDOG1_BASE + WDT_TOC, (toc & 0xffff));
  
	// WDT_CR/Control register
	// - [0] WDT enable: WDT 的启动控制位，如果启动了，只有发生 reset 后该位
	//                   才会恢复为 0，此间不接受其他的手动设置为 0。
	//                   这里我们启动 wdt
	// - [1] response mode: 这里我们设置超时两次才重启
	// - [4:2] reset pulse length，即系统 reset 过程中保持 assert 的持续时长，
	//   单位是 pclk 的脉冲周期: 这里我们取 32 个 pclk cycles
	// - [6]: 1 << 6, the mode of timeout period
	// - [7]: 1 << 7, the mode of timeout period for initialization
	mmio_write_32(WATCHDOG1_BASE + WDT_CR,
			WDT_CR_ENABLE |
			WDT_CR_RESPMODE_2PHASE |
			WDT_CR_RESET_PULSE_LEN_32 |
			WDT_CR_TOR_MODE_1 |
			WDT_CR_ITOR_MODE_1);
}

void test_wdt()
{
	request_irq(WDT1_INTR, wdt_isr, 0, "WDT INT", 0);

	// 本测试例子，取值设置如下：
	// WDT_TORR.WDT_TORR = 11
	// WDT_TORR.WDT_ITORR = 10
	// WDT_TOC = 64000
	// 默认为 25Mhz 的时钟
	// 那么对于 T_TOR = WDT_TOC << (WDT_TORR.WDT_TORR + 1) = 64000 << 12, 实际时长为 (64000 << 12) *（1/25M）~= 10s
	// 那么对于 T_ITOR = WDT_TOC << (WDT_TORR.WDT_ITORR + 1) = 64000 << 11, 实际时长为 (6400 << 11) *（1/25M）~= 5s
	// 也就是上电或者 reset 后第一次 TOP 为 5 秒，此后的 TOP 为 10 秒
	// 如果我们在 wdt timer irq handler 中喂狗的话，会看到上电或者复位后第一
	// 次 5s 后中断到，此后 10s 中断到。
	// 如果我们在 wdt timer irq handler 中不喂狗的话，会看到上电或者复位后第
	// 一次 5s 后中断到，再过 10s 后系统复位重启。
	wdt_init(11, 10, 64000);

	while (1) {
		// 每过 1 s 打印一下，方便观测我们 TOP 的工作顺序。
		mdelay(1000);
		printf("---> 1s delay\n");

		// 如果在运行期间喂狗，就不会触发中断
		// 注意，喂狗的实质是 restart/reload timer period, 所以在第一次
		// 超时未到之前强制喂狗，则 T_ITOR 被替换为 T_TOR。
		// mmio_write_32(WATCHDOG1_BASE + WDT_CRR, 0x76);

		// 如果 enale 了，只有发生 reset 后该 WDT_CR.WDT_ENABLE 位才会
		// 恢复为 0，此间不接受其他的手动设置为 0。
		// 所以下面的操作并不会停止定时器计时
		// mmio_write_32(WATCHDOG1_BASE + WDT_CR, 0);
	}
}
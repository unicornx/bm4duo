/*
 * 本实验演示 WatchDog 的使用
 */

#include "common.h"

static int wdt_isr(int intrid, void *priv)
{
	printf("---> wdt!\n");

	// 在两阶段工作模式下，当第一阶段的中断到来时，我们 clear watchdog 中断
	// 后不会启动第二阶段的计数，但是会重启第一阶段的计数，所以第二次第一阶
	// 段计数递减到 0 后中断依然会来。只要我们及时 clear 中断，则不会触发第
	// 二阶段的计数，也就不会发出复位信号。
	// mmio_read_32(WATCHDOG1_BASE + WDT_EOI);

	// 重启第一阶段的计数器，也能达到清中断的效果
	// 设置 0x76 确保安全地重启 WDT 计数器
	// A restart also clears the WDT interrupt.
	mmio_write_32(WATCHDOG1_BASE + WDT_CRR, 0x76);

	return 0;
}

// 
void wdt_init()
{
	int32_t val;

	// 系统控制器 sys_ctrl_req
	// 但是看 TRM，[1:0] 两位是 reserved，设置 0x2 感觉也没有什么作用啊？
	// 看 testcase_rtc_wdt，会设置 0x4，也就是 sys_ctrl_reg.reg_sw_root_reset_en 的 bit0
	//mmio_write_32(TOP_BASE + TOP_SYS_CTRL_REG, 0x2);
	// 我自己实验的效果看，似乎什么也不设置，默认初始值为 0 也能工作，感觉这个似乎不起作用？FIXME	
	val = mmio_read_32(TOP_BASE + TOP_SYS_CTRL_REG);
	printf("TOP_SYS_CTRL_REG is 0x%x\n", val);

  
	// 系统控制器 top_wdt_ctrl
	// reg_wdt_rst_sys_en: [2:0]: 111, enable wdt0 ~ wdt2 to reset system
	// reg_wdt_rst_cpu_en: [6:4]: 111, enable wdt0~wdt2 to reset cpu
	// reg_wdt_clk_sel [10:8]:   0: xtal clock; 1: 32k clock
	val = mmio_read_32(TOP_BASE + TOP_TOP_WDT_CTRL);
	printf("TOP_TOP_WDT_CTRL is 0x%x\n", val);
	//mmio_write_32(TOP_BASE + TOP_TOP_WDT_CTRL, 0x77);
	// 自己实验效果，默认读出来是 7，也就是 enable wdt0~wdt2 to reset system
  
	// WDT_TORR/Timeout range register
	// WDT_TORR 寄存器上保存了两个 TimeOut Period 值, 简称 TOP，
	// 一个是 WDT_TORR.WDT_TORR（TOP），还有一个是 WDT_TORR.WDT_ITORR(Initial TOP）。
	// 我本地实验了一下，发现第一次计数的超时时间取决于 WDT_TORR.WDT_ITORR，
	// 第二次计数的超时时间取决于 WDT_TORR.WDT_TORR
	// WDT_TORR.WDT_TORR [3:0]: b1011/11 
	// WDT_TORR.WDT_ITORR[7:4]: b1011/11 << 4
	// 注意这两个 TOP 的最终值得计算还和另外一个控制 mode 有关
	// WDT_TORR.WDT_TORR 分两种 mode，mode 的值由另外一个寄存器 WDT_CR.TOR_MODE 决定
	// WDT_TORR.WDT_ITORR 也分两种 mode，mode 的值由另外一个寄存器 WDT_CR.ITOR_MODE 决定
	// 大致的思路是，以 TOP 为例，ITOP 类似。如果 mode 为 0，则 T = 2^(16 + WDT_TORR)，
	// 如果 mode 为1， 则 T = WDT_TOC << (WDT_TORR +1)
	// 也就是说 mode 为 0 时，T 只取决于 WDT_TORR 而且是 64k 的整数倍，最大不会超过 2^(16+15)
	// 如果 mode 为 1，则可以自己设置一个数（由另一个寄存器 WDT_TOC）指定，并左移 [0, 16] 位，即本身或者乘上 2^16 倍
	// 这些 mode 值我们后面都会设置为 1，所以根据手册
	// TOR_MODE 设置为 1, 所以 TimeOut Period: T = WDT_TOC <<( WDT_TORR +1) = 61036 << (11+1) = 61036 << 12 = 61036 * 2^12
	// ITOR_MODE 设置为 1，所以 Initial TimeOut Period:  T = WDT_TOC <<( WDT_ITORR +1) = 61036 << (11+1) = 61036 << 12
	
	// 举个例子，假设
	// WDT_CR.TOR_MODE = WDT_CR.ITOR_MODE = 1
	// WDT_TORR.WDT_TORR = 11
	// WDT_TORR.WDT_ITORR = 10
	// WDT_TOC = 61036
	// 默认为 25Mhz 的时钟
	// 那么对于 T_TOR = WDT_TOC << (WDT_TORR +1) = 64000 << 12, 实际时长为 (64000 << 12) *（1/25M）~= 10s
	// 那么对于 T_ITOR = WDT_TOC << (WDT_TORR +1) = 64000 << 11, 实际时长为 (6400 << 11) *（1/25M）~= 5s
	// 也就是第一次超时 5 秒，然后如果不清中断，再过 10秒系统重启
	mmio_write_32(WATCHDOG1_BASE + WDT_TORR, 10 << 4 | 11);
  
	// WDT_TOC/Time Out Count
	mmio_write_32(WATCHDOG1_BASE + WDT_TOC, 64000);
  
	// WDT_CR/Control register
	// WDT_CR[4:0]: b10011/0x13 : 
	// - [4:2] reset pulse length，即系统 reset 过程中保持 assert 的持续时长，单位是 pclk 的脉冲周期: b100/ 32 个 pclk cycles
	// - [1] response mode: 1/ First generate an interrupt and if it is not cleared by the time a second timeout occurs then generate a system reset
	//                      1，超时两次才重启，0，一次超时就重启
	// - [0] WDT enable: 1 / WDT is enabled， WDT 的启动控制位，如果启动了，只有发生 reset 后该位才会恢复为 0，此间不接受其他的手动设置为 0
	// WDT_CR.TOR_MODE  [6]: 1 << 6, 1 the mode of timeout period
	// WDT_CR.ITOR_MODE [7]: 1 << 7, the mode of timeout period for initialization
	//mmio_write_32(WATCHDOG1_BASE + WDT_CR, 0x13 | 0x1 << 7 | 0x1 << 6);
	mmio_write_32(WATCHDOG1_BASE + WDT_CR,
			WDT_CR_ENABLE |
			WDT_CR_RESPMODE_2PHASE |
			WDT_CR_RESET_PULSE_LEN_32 |
			WDT_CR_TOR_MODE_1 |
			WDT_CR_ITOR_MODE_1);
}

void test_wdt()
{
	wdt_init();
	
	request_irq(WDT1_INTR, wdt_isr,0,"WDT INT",0);

	while (1) {
		mdelay(5000);
		printf("---> 5s delay\n");
	}
}
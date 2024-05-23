#include "common.h"

// 本实验演示定时器的使用

// TimerNLoadCount 是一个 buffer，实际计数器开始计数时会将保存在 TimerNLoadCount
// 中的值加载到 Timer 内部实际的计数器中再开始计时。
// 系统 Timer 可选采用外部时钟（xtal），频率为 25MHz 或者内部时钟，频率为 32KHz。
// 具体选择采用系统控制器中的 top_timer_clk_sel
// 默认采用 xtal 25Mhz 的频率。1 次计数的周期为为 40 ns，即硬件精度是 40ns
// 最大的硬件计时范围是：2^32 * 40 / 10^9 ~= 172s，这也意味着如果需要更大的
// 计时范围需要设计软件定时器进行扩展。

// Timer 有以下 2 种计数模式：
// - 自由运行模式: 定时器持续计数，当计数值减到 0 时又自动回转到其最大值，
//   并继续计数。计数长度最大值为 0xFFFFFFFF。
// - 用户定义计数模式: 定时器持续计数，当计数值减到 0 时从 TimerNLoadCount (N=1~8) 
//   寄存器中再次载入初值并继续计数。

// 启动定时器后，定时器第一次 timeout 的时间就是按照 TimerNLoadCount 计算。
// 如果我们没有设置 TimerNLoadCount 的值 或者 TimerNLoadCount 的值为 0，
// 那么定时器立刻到期。
// 如果我们没有 disable 定时器，则此后的行为根据运行模式：
// 如果是自由运行模式，则从第二次开始，计数器值自动转回到最大值 0xffffffff，所以
// 此后的 timeout 为大约 172s
// 如果是用户自定义计数模式，则从第二次开始，计数器从 TimerNLoadCount 中载入初值
// 也就是说如果是用户自定义模式，两次定时之间我们可以修改 TimerNLoadCount 达到
// 改变 timeout

// 本例子演示，
// 采用用户自定义计数模式。
// 首先设置 TimerNLoadCount 为 1s，第一次 1s 到期后设置 TimerNLoadCount 为 2s，
// 第二次 2s 到期后设置 TimerNLoadCount 为 3s。
// 依次类推，5 次到期后停止定时器结束演示。

typedef int (*timer_callback_t)(void *);
struct timer {
	uint8_t timer_id;
	struct timer_reg *root_regs;
	struct timerN_regs *timern_regs;
	int irq;
};

struct timer g_timer[MAX_TIMER_ID] = {0};

// 清中断
static inline void timer_eoi(struct timer *timer)
{
	struct timerN_regs *timern_regs = timer->timern_regs;
	mmio_read_32((uintptr_t)&timern_regs->TimerNEOI);
}

// 设置 TimerNControlReg 的 bit[0] 为 1，enable TimerN
#define MODE_FREERUNNING	0
#define MODE_USERDEFINED	1
static inline void timer_setmode(struct timer *timer, uint32_t mode)
{
	struct timerN_regs *timern_regs = timer->timern_regs;
	uint32_t val;
	val = mmio_read_32((uintptr_t)&timern_regs->TimerNControlReg);
	if (mode)
		val |= 1 << 1;
	else
		val &= ~(1 << 1);
	
	mmio_write_32((uintptr_t)&timern_regs->TimerNControlReg, val);
}

// @enable: 1: enable, 0, disable
static inline void timer_enable(struct timer *timer, int enable)
{
	struct timerN_regs *timern_regs = timer->timern_regs;
	uint32_t val;
	val = mmio_read_32((uintptr_t)&timern_regs->TimerNControlReg);
	if (enable)
		val |= 1;
	else
		val &= ~1;
	mmio_write_32((uintptr_t)&timern_regs->TimerNControlReg, val);
}

#define NS_PER_TICK		40
// top: Time Out Period, 单位 us
static inline void timer_set_top(struct timer *timer, uint64_t top)
{
	// FIXME：严格来说，我们需要对传入的 top 值控制一下范围，这里简化了没有判断

	struct timerN_regs *timern_regs = timer->timern_regs;

	uint64_t timeout_ns = top * 1000;
	uint32_t ticks = timeout_ns / NS_PER_TICK;
	printf("      prepare next timeout = %lu(ns), tick = %u\n", timeout_ns, ticks);

	mmio_write_32((uintptr_t)&timern_regs->TimerNLoadCount, ticks);
}

// 读取 timerN 的中断状态
// bit[0]: 1 表示中断发生，默认为 0
static inline uint32_t timer_get_intstatus(struct timer *timer)
{
	return mmio_read_32((uintptr_t)&timer->timern_regs->TimerNIntStatus);
}

#define SECOND (1000 * 1000 * 1000)
int g_count;
u32 g_time_old;
u32 g_time_new;
u32 g_time_elapsed;

static int timer_irq_handler(int irq, void *arg)
{
	struct timer *timer = arg;

	if (!(timer_get_intstatus(timer) & 0x1)) {
		//no interrupt generate
		return 0;
	}

	// clear interrupt first
	timer_eoi(timer);

	g_time_new = clint_timer_meter_get_ms();
	g_time_elapsed = g_time_new - g_time_old;
	g_time_old = g_time_new;
	printf("----> IRQ: %dth timeout, elapsed time is %d(ms)\n", g_count, g_time_elapsed);
	
	if (g_count < 5) {
		g_count++;
		timer_set_top(timer, (g_count + 1) * 1000 * 1000);
	} else {
		g_count = 0;
		timer_enable(timer, 0);
	}

	return 0;
}

struct timer * timer_init(uint8_t timer_id)
{
	struct timer *timer;

	//printf("enter timer_init\n");
	if (timer_id >= MAX_TIMER_ID) {
		printf("timer_id error\n");
		return NULL;
	}

	// 0：使用 xtal 25MHz， 1：使用内部 32KHz
	// FIXME：这个可能不需要设置，默认是 0x00
	mmio_write_32(TOP_BASE + TOP_TOP_TIMER_CLK_SEL, 0x00);

	timer = &g_timer[timer_id];
	timer->timer_id = timer_id;
	timer->irq = TIMER_IRQ(timer_id);
	timer->root_regs = (struct timer_reg *)REG_TIMER_BASE;
	timer->timern_regs = &timer->root_regs->timern_regs[timer_id];

	timer_enable(timer, 0);

	timer_setmode(timer, MODE_USERDEFINED);

	// 注册定时器中断，并传入 timer 作为 arg
	request_irq(TIMER_IRQ(timer_id), timer_irq_handler, 0, "timer_irq", timer);

	return timer;
}

void test_timer()
{
	struct timer *timer = NULL;

	// 利用 clint 提供的定时器作为标准衡量我们的 timer 是否准确
	// 具体见 timer_irq_handler 中 clint_timer_meter_get_ms 的用法。
	clint_timer_meter_start();

	timer = timer_init(TIMER_ID0);
	if (!timer) {
		printf("timer_init failed\n");
		return;
	}

	g_count = 1;
	
	// 设置第一次 timeout period
	timer_set_top(timer, g_count * 1000 * 1000);
	
	// 启动定时器, 开始第一次计时
	timer_enable(timer, 1);
	g_time_old = clint_timer_meter_get_ms();

	// 这里要提前设置下一次 (第二次) 的 timeout period
	// 如果在 irq handler 中再设置太晚了，
	// 因为在用户自定义计数模式下，当 timeout，也就是计数值减到 0 时，
	// Timer 会自动从 TimerNLoadCount 中再次载入下一次的 timeout 值
	// 所以如果我们希望第二次的 timeout 是 2s，那么这里就要提前设置好，否则
	// 第二次的 timeout period 还是 1s
	timer_set_top(timer, (g_count + 1) * 1000 * 1000);
	
	// 等待定时器结束
	while (g_count);
}

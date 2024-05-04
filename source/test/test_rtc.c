/*
 * 本实验演示 RTC 的使用
 *
 * test_rtc 演示 RTC 的计时功能
 * 先通过 RTC 获取当前时间 t1
 * 延迟一段时间 T
 * 再通过 RTC 获取当前时间 t2
 * 验证 t2 - t1 == T
 *
 * test_rtc_irq 演示 RTC 的 alarm 功能
 * 设置闹钟，timeout 后产生 alarm 中断
 */

#include "common.h"

// 粗调
static void rtc_32k_coarse_val_calib()
{
	uint32_t analog_calib_value = 0;
	uint32_t fc_coarse_time1 = 0;
	uint32_t fc_coarse_time2 = 0;
	uint32_t fc_coarse_value = 0;
	uint32_t offset = 128;
	uint32_t value = 0;
	int count = 0;

	// 设置 RTC_ANA_CALIB.RTC_ANA_SEL_FTUNE 为 0
	// Select 32K OSC calibration value source 为 Controlled by register
	value = mmio_read_32(RTC_CORE_REG_BASE + RTC_ANA_CALIB);
	value &= ~(1 << 31);
	mmio_write_32(RTC_CORE_REG_BASE + RTC_ANA_CALIB, value);
	// 加些延迟确保上述设置生效
	udelay(200);

	// 设置 RTC_SEC_PULSE_GEN.RTC_SEL_SEC_PULSE 为 0
	// Select second pulse signal source 为 signal generated internally
	// Select 32K OSC tuning value source from rtc_sys
	value = mmio_read_32(RTC_CORE_REG_BASE + RTC_SEC_PULSE_GEN) & ~(1 << 31);
	mmio_write_32(RTC_CORE_REG_BASE + RTC_SEC_PULSE_GEN, value);

	analog_calib_value = mmio_read_32(RTC_CORE_REG_BASE + RTC_ANA_CALIB);
	printf("RTC_ANA_CALIB: %d\n", analog_calib_value);

	while (1) {
		printf("----> count = %d\n", ++count);

		// 设置 fc_coarse_en.fc_coarse_en 为 1
		// enable 32K coarse tuning
		mmio_write_32(RTC_CTRL_REG_BASE + RTC_FC_COARSE_EN, 1);

		// 读取 fc_coarse_cal.fc_coarse_time, fc_coarse_cal 的高 16 位
		// 表示的是粗调完成的次数
		fc_coarse_time1 = mmio_read_32(RTC_CTRL_REG_BASE + RTC_FC_COARSE_CAL);
		fc_coarse_time1 >>= 16;
		printf("fc_coarse_time1 = %d\n", fc_coarse_time1);
		printf("fc_coarse_time2 = %d\n", fc_coarse_time2);

		// 确保完成了一次粗调才继续，
		// 从 log 看，我理解这个 fc_coarse_cal.fc_coarse_time 的值一直在增加
		// 只有当下一次读数 fc_coarse_time2 大于上一次读数 fc_coarse_time1 时
		// 才说明至少一次粗调完成了
		while (fc_coarse_time2 <= fc_coarse_time1) {
			fc_coarse_time2 = mmio_read_32(RTC_CTRL_REG_BASE + RTC_FC_COARSE_CAL);
			fc_coarse_time2 >>= 16;
			printf("fc_coarse_time2 = %d\n", fc_coarse_time2);
		}
		printf("loop done!\n");

		// TRM 上说此时要关闭粗调，但是我试了不行，反正后面还是要继续，所以就不关了
		//mmio_write_32(RTC_CTRL_REG_BASE + RTC_FC_COARSE_EN, 0);
		//udelay(400);

		// 读取 fc_coarse_cal.fc_coarse_value, fc_coarse_cal 的低 16 位
		// 我理解所谓的校准，就是利用高精度的相对准确的 25Mhz 的外部时钟，
		// 去采样内部 32k 时钟的一个周期。
		// 如果 32k 时钟准确的话，fc_coarse_value = 25000000/32768 ~= 763
		// 如果是 +-1% 的误差
		// 763 / 0.99 = 770
		// 763 / 1.01 = 755
		// 所以得到这两个上下限，所以我们校准的算法就是：
		// 尝试调整配置寄存器 RTC_ANA_CALIB，加快或减慢 32KHz 振荡时钟周期时间
		// 以提高 32KHz 时钟精准度。调整的结果就是看 fc_coarse_value 的值
		// 使其满足在 [755, 770] 之间。
		// 具体尝试的方法采用二分法，快速逼近。
		// 但是在实际测试中我发现，上限 770 很容易达到，导致校准不充分。
		// 我将上限减小了点为 765，效果会好一点，大概 7 次左右可以完成校准
		fc_coarse_value = mmio_read_32(RTC_CTRL_REG_BASE + RTC_FC_COARSE_CAL);
		fc_coarse_value &= 0xFFFF;
		printf("fc_coarse_value = %d\n", fc_coarse_value);

		//if (fc_coarse_value > 770) {
		if (fc_coarse_value > 765) {
			analog_calib_value += offset;
			offset >>= 1;
			mmio_write_32(RTC_CORE_REG_BASE + RTC_ANA_CALIB, analog_calib_value);
		} else if (fc_coarse_value < 755) {
			analog_calib_value -= offset;
			offset >>= 1;
			mmio_write_32(RTC_CORE_REG_BASE + RTC_ANA_CALIB, analog_calib_value);
		} else {
			mmio_write_32(RTC_CTRL_REG_BASE + RTC_FC_COARSE_EN, 0);
			printf("RTC coarse calib done\n");
			break;
		}
		if (offset == 0) {
			printf("RTC calib failed\n");
			break;
		}
		printf("RTC_ANA_CALIB: %d\n", analog_calib_value);

		// 加些延迟确保上述设置生效
		udelay(200);
	}
}

// 细调
static void rtc_32k_fine_val_calib()
{
	uint32_t fc_fine_time1 = 0;
	uint32_t fc_fine_time2 = 0;
	uint32_t fc_fine_value = 0;
	uint64_t freq = 256000000000;
	uint32_t sec_cnt;
	//uint32_t frac_ext = 10000;
	uint32_t value;

	// 配置寄存器 RTC_SEL_SEC_PULSE 为 0。
	// Select 32K OSC tuning value source from rtc_sys
	value = mmio_read_32(RTC_CORE_REG_BASE + RTC_SEC_PULSE_GEN) & ~(1 << 31);
	mmio_write_32(RTC_CORE_REG_BASE + RTC_SEC_PULSE_GEN, value);
	// 配置 RTC_FC_FINE_EN 为 1
	mmio_write_32(RTC_CTRL_REG_BASE + RTC_FC_FINE_EN, 1);
	// 开始细调

	// 轮询 RTC_FC_FINE_TIME 的值，直到大于前一次读取值, 说明细调完成
	fc_fine_time1 = mmio_read_32(RTC_CTRL_REG_BASE + RTC_FC_FINE_CAL);
	fc_fine_time1 >>= 24;
	printf("fc_fine_time1 = %d\n", fc_fine_time1);

	while (fc_fine_time2 <= fc_fine_time1) {
		fc_fine_time2 = mmio_read_32(RTC_CTRL_REG_BASE + RTC_FC_FINE_CAL);
		fc_fine_time2 >>= 24;
		printf("fc_fine_time2 = %d\n", fc_fine_time2);
	}

	// 读取 RTC_FC_FINE_VALUE，取得 25MHz 时钟采样 256 个 32KHz 时钟周期的计数值
	fc_fine_value = mmio_read_32(RTC_CTRL_REG_BASE + RTC_FC_FINE_CAL);
	fc_fine_value &= 0xFFFFFF;
	printf("fc_fine_value = %d\n", fc_fine_value);

	// 32KHz 时钟频率可由以下算式取得：
	// Frequency = 256 / (RTC_FC_FINE_VALUE x 40ns)
	// 下面的计算为了避免浮点数计算，将 part1 提前扩大 10000 倍
	freq = 256000000000 * 250 / fc_fine_value;
	printf("freq = %ld\n", freq);

	int32_t part1 = freq / 10000;
	int32_t part2 = (freq - part1 * 10000) * 256 / 10000;
	printf("part1 = %d, part2 = %d\n", part1, part2);

	// 假设计算结果是 32768.4194357
	// 取整数部份 32768，写入寄存器 RTC_SEC_PULSE_GEN.RTC_SEC_PULSE_GEN_INT,
	// 小数部份取 8-bit = 0.4194357 x 256 = 107，写入寄存器 RTC_SEC_PULSE_GEN.RTC_SEC_PULSE_GEN_FRAC
	// 注意这两部分只占了 RTC_SEC_PULSE_GEN 的 [23：0], 所以为了不影响其他位
	// 我们需要先读 RTC_SEC_PULSE_GEN，再或上
	value = mmio_read_32(RTC_CORE_REG_BASE + RTC_SEC_PULSE_GEN);
	sec_cnt = part1 << 8 | part2;
	value |= sec_cnt;
	printf("---> value = 0x%x\n", value);
	mmio_write_32(RTC_CORE_REG_BASE + RTC_SEC_PULSE_GEN, value);

	// 配置 RTC_FC_FINE_EN 为 0，结束细调
	mmio_write_32(RTC_CTRL_REG_BASE + RTC_FC_FINE_EN, 0);
}

#define SET_SEC_CNTR_VAL_INIT       (1 << 28 | 1 << 29)

static void rtc_enable_sec_counter(uint32_t sec)
{
	uint32_t val;
	uint32_t sec_ro_t;

	/* select inner sec pulse and select reg set calibration val */
	val = mmio_read_32(RTC_CORE_REG_BASE + RTC_SEC_PULSE_GEN);
	val &= ~(1 << 31);
	mmio_write_32(RTC_CORE_REG_BASE + RTC_SEC_PULSE_GEN, val);

	/* load from MACRO register */
	// 我们设置的 sec 可能比实际的时间值 sec_ro_t 要旧，如果旧了就
	// 采用 sec_ro_t。 sec_ro_t 好像是 no-die 域的时间值 FIXME
	sec_ro_t = mmio_read_32(RTC_MACRO_REG_BASE + RTC_MACRO_RO_T);
	printf("sec_ro_t is 0x%x\n", sec_ro_t);
	if (sec_ro_t > sec)
		sec = sec_ro_t;
	printf("sec is 0x%x\n", sec);

	// 写入时间，但这里 RTC_SET_SEC_CNTR_VALUE 可以认为只是一个 buffer
	// 需要后面对 RTC_SET_SEC_CNTR_TRIG 写入 1 触发其加载到 RTC 中
	mmio_write_32(RTC_CORE_REG_BASE + RTC_SET_SEC_CNTR_VALUE, sec);

	// 将 RTC_SET_SEC_CNTR_VALUE 中的时间加载到 RTC 中，此时 RTC 才
	// 开始工作
	mmio_write_32(RTC_CORE_REG_BASE + RTC_SET_SEC_CNTR_TRIG, 1);

	// 轮询 RTC_SEC_CNTR_VALUE 寄存器值，直到读取值等于
	// RTC_SET_SEC_CNTR_VALUE 值，说明 RTC 开始正常工作。
	// 原因是 RTC 启动工作需要一段时间，这里我们同步一下。
	while (1) {
		val = mmio_read_32(RTC_CORE_REG_BASE + RTC_SEC_CNTR_VALUE);
		printf("sec read is 0x%x\n", sec);
		if (val >= sec)
			break;
	}
}

static struct tm *get_current_time()
{
	uint32_t val;
	time_t val64;
	val = mmio_read_32(RTC_CORE_REG_BASE + RTC_SEC_CNTR_VALUE);
	// NOTE: time_t is 64-bits on rv64, don't force type convertion
	// may lost and hightest is not zero
	val64 = val;
	printf("====> sec read  = %ld\n", val64);

	return localtime(&val64);
}

void rtc_init()
{
	// FIXME: 参考 TRM，校准模块使用 25MHz 晶振时钟采样 32KHz 时钟
	// 看了一下电路图，存在 25Mhz，

	// 我们可以使用外部晶振 XTAL 或者内部晶振来实现及时。
	// Duo 的电路默认采用了内部晶振。
	// 实时时钟 RTC (Real Time Clock) 内部包含一个 32KHz 的振荡器，但是这个
	// 振荡器容易受外部环境影响导致精度不高。
	// 为此在外部搭配了一个 25MHz 的晶振，当使用内部晶振时，我们可以通过
	// 程序控制来对其进行校准。

	// 通过读取 RTC_CTRL.reg_rtc_mode, 我们可以知道 RTC 实际使用的 Clock Source
	// 如果是内部源 OSC32K，则我们需要对齐进行校准。
	uint32_t val = 0;
	val = mmio_read_32(RTC_CTRL_REG_BASE + RTC_CTRL);
	printf("rtc_ctrl = 0x%x\n", val);
	if (val & (1 << 10))
		printf("XTAL32K\n");
	else {
		printf("OSC32K\n");

		// 校准分两个阶段
		// 第一个阶段称之为粗调（coarse tune），第二个阶段称为细调（fine tune）
		rtc_32k_coarse_val_calib();

		rtc_32k_fine_val_calib();
	}

	time_t seconds;
	struct tm info;

	info.tm_year = 2024 - 1900;
	info.tm_mon = 3 - 1;
	info.tm_mday = 18;
	info.tm_hour = 0;
	info.tm_min = 0;
	info.tm_sec = 1;
	info.tm_isdst = -1;

	seconds = mktime(&info);
	if( seconds == -1 )
		printf("Error: unable to make time using mktime\n");
	else
		printf("seconds is 0x%x\n", (int)seconds);

	
	rtc_enable_sec_counter(seconds);

	// alarm is disabled by default

	printf("rtc init done!\n");
}

void test_rtc()
{
	rtc_init();

	struct tm *t;
	t = get_current_time();
	if (NULL == t) 
		printf("get current time failed\n");
	else
		printf("y:m:d:h:m:s = %d:%d:%d:%d:%d:%d\n",
			t->tm_year + 1900,
			t->tm_mon + 1,
			t->tm_mday,
			t->tm_hour,
			t->tm_min,
			t->tm_sec);

	// 这里采用死循环耗时来达到延时的目的
	// 可以通过墙上时钟观察两次打印的时间估计延时时长并和 RTC 的计时值进行比较
	mdelay(20000);

	t = get_current_time();
	if (NULL == t) 
		printf("get current time failed\n");
	else
		printf("y:m:d:h:m:s = %d:%d:%d:%d:%d:%d\n",
			t->tm_year + 1900,
			t->tm_mon + 1,
			t->tm_mday,
			t->tm_hour,
			t->tm_min,
			t->tm_sec);
}

static void rtc_setup_alarm(uint32_t seconds)
{
	time_t val64;

	/*
	 * Just for demo, we don't guard the input seconds and resulting
	 * seconds input RTC_ALARM_TIME
	 */
	val64 = mmio_read_32(RTC_CORE_REG_BASE + RTC_SEC_CNTR_VALUE);
	val64 += seconds;

	mmio_write_32(RTC_CORE_REG_BASE + RTC_ALARM_TIME, (uint32_t)val64);
}

static inline void rtc_enable_alarm()
{
	mmio_write_32(RTC_CORE_REG_BASE + RTC_ALARM_ENABLE, 1);
}

static inline void rtc_disable_alarm()
{
	mmio_write_32(RTC_CORE_REG_BASE + RTC_ALARM_ENABLE, 0);
}

// 闹钟到期后触发中断
// 在中断处理中通过 RTC 获取当前时间并打印
// 重新设置 3 秒后闹钟到期
// 使能闹钟
static int rtc_isr(int intrid, void *priv)
{
	// 关闭闹钟
	rtc_disable_alarm();

	struct tm *t;
	t = get_current_time();
	if (NULL == t) 
		printf("get current time failed\n");
	else
		printf("y:m:d:h:m:s = %d:%d:%d:%d:%d:%d\n",
			t->tm_year + 1900,
			t->tm_mon + 1,
			t->tm_mday,
			t->tm_hour,
			t->tm_min,
			t->tm_sec);
	
	// 从第二次开始闹钟周期变为 3 s
	rtc_setup_alarm(3);

	rtc_enable_alarm();

	return 0;
}

void test_rtc_irq()
{
	rtc_init();

	request_irq(RTC_ALARM_O, rtc_isr, 0, "RTC Alarm", 0);

	struct tm *t;
	t = get_current_time();
	if (NULL == t)
		printf("get current time failed\n");
	else
		printf("y:m:d:h:m:s = %d:%d:%d:%d:%d:%d\n",
			t->tm_year + 1900,
			t->tm_mon + 1,
			t->tm_mday,
			t->tm_hour,
			t->tm_min,
			t->tm_sec);

	// 第一次设置 1 秒后闹钟到期
	rtc_setup_alarm(1);

	// 使能闹钟
	rtc_enable_alarm();
}
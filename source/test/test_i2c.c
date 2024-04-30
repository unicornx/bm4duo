#include "common.h"

// 利用I2C0控制外围电路上支持I2C接口的的温度传感器芯片LM75，实时读取温度值并通过串口COM1打印在控制终端上。

// DUO 上支持 I2C 功能的引脚如下，都不是默认 select，需要自己 pinmux select：
// I2C0_SCL/I2C0_SDA -- GPIOA28/GPIOA29
//
// I2C1_SCL/I2C1_SDA -- GPIO19/GPIO20
// I2C1_SCL/I2C1_SDA -- GPIO21/GPIO18
// I2C1_SCL/I2C1_SDA -- GPIOC9/GPIOC10
//
// I2C3_SCL/I2C3_SDA -- GPIO23/GPIO22


#if 0
void i2c_set_pinmux(uint32_t i2c_base)
{
	if (i2c_base == I2C0_BASE) {
		mmio_clrsetbits_32(PINMUX_BASE + 0x9C, 0x3<<20, 0x1<<20); /* IIC0_SDA */
		mmio_clrsetbits_32(PINMUX_BASE + 0xA0, 0x3<<4, 0x1<<4); /* IIC0_SCL */
	} else if (i2c_base == I2C1_BASE) {
		mmio_clrsetbits_32(PINMUX_BASE + 0xA0, 0x3<<20, 0x1<<20); /* IIC1_SDA */
		mmio_clrsetbits_32(PINMUX_BASE + 0xA4, 0x3<<4, 0x1<<4); /* IIC1_SCL */
	} else if (i2c_base == I2C2_BASE) {
		mmio_clrsetbits_32(PINMUX_BASE + 0xA4, 0x3<<20, 0x1<<20); /* IIC2_SDA */
		mmio_clrsetbits_32(PINMUX_BASE + 0xA8, 0x3<<4, 0x1<<4); /* IIC2_SCL */
	} else if (i2c_base == I2C3_BASE) {
		mmio_clrsetbits_32(PINMUX_BASE + 0xA8, 0x3<<20, 0x1<<20); /* IIC3_SDA */
		mmio_clrsetbits_32(PINMUX_BASE + 0xAC, 0x3<<4, 0x1<<4); /* IIC3_SCL */
	} else if (i2c_base == I2C4_BASE) {
		mmio_clrsetbits_32(PINMUX_BASE + 0xAC, 0x3<<20, 0x1<<20); /* IIC4_SDA */
		mmio_clrsetbits_32(PINMUX_BASE + 0xB0, 0x3<<4, 0x1<<4); /* IIC4_SCL */
	}
}
#endif

// CLK_DIV CRG 寄存器章节，配置 clk_byp_0.clk_byp_0_31 可以选择 IIC_CLK 为 25MHz 预设时钟源或 100MHz 时钟源。
// FIXME：clk_byp_0_31 默认为 1，Clock Bypass to xtal for IIC’ s clock clk_i2c，具体的值是 25Mhz 还是 100Mhz？
// 我现在理解 1 表示不适用外部时钟源，所以 1 表示适用默认的预设时钟源 25MHz

// 配置相关时序配置需要在模块不使能状态。 需将 IC_ENABLE 设 置 为 0, 并 查 询 IC_ENABLE_STATUS[0] 确定为 0
// FIXME：我看这里只是设置了 IC_ENABLE，但没有严格同步查询确保生效

// 参照表格 I2C clock selection and related register configuration relationship table，按 I2C 时钟选择，
// 配置 I2C时序计数寄存器。
// 参考配置 I2C 时钟与时序参数

void i2c_set_clk_in_khz(uint32_t i2c_base, uint32_t khz)
{
	uint8_t hcnt, lcnt;

	if (khz > 400) {
#if 0
		mmio_write_32(i2c_base + I2C_CON, 0x67);
		hcnt = 0.4 * 14900 / khz;
		lcnt = 0.6 * 14900 / khz;
		mmio_write_32(i2c_base + I2C_HS_HCNT, hcnt);
		mmio_write_32(i2c_base + I2C_HS_LCNT, lcnt);
#endif
	} else {
		// FIXME: 有些不太理解这里的一些设置
		// 0x65 = 01100101
		// I2C_CON
		// .MASTER_MODE [0] = 1, master mode
		// .SPEED[2:1] = 2, fast mode (~400 kb/s)  // FIXME 这里为啥是 2，而且 reset 为 3 表示啥意思？按照 TRM 上非 DMA 方式下的传输说明，不是应该设置为 1 吗
		// .IC_10BITADDR_SLAVE[3] = 0, disable 10bit slave address mode
		// .IC_10BITADDR_MASTER[4] = 0, disable 10bit master address mode
		// .IC_RESTART_EN[5] = 1, enable I2C master to be able generate restart
		// .IC_SLAVE_DISABLE[6] = 1, slave is disabled
		mmio_write_32(i2c_base + I2C_CON, 0x65);
		hcnt = 0.4 * 20000 / khz;
		lcnt = 0.6 * 20000 / khz;
		//hcnt = 21;
		//lcnt = 42;
		mmio_write_32(i2c_base + I2C_FS_SCL_HCNT, hcnt);
		mmio_write_32(i2c_base + I2C_FS_SCL_LCNT, lcnt);
	}
	// printf("hcnt = 0x%x\n", hcnt);
	// printf("lcnt = 0x%x\n", lcnt);
}

// id 为 target 设备的地址
// 整个流程可以参考 TRM 的 非 DMA 方式下的数据传输的软件流程图
void i2c_init(uint32_t i2c_base, uint8_t id)
{
	//i2c_set_pinmux(i2c_base);
	
	// 先关闭 I2C
	mmio_write_32(i2c_base + I2C_ENABLE, 0);

	// 设置对端 slave 的地址，支持标准地址 (7bit) 和扩展地址 (10bit)。
	// 这里采用标准地址
	mmio_write_32(i2c_base + I2C_TAR, id);

	// 选择较低速率
	i2c_set_clk_in_khz(i2c_base, 200);

	// FIXME: 这里是 enable 了还是 disable？
	// 猜测是 enable，对 Mask 设置 1 才是屏蔽掉对应的中断，这里设置为 0 就是不屏蔽
	// 所有的中断，也就是 enable
	mmio_write_32(i2c_base + I2C_INTR_MASK, 0);

	// 设置 RX/TX 的 FIFO 的 Threshold Level，即上报中断的阈值
	mmio_write_32(i2c_base + I2C_RX_TL, 1);
	mmio_write_32(i2c_base + I2C_TX_TL, 1);

	// 启动 I2C
	mmio_write_32(i2c_base + I2C_ENABLE, 1);
}

// 通过轮询中断状态查看 TX FIFO 是否已经为空，说明数据已经发送
// FIXME， 也可以试试通过轮询查看 I2C_STATUS.ST_TFE
void wait_until_tx_empty(uint32_t i2c_base)
{
	uint16_t timeout = 1000;
	uint32_t sts;

	sts = mmio_read_32(i2c_base + I2C_RAW_INTR_STAS);
	printf("--> sts = 0x%x\n", sts);

	while (1) {
		udelay(90);	// somehow delay for FPGA bus speed
		uint32_t sts = mmio_read_32(i2c_base + I2C_RAW_INTR_STAS);
		if (sts & IST_TX_EMPTY) {
			printf("--> wait_until_tx_empty done!\n");
			break;
		}
		if (--timeout == 0) {
			printf("wait_until_tx_empty timeout\n");
			break;
		}
	}
}

// 通过轮询状态寄存器 I2C_STATUS.ST_RFNE 查看 RX FIFO 是否有数据，如果有则说明可以读取
// FIXME， I2C_RAW_INTR_STAS 中似乎没有符合我们类似要求的标志，
// I2C_RAW_INTR_STAS.IST_RX_UNDER 表示当我们读取导致 FIFO 读空
// I2C_RAW_INTR_STAS.IST_RX_OVER 表示 RX FIFO 溢出
// I2C_RAW_INTR_STAS.IST_RX_FULL 表示 RX FIFO 中的数据大小达到我们设置的上限

int wait_until_rx_not_empty(uint32_t i2c_base)
{
	uint16_t timeout = 1000;
	uint32_t sts;

	sts = mmio_read_32(i2c_base + I2C_RAW_INTR_STAS);
	printf("--> sts = 0x%x\n", sts);
#if 0
	while (1) {
		uint32_t sts = mmio_read_32(i2c_base + I2C_STATUS);
		if (sts & ST_RFNE)
			break;
		if (--timeout == 0) {
			printf("wait_until_rx_not_empty timeout\n");
			return -1;
		}
	}
#endif
	while (1) {
		udelay(90);	// somehow delay for FPGA bus speed
		sts = mmio_read_32(i2c_base + I2C_RAW_INTR_STAS);
		printf("sts = 0x%x\n", sts);
		if (sts & IST_RX_FULL) {
			printf("--> wait_until_rx_not_empty done!\n");
			break;
		}
		if (--timeout == 0) {
			printf("wait_until_rx_not_empty timeout\n");
			break;
		}
	}

	return 0;
}

/*
 * Refer to UM Page-665, Figure 8.2-6 Operations for Master/Transmitter Mode
 * Input:
 *	addr_s: address of slave device
 *	buf:	pointer to buffer holding the data to be transferred
 *	count:	size of bytes to be transferred
 * Output:
 *	N/A
 * Return:
 *	none	 
 */
void i2c_master_tx(uint32_t i2c_base, unsigned char *buf, int count)
{
	uint32_t val;
	while (count > 0) {
		val = *buf; // 其他bit默认为 0
		mmio_write_32(i2c_base + I2C_DATA_CMD, val);

		// wait for tx fifo empty
		wait_until_tx_empty(i2c_base);
		
		count--;
		buf++;
	}
	
	// FIXME 不写 stop？
	mmio_write_32(i2c_base + I2C_DATA_CMD, 1 << 9);
}


int i2c_master_rx(uint32_t i2c_base, unsigned char *buf, int count)
{
	int nr = 0; /* return the number of read */
	
	if (!buf) return nr;
	
	while (count > 0) {
		// trigger read
		mmio_write_16(i2c_base + I2C_DATA_CMD, 0x100);

		// wait and read rx fifo
		if (0 == wait_until_rx_not_empty(i2c_base)) {
			*buf = mmio_read_8(i2c_base + I2C_DATA_CMD);
			count--;
			buf++;
			nr++;
		} else
			return -1;
	} 
	
	return nr;
}

void b2d(unsigned char b, char *s);
void i2c_delay(int count);

char str[4] = {0};

void test_i2c()
{
	uint8_t buf[2];
	uint32_t val;

	pinmux_config(PINMUX_I2C0);

	i2c_init(I2C0_BASE, 0x48);


	val = mmio_read_32(I2C0_BASE + I2C_RAW_INTR_STAS);
	printf("--> interrupt status = 0x%x\n", val);

	while (1) {
		/* 
		 * set LM75 pointer register to point to 
		 * temperature read-only register
		 */
		buf[0] = 0;
		i2c_master_tx(I2C0_BASE, buf, 1);
		
		/* read temperature value and print to COM1 */
		if (2 != i2c_master_rx(I2C0_BASE, buf, 2)) {
			printf("read temp failed!\n");
			break;
		}

		printf("temprature = ");
		if (buf[0] & 0x80) {
			printf("-");
		} else {
			printf("+");
		}
		b2d(buf[0] & 0x07f, str);
		printf("%s", str);
		if (buf[1] & 0x80) {
			printf(".5 C\r");
		} else {
			printf(".0 C\r");
		}
		
		i2c_delay(0x200000);
	}
}


typedef struct {
	unsigned char	num;
	char	*str;
} digit_str;

digit_str array_1[10] = {
	{0,	"0"},
	{1,	"1"},
	{2,	"2"},
	{3,	"3"},
	{4,	"4"},
	{5,	"5"},
	{6,	"6"},
	{7,	"7"},
	{8,	"8"},
	{9,	"9"}
};

digit_str array_2[9] = {
	{10,	"10"},
	{20,	"20"},
	{30,	"30"},
	{40,	"40"},
	{50,	"50"},
	{60,	"60"},
	{70,	"70"},
	{80,	"80"},
	{90,	"90"}
};

digit_str array_3[2] = {
	{100,	"100"},
	{200,	"200"}
};

void b2d(unsigned char b, char *s)
{
	char *p, *p1, *p2;
	int i;
	
	p = s;
	
	for (i = 1; i >= 0; i--) {
		if (b >= array_3[i].num) {
			p1 = p;
			p2 = array_3[i].str;
			while (*p2) {
				*p1 = *p2;
				p1++;
				p2++;
			}
			*p1 = '\0';
			
			p++;
			b -= array_3[i].num;
			
			break;
		}
	}
	
	for (i = 8; i >= 0; i--) {
		if (b >= array_2[i].num) {
			p1 = p;
			p2 = array_2[i].str;
			while (*p2) {
				*p1 = *p2;
				p1++;
				p2++;
			}
			*p1 = '\0';
			
			p++;
			b -= array_2[i].num;
			
			break;
		}
	}
	
	for (i = 9; i >= 0; i--) {
		if (b >= array_1[i].num) {
			p1 = p;
			p2 = array_1[i].str;
			while (*p2) {
				*p1 = *p2;
				p1++;
				p2++;
			}
			*p1 = '\0';
			
			break;
		}
	}
}


void i2c_delay(int count)
{
	while (count-- > 0);
}
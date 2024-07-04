// 利用I2C0控制外围电路上支持I2C接口的的温度传感器芯片LM75，实时读取温度值并通过串口COM1打印在控制终端上。

// DUO 上支持 I2C 功能的引脚如下，都不是默认 select，需要自己 pinmux select：
// GP0/GP1   -- I2C0_SCL/I2C0_SDA -- XGPIOA28/XGPIOA29
// GP4/GP5   -- I2C1_SCL/I2C1_SDA -- PWR_GPIO19/PWR_GPIO20 == SD1_D2/SD1_D1
// GP7/GP6   -- I2C3_SCL/I2C3_SDA -- PWR_GPIO22/PWR_GPIO23
// GP9/GP8   -- I2C1_SCL/I2C1_SDA -- PWR_GPIO18/PWR_GPIO21 == SD1_D3/SD1_D0
// GP11/GP10 -- I2C1_SCL/I2C1_SDA -- XGPIOC10/XGPIOC9      == PAD_MIPIRX0N/PAD_MIPIRX1P

// Duo256 上支持 I2C 的引脚如下：
// GP4/GP5   -- I2C1_SCL/I2C1_SDA -- PWR_GPIO19/PWR_GPIO20 == SD1_D2/SD1_D1
// GP7/GP6   -- I2C3_SCL/I2C3_SDA -- PWR_GPIO22/PWR_GPIO23
// GP9/GP8   -- I2C1_SCL/I2C1_SDA -- PWR_GPIO18/PWR_GPIO21 == SD1_D3/SD1_D0
// GP11/GP10 -- I2C2_SCL/I2C2_SDA -- XGPIOC15/XGPIOC14

#include "common.h"

/*
 * status codes
 */
#define STATUS_IDLE			0x0
#define STATUS_WRITE_IN_PROGRESS	0x1
#define STATUS_READ_IN_PROGRESS		0x2

#define I2C0                    0x0
#define I2C1                    0x1
#define I2C2                    0x2
#define I2C3                    0x3
#define I2C4                    0x4

static struct i2c_regs *i2c_get_base(uint8_t i2c_id)
{
    struct i2c_regs *i2c_base = NULL;

    switch (i2c_id) {
    case I2C0:
        i2c_base = (struct i2c_regs *)I2C0_BASE;
        break;
    case I2C1:
        i2c_base = (struct i2c_regs *)I2C1_BASE;
        break;
    case I2C2:
        i2c_base = (struct i2c_regs *)I2C2_BASE;
        break;
    case I2C3:
        i2c_base = (struct i2c_regs *)I2C3_BASE;
        break;
    case I2C4:
        i2c_base = (struct i2c_regs *)I2C4_BASE;
        break;
    }

    return i2c_base;
}

static void i2c_enable(struct i2c_regs *i2c, unsigned int enable)
{
	uint32_t ena = enable ? IC_ENABLE : 0;
	int timeout = 100;

	do {
		mmio_write_32((uintptr_t)&i2c->ic_enable, ena);
		if ((mmio_read_32((uintptr_t)&i2c->ic_enable_status) & IC_ENABLE) == ena)
			return;

		/*
		 * Wait 10 times the signaling period of the highest I2C
		 * transfer supported by the driver (for 400KHz this is
		 * 25us) as described in the DesignWare I2C databook.
		 */
		udelay(25);
	} while (timeout--);

	printf("timeout in %sabling I2C adapter\n", enable ? "en" : "dis");
}

/*
 * i2c_flush_rxfifo - Flushes the i2c RX FIFO
 *
 * Flushes the i2c RX FIFO
 */
static void i2c_flush_rxfifo(struct i2c_regs *i2c)
{
	while (mmio_read_32((uintptr_t)&i2c->ic_status) & IC_STATUS_RFNE)
		mmio_read_32((uintptr_t)&i2c->ic_cmd_data);
}

/*
 * i2c_wait_for_bb - Waits for bus busy
 *
 * Waits for bus busy
 */
static int i2c_wait_for_bb(struct i2c_regs *i2c)
{
	uint16_t    timeout = 0;

	// Master FSM is not in IDLE state OR
	// Transmit FIFO is not empty
	while ((mmio_read_32((uintptr_t)&i2c->ic_status) & IC_STATUS_MA) ||
	       !(mmio_read_32((uintptr_t)&i2c->ic_status) & IC_STATUS_TFE)) {

		/* Evaluate timeout */
		udelay(5);
		timeout++;
		if (timeout > 200) /* exceed 1 ms */
			return 1;
	}

	return 0;
}

/*
 * i2c_setaddress - Sets the target slave address
 * @i2c_addr:	target i2c address
 *
 * Sets the target slave address.
 */
static void i2c_setaddress(struct i2c_regs *i2c, uint16_t i2c_addr)
{
	i2c_enable(i2c, 0);
	mmio_write_32((uintptr_t)&i2c->ic_tar, i2c_addr & 0x3ff);
	i2c_enable(i2c, 1);
}

// dev 是 slave 设备的 I2C 地址
// addr 是 slave 设备中寄存器的地址
static int i2c_xfer_init(struct i2c_regs *i2c, uint16_t dev, uint8_t addr)
{
	if (i2c_wait_for_bb(i2c))
		return 1;

	i2c_setaddress(i2c, dev);

	mmio_write_32((uintptr_t)&i2c->ic_cmd_data, addr | BIT_I2C_CMD_DATA_RESTART_BIT);

	return 0;
}

static int i2c_xfer_finish(struct i2c_regs *i2c)
{
	uint16_t timeout = 0;
	while (1) {
		if ((mmio_read_32((uintptr_t)&i2c->ic_raw_intr_stat) & IC_STOP_DET)) {
			mmio_read_32((uintptr_t)&i2c->ic_clr_stop_det);
			break;
		} else {
			timeout++;
			udelay(5);
			if (timeout > I2C_STOPDET_TO * 100) {
				printf("%s, tiemout\n", __func__);
				break;
			}
		}
	}

	if (i2c_wait_for_bb(i2c))
		return 1;

	i2c_flush_rxfifo(i2c);

	return 0;
}

/*
 * Read from slave memory.
 * @i2c_id: i2c controller id
 * @dev: slave device address
 * @addr: slave register address to read from, 8bit width
 * @buffer: buffer to store the data read
 * @len: number of bytes to be read
 */
int i2c_read(uint8_t i2c_id, uint8_t dev, uint8_t addr, uint8_t *buffer, uint16_t len)
{
	struct i2c_regs *i2c;
	int ret = 0;
	uint32_t val = 0;
	uint16_t size;

	i2c = i2c_get_base(i2c_id);

	i2c_enable(i2c, 1);

	if (i2c_xfer_init(i2c, dev, addr))
		return 1;

	size = len;
	while (size) {
		if (1 == size ) {
			mmio_write_32((uintptr_t)&i2c->ic_cmd_data, BIT_I2C_CMD_DATA_READ_BIT | BIT_I2C_CMD_DATA_STOP_BIT);
		} else {
			mmio_write_32((uintptr_t)&i2c->ic_cmd_data, BIT_I2C_CMD_DATA_READ_BIT);
		}
		size--;
	}

	size = len;
	while (size) {
		while (1) {
			val = mmio_read_32((uintptr_t)&i2c->ic_raw_intr_stat);
			if (val & BIT_I2C_INT_RX_FULL) {
				*buffer++ = (uint8_t)mmio_read_32((uintptr_t)&i2c->ic_cmd_data);
				size--;
				break;
			}
			printf("rx fifo not ready!\n");
		}
	}

	ret = i2c_xfer_finish(i2c);

	i2c_enable(i2c, 0);

	return ret;
}

/*
 * i2c_write - Write to i2c memory
 * @chip:	target i2c address
 * @addr:	address to read from
 * @alen:
 * @buffer:	buffer for read data
 * @len:	no of bytes to be read
 *
 * Write to i2c memory.
 */
/*
 * Write to slave memory.
 * @i2c_id: i2c controller id
 * @dev: slave device address
 * @addr: slave register address to read from, 8bit width
 * @buffer: buffer to store the data to be written
 * @len: number of bytes to be written
 */
int i2c_write(uint8_t i2c_id, uint8_t dev, uint8_t addr, uint8_t *buffer, uint16_t len)
{
	struct i2c_regs *i2c;
	int ret = 0;
	i2c = i2c_get_base(i2c_id);

	i2c_enable(i2c, 1);

	if (i2c_xfer_init(i2c, dev, addr))
		return 1;

	while (len) {
		if (i2c->ic_status & IC_STATUS_TFNF) {
			if (--len == 0) {
				mmio_write_32((uintptr_t)&i2c->ic_cmd_data, *buffer | IC_STOP);
			} else {
				mmio_write_32((uintptr_t)&i2c->ic_cmd_data, *buffer);
			}
			buffer++;
        } else
			printf("len=%d, ic status is not TFNF\n", len);
	}

	ret = i2c_xfer_finish(i2c);
	
	i2c_enable(i2c, 0);
	
	return ret;
}

/*
 * hal_i2c_set_bus_speed - Set the i2c speed
 * @speed:	required i2c speed
 *
 * Set the i2c speed.
 */
static void i2c_set_bus_speed(struct i2c_regs *i2c, unsigned int speed)
{
	unsigned int cntl;
	unsigned int hcnt, lcnt;
	int i2c_spd;

	if (speed > I2C_FAST_SPEED)
		i2c_spd = IC_SPEED_MODE_MAX;
	else if ((speed <= I2C_FAST_SPEED) && (speed > I2C_STANDARD_SPEED))
		i2c_spd = IC_SPEED_MODE_FAST;
	else
		i2c_spd = IC_SPEED_MODE_STANDARD;

	/* to set speed cltr must be disabled */
	i2c_enable(i2c, 0);

	cntl = (mmio_read_32((uintptr_t)&i2c->ic_con) & (~IC_CON_SPD_MSK));

	switch (i2c_spd) {
	case IC_SPEED_MODE_MAX:
		cntl |= IC_CON_SPD_HS;
			//hcnt = (u16)(((IC_CLK * MIN_HS100pF_SCL_HIGHTIME) / 1000) - 8);
			/* 7 = 6+1 == MIN LEN +IC_FS_SPKLEN */
			//lcnt = (u16)(((IC_CLK * MIN_HS100pF_SCL_LOWTIME) / 1000) - 1);
			hcnt = 6;
			lcnt = 8;

		mmio_write_32((uintptr_t)&i2c->ic_hs_scl_hcnt, hcnt);
		mmio_write_32((uintptr_t)&i2c->ic_hs_scl_lcnt, lcnt);
		break;

	case IC_SPEED_MODE_STANDARD:
		cntl |= IC_CON_SPD_SS;

		hcnt = (uint16_t)(((IC_CLK * MIN_SS_SCL_HIGHTIME) / 1000) - 7);
		lcnt = (uint16_t)(((IC_CLK * MIN_SS_SCL_LOWTIME) / 1000) - 1);

		mmio_write_32((uintptr_t)&i2c->ic_ss_scl_hcnt, hcnt);
		mmio_write_32((uintptr_t)&i2c->ic_ss_scl_lcnt, lcnt);
		break;

	case IC_SPEED_MODE_FAST:
	default:
		cntl |= IC_CON_SPD_FS;
		hcnt = (uint16_t)(((IC_CLK * MIN_FS_SCL_HIGHTIME) / 1000) - 7);
		lcnt = (uint16_t)(((IC_CLK * MIN_FS_SCL_LOWTIME) / 1000) - 1);

		mmio_write_32((uintptr_t)&i2c->ic_fs_scl_hcnt, hcnt);
		mmio_write_32((uintptr_t)&i2c->ic_fs_scl_lcnt, lcnt);
		break;
	}

	mmio_write_32((uintptr_t)&i2c->ic_con, cntl);

	/* Enable back i2c now speed set */
	i2c_enable(i2c, 1);
}

/*
 * __hal_i2c_init - Init function
 * @speed:	required i2c speed
 * @slaveaddr:	slave address for the device
 *
 * Initialization function.
 */
void i2c_init(uint8_t i2c_id)
{
	struct i2c_regs *i2c;

	i2c = i2c_get_base(i2c_id);

	/* Disable i2c */
	i2c_enable(i2c, 0);

	mmio_write_32((uintptr_t)&i2c->ic_con, (IC_CON_SD | IC_CON_SPD_FS | IC_CON_MM | IC_CON_RE));
	mmio_write_32((uintptr_t)&i2c->ic_rx_tl, IC_RX_TL);
	mmio_write_32((uintptr_t)&i2c->ic_tx_tl, IC_TX_TL);
	mmio_write_32((uintptr_t)&i2c->ic_intr_mask, 0x0);

	i2c_set_bus_speed(i2c, I2C_SPEED);

	/* Enable i2c */
	i2c_enable(i2c, 1);
}

// 如果统一起见，可以都使用 GP4/GP5 上对应的 I2C1
// 但我的 Duo 上的 GP4/GP5 貌似坏了，包括 PWM5 也工作不太正常
// 所以
// 在 Duo 上我选择了 I2C0
// 在 Duo256 上我还是选择 I2C1
#if   defined(CONFIG_BOARD_DUO)
#define TEST_PINMUX_I2C PINMUX_I2C0
#define TEST_I2C	I2C0
#elif defined(CONFIG_BOARD_DUO256)
#define TEST_PINMUX_I2C PINMUX_I2C1
#define TEST_I2C	I2C1
#endif

void test_i2c()
{
	uint8_t data[2] = {0xff, 0xff};
	uint16_t temp;
	float temp_calculated;

	pinmux_config(TEST_PINMUX_I2C);

	i2c_init(TEST_I2C);

	for(;;) {
		i2c_read(TEST_I2C, 0x48, 0x00, &data[0], 2);
		printf("---> read data 0x%x, 0x%x\n", data[0], data[1]);

		temp = (data[0] << 3) | (data[1] >> 5);
		temp_calculated = temp * 0.125;
		printf("LM75: temperature is %2.2fc\n", temp_calculated);

		mdelay(1000);
	}
}

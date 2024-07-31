// 本实验演示 I2C slave 的使用
//
// 本实验仅在 Duo 256m 上实验通过。
//
// 采用 IIC2 为 Master，IIC3 为 Slave
// 具体实验时，将 IIC2 和 IIC3 的 SDA 和 SCL 引脚两两互联即可。对应 Duo 256m
// 的连线方式，参考 https://milkv.io/docs/duo/getting-started/duo256m
// GP6 <----> GP10
// GP7 <----> GP11
// 主程序中循环 master 循环读取 slave，每次读一个字节并打印出来。
// slave 的初值为 0，每读一次，slave 的值加 1

#include "common.h"

#define IIC_SLAVE_ADDR 0x33

static int i2c_slave_data; // 未初始化，初值为 0

/*
 * 对主设备的读操作响应通过中断完成。
 * 对于一个单字节的从设备发送器，具体响应操作为：
 * 1. 向 IC_DATA_CMD[0:7] 写入需要发送的数据，IC_DATA_CMD[8] 写 0
 * 2. 清除 RD_REQ 中断
 */
static int  i2c_slave_isr(int intrid, void *priv)
{
	struct i2c_regs *i2c = i2c_get_base(I2C3);

	mmio_write_32((uintptr_t)&i2c->ic_cmd_data, i2c_slave_data++);
	mmio_read_32((uintptr_t)&i2c->ic_clr_intr);

	return 0;
}

static void i2c_slave_init(uint8_t i2c_id)
{
	struct i2c_regs *i2c = i2c_get_base(i2c_id);

	mmio_write_32((uintptr_t)&i2c->ic_enable, 0);
	mmio_write_32((uintptr_t)&i2c->ic_sar, IIC_SLAVE_ADDR);
	mmio_write_32((uintptr_t)&i2c->ic_con, 0x24);
	mmio_write_32((uintptr_t)&i2c->ic_fs_scl_hcnt, 40);
	mmio_write_32((uintptr_t)&i2c->ic_fs_scl_lcnt, 60);
	mmio_write_32((uintptr_t)&i2c->ic_intr_mask, 0x60); /* 取消一些中断屏蔽 */
	mmio_write_32((uintptr_t)&i2c->ic_rx_tl, 4);
	mmio_write_32((uintptr_t)&i2c->ic_tx_tl, 1);
	mmio_write_32((uintptr_t)&i2c->ic_enable, 1);

	request_irq(I2C3_INTR, i2c_slave_isr, 0, "IIC INTR", 0);
}

void test_i2c_slave()
{
	uint8_t data;

	// IIC2
	PINMUX_CONFIG(PAD_MIPI_TXM1, IIC2_SDA);
	PINMUX_CONFIG(PAD_MIPI_TXP1, IIC2_SCL);
	// IIC3
	PINMUX_CONFIG(SD1_CLK, IIC3_SDA);
	PINMUX_CONFIG(SD1_CMD, IIC3_SCL);

	i2c_init(I2C2);

	i2c_slave_init(I2C3);

	for(;;) {
		i2c_read(I2C2, IIC_SLAVE_ADDR, 0x00, &data, 1);

		printf("---> read data 0x%x\n", data);

		mdelay(1000);
	}
}

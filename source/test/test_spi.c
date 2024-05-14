// 演示 SPI 通讯
// 使用 Loop 方式，无需连接真实的 SPI slave 设备

#include "common.h"

#define SPI_BASE SPI2_BASE

#define SPI_WRITE(offset, value) \
	mmio_write_32(SPI_BASE + offset , (u32)value)

#define SPI_READ(offset) \
	mmio_read_32(SPI_BASE + offset)

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

uint8_t default_tx[] = {
	0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
	0x07, 0x08, 0x09, 0x10, 0x11, 0x12,
};
uint8_t default_rx[ARRAY_SIZE(default_tx)] = {0, };

struct dw_spi {
	/* Current message transfer state info */
	u16	len;
	void	*tx;
	void	*tx_end;
	void	*rx;
	void	*rx_end;
	u8	n_bytes;
};

static void dw_spi_enable(uint32_t enable)
{
    if (enable == 1)
		SPI_WRITE(SPIENR, 1);
	else
        SPI_WRITE(SPIENR, 0);

	udelay(5);

	//printf("%sabling SPI\n", enable ? "En" : "Dis");
}

void spi_init()
{
	uint32_t ctrl0 = 0;

	dw_spi_enable(0);
	ctrl0 = SPI_READ(CTRL0);
	// clear fields to be set
	ctrl0 &= ~(SPI_TMOD_MASK |  // [9:8] Transfer Mode
		   SPI_SCPOL_MASK | // [7] Serial Clock Polarity
		   SPI_SCPH_MASK |  // [6] Serial Clock Phase
		   SPI_FRF_MASK |   // [5:4] Frame Format
		   SPI_DFS_MASK);   // [3:0] Data Frame Size

	/* Set to SPI frame format */
	ctrl0 |= SPI_SRL |          // Loop Test
		 SPI_TMOD_TR |      // Transfer Mode：Transmit & Receive
		 SPI_SCPOL_LOW |    // Serial Clock Polarity： Inactive state of serial clock is low
		 SPI_SCPH_MIDDLE |  // Serial Clock Phase：Serial clock toggles in middle of first data bit
		 SPI_FRF_SPI |      // Frame Format：Motorola SPI
		 SPI_DFS_8BIT;      // Data Frame Size：8-bit serial data transfer
	SPI_WRITE(CTRL0, ctrl0);

	SPI_WRITE(BAUDR, SPI_BAUDR_DIV);
	SPI_WRITE(TXFTLR, 4);
	SPI_WRITE(RXFTLR, 4);
	SPI_WRITE(SER, 0x1); /* enable slave 1 device*/

	dw_spi_enable(1);
}

static int spi_loopback(struct dw_spi *dws)
{
	u32 max = dws->len;
	u16 txw = 0;
	u16 rxw = 0;
	u32 val;

	clint_timer_meter_start();
	while (max > 0) {
		/* Set the tx word if the transfer's original "tx" is not null */
		val = mmio_read_32(SPI_BASE + RISR);
		if ((val & SPI_RISR_TXOIR) != SPI_RISR_TXOIR) {
			clint_timer_meter_start();

			if (dws->n_bytes == 1)
				txw = *(u8 *)(dws->tx);
			else
				txw = *(u16 *)(dws->tx);
			SPI_WRITE(DR, txw);
			dws->tx += dws->n_bytes;
			udelay(1);

			rxw = SPI_READ(DR);

			if (dws->n_bytes == 1)
				*(u8 *)(dws->rx) = rxw;
			else
				*(u16 *)(dws->rx) = rxw;

			dws->rx += dws->n_bytes;
			max--;
		} else if (clint_timer_meter_get_ms() < 100) {
			udelay(10); /* wait FIFO exit overflow status */
		} else {
			printf("SPI TX timeout\n");
			return 1;
		}
	}
	return 0;
}

void test_spi()
{
	struct dw_spi dws;
	u8 idx = 0;

	dws.tx = (void *)default_tx;
	dws.len = ARRAY_SIZE(default_tx);
	dws.tx_end = dws.tx + dws.len;
	dws.rx = (void *)default_rx;
	dws.rx_end = dws.rx + dws.len;
	dws.n_bytes = 1; /* test 8 bits data */

	pinmux_config(PINMUX_SPI2);

	spi_init();

	printf("TX data:");
	for (idx=0; idx < dws.len; idx++)
		printf("0x%02X ", default_tx[idx]);
	printf("\n\n");

	spi_loopback(&dws);

	printf("RX data:");
	for(idx=0; idx < dws.len; idx++)
		printf("0x%02X ", default_rx[idx]);
	printf("\n\n");
}

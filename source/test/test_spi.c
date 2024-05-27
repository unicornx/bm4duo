// 本实验演示 SPI 接口
// 如果定义 LOOP 为 1 则演示 SPI 的 loop test 功能，数据简单地回环，发送到 TX FIFO 后再从自己的 RX FIFO 中读出来。
// 如果定义 LOOP 为 0 则演示通过 SPI 接口访问采用 SSD1306 芯片驱动的 OLED 屏幕，显式简单的字符阵列。

#include "common.h"

#define OLED_SPI_BASE   SPI2_BASE

#define SPI_WRITE(offset, value) \
	mmio_write_32(OLED_SPI_BASE + offset , (u32)value)

#define SPI_READ(offset) \
	mmio_read_32(OLED_SPI_BASE + offset)

static void spi_enable(uint32_t enable)
{
	if (enable == 1)
		SPI_WRITE(SPIENR, 1);
	else
        	SPI_WRITE(SPIENR, 0);

	udelay(5);

	//printf("%sabling SPI\n", enable ? "En" : "Dis");
}

void spi_init(int looptest)
{
	uint32_t ctrl0 = 0;

	pinmux_config(PINMUX_SPI2);

	spi_enable(0);// diable

	ctrl0 = SPI_READ(CTRL0);
	// clear fields to be set
	ctrl0 &= ~(SPI_TMOD_MASK |  // [9:8] Transfer Mode
		   SPI_SCPOL_MASK | // [7] Serial Clock Polarity
		   SPI_SCPH_MASK |  // [6] Serial Clock Phase
		   SPI_FRF_MASK |   // [5:4] Frame Format
		   SPI_DFS_MASK);   // [3:0] Data Frame Size
	/* Set to SPI frame format */
	ctrl0 |= SPI_TMOD_TR |      // Transfer Mode：Transmit & Receive
		 SPI_SCPOL_LOW |    // Serial Clock Polarity： Inactive state of serial clock is low
		 SPI_SCPH_MIDDLE |  // Serial Clock Phase：Serial clock toggles in middle of first data bit
		 SPI_FRF_SPI |      // Frame Format：Motorola SPI
		 SPI_DFS_8BIT;      // Data Frame Size：8-bit serial data transfer
	if (looptest)
		ctrl0 |= SPI_SRL;
	SPI_WRITE(CTRL0, ctrl0);

	// 设置SPI_SCK
	SPI_WRITE(BAUDR, SPI_BAUDR_DIV);

	SPI_WRITE(TXFTLR, 4);
	SPI_WRITE(RXFTLR, 4);

	// 片选从设备
	SPI_WRITE(SER, 0x1);

	spi_enable(1);
}

static inline void spi_write(uint8_t buf)
{
	uint32_t val;

	mmio_write_8(OLED_SPI_BASE + DR, buf);

	// 等待直到 TX FIFO 被清空
	val = SPI_READ(TXFLR) & 0xf;
	while (val) {
		udelay(1);
		val = SPI_READ(TXFLR) & 0xf;
	}
}

static inline void spi_read(uint8_t *buf)
{
	uint32_t val;

	// 等待直到 RX FIFO 有数据可读
	val = SPI_READ(RXFLR) & 0xf;
	while (!val) {
		udelay(1);
		val = SPI_READ(RXFLR) & 0xf;
	}
	*buf = mmio_read_8(OLED_SPI_BASE + DR);
}

/* OLED filed */
#define OLED_CMD    0x0
#define OLED_DATA   0x1

#define OLED_RESET  0x0
#define OLED_WORK   0x1

// 为方便连接，我们对 SSD1306 的 RES/DC 引脚在 Duo 和 Duo256 上都使用 GP10 和 GP11
// Duo256/SG2002
// - RES/GP10 选择 PAD_MIPI_TXM1 的 XGPIOC[14]
// - DC/GP11 选择 PAD_MIPI_TXP1 的 XGPIOC[15]
// Duo/CV1800B：
// - RES/GP10 选择 PAD_MIPIRX1P 的 XGPIOC[9]
// - DC/GP11 选择 PAD_MIPIRX0N 的 XGPIOC[10]

#if defined(CONFIG_BOARD_DUO256)

	#define OLED_GPIO_BASE		GPIOC_BASE
	#define GPIO_RES_PIN		(1 << 14)
	#define GPIO_DC_PIN		(1 << 15)

#elif defined(CONFIG_BOARD_DUO)

	#define OLED_GPIO_BASE		GPIOC_BASE
	#define GPIO_RES_PIN		(1 << 9)
	#define GPIO_DC_PIN		(1 << 10)

#else
	#error "Unsupported board type!"
#endif

void oled_set_pinmux(void)
{
#if defined(CONFIG_BOARD_DUO256)	
	PINMUX_CONFIG(PAD_MIPI_TXM1, XGPIOC_14);
	PINMUX_CONFIG(PAD_MIPI_TXP1, XGPIOC_15);
#elif defined(CONFIG_BOARD_DUO)
	PINMUX_CONFIG(PAD_MIPIRX1P, XGPIOC_9);
	PINMUX_CONFIG(PAD_MIPIRX0N, XGPIOC_10);
#else
	#error "Unsupported board type!"
#endif

}

void oled_gpio_init(void)
{
	// 将GPIOC[9]、GPIOC[10]设置为输出模式
	uint32_t val = mmio_read_32(OLED_GPIO_BASE + GPIO_SWPORTA_DDR);

	val |= (GPIO_DC_PIN | GPIO_RES_PIN);
	mmio_write_32(OLED_GPIO_BASE + GPIO_SWPORTA_DDR, val);
}

void oled_dc_set(uint8_t dc)
{
	uint32_t val = 0;

	if (dc) {
		// 拉高 DC 引脚，传输数据为视为 DATA
		val = mmio_read_32(OLED_GPIO_BASE + GPIO_SWPORTA_DR);
		val |= GPIO_DC_PIN;
		mmio_write_32(OLED_GPIO_BASE + GPIO_SWPORTA_DR, val);
	} else {
		// 拉低 DC 引脚，传输数据被视为 CMD
		val = mmio_read_32(OLED_GPIO_BASE + GPIO_SWPORTA_DR);
		val &= ~GPIO_DC_PIN;
		mmio_write_32(OLED_GPIO_BASE + GPIO_SWPORTA_DR, val);
	}
}

void oled_res_set(uint8_t res)
{
	uint32_t val = 0;

	if (res) {
		// WORK，拉高 RESET 后，ssd1306 进入工作状态
		val = mmio_read_32(OLED_GPIO_BASE + GPIO_SWPORTA_DR);
		val |= GPIO_RES_PIN;
		mmio_write_32(OLED_GPIO_BASE + GPIO_SWPORTA_DR, val);
	} else {
		// RESET， 拉低 RESET 后 ssd1306 复位进入初始化
		val = mmio_read_32(OLED_GPIO_BASE + GPIO_SWPORTA_DR);
		val &= ~GPIO_RES_PIN;
		mmio_write_32(OLED_GPIO_BASE + GPIO_SWPORTA_DR, val);
	}
}

void oled_write(uint8_t buf, uint8_t is_data)
{
	if (is_data)
		oled_dc_set(OLED_DATA);
	else
		oled_dc_set(OLED_CMD);

	spi_write(buf);
}

void oled_set_pos(uint8_t col, uint8_t row)
{
	oled_write(0xB0 + row, OLED_CMD);
	oled_write(((col & 0xf0) >> 4) | 0x10, OLED_CMD);
	oled_write((col & 0x0f) | 0x00, OLED_CMD);
}

void oled_clear(void)
{
	for (int row = 0; row < 8; row++) {
		oled_set_pos(0, row);
		for (int col = 0; col < 128; col++)
			oled_write(0x00, OLED_DATA);
	}
}

void oled_init(void)
{
	oled_set_pinmux();

	oled_gpio_init();

	oled_res_set(OLED_RESET);
	mdelay(1);
	oled_res_set(OLED_WORK);

	oled_write(0xAE, OLED_CMD);
	oled_write(0x40, OLED_CMD);
	oled_write(0xB0, OLED_CMD);
	oled_write(0x10, OLED_CMD);
	oled_write(0x00, OLED_CMD);
	oled_write(0xA1, OLED_CMD);
	oled_write(0xA6, OLED_CMD);
	oled_write(0xA4, OLED_CMD);
	oled_write(0xC8, OLED_CMD);
	oled_write(0xD5, OLED_CMD);
	oled_write(0x80, OLED_CMD);
	oled_write(0x8D, OLED_CMD);
	oled_write(0x14, OLED_CMD);
	oled_write(0x81, OLED_CMD);
	oled_write(0xCF, OLED_CMD);
	oled_write(0xAF, OLED_CMD);

	oled_clear();
	oled_set_pos(0, 0);
}

void oled_draw_bmp(uint8_t x, uint8_t y, uint8_t w, uint8_t h, char *bmp)
{
	for (int row = y; row < y + h; row++) {
		row %= 8;
		oled_set_pos(x, row);
		for (int col = 0; col < w; col++) {
			oled_write(*(bmp + (row - y) * w + col), OLED_DATA);
		}
	}
}

char font_array[][32] = {
	{0x08, 0x04, 0xF3, 0x52, 0x56, 0x52, 0x52, 0x50, 0x54, 0x53, 0x52, 0x56, 0xF2, 0x02, 0x02, 0x00},
	{0x10, 0x10, 0x97, 0x55, 0x3D, 0x15, 0x15, 0x15, 0x15, 0x15, 0xFD, 0x15, 0x17, 0x10, 0x10, 0x00}, /*"算",0*/
	{0x08, 0xCC, 0x4A, 0x49, 0x48, 0x4A, 0xCC, 0x18, 0x00, 0x7F, 0x88, 0x88, 0x84, 0x82, 0xE0, 0x00},
	{0x00, 0xFF, 0x12, 0x12, 0x52, 0x92, 0x7F, 0x00, 0x00, 0x7E, 0x88, 0x88, 0x84, 0x82, 0xE0, 0x00}, /*"能",1*/
	{0x24, 0x24, 0xA4, 0xFE, 0xA3, 0x22, 0x00, 0x22, 0xCC, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00},
	{0x08, 0x06, 0x01, 0xFF, 0x00, 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0xFF, 0x02, 0x02, 0x02, 0x00}, /*"科",2*/
	{0x10, 0x10, 0x10, 0xFF, 0x10, 0x90, 0x08, 0x88, 0x88, 0x88, 0xFF, 0x88, 0x88, 0x88, 0x08, 0x00},
	{0x04, 0x44, 0x82, 0x7F, 0x01, 0x80, 0x80, 0x40, 0x43, 0x2C, 0x10, 0x28, 0x46, 0x81, 0x80, 0x00}, /*"技",3*/
};

void oled_show_font_16(uint8_t x, uint8_t y, uint8_t idx)
{
	uint8_t t = 0;

	oled_set_pos(x, y);
	for (t = 0; t < 32; t++) {
		oled_write(font_array[2 * idx][t], OLED_DATA);
	}
	oled_set_pos(x, y + 1);
	for (t = 0; t < 32; t++) {
		oled_write(font_array[2 * idx + 1][t], OLED_DATA);
	}
}


void test_spi_oled()
{
	spi_init(0);

	oled_init();

	oled_show_font_16(10, 4, 0); // 算
	oled_show_font_16(40, 4, 1); // 能
	oled_show_font_16(70, 4, 2); // 科
	oled_show_font_16(100, 4, 3); // 技
}

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
uint8_t default_tx[] = {
	0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
	0x07, 0x08, 0x09, 0x10, 0x11, 0x12,
};
uint8_t default_rx[ARRAY_SIZE(default_tx)] = {0, };

void test_looptest()
{
	int len = ARRAY_SIZE(default_tx);
	int i;

	printf("----> Loop Testing!");

	spi_init(1);

	printf("TX data:");
	for (i = 0; i < len; i++)
		printf("0x%02X ", default_tx[i]);
	printf("\n\n");

	for (int i = 0; i < ARRAY_SIZE(default_tx); i++) {
		spi_write(default_tx[i]);
		spi_read(&default_rx[i]);	
	}

	printf("RX data:");
	for(i = 0; i < len; i++)
		printf("0x%02X ", default_rx[i]);
	printf("\n\n");
}

#define LOOP 0

void test_spi()
{
#if (LOOP == 1)
	test_looptest();
#else
	test_spi_oled();
#endif
}

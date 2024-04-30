#include "bits.h"
#include "mmio.h"
#include "mmap.h"
#include "pinmux.h"

#ifdef CONFIG_BOARD_DUO

void pinmux_config(int io_type)
{
	switch(io_type) {
	case PINMUX_UART0:
		// default config
		break;

	case PINMUX_UART1:
		PINMUX_CONFIG(UART0_TX, UART1_TX);
		PINMUX_CONFIG(UART0_RX, UART1_RX);
		break;	

	case PINMUX_UART2:
		PINMUX_CONFIG(IIC0_SCL, UART2_TX);
		PINMUX_CONFIG(IIC0_SDA, UART2_RX);
		break;	

	case PINMUX_I2C0:
		PINMUX_CONFIG(IIC0_SDA, IIC0_SDA);
		PINMUX_CONFIG(IIC0_SCL, IIC0_SCL);
		break;	

	case PINMUX_I2C1:
		PINMUX_CONFIG(SD1_D1, IIC1_SDA);
		PINMUX_CONFIG(SD1_D2, IIC1_SCL);
		break;	

	case PINMUX_I2C3:
		PINMUX_CONFIG(SD1_CMD, IIC3_SCL);
		PINMUX_CONFIG(SD1_CLK, IIC3_SDA);
		break;

	case PINMUX_CAM0:
		PINMUX_CONFIG(SD0_D0, CAM_MCLK1);
		PINMUX_CONFIG(SD0_D3, CAM_MCLK0);
		break;

	case PINMUX_SPI0:
		PINMUX_CONFIG(SD0_CLK, SPI0_SCK);
		PINMUX_CONFIG(SD0_D3, SPI0_CS_X);
		PINMUX_CONFIG(SD0_D0, SPI0_SDI);
		PINMUX_CONFIG(SD0_CMD, SPI0_SDO);
		break;

	// case PINMUX_SPI1:
	// 	PINMUX_CONFIG(RMII0_MDCK, SPI1_SCK);
	// 	PINMUX_CONFIG(RMII0_RXDV, SPI1_CS_X);
	// 	PINMUX_CONFIG(RMII0_REFCLKI, SPI1_SDI);
	// 	PINMUX_CONFIG(RMII0_MDIO, SPI1_SDO);
	// break;
	// case PINMUX_SPI2:
	// 	PINMUX_CONFIG(IIC3_SCL, SPI2_SCK);
	// 	PINMUX_CONFIG(IIC3_SDA, SPI2_CS_X);
	// 	PINMUX_CONFIG(IIC0_SCL, SPI2_SDI);
	// 	PINMUX_CONFIG(IIC0_SDA, SPI2_SDO);
	// break;
	// case PINMUX_SPI3:
	// 	printf ("fix me\n");
	// break;

	case PINMUX_I2S1:
		PINMUX_CONFIG(PAD_AUD_AOUTR, IIS1_DI);
		PINMUX_CONFIG(AUX0, IIS1_MCLK);
		PINMUX_CONFIG(PAD_AUD_AINR_MIC, IIS1_DO);
		PINMUX_CONFIG(PAD_AUD_AINL_MIC, IIS1_BCLK);
		PINMUX_CONFIG(PAD_AUD_AOUTL, IIS1_LRCK);
		break;

	case PINMUX_I2S2:
		PINMUX_CONFIG(PAD_ETH_RXM, IIS2_DI);
		PINMUX_CONFIG(GPIO_RTX, IIS2_MCLK);
		PINMUX_CONFIG(PAD_ETH_RXP, IIS2_DO);
		PINMUX_CONFIG(PAD_ETH_TXM, IIS2_BCLK);
		PINMUX_CONFIG(PAD_ETH_TXP, IIS2_LRCK);
		break;

	case PINMUX_SDIO0:
		PINMUX_CONFIG(SD0_CD, SDIO0_CD);
		PINMUX_CONFIG(SD0_PWR_EN, SDIO0_PWR_EN);
		PINMUX_CONFIG(SD0_CMD, SDIO0_CMD);
		PINMUX_CONFIG(SD0_CLK, SDIO0_CLK);
		PINMUX_CONFIG(SD0_D0, SDIO0_D_0);
		PINMUX_CONFIG(SD0_D1, SDIO0_D_1);
		PINMUX_CONFIG(SD0_D2, SDIO0_D_2);
		PINMUX_CONFIG(SD0_D3, SDIO0_D_3);
		break;

	case PINMUX_SDIO1:
#if defined(SDIO1_1ST_PAD)
		/*
		 * Name            Address            SD1  MIPI
		 * reg_sd1_phy_sel REG_0x300_0294[10] 0x0  0x1
		 */
		mmio_write_32(TOP_BASE + 0x294,
			(mmio_read_32(TOP_BASE + 0x294) & 0xFFFFFBFF));
		PINMUX_CONFIG(SD1_CMD, PWR_SD1_CMD_VO36);
		PINMUX_CONFIG(SD1_CLK, PWR_SD1_CLK_VO37);
		PINMUX_CONFIG(SD1_D0, PWR_SD1_D0_VO35);
		PINMUX_CONFIG(SD1_D1, PWR_SD1_D1_VO34);
		PINMUX_CONFIG(SD1_D2, PWR_SD1_D2_VO33);
		PINMUX_CONFIG(SD1_D3, PWR_SD1_D3_VO32);
#elif defined(SDIO1_2ND_PAD)
		/*
		 * Name            Address            SD1  MIPI
		 * reg_sd1_phy_sel REG_0x300_0294[10] 0x0  0x1
		 */
		mmio_write_32(TOP_BASE + 0x294,
			(mmio_read_32(TOP_BASE + 0x294) & 0xFFFFFBFF) | BIT_MASK(10));
		PINMUX_CONFIG(PAD_MIPI_TXM4, SD1_CLK);
		PINMUX_CONFIG(PAD_MIPI_TXP4, SD1_CMD);
		PINMUX_CONFIG(PAD_MIPI_TXM3, SD1_D0);
		PINMUX_CONFIG(PAD_MIPI_TXP3, SD1_D1);
		PINMUX_CONFIG(PAD_MIPI_TXM2, SD1_D2);
		PINMUX_CONFIG(PAD_MIPI_TXP2, SD1_D3);
#endif
		break;

	case PINMUX_EMMC:
		break;

	case PINMUX_SPI_NOR:
	 	PINMUX_CONFIG(SPINOR_SCK, SPINOR_SCK);
	 	PINMUX_CONFIG(SPINOR_MISO, SPINOR_MISO);
	 	PINMUX_CONFIG(SPINOR_CS_X, SPINOR_CS_X);
	 	PINMUX_CONFIG(SPINOR_MOSI, SPINOR_MOSI);
	 	PINMUX_CONFIG(SPINOR_HOLD_X, SPINOR_HOLD_X);
	 	PINMUX_CONFIG(SPINOR_WP_X, SPINOR_WP_X);
		break;

	 case PINMUX_SPI_NAND:
	 	PINMUX_CONFIG(SPINOR_SCK, SPINAND_CLK);
	 	PINMUX_CONFIG(SPINOR_MISO, SPINAND_MISO);
	 	PINMUX_CONFIG(SPINOR_CS_X, SPINAND_CS);
	 	PINMUX_CONFIG(SPINOR_MOSI, SPINAND_MOSI);
	 	PINMUX_CONFIG(SPINOR_HOLD_X, SPINAND_HOLD);
	 	PINMUX_CONFIG(SPINOR_WP_X, SPINAND_WP);
		break;

	// case PINMUX_CAM0:
	// 	PINMUX_CONFIG(CAM_PD0, CAM_MCLK1);
	// 	PINMUX_CONFIG(CAM_MCLK0, CAM_MCLK0);
	// break;
	// case PINMUX_VI0:
	// 	printf ("fix me\n");
	// break;
	// case PINMUX_VO:
	// 	printf ("fix me\n");
	// break;
	// case PINMUX_PWM1:
	// 	PINMUX_CONFIG(UART1_TX, PWM_4);
	// 	PINMUX_CONFIG(UART1_RX, PWM_5);
	// 	PINMUX_CONFIG(UART1_CTS, PWM_6);
	// 	PINMUX_CONFIG(UART1_RTS, PWM_7);
	// break;

	case PINMUX_SPI_NOR1:
		PINMUX_CONFIG(SD1_D3, PWR_SPINOR1_CS_X);
		PINMUX_CONFIG(SD1_D2, PWR_SPINOR1_HOLD_X);
		PINMUX_CONFIG(SD1_D1, PWR_SPINOR1_WP_X);
		PINMUX_CONFIG(SD1_D0, PWR_SPINOR1_MISO);
		PINMUX_CONFIG(SD1_CMD, PWR_SPINOR1_MOSI);
		PINMUX_CONFIG(SD1_CLK, PWR_SPINOR1_SCK);
		break;

	default:
		break;
	}
}

#endif // CONFIG_BOARD_DUO


#ifdef CONFIG_BOARD_DUO256

void pinmux_config(int io_type)
{
	switch(io_type) {
	case PINMUX_UART0:
		// default config
	break;
	case PINMUX_UART1:
		PINMUX_CONFIG(UART0_TX, UART1_TX);
		PINMUX_CONFIG(UART0_RX, UART1_RX);
		break;
	case PINMUX_UART2:
		PINMUX_CONFIG(UART2_TX, UART2_TX);
		PINMUX_CONFIG(UART2_RX, UART2_RX);
		break;
	case PINMUX_I2C0:
		PINMUX_CONFIG(IIC0_SDA, IIC0_SDA);
		PINMUX_CONFIG(IIC0_SCL, IIC0_SCL);
		break;
	case PINMUX_I2C1:
		PINMUX_CONFIG(SD1_D1, IIC1_SDA);
		PINMUX_CONFIG(SD1_D2, IIC1_SCL);
		break;
	case PINMUX_I2C3:
		PINMUX_CONFIG(IIC3_SCL, IIC3_SCL);
		PINMUX_CONFIG(IIC3_SDA, IIC3_SDA);
		break;
	case PINMUX_CAM0:
		PINMUX_CONFIG(SD0_D0, CAM_MCLK1);
		PINMUX_CONFIG(SD0_D3, CAM_MCLK0);
		break;
	case PINMUX_SPI0:
		PINMUX_CONFIG(SD0_CLK, SPI0_SCK);
		PINMUX_CONFIG(SD0_D3, SPI0_CS_X);
		PINMUX_CONFIG(SD0_D0, SPI0_SDI);
		PINMUX_CONFIG(SD0_CMD, SPI0_SDO);
		break;
	// case PINMUX_SPI1:
	// 	PINMUX_CONFIG(RMII0_MDCK, SPI1_SCK);
	// 	PINMUX_CONFIG(RMII0_RXDV, SPI1_CS_X);
	// 	PINMUX_CONFIG(RMII0_REFCLKI, SPI1_SDI);
	// 	PINMUX_CONFIG(RMII0_MDIO, SPI1_SDO);
	// break;
	// case PINMUX_SPI2:
	// 	PINMUX_CONFIG(IIC3_SCL, SPI2_SCK);
	// 	PINMUX_CONFIG(IIC3_SDA, SPI2_CS_X);
	// 	PINMUX_CONFIG(IIC0_SCL, SPI2_SDI);
	// 	PINMUX_CONFIG(IIC0_SDA, SPI2_SDO);
	// break;
	// case PINMUX_SPI3:
	// 	printf ("fix me\n");
	// break;
	case PINMUX_I2S1:
		PINMUX_CONFIG(PAD_AUD_AOUTR, IIS1_DI);
		PINMUX_CONFIG(AUX0, IIS1_MCLK);
		PINMUX_CONFIG(PAD_AUD_AINR_MIC, IIS1_DO);
		PINMUX_CONFIG(PAD_AUD_AINL_MIC, IIS1_BCLK);
		PINMUX_CONFIG(PAD_AUD_AOUTL, IIS1_LRCK);
		break;
	case PINMUX_I2S2:
		PINMUX_CONFIG(UART2_RX, IIS2_DI);
		PINMUX_CONFIG(IIC2_SDA, IIS2_MCLK);
		PINMUX_CONFIG(UART2_RTS, IIS2_DO);
		PINMUX_CONFIG(UART2_TX, IIS2_BCLK);
		PINMUX_CONFIG(UART2_CTS, IIS2_LRCK);
		break;
	case PINMUX_SDIO0:
		PINMUX_CONFIG(SD0_CD, SDIO0_CD);
		PINMUX_CONFIG(SD0_PWR_EN, SDIO0_PWR_EN);
		PINMUX_CONFIG(SD0_CMD, SDIO0_CMD);
		PINMUX_CONFIG(SD0_CLK, SDIO0_CLK);
		PINMUX_CONFIG(SD0_D0, SDIO0_D_0);
		PINMUX_CONFIG(SD0_D1, SDIO0_D_1);
		PINMUX_CONFIG(SD0_D2, SDIO0_D_2);
		PINMUX_CONFIG(SD0_D3, SDIO0_D_3);
		break;
	case PINMUX_SDIO1:
#if defined(SDIO1_1ST_PAD)
		/*
		 * Name            Address            SD1  MIPI
		 * reg_sd1_phy_sel REG_0x300_0294[10] 0x0  0x1
		 */
		mmio_write_32(TOP_BASE + 0x294,
			(mmio_read_32(TOP_BASE + 0x294) & 0xFFFFFBFF));
		PINMUX_CONFIG(SD1_CMD, PWR_SD1_CMD_VO36);
		PINMUX_CONFIG(SD1_CLK, PWR_SD1_CLK_VO37);
		PINMUX_CONFIG(SD1_D0, PWR_SD1_D0_VO35);
		PINMUX_CONFIG(SD1_D1, PWR_SD1_D1_VO34);
		PINMUX_CONFIG(SD1_D2, PWR_SD1_D2_VO33);
		PINMUX_CONFIG(SD1_D3, PWR_SD1_D3_VO32);
#elif defined(SDIO1_2ND_PAD)
		/*
		 * Name            Address            SD1  MIPI
		 * reg_sd1_phy_sel REG_0x300_0294[10] 0x0  0x1
		 */
		mmio_write_32(TOP_BASE + 0x294,
			(mmio_read_32(TOP_BASE + 0x294) & 0xFFFFFBFF) | BIT(10));
		PINMUX_CONFIG(PAD_MIPI_TXM4, SD1_CLK);
		PINMUX_CONFIG(PAD_MIPI_TXP4, SD1_CMD);
		PINMUX_CONFIG(PAD_MIPI_TXM3, SD1_D0);
		PINMUX_CONFIG(PAD_MIPI_TXP3, SD1_D1);
		PINMUX_CONFIG(PAD_MIPI_TXM2, SD1_D2);
		PINMUX_CONFIG(PAD_MIPI_TXP2, SD1_D3);
#endif
		break;
	case PINMUX_EMMC:
	 	PINMUX_CONFIG(EMMC_CLK, EMMC_CLK);
	 	PINMUX_CONFIG(EMMC_RSTN, EMMC_RSTN);
	 	PINMUX_CONFIG(EMMC_CMD, EMMC_CMD);
	 	PINMUX_CONFIG(EMMC_DAT1, EMMC_DAT_1);
	 	PINMUX_CONFIG(EMMC_DAT0, EMMC_DAT_0);
	 	PINMUX_CONFIG(EMMC_DAT2, EMMC_DAT_2);
	 	PINMUX_CONFIG(EMMC_DAT3, EMMC_DAT_3);
		break;
	case PINMUX_SPI_NOR:
	 	PINMUX_CONFIG(EMMC_CLK, SPINOR_SCK);
	 	PINMUX_CONFIG(EMMC_CMD, SPINOR_MISO);
	 	PINMUX_CONFIG(EMMC_DAT1, SPINOR_CS_X);
	 	PINMUX_CONFIG(EMMC_DAT0, SPINOR_MOSI);
	 	PINMUX_CONFIG(EMMC_DAT2, SPINOR_HOLD_X);
	 	PINMUX_CONFIG(EMMC_DAT3, SPINOR_WP_X);
		break;
	 case PINMUX_SPI_NAND:
	 	PINMUX_CONFIG(EMMC_CLK, SPINAND_CLK);
	 	PINMUX_CONFIG(EMMC_CMD, SPINAND_MISO);
	 	PINMUX_CONFIG(EMMC_DAT1, SPINAND_CS);
	 	PINMUX_CONFIG(EMMC_DAT0, SPINAND_MOSI);
	 	PINMUX_CONFIG(EMMC_DAT2, SPINAND_HOLD);
	 	PINMUX_CONFIG(EMMC_DAT3, SPINAND_WP);
		break;
	// case PINMUX_CAM0:
	// 	PINMUX_CONFIG(CAM_PD0, CAM_MCLK1);
	// 	PINMUX_CONFIG(CAM_MCLK0, CAM_MCLK0);
	// break;
	// case PINMUX_VI0:
	// 	printf ("fix me\n");
	// break;
	case PINMUX_VO:

#if 1//PINMUX for FRD Panel
		PINMUX_CONFIG(PAD_MIPI_TXP2, VO_CLK0);//data_2:6
		PINMUX_CONFIG(VIVO_CLK, VO_CLK1); //3

		PINMUX_CONFIG(PAD_MIPI_TXM2, VO_D_0); //data_3:7
		PINMUX_CONFIG(PAD_MIPI_TXP1, VO_D_1);//data_1:5
		PINMUX_CONFIG(PAD_MIPI_TXM1,VO_D_2);//0

		PINMUX_CONFIG(PAD_MIPI_TXP0, VO_D_3);//data_0:4
		PINMUX_CONFIG(PAD_MIPI_TXM0, VO_D_4);

		PINMUX_CONFIG(PAD_MIPIRX0P,VO_D_5);
		PINMUX_CONFIG(PAD_MIPIRX0N,VO_D_6);
		PINMUX_CONFIG(PAD_MIPIRX1P,VO_D_7);
		PINMUX_CONFIG(PAD_MIPIRX1N,VO_D_8);

		PINMUX_CONFIG(PAD_MIPIRX2P,VO_D_9);
		PINMUX_CONFIG(PAD_MIPIRX2N,VO_D_10);
		PINMUX_CONFIG(PAD_MIPIRX5P,VO_D_11);
		PINMUX_CONFIG(PAD_MIPIRX5N,VO_D_12);

		PINMUX_CONFIG(VIVO_D0, VO_D_13);
		PINMUX_CONFIG(VIVO_D1, VO_D_14);
		PINMUX_CONFIG(VIVO_D2, VO_D_15);
		PINMUX_CONFIG(VIVO_D3, VO_D_16);

		PINMUX_CONFIG(VIVO_D4,VO_D_17);
		PINMUX_CONFIG(VIVO_D5,VO_D_18);
		PINMUX_CONFIG(VIVO_D6,VO_D_19);
		PINMUX_CONFIG(VIVO_D7,VO_D_20);
		PINMUX_CONFIG(VIVO_D8,VO_D_21);
		PINMUX_CONFIG(VIVO_D9,VO_D_22);
		PINMUX_CONFIG(VIVO_D10,VO_D_23);

		//PINMUX_CONFIG(PAD_MIPI_TXM4, VO_D_24);//Reset pin for GPIO
		PINMUX_CONFIG(PAD_MIPI_TXP4, VO_D_25);
		PINMUX_CONFIG(PAD_MIPI_TXM3, VO_D_26);
		PINMUX_CONFIG(PAD_MIPI_TXP3, VO_D_27);
#endif
		break;
	// case PINMUX_PWM1:
	// 	PINMUX_CONFIG(UART1_TX, PWM_4);
	// 	PINMUX_CONFIG(UART1_RX, PWM_5);
	// 	PINMUX_CONFIG(UART1_CTS, PWM_6);
	// 	PINMUX_CONFIG(UART1_RTS, PWM_7);
	// break;
	case PINMUX_SPI_NOR1:
		PINMUX_CONFIG(SD1_D3, PWR_SPINOR1_CS_X);
		PINMUX_CONFIG(SD1_D2, PWR_SPINOR1_HOLD_X);
		PINMUX_CONFIG(SD1_D1, PWR_SPINOR1_WP_X);
		PINMUX_CONFIG(SD1_D0, PWR_SPINOR1_MISO);
		PINMUX_CONFIG(SD1_CMD, PWR_SPINOR1_MOSI);
		PINMUX_CONFIG(SD1_CLK, PWR_SPINOR1_SCK);
		break;
	default:
		break;
	}
}

#endif // CONFIG_BOARD_DUO256
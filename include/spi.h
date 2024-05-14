#ifndef __SPI_H__
#define __SPI_H__

#define CTRL0		0X00
#define CTRL1		0X04
#define SPIENR		0X08
#define MWCR		0X0C
#define SER		0X10
#define BAUDR		0X14
#define TXFTLR		0X18
#define RXFTLR		0X1C
#define TXFLR		0X20
#define RXFLR		0X24
#define SR		0X28
#define IMR		0X2C
#define ISR		0X30
#define RISR		0X34
#define TXOICR		0X38
#define RXOICR		0X3C
#define RXUICR		0X40
#define MSTICR		0X44
#define ICR		0X48
#define DMACR		0X4C
#define DMATDLR		0X50
#define DMARDLR		0X54
#define IDR		0X58
#define VERSION		0X5C
#define DR		0X60
#define RX_SAMPLE_DLY	0XF0
#define CS_OVERRIDE	0XF4

/* CTRL0 filed*/
#define SPI_SRL			(1 << 11)
#define SPI_TMOD_MASK		(0x3 << 8)
#define	SPI_TMOD_TR		0x0000 /* xmit & recv */
#define SPI_TMOD_TO		0x0100 /* xmit only */
#define SPI_TMOD_RO		0x0200 /* recv only */
#define SPI_EERPOMREAD		0x0300 /* EEPROM READ */
#define SPI_FRF_MASK		0x0030
#define SPI_SCPOL_MASK		0x0080
#define SPI_SCPOL_LOW		0x0000
#define SPI_SCPOL_HGIH		0x0080
#define SPI_SCPH_MASK		0x0040
#define SPI_SCPH_MIDDLE		0x0000
#define SPI_SCPH_START		0x0040
#define SPI_FRF_SPI		0x0
#define SPI_DFS_MASK		0x000F
#define SPI_DFS_8BIT		0x0007
#define SPI_DFS_16BIT		0x000F

#define SPI_BAUDR_DIV		0x32

/* RISR filed */
#define SPI_RISR_RXFIR		0x10
#define SPI_RISR_RXOIR		0x08
#define SPI_RISR_RXUIR		0x04
#define SPI_RISR_TXOIR		0x02
#define SPI_RISR_TXEIR		0x01

#endif // __SPI_H__
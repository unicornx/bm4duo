#ifndef __IIC_H__
#define __IIC_H__

#define I2C_CON			0x000
#define I2C_TAR			0x004
#define I2C_SAR			0x008
#define I2C_DATA_CMD		0x010
#define I2C_SS_SCL_HCNT		0x014 // I2C_SS_HCNT
#define I2C_SS_SCL_LCNT		0x018 // I2C_SS_LCNT
#define I2C_FS_SCL_HCNT		0x01C // I2C_FS_HCNT
#define I2C_FS_SCL_LCNT		0x020 // I2C_FS_LCNT
//#define I2C_HS_HCNT	0x24
//#define I2C_HS_LCNT	0x28
#define I2C_INTR_STAS		0x2C // I2C_INT_STS
#define I2C_INTR_MASK		0x30 // I2C_INT_MASK
#define I2C_RAW_INTR_STAS	0x34 // I2C_RAW_INT_STS
#define I2C_RX_TL		0x38
#define I2C_TX_TL		0x3C

#define I2C_ENABLE		0x6C
#define I2C_STATUS		0x70


#define IST_RX_FULL		(1<<2)
#define IST_TX_EMPTY		(1<<4)

#define ST_RFNE			(1<<3)


#endif // __IIC_H__
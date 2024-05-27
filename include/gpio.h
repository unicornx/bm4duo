#ifndef __GPIO_H__
#define __GPIO_H__

#define GPIO_SWPORTA_DR		0x000
#define GPIO_SWPORTA_DDR	0x004
#define GPIO_INTEN		0x030
#define GPIO_INTMASK		0x034
#define GPIO_INTTYPE_LEVEL	0x038
#define GPIO_INT_POLARITY	0x03c
#define GPIO_INTSTATUS		0x040
#define GPIO_RAW_INTSTATUS	0x044
#define GPIO_DEBOUNCE		0x048
#define GPIO_PORTA_EOI		0x04c
#define GPIO_EXT_PORTA		0x050
#define GPIO_LS_SYNC		0x060

/* GPIO alias */
#define GPIOA_BASE          GPIO0_BASE
#define GPIOB_BASE          GPIO1_BASE
#define GPIOC_BASE          GPIO2_BASE
#define GPIOD_BASE          GPIO3_BASE

#endif // __GPIO_H__
#include <stdint.h>

#include "mmap.h"

/*
 * RBR_THR_DLL: 0x00 Receive Buffer,Transmit Holding or Divisor Latch Low byte Register
 * IER_DLH: 0x04 Interrupt Enable or Divisor Latch High byte Register
 * FCR_IIR: 0x08 FIFO Control or Interrupt Identification Register
 * LCR: 0x0C Line Control Register
 * MCR: 0x10 Modem Control Register
 * LSR: 0x14 Line Status Register
 * MSR: 0x18 Modem Status Register
 */ 
struct dw_regs {
	volatile uint32_t	rbr;
	volatile uint32_t	ier;
	volatile uint32_t	fcr;
	volatile uint32_t	lcr;
	volatile uint32_t	mcr;
	volatile uint32_t	lsr;
	volatile uint32_t	msr;
};

#define thr rbr
#define dll rbr
#define iir fcr
#define dlh ier

#define UART_LCR_DLS_MSK	0x03	/* Data Length Select mask */
#define UART_LCR_DLS_5		0x00	/* 5 bit character length */
#define UART_LCR_DLS_6		0x01	/* 6 bit character length */
#define UART_LCR_DLS_7		0x02	/* 7 bit character length */
#define UART_LCR_DLS_8		0x03	/* 8 bit character length */
#define UART_LCR_STB		0x04	/* Number of STop Bits, off=1, on=1.5 or 2) */
#define UART_LCR_PEN		0x08	/* Parity ENeble */
#define UART_LCR_EPS		0x10	/* Even Parity Select */
#define UART_LCR_STKP		0x20	/* STicK Parity */
#define UART_LCR_BCB		0x40	/* Break Control Bit */
#define UART_LCR_DLAB		0x80	/* Divisor Latch Access Bit */

#define UART_IER_NULL	0x00	/* Disable ALL Interrupts */
#define UART_IER_RDA	0x01	/* Enable Received Data Available Interrupt */
#define UART_IER_THR	0x02	/* Enable Transmit Holding Register Empty Interrupt */
#define UART_IER_RLS	0x04	/* Enable Receiver Line Status Interrupt */
#define UART_IER_MS	0x08	/* Enable Modem Status Interrupt */
#define UART_IER_THRE	0x80	/* Enable Programmable THRE Interrupt Mode */

#define UART_MCR_DTR	0x01	/* DTR   */
#define UART_MCR_RTS	0x02	/* RTS   */

#define UART_LSR_THRE	0x20	/* Transmit-hold-register empty */
#define UART_LSR_DR	0x01	/* Receiver data ready */
#define UART_LSR_TEMT	0x40	/* Xmitter empty */

#define UART_FCR_FIFO_EN	0x01	/* Fifo enable */
#define UART_FCR_RXSR		0x02	/* Receiver soft reset */
#define UART_FCR_TXSR		0x04	/* Transmitter soft reset */

#define UART_MCRVAL (UART_MCR_DTR | UART_MCR_RTS)      /* RTS/DTR */
#define UART_FCR_DEFVAL	(UART_FCR_FIFO_EN | UART_FCR_RXSR | UART_FCR_TXSR)

static struct dw_regs *uart = (struct dw_regs *)UART0_BASE;

#define UART_BAUDRATE	115200
#define UART_CLOCKRATE	25 * 1000 * 1000

void uart_init(int port)
{
	switch (port) {
	case 0:
		uart = (struct dw_regs *)UART0_BASE;
		break;
	case 1:
		uart = (struct dw_regs *)UART1_BASE;
		break;
	case 2:
		uart = (struct dw_regs *)UART2_BASE;
		break;
	case 3:
		uart = (struct dw_regs *)UART3_BASE;
		break;
	case 4:
		uart = (struct dw_regs *)UART4_BASE;
		break;
	case 5:
		uart = (struct dw_regs *)PWR_UART_BASE;
		break;
	default:
		uart = (struct dw_regs *)UART0_BASE;// default
		return;
	}

	/*
	 * Calculate the divisor and set it.
	 * LCR[7] is set 1 means:
	 * - RBR_THR_DLL works as DLL and it contains Lower 8-bits of a 16-bit divisor
	 * - IER_DLH wors as DLH and it contains Upper 8-bits of a 16-bit divisor
	 * restore LCR[7] to zero after setting of divisor is done
	 *
	 * 严格按照 TRM 公式计算 divisor = UART_CLOCKRATE / (16 * UART_BAUDRATE)
	 * 得到 ~13.56，但是除法直接舍弃小数，导致得到 13，精度损失较大，用 14
	 * 误差较小
	 */
	int divisor = 14;
	uart->lcr = uart->lcr | UART_LCR_DLAB;
	uart->dll = divisor & 0xff;
	uart->dlh = (divisor >> 8) & 0xff;
	uart->lcr = uart->lcr & (~UART_LCR_DLAB);

	/* Default we don't use interrupt for UART handling */
//	uart->ier = UART_IER_NULL;
	uart->ier = (uart->ier) | 1;

	/*
	 * Setting the asynchronous data communication format.
	 * - number of the word length: 8 bits
	 * - number of stop bits：1 bit when word length is 8 bits
	 * - no parity
	 * - no break control
	 * - disabled baud latch
	 */
	uart->lcr = UART_LCR_DLS_8;
}

void _uart_putc(uint8_t ch)
{
	while (!(uart->lsr & UART_LSR_THRE))
		;
	uart->rbr= ch;
}

void uart_putc(uint8_t ch)
{
	if (ch == '\n') {
		_uart_putc('\r');
	}
	_uart_putc(ch);
}

void uart_puts(char *str)
{
	if (!str)
		return;

	while (*str) {
		uart_putc(*str++);
	}
}

int uart_getc(void)
{
	while (!(uart->lsr & UART_LSR_DR))
		;
	return (int)uart->rbr;
}

int uart_tstc(void)
{
	return (!!(uart->lsr & UART_LSR_DR));
}

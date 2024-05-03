#include "common.h"

// 本实验演示中断
// 注册 UART1 上的中断，当 RX 上有数据到达时中断发生
// 在中断处理函数中打印接收到的字符。
// 注意串口对应的引脚在 system 初始化时已经通过 pinmux 选择了 UART1

static int uart_isr(int intrid, void *priv)
{
	int c = uart_getc();
	uart_putc((char)c);
	uart_putc('\n');
	uart_puts("Please press key: ");
	return 0;
}

void test_irq()
{
	uart_puts("Please press key: ");

	request_irq(UART1_INTR, uart_isr, 0, "UART INT", 0);

	while(1);
}

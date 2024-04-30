#include "common.h"

// 本实验演示通过接收 UART1 上的中断
// 串口对应的引脚在 system 初始化时已经通过 pinmux 选择了 UART1
// 控制台的输出就是通过轮询方式实现的 UART controller 的输出，
// 这里继续演示中断方式下 UART controller 接收（输入）数据

static int uart_isr(int intrid, void *priv)
{
	int c = uart_getc();
	uart_putc((char)c);
	uart_putc('\n');
	uart_puts("Please press key: ");
	return 0;
}

void test_uart_irq()
{
	uart_puts("Please press key: ");

	request_irq(UART1_INTR, uart_isr, 0, "UART INT", 0);

	while(1);
}

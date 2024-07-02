#include "trace.h"

int main(void)
{
#if defined(CONFIG_TEST_DBG)
	extern void led_pinmux_config(void);
	extern void led_pinctrl_config(void);
	extern void led_on(void);

	printf("====> In DEBUG mode ......\n");
	// 将板子上的 LED 灯常亮，说明进入调试模式
	led_pinmux_config();
	led_pinctrl_config();
	led_on();

#else
	extern int testcase_main();

	printf("====> test start\n");

	testcase_main();

	printf("====> test done!\n");
#endif

	// NOTICE!!! don't remove this line
	while(1);

	return 0;
}

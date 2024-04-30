#include "trace.h"

extern int testcase_main();

int main(void)
{
	int r;
	printf("====> test start\n");

	r = testcase_main();

	printf("====> test done!\n");
	
	// NOTICE!!! don't remove this line
	while(1);

	return r;
}

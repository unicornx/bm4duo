#ifndef __PWM_H__
#define __PWM_H__

#define	HLPERIOD0		0x0
#define PERIOD0			0x4
#define HLPERIOD1		0x8
#define PERIOD1			0xc
#define HLPERIOD2		0x10
#define PERIOD2			0x14
#define HLPERIOD3		0x18
#define PERIOD3			0x1c
#define FREQNUM			0x20
#define FREQDATA		0x24
#define POLARITY		0x40
#define PWMSTART		0x44
#define PCOUNT0			0x50
#define PCOUNT1			0x54
#define PCOUNT2			0x58
#define PCOUNT3			0x5C
#define PCOUNT0_STATUS		0x60
#define PCOUNT1_STATUS		0x64
#define PCOUNT2_STATUS		0x68
#define PCOUNT3_STATUS		0x6C
#define SHIFTCOUNT0		0x80
#define SHIFTCOUNT1		0x84
#define SHIFTCOUNT2		0x88
#define SHIFTCOUNT3		0x8C
#define SHIFTSTART		0x90
#define FREQEN			0x9c
#define FREQE_DONE_NUM		0xc0
#define PWMOE			0xd0


#endif // __PWM_H__

	.option norvc
	.section .text.init,"ax",@progbits
	.globl entry_vector
entry_vector:
	fence.i
	li x1, 0
	li x2, 0
	li x3, 0
	li x4, 0
	li x5, 0
	li x6, 0
	li x7, 0
	li x8, 0
	li x9, 0
	li x10, 0
	li x11, 0
	li x12, 0
	li x13, 0
	li x14, 0
	li x15, 0
	li x16, 0
	li x17, 0
	li x18, 0
	li x19, 0
	li x20, 0
	li x21, 0
	li x22, 0
	li x23, 0
	li x24, 0
	li x25, 0
	li x26, 0
	li x27, 0
	li x28, 0
	li x29, 0
	li x30, 0
	li x31, 0
	csrw mscratch, x0

	csrw mie, x0

	# write mtvec and make sure it sticks
	la t0, trap_vector
	csrw mtvec, t0

	csrr t0, mhartid
	bnez t0, second_core

	# this setting is for RISCV C906
	# invalid I-cache
	li x3, 0x33
	csrc mcor, x3
	li x3, 0x11
	csrs mcor, x3
	# enable I-cache
	li x3, 0x1
	csrs mhcr, x3
	# invalid D-cache
	li x3, 0x33
	csrc mcor, x3
	li x3, 0x12
	csrs mcor, x3
	# disable D-cache for some test cases
	li x3, 0x2
	csrc mhcr, x3

	# enable fp
	li x3, 0x1 << 13
	csrs mstatus, x3

	la sp, STACK_TOP0-8
	# double-word aligned in rom.lds
	la a2, __DATA_LMA_START__
	la a3, __DATA_START__
	la a4, __DATA_SIZE__

data_cpy:
	ld a5, 0(a2)
	sd a5, 0(a3)
	addi a2, a2, 8
	addi a3, a3, 8
	addi a4, a4, -8
	bnez a4, data_cpy

	la a3, __BSS_START__
	la a4, __BSS_SIZE__

data_clear:
	sd x0, 0(a3)
	addi a3, a3, 8
	addi a4, a4, -8
	bnez a4, data_clear

	j core_init_done

second_core:
	// only use one hart for bmtest
	j .

core_init_done:
	jal system_init
	call main
	j die

trap_vector:
	j trap_entry
	// never reach here
	j die


#define LREG                    ld
#define SREG                    sd
#define REGBYTES                8
/*
 * trap entry
 */
trap_entry:
	addi sp, sp, -35*REGBYTES
	SREG x1, 1*REGBYTES(sp)
	SREG x2, 2*REGBYTES(sp)
	SREG x3, 3*REGBYTES(sp)
	SREG x4, 4*REGBYTES(sp)
	SREG x5, 5*REGBYTES(sp)
	SREG x6, 6*REGBYTES(sp)
	SREG x7, 7*REGBYTES(sp)
	SREG x8, 8*REGBYTES(sp)
	SREG x9, 9*REGBYTES(sp)
	SREG x10, 10*REGBYTES(sp)
	SREG x11, 11*REGBYTES(sp)
	SREG x12, 12*REGBYTES(sp)
	SREG x13, 13*REGBYTES(sp)
	SREG x14, 14*REGBYTES(sp)
	SREG x15, 15*REGBYTES(sp)
	SREG x16, 16*REGBYTES(sp)
	SREG x17, 17*REGBYTES(sp)
	SREG x18, 18*REGBYTES(sp)
	SREG x19, 19*REGBYTES(sp)
	SREG x20, 20*REGBYTES(sp)
	SREG x21, 21*REGBYTES(sp)
	SREG x22, 22*REGBYTES(sp)
	SREG x23, 23*REGBYTES(sp)
	SREG x24, 24*REGBYTES(sp)
	SREG x25, 25*REGBYTES(sp)
	SREG x26, 26*REGBYTES(sp)
	SREG x27, 27*REGBYTES(sp)
	SREG x28, 28*REGBYTES(sp)
	SREG x29, 29*REGBYTES(sp)
	SREG x30, 30*REGBYTES(sp)
	SREG x31, 31*REGBYTES(sp)
	csrr a1, mepc
	SREG a1,   0*REGBYTES(sp)
	csrr t0, mstatus
	SREG t0,  32*REGBYTES(sp)
	csrr t1, mbadaddr
	SREG t1,  33*REGBYTES(sp)
	csrr a0, mcause
	SREG a0,  34*REGBYTES(sp)
	mv a2, sp
	jal handle_trap

	LREG t0, 32*REGBYTES(sp)
	LREG a1,  0*REGBYTES(sp)
	csrw mstatus, t0
	csrw mepc, a1

	LREG x1, 1*REGBYTES(sp)
	LREG x2, 2*REGBYTES(sp)
	LREG x3, 3*REGBYTES(sp)
	LREG x4, 4*REGBYTES(sp)
	LREG x5, 5*REGBYTES(sp)
	LREG x6, 6*REGBYTES(sp)
	LREG x7, 7*REGBYTES(sp)
	LREG x8, 8*REGBYTES(sp)
	LREG x9, 9*REGBYTES(sp)
	LREG x10, 10*REGBYTES(sp)
	LREG x11, 11*REGBYTES(sp)
	LREG x12, 12*REGBYTES(sp)
	LREG x13, 13*REGBYTES(sp)
	LREG x14, 14*REGBYTES(sp)
	LREG x15, 15*REGBYTES(sp)
	LREG x16, 16*REGBYTES(sp)
	LREG x17, 17*REGBYTES(sp)
	LREG x18, 18*REGBYTES(sp)
	LREG x19, 19*REGBYTES(sp)
	LREG x20, 20*REGBYTES(sp)
	LREG x21, 21*REGBYTES(sp)
	LREG x22, 22*REGBYTES(sp)
	LREG x23, 23*REGBYTES(sp)
	LREG x24, 24*REGBYTES(sp)
	LREG x25, 25*REGBYTES(sp)
	LREG x26, 26*REGBYTES(sp)
	LREG x27, 27*REGBYTES(sp)
	LREG x28, 28*REGBYTES(sp)
	LREG x29, 29*REGBYTES(sp)
	LREG x30, 30*REGBYTES(sp)
	LREG x31, 31*REGBYTES(sp)
	addi sp, sp, 35*REGBYTES
	mret

die:
1:
	j 1b


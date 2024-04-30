OUTPUT_ARCH( "riscv" )

ENTRY( entry_vector )
/*
 * Only constant here
 * No function !
 */

MEMORY {
	RAM (rwx): ORIGIN = 0x80000000, LENGTH = 64M
}
SECTIONS
{
	/*--------------------------------------------------------------------*/
	/* Code and read-only segment                                         */
	/*--------------------------------------------------------------------*/
	/* Begining of code and text segment */
	. = ORIGIN(RAM);
	.text :	{
		*(.text.init)
	} >RAM
	/* text: Program code section */
	.text :	{
		*(.text)
		*(.text.*)
		*(.gnu.linkonce.t.*)
	} >RAM
	/* rodata: Read-only data */

	. = ALIGN(8);
	.rodata : {
		*(.rdata)
		*(.rodata)
		*(.rodata.*)
		*(.gnu.linkonce.r.*)
		/* data aligned 8 for fast init */
		. = ALIGN(8);
	} >RAM

	/* End of code and read-only segment */

	. = ALIGN(8);
	/*--------------------------------------------------------------------*/
	/* Initialized data segment                                           */
	/*--------------------------------------------------------------------*/
	/* data: Writable data */
	.data  : {
		/* data aligned 8 for fast init */
		. = ALIGN(8);
		__DATA_START__ = .;
		*(.data)
		*(.data*)
		*(.srodata*)
		*(.sdata)
		*(.sdata.*)
		*(.gnu.linkonce.d.*)
		*(.comment)
		. = ALIGN(8);
		__DATA_END__ = .;
		_edata = .;
	} >RAM

	. = ALIGN(8);
	.bss (NOLOAD): /*AT (ADDR(.data)+SIZEOF(.data)) */ {
		/* bss aligned 8 for fast init */
		. = ALIGN(8);
		__BSS_START__ = .;
		*(.bss)
		*(.bss.*)
		*(.sbss*)
		*(.gnu.linkonce.b.*)
		*(COMMON)
		. = ALIGN(8);
		__BSS_END__ = .;
	} >RAM

	. = ALIGN(8); 
	__DATA_LMA_START__ = LOADADDR(.data);
	__DATA_SIZE__      = SIZEOF(.data);
	__DATA_LMA_END__   = __DATA_LMA_START__ + __DATA_SIZE__ ;
	__BSS_LMA_START__ = LOADADDR(.bss);
	__BSS_SIZE__ = SIZEOF(.bss);
	/* Stack after _end, align 2XLEN */
	STACK_SIZE = 0x8000;
	STACK_TOP0 = . + STACK_SIZE;
	/*STACK_TOP1 = ORIGIN(RAM1) + STACK_SIZE;*/

	PROVIDE( __global_pointer$ = . + (2K / 2) );
	. = ALIGN(16)+1K;
	_end = . ;
}

/* destroyed linkscript */

OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)

MEMORY {

	rom	: ORIGIN = 0x08000000, LENGTH = 32M
	iwram	: ORIGIN = 0x03000000, LENGTH = 32K
	ewram	: ORIGIN = 0x02000000, LENGTH = 256K
	vram : ORIGIN = 0x05000200, LENGTH = 1K
}



__boot_start	=	ORIGIN(ewram)+0xC0;
__image_vma	=	ORIGIN(vram);
__iwram_top	=	ORIGIN(iwram) + LENGTH(iwram);

__sp_irq	=	__iwram_top - 0x060;
__sp_usr	=	__sp_irq - 0x0a0;
__irq_flags	=	0x03007ff8;
__irq_vector = 0x3007ffc;

SECTIONS
{
	. = __boot_start;
	.boot : AT (__boot_start)
	{
		*(.boot.*)
		*(.text*)
		*(.data*)
		*(.bss*)
		*(.glue_7*)
		. = ALIGN(4);
	} >ewram =0xff

	__image_lma = __boot_start + SIZEOF( .boot );
	.image : AT (__image_lma)
	{
		*(.loader*)
		*(.rodata*)
		. = ALIGN(4);
	} >vram =0x00
	
	__image_end = .;
	__image_size = SIZEOF( .loader );
}

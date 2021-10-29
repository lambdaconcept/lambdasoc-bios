#include <arch/header.ld>
#include <config.h>

MEMORY
{
	bootrom    (rx)  : ORIGIN = CONFIG_BOOTROM_BASE,    LENGTH = CONFIG_BOOTROM_SIZE
	scratchpad (rwx) : ORIGIN = CONFIG_SCRATCHPAD_BASE, LENGTH = CONFIG_SCRATCHPAD_SIZE
}

SECTIONS
{
	.text :
	{
		_ftext = .;
		*entry*.o(.text)
		*(.text .stub .text.* .gnu.linkonce.t.*)
		_etext = .;
	} > bootrom

	.rodata :
	{
		. = ALIGN(8);
		_frodata = .;
		*(.rodata .rodata.* .gnu.linkonce.r.*)
		*(.rodata1)
		*(.got .got.*)
		*(.toc .toc.*)

		/* Make sure the file is aligned on disk as well
		   as in memory; CRC calculation requires that. */
		FILL(0);
		. = ALIGN(8);
		_erodata = .;
	} > bootrom

	.data : ALIGN(8)
	{
		. = ALIGN(8);
		_fdata = .;
		*(.data .data.* .gnu.linkonce.d.*)
		*(.data1)
		*(.sdata .sdata.* .gnu.linkonce.s.*)

		/* Make sure the file is aligned on disk as well
		   as in memory; CRC calculation requires that. */
		FILL(0);
		. = ALIGN(8);
		_edata = .;
	} > scratchpad AT> bootrom

	.bss :
	{
		. = ALIGN(8);
		_fbss = .;
		*(.dynsbss)
		*(.sbss .sbss.* .gnu.linkonce.sb.*)
		*(.scommon)
		*(.dynbss)
		*(.bss .bss.* .gnu.linkonce.b.*)
		*(COMMON)
		. = ALIGN(8);
		_ebss = .;
		_end = .;
	} > scratchpad

	/DISCARD/ :
	{
		*(.eh_frame)
		*(.comment)
	}
}

PROVIDE(_fstack = ORIGIN(scratchpad) + LENGTH(scratchpad) - 8);

PROVIDE(_fdata_bootrom = LOADADDR(.data));
PROVIDE(_edata_bootrom = LOADADDR(.data) + SIZEOF(.data));

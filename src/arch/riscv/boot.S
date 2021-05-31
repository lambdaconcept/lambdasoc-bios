	.section .text, "ax", @progbits
	.global boot_helper
boot_helper:
	// flush instruction cache
	fence.i
	nop // FIXME
	nop
	nop
	nop
	nop
	nop
	// jump to payload
	jr a3

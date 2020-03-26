	.section .text, "ax", @progbits
	.global boot_helper
boot_helper:
	// flush instruction cache
	/* fence.i */ // FIXME
	// jump to payload
	jr a3

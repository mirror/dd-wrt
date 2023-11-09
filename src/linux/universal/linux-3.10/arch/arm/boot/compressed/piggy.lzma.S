	.section .piggydata,"a"
	.globl	input_data
input_data:
	.incbin	"arch/arm/boot/compressed/piggy.lzma"
	.globl	input_data_end
input_data_end:

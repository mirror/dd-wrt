		.macro test_for_cache_ipi, irqnr, irqstat, base, tmp
		bic	\irqnr, \irqstat, #0x1c00
		mov 	\tmp, #0
		cmp	\irqnr, #1
		moveq	\tmp, #1
		streq	\irqstat, [\base, #GIC_CPU_EOI]
		cmp	\tmp, #0
		.endm

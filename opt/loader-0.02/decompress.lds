OUTPUT_ARCH(mips)
ENTRY(entry)
SECTIONS {
	. = 0x80700000;
	.text : {
		*(.text.entry)
		*(.text)
		*(.rodata)
	}

	.data : {
		*(.data)
	}

	.bss : {
		*(.bss)
	}

	workspace = .;
}

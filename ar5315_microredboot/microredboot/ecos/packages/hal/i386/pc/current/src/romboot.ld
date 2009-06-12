

MEMORY
{
    rom : ORIGIN = 0x00000, LENGTH = 0x100
}

SECTIONS
{
	.text : { *(.text) *(.data) *(.bss) } > rom
}
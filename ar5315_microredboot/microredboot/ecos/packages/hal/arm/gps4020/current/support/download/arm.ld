ENTRY(start)

MEMORY
{
    ram : ORIGIN = 0x0, LENGTH = 0x2000
}

SECTIONS
{
    .text 0x00000000 : { _stext = .; *(.text*) *(.gnu.warning) *(.gnu.linkonce.t.*) *(.init) *(.glue_7) *(.glue_7t) } > ram _etext = .; PROVIDE (etext = .);
    .fini ALIGN (0x4) : { . = .; *(.fini) } > ram
    .rodata1 ALIGN (0x8) : { . = .; *(.rodata1*) } > ram
    .rodata ALIGN (0x8) : { . = .; *(.rodata*) } > ram
    .fixup ALIGN (0x4) : { __FIXUP_START__ = ABSOLUTE(.); *(.fixup) __FIXUP_END__ = ABSOLUTE(.);} > ram
    .gcc_except_table ALIGN (0x1) : { __EXCEPT_START__ = ABSOLUTE(.); *(.gcc_except_table) __EXCEPT_END__ = ABSOLUTE(.);} > ram
    .data ALIGN (0x8) : { __ram_data_start = ABSOLUTE(.); *(.data*) __GOT1_START__ = ABSOLUTE(.); *(.got1) __GOT1_END__ = ABSOLUTE(.); . = ALIGN(8); __CTOR_LIST__ = ABSOLUTE(.); KEEP(*(SORT(.ctors*))) __CTOR_END__ = ABSOLUTE(.); __DTOR_LIST__ = ABSOLUTE(.); KEEP(*(SORT(.dtors*))) __DTOR_END__ = ABSOLUTE(.); . = ALIGN(8); KEEP(*( SORT (.ecos.table.*))) ; . = ALIGN(4); *( .2ram.*) ; __GOT2_START__ = ABSOLUTE(.); *(.got2) __GOT2_END__ = ABSOLUTE(.); __GOT_START = ABSOLUTE(.); _GLOBAL_OFFSET_TABLE_ = ABSOLUTE(. + 32768); _SDA_BASE_ = ABSOLUTE(.); *(.got.plt) *(.got) __GOT_END__ = ABSOLUTE(.); *(.dynamic) *(.eh_frame) __SDATA_START__ = ABSOLUTE(.); *(.sdata) *(.sdata.*) __SDATA2_START__ = ABSOLUTE(.); *(.sdata2*) } > ram __rom_data_start = LOADADDR(.data); __ram_data_end = .; PROVIDE(__ram_data_end = .); _edata = .; PROVIDE (edata = .);
    .sbss ALIGN (0x4) : { __sbss_start = ABSOLUTE (.); __SBSS_START__ = ABSOLUTE(.); *(.sbss.*) __SBSS_END__ = ABSOLUTE(.); __SBSSx_START__ = ABSOLUTE(.); *(.sbss*) __SBSSx_END__ = ABSOLUTE(.); *(.scommon*) __sbss_end = ABSOLUTE (.); } > ram
    .bss ALIGN (0x10) : { __bss_start = ABSOLUTE (.); . = .; *(.dynbss*) *(.bss*) *(COMMON) __bss_end = ABSOLUTE (.); } > ram
    __heap1 = ALIGN (0x8);
    . = ALIGN(4); _end = .; PROVIDE (end = .);
}

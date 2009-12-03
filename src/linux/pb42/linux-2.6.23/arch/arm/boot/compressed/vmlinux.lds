/*
 *  linux/arch/arm/boot/compressed/vmlinux.lds.in
 *
 *  Copyright (C) 2000 Russell King
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
OUTPUT_ARCH(arm)
ENTRY(_start)
SECTIONS
{
  . = 0;
  _text = .;

  .text : {
    _start = .;
    *(.start)
    *(.text)
    *(.text.*)
    *(.fixup)
    *(.gnu.warning)
    *(.rodata)
    *(.rodata.*)
    *(.glue_7)
    *(.glue_7t)
    *(.piggydata)
    . = ALIGN(4);
  }

  _etext = .;

  _got_start = .;
  .got			: { *(.got) }
  _got_end = .;
  .got.plt		: { *(.got.plt) }
  .data			: { *(.data) }
  _edata = .;

  . = ALIGN(4);
  __bss_start = .;
  .bss			: { *(.bss) }
  _end = .;

  .stack (NOLOAD)	: { *(.stack) }

  .stab 0		: { *(.stab) }
  .stabstr 0		: { *(.stabstr) }
  .stab.excl 0		: { *(.stab.excl) }
  .stab.exclstr 0	: { *(.stab.exclstr) }
  .stab.index 0		: { *(.stab.index) }
  .stab.indexstr 0	: { *(.stab.indexstr) }
  .comment 0		: { *(.comment) }
}


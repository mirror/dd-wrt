//=============================================================================
//
// MLT linker script for NEC
// adapted from packages/hal/arm/pid/v1_1/src/pid.ld
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//=============================================================================

#include <pkgconf/system.h>

STARTUP(vectors.o)
// Enter at reset_vector in non-RAM startups so they are more representative
// of actual execution when loaded over ICE
#ifdef CYG_HAL_STARTUP_RAM
ENTRY(start)
#else
ENTRY(reset_vector)
#endif
#ifdef EXTRAS
INPUT(extras.o)
#endif
#if (__GNUC__ >= 3)
GROUP(libtarget.a libgcc.a libsupc++.a)
#else
GROUP(libtarget.a libgcc.a)
#endif

#define ALIGN_LMA 4
#define FOLLOWING(_section_) AT ((LOADADDR (_section_) + SIZEOF (_section_) + ALIGN_LMA - 1) & ~ (ALIGN_LMA - 1))
#define LMA_EQ_VMA
#define FORCE_OUTPUT . = .

#define SECTIONS_BEGIN                          \
  /* Debug information */                       \
  .debug_aranges  0 : { *(.debug_aranges) }     \
  .debug_pubnames 0 : { *(.debug_pubnames) }    \
  .debug_info     0 : { *(.debug_info) }        \
  .debug_abbrev   0 : { *(.debug_abbrev) }      \
  .debug_line     0 : { *(.debug_line) }        \
  .debug_frame    0 : { *(.debug_frame) }       \
  .debug_str      0 : { *(.debug_str) }         \
  .debug_loc      0 : { *(.debug_loc) }         \
  .debug_macinfo  0 : { *(.debug_macinfo) }

#define SECTION_rom_vectors(_region_, _vma_, _lma_) \
    .rom_vectors _vma_ : _lma_ \
    { FORCE_OUTPUT; KEEP (*(.vectors)) } \
    > _region_

#define SECTION_ram_vectors(_region_, _vma_, _lma_) \
    .ram_vectors _vma_ : _lma_ \
    { ___ram_vectors_start = ABSOLUTE(.); \
      FORCE_OUTPUT; KEEP (*(.ram_vectors)) \
      ___ram_vectors_end = ABSOLUTE(.); \
    } > _region_ \
    ___ram_vectors_loadaddr = LOADADDR(.ram_vectors);

#define SECTION_text(_region_, _vma_, _lma_) \
    .text _vma_ : _lma_ \
    { stext = ABSOLUTE(.); \
    PROVIDE (__stext = ABSOLUTE(.)); \
    *(.text*) *(.gnu.warning) *(.gnu.linkonce*) *(.init) \
    *(.glue_7) *(.glue_7t)  \
    } > _region_ \
    etext = .; PROVIDE (__etext = .);

#define SECTION_fini(_region_, _vma_, _lma_) \
    .fini _vma_ : _lma_ \
    { FORCE_OUTPUT; *(.fini) } \
    > _region_

#define SECTION_rodata(_region_, _vma_, _lma_) \
    .rodata _vma_ : _lma_ \
    { FORCE_OUTPUT; *(.rodata*) } \
    > _region_

#define SECTION_rodata1(_region_, _vma_, _lma_) \
    .rodata1 _vma_ : _lma_ \
    { FORCE_OUTPUT; *(.rodata1) } \
    > _region_

#define SECTION_fixup(_region_, _vma_, _lma_) \
    .fixup _vma_ : _lma_ \
    { FORCE_OUTPUT; *(.fixup) } \
    > _region_

#define SECTION_gcc_except_table(_region_, _vma_, _lma_) \
    .gcc_except_table _vma_ : _lma_ \
    { FORCE_OUTPUT; *(.gcc_except_table) } \
    > _region_

#define SECTION_data(_region_,  _vma_, _lma_) \
    .data _vma_ : _lma_ \
    { ___ram_data_start = ABSOLUTE (.); *(.data*) *(.data1); \
    _GOT1_START_ = ABSOLUTE (.); *(.got1) _GOT1_END_ = ABSOLUTE (.); \
    _GOT2_START_ = ABSOLUTE (.); *(.got2) _GOT2_END_ = ABSOLUTE (.); \
    . = ALIGN (4); \
    KEEP(*( SORT (.ecos.table.*))) ; \
    ___CTOR_LIST__ = ABSOLUTE (.); KEEP (*(SORT (.ctors*))) ___CTOR_END__ = ABSOLUTE (.); \
    ___DTOR_LIST__ = ABSOLUTE (.); KEEP (*(SORT (.dtors*))) ___DTOR_END__ = ABSOLUTE (.); \
    *(.eh_frame) \
    *(.dynamic) *(.sdata*) *(.sbss*) } \
    > _region_ \
    ___rom_data_start = LOADADDR (.data); \
    ___ram_data_end = .; PROVIDE (___ram_data_end = .); _edata = .; PROVIDE (edata = .); \
    PROVIDE (___rom_data_end = LOADADDR (.data) + SIZEOF(.data));

#define SECTION_bss(_region_,  _vma_, _lma_) \
    .bss _vma_ : _lma_ \
    { ___bss_start = ABSOLUTE (.); \
    *(.scommon) *(.dynbss) *(.bss*) *(COMMON) \
    ___bss_end = ABSOLUTE (.); } \
    > _region_

#include <cyg/hal/plf_sections.h>

#include <pkgconf/hal_v85x.h>
#include <pkgconf/hal_v85x_v850.h>
#include CYGHWR_MEMORY_LAYOUT_LDI

// EOF v85x_v850.ld

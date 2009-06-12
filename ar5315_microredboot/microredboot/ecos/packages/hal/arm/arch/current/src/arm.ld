//=============================================================================
//
// MLT linker script for ARM
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
ENTRY(reset_vector)
INPUT(extras.o)
#if (__GNUC__ >= 3)
GROUP(libtarget.a libgcc.a libsupc++.a)
#else
GROUP(libtarget.a libgcc.a)
#endif

#if defined(__ARMEB__)
OUTPUT_FORMAT(elf32-bigarm)
#endif

#if defined(CYG_HAL_STARTUP_ROM)    \
    && defined(CYGPKG_HAL_ARM_PID)  \
    && defined(__ARMEB__)
// Note: This is only needed when the PID board is used in BE mode without
//       the proper ROM controller part.
//
// In big-endian mode, merge .RODATA sections into the .DATA section. This
// causes the data in these sections to be copied to RAM. When the CPU
// makes byte or halfword access to RAM, A0/A1 are modified correctly.
// This does not happen on ROM access (unless the proper ROM controller
// is used).
//
#define MERGE_IN_RODATA *(.rodata*)
#define CYGPRI_PID_BE_WORKAROUND
#else
// Keep RODATA in separate sections.
#define MERGE_IN_RODATA
#undef  CYGPRI_PID_BE_WORKAROUND
#endif

#define ALIGN_LMA 4
#define FOLLOWING(_section_) AT ((LOADADDR (_section_) + SIZEOF (_section_) + ALIGN_LMA - 1) & ~ (ALIGN_LMA - 1))
#define LMA_EQ_VMA
#define FORCE_OUTPUT . = .

// Some versions of gcc define "arm" which causes problems with .note.arm.ident
#undef arm
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
  .debug_macinfo  0 : { *(.debug_macinfo) }     \
  .note.arm.ident 0 : { KEEP (*(.note.arm.ident)) }

#define SECTION_fixed_vectors(_region_, _vma_, _lma_) \
    .fixed_vectors _vma_ : _lma_ \
    { FORCE_OUTPUT; KEEP (*(.fixed_vectors)) } \
    > _region_

#define SECTION_rom_vectors(_region_, _vma_, _lma_) \
    .rom_vectors _vma_ : _lma_ \
    { __rom_vectors_vma = ABSOLUTE(.); \
      FORCE_OUTPUT; KEEP (*(.vectors)) } \
    > _region_ \
    __rom_vectors_lma = LOADADDR(.rom_vectors);

#define SECTION_text(_region_, _vma_, _lma_) \
    .text _vma_ : _lma_ \
    { _stext = ABSOLUTE(.); \
    PROVIDE (__stext = ABSOLUTE(.)); \
    *(.text*) *(.gnu.warning) *(.gnu.linkonce.t.*) *(.init) \
    *(.glue_7) *(.glue_7t)  \
    } > _region_ \
    _etext = .; PROVIDE (__etext = .);

#define SECTION_fini(_region_, _vma_, _lma_) \
    .fini _vma_ : _lma_ \
    { FORCE_OUTPUT; *(.fini) } \
    > _region_

#if defined(CYGPRI_PID_BE_WORKAROUND)
#define SECTION_rodata(_region_, _vma_, _lma_)
#define SECTION_rodata1(_region_, _vma_, _lma_)
#else
#define SECTION_rodata(_region_, _vma_, _lma_) \
    .rodata _vma_ : _lma_ \
    { FORCE_OUTPUT; *(.rodata*) *(.gnu.linkonce.r.*) } \
    > _region_

#define SECTION_rodata1(_region_, _vma_, _lma_) \
    .rodata1 _vma_ : _lma_ \
    { FORCE_OUTPUT; *(.rodata1) } \
    > _region_
#endif // CYGPRI_PID_BE_WORKAROUND

#define SECTION_fixup(_region_, _vma_, _lma_) \
    .fixup _vma_ : _lma_ \
    { FORCE_OUTPUT; *(.fixup) } \
    > _region_

#define SECTION_gcc_except_table(_region_, _vma_, _lma_) \
    .gcc_except_table _vma_ : _lma_ \
    { FORCE_OUTPUT; *(.gcc_except_table) } \
    > _region_

#define SECTION_eh_frame(_region_, _vma_, _lma_)      \
  .eh_frame _vma_ : _lma_                             \
    {                                                 \
       FORCE_OUTPUT;  __EH_FRAME_BEGIN__ = .;         \
       KEEP(*(.eh_frame))                             \
       __FRAME_END__ = .;                             \
       . = . + 8;                                     \
    } > _region_ = 0

#define SECTION_RELOCS(_region_, _vma_, _lma_)                              \
  .rel.text      :                                                          \
    {                                                                       \
      *(.rel.text)                                                          \
      *(.rel.text.*)                                                        \
      *(.rel.gnu.linkonce.t*)                                               \
    } > _region_                                                            \
  .rela.text     :                                                          \
    {                                                                       \
      *(.rela.text)                                                         \
      *(.rela.text.*)                                                       \
      *(.rela.gnu.linkonce.t*)                                              \
    } > _region_                                                            \
  .rel.data      :                                                          \
    {                                                                       \
      *(.rel.data)                                                          \
      *(.rel.data.*)                                                        \
      *(.rel.gnu.linkonce.d*)                                               \
    } > _region_                                                            \
  .rela.data     :                                                          \
    {                                                                       \
      *(.rela.data)                                                         \
      *(.rela.data.*)                                                       \
      *(.rela.gnu.linkonce.d*)                                              \
    } > _region_                                                            \
  .rel.rodata    :                                                          \
    {                                                                       \
      *(.rel.rodata)                                                        \
      *(.rel.rodata.*)                                                      \
      *(.rel.gnu.linkonce.r*)                                               \
    } > _region_                                                            \
  .rela.rodata   :                                                          \
    {                                                                       \
      *(.rela.rodata)                                                       \
      *(.rela.rodata.*)                                                     \
      *(.rela.gnu.linkonce.r*)                                              \
    } > _region_                                                            \
  .rel.got       :   { *(.rel.got)    } > _region_                          \
  .rela.got      :   { *(.rela.got)   } > _region_                          \
  .rel.ctors     :   { *(.rel.ctors)  } > _region_                          \
  .rela.ctors    :   { *(.rela.ctors) } > _region_                          \
  .rel.dtors     :   { *(.rel.dtors)  } > _region_                          \
  .rela.dtors    :   { *(.rela.dtors) } > _region_                          \
  .rel.init      :   { *(.rel.init)   } > _region_                          \
  .rela.init     :   { *(.rela.init)  } > _region_                          \
  .rel.fini      :   { *(.rel.fini)   } > _region_                          \
  .rela.fini     :   { *(.rela.fini)  } > _region_                          \
  .rel.bss       :   { *(.rel.bss)    } > _region_                          \
  .rela.bss      :   { *(.rela.bss)   } > _region_                          \
  .rel.plt       :   { *(.rel.plt)    } > _region_                          \
  .rela.plt      :   { *(.rela.plt)   } > _region_                          \
  .rel.dyn       :   { *(.rel.dyn)    } > _region_

#define SECTION_got(_region_, _vma_, _lma_)                                 \
  .got _vma_ : _lma_                                                        \
    {                                                                       \
      FORCE_OUTPUT; *(.got.plt) *(.got)                                     \
      _GOT1_START_ = ABSOLUTE (.); *(.got1) _GOT1_END_ = ABSOLUTE (.);      \
      _GOT2_START_ = ABSOLUTE (.); *(.got2) _GOT2_END_ = ABSOLUTE (.);      \
    } > _region_

#define SECTION_mmu_tables(_region_, _vma_, _lma_) \
    .mmu_tables _vma_ : _lma_ \
    { FORCE_OUTPUT; *(.mmu_tables) } \
    > _region_

#define SECTION_sram(_region_, _vma_, _lma_) \
    .sram _vma_ : _lma_ \
    { FORCE_OUTPUT; *(.sram*) } \
    > _region_

#define SECTION_data(_region_,  _vma_, _lma_) \
    .data _vma_ : _lma_ \
    { __ram_data_start = ABSOLUTE (.); \
    *(.data*) *(.data1) *(.gnu.linkonce.d.*) MERGE_IN_RODATA \
    . = ALIGN (4); \
    KEEP(*( SORT (.ecos.table.*))) ;            \
    . = ALIGN (4); \
    __CTOR_LIST__ = ABSOLUTE (.); KEEP (*(SORT (.ctors*))) __CTOR_END__ = ABSOLUTE (.); \
    __DTOR_LIST__ = ABSOLUTE (.); KEEP (*(SORT (.dtors*))) __DTOR_END__ = ABSOLUTE (.); \
    *(.dynamic) *(.sdata*) *(.gnu.linkonce.s.*) \
    . = ALIGN (4); *(.2ram.*) } \
    > _region_ \
    __rom_data_start = LOADADDR (.data); \
    __ram_data_end = .; PROVIDE (__ram_data_end = .); _edata = .; PROVIDE (edata = .); \
    PROVIDE (__rom_data_end = LOADADDR (.data) + SIZEOF(.data));

#define SECTION_bss(_region_,  _vma_, _lma_) \
    .bss _vma_ : _lma_ \
    { __bss_start = ABSOLUTE (.); \
    *(.scommon) *(.dynsbss) *(.sbss*) *(.gnu.linkonce.sb.*) \
    *(.dynbss) *(.bss*) *(.gnu.linkonce.b.*) *(COMMON) \
    __bss_end = ABSOLUTE (.); } \
    > _region_

#define SECTIONS_END . = ALIGN(4); _end = .; PROVIDE (end = .);

#include <pkgconf/hal_arm.h>
#include CYGHWR_MEMORY_LAYOUT_LDI

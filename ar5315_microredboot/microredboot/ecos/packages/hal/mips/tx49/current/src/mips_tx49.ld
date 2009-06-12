//===========================================================================
//
// MLT linker script for MIPS TX49
//
//===========================================================================
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
//===========================================================================

#include <pkgconf/system.h>

OUTPUT_FORMAT("elf32-bigmips", "elf32-bigmips",
              "elf32-littlemips")
/* The preprocessor defines mips, but we know we're mips :-) */
#undef mips
OUTPUT_ARCH(mips:4000)

STARTUP(vectors.o)
ENTRY(reset_vector)
#if defined(EXTRAS)
INPUT(extras.o)
#endif
#if (__GNUC__ >= 3)
GROUP(libtarget.a libgcc.a libsupc++.a)
#else
GROUP(libtarget.a libgcc.a)
#endif

/* FIXME: The MLT should pass in the required alignment since it must be
 * the same as the VMA's alignment. As a result of this bug, all the REF4955
 * ROM mlt files have alignment 8, when some should have alignment 4
 * (902557-CR)
 */
#define ALIGN_LMA 8
#define FOLLOWING(_section_) AT ((LOADADDR (_section_) + SIZEOF (_section_) + ALIGN_LMA - 1) & ~ (ALIGN_LMA - 1))
#define LMA_EQ_VMA
#define FORCE_OUTPUT . = .

#define SECTIONS_BEGIN

#if defined(CYG_HAL_STARTUP_RAM)

/* this version for RAM startup */
#define SECTION_rom_vectors(_region_, _vma_, _lma_) \
    .rom_vectors _vma_ : _lma_ \
    { KEEP (*(.utlb_vector)) \
    . = ALIGN(0x80); KEEP(*(.other_vector)) \
        /* debug and reset vector not used in RAM version */ \
        KEEP(*(.debug_vector)) \
        KEEP (*(.reset_vector)) } \
    > _region_

#elif defined(CYG_HAL_STARTUP_ROM)

/* this version for ROM startup */
#define SECTION_rom_vectors(_region_, _vma_, _lma_) \
    .rom_vectors _vma_ : _lma_ \
    { KEEP (*(.reset_vector)) \
    . = ALIGN(0x200); KEEP (*(.utlb_vector)) \
    . = ALIGN(0x100); . = . + 4; \
    . = ALIGN(0x80); KEEP(*(.other_vector)) \
    . = ALIGN(0x100); KEEP(*(.debug_vector)) } \
    > _region_

#endif /* ROM startup version of ROM vectors */

#define SECTION_ROMISC(_region_, _vma_, _lma_)                                 \
  .interp _vma_  : _lma_ { *(.interp)                            } > _region_  \
  .hash          : FOLLOWING(.interp)        { *(.hash)          } > _region_  \
  .dynsym        : FOLLOWING(.hash)          { *(.dynsym)        } > _region_  \
  .dynstr        : FOLLOWING(.dynsym)        { *(.dynstr)        } > _region_  \
  .gnu.version   : FOLLOWING(.dynstr)        { *(.gnu.version)   } > _region_  \
  .gnu.version_d : FOLLOWING(.gnu.version)   { *(.gnu.version_d) } > _region_  \
  .gnu.version_r : FOLLOWING(.gnu.version_d) { *(.gnu.version_r) } > _region_  \
  .plt           : FOLLOWING(.gnu.version_r) { *(.plt)           } > _region_

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

#define SECTION_init(_region_, _vma_, _lma_)   \
  .init _vma_ : _lma_                          \
    {                                          \
      FORCE_OUTPUT; KEEP (*(.init))            \
    } > _region_ =0

#define SECTION_text(_region_, _vma_, _lma_)   \
  .text _vma_ : _lma_                          \
    {                                          \
      _stext = .; _ftext = . ;                 \
      *(.text)                                 \
      *(.text.*)                               \
      *(.stub)                                 \
      *(.gnu.warning)                          \
      *(.gnu.linkonce.t*)                      \
      *(.mips16.fn.*) *(.mips16.call.*)        \
    } > _region_ =0                            \
  _etext = .; PROVIDE (etext = .);

#define SECTION_fini(_region_, _vma_, _lma_)   \
  .fini _vma_ : _lma_                          \
    {                                          \
      FORCE_OUTPUT; KEEP (*(.fini))                   \
    } > _region_ =0

#define SECTION_rodata(_region_, _vma_, _lma_)        \
  .rodata _vma_ : _lma_                               \
    {                                                 \
      FORCE_OUTPUT; *(.rodata) *(.rodata.*) *(.gnu.linkonce.r*)    \
    } > _region_

#define SECTION_rodata1(_region_, _vma_, _lma_)       \
  .rodata1 _vma_ : _lma_                              \
    {                                                 \
     FORCE_OUTPUT; *(.rodata1) *(.rodata1.*)          \
    } > _region_

#define SECTION_vsr_table(_region_, _vma_, _lma_)     \
  .vsr_table _vma_ : _lma_                            \
    {                                                 \
      FORCE_OUTPUT; *(.vsr_table)                     \
    } > _region_

#define SECTION_data(_region_, _vma_, _lma_)          \
  .data _vma_ : _lma_                                 \
    {                                                 \
      __ram_data_start = ABSOLUTE (.); _fdata = . ;   \
      *(.data) *(.data.*) *(.gnu.linkonce.d*)         \
      *(.2ram.*)                                      \
    . = ALIGN (8);                                    \
    SORT(CONSTRUCTORS)                                \
    } > _region_                                      \
    __rom_data_start = LOADADDR(.data);

#define SECTION_data1(_region_, _vma_, _lma_)         \
  .data1 _vma_ : _lma_                                \
    {                                                 \
       FORCE_OUTPUT; *(.data1) *(.data1.*)            \
    } > _region_

#define SECTION_eh_frame(_region_, _vma_, _lma_)      \
  .eh_frame _vma_ : _lma_                             \
    {                                                 \
       FORCE_OUTPUT; *(.eh_frame)                     \
    } > _region_

#define SECTION_gcc_except_table(_region_, _vma_, _lma_) \
  .gcc_except_table _vma_ : _lma_                        \
    {                                                    \
      FORCE_OUTPUT; *(.gcc_except_table)                 \
    } > _region_


/* FIXME: We shouldn't need to define __CTOR_LIST__/__CTOR_END__
   and __DTOR_LIST__/__DTOR_END__ except by the PROVIDE lines.
   However this doesn't work for old (99r1-era) toolchains, so
   leave it for now. */

/* The KEEP(*_ctors.o(.ctors)) rule is included to prevent
   constructors from libgcc.a from turning up in the image.
   They are NULL anyway. */

#define SECTION_ctors(_region_, _vma_, _lma_)     \
  .ctors _vma_ : _lma_                            \
    {                                             \
      FORCE_OUTPUT;                               \
      KEEP (*crtbegin.o(.ctors))                  \
      KEEP (*_ctors.o(.ctors))                  \
      __CTOR_LIST__ = .;                \
      PROVIDE (__CTOR_LIST__ = .);                \
      KEEP (*(.ctors));                      \
      KEEP (*(SORT(.ctors.*)))                    \
      __CTOR_END__ = .;                 \
      PROVIDE (__CTOR_END__ = .);                 \
    } > _region_

#define SECTION_dtors(_region_, _vma_, _lma_)     \
  .dtors _vma_ : _lma_                            \
    {                                             \
      FORCE_OUTPUT;                               \
      KEEP (*crtbegin.o(.dtors))                  \
      __DTOR_LIST__ = .;                \
      PROVIDE (__DTOR_LIST__ = .);                \
      KEEP (*(SORT(.dtors.*)))                    \
      __DTOR_END__ = .;                 \
      KEEP (*(.dtors));                      \
      PROVIDE (__DTOR_END__ = .);                 \
    } > _region_

#define SECTION_devtab(_region_, _vma_, _lma_)    \
  .devtab _vma_ : _lma_                           \
    {                                             \
      FORCE_OUTPUT;                               \
      KEEP(*( SORT (.ecos.table.*))) ;            \
    } > _region_

#define SECTION_got(_region_, _vma_, _lma_)     \
  _gp = ALIGN(16) + 0x7ff0;                     \
  .got _vma_ : _lma_                            \
    {                                           \
      FORCE_OUTPUT; *(.got.plt) *(.got)         \
    } > _region_

#define SECTION_dynamic(_region_, _vma_, _lma_) \
  .dynamic _vma_ : _lma_                        \
    {                                           \
      FORCE_OUTPUT; *(.dynamic)                 \
    } > _region_

  /* We want the small data sections together, so single-instruction offsets
     can access them all, and initialized data all before uninitialized, so
     we can shorten the on-disk segment size.  */

#define SECTION_sdata(_region_, _vma_, _lma_)                 \
  .sdata _vma_ : _lma_                                        \
    {                                                         \
      FORCE_OUTPUT; *(.sdata) *(.sdata.*) *(.gnu.linkonce.s*) \
    } > _region_

#define SECTION_lit8(_region_, _vma_, _lma_)    \
  .lit8 _vma_ : _lma_                           \
    {                                           \
      FORCE_OUTPUT; *(.lit8)                    \
    } > _region_

#define SECTION_lit4(_region_, _vma_, _lma_)    \
  .lit4 : FOLLOWING(.lit8)                      \
    {                                           \
      FORCE_OUTPUT; *(.lit4)                    \
    } > _region_                                \
  __ram_data_end = .; _edata = . ;              \
  PROVIDE (edata = .);

#define SECTION_sbss(_region_, _vma_, _lma_)                    \
  __bss_start = .; _fbss = .;                                   \
  .sbss _vma_ : _lma_                                           \
    {                                                           \
      FORCE_OUTPUT; *(.dynsbss) *(.sbss) *(.sbss.*) *(.scommon) \
    } > _region_

#define SECTION_bss(_region_, _vma_, _lma_)       \
  .bss _vma_ : _lma_                              \
    {                                             \
      *(.dynbss) *(.bss) *(.bss.*) *(COMMON)      \
    } > _region_                                  \
  __bss_end = .;

/* The /DISCARD/ section ensures that the output will not contain a
 * .mdebug section as it confuses GDB. This is a workaround for CR 100804.
 */

#define SECTIONS_END . = ALIGN(4); _end = .; PROVIDE (end = .); \
  /* Stabs debugging sections.  */                              \
  .stab          0 : { *(.stab) }                               \
  .stabstr       0 : { *(.stabstr) }                            \
  .stab.excl     0 : { *(.stab.excl) }                          \
  .stab.exclstr  0 : { *(.stab.exclstr) }                       \
  .stab.index    0 : { *(.stab.index) }                         \
  .stab.indexstr 0 : { *(.stab.indexstr) }                      \
  .comment       0 : { *(.comment) }                            \
  /* DWARF debug sections.                                      \
     Symbols in the DWARF debugging sections are relative to    \
     the beginning of the section so we begin them at 0.  */    \
  /* DWARF 1 */                                                 \
  .debug          0 : { *(.debug) }                             \
  .line           0 : { *(.line) }                              \
  /* GNU DWARF 1 extensions */                                  \
  .debug_srcinfo  0 : { *(.debug_srcinfo) }                     \
  .debug_sfnames  0 : { *(.debug_sfnames) }                     \
  /* DWARF 1.1 and DWARF 2 */                                   \
  .debug_aranges  0 : { *(.debug_aranges) }                     \
  .debug_pubnames 0 : { *(.debug_pubnames) }                    \
  /* DWARF 2 */                                                 \
  .debug_info     0 : { *(.debug_info) }                        \
  .debug_abbrev   0 : { *(.debug_abbrev) }                      \
  .debug_line     0 : { *(.debug_line) }                        \
  .debug_frame    0 : { *(.debug_frame) }                       \
  .debug_str      0 : { *(.debug_str) }                         \
  .debug_loc      0 : { *(.debug_loc) }                         \
  .debug_macinfo  0 : { *(.debug_macinfo) }                     \
  /* SGI/MIPS DWARF 2 extensions */                             \
  .debug_weaknames 0 : { *(.debug_weaknames) }                  \
  .debug_funcnames 0 : { *(.debug_funcnames) }                  \
  .debug_typenames 0 : { *(.debug_typenames) }                  \
  .debug_varnames  0 : { *(.debug_varnames) }                   \
  /* These must appear regardless of  .  */                     \
  .gptab.sdata : { *(.gptab.data) *(.gptab.sdata) }             \
  .gptab.sbss : { *(.gptab.bss) *(.gptab.sbss) }                \
  /DISCARD/        0 : { *(.mdebug) }

#include CYGHWR_MEMORY_LAYOUT_LDI

// 0-0x200 reserved for vectors
hal_vsr_table = 0x80000200;
hal_virtual_vector_table = 0x80000300;

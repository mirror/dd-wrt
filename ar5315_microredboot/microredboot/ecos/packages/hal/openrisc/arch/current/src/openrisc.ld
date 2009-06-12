//==========================================================================
//
//      openrisc.ld
//
//      Linker script for OpenRISC architecture
//
//==========================================================================
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
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   sfurman
// Contributors:jskov
// Date:        2002-02-28
// Purpose:     OpenRISC Linker script
//
//####DESCRIPTIONEND####
//
//==========================================================================
STARTUP(vectors.o)
ENTRY(__exception_reset)
#ifdef EXTRAS
INPUT(extras.o)
#endif
#if (__GNUC__ >= 3)
GROUP(libtarget.a libgcc.a libsupc++.a)
#else
GROUP(libtarget.a libgcc.a)
#endif

#define ALIGN_LMA 8
#define FOLLOWING(_section_) AT ((LOADADDR (_section_) + SIZEOF (_section_) + ALIGN_LMA - 1) & ~ (ALIGN_LMA - 1))
#define LMA_EQ_VMA
#define FORCE_OUTPUT . = .


#define SECTIONS_BEGIN

#define SECTION_ROMISC(_region_, _vma_, _lma_)                                 \
  .interp _vma_  : _lma_ { *(.interp)                            } > _region_  \
  .hash          : FOLLOWING(.interp)        { *(.hash)          } > _region_  \
  .dynsym        : FOLLOWING(.hash)          { *(.dynsym)        } > _region_  \
  .dynstr        : FOLLOWING(.dynsym)        { *(.dynstr)        } > _region_  \
  .gnu.version   : FOLLOWING(.dynstr)        { *(.gnu.version)   } > _region_  \
  .gnu.version_d : FOLLOWING(.gnu.version)   { *(.gnu.version_d) } > _region_  \
  .gnu.version_r : FOLLOWING(.gnu.version_d) { *(.gnu.version_r) } > _region_  \
  .plt           : FOLLOWING(.gnu.version_r) { *(.plt)           } > _region_

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

#define SECTION_vectors(_region_, _vma_, _lma_)        \
  .vectors _vma_ : _lma_                                              \
  { FORCE_OUTPUT; KEEP(*(.vectors)) }         \
  > _region_

#define SECTION_init(_region_, _vma_, _lma_)   \
  .init _vma_ : _lma_                          \
    {                                          \
      FORCE_OUTPUT; KEEP (*(.init))            \
    } > _region_ =0

#define SECTION_fini(_region_, _vma_, _lma_)   \
  .fini _vma_ : _lma_                          \
    {                                          \
      FORCE_OUTPUT; KEEP (*(.fini))            \
    } > _region_ =0

// Code that might be executed out of either RAM or ROM
#define SECTION_text(_region_, _vma_, _lma_)                   \
  .text _vma_ : _lma_                                          \
  { __stext = .;                                               \
    *(.text*) *(.gnu.warning) *(.gnu.linkonce.t.*) }           \
  > _region_                                                   \
  __etext = .;  PROVIDE (etext = .);

// The .text.ram section is for performance-sensitive code that is
// always executed from RAM.  (If the code is loaded into ROM, it will
// be copied into RAM prior to execution.)
//
// Note: The SECTION_text_ram() macro must be listed in the linker
// include script (*.ldi file) prior to SECTION_text().
//
// All eCos code is put in the .text section unless explicitly placed
// into a section named section .text.ram*.

#define SECTION_text_ram(_region_, _vma_, _lma_)           \
  .text.ram _vma_ : _lma_                                  \
  { __stext_ram = .;                                       \
    __ram_text_start = ABSOLUTE(.);                        \
    *(.text.ram*)                                          \
  } > _region_                                             \
  __load_addr_text_ram = LOADADDR(.text.ram);              \
  __ram_text_end = .; PROVIDE(__ram_text_end = .);         \
  __etext_ram = .;  PROVIDE (etext_ram = .);

#define SECTION_rodata1(_region_, _vma_, _lma_)        \
    .rodata1 _vma_ : _lma_                             \
    { FORCE_OUTPUT; *(.rodata1) *(.rodata1*) }         \
    > _region_

#define SECTION_rodata(_region_, _vma_, _lma_)                     \
    .rodata _vma_ : _lma_                                          \
    { FORCE_OUTPUT;  *(.rodata) *(.rodata*) *(.gnu.linkonce.r*) }  \
    > _region_

#define SECTION_fixup(_region_, _vma_, _lma_)                      \
    .fixup _vma_ : _lma_                                                          \
    { __FIXUP_START__ = ABSOLUTE(.); *(.fixup) __FIXUP_END__ = ABSOLUTE(.);}\
    > _region_

#define SECTION_gcc_except_table(_region_, _vma_, _lma_)       \
    .gcc_except_table _vma_ : _lma_                                           \
    { __EXCEPT_START__ = ABSOLUTE(.); *(.gcc_except_table)              \
      __EXCEPT_END__ = ABSOLUTE(.);}                                    \
    > _region_

#define SECTION_ctors(_region_, _vma_, _lma_)     \
  .ctors _vma_ : _lma_                            \
    {                                             \
      FORCE_OUTPUT;                               \
      KEEP (*crtbegin.o(.ctors))                  \
      KEEP (*_ctors.o(.ctors))                    \
      __CTOR_LIST__ = .;                          \
      PROVIDE (__CTOR_LIST__ = .);                \
      KEEP (*(.ctors));                           \
      KEEP (*(SORT(.ctors.*)))                    \
      __CTOR_END__ = .;                           \
      PROVIDE (__CTOR_END__ = .);                 \
    } > _region_

#define SECTION_dtors(_region_, _vma_, _lma_)     \
  .dtors _vma_ : _lma_                            \
    {                                             \
      FORCE_OUTPUT;                               \
      KEEP (*crtbegin.o(.dtors))                  \
      __DTOR_LIST__ = .;                          \
      PROVIDE (__DTOR_LIST__ = .);                \
      KEEP (*(SORT(.dtors.*)))                    \
      __DTOR_END__ = .;                           \
      KEEP (*(.dtors));                           \
      PROVIDE (__DTOR_END__ = .);                 \
    } > _region_

#define SECTION_got(_region_, _vma_, _lma_)     \
  .got _vma_ : _lma_                            \
    {                                           \
      FORCE_OUTPUT; *(.got.plt) *(.got*)        \
    } > _region_

#define SECTION_dynamic(_region_, _vma_, _lma_) \
  .dynamic _vma_ : _lma_                        \
    {                                           \
      FORCE_OUTPUT; *(.dynamic)                 \
    } > _region_

#define SECTION_devtab(_region_, _vma_, _lma_)    \
  .devtab _vma_ : _lma_                           \
    {                                             \
      FORCE_OUTPUT;                               \
      KEEP(*( SORT (.ecos.table.*))) ;            \
    } > _region_

#define SECTION_data(_region_, _vma_, _lma_)                                  \
    .data _vma_ : _lma_                                                       \
    { __ram_data_start = ABSOLUTE(.);                                         \
      *(.data*)  *(.data.*) *(.gnu.linkonce.d*)                               \
      *( .2ram.*)                                                             \
      . = ALIGN (8);                                                          \
      SORT(CONSTRUCTORS)                                                      \
    } > _region_                                                              \
    __rom_data_start = LOADADDR(.data);                                       

  /* We want the small data sections together, so single-instruction offsets
     can access them all, and initialized data all before uninitialized, so
     we can shorten the on-disk segment size.  */

#define SECTION_sdata(_region_, _vma_, _lma_)                 \
  .sdata _vma_ : _lma_                                        \
    {                                                         \
    __SDATA_START__ = ABSOLUTE(.);                            \
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

#define SECTION_sbss(_region_, _vma_, _lma_)                     \
    .sbss _vma_ : _lma_                                                         \
    { __sbss_start = ABSOLUTE (.);                                        \
    __SBSS_START__ = ABSOLUTE(.); *(.sbss.*) __SBSS_END__ = ABSOLUTE(.);   \
    __SBSSx_START__ = ABSOLUTE(.); *(.sbss*) __SBSSx_END__ = ABSOLUTE(.);\
    *(.scommon*)                                                          \
    __sbss_end = ABSOLUTE (.); }                                          \
    > _region_

#define SECTION_bss(_region_, _vma_, _lma_)                      \
    .bss _vma_ : _lma_                                                          \
    { __bss_start = ABSOLUTE (.);                           \
      FORCE_OUTPUT; *(.dynbss*) *(.bss) *(.bss.*) *(COMMON) \
      __bss_end = ABSOLUTE (.); }                           \
    > _region_

#define SECTIONS_END . = ALIGN(4); _end = .; PROVIDE (end = .);

#include <pkgconf/system.h>
#include CYGHWR_MEMORY_LAYOUT_LDI

// Define VSR and virtual tables to reside at fixed addresses.
#include CYGBLD_HAL_TARGET_H
_hal_vsr_table = CYGHWR_HAL_VSR_TABLE;
_hal_virtual_vector_table = CYGHWR_HAL_VIRTUAL_VECTOR_TABLE;

// EOF openrisc.ld

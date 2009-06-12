/*==========================================================================
//
//	m68k.ld
//
//	Linker script
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
*/

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

#define ALIGN_LMA 4
#define FOLLOWING(_section_) AT ((LOADADDR (_section_) + SIZEOF (_section_) + ALIGN_LMA - 1) & ~ (ALIGN_LMA - 1))
#define LMA_EQ_VMA
#define FORCE_OUTPUT . = .

#ifndef CYG_LABEL_DEFN
# define CYG_LABEL_DEFN(_label) _label
#endif

#define SECTIONS_BEGIN

#define SECTION_text(_region_, _vma_, _lma_)           \
  .text _vma_ : _lma_                                                 \
  { _stext = .;       \
    *(.text*) *(.gnu.warning) *(.gnu.linkonce*) *(.init) }      \
  > _region_                                                    \
  _etext = .;  PROVIDE (etext = .);

#define SECTION_fini(_region_, _vma_, _lma_)   \
    .fini _vma_ : _lma_                                       \
    { FORCE_OUTPUT; *(.fini) }        \
    > _region_

#define SECTION_rodata1(_region_, _vma_, _lma_)        \
    .rodata1 _vma_ : _lma_                                            \
    { FORCE_OUTPUT; *(.rodata1) }             \
    > _region_

#define SECTION_rodata(_region_, _vma_, _lma_) \
    .rodata _vma_ : _lma_                                     \
    { FORCE_OUTPUT; *(.rodata*) }     \
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

#define SECTION_data(_region_, _vma_, _lma_, _rom_data_addr_)                 \
    .data _vma_ : _lma_                                                       \
    {                                                                         \
    __ram_data_start = ABSOLUTE(.);                                           \
    *(.data*)                                                                 \
    __GOT1_START__ = ABSOLUTE(.); *(.got1) __GOT1_END__ = ABSOLUTE(.);        \
    . = ALIGN (4);                                                            \
    KEEP(*( SORT (.ecos.table.*)));                                           \
    . = ALIGN (4);                                                            \
    __CTOR_LIST__ = ABSOLUTE(.);                                              \
    KEEP(*(SORT(.ctors*)));                                                   \
    __CTOR_END__ = ABSOLUTE(.);                                               \
    __DTOR_LIST__ = ABSOLUTE(.);                                              \
    KEEP(*(SORT(.dtors*)))                                                    \
    __DTOR_END__ = ABSOLUTE(.);                                               \
    . = ALIGN(8);                                                             \
    __GOT2_START__ = ABSOLUTE(.); *(.got2) __GOT2_END__ = ABSOLUTE(.);        \
    __GOT_START = ABSOLUTE(.); _GLOBAL_OFFSET_TABLE_ = ABSOLUTE(. + 32768);   \
    _SDA_BASE_ = ABSOLUTE(.); *(.got.plt) *(.got)                             \
    __GOT_END__ = ABSOLUTE(.);  *(.dynamic)                                   \
    *(.eh_frame)                                                              \
      /* We want the small data sections together, so single-instruction */   \
      /* offsets can access them all, and initialized data all before    */   \
      /* uninitialized, so we can shorten the on-disk segment size.      */   \
    __SDATA_START__ = ABSOLUTE(.); *(.sdata)                                  \
    __ram_data_end = ABSOLUTE(.);                                             \
    __ram_data_size = ABSOLUTE (.) - ABSOLUTE(__ram_data_start);              \
    }                                                                         \
    > _region_                                                                \
    __rom_data_start = ABSOLUTE(_rom_data_addr_);                             \
    __rom_data_size = SIZEOF(.data);                                          \
    __rom_data_end = __rom_data_start + __rom_data_size;

#define SECTION_sbss(_region_, _vma_, _lma_)                                    \
    .sbss _vma_ : _lma_                                                         \
    { __sbss_start = ABSOLUTE (.);                                              \
      __SBSS_START__ = ABSOLUTE(.); *(.sbss) __SBSS_END__ = ABSOLUTE(.);        \
      *(.scommon)                                                               \
      __sbss_end = ABSOLUTE (.);                                                \
      __sbss_size = ABSOLUTE (.) - ABSOLUTE(__sbss_start);                      \
    }                                                                           \
    > _region_

#define SECTION_bss(_region_, _vma_, _lma_)                                     \
    .bss _vma_ : _lma_                                                          \
    { __bss_start = ABSOLUTE (.);                                               \
      FORCE_OUTPUT; *(.dynbss) *(.bss) *(COMMON)                                \
      __bss_end = ABSOLUTE (.);                                                 \
      __bss_size = ABSOLUTE (.) - ABSOLUTE(__bss_start);                        \
    }                                                                           \
    > _region_

#define SECTION_stab \
    .stab 0 (NOLOAD) : \
    { \
        *(.stab) \
    }

#define SECTION_stabstr \
    .stabstr 0 (NOLOAD) : \
    { \
        *(.stabstr) \
    }

#define SECTION_comment \
    .comment 0 (NOLOAD) : \
    { \
        *(.comment) \
    }

#define SECTION_heap1(_region_, _vma_, _lma_, _minsize_)                        \
    .heap1 _vma_ : _lma_                                                        \
    { __heap1_start = ABSOLUTE (.);                                             \
      . += (_minsize_);                                                         \
      __heap1_end = ABSOLUTE (.);                                               \
      __heap1_size = ABSOLUTE (.) - ABSOLUTE(__heap1_start);                    \
    }                                                                           \
    > _region_

#define SECTION_uninvar(_region_, _vma_, _lma_) \
    .uninvar _vma_ : _lma_ \
    { __uninvar_start = ABSOLUTE (.); \
      FORCE_OUTPUT; *(.uninvar) \
      __uninvar_end = ABSOLUTE (.); \
      __uninvar_size = ABSOLUTE (.) - ABSOLUTE(__uninvar_start); \
    } \
    > _region_

#define GENERIC_SECTION(_name_, _region_, _vma_, _lma_, _minsize_)              \
    . ## _name_ _vma_ : _lma_                                                   \
    { __ ## _name_ ## _start = ABSOLUTE (.);                                    \
      FORCE_OUTPUT; *(. ## _name_ ## )                                              \
      __ ## _name_ ## _datasize = (ABSOLUTE (.) - ABSOLUTE(__ ## _name_ ## _start));      \
      . += (_minsize_) > (__ ## _name_ ## _datasize) ? (_minsize_) - (__ ## _name_ ## _datasize) : 0; \
      __ ## _name_ ## _end = ABSOLUTE (.);                                      \
      __ ## _name_ ## _size = (ABSOLUTE (.) - ABSOLUTE(__ ## _name_ ## _start));          \
    }                                                                           \
    > _region_

#define SECTIONS_END \
    SECTION_stab \
    SECTION_stabstr \
    SECTION_comment \
    . = ALIGN(4); _end = .; PROVIDE (end = .);

#include <pkgconf/system.h>
#include CYGHWR_MEMORY_LAYOUT_LDI


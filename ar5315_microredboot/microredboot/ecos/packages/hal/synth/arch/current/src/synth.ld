//==========================================================================
//
//      synth.ld
//
//      Linker script for the synthetic target
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
// Author(s):   jskov
// Contributors:jskov, bartv
// Date:        1999-01-18
// Purpose:     synthetic target linker script
//
//####DESCRIPTIONEND####
//
//==========================================================================
STARTUP(vectors.o)
ENTRY(_start)
#ifdef EXTRAS
INPUT(extras.o)
#endif
#if (__GNUC__ >= 3)
GROUP(libtarget.a libgcc.a libsupc++.a libgcc_eh.a)
#else
GROUP(libtarget.a libgcc.a)
#endif

#define ALIGN_LMA 8
#define FOLLOWING(_section_) AT ((LOADADDR (_section_) + SIZEOF (_section_) + ALIGN_LMA - 1) & ~ (ALIGN_LMA - 1))
#define LMA_EQ_VMA
#define FORCE_OUTPUT . = .


#define SECTIONS_BEGIN

#define SECTION_vectors(_region_, _vma_, _lma_)         \
  .vectors _vma_ : _lma_                                \
  { FORCE_OUTPUT; KEEP(*(.vectors)) }                   \
  > _region_

#define SECTION_text(_region_, _vma_, _lma_)                    \
  .text _vma_ : _lma_                                           \
  { _stext = .;                                                 \
    *(.text*) *(.gnu.warning) *(.gnu.linkonce.t.*) *(.init) }   \
  > _region_                                                    \
  _etext = .;  PROVIDE (etext = .);

#define SECTION_fini(_region_, _vma_, _lma_)                    \
    .fini _vma_ : _lma_                                         \
    { FORCE_OUTPUT; *(.fini) }                                  \
    > _region_

#define SECTION_rodata1(_region_, _vma_, _lma_)                 \
    .rodata1 _vma_ : _lma_                                      \
    { FORCE_OUTPUT; *(.rodata1) }                               \
    > _region_

#define SECTION_rodata(_region_, _vma_, _lma_)                  \
    .rodata _vma_ : _lma_                                       \
    { FORCE_OUTPUT; *(.rodata*) *(.gnu.linkonce.r.*) }          \
    > _region_

#define SECTION_fixup(_region_, _vma_, _lma_)                   		\
    .fixup _vma_ : _lma_                                                       	\
    { _FIXUP_START_ = ABSOLUTE(.); *(.fixup) _FIXUP_END_ = ABSOLUTE(.);} 	\
    > _region_

#define SECTION_gcc_except_table(_region_, _vma_, _lma_)       			\
    .gcc_except_table _vma_ : _lma_                                           	\
    { _EXCEPT_START_ = ABSOLUTE(.); *(.gcc_except_table)                	\
      _EXCEPT_END_ = ABSOLUTE(.);}                                      	\
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

// Note: The __ in the name is an encoding of the .
#define SECTION_rel__got(_region_, _vma_, _lma_)        			\
    .rel.got _vma_ : _lma_                                            		\
    { *(.rel.got)      }                                        		\
    > _region_

#define SECTION_data(_region_, _vma_, _lma_)                                    \
    .data _vma_ : _lma_                                                         \
    { __ram_data_start = ABSOLUTE(.); *(.data*) *(.gnu.linkonce.d.*)            \
    _GOT1_START_ = ABSOLUTE(.); *(.got1) _GOT1_END_ = ABSOLUTE(.);              \
      /* Put .ctors and .dtors next to the .got2 section, so that */            \
      /* the pointers get relocated with -mrelocatable.           */            \
     . = ALIGN(8); __CTOR_LIST__ = ABSOLUTE(.);                                 \
      KEEP(*(SORT(.ctors*))) __CTOR_END__ = ABSOLUTE(.);                        \
     __DTOR_LIST__ = ABSOLUTE(.);                                            	\
       KEEP(*(SORT(.dtors*))) __DTOR_END__ = ABSOLUTE(.);                    	\
    . = ALIGN(32);                                                              \
    KEEP(*( SORT (.ecos.table.*)));                                             \
    _GOT2_START_ = ABSOLUTE(.); *(.got2) _GOT2_END_ = ABSOLUTE(.);              \
    _GOT_START_ = ABSOLUTE(.); _GLOBAL_OFFSET_TABLE_ = ABSOLUTE(. + 32768);     \
    _SDA_BASE_ = ABSOLUTE(.); *(.got.plt) *(.got)                               \
    _GOT_END_ = ABSOLUTE(.);  *(.dynamic)                                       \
      /* We want the small data sections together, so single-instruction */     \
      /* offsets can access them all, and initialized data all before    */     \
      /* uninitialized, so we can shorten the on-disk segment size.      */     \
    _SDATA_START_ = ABSOLUTE(.); *(.sdata*) *(.gnu.linkonce.s.*) }              \
    > _region_                                                                  \
    __rom_data_start = LOADADDR(.data);                                         \
    __ram_data_end = .; PROVIDE(__ram_data_end = .);                            \
    _edata = .; PROVIDE (edata = .);

#define SECTION_sbss(_region_, _vma_, _lma_)                                    \
    .sbss _vma_ : _lma_                                                         \
    { __bss_start = ABSOLUTE (.);                                               \
    _SBSS_START_ = ABSOLUTE(.); *(.sbss*) *(.gnu.linkonce.sb.*)                 \
    _SBSS_END_ = ABSOLUTE(.);                                                   \
    *(.scommon*) }                                                              \
    > _region_

#define SECTION_bss(_region_, _vma_, _lma_)                                     \
    .bss _vma_ : _lma_                                                          \
    { FORCE_OUTPUT; *(.dynbss*) *(.bss*) *(COMMON) *(.gnu.linkonce.b.*) }       \
    > _region_                                                                  \
    __bss_end = .;

#define SECTIONS_END . = ALIGN(4); _end = .; PROVIDE (end = .);

#include <pkgconf/system.h>
#include CYGHWR_MEMORY_LAYOUT_LDI

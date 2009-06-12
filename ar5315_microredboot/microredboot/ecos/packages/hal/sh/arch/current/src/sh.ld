/*==========================================================================
//
//	sh.ld
//
//	Linker script for SH
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
// Author(s): 	jskov
// Contributors:jskov
// Date:	1999-04-24
// Purpose:	SH Linker script
//
//####DESCRIPTIONEND####
//
//========================================================================*/

#include <pkgconf/system.h>
#include <cyg/hal/basetype.h>

STARTUP(vectors.o)
ENTRY(CYG_LABEL_DEFN(_reset))
INPUT(extras.o)
#if (__GNUC__ >= 3)
GROUP(libtarget.a libgcc.a libsupc++.a)
#else
GROUP(libtarget.a libgcc.a)
#endif

#define ALIGN_LMA 16
#define FOLLOWING(_section_) AT ((LOADADDR (_section_) + SIZEOF (_section_) + ALIGN_LMA - 1) & ~ (ALIGN_LMA - 1))
#define LMA_EQ_VMA
#define FORCE_OUTPUT . = .


# define SECTIONS_BEGIN

# define SECTION_vectors(_region_, _vma_, _lma_)		\
  .vectors _vma_ : _lma_					\
  { FORCE_OUTPUT; KEEP(*(.vectors))				\
  }								\
  > _region_							\
  CYG_LABEL_DEFN(_vector_code_lma) = LOADADDR(.vectors);

# define SECTION_text(_region_, _vma_, _lma_)			\
  .text _vma_ : _lma_						\
  { CYG_LABEL_DEFN(_stext) = .;					\
    *(.text*) *(.gnu.warning) *(.gnu.linkonce*) *(.init) }	\
  > _region_							\
  CYG_LABEL_DEFN(_etext) = .;  PROVIDE (__etext = .);

# define SECTION_data(_region_, _vma_, _lma_)				 \
    .data _vma_ : _lma_							 \
    { CYG_LABEL_DEFN(_ram_data_start) = ABSOLUTE(.); *(.data*)		 \
      *( .2ram.*) ;                                                      \
     . = ALIGN(8); CYG_LABEL_DEFN(__CTOR_LIST__) = ABSOLUTE(.);		 \
      KEEP(*(SORT(.ctors*))) CYG_LABEL_DEFN(__CTOR_END__) = ABSOLUTE(.); \
    CYG_LABEL_DEFN(__DTOR_LIST__) = ABSOLUTE(.);			 \
      KEEP(*(SORT(.dtors*))) CYG_LABEL_DEFN(__DTOR_END__) = ABSOLUTE(.); \
    . = ALIGN(8);							 \
      KEEP(*( SORT (.ecos.table.*))) ;					 \
    . = ALIGN(4);                                                        \
     *(.stack)                                                           \
    } > _region_							 \
    CYG_LABEL_DEFN(_rom_data_start) = LOADADDR(.data);			 \
    CYG_LABEL_DEFN(_ram_data_end) = .; PROVIDE(_ram_data_end = .);	 \
    CYG_LABEL_DEFN(edata) = .; PROVIDE (edata = .);


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
    } > _region_

#define SECTION_fini(_region_, _vma_, _lma_)	\
    .fini _vma_ : _lma_				\
    { FORCE_OUTPUT; *(.fini) }			\
    > _region_

#define SECTION_rodata1(_region_, _vma_, _lma_)	\
    .rodata1 _vma_ : _lma_			\
    { FORCE_OUTPUT; *(.rodata1) }		\
    > _region_

#define SECTION_rodata(_region_, _vma_, _lma_)	\
    .rodata _vma_ : _lma_			\
    { FORCE_OUTPUT; *(.rodata*) }		\
    > _region_

#define SECTION_fixup(_region_, _vma_, _lma_)				     \
    .fixup _vma_ : _lma_						     \
    { __FIXUP_START__ = ABSOLUTE(.); *(.fixup) __FIXUP_END__ = ABSOLUTE(.);} \
    > _region_

#define SECTION_gcc_except_table(_region_, _vma_, _lma_)	\
    .gcc_except_table _vma_ : _lma_				\
    { __EXCEPT_START__ = ABSOLUTE(.); *(.gcc_except_table)	\
      __EXCEPT_END__ = ABSOLUTE(.);}				\
    > _region_

#define SECTION_bss(_region_, _vma_, _lma_)		\
    .bss _vma_ : _lma_					\
    { CYG_LABEL_DEFN(_bss_start) = ABSOLUTE (.);	\
      FORCE_OUTPUT; *(.dynbss) *(.bss*) *(COMMON)	\
      . = ALIGN(4);                                     \
      CYG_LABEL_DEFN(_bss_end) = ABSOLUTE (.); }	\
    > _region_

# define SECTION_entry(_region_, _vma_, _lma_)		\
  .entry _vma_ : _lma_					\
  { 							\
	KEEP(*(.entry))					\
  }							\
  > _region_

#define SECTIONS_END . = ALIGN(4); CYG_LABEL_DEFN(end) = .; PROVIDE (end = .);

#include <pkgconf/system.h>
#include CYGHWR_MEMORY_LAYOUT_LDI
#include CYGBLD_HAL_PLATFORM_H

// Define VSR and vector tables to reside at fixed address.
CYG_LABEL_DEFN(hal_vsr_table) = CYGHWR_HAL_VSR_TABLE;
CYG_LABEL_DEFN(hal_virtual_vector_table) = CYGHWR_HAL_VECTOR_TABLE;

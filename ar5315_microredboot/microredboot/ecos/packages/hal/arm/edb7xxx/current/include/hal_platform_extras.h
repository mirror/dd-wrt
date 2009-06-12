#ifndef CYGONCE_HAL_PLATFORM_EXTRAS_H
#define CYGONCE_HAL_PLATFORM_EXTRAS_H

/*=============================================================================
//
//      hal_platform_extras.h
//
//      Platform specific support for HAL (assembly code)
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
//#####DESCRIPTIONBEGIN####
//
// Author(s):    gthomas
// Contributors: gthomas
// Date:         1999-11-23
// Purpose:      Cirrus EDB7XXX platform extras, in particular MMU tables
// Description: 
// Usage:       #include <cyg/hal/hal_platform_extras.h>
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

        .section ".mmu_tables","aw"

#define NEXT_PAGE                \
        .long   PTE                     ; \
        .set    PTE,PTE+MMU_PAGE_SIZE
_PT_0x0:
#if (CYGHWR_HAL_ARM_EDB7XXX_DRAM_SIZE == 2)
#err Static MMU tables not yet available for 2M boards.
#elif (CYGHWR_HAL_ARM_EDB7XXX_DRAM_SIZE == 16)
        .set    PTE,(DRAM_PA_START+LCD_BUFFER_SIZE)|MMU_L2_TYPE_Small\
                    |MMU_AP_Any|MMU_Bufferable|MMU_Cacheable
        .rept   (0x800000-LCD_BUFFER_SIZE)/0x1000
        NEXT_PAGE
        .endr
        .set    PTE,(DRAM_PA_START+0x01000000)|MMU_L2_TYPE_Small\
                    |MMU_AP_Any|MMU_Bufferable|MMU_Cacheable
        .rept   0x800000/0x1000
        NEXT_PAGE
        .endr
#endif
        .balign 0x0400

// Too bad these macros don't work ['as' bug?] :-(

#define NEXT_SECTION                                            \
        .long   PTE+OFF                                        ;\
        .set    OFF,OFF+(MMU_SECTION_SIZE/MMU_PAGE_SIZE)*4

#define FILL_SEGMENT_(base)                     \
        .set    PTE,base                        \
        .set    OFF,0x00000000                  \
        .rept   0x10000000/MMU_SECTION_SIZE     \
            NEXT_SECTION                        \
        .endr

#define OFF     ((MMU_SECTION_SIZE/MMU_PAGE_SIZE)*4)

#define FILL_16M_SEGMENT(base,seg)              \
        .long   base+(OFF*(seg+0x00));          \
        .long   base+(OFF*(seg+0x01));          \
        .long   base+(OFF*(seg+0x02));          \
        .long   base+(OFF*(seg+0x03));          \
        .long   base+(OFF*(seg+0x04));          \
        .long   base+(OFF*(seg+0x05));          \
        .long   base+(OFF*(seg+0x06));          \
        .long   base+(OFF*(seg+0x07));          \
        .long   base+(OFF*(seg+0x08));          \
        .long   base+(OFF*(seg+0x09));          \
        .long   base+(OFF*(seg+0x0A));          \
        .long   base+(OFF*(seg+0x0B));          \
        .long   base+(OFF*(seg+0x0C));          \
        .long   base+(OFF*(seg+0x0D));          \
        .long   base+(OFF*(seg+0x0E));          \
        .long   base+(OFF*(seg+0x0F));

#define FILL_256M_SEGMENT(base)                 \
        FILL_16M_SEGMENT(base,0x00);            \
        FILL_16M_SEGMENT(base,0x10);            \
        FILL_16M_SEGMENT(base,0x20);            \
        FILL_16M_SEGMENT(base,0x30);            \
        FILL_16M_SEGMENT(base,0x40);            \
        FILL_16M_SEGMENT(base,0x50);            \
        FILL_16M_SEGMENT(base,0x60);            \
        FILL_16M_SEGMENT(base,0x70);            \
        FILL_16M_SEGMENT(base,0x80);            \
        FILL_16M_SEGMENT(base,0x90);            \
        FILL_16M_SEGMENT(base,0xA0);            \
        FILL_16M_SEGMENT(base,0xB0);            \
        FILL_16M_SEGMENT(base,0xC0);            \
        FILL_16M_SEGMENT(base,0xD0);            \
        FILL_16M_SEGMENT(base,0xE0);            \
        FILL_16M_SEGMENT(base,0xF0);

        .balign 0x4000
_MMU_table:     
        FILL_16M_SEGMENT(_PT_0x0-0xE0000000+MMU_L1_TYPE_Page,0x00)
        FILL_16M_SEGMENT(MMU_L1_TYPE_Fault,0x00)
        FILL_16M_SEGMENT(MMU_L1_TYPE_Fault,0x00)
        FILL_16M_SEGMENT(MMU_L1_TYPE_Fault,0x00)
        FILL_16M_SEGMENT(MMU_L1_TYPE_Fault,0x00)
        FILL_16M_SEGMENT(MMU_L1_TYPE_Fault,0x00)
        FILL_16M_SEGMENT(MMU_L1_TYPE_Fault,0x00)
        FILL_16M_SEGMENT(MMU_L1_TYPE_Fault,0x00)
        FILL_16M_SEGMENT(MMU_L1_TYPE_Fault,0x00)
        FILL_16M_SEGMENT(MMU_L1_TYPE_Fault,0x00)
        FILL_16M_SEGMENT(MMU_L1_TYPE_Fault,0x00)
        FILL_16M_SEGMENT(MMU_L1_TYPE_Fault,0x00)
        FILL_16M_SEGMENT(MMU_L1_TYPE_Fault,0x00)
        FILL_16M_SEGMENT(MMU_L1_TYPE_Fault,0x00)
        FILL_16M_SEGMENT(MMU_L1_TYPE_Fault,0x00)
        FILL_16M_SEGMENT(MMU_L1_TYPE_Fault,0x00)
        FILL_256M_SEGMENT(MMU_L1_TYPE_Fault)
        FILL_256M_SEGMENT(EXPANSION2_PA|MMU_L1_TYPE_Section|MMU_AP_Any)
        FILL_256M_SEGMENT(EXPANSION3_PA|MMU_L1_TYPE_Section|MMU_AP_Any)
        FILL_256M_SEGMENT(MMU_L1_TYPE_Fault)
        FILL_256M_SEGMENT(MMU_L1_TYPE_Fault)
        FILL_256M_SEGMENT(SRAM_PA|MMU_L1_TYPE_Section|MMU_AP_Any)
        FILL_256M_SEGMENT(MMU_L1_TYPE_Fault)
        FILL_256M_SEGMENT(IO_PA|MMU_L1_TYPE_Section|MMU_AP_Any)
        FILL_256M_SEGMENT(MMU_L1_TYPE_Fault)
        FILL_256M_SEGMENT(MMU_L1_TYPE_Fault)
        FILL_256M_SEGMENT(MMU_L1_TYPE_Fault)
        FILL_256M_SEGMENT(DRAM_PA|MMU_L1_TYPE_Section|MMU_AP_Any)
        FILL_256M_SEGMENT(MMU_L1_TYPE_Fault)
        FILL_256M_SEGMENT(ROM0_PA|MMU_L1_TYPE_Section|MMU_AP_Any)
        FILL_256M_SEGMENT(ROM1_PA|MMU_L1_TYPE_Section|MMU_AP_Any)

#endif // CYGONCE_HAL_PLATFORM_EXTRAS_H

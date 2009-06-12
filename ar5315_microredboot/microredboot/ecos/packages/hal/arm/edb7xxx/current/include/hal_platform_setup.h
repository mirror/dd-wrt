#ifndef CYGONCE_HAL_PLATFORM_SETUP_H
#define CYGONCE_HAL_PLATFORM_SETUP_H

/*=============================================================================
//
//      hal_platform_setup.h
//
//      Platform specific support for HAL (assembly code)
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2003 Gary Thomas <gary@mind.be>
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
// Date:         1999-04-21
// Purpose:      Cirrus EDB7XXX platform specific support routines
// Description: 
// Usage:       #include <cyg/hal/hal_platform_setup.h>
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <pkgconf/system.h>           // System-wide configuration info
#include <pkgconf/hal.h>              // Architecture independent configuration
#include CYGBLD_HAL_PLATFORM_H        // Platform specific configuration
#include <cyg/hal/hal_edb7xxx.h>      // Platform specific hardware definitions
#include <cyg/hal/hal_mmu.h>          // MMU definitions

#define CYGHWR_HAL_ARM_HAS_MMU        // This processor has an MMU
#define CYGSEM_HAL_ROM_RESET_USES_JUMP
//
// Memory map - set up by ROM (GDB stubs)
//
// Region        Logical Address              Physical Address
//   DRAM          0x00000000..0x00xFFFFF        0xC00x0000  (see below)
//   Expansion 2   0x20000000                    0x20000000
//   Expansion 3   0x30000000                    0x30000000
//   PCMCIA 0      0x40000000                    0x40000000
//   PCMCIA 1      0x50000000                    0x50000000
//   SRAM          0x60000000..0x600007FF        0x60000000
//   I/O           0x80000000                    0x80000000
//   MMU Tables                                  0xC00y0000
//   LCD buffer    0xC0000000..0xC001FFFF        0xC0000000
//   ROM           0xE0000000..0xEFFFFFFF        0x00000000
//   ROM           0xF0000000..0xFFFFFFFF        0x10000000

#ifdef CYGHWR_HAL_ARM_EDB7XXX_LCD_INSTALLED
#define LCD_BUFFER_SIZE  0x00020000
#else
#define LCD_BUFFER_SIZE  0x00000000
#endif
#define DRAM_PA_START    0xC0000000
#define MMU_BASE         DRAM_PA_START+LCD_BUFFER_SIZE
#define PTE_BASE         MMU_BASE+0x4000
#if (CYGHWR_HAL_ARM_EDB7XXX_DRAM_SIZE == 2)
#define MMU_TABLES_SIZE  (0x4000+0x1000+0x1000)   // RAM used for PTE entries
#define DRAM_LA_END      (0x00200000-MMU_TABLES_SIZE-LCD_BUFFER_SIZE)
#elif (CYGHWR_HAL_ARM_EDB7XXX_DRAM_SIZE == 16)
#define MMU_TABLES_SIZE  (0x4000+0x4000+0x1000)  // RAM used for PTE entries
#define DRAM_LA_END      (0x01000000-MMU_TABLES_SIZE-LCD_BUFFER_SIZE)
#endif
#define DRAM_LA_START    0x00000000
#define DRAM_PA          MMU_BASE+MMU_TABLES_SIZE
#define LCD_LA_START     0xC0000000
#define LCD_LA_END       0xC0020000
#define LCD_PA           0xC0000000
#define ROM0_LA_START    0xE0000000
#define ROM0_PA          0x00000000
#if defined (__EDB7211) 
#define ROM0_LA_END      0xE0800000
#else
#define ROM0_LA_END      0xF0000000
#endif
#define ROM1_LA_START    ROM0_LA_END 
#define ROM1_LA_END      0x00000000
#define ROM1_PA          0x10000000
#define EXPANSION2_LA_START 0x20000000
#define EXPANSION2_PA       0x20000000
#define EXPANSION3_LA_START 0x30000000
#define EXPANSION3_PA       0x30000000
#define PCMCIA0_LA_START 0x40000000
#define PCMCIA0_PA       0x40000000
#define PCMCIA1_LA_START 0x50000000
#define PCMCIA1_PA       0x50000000
#define SRAM_LA_START    0x60000000
#ifdef CYGHWR_HAL_ARM_EDB7XXX_VARIANT_CL_PS7111 // 4K SRAM
#define SRAM_LA_END      0x60001000
#else  // 72xx - 37.5K SRAM
#define SRAM_LA_END      0x6000A000
#endif
#define SRAM_PA          0x60000000
#define IO_LA_START      0x80000000
#define IO_LA_END        0x8000f000
#define IO_PA            0x80000000

#ifndef _CYGHWR_LAYOUT_ONLY
// Define startup code [macros]
#if defined(CYGSEM_HAL_INSTALL_MMU_TABLES)

#ifdef CYGHWR_HAL_ARM_EDB7XXX_VARIANT_CL_PS7111 // CL7111, 710 processor
        .macro MMU_INITIALIZE
	ldr	r2,=MMU_Control_Init                                    
	mcr	MMU_CP,0,r2,MMU_Control,c0	/* MMU off */           
	mcr	MMU_CP,0,r1,MMU_Base,c0                                 
	mcr	MMU_CP,0,r1,MMU_FlushTLB,c0,0	/* Invalidate TLB */    
	mcr	MMU_CP,0,r1,MMU_FlushIDC,c0,0	/* Invalidate Caches */ 
	ldr	r1,=0xFFFFFFFF                                          
	mcr	MMU_CP,0,r1,MMU_DomainAccess,c0                         
	ldr	r2,=10f                                                 
	ldr	r1,=MMU_Control_Init|MMU_Control_M                      
	mcr	MMU_CP,0,r1,MMU_Control,c0                              
	mov	pc,r2    /* Change address spaces */                    
	nop                                                             
	nop                                                             
	nop                                                             
10:
        .endm
#else // EP7xxx, 720T processor
        .macro  MMU_INITIALIZE                                        
	ldr	r2,=MMU_Control_Init                                    
	mcr	MMU_CP,0,r2,MMU_Control,c0    /* MMU off */             
	mcr	MMU_CP,0,r1,MMU_Base,c0                                 
	mcr	MMU_CP,0,r1,MMU_TLB,c7,0      /* Invalidate TLB */      
	mcr	MMU_CP,0,r1,MMU_FlushIDC,c0,0 /* Invalidate Caches */   
	ldr	r1,=0xFFFFFFFF                                          
	mcr	MMU_CP,0,r1,MMU_DomainAccess,c0                         
	ldr	r2,=10f      
#ifdef CYG_HAL_STARTUP_ROMRAM
	ldr     r3,=__exception_handlers                                
	sub     r2,r2,r3                                                
	ldr     r3,=ROM0_LA_START                                       
 	add     r2,r2,r3  
#endif                                                      
	ldr	r1,=MMU_Control_Init|MMU_Control_M                      
	mcr	MMU_CP,0,r1,MMU_Control,c0                              
	mov	pc,r2    /* Change address spaces */                    
	nop                                                             
 	nop                                                             
	nop                                                             
10:
        .endm
#endif // EP7xxx,720T processor

#ifdef CYG_HAL_STARTUP_ROMRAM                                                  
        .macro  RELOCATE_TEXT_SEGMENT
        ldr     r2,=__exception_handlers
        ldr     r3,=ROM0_LA_START       
        cmp     r2,r3                   
        beq     20f                     
        ldr     r4,=__rom_data_end      
15:                                     
        ldr     r0,[r3],#4              
        str     r0,[r2],#4              
        cmp     r2,r4                   
        bne     15b            
	ldr	r2,=20f      
	mov	pc,r2    /* Change address spaces */                    
	nop                                                             
 	nop                                                             
	nop                                                             
20:
        .endm        
#endif
        
#ifdef CYG_HAL_STARTUP_RAM
        .macro  RELOCATE_RAM_IMAGE
// Special code to handle case where program has been loaded into DRAM
// _somewhere_.  This code first relocates itself into DRAM where eCos
// mapping will expect it to be.  Note since we don't know the current
// MMU mapping, this is tricky.  This is handled by putting a small
// routine into SRAM (which is always mapped 1-1) that turns off the
// MMU whilst it stores one word into physical memory.  Once the whole
// program has been relocated thusly, the MMU is shut off again while
// the eCos memory mapping takes place                
        bl      5f
// Routine to store one item in physical memory, with the MMU off        
_phys_store:
	ldr	r5,=MMU_Control_Init
	mcr	MMU_CP,0,r5,MMU_Control,c0    /* MMU off */
	nop
	nop
	nop
	mcr	MMU_CP,0,r1,MMU_TLB,c7,0      /* Invalidate TLB */
	mcr	MMU_CP,0,r1,MMU_FlushIDC,c0,0 /* Invalidate Caches */
        str     r4,[r2],#4
	ldr	r5,=MMU_Control_Init|MMU_Control_M
	mcr	MMU_CP,0,r5,MMU_Control,c0
        mov     pc,lr
_phys_store_end:                
// Copy above routine to SRAM, whose address does not change with MMU        
5:      mov     r1,lr
        add     r2,r1,#_phys_store_end-_phys_store
        ldr     r3,=SRAM_LA_START
6:      ldr     r4,[r1],#4
        str     r4,[r3],#4
        cmp     r1,r2
        bne     6b
        ldr     r6,=SRAM_LA_START
// Relocate code in DRAM to where eCos mapping wants it
        bl      7f
7:      mov     r2,lr
        ldr     r1,=7b
        sub     r1,r2,r1
        ldr     r2,=__exception_handlers
        add     r1,r1,r2
//        ldr     r1,=0xF0020000
        ldr     r3,=DRAM_PA
        add     r2,r2,r3
//        ldr     r2,=DRAM_PA+0x20000
        ldr     r3,=_edata
        ldr     r4,=__exception_handlers
        sub     r3,r3,r4
        add     r3,r1,r3
//        ldr     r3,=0xF0040000
10:     ldr     r4,[r1],#4
        mov     lr,pc           // Call phys_store() function above
        mov     pc,r6
        cmp     r1,r3
        bne     10b
// Now, turn off the MMU an execute the rest of this code in PHYSICAL memory        
        ldr     r1,=15f
        ldr     r2,=DRAM_PA
        add     r1,r1,r2
	ldr	r5,=MMU_Control_Init
	mcr	MMU_CP,0,r5,MMU_Control,c0    /* MMU off */
        mov     pc,r1
	nop
	nop
	nop
15:	mcr	MMU_CP,0,r1,MMU_TLB,c7,0      /* Invalidate TLB */
	mcr	MMU_CP,0,r1,MMU_FlushIDC,c0,0 /* Invalidate Caches */
        .endm
#endif // CYG_HAL_STARTUP_RAM        
#ifdef CYGHWR_HAL_ARM_EDB7XXX_VARIANT_EP7312
        .macro  INIT_MEMORY_CONFIG
        mov     r0,#(CPSR_IRQ_DISABLE|CPSR_FIQ_DISABLE|CPSR_SUPERVISOR_MODE)
        msr     cpsr,r0
        ldr     r10,=UARTDR1
        ldr     r11,=SYSFLG1
        ldr     r12,=SYSFLG1_UTXFF1
	ldr	r1,=SDCONF			
	ldr	r2,=0x00000522			
	str	r2,[r1]				
	ldr	r1,=SDRFOR			
	ldr	r2,=0x00000240			
	str	r2,[r1]				
	ldr	r1,=MEMCFG1			
	ldr	r2,=0x1F101710			
	str	r2,[r1]				
	ldr	r1,=MEMCFG2			
	ldr	r2,=0x00001F13			
	str	r2,[r1]
        .endm
#else
#ifdef CYGHWR_HAL_ARM_EDB7XXX_VARIANT_EP7209
// No DRAM controller
        .macro  INIT_MEMORY_CONFIG
/* Initialize memory configuration */
	ldr	r1,=MEMCFG1
	ldr	r2,=0x8200A080
	str	r2,[r1]
	ldr	r1,=MEMCFG2
	ldr	r2,=0xFEFC0000
	str	r2,[r1]
        .endm
#else // CYGHWR_HAL_ARM_EDB7XXX_VARIANT = EP7211, EP7212
        .macro  INIT_MEMORY_CONFIG
/* Initialize memory configuration */
	ldr	r1,=MEMCFG1
	ldr	r2,=0x8200A080
	str	r2,[r1]
	ldr	r1,=MEMCFG2
	ldr	r2,=0xFEFC0000
	str	r2,[r1]
	ldr	r1,=DRFPR
	ldr	r2,=0x81	/* DRAM refresh = 64KHz */
	strb	r2,[r1]
        .endm
#endif
#endif

#if defined(CYGSEM_HAL_STATIC_MMU_TABLES)
#define PLATFORM_SETUP1                          \
        INIT_MEMORY_CONFIG                      ;\
        ldr     r1,=_MMU_table-0xE0000000       ;\
        MMU_INITIALIZE                          ;\
        RELOCATE_TEXT_SEGMENT

#define PLATFORM_EXTRAS <cyg/hal/hal_platform_extras.h>

#else
// MMU tables placed in DRAM

#if (CYGHWR_HAL_ARM_EDB7XXX_DRAM_SIZE == 2)
// Note: The DRAM on this board is very irregular in that every
// other 256K piece is missing.  E.g. only these [physical]
// addresses are valid:
//   0xC0000000..0xC003FFFF
//   0xC0080000..0xC00BFFFF
//   0xC0200000..0xC023FFFF    Note the additional GAP!
//      etc.
//   0xC0800000..0xC083FFFF    Note the additional GAP!
//   0xC0880000..0xC08CFFFF
//   0xC0A00000..0xC0A3FFFF
//      etc.
// The MMU mapping code takes this into consideration and creates
// a continuous logical map for the DRAM.
        .macro  MAP_DRAM
/* Map DRAM */
	ldr	r3,=DRAM_LA_START
	ldr	r4,=DRAM_LA_END
	ldr	r5,=DRAM_PA
/* 0x00000000..0x000FFFFF */
	mov	r6,r2		/* Set up page table descriptor */
	ldr	r7,=MMU_L1_TYPE_Page
	orr	r6,r6,r7
	str	r6,[r1],#4	/* Store PTE, update pointer */
10:	mov	r6,r5		/* Build page table entry */
	ldr	r7,=MMU_L2_TYPE_Small|MMU_AP_Any|MMU_Bufferable|MMU_Cacheable
	orr	r6,r6,r7
	ldr	r7,=MMU_PAGE_SIZE
	str	r6,[r2],#4	/* Next page */
	add	r3,r3,r7
	add	r5,r5,r7
	ldr	r8,=DRAM_LA_START+MMU_SECTION_SIZE
	cmp	r3,r8		/* Done with first 1M? */
	beq	20f
	ldr	r7,=0x40000	/* Special check for 256K boundary */
	and	r7,r7,r5
	cmp	r7,#0
	beq	10b
	add	r5,r5,r7	/* Skip 256K hole */
	ldr	r7,=0x100000
	and	r7,r5,r7
	beq	10b
	add	r5,r5,r7	/* Nothing at 0xC0100000 */
	ldr	r7,=0x400000	/* Also nothing at 0xC0400000 */
	and	r7,r5,r7
	beq	10b
	add	r5,r5,r7
	b	10b
20:
/* 0x00100000..0x001FFFFF */
	mov	r6,r2		/* Set up page table descriptor */
	ldr	r7,=MMU_L1_TYPE_Page
	orr	r6,r6,r7
	str	r6,[r1],#4	/* Store PTE, update pointer */
10:	mov	r6,r5		/* Build page table entry */
	ldr	r7,=MMU_L2_TYPE_Small|MMU_AP_Any|MMU_Bufferable|MMU_Cacheable
	orr	r6,r6,r7
	ldr	r7,=MMU_PAGE_SIZE
	str	r6,[r2],#4	/* Next page */
	add	r3,r3,r7
	cmp	r3,r4		/* Done with first DRAM? */
	beq	20f
	add	r5,r5,r7
	ldr	r7,=0x40000	/* Special check for 256K boundary */
	and	r7,r7,r5
	cmp	r7,#0
	beq	10b
	add	r5,r5,r7	/* Skip 256K hole */
	ldr	r7,=0x100000
	and	r7,r5,r7
	beq	10b
	add	r5,r5,r7	/* Nothing at 0xC0300000 */
	ldr	r7,=0x400000	/* Also nothing at 0xC0400000 */
	and	r7,r5,r7
	beq	10b
	add	r5,r5,r7
	b	10b
20:

#elif (CYGHWR_HAL_ARM_EDB7XXX_DRAM_SIZE == 16)
// The 16M EDB72xx boards are arranged as:
//   0xC0000000..0xC07FFFFF
//   0xC1000000..0xC17FFFFF
// The 16M EDB7312 board is arranged as:
//   0xC0000000..0xC0FFFFFF
        .macro  MAP_DRAM
/* Map DRAM */
	ldr	r3,=DRAM_LA_START
	ldr	r4,=DRAM_LA_END
	ldr	r5,=DRAM_PA
/* 0xXXX00000..0xXXXFFFFF */
10:	mov	r6,r2		/* Set up page table descriptor */
	ldr	r7,=MMU_L1_TYPE_Page
	orr	r6,r6,r7
	str	r6,[r1],#4	/* Store PTE, update pointer */
        ldr     r8,=MMU_SECTION_SIZE/MMU_PAGE_SIZE
#if !defined(__EDB7312)
// EDB7312 has contiguous SDRAM
	ldr	r9,=DRAM_PA_START+0x00800000   /* Skip at 8M boundary */
12:     cmp     r5,r9
        bne     15f
	ldr	r5,=DRAM_PA_START+0x01000000   /* Next chunk of DRAM */
#else
12:
#endif
15:	mov	r6,r5		/* Build page table entry */
	ldr	r7,=MMU_L2_TYPE_Small|MMU_AP_Any|MMU_Bufferable|MMU_Cacheable
	orr	r6,r6,r7
	ldr	r7,=MMU_PAGE_SIZE
	str	r6,[r2],#4	/* Next page */
	add	r3,r3,r7
	add	r5,r5,r7
        cmp     r3,r4           /* End of DRAM? */
        beq     20f
        sub     r8,r8,#1        /* End of 1M section? */
        cmp     r8,#0
        bne     12b             /* Next page */
        b       10b             /* Next section */
20:
        .endm
#else
#error Invalid DRAM size select
#endif

        .macro  MAP_L1_SEGMENT start end phys prot
	ldr	r3,=0x3FF	/* Page tables need 2K boundary */
	add	r2,r2,r3
	ldr	r3,=~0x3FF
	and	r2,r2,r3
	ldr	r3,=\start
	ldr	r4,=\end
	ldr	r5,=\phys
	ldr	r6,=\prot
	ldr	r7,=MMU_SECTION_SIZE
10:	orr	r0,r5,r6
	str	r0,[r1],#4
	add	r5,r5,r7
	add	r3,r3,r7
	cmp	r3,r4
	bne	10b
        .endm

        .macro  PLATFORM_SETUP1
        INIT_MEMORY_CONFIG
#ifdef CYG_HAL_STARTUP_RAM
        RELOCATE_RAM_IMAGE
#endif        
/* Initialize MMU to create new memory map */
	ldr	r1,=MMU_BASE
	ldr	r2,=PTE_BASE
	MAP_DRAM
/* Nothing until PCMCIA0 */
        MAP_L1_SEGMENT (DRAM_LA_END+0x000FFFFF)&0xFFF00000 EXPANSION2_LA_START 0 MMU_L1_TYPE_Fault
/* EXPANSION2, EXPANSION3, PCMCIA0, PCMCIA1 */
        MAP_L1_SEGMENT EXPANSION2_LA_START SRAM_LA_START EXPANSION2_PA MMU_L1_TYPE_Section|MMU_AP_Any
/* SRAM */
	ldr	r3,=SRAM_LA_START
	ldr	r4,=MMU_L1_TYPE_Page|MMU_DOMAIN(0)
	orr	r4,r4,r2
	str	r4,[r1],#4
	ldr	r7,=MMU_PAGE_SIZE
        ldr     r5,=SRAM_LA_END
05:	ldr	r4,=MMU_L2_TYPE_Small|MMU_AP_Any|MMU_Bufferable|MMU_Cacheable
	orr	r4,r3,r4
	str	r4,[r2],#4
        add     r3,r3,r7
        cmp     r3,r5
        bne     05b
	ldr	r4,=SRAM_LA_START+MMU_SECTION_SIZE
	ldr	r5,=MMU_L2_TYPE_Fault
10:	str	r5,[r2],#4
	add	r3,r3,r7
	cmp	r3,r4
	bne	10b
	ldr	r4,=IO_LA_START
	ldr	r5,=MMU_L1_TYPE_Fault
	ldr	r7,=MMU_SECTION_SIZE
20:	str	r5,[r1],#4
	add	r3,r3,r7
	cmp	r3,r4
	bne	20b
/* I/O */
	ldr	r3,=0x3FF	/* Page tables need 2K boundary */
	add	r2,r2,r3
	ldr	r3,=~0x3FF
	and	r2,r2,r3
	ldr	r4,=MMU_L1_TYPE_Page|MMU_DOMAIN(0)
	orr	r4,r4,r2
	str	r4,[r1],#4
	ldr	r3,=IO_LA_START
	ldr	r4,=IO_LA_END
	ldr	r7,=MMU_PAGE_SIZE
	ldr	r5,=IO_PA|MMU_L2_TYPE_Small|MMU_AP_All
10:	str	r5,[r2],#4
	add	r5,r5,r7
	add	r3,r3,r7
	cmp	r3,r4
	bne	10b
	ldr	r4,=IO_LA_START+MMU_SECTION_SIZE
	ldr	r5,=MMU_L2_TYPE_Fault
	ldr	r7,=MMU_PAGE_SIZE
10:	str	r5,[r2],#4
	add	r3,r3,r7
	cmp	r3,r4
	bne	10b
	ldr	r4,=LCD_LA_START
	ldr	r5,=MMU_L1_TYPE_Fault
	ldr	r7,=MMU_SECTION_SIZE
20:	str	r5,[r1],#4
	add	r3,r3,r7
	cmp	r3,r4
	bne	20b
/* LCD Buffer & Unmapped DRAM (holes and all) */
        MAP_L1_SEGMENT LCD_LA_START ROM0_LA_START LCD_PA MMU_L1_TYPE_Section|MMU_AP_Any|MMU_Bufferable|MMU_Cacheable
/* ROM0 */
        MAP_L1_SEGMENT ROM0_LA_START ROM0_LA_END ROM0_PA MMU_L1_TYPE_Section|MMU_AP_Any
/* ROM1 */
        MAP_L1_SEGMENT ROM1_LA_START ROM1_LA_END ROM1_PA MMU_L1_TYPE_Section|MMU_AP_Any
/* Now initialize the MMU to use this new page table */
	ldr	r1,=MMU_BASE
        MMU_INITIALIZE
#ifdef CYG_HAL_STARTUP_ROMRAM                                                  
        RELOCATE_TEXT_SEGMENT
#endif // CYG_HAL_STARTUP_ROM                                                          
        .endm
#endif // CYGSEM_HAL_STATIC_MMU_TABLES

#else  // CYGSEM_HAL_INSTALL_MMU_TABLES

#define PLATFORM_SETUP1
#endif
#endif //_CYGHWR_LAYOUT_ONLY

/*---------------------------------------------------------------------------*/
/* end of hal_platform_setup.h                                               */
#endif /* CYGONCE_HAL_PLATFORM_SETUP_H */

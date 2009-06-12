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
// Date:        1999-04-13
// Purpose:     ARM/AEB-1 platform specific support routines
// Description: 
// Usage:       #include <cyg/hal/hal_platform_setup.h>
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

// From <cyg/hal/hal_cache.h> Need to make that file assembly safe.
#define CYG_DEVICE_CCR          0xFFFFA400
#define CCR_I                   0x08      // Invalidate mode

#if CYGNUM_HAL_COMMON_INTERRUPTS_STACK_SIZE==4096
// Override default to a more sensible value
#undef  CYGNUM_HAL_COMMON_INTERRUPTS_STACK_SIZE
#define CYGNUM_HAL_COMMON_INTERRUPTS_STACK_SIZE 2048
#endif

#ifdef CYGHWR_HAL_ARM_AEB_REVISION_C
// AEB rev C has 256kB of memory. Cache is working (set cachable)
#define AEB_SRAM .long	0xFFFFA008,0x00008000,0x00048000,0x00007c04
#define AEB_BAD  .long	0xFFFFA00C,0x00048000,0x01000000,0x00000000
#define AEB_CACHE .long	0xFFFFA010,0x60000000,0x61000000,0x00007801
#else
// AEB rev B has 128kB of memory. Cache is broken (clear cachable)
#define AEB_SRAM .long	0xFFFFA008,0x00008000,0x00028000,0x00007804
#define AEB_BAD  .long	0xFFFFA00C,0x00028000,0x01000000,0x00000000
#define AEB_CACHE .long	0xFFFFA010,0x60000000,0x61000000,0x00007801
#endif

#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
#define PLATFORM_SETUP1                                                       \
        ldr     r1,=CYG_DEVICE_CCR                                           ;\
        mov     r2,#CCR_I                                                    ;\
        strb    r2,[r1,#0]      /* invalidate... */                          ;\
        mov     r2,#0                                                        ;\
        strb    r2,[r1,#0]      /* and disable the cache. */                 ;\
	ldr	r1,=segment_register_setups                                  ;\
10:	ldr	r2,[r1,#0]	/* segment address */                        ;\
	cmp	r2,#0                                                        ;\
	beq	20f                                                          ;\
	ldr	r3,[r1,#4]	/* segment start */                          ;\
	str	r3,[r2,#0x00]                                                ;\
	ldr	r3,[r1,#8]	/* segment end */                            ;\
	str	r3,[r2,#0x20]                                                ;\
	ldr	r3,[r1,#12]	/* segment flags */                          ;\
	str	r3,[r2,#0x40]                                                ;\
	add	r1,r1,#16	/* next segment  */                          ;\
	b	10b                                                          ;\
segment_register_setups:                                                     ;\
	AEB_SRAM  /* segment 2 */                                            ;\
	AEB_BAD   /* segment 3 */                                            ;\
	AEB_CACHE /* segment 1 */                                            ;\
	.long 0                                                              ;\
20:
#else
#define PLATFORM_SETUP1
#endif

/*---------------------------------------------------------------------------*/
/* end of hal_platform_setup.h                                               */
#endif /* CYGONCE_HAL_PLATFORM_SETUP_H */

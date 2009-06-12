/*=============================================================================
//
//      hal_xscale.h
//
//      XScale Core I/O module support.
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004 Red Hat, Inc.
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
// Author(s):    msalter
// Contributors: msalter
// Date:         2002-10-18
// Purpose:      
// Description:  XScale core I/O modules support.
// Usage:        #include <cyg/hal/hal_xscale.h>
//
//####DESCRIPTIONEND####
//
//===========================================================================*/
#ifndef CYGONCE_HAL_ARM_XSCALE_HAL_XSCALE_H
#define CYGONCE_HAL_ARM_XSCALE_HAL_XSCALE_H

#include <pkgconf/system.h>
#include <pkgconf/hal_arm_xscale_core.h>

#ifdef __ASSEMBLER__
	// Useful CPU macros

	// Delay a bit
	.macro DELAY_FOR cycles, reg0
	ldr	\reg0, =\cycles
	subs	\reg0, \reg0, #1
	subne	pc,  pc, #0xc
	.endm
	
	// wait for coprocessor write complete
	.macro CPWAIT reg
        mrc  p15,0,\reg,c2,c0,0
	mov  \reg,\reg
	sub  pc,pc,#4
	.endm

	// Enable the BTB
	.macro BTB_INIT reg
	mrc	p15, 0, \reg, c1, c0, 0
#ifdef CYGSEM_HAL_ARM_XSCALE_BTB
	orr	\reg, \reg, #MMU_Control_BTB
#else
	bic	\reg, \reg, #MMU_Control_BTB
#endif
	mcr	p15, 0, \reg, c1, c0, 0
	CPWAIT  \reg
	.endm
#else
static inline void CPWAIT(void) {
    cyg_uint32 tmp;
    asm volatile ("mrc  p15,0,%0,c2,c0,0\n"
		  "mov  %0,%0\n"
		  "sub  pc,pc,#4" : "=r" (tmp));
}
#endif

#define __CYGARC_GET_CTLREG \
              "   mrc p15,0,r0,c1,c0,0\n"

#define __CYGARC_CLR_MMU_BITS \
              "   bic r0,r0,#0x05\n"

#ifdef CYGHWR_HAL_ARM_BIGENDIAN
#define __CYGARC_CLR_MMU_BITS_X \
              "   bic r0,r0,#0x85\n"
#else
#define __CYGARC_CLR_MMU_BITS_X \
              "   bic r0,r0,#0x05\n"         \
              "   orr r0,r0,#0x80\n"
#endif

#define __CYGARC_SET_CTLREG(__paddr__) \
              "   b 99f\n"                   \
              "   .p2align 5\n"              \
              "99:\n"                        \
              "   mcr p15,0,r0,c1,c0,0\n"    \
              "   mrc p15,0,r0,c2,c0,0\n"    \
              "   mov r0,r0\n"	             \
              "   sub pc,pc,#4\n"            \
              "   mov pc," #__paddr__ "\n"


// Override the default MMU off code. This is intended
// to be included in an inline asm statement.
#define CYGARC_HAL_MMU_OFF(__paddr__)        \
    __CYGARC_GET_CTLREG                      \
    __CYGARC_CLR_MMU_BITS                    \
    __CYGARC_SET_CTLREG(__paddr__)

#define CYGARC_HAL_MMU_OFF_X(__paddr__)      \
    __CYGARC_GET_CTLREG                      \
    __CYGARC_CLR_MMU_BITS_X                  \
    __CYGARC_SET_CTLREG(__paddr__)

#ifdef __ASSEMBLER__

#define REG8(a,b)  (b)
#define REG16(a,b) (b)
#define REG32(a,b) (b)

#else /* __ASSEMBLER__ */

#define REG8(a,b)  ((volatile unsigned char *)((a)+(b)))
#define REG16(a,b) ((volatile unsigned short *)((a)+(b)))
#define REG32(a,b) ((volatile unsigned int *)((a)+(b)))

extern void hal_xscale_core_init(void);
#endif /* __ASSEMBLER__ */

//--------------------------------------------------------------
#endif // CYGONCE_HAL_ARM_XSCALE_HAL_XSCALE_H
// EOF hal_xscale.h

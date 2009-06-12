#ifndef CYGONCE_HAL_VARIANT_INC
#define CYGONCE_HAL_VARIANT_INC
##=============================================================================
##
##      variant.inc
##
##      SH2 variant assembler header file
##
##=============================================================================
#####ECOSGPLCOPYRIGHTBEGIN####
## -------------------------------------------
## This file is part of eCos, the Embedded Configurable Operating System.
## Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
##
## eCos is free software; you can redistribute it and/or modify it under
## the terms of the GNU General Public License as published by the Free
## Software Foundation; either version 2 or (at your option) any later version.
##
## eCos is distributed in the hope that it will be useful, but WITHOUT ANY
## WARRANTY; without even the implied warranty of MERCHANTABILITY or
## FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
## for more details.
##
## You should have received a copy of the GNU General Public License along
## with eCos; if not, write to the Free Software Foundation, Inc.,
## 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
##
## As a special exception, if other files instantiate templates or use macros
## or inline functions from this file, or you compile this file and link it
## with other works to produce a work based on this file, this file does not
## by itself cause the resulting work to be covered by the GNU General Public
## License. However the source code for this file must still be made available
## in accordance with section (3) of the GNU General Public License.
##
## This exception does not invalidate any other reasons why a work based on
## this file might be covered by the GNU General Public License.
##
## Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
## at http://sources.redhat.com/ecos/ecos-license/
## -------------------------------------------
#####ECOSGPLCOPYRIGHTEND####
##=============================================================================
#######DESCRIPTIONBEGIN####
##
## Author(s):   jskov
## Contributors:jskov
## Date:        2002-01-17
## Purpose:     SH2 variant definitions and init code
## Description: This file contains various definitions and macros that are
##              useful for writing assembly code for the SH2 CPU family.
## Usage:
##              #include <cyg/hal/variant.inc>
##              ...
##              
##
######DESCRIPTIONEND####
##
##=============================================================================

#include <pkgconf/hal.h>
#include <cyg/hal/sh_regs.h>
        
#===========================================================================
## SR initialization value
## zero all bits except:
## I0-3 = Mask out all interrupts but NMI.

#define CYG_SR (CYGARC_REG_SR_IMASK)

##-----------------------------------------------------------------------------
## Hardware init macros
#ifndef CYGPKG_HAL_SH_POST_RESET_INIT
	.macro  hal_post_reset_init
        # Initialize CPU
        mov.l   $nCYG_SR,r1             ! Put CPU in a well-known state
        ldc     r1,sr
        mov     #0,r0
        mov.l   $CYGARC_REG_CCR,r1      ! Disable cache
#if (CYGARC_SH_MOD_CAC == 1)
        mov.b   r0,@r1
#elif (CYGARC_SH_MOD_CAC == 2)
	mov.w   r0,@r1
#else
# error "No CAC clear code"
#endif
#ifdef CYGARC_SH_MOD_UBC
#if (CYGARC_SH_MOD_UBC == 2)
        mov.l   $CYGARC_REG_BBRA,r1     ! Disable UBC Channel A
        mov.w   r0,@r1
        mov.l   $CYGARC_REG_BBRB,r1     ! Disable UBC Channel B
        mov.w   r0,@r1
        mov.l   $CYGARC_REG_BBRC,r1     ! Disable UBC Channel C
        mov.w   r0,@r1
        mov.l   $CYGARC_REG_BBRD,r1     ! Disable UBC Channel D
        mov.w   r0,@r1
#endif
#endif
#ifdef CYGARC_SH_MOD_FRT
        mov.l   $CYGARC_REG_TIER,r1     ! Disable FRT interrupts
        mov.b   r0,@r1
#endif
#ifdef CYGARC_SH_MOD_CMT
        mov.l   $CYGARC_REG_CMSTR,r1    ! Disable CMT timers
        mov.w   r0,@r1
#endif
        mov.l   $CYGARC_REG_IPRA,r1     ! Disable interrupt request sources
        mov.w   r0,@r1
        mov.l   $CYGARC_REG_IPRB,r1
        mov.w   r0,@r1
#if (CYGARC_SH_MOD_INTC == 1)
        mov.l   $CYGARC_REG_IPRC,r1
        mov.w   r0,@r1
        mov.l   $CYGARC_REG_IPRD,r1
        mov.w   r0,@r1
        mov.l   $CYGARC_REG_IPRE,r1
        mov.w   r0,@r1
	mov.w   $nCYG_ICR_INIT,r0
        mov.l   $CYGARC_REG_ICR,r1      ! Set interrupt controller mode
        mov.w   r0,@r1
#elif (CYGARC_SH_MOD_INTC == 2)
        mov.l   $CYGARC_REG_IPRC,r1
        mov.w   r0,@r1
        mov.l   $CYGARC_REG_IPRD,r1
        mov.w   r0,@r1
        mov.l   $CYGARC_REG_IPRE,r1
        mov.w   r0,@r1
        mov.l   $CYGARC_REG_IPRF,r1
        mov.w   r0,@r1
        mov.l   $CYGARC_REG_IPRG,r1
        mov.w   r0,@r1
        mov.l   $CYGARC_REG_IPRH,r1
        mov.w   r0,@r1
        mov.l   $CYGARC_REG_ICR,r1      ! level triggered
        mov.w   r0,@r1
        mov.l   $CYGARC_REG_ISR,r1      ! Clear any flags
        mov.w   r0,@r1
#else
# error "No INTC clear code"
#endif
	mov.w	$nCYG_WTCSR_INIT,r0     ! Disable watchdog
	mov.l	$CYGARC_REG_WTCSR_W,r1
	mov.w	r0,@r1

        # Initialize VBR if necessary
#if !defined(CYG_HAL_STARTUP_RAM) || !defined(CYGSEM_HAL_USE_ROM_MONITOR)
        mov.l   $_HW_EXC_ENTRY_TABLE,r1             ! Set VBR
        ldc     r1,vbr
#endif
	bra	1f
	 nop
	
$nCYG_WTCSR_INIT:
	.word	CYGARC_REG_WTCSR_INIT ! clear all CSR bits

	.align	2
$nCYG_SR:
        .long   CYG_SR
$CYGARC_REG_CCR:
	.long   CYGARC_REG_CCR
$CYGARC_REG_BBRA:
	.long   CYGARC_REG_BBRA
$CYGARC_REG_BBRB:
	.long   CYGARC_REG_BBRB
$CYGARC_REG_BBRC:
	.long   CYGARC_REG_BBRC
$CYGARC_REG_BBRD:
	.long   CYGARC_REG_BBRD
#ifdef CYGARC_SH_MOD_FRT
$CYGARC_REG_TIER:
        .long   CYGARC_REG_TIER
#endif
#ifdef CYGARC_SH_MOD_CMT
$CYGARC_REG_CMSTR:
        .long   CYGARC_REG_CMSTR
#endif
$CYGARC_REG_WTCSR_W:
	.long   CYGARC_REG_WTCSR_W
#if !defined(CYG_HAL_STARTUP_RAM) || !defined(CYGSEM_HAL_USE_ROM_MONITOR)
	SYM_PTR_REF(_HW_EXC_ENTRY_TABLE)
#endif
$CYGARC_REG_IPRA:
        .long   CYGARC_REG_IPRA
$CYGARC_REG_IPRB:
        .long   CYGARC_REG_IPRB
#if (CYGARC_SH_MOD_INTC == 1)
$CYGARC_REG_IPRC:
        .long   CYGARC_REG_IPRC
$CYGARC_REG_IPRD:
        .long   CYGARC_REG_IPRD
$CYGARC_REG_IPRE:
        .long   CYGARC_REG_IPRE
$CYGARC_REG_ICR:
        .long   CYGARC_REG_ICR
$nCYG_ICR_INIT:
	.word	CYGARC_REG_ICR_INIT
#elif (CYGARC_SH_MOD_INTC == 2)
$CYGARC_REG_IPRC:
        .long   CYGARC_REG_IPRC
$CYGARC_REG_IPRD:
        .long   CYGARC_REG_IPRD
$CYGARC_REG_IPRE:
        .long   CYGARC_REG_IPRE
$CYGARC_REG_IPRF:
        .long   CYGARC_REG_IPRF
$CYGARC_REG_IPRG:
        .long   CYGARC_REG_IPRG
$CYGARC_REG_IPRH:
        .long   CYGARC_REG_IPRH
$CYGARC_REG_ICR:
        .long   CYGARC_REG_ICR
$CYGARC_REG_ISR:
        .long   CYGARC_REG_ISR
#endif

1:	
	.endm
#define CYGPKG_HAL_SH_POST_RESET_INIT
#endif

##-----------------------------------------------------------------------------
## Interrupt decode macros
	.macro	hal_intc_decode tmp,inum
	.endm
	
#ifdef CYGIMP_HAL_COMMON_INTERRUPTS_CHAIN
	.macro	hal_intc_translate inum,vnum
	mov     #0,\vnum		! Just vector zero is supported
	.endm
#else			
	.macro	hal_intc_translate inum,vnum
	mov	\inum,\vnum		! Vector == interrupt number
        shll2   \vnum                   ! get from vector number to ISR index
	.endm
#endif

#------------------------------------------------------------------------------
#endif // CYGONCE_HAL_VARIANT_INC
# end of variant.inc

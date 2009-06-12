#ifndef CYGONCE_HAL_VARIANT_INC
#define CYGONCE_HAL_VARIANT_INC
##=============================================================================
##
##      variant.inc
##
##      SH3 variant assembler header file
##
##=============================================================================
#####ECOSGPLCOPYRIGHTBEGIN####
## -------------------------------------------
## This file is part of eCos, the Embedded Configurable Operating System.
## Copyright (C) 2003 Bart Veer
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
## Date:        2000-10-30
## Purpose:     SH3 variant definitions and init code
## Description: This file contains various definitions and macros that are
##              useful for writing assembly code for the SH3 CPU family.
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
## MD = Processor operation mode field (privileged mode)
## I0-3 = Mask out all interrupts but NMI.
## 
## When saving or restoring the state of an exception or interrupt, the bit
## CYGARC_REG_SR_RB is also set, switching the register bank. When this bit
## is set, interrupts must be disabled.
## 
## Note: We could also use the BL bit to prevent interrupts, but that would
##       also prevent the use of breakpoints.

#define CYG_SR (CYGARC_REG_SR_MD|CYGARC_REG_SR_IMASK)
#define CYG_SR_BANK1 (CYGARC_REG_SR_MD|CYGARC_REG_SR_IMASK|CYGARC_REG_SR_RB)

##-----------------------------------------------------------------------------
## Hardware init macros
#ifndef CYGPKG_HAL_SH_POST_RESET_INIT
	.macro  hal_post_reset_init
        # Initialize CPU
        mov.l   $nCYG_SR,r1             ! Put CPU in a well-known state
        ldc     r1,sr
        mov     #0,r0
        mov     #CYGARC_REG_CCR & 0x0FF,r1      ! Disable cache
        mov.l   r0,@r1
        mov     #CYGARC_REG_MMUCR & 0x0FF,r1    ! Disable MMU
        mov.l   r0,@r1
        mov     #CYGARC_REG_BBRA & 0x0FF,r1     ! Disable UBC Channel A
        mov.w   r0,@r1
        mov     #CYGARC_REG_BBRB & 0x0FF,r1     ! Disable UBC Channel B
        mov.w   r0,@r1
        mov     #CYGARC_REG_BRCR & 0x0FF,r1     ! Reset UBC common register
        mov.w   r0,@r1
        mov.l   $CYGARC_REG_TSTR,r1     ! Disable timers
        mov.b   r0,@r1
        mov.l   $CYGARC_REG_IPRA,r1     ! Disable interrupt request sources
        mov.w   r0,@r1
        mov.l   $CYGARC_REG_IPRB,r1
        mov.w   r0,@r1
#if (CYGARC_SH_MOD_INTC >= 2)
        mov.l   $CYGARC_REG_IPRC,r1
        mov.w   r0,@r1
        mov.l   $CYGARC_REG_IPRD,r1
        mov.w   r0,@r1
        mov.l   $CYGARC_REG_IPRE,r1
        mov.w   r0,@r1
#if (CYGARC_SH_MOD_INTC >= 3)
        mov.l   $CYGARC_REG_IPRF,r1
        mov.w   r0,@r1
#endif
	mov.w   $nCYG_ICR1_INIT,r0
        mov.l   $CYGARC_REG_ICR1,r1     ! Set interrupt controller to IRQ mode
        mov.w   r0,@r1
#endif
	mov.w	$nCYG_WTCSR,r0          ! Clear watchdog
	mov	#CYGARC_REG_WTCSR & 0x0FF,r1
	mov.w	r0,@r1

        # Initialize VBR if necessary
#if     !defined(CYG_HAL_STARTUP_RAM) ||                \
        (       defined(CYG_HAL_STARTUP_RAM) &&         \
                !defined(CYGSEM_HAL_USE_ROM_MONITOR))
        mov.l   $_reset,r1             ! Set VBR
        ldc     r1,vbr
#endif
	bra	1f
	 nop
	
$nCYG_WTCSR:
	.word	0xa500		! clear all CSR bits

	.align	2
$nCYG_SR:
        .long   CYG_SR
$CYGARC_REG_TSTR:
        .long   CYGARC_REG_TSTR
$CYGARC_REG_IPRA:
        .long   CYGARC_REG_IPRA
$CYGARC_REG_IPRB:
        .long   CYGARC_REG_IPRB
#if (CYGARC_SH_MOD_INTC >= 2)
$CYGARC_REG_IPRC:
        .long   CYGARC_REG_IPRC
$CYGARC_REG_IPRD:
        .long   CYGARC_REG_IPRD
$CYGARC_REG_IPRE:
        .long   CYGARC_REG_IPRE
$CYGARC_REG_ICR1:
        .long   CYGARC_REG_ICR1
$nCYG_ICR1_INIT:
	.word	CYGARC_REG_ICR1_INIT
	.align	2
#endif
#if (CYGARC_SH_MOD_INTC >= 3)
$CYGARC_REG_IPRF:
        .long   CYGARC_REG_IPRF
#endif
	SYM_PTR_REF(_reset)

1:	
	.endm
#define CYGPKG_HAL_SH_POST_RESET_INIT
#endif

##-----------------------------------------------------------------------------
## Interrupt decode macros
	.macro	hal_intc_decode tmp,inum
        mov.l   1f,\tmp
        mov.l   @\tmp,\inum
        mov     #-5,\tmp                ! divide cause by 0x20
        shld    \tmp,\inum
	bra     2f
         add     #-14,\inum             ! adjust so NMI becomes 0
	.align  2
1:
#if (CYGARC_SH_MOD_INTC >= 2)
	.long   CYGARC_REG_INTEVT2
#else
	.long   CYGARC_REG_INTEVT
#endif	
2:
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

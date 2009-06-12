// #===========================================================================
// #
// #    context.S
// #
// #    FUJITSU context switch code
// #
// #===========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2004 Red Hat, Inc.
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
// #===========================================================================
// ######DESCRIPTIONBEGIN####
// #
// # Author(s):    nickg, gthomas
// # Contributors: nickg, gthomas
// # Date:         1998-09-15
// # Purpose:      FUJITSU context switch code
// # Description:  This file contains implementations of the thread context 
// #               switch routines. It also contains the longjmp() and setjmp()
// #               routines.
// #
// #####DESCRIPTIONEND####
// #
// #===========================================================================

#include <pkgconf/hal.h>

#include "frv.inc"

        .text
        
// ----------------------------------------------------------------------------
//  hal_thread_switch_context(new, old)
//  Switch thread contexts
//  new = address of context of next thread to execute
//  old = address of context save location of current thread
//  thread state is saved on the current stack        

        .globl  hal_thread_switch_context
hal_thread_switch_context:
        subi    sp,_TS_size,sp          // Space for saved frame
        sti     gr2,@(sp,_TS_GPR2)       // Save registers
        sti     gr3,@(sp,_TS_GPR3)
        sti     gr4,@(sp,_TS_GPR4)
        sti     gr5,@(sp,_TS_GPR5)
        sti     gr6,@(sp,_TS_GPR6)
        sti     gr7,@(sp,_TS_GPR7)
        sti     gr8,@(sp,_TS_GPR8)
        sti     gr9,@(sp,_TS_GPR9)
        sti     gr10,@(sp,_TS_GPR10)
        sti     gr11,@(sp,_TS_GPR11)
        sti     gr12,@(sp,_TS_GPR12)
        sti     gr13,@(sp,_TS_GPR13)
        sti     gr14,@(sp,_TS_GPR14)
        sti     gr15,@(sp,_TS_GPR15)
        sti     gr16,@(sp,_TS_GPR16)
        sti     gr17,@(sp,_TS_GPR17)
        sti     gr18,@(sp,_TS_GPR18)
        sti     gr19,@(sp,_TS_GPR19)
        sti     gr20,@(sp,_TS_GPR20)
        sti     gr21,@(sp,_TS_GPR21)
        sti     gr22,@(sp,_TS_GPR22)
        sti     gr23,@(sp,_TS_GPR23)
        sti     gr24,@(sp,_TS_GPR24)
        sti     gr25,@(sp,_TS_GPR25)
        sti     gr26,@(sp,_TS_GPR26)
        sti     gr27,@(sp,_TS_GPR27)
        sti     gr28,@(sp,_TS_GPR28)
        sti     gr29,@(sp,_TS_GPR29)
        sti     gr30,@(sp,_TS_GPR30)
        sti     gr31,@(sp,_TS_GPR31)
#if _NGPR != 32
#ifdef CYGINT_HAL_FRV_ARCH_FR400	
	movsg	HSR0,gr5
	srli	gr5,#10,gr5
	andicc	gr5,#1,gr0,icc0
	bne	icc0,0,1f
#endif
        sti     gr32,@(sp,_TS_GPR32)
        sti     gr33,@(sp,_TS_GPR33)
        sti     gr34,@(sp,_TS_GPR34)
        sti     gr35,@(sp,_TS_GPR35)
        sti     gr36,@(sp,_TS_GPR36)
        sti     gr37,@(sp,_TS_GPR37)
        sti     gr38,@(sp,_TS_GPR38)
        sti     gr39,@(sp,_TS_GPR39)
        sti     gr40,@(sp,_TS_GPR40)
        sti     gr41,@(sp,_TS_GPR41)
        sti     gr42,@(sp,_TS_GPR42)
        sti     gr43,@(sp,_TS_GPR43)
        sti     gr44,@(sp,_TS_GPR44)
        sti     gr45,@(sp,_TS_GPR45)
        sti     gr46,@(sp,_TS_GPR46)
        sti     gr47,@(sp,_TS_GPR47)
        sti     gr48,@(sp,_TS_GPR48)
        sti     gr49,@(sp,_TS_GPR49)
        sti     gr50,@(sp,_TS_GPR50)
        sti     gr51,@(sp,_TS_GPR51)
        sti     gr52,@(sp,_TS_GPR52)
        sti     gr53,@(sp,_TS_GPR53)
        sti     gr54,@(sp,_TS_GPR54)
        sti     gr55,@(sp,_TS_GPR55)
        sti     gr56,@(sp,_TS_GPR56)
        sti     gr57,@(sp,_TS_GPR57)
        sti     gr58,@(sp,_TS_GPR58)
        sti     gr59,@(sp,_TS_GPR59)
        sti     gr60,@(sp,_TS_GPR60)
        sti     gr61,@(sp,_TS_GPR61)
        sti     gr62,@(sp,_TS_GPR62)
        sti     gr63,@(sp,_TS_GPR63)
1:	
#endif        
        movsg   psr,gr4
        sti     gr4,@(sp,_TS_PSR)
        movsg   lr,gr4
        sti     gr4,@(sp,_TS_PC)
        movsg   ccr,gr4
        sti     gr4,@(sp,_TS_CCR)
        movsg   lcr,gr4
        sti     gr4,@(sp,_TS_LCR)
        movsg   cccr,gr4
        sti     gr4,@(sp,_TS_CCCR)
        addi    sp,_TS_size,gr4
        sti     gr4,@(sp,_TS_SP)
        sti     sp,@(gr9,0)             // Pointer to saved context

        # Now load the destination thread by dropping through
        # to hal_thread_load_context
        
// ----------------------------------------------------------------------------
//  hal_thread_load_context(new)
//  Load thread context
//  new = address of context of next thread to execute
//  Note that this function is also the second half of
//  hal_thread_switch_context and is simply dropped into from it.

        .globl  hal_thread_load_context
hal_thread_load_context:
        ldi     @(gr8,0),sp             // Saved context
        ldi     @(sp,_TS_PSR),gr8
        setlos  #~_PSR_ET,gr9           // Turn off exceptions
        and     gr8,gr9,gr8
        setlos  #_PSR_PS|_PSR_S,gr9     // Stay in supervisor mode
        or      gr8,gr9,gr8
        movgs   gr8,psr
        ldi     @(sp,_TS_PC),gr8
        movgs   gr8,pcsr
        ldi     @(sp,_TS_CCR),gr8
        movgs   gr8,ccr
        ldi     @(sp,_TS_LCR),gr8
        movgs   gr8,lcr
        ldi     @(sp,_TS_CCCR),gr8
        movgs   gr8,cccr
#if _NGPR != 32
#ifdef CYGINT_HAL_FRV_ARCH_FR400	
	movsg	HSR0,gr5
	srli	gr5,#10,gr5
	andicc	gr5,#1,gr0,icc0
	bne	icc0,0,1f
#endif	
        ldi     @(sp,_TS_GPR32),gr32
        ldi     @(sp,_TS_GPR33),gr33
        ldi     @(sp,_TS_GPR34),gr34
        ldi     @(sp,_TS_GPR35),gr35
        ldi     @(sp,_TS_GPR36),gr36
        ldi     @(sp,_TS_GPR37),gr37
        ldi     @(sp,_TS_GPR38),gr38
        ldi     @(sp,_TS_GPR39),gr39
        ldi     @(sp,_TS_GPR40),gr40
        ldi     @(sp,_TS_GPR41),gr41
        ldi     @(sp,_TS_GPR42),gr42
        ldi     @(sp,_TS_GPR43),gr43
        ldi     @(sp,_TS_GPR44),gr44
        ldi     @(sp,_TS_GPR45),gr45
        ldi     @(sp,_TS_GPR46),gr46
        ldi     @(sp,_TS_GPR47),gr47
        ldi     @(sp,_TS_GPR48),gr48
        ldi     @(sp,_TS_GPR49),gr49
        ldi     @(sp,_TS_GPR50),gr50
        ldi     @(sp,_TS_GPR51),gr51
        ldi     @(sp,_TS_GPR52),gr52
        ldi     @(sp,_TS_GPR53),gr53
        ldi     @(sp,_TS_GPR54),gr54
        ldi     @(sp,_TS_GPR55),gr55
        ldi     @(sp,_TS_GPR56),gr56
        ldi     @(sp,_TS_GPR57),gr57
        ldi     @(sp,_TS_GPR58),gr58
        ldi     @(sp,_TS_GPR59),gr59
        ldi     @(sp,_TS_GPR60),gr60
        ldi     @(sp,_TS_GPR61),gr61
        ldi     @(sp,_TS_GPR62),gr62
        ldi     @(sp,_TS_GPR63),gr63
1:	
#endif        
        ldi     @(sp,_TS_GPR2),gr2       // Restore registers
        ldi     @(sp,_TS_GPR3),gr3
        ldi     @(sp,_TS_GPR4),gr4
        ldi     @(sp,_TS_GPR5),gr5
        ldi     @(sp,_TS_GPR6),gr6
        ldi     @(sp,_TS_GPR7),gr7
        ldi     @(sp,_TS_GPR8),gr8
        ldi     @(sp,_TS_GPR9),gr9
        ldi     @(sp,_TS_GPR10),gr10
        ldi     @(sp,_TS_GPR11),gr11
        ldi     @(sp,_TS_GPR12),gr12
        ldi     @(sp,_TS_GPR13),gr13
        ldi     @(sp,_TS_GPR14),gr14
        ldi     @(sp,_TS_GPR15),gr15
        ldi     @(sp,_TS_GPR16),gr16
        ldi     @(sp,_TS_GPR17),gr17
        ldi     @(sp,_TS_GPR18),gr18
        ldi     @(sp,_TS_GPR19),gr19
        ldi     @(sp,_TS_GPR20),gr20
        ldi     @(sp,_TS_GPR21),gr21
        ldi     @(sp,_TS_GPR22),gr22
        ldi     @(sp,_TS_GPR23),gr23
        ldi     @(sp,_TS_GPR24),gr24
        ldi     @(sp,_TS_GPR25),gr25
        ldi     @(sp,_TS_GPR26),gr26
        ldi     @(sp,_TS_GPR27),gr27
        ldi     @(sp,_TS_GPR28),gr28
        ldi     @(sp,_TS_GPR29),gr29
        ldi     @(sp,_TS_GPR30),gr30
        ldi     @(sp,_TS_GPR31),gr31
        ldi     @(sp,_TS_SP),sp
        rett    #0

// ----------------------------------------------------------------------------
//  HAL longjmp, setjmp implementations - based on newlib
//    Register      jmpbuf offset
//    R16-R31       0x0-0x03c
//    R48-R63       0x40-0x7c
//    FR16-FR31     0x80-0xbc
//    FR48-FR63     0xc0-0xfc
//    LR            0x100
//    SP            0x104
//    FP            0x108
//
// R8 contains the pointer to jmpbuf

	.text
	.global	hal_setjmp
	.type	hal_setjmp,@function
hal_setjmp:
	stdi	gr16, @(gr8,0)
	stdi	gr18, @(gr8,8)
	stdi 	gr20, @(gr8,16)
	stdi	gr22, @(gr8,24)
	stdi	gr24, @(gr8,32)
	stdi	gr26, @(gr8,40)
	stdi	gr28, @(gr8,48)
	stdi	gr30, @(gr8,56)
#if _NGPR != 32
#ifdef CYGINT_HAL_FRV_ARCH_FR400	
	movsg	HSR0,gr5
	srli	gr5,#10,gr5
	andicc	gr5,#1,gr0,icc0
	bne	icc0,0,1f
#endif	
	stdi	gr48, @(gr8,64)
	stdi	gr50, @(gr8,72)
	stdi	gr52, @(gr8,80)
	stdi	gr54, @(gr8,88)
	stdi	gr56, @(gr8,96)
	stdi	gr58, @(gr8,104)
	stdi	gr60, @(gr8,112)
	stdi	gr62, @(gr8,120)
1:	
#endif

#if _NFPR != 0
	stdfi	fr16, @(gr8,128)
	stdfi	fr18, @(gr8,136)
	stdfi 	fr20, @(gr8,144)
	stdfi	fr22, @(gr8,152)
	stdfi	fr24, @(gr8,160)
	stdfi	fr26, @(gr8,168)
	stdfi	fr28, @(gr8,176)
	stdfi	fr30, @(gr8,184)
#if _NFPR != 32
#ifdef CYGINT_HAL_FRV_ARCH_FR400	
	movsg	HSR0,gr5
	srli	gr5,#11,gr5
	andicc	gr5,#1,gr0,icc0
	bne	icc0,0,1f
#endif	
	stdfi	fr48, @(gr8,192)
	stdfi	fr50, @(gr8,200)
	stdfi	fr52, @(gr8,208)
	stdfi	fr54, @(gr8,216)
	stdfi	fr56, @(gr8,224)
	stdfi	fr58, @(gr8,232)
	stdfi	fr60, @(gr8,240)
	stdfi	fr62, @(gr8,248)
1:	
#endif
#endif

        movsg	lr, gr4
	sti     gr4, @(gr8,256)
	sti     sp, @(gr8,260)
	sti     fp, @(gr8,264)
	
	mov     gr0,gr8
	ret
.Lend1:
	.size	hal_setjmp,.Lend1-hal_setjmp

	.global	hal_longjmp
	.type	hal_longjmp,@function
hal_longjmp:
#if _NFPR != 0
	lddfi	@(gr8,128), fr16
	lddfi	@(gr8,136), fr18
	lddfi 	@(gr8,144), fr20
	lddfi	@(gr8,152), fr22
	lddfi	@(gr8,160), fr24
	lddfi	@(gr8,168), fr26
	lddfi	@(gr8,176), fr28
	lddfi	@(gr8,184), fr30
#if _NFPR != 32
#ifdef CYGINT_HAL_FRV_ARCH_FR400	
	movsg	HSR0,gr5
	srli	gr5,#11,gr5
	andicc	gr5,#1,gr0,icc0
	bne	icc0,0,1f
#endif	
	lddfi	@(gr8,192), fr48
	lddfi	@(gr8,200), fr50
	lddfi	@(gr8,208), fr52
	lddfi	@(gr8,216), fr54
	lddfi	@(gr8,224), fr56
	lddfi	@(gr8,232), fr58
	lddfi	@(gr8,240), fr60
	lddfi	@(gr8,248), fr62
1:
#endif
#endif

#if _NGPR != 32
#ifdef CYGINT_HAL_FRV_ARCH_FR400	
	movsg	HSR0,gr5
	srli	gr5,#10,gr5
	andicc	gr5,#1,gr0,icc0
	bne	icc0,0,1f
#endif	
	lddi	@(gr8,64), gr48
	lddi	@(gr8,72), gr50
	lddi	@(gr8,80), gr52
	lddi	@(gr8,88), gr54
	lddi	@(gr8,96), gr56
	lddi	@(gr8,104), gr58
	lddi	@(gr8,112), gr60
	lddi	@(gr8,120), gr62
1:	
#endif
	lddi	@(gr8,0), gr16
	lddi	@(gr8,8), gr18
	lddi 	@(gr8,16), gr20
	lddi	@(gr8,24), gr22
	lddi	@(gr8,32), gr24
	lddi	@(gr8,40), gr26
	lddi	@(gr8,48), gr28
	lddi	@(gr8,56), gr30


        ldi     @(gr8,256), gr4
	movgs   gr4,lr

        ldi     @(gr8,260), sp
        ldi     @(gr8,264), fp
	
# Value to return is in r9.  If zero, return 1
	cmp	gr9, gr0, icc0
	setlos	#1, gr8
	ckne	icc0, cc4
	cmov	gr9, gr8, cc4, 1
	ret
.Lend2:
	.size	hal_longjmp,.Lend2-hal_longjmp2
        
// ----------------------------------------------------------------------------
//  end of context.S

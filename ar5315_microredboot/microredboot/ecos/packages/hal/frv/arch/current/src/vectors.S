// #========================================================================
// #
// #    vectors.S
// #
// #    Fujitsu exception vectors
// #
// #========================================================================
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
// #========================================================================
// ######DESCRIPTIONBEGIN####
// #
// # Author(s):     gthomas
// # Contributors:  gthomas
// # Date:          2001-09-16
// # Purpose:       Fujitsu exception vectors
// # Description:   This file defines the code placed into the exception
// #                vectors. It also contains the first level default VSRs
// #                that save and restore state for both exceptions and
// #                interrupts.
// #
// #####DESCRIPTIONEND####
// #
// #========================================================================

#include <pkgconf/hal.h>
#include CYGBLD_HAL_PLF_DEFS_H        
#include "frv.inc"
#include <cyg/hal/platform.inc>

        .macro  lda a r
        sethi   #gprelhi(\a),\r
        setlo   #gprello(\a),\r
        add     \r,gr16,\r
        .endm

        .macro  li v r
        sethi   #((\v)>>16),\r
        setlo   #((\v)&0xFFFF),\r
        .endm
        
        .macro  save_GDB_exception_regs base,orig
        // 'orig' points to an area where some registers were already saved
        // orig+0:      GPR4
        // orig+4:      GPR5
        // orig+8:      GPR6
        // orig+12:     GPR16
        // orig+16:     LR
        // orig+20:     CCR
        sti     gr4,@(\base,_TS_VECTOR)
        addi    \orig,24,gr5
        sti     gr5,@(\base,_TS_SP)
        ldi     @(\orig,0),gr5
        sti     gr5,@(\base,_TS_GPR4)
        ldi     @(\orig,4),gr5
        sti     gr5,@(\base,_TS_GPR5)
        ldi     @(\orig,8),gr5
        sti     gr5,@(\base,_TS_GPR6)
        ldi     @(\orig,12),gr5
        sti     gr5,@(\base,_TS_GPR16)
        ldi     @(\orig,16),gr5
        sti     gr5,@(\base,_TS_LR)
        ldi     @(\orig,20),gr5
        sti     gr5,@(\base,_TS_CCR)
        .endm
        
        .macro  save_exception_regs base                
        sti     gr4,@(\base,_TS_VECTOR)
        sti     gr5,@(\base,_TS_GPR5)
        addi    sp,_TS_size,gr5
        sti     gr5,@(\base,_TS_SP)
        sti     gr6,@(\base,_TS_GPR6)
        sti     gr16,@(\base,_TS_GPR16)
        movsg   lr,gr5
        sti     gr5,@(\base,_TS_LR)
        movsg   ccr,gr5
        sti     gr5,@(\base,_TS_CCR)       
        .endm

        // Save the machine state after an interrupt/exception
        // Note: it might be possible to use stdi/lddi instructions here,
        // but it would require that the stack pointer always be 64 bit
        // (doubleword) aligned.
        .macro  save_state base
        sti     gr0,@(\base,_TS_GPR0)
        sti     gr2,@(\base,_TS_GPR2)
        sti     gr3,@(\base,_TS_GPR3)
        sti     gr7,@(\base,_TS_GPR7)
        sti     gr8,@(\base,_TS_GPR8)
        sti     gr9,@(\base,_TS_GPR9)
        sti     gr10,@(\base,_TS_GPR10)
        sti     gr11,@(\base,_TS_GPR11)
        sti     gr12,@(\base,_TS_GPR12)
        sti     gr13,@(\base,_TS_GPR13)
        sti     gr14,@(\base,_TS_GPR14)
        sti     gr15,@(\base,_TS_GPR15)
        sti     gr17,@(\base,_TS_GPR17)
        sti     gr18,@(\base,_TS_GPR18)
        sti     gr19,@(\base,_TS_GPR19)
        sti     gr20,@(\base,_TS_GPR20)
        sti     gr21,@(\base,_TS_GPR21)
        sti     gr22,@(\base,_TS_GPR22)
        sti     gr23,@(\base,_TS_GPR23)
        sti     gr24,@(\base,_TS_GPR24)
        sti     gr25,@(\base,_TS_GPR25)
        sti     gr26,@(\base,_TS_GPR26)
        sti     gr27,@(\base,_TS_GPR27)
        sti     gr28,@(\base,_TS_GPR28)
        sti     gr29,@(\base,_TS_GPR29)
        sti     gr30,@(\base,_TS_GPR30)
        sti     gr31,@(\base,_TS_GPR31)        
#if _NGPR != 32
#ifdef CYGINT_HAL_FRV_ARCH_FR400
	movsg	HSR0,gr5
	srli	gr5,#10,gr5
	andicc	gr5,#1,gr0,icc0
	bne	icc0,0,1f
#endif
        sti     gr32,@(\base,_TS_GPR32)
        sti     gr33,@(\base,_TS_GPR33)
        sti     gr34,@(\base,_TS_GPR34)
        sti     gr35,@(\base,_TS_GPR35)
        sti     gr36,@(\base,_TS_GPR36)
        sti     gr37,@(\base,_TS_GPR37)
        sti     gr38,@(\base,_TS_GPR38)
        sti     gr39,@(\base,_TS_GPR39)
        sti     gr40,@(\base,_TS_GPR40)
        sti     gr41,@(\base,_TS_GPR41)
        sti     gr42,@(\base,_TS_GPR42)
        sti     gr43,@(\base,_TS_GPR43)
        sti     gr44,@(\base,_TS_GPR44)
        sti     gr45,@(\base,_TS_GPR45)
        sti     gr46,@(\base,_TS_GPR46)
        sti     gr47,@(\base,_TS_GPR47)
        sti     gr48,@(\base,_TS_GPR48)
        sti     gr49,@(\base,_TS_GPR49)
        sti     gr50,@(\base,_TS_GPR50)
        sti     gr51,@(\base,_TS_GPR51)
        sti     gr52,@(\base,_TS_GPR52)
        sti     gr53,@(\base,_TS_GPR53)
        sti     gr54,@(\base,_TS_GPR54)
        sti     gr55,@(\base,_TS_GPR55)
        sti     gr56,@(\base,_TS_GPR56)
        sti     gr57,@(\base,_TS_GPR57)
        sti     gr58,@(\base,_TS_GPR58)
        sti     gr59,@(\base,_TS_GPR59)
        sti     gr60,@(\base,_TS_GPR60)
        sti     gr61,@(\base,_TS_GPR61)
        sti     gr62,@(\base,_TS_GPR62)
        sti     gr63,@(\base,_TS_GPR63)
1:	
#endif
	movsg	psr,gr5
	sti	gr5,@(\base,_TS_PSR)
	movsg	lcr,gr5
	sti	gr5,@(\base,_TS_LCR)
	movsg	cccr,gr5
	sti	gr5,@(\base,_TS_CCCR)
	movsg	bpcsr,gr5
        cmpi    gr4,#CYGNUM_HAL_VECTOR_BREAKPOINT,icc0
        beq     icc0,0,10f
5:      movsg	pcsr,gr5
        cmpi    gr4,#CYGNUM_HAL_VECTOR_SYSCALL,icc0
        blt     icc0,0,10f
6:      subi    gr5,#4,gr5              // traps show PC+4
10:     sti	gr5,@(\base,_TS_PC)
        .endm

        // Restore the machine state after an interrupt/exception                        
        .macro  restore_state base,pcreg,retv
	ldi	@(\base,_TS_PC),gr5
        movgs   gr5,\pcreg
	ldi	@(\base,_TS_CCR),gr5
        movgs   gr5,ccr
	ldi	@(\base,_TS_LR),gr5
        movgs   gr5,lr
	ldi	@(\base,_TS_PSR),gr5
        movgs   gr5,psr
	ldi	@(\base,_TS_LCR),gr5
        movgs   gr5,lcr
	ldi	@(\base,_TS_CCCR),gr5
        movgs   gr5,cccr
#if _NGPR != 32
#ifdef CYGINT_HAL_FRV_ARCH_FR400
	movsg	HSR0,gr5
	srli	gr5,#10,gr5
	andicc	gr5,#1,gr0,icc0
	bne	icc0,0,1f
#endif
        ldi     @(\base,_TS_GPR32),gr32
        ldi     @(\base,_TS_GPR33),gr33
        ldi     @(\base,_TS_GPR34),gr34
        ldi     @(\base,_TS_GPR35),gr35
        ldi     @(\base,_TS_GPR36),gr36
        ldi     @(\base,_TS_GPR37),gr37
        ldi     @(\base,_TS_GPR38),gr38
        ldi     @(\base,_TS_GPR39),gr39
        ldi     @(\base,_TS_GPR40),gr40
        ldi     @(\base,_TS_GPR41),gr41
        ldi     @(\base,_TS_GPR42),gr42
        ldi     @(\base,_TS_GPR43),gr43
        ldi     @(\base,_TS_GPR44),gr44
        ldi     @(\base,_TS_GPR45),gr45
        ldi     @(\base,_TS_GPR46),gr46
        ldi     @(\base,_TS_GPR47),gr47
        ldi     @(\base,_TS_GPR48),gr48
        ldi     @(\base,_TS_GPR49),gr49
        ldi     @(\base,_TS_GPR50),gr50
        ldi     @(\base,_TS_GPR51),gr51
        ldi     @(\base,_TS_GPR52),gr52
        ldi     @(\base,_TS_GPR53),gr53
        ldi     @(\base,_TS_GPR54),gr54
        ldi     @(\base,_TS_GPR55),gr55
        ldi     @(\base,_TS_GPR56),gr56
        ldi     @(\base,_TS_GPR57),gr57
        ldi     @(\base,_TS_GPR58),gr58
        ldi     @(\base,_TS_GPR59),gr59
        ldi     @(\base,_TS_GPR60),gr60
        ldi     @(\base,_TS_GPR61),gr61
        ldi     @(\base,_TS_GPR62),gr62
        ldi     @(\base,_TS_GPR63),gr63
1:	
#endif        
        ldi     @(\base,_TS_GPR2),gr2
        ldi     @(\base,_TS_GPR3),gr3
        ldi     @(\base,_TS_GPR4),gr4
        ldi     @(\base,_TS_GPR5),gr5
        ldi     @(\base,_TS_GPR6),gr6
        ldi     @(\base,_TS_GPR7),gr7
        ldi     @(\base,_TS_GPR8),gr8
        ldi     @(\base,_TS_GPR9),gr9
        ldi     @(\base,_TS_GPR10),gr10
        ldi     @(\base,_TS_GPR11),gr11
        ldi     @(\base,_TS_GPR12),gr12
        ldi     @(\base,_TS_GPR13),gr13
        ldi     @(\base,_TS_GPR14),gr14
        ldi     @(\base,_TS_GPR15),gr15
        ldi     @(\base,_TS_GPR16),gr16
        ldi     @(\base,_TS_GPR17),gr17
        ldi     @(\base,_TS_GPR18),gr18
        ldi     @(\base,_TS_GPR19),gr19
        ldi     @(\base,_TS_GPR20),gr20
        ldi     @(\base,_TS_GPR21),gr21
        ldi     @(\base,_TS_GPR22),gr22
        ldi     @(\base,_TS_GPR23),gr23
        ldi     @(\base,_TS_GPR24),gr24
        ldi     @(\base,_TS_GPR25),gr25
        ldi     @(\base,_TS_GPR26),gr26
        ldi     @(\base,_TS_GPR27),gr27
        ldi     @(\base,_TS_GPR28),gr28
        ldi     @(\base,_TS_GPR29),gr29
        ldi     @(\base,_TS_GPR30),gr30
        ldi     @(\base,_TS_GPR31),gr31
        ldi     @(\base,_TS_SP),gr1   // This has to be last - Stack pointer
        rett    #\retv
        .endm

#ifndef CYGSEM_HAL_USE_ROM_MONITOR
	
        .macro  exception_VSR
#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
        subi    sp,24,sp
        sti     gr4,@(sp,0)
#else
        subi    sp,_TS_size,sp
        sti     gr4,@(sp,_TS_GPR4)
#endif                
        addi    gr0,#((.-8)-_vectors)/16,gr4
        bra     _exception
        .endm

#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
        .macro  break_VSR
        subi    sp,24,sp
        sti     gr4,@(sp,0)
        addi    gr0,#((.-8)-_vectors)/16,gr4
        bra     _break
        .endm
#else
#define break_VSR exception_VSR
#endif // STUBS

        .macro  interrupt_VSR
        subi    sp,_TS_size,sp
        sti     gr4,@(sp,_TS_GPR4)
        addi    gr0,#((.-8)-_vectors)/16,gr4
        bra     _interrupt
        .endm
        
#if defined(CYG_HAL_STARTUP_ROM) || defined(CYG_HAL_STARTUP_ROMRAM)
        .section ".rom_vectors","ax"
_vectors:               
        call    reset_vector
#else /* RAM */
	.section ".rodata","a"
	.balign	4096
_vectors:               
	nop
#endif	
        nop	// I hate fencepost stuff like this....                 
        nop	// (NEXT_INDEX_AFTER_THIS - 1) - (PREVIOUS_INDEX_FILLED)
        nop	// (LAST_INDEX_TO_FILL)        - (PREVIOUS_INDEX_FILLED)
        .rept   (CYGNUM_HAL_VECTOR_EXTERNAL_INTERRUPT_LEVEL_1-1)-0
        exception_VSR
        .endr
        .rept   (CYGNUM_HAL_VECTOR_EXTERNAL_INTERRUPT_LEVEL_15) - (CYGNUM_HAL_VECTOR_EXTERNAL_INTERRUPT_LEVEL_1-1)
        interrupt_VSR
        .endr
        .rept   (254) - CYGNUM_HAL_VECTOR_EXTERNAL_INTERRUPT_LEVEL_15
        exception_VSR
        .endr
	break_VSR // in index 255
	
//
// Handle a break
//
// just like _exception, but it calls break_handler instead.
//
#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
_break:
        // Save current register state
        sti     gr5,@(sp,4)

	// First, check BRR to see whether we're processing a break
	// instruction.  The stub uses this instruction to enter debug mode.
	movsg	brr,gr5
	andicc	gr5,#2,gr0,icc0
	beq	icc0,#0,1f

	// It is a break instruction.  Clear BRR to acknowledge it.
	movgs	gr0,brr

	// Set LR to the address of the instruction after the break.
	// Since software breaks are post-execution ones, BPCSR already
	// contains the right address.
	movsg	bpcsr,gr5
	movgs	gr5,lr

	// Restore the other registers and return as if the break were a
	// call.  Don't worry, the user of the break instruction knows
	// that LR will be clobbered!
	ldi	@(sp,0),gr4
	ldi	@(sp,4),gr5
	addi	sp,#24,sp
	ret

1:
	// We didn't come here from a break instruction so assume a
	// breakpoint or watchpoint has been triggered.  Since the
	// interrupt and exception handlers save their registers on the
	// application stack, there's a chance that these handlers could
	// trigger watchpoints accidentally.  We should just ignore
	// the watchpoint when that happens.

	// Use the previous value of SPR:ET to decide whether the break
	// was triggered by stub or user code.  The stub runs with traps
	// disabled, while any user code that disables traps will not be
	// debuggable.
	movsg	bpsr,gr5
	andicc	gr5,#1,gr0,icc0
	bne	icc0,#2,1f

	// Hmm, it looks like the GDB stub has triggered an old watchpoint.
	// Acknowledge it by clearing brr and return as if nothing had
	// happened.
	movgs	gr0,brr
	ldi	@(sp,0),gr4
	ldi	@(sp,4),gr5
	addi	sp,#24,sp
	rett	#1
1:
        sti     gr6,@(sp,8)
        sti     gr16,@(sp,12)
	movsg	lr,gr5
	sti	gr5,@(sp,16)
	movsg	ccr,gr5
	sti	gr5,@(sp,20)
        // Set the global offset register (gr16)
        call    .Lbrk
.Lbrk:  movsg   lr,gr16
        sethi   #gprelhi(.Lbrk),gr5
        setlo   #gprello(.Lbrk),gr5
        sub     gr16,gr5,gr16
        mov     sp,gr6                  // Original stack pointer
        lda     __GDB_stack,gr5         // already on GDB stack?
        cmp     sp,gr5,icc0
        bhi     icc0,0,10f              // no - need to switch
        lda     __GDB_stack_base,gr5
        cmp     sp,gr5,icc0
        bhi     icc0,0,11f
10:     lda     __GDB_stack,sp          // already on GDB stack?
11:     
        subi    sp,_TS_size,sp          // Space for scratch saves
        save_GDB_exception_regs sp,gr6
        save_state sp

        LED     0x0FF1
        add     sp,gr0,gr8
        call    break_handler
        restore_state sp,bpcsr,1
#endif // STUBS

//
// Handle an exception
//
_exception:
	// Save current register state
#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
        sti     gr5,@(sp,4)
        sti     gr6,@(sp,8)
        sti     gr16,@(sp,12)
	movsg	lr,gr5
	sti	gr5,@(sp,16)
	movsg	ccr,gr5
	sti	gr5,@(sp,20)
        // Set the global offset register (gr16)
        call    .Lexp
.Lexp:  movsg   lr,gr16
        sethi   #gprelhi(.Lexp),gr5
        setlo   #gprello(.Lexp),gr5
        sub     gr16,gr5,gr16
        mov     sp,gr6                  // Original stack pointer
        lda     __GDB_stack,gr5         // already on GDB stack?
        cmp     sp,gr5,icc0
        bhi     icc0,0,10f              // no - need to switch
        lda     __GDB_stack_base,gr5
        cmp     sp,gr5,icc0
        bhi     icc0,0,11f
10:     lda     __GDB_stack,sp          // already on GDB stack?
11:     
        subi    sp,_TS_size,sp          // Space for scratch saves
        save_GDB_exception_regs sp,gr6
        save_state sp
#else
        save_exception_regs sp
        save_state sp
#endif
        LED     0x0FF2
	mov	sp,gr8
        call    exception_handler
        bra     _exception_return

//
// Handle an interrupt
//   Separated from exception handling to support eCos multi-level
//  (ISR/DSR) interrupt structure.
//
_interrupt:                                                      
        save_exception_regs sp
        save_state sp

        mov     gr4,gr30                // save vector #
        
        // Set the global offset register (gr16)
        call    .Lexp1
.Lexp1: movsg   lr,gr16
        sethi   #gprelhi(.Lexp1),gr5
        setlo   #gprello(.Lexp1),gr5
        sub     gr16,gr5,gr16
        LED     0x0700

        lda     hal_vsr_table,gr8
        slli    gr30,#2,gr4             // Vector in GR30
        ld      @(gr8,gr4),gr8          // Handler (VSR) defined?
        cmpi    gr8,0,icc0
        beq     icc0,0,10f              // No - use default
        LED     0x0702
        callil  @(gr8,0)                // Yes - call it
        bra     _exception_return

//
// Default interrupt processing
//                        
10:             
#if defined(CYGDBG_HAL_DEBUG_GDB_CTRLC_SUPPORT) \
    || defined(CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT)
        // If we are supporting Ctrl-C interrupts from GDB, we must squirrel
        // away a pointer to the save interrupt state here so that we can
        // plant a breakpoint at some later time.

       .extern  hal_saved_interrupt_state
        lda     hal_saved_interrupt_state,gr5
        sti     sp,@(gr5,0)
#endif
        lda     hal_interrupt_data,gr5
        lda     hal_interrupt_handlers,gr6
        mov     gr30,gr8                // Interrupt vector #
        slli    gr30,#2,gr4
        ld      @(gr5,gr4),gr9         // data pointer
        ld      @(gr6,gr4),gr6         // function
        callil  @(gr6,0)
        LED     0x0701
        // FIXME - no DSR processing

//
// Return from an exception/interrupt
//                        
_exception_return:               
        LED     0x0000
        restore_state sp,pcsr,0

#if defined(CYG_HAL_STARTUP_ROM) || defined(CYG_HAL_STARTUP_ROMRAM)
        .global reset_vector
reset_vector:
#if 1
	// I'm not sure why this is necessary, but power on reset
	// of CB70/VDK appears very unreliable without it.
	setlos	#1000,gr4
99:	subicc	gr4,#1,gr4,icc0
	bne	icc0,0,99b
#endif
        // Make sure the CPU is in the proper mode (system), interrupts disabled
        movsg   psr,gr4
        li      ~(_PSR_ET|_PSR_PS|_PSR_PIVL_MASK),gr5
        and     gr4,gr5,gr4
        li      (_PSR_S|_PSR_ET|_PSR_CM|(0x0F<<_PSR_PIVL_SHIFT)),gr5
        or      gr4,gr5,gr4
	movgs   gr4,psr
        
        movsg   hsr0,gr4
        // Make sure caches, MMUs, sleep states are disabled
        li      ~(_HSR0_ICE|_HSR0_DCE|_HSR0_IMMU|_HSR0_DMMU|7),gr5
        and     gr4,gr5,gr5
        movgs   gr5,hsr0

#ifdef __CB70_DEBUG
	membar
	sethi	#0xffc0,gr11
	setlo	#0x0000,gr11

	setlo	#0x5555,gr6
	sthi	gr6,@(gr11,0x100)  // leds
	membar
#endif
	
#ifdef CYGSEM_REDBOOT_FRV_LINUX_BOOT
	andicc	gr4,#7,gr0,icc0	// Were we in a sleep state?
	beq	icc0,0,3f	// No... normal startup

#ifdef __CB70_DEBUG
	setlo	#1,gr6
	sthi	gr6,@(gr11,0x100)  // leds
	membar
#endif
	
	not	gr14,gr5	// If gr14 = ~gr13 then gr14 is return address
	cmp	gr5,gr13,icc0	// ... else normal startup
	bne	icc0,0,3f

#ifdef __CB70_DEBUG
	setlo	#2,gr6
	sthi	gr6,@(gr11,0x100)  // leds
	membar
#endif
	
	// Assume FR400 since FR5xx doesn't run Linux (yet)
	li	_FRV400_SDRAM_STS, gr4
	sti	gr0,@(gr4,8)	// DRCN. Turn off self-refresh.
2:	
	ldi	@(gr4,0),gr5	// Wait for it to come back...
	andicc	gr5,#1,gr0,icc0
	bne	icc0,0,2b

#ifdef __CB70_DEBUG
	setlo	#4,gr6
	sthi	gr6,@(gr11,0x100)  // leds
	membar
#endif

	jmpil	@(gr14,#0)
	
3:
#endif		
        // Initialize hardware platform - this macro only contains
        // code which must be run before any "normal" accesses are
        // allowed, such as enabling DRAM controllers, etc.
        platform_init
        
#if defined(CYG_HAL_STARTUP_ROMRAM)
// Relocate code from ROM to static RAM
	call	10f		// Actual address loaded at
5:      .long   _vectors
        .long   20f
        .long   5b-_vectors
        .long   __rom_data_end
10:	movsg	lr,gr4
        ldi     @(gr4,0),gr6
        ldi     @(gr4,4),gr10
        ldi     @(gr4,8),gr7
        ldi     @(gr4,12),gr8
	sub	gr4,gr7,gr4	// GR4 - absolute base address
	subi	gr4,#4,gr4
	subi	gr6,#4,gr6
	setlos	#4,gr7
15:	ldu	@(gr4,gr7),gr5
	stu	gr5,@(gr6,gr7)
	cmp	gr6,gr8,icc0
	bne	icc0,0,15b        
        LED     0x5001
        jmpl    @(gr10,gr0)
20:     nop        
#endif // ROMRAM

        // Fall through to normal program startup
#endif // ROM || ROMRAM        
#endif  // !CYGSEM_HAL_USE_ROM_MONITOR

        .text
        
        .global _start
_start:         
        LED 0x5002
        // Set the global offset register (gr16) call
        call    .Lcall
    .Lcall:
        movsg   lr,gr4
        sethi   #gprelhi(.Lcall),gr5
        setlo   #gprello(.Lcall),gr5
        sub     gr4,gr5,gr16

        LED     0x0000
#ifndef CYGSEM_HAL_USE_ROM_MONITOR        
        // Set up trap base register                
        lda     _vectors,gr4
        movgs   gr4,tbr

        LED     0x0001
#endif
#if defined(CYG_HAL_STARTUP_ROM) || defined(CYG_HAL_STARTUP_ROMRAM)
        
        // Relocate [copy] data from ROM to RAM
        lda     __rom_data_start,gr4
        lda     __ram_data_start,gr5
        lda     __ram_data_end,gr6
        cmp     gr5,gr6,icc0
        beq     icc0,0,2f
        setlos  #4,gr7
        sub     gr4,gr7,gr4
        sub     gr5,gr7,gr5
1:      ldu     @(gr4,gr7),gr8
        stu     gr8,@(gr5,gr7)
        cmp     gr5,gr6,icc0
        bne     icc0,0,1b
2:                   
#endif
                
        // Set up stack, initial environment
        lda     __startup_stack,gr4
        mov     gr4,sp

        // Enable caches early, based on configuration
#ifdef CYGSEM_HAL_ENABLE_ICACHE_ON_STARTUP
        icei    @(gr4,gr0),1    // purges current contents
        movsg   hsr0,gr4
        li      _HSR0_ICE,gr5   // enable instruction cache
        or      gr4,gr5,gr4
        movgs   gr4,hsr0
#endif
#ifdef CYGSEM_HAL_ENABLE_DCACHE_ON_STARTUP
        dcef    @(gr4,gr0),1    // flush contents to memory
        dcei    @(gr4,gr0),1    // purges current contents
        movsg   hsr0,gr4
        li      _HSR0_DCE,gr5   // enable data cache
        or      gr4,gr5,gr4
        movgs   gr4,hsr0
#endif
        
        LED     0x0002
        
        // Clear BSS space
        lda     __bss_start,gr4
        lda     __bss_end,gr5
1:      sti     gr0,@(gr4,0)
        addi    gr4,#4,gr4
        cmp     gr4,gr5,icc0
        bne     icc0,0,1b

        // Initialize interrupt handlers, etc
        li      CYGNUM_HAL_ISR_COUNT,gr4
        lda     hal_interrupt_handlers,gr5
        subi    gr5,4,gr5
        lda     hal_default_isr,gr6
        lda     hal_vsr_table,gr8
        subi    gr8,4,gr8
        // Note: this should be controlled by a CDL option
        lda     _handle_interrupt,gr9
        setlos  #4,gr7
10:     stu     gr6,@(gr5,gr7)
        stu     gr9,@(gr8,gr7)
        subi    gr4,1,gr4
        cmp     gr4,gr0,icc0
        bne     icc0,0,10b

        LED     0x0003
        
        // Initialize hardware
        call    hal_hardware_init

        LED     0x0004
        
        // Run any C++ initializations
        call    cyg_hal_invoke_constructors

        LED     0x0005
        
#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
        call    initialize_stub
#endif        

        LED     0x0006

        // Start eCos application
        call    cyg_start

0:      LED     0x0999          // Should never get here
        bra     0b

//
// Interrupt processing
//                        
_handle_interrupt:              
        mov     sp,gr31         // Save pointer to state frame
        movsg   lr,gr29

        // Set the global offset register (gr16)
        call    10f
10:     movsg   lr,gr4
        sethi   #gprelhi(10b),gr5
        setlo   #gprello(10b),gr5
        sub     gr4,gr5,gr16
        
#ifdef CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK
        // Switch to interrupt stack
#endif
        // The entire CPU state is now stashed on the stack,
        // increment the scheduler lock and handle the interrupt

#ifdef CYGFUN_HAL_COMMON_KERNEL_SUPPORT                 
        .extern cyg_scheduler_sched_lock
        lda     cyg_scheduler_sched_lock,gr8
        ldi     @(gr8,0),gr9
        addi    gr9,1,gr9
        sti     gr9,@(gr8,0)
#endif

#if defined(CYGPKG_KERNEL_INSTRUMENT) && \
    defined(CYGDBG_KERNEL_INSTRUMENT_INTR)
        setlos  #RAISE_INTR,gr8         // arg0 = type = INTR,RAISE
        ldi     @(gr31,_TS_VECTOR),gr9  // arg1 = vector
        mov     gr0,gr10                // arg2 = 0
        call    cyg_instrument          // call instrument function
#endif
        
#if defined(CYGDBG_HAL_DEBUG_GDB_CTRLC_SUPPORT) \
    || defined(CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT)
        // If we are supporting Ctrl-C interrupts from GDB, we must squirrel
        // away a pointer to the save interrupt state here so that we can
        // plant a breakpoint at some later time.

       .extern  hal_saved_interrupt_state
        lda     hal_saved_interrupt_state,gr5
        sti     sp,@(gr5,0)
#endif
        lda     hal_interrupt_data,gr5
        lda     hal_interrupt_handlers,gr7
        mov     gr30,gr8                // Interrupt vector #
        slli    gr30,#2,gr4
        ld      @(gr5,gr4),gr9         // data pointer
        ld      @(gr7,gr4),gr6         // function
        callil  @(gr6,0)
        LED     0x0701

#ifdef CYGFUN_HAL_COMMON_KERNEL_SUPPORT
        // The return value from the handler (in gr8) will indicate whether a 
        // DSR is to be posted. Pass this together with a pointer to the
        // interrupt object we have just used to the interrupt tidy up routine.

        lda     hal_interrupt_objects,gr5
        slli    gr30,#2,gr4
        ld      @(gr5,gr4),gr9
        mov     gr31,gr10               // register frame

        call    interrupt_end           // post any bottom layer handler
                                        // threads and call scheduler
#endif
        movgs   gr29,lr
        ret
        
#ifdef CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK
// Execute pending DSRs the interrupt stack
// Note: this can only be called from code running on a thread stack
        .globl  hal_interrupt_stack_call_pending_DSRs
hal_interrupt_stack_call_pending_DSRs:         
        subi    sp,32,sp                // Save important registers
        movsg   lr,gr8
        sti     gr8,@(sp,0)
        // Turn on interrupts, switch to interrupt stack
        call    cyg_interrupt_call_pending_DSRs
        // Turn off interrupts, switch back to thread stack
        ldi     @(sp,0),gr8
        movgs   gr8,lr
        addi    sp,32,sp
        ret
#endif // CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK

//
// Restore interrupt state 
//   GR8 has state
//                                
        .global hal_restore_interrupts
hal_restore_interrupts:         
        movsg   psr,gr4
        setlos  _PSR_PIVL_MASK,gr5
        and     gr8,gr5,gr8             // Save level being restored
        not     gr5,gr5
        and     gr4,gr5,gr4             // Clear out current level
        or      gr8,gr4,gr4             // Insert new level
        movgs   gr4,psr
        ret
        
        .global hal_query_interrupts
hal_query_interrupts:         
        movsg   psr,gr8
        setlos  _PSR_PIVL_MASK,gr5
        and     gr8,gr5,gr8
        ret
        
//
// "Vectors" - fixed location data items
//    This section contains any data which might be shared between
// an eCos application and any other environment, e.g. the debug
// ROM.                        
//
        .section ".fixed_vectors"

// Space for the virtual vectors        
        .balign 16
// Vectors used to communicate between eCos and ROM environments
        .globl  hal_virtual_vector_table
hal_virtual_vector_table:
        .rept   CYGNUM_CALL_IF_TABLE_SIZE
        .long   0
        .endr

        .globl  hal_vsr_table
hal_vsr_table:
        .rept   CYGNUM_HAL_ISR_COUNT            // exceptions & interrupts
        .long   0
        .endr

        .section ".data"
#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
        .balign 16
__GDB_stack_base:
        .rept CYGNUM_HAL_COMMON_INTERRUPTS_STACK_SIZE // rather than 1k
        .byte 0
        .endr
__GDB_stack:
#endif
        .balign 16
__startup_stack_base:
#ifdef CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK
        .rept 512
#else
        .rept CYGNUM_HAL_COMMON_INTERRUPTS_STACK_SIZE
#endif
        .byte 0
        .endr
        .balign 16
        .global  __startup_stack
__startup_stack:

        .globl  hal_interrupt_handlers
hal_interrupt_handlers:
        .rept   CYGNUM_HAL_ISR_COUNT
        .long   0
        .endr

        .globl  hal_interrupt_data
hal_interrupt_data:
        .rept   CYGNUM_HAL_ISR_COUNT
        .long   0
        .endr

        .globl  hal_interrupt_objects
hal_interrupt_objects:
        .rept   CYGNUM_HAL_ISR_COUNT
        .long   0
        .endr

##==========================================================================
##
##      Vectors.S
##
##      OpenRISC exception vectors, interrupt-handling, reset and
##        platform-indepent initialization
##
##==========================================================================
#####ECOSGPLCOPYRIGHTBEGIN####
## -------------------------------------------
## This file is part of eCos, the Embedded Configurable Operating System.
## Copyright (C) 2002 Red Hat, Inc.
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
##==========================================================================
#######DESCRIPTIONBEGIN####
##
## Author(s):    sfurman
## Contributors: 
## Date:         2003-01-20
## Purpose:      OpenRISC interrupts, exception vectors and reset
## Description:  This file defines the code placed into the exception
##               vectors. It also contains the first level default VSRs
##               that save and restore state for both exceptions and
##               interrupts.
##
######DESCRIPTIONEND####
##
##==========================================================================

#include <pkgconf/hal.h>

#ifdef CYGPKG_KERNEL
#include <pkgconf/kernel.h>     // CYGPKG_KERNEL_INSTRUMENT
#endif

#include <cyg/hal/hal_intr.h>
#include <cyg/hal/hal_cache.h>

#===========================================================================
                
        
        .extern _hal_vsr_table

        .extern _cyg_hal_invoke_constructors
        .extern _cyg_instrument
        .extern _cyg_start
        .extern _hal_IRQ_init
        .extern _hal_platform_init
        .extern _initialize_stub

        .extern __bss_start
        .extern __bss_end
        .extern __sbss_start
        .extern __sbss_end

# Include variant macros after MSR definition.        
#include <cyg/hal/arch.inc>
#include <cyg/hal/openrisc.inc>


#===========================================================================
# Start by defining the exceptions vectors that must be placed in low
# memory, starting at location 0x100.
        
        .section ".vectors","ax"

#---------------------------------------------------------------------------
# Macros for generating an exception vector service routine

# Reset vector macro

        .macro  reset_vector name org
        .p2align 8
        .globl  __exception_\name
__exception_\name:
        load32i r3,_start
        l.jr    r3
        l.nop           # delay slot
        .endm
        
# Generic vector macro
                
        .macro  exception_vector name org
        .p2align 8
        .globl  __exception_\name
__exception_\name:
        l.addi  sp,sp,-SIZEOF_OR1KREGS  # space for registers

        # Store General Purpose Registers (GPRs).

        l.sw     3 * OR1K_GPRSIZE(sp), r3
        l.sw     4 * OR1K_GPRSIZE(sp), r4
        l.sw     5 * OR1K_GPRSIZE(sp), r5
        l.sw     6 * OR1K_GPRSIZE(sp), r6
        l.sw     7 * OR1K_GPRSIZE(sp), r7
        l.sw     8 * OR1K_GPRSIZE(sp), r8
        l.sw     9 * OR1K_GPRSIZE(sp), r9
        l.sw    11 * OR1K_GPRSIZE(sp), r11
        l.sw    13 * OR1K_GPRSIZE(sp), r13
        l.sw    15 * OR1K_GPRSIZE(sp), r15
        l.sw    17 * OR1K_GPRSIZE(sp), r17
        l.sw    19 * OR1K_GPRSIZE(sp), r19
        l.sw    21 * OR1K_GPRSIZE(sp), r21
        l.sw    23 * OR1K_GPRSIZE(sp), r23
        l.sw    25 * OR1K_GPRSIZE(sp), r25
        l.sw    27 * OR1K_GPRSIZE(sp), r27
        l.sw    29 * OR1K_GPRSIZE(sp), r29
        l.sw    31 * OR1K_GPRSIZE(sp), r31


#ifndef CYGDBG_HAL_COMMON_INTERRUPTS_SAVE_MINIMUM_CONTEXT

        # R0 is not typically stored because it is always zero-valued,
        # but we store it here for consistency when examining registers
        # in the debugger.
        l.sw     0 * OR1K_GPRSIZE(sp), r0

        # Callee-saved regs don't need to be preserved across a call into
        # an ISR, but we can do so to make debugging easier.
        l.sw     2 * OR1K_GPRSIZE(sp), r2
        l.sw    10 * OR1K_GPRSIZE(sp), r10
        l.sw    12 * OR1K_GPRSIZE(sp), r12
        l.sw    14 * OR1K_GPRSIZE(sp), r14
        l.sw    16 * OR1K_GPRSIZE(sp), r16
        l.sw    18 * OR1K_GPRSIZE(sp), r18
        l.sw    20 * OR1K_GPRSIZE(sp), r20
        l.sw    22 * OR1K_GPRSIZE(sp), r22
        l.sw    24 * OR1K_GPRSIZE(sp), r24
        l.sw    26 * OR1K_GPRSIZE(sp), r26
        l.sw    28 * OR1K_GPRSIZE(sp), r28
        l.sw    30 * OR1K_GPRSIZE(sp), r30

	# save MAC LO and HI regs
        l.mfspr r5,r0,SPR_MACLO
        l.sw    OR1KREG_MACLO(sp),r5
        l.mfspr r5,r0,SPR_MACHI
        l.sw    OR1KREG_MACHI(sp),r5
#endif
        
        # Save SP of interruptee in reg dump
        l.addi  r5,sp,SIZEOF_OR1KREGS
        l.sw     1 * OR1K_GPRSIZE(sp),r5

        # ...and the PC
        l.mfspr r5,r0,SPR_EPCR_BASE
        l.sw    OR1KREG_PC(sp),r5

        # ... and the Supervisor Register
        l.mfspr r5,r0,SPR_ESR_BASE
        l.sw    OR1KREG_SR(sp),r5

        # ... and the exception's effective address, if there is one.
        # FIXME - don't need to do this for some exceptions
        l.mfspr r5,r0,SPR_EEAR_BASE
        l.sw    OR1KREG_EEAR(sp),r5

        # Second arg to VSR is exception number
        # First vector is located at 0x100, second at 0x200, etc.
        # Shift right to get vector number for address lookup.
        l.ori   r4,r0,(\org>>8)
        l.sw    OR1KREG_VECTOR(sp),r4

        # Lookup address of VSR in table and jump to it
        #   Arg 0: Pointer to HAL_SavedRegisters struct
        #   Arg 1: Vector #
        load32i r5,_hal_vsr_table+(\org>>6)
        l.lwz   r5,0(r5)
        l.jr    r5                           # To the VSR, Batman

        # First arg to VSR is SP
        l.or    r3,r0,sp                     # Delay slot
        
        .endm
        
#---------------------------------------------------------------------------
# Define the exception vectors.

rom_vectors:
        # These are the architecture-defined vectors that
        # are always present.

        reset_vector            reset                   0x100
        exception_vector        bus_error               0x200
        exception_vector        data_page_fault         0x300
        exception_vector        instruction_page_fault  0x400
        exception_vector        tick_timer              0x500
        exception_vector        unaligned_access        0x600
        exception_vector        illegal_instruction     0x700
        exception_vector        external_interrupt      0x800
        exception_vector        dtlb_miss               0x900
        exception_vector        itlb_miss               0xa00
        exception_vector        range                   0xb00
        exception_vector        syscall                 0xc00
        exception_vector        reserved                0xd00
        exception_vector        trap                    0xe00

rom_vectors_end:   


#if     defined(CYG_HAL_STARTUP_ROM) ||                 \
        (       defined(CYG_HAL_STARTUP_RAM) &&         \
                !defined(CYGSEM_HAL_USE_ROM_MONITOR))

        .macro  hal_vsr_table_init

        # Next initialize the VSR table. This happens whether the
        # vectors were copied to RAM or not.

        # First fill with exception handlers
        load32i r3,_cyg_hal_default_exception_vsr
        load32i r4,_hal_vsr_table+4  # First entry in table is unused
        l.ori   r5,r0,CYGNUM_HAL_VSR_COUNT
1:      l.sw    0(r4),r3
        l.addi  r5,r5,-1
        l.sfgtsi r5,0
        l.bf    1b
        l.addi  r4,r4,4         # delay slot

        # Then fill in the interrupt handlers
        load32i r4,_hal_vsr_table
        load32i r3,_cyg_hal_default_interrupt_vsr
        l.sw    CYGNUM_HAL_VECTOR_INTERRUPT*4(r4),r3
        l.sw    CYGNUM_HAL_VECTOR_TICK_TIMER*4(r4),r3
        .endm

#elif defined(CYG_HAL_STARTUP_RAM) && defined(CYGSEM_HAL_USE_ROM_MONITOR)

        # Initialize the VSR table entries
        # We only take control of the interrupt vectors,
        # the rest are left to the ROM for now...

        .macro  hal_vsr_table_init
        load32i r4,_hal_vsr_table
        load32i r3,_cyg_hal_default_interrupt_vsr
        l.sw    CYGNUM_HAL_VECTOR_INTERRUPT*4(r4),r3
        l.sw    CYGNUM_HAL_VECTOR_TICK_TIMER*4(r4),r3
        .endm


#else

#error "Need to define hal_vsr_table_init"

#endif

# I-Cache initialization macro
        .macro  hal_icache_init
        /* Disable I-Cache */
        l.mfspr r13,r0,SPR_SR
        l.addi  r11,r0,-1
        l.xori  r11,r11,SPR_SR_ICE
        l.and   r11,r13,r11
        l.mtspr r0,r11,SPR_SR
      
        /* Invalidate I-Cache */
        l.addi  r13,r0,0
        l.addi  r11,r0,HAL_ICACHE_SIZE
1:
        l.mtspr r0,r13,SPR_ICBIR
        l.sfne  r13,r11
        l.bf    1b
        l.addi  r13,r13,HAL_ICACHE_LINE_SIZE
        
        /* Enable I-Cache */
        l.mfspr r13,r0,SPR_SR
        l.ori   r13,r13,SPR_SR_ICE
        l.mtspr r0,r13,SPR_SR

        /* Flush instructions out of instruction buffer */
        l.nop
        l.nop
        l.nop
        l.nop
        l.nop
        .endm
        
# D-Cache initialization macro
        .macro  hal_dcache_init

        /* Flush DC */
        l.addi  r10,r0,0
        l.addi  r11,r0,HAL_DCACHE_SIZE
1:
        l.mtspr r0,r10,SPR_DCBIR
        l.sfne  r10,r11
        l.bf    1b
        l.addi  r10,r10,HAL_DCACHE_LINE_SIZE

        /* Enable DC */
        l.mfspr r10,r0,SPR_SR
        l.ori   r10,r10,SPR_SR_DCE
        l.mtspr r0,r10,SPR_SR
        .endm
        
#===========================================================================
# Startup code:  We jump here from the reset vector to set up the world.
        
        .text   

FUNC_START(start)

        # Initialize Supervision Register:
        #   Supervisor mode on, all interrupts off, caches off
        #
        # (If we've entered here from a hardware reset, then the SR is already
        # set to this value, but we may have jumped here as part of a soft
        # system reset.)
        l.ori   r3,r0,SPR_SR_SM
        l.mtspr r0,r3,SPR_SR
        
        # Run platform-specific hardware initialization code.
        # This may include memory controller initialization.
        # Hence, it is not safe to access RAM until after this point.
        hal_hardware_init 

#ifdef CYGSEM_HAL_ENABLE_ICACHE_ON_STARTUP
        # Enable I-Cache
        hal_icache_init
#endif

#ifdef CYGSEM_HAL_ENABLE_DCACHE_ON_STARTUP
        # Enable D-Cache
        hal_dcache_init
#endif

	# Start the tick timer, in case timer polling routine hal_delay_us() is called.
	# Initially, no interrupts are generated by the tick timer.  Later on, that
	# may change when the kernel is initialized.
	l.movhi	r3, hi(0x40000000|CYGNUM_HAL_RTC_PERIOD)
	l.mtspr	r0,r3, SPR_TTMR
        
        .globl  _hal_hardware_init_done
_hal_hardware_init_done:                

        # set up stack
        load32i sp,__interrupt_stack

        # Make a dummy frame on the stack, so that stack backtraces are sane
        # for debugging.  On return from that function, the restore_state()
        # function is called to resume the interrupted thread.
        l.addi  sp,sp,-8
        l.sw    4(sp),r0        # Dummy saved FP
        l.sw    0(sp),r0        # Dummy saved LR
        
        # Set up exception handlers and VSR table, taking care not to
        # step on any ROM monitor VSRs.
        hal_vsr_table_init        

#if defined(CYG_HAL_STARTUP_ROM)
        # Copy exception/interrupt vectors from ROM to address 0x100
        load32i r4,0x100
        load32i r3,rom_vectors
        load32i r5,rom_vectors_end
1:      l.sfeq  r3,r5
        l.bf    2f
        l.lwz   r6,0(r3)
        l.sw    0(r4),r6
        l.addi  r3,r3,4
        l.j     1b
        l.addi  r4,r4,4         # delay slot
2:

        # Copy .data section into RAM
        load32i r3,__rom_data_start
        load32i r4,__ram_data_start
        load32i r5,__ram_data_end
1:      l.sfeq  r4,r5
        l.bf    2f
        l.lwz   r6,0(r3)
        l.sw    0(r4),r6
        l.addi  r3,r3,4
        l.j     1b
        l.addi  r4,r4,4         # delay slot
2:

#endif

        # clear BSS
        load32i r4,__bss_start
        load32i r5,__bss_end
1:      l.sfeq  r4,r5
        l.bf    2f
        l.nop
        l.sw    0(r4),r0
        l.j     1b
        l.addi  r4,r4,4
2:

        # Note:  no SBSS section to clear with OpenRISC target
        
        # Platform-specific initialization
        l.jal   _hal_platform_init
        l.nop   # delay slot
        
        # call c++ constructors
        l.jal   _cyg_hal_invoke_constructors
        l.nop   # delay slot

#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
        l.jal   _initialize_stub
        l.nop   # delay slot
#endif

#if defined(CYGDBG_HAL_DEBUG_GDB_CTRLC_SUPPORT) \
    || defined(CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT)
        .extern _hal_ctrlc_isr_init
        l.jal   _hal_ctrlc_isr_init
        l.nop   # delay slot
#endif
        
        l.jal   _cyg_start       # call cyg_start()
        l.nop   # delay slot
9:      
        l.j     9b              # if we return, loop

FUNC_END(start)
        
#---------------------------------------------------------------------------
# This code handles the common part of all exception handlers.
# On entry, the machine state is already saved on the stack.
#
# R3 = pointer to HAL_SavedRegisters struct containing saved machine state
# R4 = Vector number
#       
# It calls a C routine to do any work, which may result in
# thread switches and changes to the saved state. When we return
# here, the saved state is restored and execution is continued.
        
        .text

FUNC_START(cyg_hal_default_exception_vsr)

        .extern _cyg_hal_exception_handler

        # Call C code

        # When cyg_hal_exception_handler() returns, it will jump
	# directly to restore_state(), which will resume execution
	# at the location of the exception.
        l.movhi r9, hi(restore_state)
        l.j     _cyg_hal_exception_handler
        l.ori   r9,r9,lo(restore_state) #Delay slot

        # Control never reaches this point, 

FUNC_END(cyg_hal_default_exception_vsr)

#---------------------------------------------------------------------------
# This code handles all interrupts and dispatches to a C ISR function
# On entry, the machine state is already saved on the stack.
#
# R3 = pointer to HAL_SavedRegisters struct containing saved machine state
# R4 = Vector number
#       
# After we return here, the saved state is restored and execution is continued.
        
#ifdef CYGIMP_FORCE_INTERRUPT_HANDLING_CODE_IN_RAM
	.section .text.ram,"ax"
#else
	.section .text,"ax"
#endif
	
FUNC_START(cyg_hal_default_interrupt_vsr)

        # Stash away pointer to saved regs for later
        l.or    r31,r3,r3
        
        # Set scheduler lock to prevent thread rescheduling while the ISR runs
#ifdef CYGFUN_HAL_COMMON_KERNEL_SUPPORT                 
        .extern _cyg_scheduler_sched_lock
        load32i r5, _cyg_scheduler_sched_lock
        l.lwz   r6,0(r5)
        l.addi  r6,r6,1
        l.sw    0(r5),r6
#endif
        
#ifdef CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK
        # Interrupts execute on their own dedicated stack.
        # If we're on a thread stack, switch to the interrupt stack.
        # If we're called inside a nested interrupt, do nothing.
        l.or    r6,sp,sp                        # Stash SP for later
        load32i r7,__interrupt_stack            # stack top (highest addr + 1)
        load32i r8,__interrupt_stack_base       # stack base (lowest addr)
        l.sfltu sp,r8                           # if (sp < __interrupt_stack_base) 
        l.bf    1f                              #    switch to interrupt stack
        l.sfltu sp,r7                           # if (sp < __interrupt_stack_top)
        l.bf   2f                               #    already on interrupt stack
        l.nop                                   # delay slot
1:      l.or    sp,r7,r7                        # Switch to interrupt stack
2:      l.addi  sp,sp,-8                        # Make space to save old SP...
        l.sw    0(sp),r6                        # ...and save it on the stack           
#endif

        # Call C code

#if defined(CYGPKG_KERNEL_INSTRUMENT) && defined(CYGDBG_KERNEL_INSTRUMENT_INTR)
        # Log the interrupt if kernel tracing is enabled
        l.ori   r3,r0,0x0301                    # arg1 = type = INTR,RAISE
                                                # arg2 = vector number
        l.ori   r5,r0,r0                        # arg3 = 0
        l.jal   _cyg_instrument                  # call instrument function
        
#endif

#if defined(CYGDBG_HAL_DEBUG_GDB_CTRLC_SUPPORT) \
    || defined(CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT)
        # If we are supporting Ctrl-C interrupts from GDB, we must squirrel
        # away a pointer to the save interrupt state here so that we can
        # plant a breakpoint at some later time.
        
        .extern _hal_saved_interrupt_state
        load32i r8,_hal_saved_interrupt_state
        l.sw    0(r8),r31
        
#endif

        # In the event of multiple pending interrupts, determine which
        # one will be serviced first.  By software convention, the lowest
        # numbered external interrupt gets priority.
        #
        # The (internal) tick timer interrupt is serviced only if no
        # external interrupts are pending.

        # Read the PIC interrupt controller's status register
        l.mfspr r9,r0,SPR_PICSR

        # Any pending external interrupts ?
        l.sfnei r9,0
        l.bf    check_for_external_interrupts

        # Theoretically, the only way we could get here is if the tick timer
        # interrupt fired, but we check to be sure that's what happened.
        l.sfeqi r4,CYGNUM_HAL_VECTOR_TICK_TIMER
        l.bf    3f
        l.ori   r3,r0,CYGNUM_HAL_INTERRUPT_RTC  # delay slot
        
#ifndef CYGIMP_HAL_COMMON_INTERRUPTS_IGNORE_SPURIOUS
        l.jal   _hal_spurious_IRQ
        l.nop
#endif // CYGIMP_HAL_COMMON_INTERRUPTS_IGNORE_SPURIOUS
        l.j     ignore_spurious_interrupt
        
        # Identify the lowest numbered interrupt bit in the PIC's PSR,
        # numbering the MSB as 31 and the LSB as 0
check_for_external_interrupts:  
        l.ori   r3,r0,0
2:      l.andi  r11,r9,1                        # Test low bit
        l.sfnei r11,0
        l.bf    3f
        l.srli  r9,r9,1                         # Shift right 1 bit
        l.j     2b
        l.addi  r3,r3,1                         # Delay slot
3:              

        # At this point, r3 contains the ISR number, from 0-32
        # which will be used to index the table of ISRs
        l.slli  r15,r3,2
        load32i r9, _hal_interrupt_handlers     # get interrupt handler table
        l.add   r9,r9,r15
        l.lwz   r11,0(r9)                       # load ISR pointer
        load32i r9, _hal_interrupt_data         # get interrupt data table
        l.add   r9,r9,r15
        l.lwz   r4,0(r9)                        # load data arg to ISR

        # Call ISR
        #   arg0 = ISR #
        #   arg1 = data arg associated with interrupt
        l.jalr  r11
        l.nop

ignore_spurious_interrupt:
        
#ifdef CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK

        # If we are returning from the last nested interrupt, move back
        # to the thread stack. interrupt_end() must be called on the
        # thread stack since it potentially causes a context switch.
        # Since we have arranged for the top of stack location to
        # contain the sp we need to go back to here, just pop it off
        # and put it in SP.

        l.lwz   sp,0(sp)        
#endif  

#ifdef CYGFUN_HAL_COMMON_KERNEL_SUPPORT

        # We only need to call _interrupt_end() when there is a kernel
        # present to do any tidying up.
        
        # on return r11 bit 1 will indicate whether a DSR is
        # to be posted. Pass this together with a pointer to
        # the interrupt object we have just used to the
        # interrupt tidy up routine.
        l.or    r3,r11,r11

        # Get pointer to HAL_SavedRegisters struct, stashed earlier
        l.or    r5,r31,r31

        # Get opaque object associated w/ interrupt vector
        load32i r9, _hal_interrupt_objects          # get interrupt data table
        l.add   r9,r9,r15
        l.lwz   r4,0(r9)

        # Call interrupt_end() to execute any pending DSRs
        #   Arg 0 = return value from ISR
        #   Arg 1 = object associated with interrupt
        #   Arg 2 = HAL_SavedRegisters struct

        .extern _interrupt_end
        l.jal   _interrupt_end                   # call into C to finish off
        l.nop
#endif

        # Fall through to restore_state...

# Return from either an interrupt or an exception
#       
# On entry:     
#    SP = pointer to (HAL_SavedRegisters struct)
#
restore_state:

        # Restore General Purpose Registers (GPRs).
        # R0 is not restored because it is always zero-valued.
        # R1, R3, and R4 are used as temps, so they are restored a little later
        l.lwz   r5,   5 * OR1K_GPRSIZE(sp)
        l.lwz   r6,   6 * OR1K_GPRSIZE(sp)
        l.lwz   r7,   7 * OR1K_GPRSIZE(sp)
        l.lwz   r8,   8 * OR1K_GPRSIZE(sp)
        l.lwz   r9,   9 * OR1K_GPRSIZE(sp)
        l.lwz   r11, 11 * OR1K_GPRSIZE(sp)
        l.lwz   r13, 13 * OR1K_GPRSIZE(sp)
        l.lwz   r15, 15 * OR1K_GPRSIZE(sp)
        l.lwz   r17, 17 * OR1K_GPRSIZE(sp)
        l.lwz   r19, 19 * OR1K_GPRSIZE(sp)
        l.lwz   r21, 21 * OR1K_GPRSIZE(sp)
        l.lwz   r23, 23 * OR1K_GPRSIZE(sp)
        l.lwz   r25, 25 * OR1K_GPRSIZE(sp)
        l.lwz   r27, 27 * OR1K_GPRSIZE(sp)
        l.lwz   r29, 29 * OR1K_GPRSIZE(sp)
        l.lwz   r31, 31 * OR1K_GPRSIZE(sp)

#ifndef CYGDBG_HAL_COMMON_INTERRUPTS_SAVE_MINIMUM_CONTEXT
        # Callee-saved regs don't need to be preserved across a call into
        # an ISR, but we can do so to make debugging easier.

        l.lwz   r2,   2 * OR1K_GPRSIZE(sp)
        l.lwz   r10, 10 * OR1K_GPRSIZE(sp)
        l.lwz   r12, 12 * OR1K_GPRSIZE(sp)
        l.lwz   r14, 14 * OR1K_GPRSIZE(sp)
        l.lwz   r16, 16 * OR1K_GPRSIZE(sp)
        l.lwz   r18, 18 * OR1K_GPRSIZE(sp)
        l.lwz   r20, 20 * OR1K_GPRSIZE(sp)
        l.lwz   r22, 22 * OR1K_GPRSIZE(sp)
        l.lwz   r24, 24 * OR1K_GPRSIZE(sp)
        l.lwz   r26, 26 * OR1K_GPRSIZE(sp)
        l.lwz   r28, 28 * OR1K_GPRSIZE(sp)
        l.lwz   r30, 30 * OR1K_GPRSIZE(sp)

        # Restore MAC LO and HI regs
        l.lwz   r4, OR1KREG_MACLO(sp)
        l.mtspr r0,r4,SPR_MACLO
        l.lwz   r4, OR1KREG_MACHI(sp)
        l.mtspr r0,r4,SPR_MACHI
#endif

        # Must disable interrupts, since they could clobber ESR and EPC regs
        l.mfspr r3, r0, SPR_SR
        load32i r4,~(SPR_SR_TEE|SPR_SR_IEE)
        l.and   r3, r4, r3
        l.mtspr r0, r3, SPR_SR

        # At this point we've restored all the pre-interrupt GPRs except for the SP.
        # Restore pre-interrupt SR, SP, and PC
        l.lwz    r4,  OR1KREG_SR(sp)
        l.mtspr  r0, r4, SPR_ESR_BASE

        l.lwz    r4,  OR1KREG_PC(sp)
        l.mtspr  r0, r4, SPR_EPCR_BASE

        l.lwz   r4,   4 * OR1K_GPRSIZE(sp)
        l.lwz   r3,   3 * OR1K_GPRSIZE(sp)
        l.lwz    sp,  1 * OR1K_GPRSIZE(sp)

        # All done, restore CPU state and continue
        l.rfe
        l.nop           # Delay slot


##-----------------------------------------------------------------------------
## Execute pending DSRs on the interrupt stack with interrupts enabled.
## Note: this can only be called from code running on a thread stack

#ifdef CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK
        .extern _cyg_interrupt_call_pending_DSRs

	.text
FUNC_START(hal_interrupt_stack_call_pending_DSRs)
        # Switch to interrupt stack
        l.or    r3, sp, sp      # Stash entry SP
        load32i sp, __interrupt_stack
        l.addi  sp, sp, -16
        l.sw    0(sp), r3       # Save entry SP
        l.mfspr r4,r0,SPR_SR
        l.sw    4(sp), r4       # Save interrupt state
        l.ori   r4, r4, SPR_SR_IEE|SPR_SR_TEE
        l.sw    8(sp),lr
        
        l.jal   _cyg_interrupt_call_pending_DSRs
	# Enable interrupts before calling DSRs
	l.mtspr r0, r4, SPR_SR  # Delay slot

        l.lwz   r4, 4(sp)
        l.lwz   lr, 8(sp)
        l.lwz   sp, 0(sp)

        # Merge original interrupt state with (possibly altered) SR reg
        l.andi  r4, r4, SPR_SR_IEE|SPR_SR_TEE
        l.mfspr r5, r0, SPR_SR
        load32i r6, ~(SPR_SR_IEE|SPR_SR_TEE)
        l.and   r5, r5, r6
        l.or    r4, r4, r5
        
        l.jr    r9
        l.mtspr r0, r4, SPR_SR  # Delay slot
        
FUNC_END(hal_interrupt_stack_call_pending_DSRs)
#endif          

##-----------------------------------------------------------------------------
## Switch to a new stack.
## This is used in RedBoot to allow code to execute in a different
## stack context.

FUNC_START(hal_program_new_stack)
        # Arguments are:
        # r3 = function to call
        # r4 = stack pointer to use

        # Dummy prologue, so that debugger is fooled into thinking there
        # is a stack frame.  The debugger will use the offsets in the prologue
        # below to read the saved register values out of the *new* stack.
        l.addi  sp,sp,-8
        l.sw    0(sp),fp
        l.addi  fp,sp,8
        l.sw    4(sp),lr
        
        l.or    r5,sp,sp        # Remember original SP
        l.addi  r6,fp,-8        # Remember original FP
        l.or    sp,r4,r4        # Switch to new stack

        # "Real prologue" - Offsets here must match dummy prologue above
        l.addi  sp,sp,-16
        l.sw    0(sp),r6        # So debugger can know caller's FP
        l.sw    4(sp),lr        # So debugger can know caller's PC
        l.sw    8(sp),r5        # Save old SP on stack

        # Call function
        l.jalr  r3
        l.nop
        
        l.lwz   sp, 8(sp)       # Restore original SP
        l.lwz   lr, 4(sp)
        l.jr    lr              # Return to caller
        l.addi  sp,sp, 8        # Delay slot
        
FUNC_END(hal_program_new_stack)

#---------------------------------------------------------------------------
## Temporary interrupt stack
        
        .section ".bss"

        .balign 16
        .global _cyg_interrupt_stack_base
_cyg_interrupt_stack_base:
__interrupt_stack_base:
        .rept CYGNUM_HAL_COMMON_INTERRUPTS_STACK_SIZE
        .byte 0
        .endr
        .balign 16
        .global _cyg_interrupt_stack
_cyg_interrupt_stack:
__interrupt_stack:
        
        .long   0,0,0,0,0,0,0,0 

#--------------------------------------
        .data
        .extern _hal_default_isr

        .globl  _hal_interrupt_handlers
_hal_interrupt_handlers:
        .rept   CYGNUM_HAL_ISR_COUNT
        .long   _hal_default_isr
        .endr

        .globl  _hal_interrupt_data
_hal_interrupt_data:
        .rept   CYGNUM_HAL_ISR_COUNT
        .long   0
        .endr

        .globl  _hal_interrupt_objects
_hal_interrupt_objects:
        .rept   CYGNUM_HAL_ISR_COUNT
        .long   0
        .endr

#---------------------------------------------------------------------------
# end of vectors.S

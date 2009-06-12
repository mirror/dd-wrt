##==========================================================================
##
##      hal_var_bank.inc
##
##      SH support code for variants using register banks
##
##==========================================================================
#####ECOSGPLCOPYRIGHTBEGIN####
## -------------------------------------------
## This file is part of eCos, the Embedded Configurable Operating System.
## Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
## Copyright (C) 2003 Nick Garnett 
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
## Author(s):    jskov
## Contributors: jskov, nickg
## Date:         2002-01-11
## Purpose:      SH support code for variants using register banks
##
######DESCRIPTIONEND####
##
##==========================================================================

#---------------------------------------------------------------------------
# The models with banked registers jump directly to code without storing
# anything on the stack. Instead, the CPU switches to use banked registers
# leaving initial saving of state to the VSRs.

# reset            0xa0000000
# other exceptions VBR+0x100
# tlb miss         VBR+0x400
# interrupts       VBR+0x600


#===========================================================================
# Reset code must be PC relative so it can be executed out of the shadow
# area during startup. Not until after hal_hardware_init can the system
# be expected to provide the proper address space (at that time we
# jump to the VMA base of the code).

        .org    0x000
FUNC_START(_reset)
        mov.l   $_reset_platform,r0
        jmp     @r0
         nop

# Note: this is the unmapped, shadow address of the start of code
# Sadly, it is too far to just branch to.                        
        .align   2
$_reset_platform:
#ifdef CYG_HAL_STARTUP_RAM
        .long   CYG_LABEL_DEFN(_reset_platform)
#elif defined(CYG_HAL_STARTUP_ROMRAM)
        // Uncached "shadow" address but adjusted for VMA/LMA differences
        .long   __reset_platform+0x20000000-CYGMEM_REGION_ram+CYGMEM_REGION_rom
#else
        // Uncached "shadow" address
        .long   CYG_LABEL_DEFN(_reset_platform)+0x20000000
#endif
                
#---------------------------------------------------------------------------
# Exception entry
        
        .org    0x100
__exception:

#if defined(CYGPKG_KERNEL_INSTRUMENT) && defined(CYGDBG_KERNEL_INSTRUMENT_INTR)
        mov     #1,r7
#endif

        mov.l   $nCYGARC_REG_EXCEVT2,r1
        mov.l   @r1,r1
        shlr2   r1
        shlr    r1                      ! divide cause by 0x08
        mov.l   $hal_vsr_table2,r0
        mov.l   @(r0,r1),r1
        jmp     @r1
         nop
        
        .align   2
        SYM_PTR_REFn(hal_vsr_table,2)

#---------------------------------------------------------------------------
# TLB miss entry

        .org    0x400
__tlb_miss:
#if defined(CYGPKG_KERNEL_INSTRUMENT) && defined(CYGDBG_KERNEL_INSTRUMENT_INTR)
        mov      #2,r7
#endif

        mov.l   $nCYGARC_REG_EXCEVT2,r1
        mov.l   @r1,r1
        shlr2   r1
        shlr    r1                      ! divide cause by 0x08
        mov.l   $hal_vsr_table3,r0
        mov.l   @(r0,r1),r1
        jmp     @r1
         nop

        .align   2
        SYM_PTR_REFn(hal_vsr_table,3)
$nCYGARC_REG_EXCEVT2:
        .long   CYGARC_REG_EXCEVT

#---------------------------------------------------------------------------
# Interrupt entry

        .org    0x600
__interrupt:        

#ifdef CYGHWR_HAL_SH_IRQ_HANDLE_SPURIOUS_INTERRUPTS
        # A spurious interrupt with INTEVT=0 may be caused by
        # clearing of BL. Those interrupts need to be ignored.
        mov.l   1f,r0
        mov.l   @r0,r0
        cmp/eq  #0,r0
        bf      2f
        rte
         nop
	.align  2
1:	.long   CYGARC_REG_INTEVT
2:      
#endif        


#if defined(CYGPKG_KERNEL_INSTRUMENT) && defined(CYGDBG_KERNEL_INSTRUMENT_INTR)
        mov      #3,r7
#endif

        mov.l    $hal_vsr_table_int,r1
        mov.l    @r1,r1
        jmp      @r1
         nop

        .align   2
$hal_vsr_table_int:
        .long    CYG_LABEL_DEFN(hal_vsr_table)+CYGNUM_HAL_VECTOR_INTERRUPT*4


##============================================================================
## Macros for saving/restoring register state on an exception.

#---------------------------------------------------------------------------
## Save registers on exception:
## At entry:
## r15 is location to be stored to
## r0  is the available scratch register

## At exit:
## r7  is the vector #
## all other registers (except sp) are available

	.macro hal_cpu_save_regs
	
        # We come here with all register containing their
        # pre-exception values except:
        # r0_b-r7_b  = saved r0-r7
        # r7    = vector #
        # spc   = saved pc
        # ssr   = saved sr
      
        # Save away some registers
        mov     r15,r1                  ! entry sp
#ifdef CYGDBG_HAL_COMMON_INTERRUPTS_SAVE_MINIMUM_CONTEXT
        add     #-12,r15                ! Space for cause, gbr, and vbr
#else
        add     #-4,r15                 ! Space for cause

        stc     gbr,r0                  ! GBR
        mov.l   r0,@-r15
        stc     vbr,r0                  ! VBR
        mov.l   r0,@-r15
#endif

        stc     spc,r0
        mov.l   r0,@-r15                ! PC
        stc     ssr,r0
        mov.l   r0,@-r15                ! SR

        sts.l   pr,@-r15                ! PR

#ifdef CYGHWR_HAL_SH_FPU
        sts.l   fpscr,@-r15
        sts.l   fpul,@-r15
        mov.l   3f,r0                   ! set FPSCR to known value
        lds     r0,fpscr
#if CYGHWR_HAL_SH_FPU_REGS == 32
        frchg
        fmov.s  fr15,@-r15
        fmov.s  fr14,@-r15
        fmov.s  fr13,@-r15
        fmov.s  fr12,@-r15
        fmov.s  fr11,@-r15
        fmov.s  fr10,@-r15
        fmov.s  fr9,@-r15
        fmov.s  fr8,@-r15
        fmov.s  fr7,@-r15
        fmov.s  fr6,@-r15
        fmov.s  fr5,@-r15
        fmov.s  fr4,@-r15
        fmov.s  fr3,@-r15
        fmov.s  fr2,@-r15
        fmov.s  fr1,@-r15
        fmov.s  fr0,@-r15        
        frchg
#endif          
        fmov.s  fr15,@-r15
        fmov.s  fr14,@-r15
        fmov.s  fr13,@-r15
        fmov.s  fr12,@-r15
        fmov.s  fr11,@-r15
        fmov.s  fr10,@-r15
        fmov.s  fr9,@-r15
        fmov.s  fr8,@-r15
        fmov.s  fr7,@-r15
        fmov.s  fr6,@-r15
        fmov.s  fr5,@-r15
        fmov.s  fr4,@-r15
        fmov.s  fr3,@-r15
        fmov.s  fr2,@-r15
        fmov.s  fr1,@-r15
        fmov.s  fr0,@-r15
#endif

        sts.l   mach,@-r15              ! mach
        sts.l   macl,@-r15              ! macl

        mov.l   r1,@-r15                ! entry sp
        mov.l   r14,@-r15               ! r14-r0
        mov.l   r13,@-r15
        mov.l   r12,@-r15
        mov.l   r11,@-r15
        mov.l   r10,@-r15
        mov.l   r9,@-r15
        mov.l   r8,@-r15
        stc.l   r7_bank,@-r15
        stc.l   r6_bank,@-r15
        stc.l   r5_bank,@-r15
        stc.l   r4_bank,@-r15
        stc.l   r3_bank,@-r15
        stc.l   r2_bank,@-r15
        stc.l   r1_bank,@-r15
        stc.l   r0_bank,@-r15

        # After setting the SR it will be possible to use breakpoints.
        mov.l   1f,r1
        ldc     r1,sr
	bra	2f
	 nop
        .align  2        
1:	.long   CYG_SR
#ifdef CYGHWR_HAL_SH_FPU
3:      .long   CYG_FPSCR
#endif
        # ------------------------------------------------------------
        # Register bank has changed now. 
2:
	.endm

	# No additional magic needed. hal_cpu_save_regs does it all.
	.macro hal_exception_entry_extras
	.endm
	.macro hal_interrupt_entry_extras
	.endm

#---------------------------------------------------------------------------
## Restore registers after exception:
## At entry:
## r15 is location to be loaded from
## all other registers (except sp) are available

## At exit:
##  Returns to interrupted code

	.macro hal_cpu_restore_regs_return
        # Disable interrupts and switch register bank during the restore 
        # operation
        mov.l   1f,r1
        ldc     r1,sr
        # ------------------------------------------------------------
        # Register bank has changed now. 

        #
        # Single step debugging becomes impossible after this point!
        #
        mov     r15,r0
        ldc.l   @r0+,r0_bank
        ldc.l   @r0+,r1_bank
        ldc.l   @r0+,r2_bank
        ldc.l   @r0+,r3_bank
        ldc.l   @r0+,r4_bank
        ldc.l   @r0+,r5_bank
        ldc.l   @r0+,r6_bank
        ldc.l   @r0+,r7_bank
        mov.l   @r0+,r8
        mov.l   @r0+,r9
        mov.l   @r0+,r10
        mov.l   @r0+,r11
        mov.l   @r0+,r12
        mov.l   @r0+,r13
        mov.l   @r0+,r14
        mov.l   @r0+,r15                ! return SP

        lds.l   @r0+,macl               ! macl
        lds.l   @r0+,mach               ! mach

#ifdef CYGHWR_HAL_SH_FPU
        fmov.s  @r0+,fr0
        fmov.s  @r0+,fr1
        fmov.s  @r0+,fr2
        fmov.s  @r0+,fr3
        fmov.s  @r0+,fr4
        fmov.s  @r0+,fr5
        fmov.s  @r0+,fr6
        fmov.s  @r0+,fr7
        fmov.s  @r0+,fr8
        fmov.s  @r0+,fr9
        fmov.s  @r0+,fr10
        fmov.s  @r0+,fr11
        fmov.s  @r0+,fr12
        fmov.s  @r0+,fr13
        fmov.s  @r0+,fr14
        fmov.s  @r0+,fr15
#if CYGHWR_HAL_SH_FPU_REGS == 32
        frchg
        fmov.s  @r0+,fr0
        fmov.s  @r0+,fr1
        fmov.s  @r0+,fr2
        fmov.s  @r0+,fr3
        fmov.s  @r0+,fr4
        fmov.s  @r0+,fr5
        fmov.s  @r0+,fr6
        fmov.s  @r0+,fr7
        fmov.s  @r0+,fr8
        fmov.s  @r0+,fr9
        fmov.s  @r0+,fr10
        fmov.s  @r0+,fr11
        fmov.s  @r0+,fr12
        fmov.s  @r0+,fr13
        fmov.s  @r0+,fr14
        fmov.s  @r0+,fr15
        frchg
#endif                  
        lds.l   @r0+,fpul
        lds.l   @r0+,fpscr
#endif

        lds.l   @r0+,pr                 ! PR

        mov.l   @r0+,r2
        ldc     r2,ssr                  ! return SR
        mov.l   @r0+,r2
        ldc     r2,spc                  ! return PC

#ifndef CYGDBG_HAL_COMMON_INTERRUPTS_SAVE_MINIMUM_CONTEXT
        mov.l   @r0+,r2
        ldc     r2,vbr                  ! return VBR
        mov.l   @r0+,r2
        ldc     r2,gbr                  ! return GBR
#endif

        rte
         nop
        .align  2        
1:	.long   CYG_SR_BANK1
	.endm

#---------------------------------------------------------------------------
# Translate cause of exception to a vector number
	.macro hal_exception_translate
        mov.l   1f,r0
        mov.l   @r0,r4
        mov     #-5,r0                  ! divide cause by 0x20
        shld    r0,r4
        mov.l   3f,r0
	bra     2f
         mov.l   r4,@(r0,r15)            ! store decoded vector number back
	.align  2
1:	.long   CYGARC_REG_EXCEVT
3:      .long   CYGARC_SHREG_EVENT
2:	
	.endm

#---------------------------------------------------------------------------
# end of hal_var_bank.inc

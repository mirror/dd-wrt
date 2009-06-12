//vic - change use of current_set to just current - needs verification
/*
 * Feb 27, 2000 - Ken Hill
 *    Convert to Nios assembler. Note, most immediate values have to be word
 *    sized. The Nios processor word scales (*4) most immediate values.
 *    Convert most defines to .macro would not process them. this also allows for
 *    more offset size checking for required PFX instruction.
 *
 * winmacro.h: Window loading-unloading macros.
 *
 * Copyright (C) 1995 David S. Miller (davem@caip.rutgers.edu)
 */

#ifndef _NIOS_WINMACRO_H
#define _NIOS_WINMACRO_H

#include <asm/ptrace.h>
#include <asm/psr.h>

#define WRITE_PAUSE      nop; /* Have to do this after %psr chg */

	.macro FLUSH_ALL_KERNEL_WINDOWS

	rdctl	%g7			;vic; get STATUS
	mov	%g6, %g7		;vic;
	ANDIP	%g6, 0x01ff ;vic; clear IE, IPRI
	ORIP	%g6, 0x0600 ;vic; set IPRI = 3
	wrctl	%g6			;vic;
	 nop					;vic;
	 nop					;vic;

	.rept	31
	save	%sp, -REGWIN_SZ/4
	.endr
	.rept	31
	restore
	.endr

	wrctl	%g7			;vic; restore STATUS
	 nop					;vic;

	.endm

/* Store the register window onto the stack. */

	.macro STORE_WINDOW

	sts	[%sp, RW_L0/4], %l0;
	sts	[%sp, RW_L1/4], %l1;
	sts	[%sp, RW_L2/4], %l2;
	sts	[%sp, RW_L3/4], %l3;
	sts	[%sp, RW_L4/4], %l4;
	sts	[%sp, RW_L5/4], %l5;
	sts	[%sp, RW_L6/4], %l6;
	sts	[%sp, RW_L7/4], %l7;
	sts	[%sp, RW_I0/4], %i0;
	sts	[%sp, RW_I1/4], %i1;
	sts	[%sp, RW_I2/4], %i2;
	sts	[%sp, RW_I3/4], %i3;
	sts	[%sp, RW_I4/4], %i4;
	sts	[%sp, RW_I5/4], %i5;
	sts	[%sp, RW_I6/4], %i6;
	sts	[%sp, RW_I7/4], %i7;

	.endm


/* Load a register window from the area beginning at %sp. */

	.macro LOAD_WINDOW

	lds	%l0, [%sp, RW_L0/4];
	lds	%l1, [%sp, RW_L1/4];
	lds	%l2, [%sp, RW_L2/4];
	lds	%l3, [%sp, RW_L3/4];
	lds	%l4, [%sp, RW_L4/4];
	lds	%l5, [%sp, RW_L5/4];
	lds	%l6, [%sp, RW_L6/4];
	lds	%l7, [%sp, RW_L7/4];
	lds	%i0, [%sp, RW_I0/4];
	lds	%i1, [%sp, RW_I1/4];
	lds	%i2, [%sp, RW_I2/4];
	lds	%i3, [%sp, RW_I3/4];
	lds	%i4, [%sp, RW_I4/4];
	lds	%i5, [%sp, RW_I5/4];
	lds	%i6, [%sp, RW_I6/4];
	lds	%i7, [%sp, RW_I7/4]; 

	.endm

/* Loading and storing struct pt_reg trap frames. */

	.macro LOAD_PT_INS

        lds    %i0, [%sp, (REGWIN_SZ + PT_I0)/4]
        lds    %i1, [%sp, (REGWIN_SZ + PT_I1)/4]
        lds    %i2, [%sp, (REGWIN_SZ + PT_I2)/4]
        lds    %i3, [%sp, (REGWIN_SZ + PT_I3)/4]
        lds    %i4, [%sp, (REGWIN_SZ + PT_I4)/4]
        lds    %i5, [%sp, (REGWIN_SZ + PT_I5)/4]
        lds    %i6, [%sp, (REGWIN_SZ + PT_I6)/4]
        lds    %i7, [%sp, (REGWIN_SZ + PT_I7)/4]

	.endm

	.macro LOAD_PT_GLOBALS

        lds    %g0, [%sp, (REGWIN_SZ + PT_G0)/4]
        lds    %g1, [%sp, (REGWIN_SZ + PT_G1)/4]
        lds    %g2, [%sp, (REGWIN_SZ + PT_G2)/4]
        lds    %g3, [%sp, (REGWIN_SZ + PT_G3)/4]
        lds    %g4, [%sp, (REGWIN_SZ + PT_G4)/4]
        lds    %g5, [%sp, (REGWIN_SZ + PT_G5)/4]
        lds    %g6, [%sp, (REGWIN_SZ + PT_G6)/4]
        lds    %g7, [%sp, (REGWIN_SZ + PT_G7)/4]

	.endm

	.macro LOAD_PT_PRIV pt_psr, pt_pc

        lds    %\pt_psr, [%sp, (REGWIN_SZ + PT_PSR)/4]
        lds    %\pt_pc, [%sp, (REGWIN_SZ + PT_PC)/4] 

	.endm

	.macro LOAD_PT_ALL pt_psr, pt_pc

        LOAD_PT_INS
        LOAD_PT_GLOBALS
        LOAD_PT_PRIV \pt_psr, \pt_pc

	.endm

	.macro STORE_PT_INS

        sts    [%sp, (REGWIN_SZ + PT_I0)/4], %i0
        sts    [%sp, (REGWIN_SZ + PT_I1)/4], %i1
        sts    [%sp, (REGWIN_SZ + PT_I2)/4], %i2
        sts    [%sp, (REGWIN_SZ + PT_I3)/4], %i3
        sts    [%sp, (REGWIN_SZ + PT_I4)/4], %i4
        sts    [%sp, (REGWIN_SZ + PT_I5)/4], %i5
        sts    [%sp, (REGWIN_SZ + PT_I6)/4], %i6
        sts    [%sp, (REGWIN_SZ + PT_I7)/4], %i7

	.endm

	.macro STORE_PT_GLOBALS

        sts    [%sp, (REGWIN_SZ + PT_G0)/4], %g0
        sts    [%sp, (REGWIN_SZ + PT_G1)/4], %g1
        sts    [%sp, (REGWIN_SZ + PT_G2)/4], %g2
        sts    [%sp, (REGWIN_SZ + PT_G3)/4], %g3
        sts    [%sp, (REGWIN_SZ + PT_G4)/4], %g4
        sts    [%sp, (REGWIN_SZ + PT_G5)/4], %g5
        sts    [%sp, (REGWIN_SZ + PT_G6)/4], %g6
        sts    [%sp, (REGWIN_SZ + PT_G7)/4], %g7

	.endm

	.macro STORE_PT_PRIV pt_psr, pt_pc

        sts    [%sp, (REGWIN_SZ + PT_PSR)/4], %\pt_psr
        sts    [%sp, (REGWIN_SZ + PT_PC)/4], %\pt_pc

	.endm

	.macro STORE_PT_ALL reg_psr, reg_pc

        STORE_PT_PRIV \reg_psr, \reg_pc
        STORE_PT_GLOBALS
        STORE_PT_INS

	.endm

	.macro LOAD_CURRENT dest_reg

//vic	MOVIA	%\dest_reg,C_LABEL(current_set);
//vic	MOVIA	%\dest_reg,C_LABEL(current);
	MOVIA	%\dest_reg,C_LABEL(_current_task);
	ld      %\dest_reg, [%\dest_reg];

	.endm

	.macro STORE_CURRENT new_task, scratch_reg

//vic	MOVIA	%\scratch_reg,C_LABEL(current_set);
//vic	MOVIA	%\scratch_reg,C_LABEL(current);
	MOVIA	%\scratch_reg,C_LABEL(_current_task);
	st      [%\scratch_reg], %\new_task;

	.endm

;;	As long as kernel sp is valid, the task struct can be accessed
	.macro GET_CURRENT dest_reg

	mov	%\dest_reg, %sp
	lsri	%\dest_reg, 13
	lsli	%\dest_reg, 13

	.endm
/* macro for flushing register from current CWP to HI_LIMIT into the stack windows */
/* xwt: the caller is responsible for disabling interrupts for the following two macros */

	.macro	FLUSH_WINDOWS

;;	pfx	8
;;	wrctl	%g5				; disable interrupts

;	pfx	0
	rdctl	%g5				; read STATUS
	mov	%g3, %g5			; save STATUS
	mov	%g6, %o6			; save stack ptr
	mov	%g7, %o7			; save return address

	ANDIP	%g5, PSR_CWP			; isolate CWP

	pfx	2
	rdctl	%g4				; read WVALID
	lsri	%g4, 1				; get HI_LIMIT into CWP position
	ANDIP	%g4, PSR_CWP			; isolate it
8:
	STORE_WINDOW				; save %i's & %l's into stack register window
	restore					; CWP += 1
	cmp	%g5, %g4			; reached HI_LIMIT yet ?
	BNE	8b					; No
	 addi	%g5, 0x10			; step our value for CWP

;	pfx	0
	wrctl	%g3				; restore STATUS
	 nop
	 nop
	mov	%sp, %g6			; restore stack ptr
	mov	%o7, %g7			; restore return address

;;	pfx	9
;;	wrctl	%g5				; enable interrupts

	.endm

/* macro for filling register from current CWP to HI_LIMIT from the stack windows */
	.macro	FILL_WINDOWS

;;	pfx	8
;;	wrctl	%g5				; disable interrupts

;;	pfx	0
	rdctl	%g5				; read STATUS
	mov	%g3, %g5			; save STATUS
	mov	%g6, %sp			; save stack ptr
	mov	%g7, %o7			; save return address

	ANDIP	%g5, PSR_CWP			; isolate CWP

	pfx	2
	rdctl	%g4				; read WVALID
	lsri	%g4, 1				; get HI_LIMIT into CWP position
	ANDIP	%g4, PSR_CWP			; isolate it
9:
	LOAD_WINDOW				; load %i's & %l's from stack into register window
	restore					; CWP += 1
	cmp	%g5, %g4			; reached HI_LIMIT yet ?
	BNE	9b				; No
	 addi	%g5, 0x10			; step our value for CWP

;;	pfx	0
	wrctl	%g3				; restore STATUS
	 nop
	 nop
	mov	%sp, %g6			; restore stack ptr
	mov	%o7, %g7			; restore return address

;;	pfx	9
;;	wrctl	%g5				; enable interrupts

	.endm


#endif /* !(_NIOS_WINMACRO_H) */








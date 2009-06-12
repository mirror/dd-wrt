##=============================================================================
##
##	platform.S
##
##	MIPS IDT platform code
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
## Author(s): 	 tmichals
## Contributors: nickg
## Date:	 2002-10-02
## Purpose:	
## Description:	 MIPS IDT platform code
##              
######DESCRIPTIONEND####
##
##=============================================================================

#include <pkgconf/system.h>	
#include <pkgconf/hal.h>

#ifdef CYGPKG_KERNEL
# include <pkgconf/kernel.h>	
#endif
			
#include <cyg/hal/arch.inc>	
#include <cyg/hal/idt79rc233x.h>


FUNC_START(hal_memc_setup)
#define PIO_BASE_ADDR     0xb8000600
#define PIO_DC_SET        0x00500050
#define PIO_DC_MASK       0xff0fff0f

		li      t0,PIO_BASE_ADDR
		lw      t1,0x4(t0)
		nop
		li      t2,PIO_DC_MASK          #mask the UART bits
		and     t1,t1,t2
		nop
		li      t2,PIO_DC_SET           #rx0=rx1=input, tx0=tx1=output
		or      t1,t1,t2
		nop
		sw      t1,0x4(t0)


		li      t0,PORT_WIDTH_CNTL_REG /* Set PortWidth and Bus parameters */
		li      t1,PORT_WIDTH_CNTL_VAL
		sw      t1,0(t0)

		li      t0,BUS_TURN_AROUND_CNTRL_REG
		li      t1,BUS_TURN_AROUND_VAL /* Sets all turnarounds to 3 cycles.  0x18000000*/
		sw      t1,0(t0)
	
		li      t0,BUS_TURN_AROUND_REG
		li      t1,BUS_TURN_AROUND_VAL /* Sets all turnarounds to 3 cycles. 0xffffe204*/
		sw      t1,0(t0)
	

		li      t0,ADDRESS_LATCH_TIMING_REG
		li      t1,ADDRESS_LATCH_TIMING_VAL
		sw      t1,0(t0)



/* CHIP SELECT 0 for FLASH */
        li      t0,MEM_BASE_BASE  
        li      t1,MBA_REG0
        sw      t1,0(t0)          	/* 0x18000080 */
        li      t1,MBM_REG0
        sw      t1,0x4(t0)       	/* 0x18000084 */
        li      t0,MEM_CTL_BASE
        li      t1,MCR_CS0_BS
        sw      t1,0(t0)          	/* 0x18000200 */
	
/* CHIP SELECT 1 for FLASH */

        li      t0,MEM_BASE_BASE  
        li      t1,MBA_REG1
        sw      t1,8(t0)	  	/* 0x18000088 */
        li      t1,MBM_REG1		/* 0x1800008C */
        sw      t1,0xc(t0)
        li      t0,MEM_CTL_BASE	 
        li      t1,MCR_CS1_BS  		/* chip select 2 for other flash */
        sw      t1,0x4(t0)	 	/* 0x18000204 */     

/* CHIP SELECT 2*/
		li	t1,MCR_CS2_BS
		sw	t1,0x8(t0)

/* CHIP SELECT 3*/
		li	t1,MCR_CS3_BS
		sw	t1,0xc(t0)


/* CHIP SELECT 4*/
		li	t1,MCR_CS4_BS
		sw	t1,0x10(t0)


/* CHIP SELECT 5*/
		li	t1,MCR_CS5_BS
		sw	t1,0x14(t0)
	


		li      t0,RHEA_IREG_BASE
		li      t1,DISABLE_TIMER
		sw      t1,0x730(t0)

		li      t1,0
		sw      t1,0x310(t0)


/* SDRAM Initialization:- Start Here!  */
        li      t1,DRAM_BNK0_BASE
        sw      t1,0xc0(t0)
        li      t1,DRAM_BNK1_BASE
        sw      t1,0xc8(t0)
        li      t1,DRAM_BNK2_BASE
        sw      t1,0xd0(t0)
        li      t1,DRAM_BNK3_BASE
        sw      t1,0xd8(t0)

        li      t1,DRAM_BNK0_MASK
        sw      t1,0xc4(t0)
        li      t1,DRAM_BNK1_MASK
        sw      t1,0xcc(t0)
        li      t1,DRAM_BNK2_MASK
        sw      t1,0xd4(t0)
        li      t1,DRAM_BNK3_MASK
        sw      t1,0xdc(t0)

        li      t0,MEM_BASE_BASE
        li      t1,MBA_REG1
        sw      t1,0x8(t0)
        li      t1,MBM_REG1
        sw      t1,0xc(t0)

        li      t0,TIMER_BASE
        li      t1,DISABLE_TIMER
        sw      t1,0x60(t0)
		nop
        li      t1,0x0
        sw      t1,0x64(t0)
        li      t1,CPU_BERR_BS
        sw      t1,0x48(t0)
        li      t1,IP_BERR_BS
        sw      t1,0x58(t0)


        li      t0,RHEA_IREG_BASE
        li      t1,SDRAM_CR_BS
        sw      t1,0x300(t0)

        li      t2,2
        li      t3,0
1:
        li      t1,SDRAM_PC_VAL
        sw      t1,0x300(t0)
        nop
        li      t4,SDRAM_TEST_PATTERN
        li      t5,K1BASE | DRAM_BNK0_BASE
        sw      t4,0x1000(t5)
        addu    t3,1
        bne     t3,t2,1b
		nop

        li      t2,2
        li      t3,0
1:
        li      t1,SDRAM_RFRSH_CMD
        sw      t1,0x300(t0)
        sw      t4,0x0(t5)
        addu    t3,1
        bne     t3,t2,1b
		nop


        li      t1,SDRAM_MODE_REG
        sw      t1,0x300(t0)
        sw      t4,0x80(t5)

        li      t0,TIMER_BASE
        li      t1,DRAM_RF_CMPR_BS
        sw      t1,0x68(t0)
        li      t1,ENABLE_TIMER
        sw      t1,0x60(t0)


       /* Delay for Sdram to stabilise with fast refresh */
        li      t2, 0x2FF00
1:
        subu    t2, 1
        bne     t2, zero,1b
        nop

       /* Change the Sdram refresh to a standard refresh */
        li      t0,TIMER_BASE
        li      t1,DRAM_RF_CMPR_SE_BS
        sw      t1,0x68(t0)

/* SDRAM Initialization : End Here !  */


		jr	ra
		nop

FUNC_END(hal_memc_setup)


FUNC_START(hal_setTlbEntry)
         .set noreorder
		mtc0 a0,C0_INX
		mtc0 a1,C0_TLBHI
		mtc0 a2,C0_TLBLO0
		mtc0 a3,C0_TLBLO1
		nop
		nop
		tlbwi
		nop
.set reorder

		j ra
         nop
FUNC_END(hal_setTlbEntry)

FUNC_START(hal_setPageSize)
        .set noreorder
        mtc0  a0,C0_PAGEMASK
        nop
        nop
        .set reorder
        j    ra
        nop
FUNC_END(hal_setPageSize)




FUNC_START (hal_sysConfigOutByte)
      .set noreorder
      li   t0,0xb8002cf8
      li   t1,0xb8002cfc
      li   t2,0x3
      and  a2,t2
      sw   a0,0x0(t0)
      xori a2,0x3
      or   t1,a2
      sb   a1,0x0(t1)
      j    ra
      nop
      .set reorder
FUNC_END(hal_sysConfigOutByte)



FUNC_START(hal_sysConfigOutHalfWord)
      .set noreorder
      li   t0,0xb8002cf8
      li   t1,0xb8002cfc
      li   t2,0x3
      and  a2,t2
      sw   a0,0x0(t0)
      xori a2,0x2
      or   t1,a2
      sh   a1,0x0(t1)
      j    ra
      nop
      .set reorder
FUNC_END (hal_sysConfigOutHalfWord)

FUNC_START (hal_sysConfigOutWord)
      .set noreorder
      li   t0,0xb8002cf8
      li   t1,0xb8002cfc
      sw   a0,0x0(t0)
      sw   a1,0x0(t1)
      j    ra
      nop
      .set reorder
FUNC_END (hal_sysConfigOutWord)
     

/*
 * Function:
 *	unsigned char sysConfigInByte (int pciConfigAddress) ;
 * Inputs:
 *	a0 - pciConfigAddress
 * Outputs:
 *	v0 - byte read from PCI Configuration space.
 */ 
 FUNC_START (hal_sysConfigInByte)
sysConfigInByte:
	li	t0, 0xb8000000	/* t0 = Upper 16 bits of PCI config */
					/*	space address register */
	andi	t1, a0, 0x00000003	/* t1 = PCI config space byte offset. */
	xor	t2, t1, a0		/* t2 = PCI config address of	*/
					/*	longword containing byte. */
	xori	t1, 0x00000003	/* t1 = big endian byte offset. */
					/*	offset 0 -> 3 */
					/*	offset 1 -> 2 */
					/*	offset 2 -> 1 */
					/*	offset 3 -> 0 */
	or	t1, t0			/* t1 = byte corrected PCI config */
					/*	data address. */
	.set	noreorder		/* Instructions must not be moved ! */
	sw	t2, 0x2cf8 (t0)		/* Write PCI config space address reg */
	lw	$0, 0x2cf8 (t0)		/* wbflush() insures write-then-read */

	lbu	v0, 0x2cfc (t1)		/* v0 = PCI config space byte read */

	j	ra			/* Return to caller. */
	sw	$0, 0x2cf8 (t0)		/* BDSLOT - Park PCI bus */

	.set	reorder
FUNC_END (hal_sysConfigInByte)

/*
 * Function:
 *	unsigned short sysConfigInHalfWord (int pciConfigAddress) ;
 *
 * Inputs:
 *	a0 - pciConfigAddress
 *
 * Outputs:
 *	v0 - half word read from PCI Configuration space.
 */ 

FUNC_START (hal_sysConfigInHalfWord)
sysConfigInHalfWord:
	li	t0, 0xb8000000	/* t0 = Upper 16 bits of PCI config */
					/*	space address register */
	andi	t1, a0, 0x00000003	/* t1 = Config space halfword offset. */
	xor	t2, t1, a0		/* t2 = PCI config address of	*/
					/*	longword containing halfword. */

	xori	t1, 0x00000002	/* t1 = big endian halfword offset. */
					/*	offset 0 -> 2 */
					/*	offset 2 -> 0 */

	or	t1, t0			/* t1 = halfword corrected PCI config */
					/*	data address. */
	.set	noreorder		/* Instructions must not be moved ! */
	sw	t2, 0x2cf8 (t0)		/* Write PCI config space address reg */
	lw	$0, 0x2cf8 (t0)		/* wbflush() insures write-then-read */

	lhu	v0, 0x2cfc (t1)		/* v0 = config space halfword read */

	j	ra			/* Return to caller. */
	sw	$0, 0x2cf8 (t0)		/* BDSLOT - Park PCI bus */

	.set	reorder
FUNC_END (hal_sysConfigInHalfWord)

/*
 * Function:
 *	unsigned short sysConfigInWord (int pciConfigAddress) ;
 *
 * Inputs:
 *	a0 - pciConfigAddress
 *
 * Outputs:
 *	v0 - word read from PCI Configuration space.
 */ 

FUNC_START (hal_sysConfigInWord)
	li	t0, 0xb8000000		/* t0 = Upper 16 bits of PCI config */
					/*	space address register */
	.set	noreorder		/* Instructions must not be moved ! */
	sw	a0, 0x2cf8 (t0)		/* Write PCI config space address reg */
	lw	$0, 0x2cf8 (t0)		/* wbflush() insures write-then-read */

	lw	v0, 0x2cfc (t0)		/* v0 = config space word read */

	j	ra			/* Return to caller. */
	sw	$0, 0x2cf8 (t0)		/* BDSLOT - Park PCI bus */

	.set	reorder
FUNC_END(hal_sysConfigInWord)


##-----------------------------------------------------------------------------
# Interrupt vector tables.
# These tables contain the isr, data and object pointers used to deliver
# interrupts to user code.
		
	.extern hal_default_isr
	.data

	.globl	hal_interrupt_handlers
hal_interrupt_handlers:
	.rept	30
	.long	hal_default_isr
	.endr

	.globl	hal_interrupt_data
hal_interrupt_data:
	.rept	30
	.long	0
	.endr

	.globl	hal_interrupt_objects
hal_interrupt_objects:
	.rept	30
	.long	0
	.endr

##-----------------------------------------------------------------------------
## end of platform.S
	

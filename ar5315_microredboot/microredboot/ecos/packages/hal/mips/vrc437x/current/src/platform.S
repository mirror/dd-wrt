##=============================================================================
##
##	platform.S
##
##	MIPS VRC4372 platform code
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
## Author(s): 	nickg
## Contributors:	nickg
## Date:	1999-04-20
## Purpose:	MIPS vrc4372 platform code
## Description:	Platform specific code for VRC4372 board.
##              
##		
##		
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

##-----------------------------------------------------------------------------
## ISR springboard.
## This routine decodes the interrupt from the VRC4372 interrupt controller
## and vectors to it.

	# On entry:
	# a0 = MIPS status register interrupt number (1,2 or 3)
	# a1 = ISR data value (= interrupt status reg address)
	# a2 = saved reg dump ptr
	# s0 = saved reg dump ptr
	# s1 = vector table offset
	# s2 = interrupt number
	# a3,v0,v1 etc available for use

	.text
			
hal_isr_springboard:
		
	lw	v0,0(a1)			# v0 = stat reg value

	# The following code implements an ls bit index algorithm similar
	# to that in hal_lsbit_index() in hal_misc.c. 
	negu	v1,v0				# v1 = -v0
	and	v1,v1,v0			# v1 &= v0 [isolate ls bit]
	sll	v0,v1,16			# v0 = v1<<16
	subu	v1,v0,v1			# v1 = v0 - v1
	sll	a0,v1,6				# a0 = v1<<6
	addu	v1,v1,a0			# v1 += a0
	sll	a1,v1,4				# a1 = v1<<4
	addu	v1,v1,a1			# v1 += a1
	la	v0,hal_isr_springboard_table	# v0 = table address
	srl	v1,v1,26			# v1 = v1>>26
	addu	v1,v1,v0			# v1 = table entry address
	lb	a0,0(v1)			# a0 = intc isr number
	
	addi	s2,a0,6				# s2 = eCos isr number
	sll	s1,s2,2				# s1 = isr table index

	la	v1,hal_interrupt_handlers
	addu	v1,v1,s1			# v1 = isr handler address
	lw	v1,0(v1)			# v1 = isr handler

	la	a1,hal_interrupt_data
	addu	a1,a1,s1			# a1 = address of data ptr
	lw	a1,0(a1)			# a1 = data pointer

	move	a0,s2				# pass interrupt number	

	jr	v1				# jump to handler, return is to
						# default vsr already in ra
	nop

hal_isr_springboard_table:	
	.byte  -1,  0,  1, 12,  2,  6,  0, 13
	.byte   3,  0,  7,  0,  0,  0,  0, 14
	.byte  10,  4,  0,  0,  8,  0,  0, 25
	.byte   0,  0,  0,  0,  0, 21, 27, 15
	.byte  31, 11,  5,  0,  0,  0,  0,  0
	.byte   9,  0,  0, 24,  0,  0, 20, 26
	.byte  30,  0,  0,  0,  0, 23,  0, 19
	.byte  29,  0, 22, 18, 28, 17, 16,  0

##-----------------------------------------------------------------------------
## Data Load/Store Bus error VSR
##
## If the VRC4373 tries to access a PCI config space register that does
## not have a real device behind it, it provokes a bus error.
## This is a special bus error VSR that detects when we get a bus error
## from a PCI configuration space access and fixes it up to allow the 
## program to continue.
## This VSR works in conjunction with hal_pci_config_read().
## Essentially, if we get a bus error it checks the EPC value and if it 
## is at the instruction that accesses the PCI bus, in hal_pci_config_read(),
## it skips it and fixes the registers to allow the program to continue.
## None of this would be necessary if the VRC4373 behaved sensibly and
## returned 0xFFFFFFFF for reads from empty device slots like it should.

	.text
	
	.globl	hal_bus_error_vsr	
hal_bus_error_vsr:
	# We come here with everything still in registers and:
	# K0 = vector number (==7*4)
	# K1 = address of this routine

	# Check for special address
	
	la	k1,hal_pci_config_read_load
	mvafc0	k0,epc
	bne	k0,k1,1f
	nop

	# We have a match, skip the offending instruction
	# and put 0xFFFFFFFF into V0.
	# First we must clear the EXL bit so that we can
	# write to EPC.
	mfc0	k1,status		# Get status reg
	la	v0,0xFFFFFFFD		# mask for EXL
	and	k1,v0,k1		# clear EXL
	mtc0	k1,status		# return to CP0
	nop				# let it work
	nop
	nop
	addi	k0,k0,4			# skip offending instruction
	mvatc0	k0,epc			# return to EPC
	nop				# let it work
	nop
	nop
	la	v0,0xFFFFFFFF		# Put 0xFFFFFFFF into v0
	sync				# let everything settle
	eret				# and return
	nop

	# If this is not the special location, we need to continue
	# with the usual bus error handling. Since we no longer have
	# the original VSR table entry, we use the one just before it
	# (for instruction fetch bus error). At present this points
	# to the same error handler.
1:
	la	k1,hal_vsr_table	# Get VSR table
	lw	k1,24(k1)		# Pick another vector to follow
	la	k0,28			# but pretend we are still Bus Error
	jr	k1			# go there
	nop
	
	
##-----------------------------------------------------------------------------
## PCI config space access function
##
## This is the only function that should be used to read data from the PCI
## configuration space data register. It works with the bus error VSR
## above to work around any bus errors provoked by the VRC4373.
	
FUNC_START(hal_pci_config_read)

hal_pci_config_read_load:
        .set noreorder
	lw	v0,0(a0)	# Read the value. If this bus-errors the
				# handler will skip this instruction and
				# put 0xFFFFFFFF into v0.
	jr	ra		# And return
	nop
	
FUNC_END(hal_pci_config_read)
				
	
##-----------------------------------------------------------------------------
## Breakpoint springboard

## The following value has been determined by looking at the code that
## PMON puts in the interrupt vector. Clearly it will be different for
## different versions of PMON. A better approach might be to copy the original
## code out and execute that here. However, there is no guarantee that it is
## position independent. 
			
#define CYGHWR_MIPS_PMON_EXEPTION_ENTRY 0xa0000dd4

	.text
			
	.globl hal_breakpoint_springboard
hal_breakpoint_springboard:

	# We enter here with all of the CPU state still
	# in its registers except:
	# K0 = vector index
	# K1 = address of this function

	la	k0,CYGHWR_MIPS_PMON_EXEPTION_ENTRY
	jr	k0	
	nop				

##-----------------------------------------------------------------------------
## ISR tables.
			
	.extern hal_default_isr
	
	.data

	.globl	hal_interrupt_handlers
hal_interrupt_handlers:
	.long	hal_default_isr
	.long	hal_isr_springboard
	.long	hal_isr_springboard
	.long	hal_isr_springboard
	.long	hal_default_isr
	.long	hal_default_isr

	.rept	32
	.long	hal_default_isr
	.endr
	
	.globl	hal_interrupt_data
hal_interrupt_data:

	.long	0
	.long	CYGHWR_HAL_MIPS_VRC4373_INTC_STAT0
	.long	CYGHWR_HAL_MIPS_VRC4373_INTC_STAT1
	.long	CYGHWR_HAL_MIPS_VRC4373_INTC_STAT2
	.long	0
	.long	0

	.rept	32
	.long	0
	.endr

	.globl	hal_interrupt_objects
hal_interrupt_objects:
	.rept	38
	.long	0
	.endr

	.globl	cyg_hal_interrupt_level
cyg_hal_interrupt_level:
	.rept	38
	.byte	0
	.endr

##-----------------------------------------------------------------------------
## MMU setup.
## Much of this code is taken from the PMON sources, hence it does not fully
## conform to our normal coding conventions.

#if defined(CYG_HAL_STARTUP_ROM) || defined(CYG_HAL_STARTUP_ROMRAM)

## DEFINITIONS FOR THE TLB SUPPORT

#define IO_PHYSICAL_BASE 0x80000000
#define IO_VIRTUAL_BASE  0xc0000000
#define NTLBENTRIES      32 
#define K0BASE           0x80000000
#define TLBLO_G          0x00000001
#define TLBLO_V          0x00000002
#define TLBLO_D          0x00000004
#define TLBPGMASK_MASK   0x01ffe000
#define TLBINX_INXMASK   0x0000003f
#define CFG_C_UNCACHED   2
#define TLBLO_CSHIFT     3
#define TLBLO_PFNSHIFT   6
#define TLBLO_PFNMASK    0x3fffffc0
#define TLBHI_VPN2MASK	0xffffe000
#define TLBHI_VPN2SHIFT	13
#define TLBHI_VPNMASK	0xfffff000
#define TLBHI_VPNSHIFT	12


#define TLBLO_UNCACHED  (CFG_C_UNCACHED<<TLBLO_CSHIFT)
#define PAGE_SIZE       0x01000000
#define PADDR_INC       0x02000000
#define NUMB_PG         8

	.text
	
	.set noreorder
		
	.extern	hal_mmu_setup_return
	
FUNC_START(hal_mmu_setup)

	mtc0	zero,tlbhi
	nop
	li      a0,0
2:
	bal     resettlb
	nop
	addiu   a0,a0,1
	bne     a0,NTLBENTRIES,2b
	nop
	

#define tlblo_even	s0
#define tlblo_odd	s1
#define vaddr           s2
#define paddr0          s3
#define paddr1          s4
#define pmask           s5
#define tmp	        s6
#define ix              k1


	li	ix,0
	li	vaddr,IO_VIRTUAL_BASE
	li	paddr0,IO_PHYSICAL_BASE
loop:

        li	a0,((PAGE_SIZE - 1) << 1)
        li	a1,TLBPGMASK_MASK
        and	pmask, a0,a1

	li	tlblo_even, TLBLO_V | TLBLO_D | TLBLO_G
	li	tlblo_odd, TLBLO_V | TLBLO_D | TLBLO_G
 
	li	tmp,PAGE_SIZE
	addu	paddr1,paddr0,tmp
	li	tmp,TLBHI_VPN2MASK
        and	a1,vaddr,tmp

	li	tmp,TLBLO_PFNSHIFT
	srl	a2,paddr0,tmp
	li	tmp,TLBLO_PFNMASK
	and	a2,a2,tmp
	or	a2,a2,TLBLO_UNCACHED
	or	a2,a2,tlblo_even
	
	li	tmp,TLBLO_PFNSHIFT
	srl	a3,paddr1,tmp
	li	tmp,TLBLO_PFNMASK
	and	a3,a3,tmp
	or	a3,a3,TLBLO_UNCACHED
	or	a3,a3,tlblo_odd


	move	a0,pmask
	or	a0,a0,ix

	bal	map_tlb4000
	nop
	
	addi	ix,ix,1

	li	tmp,PADDR_INC
	addu	vaddr,vaddr,tmp
	addu	paddr0,paddr0,tmp

	li	tmp,NUMB_PG
	bne	ix,tmp,loop
	nop

	# Go back via a jump, not a return
	lar	k0,hal_mmu_setup_return
	jr	k0
	nop
		
FUNC_END(hal_mmu_setup)			


## Invalidate the TLB entry specified by index
##
## resettlb(index) 
##	a0  =  index
##
## Uses : t0, t2, v0, a0 

FUNC_START(resettlb)

	li	t2,K0BASE & TLBHI_VPN2MASK
	mfc0	t0,tlbhi		# save current TLBHI
	mfc0	v0,status		# save SR and disable interrupts
	mtc0	zero,status		# clear interrupts
	mtc0	t2,tlbhi		# invalidate entry
	mtc0	zero,tlblo0
	mtc0	zero,tlblo1
	mtc0	a0,index
	nop; nop; nop; nop; nop; nop; nop; nop
	tlbwi
	nop; nop; nop; nop; nop; nop; nop; nop
	mtc0	t0,tlbhi
	mtc0	v0,status
	j	ra
	nop
	
FUNC_END(resettlb)

## Setup R4000 TLB entry
##
## map_tlb4000(mask_index, tlbhi, pte_even, pte_odd)
## 	a0  =  TLB entry index and page mask
##	a1  =  virtual page number and PID
##      a2  =  pte -- contents of even pte
##      a3  =  pte -- contents of odd pte
##
##
##
## Uses : t0, t1, t2, v0, a0, a1, a2, a3 

FUNC_START(map_tlb4000)
	and	t2,a0,TLBPGMASK_MASK
	and	a0,TLBINX_INXMASK
	mfc0	t1,tlbhi		# save current TLBPID
	mfc0	v0,status		# save SR and disable interrupts
	mtc0	zero,status		# clear interrupts
	mtc0	t2,pagemask		# set 
	mtc0	a1,tlbhi		# set VPN and TLBPID
	mtc0	a2,tlblo0		# set PPN and access bits
	mtc0	a3,tlblo1		# set PPN and access bits
	mtc0	a0,index		# set INDEX to wired entry
	nop; nop; nop; nop; nop; nop; nop; nop
	tlbwi				# drop it in
	nop; nop; nop; nop; nop; nop; nop; nop
	mtc0	t1,tlbhi		# restore TLBPID
	mtc0	v0,status		# restore SR
	j	ra
	nop
FUNC_END(map_tlb4000)

#endif
	
##-----------------------------------------------------------------------------
## MEMC initialization.
## This also initializes the PCI bus and ISA bus bridge, so at the end of this
## we should have full access to all the memory and devices we need.
## This code is table driven, which is somewhat more compact that coding it all.
## Table entries consist of an address and a value to store in that address.
## A zero address terminates the table. Two special address values modify the
## behaviour:	
## DELAY_LOOP	loops for the number of iterations in the value field.
## WRITE16	treats the next 2 words as an address and value to be written 
##        	with a 16 bit write cycle.
	
#if defined(CYG_HAL_STARTUP_ROM) || defined(CYG_HAL_STARTUP_ROMRAM)
	
#define	DELAY_LOOP	1
#define WRITE16		2
#define WRITEREAD32	3	

		
FUNC_START(hal_memc_setup)

	lar	t0,hal_memc_setup_table
	la	t1,0xbfc00000
	la	t2,DELAY_LOOP
	la	t3,WRITE16
	la	t4,WRITEREAD32
1:	
	lw	a0,0(t0)	# next table entry
	lw	a1,4(t0)	# value to write
	addiu	t0,8		# go to next entry
	beq	a0,t2,2f	# Check for delay
	nop
	beq	a0,t3,3f	# Check for 16 bit write
	nop
	beq	a0,t4,4f	# Check for 32 bit write/read
	nop
	beqz	a0,9f		# zero terminates loop
	nop
	lw	zero,0(t1)	# uncached read to flush write buffer
	sw	a1,0(a0)	# write it
	lw	zero,0(t1)	# uncached read to flush write buffer
	b	1b
	nop
4:
	lw	a3,0(t0)	# get next word
	addiu	t0,4		# skip it
	lw	zero,0(t1)	# uncached read to flush write buffer
	sw	a3,0(a1)	# write it
	lw	a3,0(a1)	# and read it back
	lw	zero,0(t1)	# uncached read to flush write buffer
	b	1b
	nop	
2:
	lw	zero,0(t1)	# uncached read to flush write buffer	
	bnez	a1,2b		# count down by value in a1
	addiu	a1,-1		# decrement in delay slot
	b	1b		# go back to loop
	nop
3:	
	lw	a3,0(t0)	# get next word
	addiu	t0,4		# skip it
	sh	a3,0(a1)	# store halfword	
	lw	zero,0(t1)	# uncached read to flush write buffer
	b	1b
	nop
9:
	jr	ra
	nop

FUNC_END(hal_memc_setup)	

##-----------------------------------------------------------------------------	
## Definitions for the VRC4373, also known as the Nile I, 
## and maybe the Nile III as well.
	
#define N1_BASE		0xAF000000
#define N2_BASE		0xBC000000
		
#define BMCTRL         (N1_BASE + 0x00)
#define S1CTRL         (N1_BASE + 0x04)
#define S2CTRL         (N1_BASE + 0x08)
#define S3CTRL         (N1_BASE + 0x0C)
#define S4CTRL         (N1_BASE + 0x10)
#define PCIMSTRADDR1   (N1_BASE + 0x14)
#define PCIMSTRADDR2   (N1_BASE + 0x18)
#define PCISLAVADDR1   (N1_BASE + 0x1C)
#define PCISLAVADDR2   (N1_BASE + 0x20)
#define PCIMSTRIO      (N1_BASE + 0x24)
#define PCICNFGDATA    (N1_BASE + 0x28)
#define PCICNFGADDR    (N1_BASE + 0x2C)
#define PCIMBOX1       (N1_BASE + 0x30)
#define PCIMBOX2       (N1_BASE + 0x34)
#define DMACTRL1       (N1_BASE + 0x38)
#define DMALCLADDR1    (N1_BASE + 0x3C)
#define DMAPCIADDR1    (N1_BASE + 0x40)
#define DMACTRL2       (N1_BASE + 0x44)
#define DMALCLADDR2    (N1_BASE + 0x48)
#define DMAPCIADDR2    (N1_BASE + 0x4C)
#define BUSERRSTAT     (N1_BASE + 0x50)
#define INTCNTRL       (N1_BASE + 0x54)
#define REFRESHRATE    (N1_BASE + 0x58)
#define BOOTROMWPRTCT  (N1_BASE + 0x5C)
#define PCI_EXCL_ACC   (N1_BASE + 0x60)
#define PCI_ENABLE_REG (N1_BASE + 0x74)
#define N3_POM_INIT    (N1_BASE + 0x78)
/*
#define N1RESERVED1    (N1_BASE + 0x60)
*/

#if defined(CYGPKG_HAL_MIPS_VR4300_VRC4375)
#define N3_EM		(N1_BASE + 0x7C)
#define N3_ARB		(N1_BASE + 0x80)
#define UARTRBR		(N1_BASE + 0x84)
#define UARTTHR		(N1_BASE + 0x84)
#define UARTIER		(N1_BASE + 0x88)
#define UARTDLL		(N1_BASE + 0x84)
#define UARTDLM		(N1_BASE + 0x88)
#define UARTIIR		(N1_BASE + 0x8C)
#define UARTFCR		(N1_BASE + 0x8C)
#define UARTLCR		(N1_BASE + 0x90)
#define UARTMCR		(N1_BASE + 0x94)
#define UARTLSR		(N1_BASE + 0x98)
#define UARTMSR		(N1_BASE + 0x9C)
#define UARTSCR		(N1_BASE + 0xA0)
#endif

#define N1PCIHEADER    (N1_BASE + 0x100)
#define N1PCICMD       (N1PCIHEADER + 0x04)
#define MBOXPCIADDR    (N1PCIHEADER + 0x10)

#define N1CLSIZE       (N1PCIHEADER + 0x0c)  /* Cache Line Size */
#define N1MLTIM        (N1PCIHEADER + 0x0d)
#define N1MBADD        (N1PCIHEADER + 0x10)
#define N1RSVD2        (N1PCIHEADER + 0x40)  /* Word boundary */
#define N1RTYTIM       (N1PCIHEADER + 0x41)

##-----------------------------------------------------------------------------	
## Definitions for the VRC4372, also known as the Nile II.	

#if defined(CYGPKG_HAL_MIPS_VR4300_VRC4373)	
#define PCI_N2_DEV		0x00010000 	/* PCI Device # - Nile 2 */
#elif defined(CYGPKG_HAL_MIPS_VR4300_VRC4375)	
#define PCI_N2_DEV		0x00080000 	/* PCI Device # - Nile 2 */
#endif
	
#define N2CLSIZE		( 0x0c)  /* Cache Line Size */
#define N2IOADD			( 0x14)
#define N2DSCTIM		( 0x40)

#define IOCON             (N2_BASE + 0x0000)

  /* GPIO Bus   Controller Registers    */
#define GPDIR             (IOCON + 0x0100)
#define GPOUT             (IOCON + 0x0104)
#define GPIN              (IOCON + 0x0108)

  /* Interrupt  Controller Registers    */
#define INTPOL            (IOCON + 0x200)
#define INTTRIG           (IOCON + 0x204)
#define INTPINS           (IOCON + 0x208)
#define INTMASK0          (IOCON + 0x20C)
#define INTSTAT0          (IOCON + 0x210)
#define INTMASK1          (IOCON + 0x214)
#define INTSTAT1          (IOCON + 0x218)
#define INTMASK2          (IOCON + 0x21C)
#define INTSTAT2          (IOCON + 0x220)
		
  /* IO Bus     Controller Registers    */
#if defined(CYGPKG_HAL_MIPS_MSBFIRST)
// The 16 bit registers are swapped pairwise in each word.	
#define IOPROF_00         (IOCON + 0x0402)
#define IOPROF_01         (IOCON + 0x0400)
#define IOPROF_02         (IOCON + 0x0406)
#define IOPROF_03         (IOCON + 0x0404)
#define IOPROF_04         (IOCON + 0x040A)
#define IOPROF_05         (IOCON + 0x0408)
#define IOPROF_06         (IOCON + 0x040E)
#define IOPROF_07         (IOCON + 0x040C)
#define IOPROF_08         (IOCON + 0x0412)
#define IOPROF_09         (IOCON + 0x0410)
#define IOPROF_0A         (IOCON + 0x0416)
#define IOPROF_0B         (IOCON + 0x0414)
#define IOPROF_0C         (IOCON + 0x041a)
#define IOPROF_10         (IOCON + 0x0422)
#define IOPROF_11         (IOCON + 0x0420)
#define IOPROF_12         (IOCON + 0x0426)
#define IOPROF_13         (IOCON + 0x0424)	
#elif defined(CYGPKG_HAL_MIPS_LSBFIRST)
#define IOPROF_00         (IOCON + 0x0400)
#define IOPROF_01         (IOCON + 0x0402)
#define IOPROF_02         (IOCON + 0x0404)
#define IOPROF_03         (IOCON + 0x0406)
#define IOPROF_04         (IOCON + 0x0408)
#define IOPROF_05         (IOCON + 0x040A)
#define IOPROF_06         (IOCON + 0x040C)
#define IOPROF_07         (IOCON + 0x040E)
#define IOPROF_08         (IOCON + 0x0410)
#define IOPROF_09         (IOCON + 0x0412)
#define IOPROF_0A         (IOCON + 0x0414)
#define IOPROF_0B         (IOCON + 0x0416)
#define IOPROF_0C         (IOCON + 0x0418)
#define IOPROF_10         (IOCON + 0x0420)
#define IOPROF_11         (IOCON + 0x0422)
#define IOPROF_12         (IOCON + 0x0424)
#define IOPROF_13         (IOCON + 0x0426)
#else
#error MIPS endianness not define by configuration		
#endif	
#define IOTOUT            (IOCON + 0x0430)
#define IOPOL             (IOCON + 0x0434)
	
##-----------------------------------------------------------------------------			
## PCI definitions
	
#define PCI_IOSPACE_BASE	0x0c		/* PCI I/O Space Adr */

#define PCIADR_REGNO 	0x000000fc  /* Register # mask 		 */
	
#define PCI_CONFIG( dev, func, reg ) ( dev | ( reg  & PCIADR_REGNO) ) 	

#define	PCI_COMMAND_STATUS_REG		0x04
#define	PCI_MAP_REG_START		0x10
#define	PCI_INTERRUPT_REG		0x3c

##-----------------------------------------------------------------------------
## The initialization table
	
hal_memc_setup_table:	

	.long	DELAY_LOOP,	0x00010000	# Wait for HW to settle


	# Set up SDRAM and SIMM1

#if defined(CYGPKG_HAL_MIPS_VR4300_VRC4373)

	.long	BMCTRL,		0x0000803b	# Base memory setup
	.long	S1CTRL,		0x0402013f	# SIMM1 

#elif defined(CYGPKG_HAL_MIPS_VR4300_VRC4375)

	.long	N3_EM,		0x00000000
	.long	N3_ARB,		0x00000000
	
	.long	BMCTRL,		0x0000100b	# Base memory setup

	# Dont set up SIMMs on VR4375 at present.
	
#else
#error Unsupported board	
#endif

	# Set up power-on memory state
	.long			N3_POM_INIT,	0x00000002	# Power-on memory init
	.long	DELAY_LOOP,	0x00000100			# wait a bit
	.long	WRITEREAD32,	N3_POM_INIT,	0x00000004	# Power-on memory init
	.long	DELAY_LOOP,	0x00000100			# wait a bit
	.long	WRITEREAD32,	N3_POM_INIT,	0x00000001	# Power-on memory init
	.long	DELAY_LOOP,	0x00000100			# wait a bit
	.long	REFRESHRATE,	0x00000fff			# DRAM refresh rate

#if defined(CYGPKG_HAL_MIPS_VR4300_VRC4375)
	.long	N3_ARB,		0x00000005
#endif
		
	.long	BOOTROMWPRTCT,	0xffffff3e	# Write protect boot ROM

	# Set up PCI master address window
	.long	PCIMSTRADDR1,	0x1c0fd01c
	.long	PCIMSTRADDR2,	0x80081080

	# Set up PCI slave address window
	.long	PCISLAVADDR1,	0x00081000
	.long	PCISLAVADDR2,	0x10081080

	# Map PCI IO space Phys == Local
	.long	PCIMSTRIO,	0x0c0fd00c

	# VRC437X Bridge config space

	.long	N1PCICMD,	0x00000006	# Memory space enable, bus master
	.long	N1CLSIZE,	0x00003000	# Master latency time
	.long	N1MBADD,	0x20000000	# Mailbox base address
	.long	N1RSVD2,	0x0000f000	# Retry value

	.long	PCI_EXCL_ACC,	0x00000020	# PCI exclusive access register
	.long	PCI_ENABLE_REG,	0x00000005	# PCI config done, enable arbitration

	# Set up 4372. This is configured entirely through its PCI
	# configuration space. Hence the following all consist of
	# writing an address to the PCICNFGADDR register followed
	# by a write of the data to the PCICNFGDATA register.

	.long	PCICNFGADDR,	PCI_CONFIG( PCI_N2_DEV, 0, PCI_COMMAND_STATUS_REG)
	.long	PCICNFGDATA,	0x00000146

	.long	PCICNFGADDR,	PCI_CONFIG( PCI_N2_DEV, 0, N2CLSIZE)
	.long	PCICNFGDATA,	0x00003000

	.long	PCICNFGADDR,	PCI_CONFIG( PCI_N2_DEV, 0, PCI_MAP_REG_START)
	.long	PCICNFGDATA,	0x1c000000

	.long	PCICNFGADDR,	PCI_CONFIG( PCI_N2_DEV, 0, N2IOADD)
	.long	PCICNFGDATA,	0x80000000

	.long	PCICNFGADDR,	PCI_CONFIG( PCI_N2_DEV, 0, PCI_INTERRUPT_REG)
	.long	PCICNFGDATA,	0x00000000

	.long	PCICNFGADDR,	PCI_CONFIG( PCI_N2_DEV, 0, N2DSCTIM)
	.long	PCICNFGDATA,	0x0070f000

	# Set up IO bus controller chip select profile registers to
	# allow access to the devices.

	.long	WRITE16,	IOPROF_00,	0x04d1	# RTC - DS1386
	.long	WRITE16,	IOPROF_01,	0x007f	# DUART - Z8530
	.long	WRITE16,	IOPROF_02,	0x007f	# Timer - Z8536
	.long	WRITE16,	IOPROF_03,	0x00d9	# Parallel = 8255
	.long	WRITE16,	IOPROF_06,	0x007f	# 7 segment display
	.long	WRITE16,	IOPROF_07,	0x007f	# 7 segment display

	# Configure the rest of the 4372

	.long	IOTOUT,		0x200003c0	# IO timout control
	.long	IOPOL,		0x00000000	# IO bus pin polarity
	.long	GPDIR,		0x000007c0	# GPIO pins 6-10 output
	.long	INTPOL,		0x00003fff	# Interrupt polarity
	.long	INTTRIG,	0x00003fff	# Interrupt trigger
	.long	INTMASK0,	0x00000000
	.long	INTMASK1,	0x00000000
	.long	INTMASK2,	0x00000000
	.long	INTSTAT0,	0x00000000	# Zero all int status regs
	.long	INTSTAT1,	0x00000000
	.long	INTSTAT2,	0x00000000

	.long	DELAY_LOOP,	0x00010000	# Wait for HW to settle

	.long	0,		0

#endif
		
##-----------------------------------------------------------------------------
## end of platform.S
	

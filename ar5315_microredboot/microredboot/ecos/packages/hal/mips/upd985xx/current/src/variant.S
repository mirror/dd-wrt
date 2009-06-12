##=============================================================================
##
##	variant.S
##
##	MIPS uPD9875xx device code
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
## Author(s): 	hmt, nickg
## Contributors:nickg
## Date:	2001-05-25
## Purpose:	NEC MIPS uPD985xx device code
## Description:	Device specific code
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

#include <cyg/hal/var_arch.h>

##-----------------------------------------------------------------------------
## MMU setup.
## Some of this code is taken from the PMON sources, hence it does not fully
## conform to our normal coding conventions.

#if defined(CYG_HAL_STARTUP_ROM) || defined(CYG_HAL_STARTUP_ROMRAM)

// There is none.  We can access all areas via kseg[01] so we are happy
// with no MMU setup.

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

		
FUNC_START(hal_memc_setup)

	lar	t0,hal_memc_setup_table
	la	t1,0xbfc00000
	la	t2,DELAY_LOOP
	la	t3,WRITE16
1:	
	lw	a0,0(t0)	# next table entry
	lw	a1,4(t0)	# value to write
	addiu	t0,8		# go to next entry
	beq	a0,t2,2f	# Check for delay
	nop
	beq	a0,t3,3f	# Check for 16 bit write
	nop
	beqz	a0,9f		# zero terminates loop
	nop
	sw	a1,0(a0)	# write it
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
##-----------------------------------------------------------------------------
## The initialization table
	
hal_memc_setup_table:	
	.long	DELAY_LOOP,	0x00010000	# Wait for HW to settle

	.long	RMMDR_ADR, 	RMMDR_28F640
	.long	RMATR_ADR,	RMATR_28F640

	.long	DELAY_LOOP,	0x00010000	# Wait for HW to settle
		
        .long	SDMDR_ADR,	SDMDR_INIT
        .long	SDTSR_ADR,	SDTSR_INIT
        .long	SDPTR_ADR,	SDPTR_INIT
        .long	SDRMR_ADR,	SDRMR_INIT
	
	.long	DELAY_LOOP,	0x00010000	# Wait for HW to settle
		
	.long	0,		0


#endif
		
##-----------------------------------------------------------------------------
## end of platform.S
	

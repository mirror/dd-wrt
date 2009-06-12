##=============================================================================
##
##	variant.S
##
##	MN10300 AM33 variant code
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
## Date:	1997-10-16
## Purpose:	MN10300 AM33 variant code
## Description:	This file contains variant specific assembly code for the AM33.
##
######DESCRIPTIONEND####
##
##=============================================================================

#include <pkgconf/hal.h>

#include <cyg/hal/arch.inc>	


##-----------------------------------------------------------------------------
## NMI trampoline VSRs. All NMI interrupts are routed here initially, where we
## decode the NMICR and ISR register contents and vector to the
## correct VSR later in the table.

	.text

	.extern hal_lsbit_table
			
	.globl nmi_vsr_trampoline
nmi_vsr_trampoline:
	movhu	(NMICR),d0		# D0 = NMI control register
	and	0x7,d0			# LS 3 bits only
	mov	3,d1			# search from bit 3
	bsch	d0,d1			# get ms bit in d1
	bcs	2f			# jump if no bits there
	add	9,d1			# D1 = offset into VSR table
	jmp	3f			# go to rest of code

	.globl nmi_sysef_trampoline
nmi_sysef_trampoline:		
	movhu	(ISR),d0		# D0 = Interrupt Status Register
	mov	d0,d1			# D1 = copy of D0
	add	-1,d1			# D1 = D0-1
	not	d1			# D1 = ~(D0-1) = -D0
	and	d1,d0			# D0 = D0 & -D0 = ls bit only
	mov	16,d1			# D1 = start of search bit
	bsch	d0,d1			# search for 1 bit in d0
	bcs	2f			# jump if failed
	add	12,d1			# d1 = offset in VSR table
3:	
	asl	2,d1			# D1 = word offset in vsr table
	mov	_hal_vsr_table,a0	# A0 = VSR table base
	add	d1,a0			# A0 = address of table entry we want
	mov	(a0),a0			# A0 = VSR to call
	jmp	(a0)			# Call it
	
2:	
	# no bits set in ISR or NMICR when we expected them.
	# This should never happen, but if it does, use an otherwise
	# unused VSR table entry to indicate this.
	
	mov	27,d1			# use last VSR table entry.
	jmp	3b
	
	
##-----------------------------------------------------------------------------

#define HAL_CHCTR               0xC0000070
#define HAL_DCACHE_PURGE_WAY0   0xC8400000
#define HAL_CHCTR_DCEN          0x0002
#define HAL_CHCTR_DCBUSY        0x0008

	.text
	
	.globl	_cyg_hal_dcache_store
_cyg_hal_dcache_store:	
	movm	[d2,d3],(sp)

	mov	HAL_CHCTR,a0			# A0 = control reg
	mov	HAL_DCACHE_PURGE_WAY0,a1	# A1 = purge base address
	
	# Disable DCACHE if it is enabled

	movhu	(a0),d2				# D2 = old value of control reg

	btst	HAL_CHCTR_DCEN,d2		# check for cache enabled
	beq	1f				# if not, skip disable
	
	mov	d2,d3				# make a copy of CHCTR
	and	~HAL_CHCTR_DCEN,d3		# clear DCEN bit
	movhu	d3,(a0)				# store in reg

2:	movhu	(a0),d3				# get CHCTR
	btst	HAL_CHCTR_DCBUSY,d3		# test DCBUSY bit
	bne	2b				# loop while set

1:
	# The cache is now disabled
	
	and	0x000003f0,d0			# isolate index bits of base addr
	add	d0,a1				# offset a1 to base address

	add	63,d1				# adjust size to whole multiple of
	and	0xFFFFFFC0,d1			# set size.
3:	
	mov	(0x0000,a1),d0			# purge way 0
	mov	(0x1000,a1),d0			# purge way 1
	mov	(0x2000,a1),d0			# purge way 2
	mov	(0x3000,a1),d0			# purge way 3

	add	16,a1				# advance to next set

	sub	64,d1				# decrement count
	bne	3b				# loop while non zero
	
	# Restore original cache state from saved CHCTR in D2

	movhu	d2,(a0)				
		
	movm	(sp),[d2,d3]			# restore work regs
		
	rets	

	
##-----------------------------------------------------------------------------
## end of variant.S
	

	

#ifndef CYGONCE_HAL_MIPS_INC
#define CYGONCE_HAL_MIPS_INC

##=============================================================================
##
##	mips.inc
##
##	MIPS assembler header file
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
## Purpose:	MIPS definitions.
## Description:	This file contains various definitions and macros that are
##              useful for writing assembly code for the MIPS CPU family.
## Usage:
##		#include <cyg/hal/mips.inc>
##		...
##		
##
######DESCRIPTIONEND####
##
##=============================================================================

##-----------------------------------------------------------------------------
## Standard MIPS register names:

#define zero	$0
#define z0	$0
#define v0	$2
#define v1	$3
#define a0	$4
#define a1	$5
#define a2	$6
#define a3	$7
#define t0	$8
#define t1	$9
#define t2	$10
#define t3	$11
#define t4	$12
#define t5	$13
#define t6	$14
#define t7	$15
#define s0	$16
#define s1	$17
#define s2	$18
#define s3	$19
#define s4	$20
#define s5	$21
#define s6	$22
#define s7	$23
#define t8	$24
#define t9	$25
#define k0	$26	/* kernel private register 0 */
#define k1	$27	/* kernel private register 1 */
#define gp	$28	/* global data pointer */
#define sp 	$29	/* stack-pointer */
#define fp	$30	/* frame-pointer */
#define ra	$31	/* return address */
#define pc	$pc	/* pc, used on mips16 */

#define f0	$f0
#define f1	$f1
#define f2	$f2
#define f3	$f3
#define f4	$f4
#define f5	$f5
#define f6	$f6
#define f7	$f7
#define f8	$f8
#define f9	$f9
#define f10	$f10
#define f11	$f11
#define f12	$f12
#define f13	$f13
#define f14	$f14
#define f15	$f15
#define f16	$f16
#define f17	$f17
#define f18	$f18
#define f19	$f19
#define f20	$f20
#define f21	$f21
#define f22	$f22
#define f23	$f23
#define f24	$f24
#define f25	$f25
#define f26	$f26
#define f27	$f27
#define f28	$f28
#define f29	$f29
#define f30	$f30
#define f31	$f31

// Coprocessor 0 registers

#define	index		$0	// TLB entry index register
#define random		$1	// TLB random index register
#define	tlblo0		$2	// TLB even page entry register
#define	tlblo1		$3	// TLB odd page entry register
#define config		$3	// Configuration register (TX39 only)
#define	context		$4	// TLB context register
#define pagemask	$5	// TLB page size mask
#define	wired		$6	// TLB wired boundary
#define cachectrl	$7	// Cache control
#define badvr		$8	// Bad virtual address
#define count		$9	// Timer cycle count register
#define	tlbhi		$10	// TLB virtual address match register
#define compare		$11	// Timer compare register
#define status		$12	// Status register	
#define cause		$13	// Exception cause
#define	epc		$14	// Exception pc value
#define prid		$15	// processor ID
#define config0		$16	// Config register 0
#define lladdr          $17     // LLAdddr
#define xcontext        $20     // XContext register
#define ecc             $26     // Error Checking and Correcting
#define cache_err       $27     // Cache Error and Status
#define taglo           $28     // TagLo
#define taghi           $29     // TagHi
#define error_epc       $30     // Error exception pc value


#------------------------------------------------------------------------------

#define FUNC_START(name)	\
        .type name,@function;	\
        .set push ;             \
        .globl name;		\
        .ent   name;		\
        .set noreorder ;        \
name:

#define FUNC_END(name)		\
name##_end:			\
        .set pop;               \
        .end name		\

#------------------------------------------------------------------------------
#endif // ifndef CYGONCE_HAL_MIPS_INC
# end of mips.inc

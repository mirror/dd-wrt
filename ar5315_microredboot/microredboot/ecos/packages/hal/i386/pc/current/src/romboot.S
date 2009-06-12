##=============================================================================
##
##      romboot.S
##
##      x86 romboot
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
## Author(s):   jskov
## Contributors:jskov
## Date:        1999-01-07
## Purpose:     x86 romboot
## Description: When booting from ROM we need to run a short sequence of real
##		mode code before jumping into 32 bit protected mode. There are
##              problems in GAS with mixing all this code together, and it is
##              easier to just move this bit of code out to a separate file.
##              
##
######DESCRIPTIONEND####
##
##=============================================================================


#include <pkgconf/system.h>
#include <pkgconf/hal.h>

#include <cyg/hal/arch.inc>

#==============================================================================

//    .file   "romboot.S"

#------------------------------------------------------------------------------

	.code16

romboot_start:			
	/* Disable interrupt handling. */
	cli

	# Set DS == CS
	movw	%cs,%ax
	movw	%ax,%ds
	# set ES == 0
	movw	$0,%ax
	movw	%ax,%es

	# Call video bios to init display
  	movw	$0xc000,%bx
	movw  	%bx,%ds
  	movw  	0x0000,%ax
  	movw  	$0x0000,%bx
  	movw  	%bx,%ds
	cmpw  	$0xAA55,%ax
  	jne  	1f
	.byte	0x9a		# lcall
	.word	0x0003		# offset
	.word   0xc000		# segment
1:
	
	movw	%cs,%ax
	movw	%ax,%ds
	# set ES == 0
	movw	$0,%ax
	movw	%ax,%es

	# build a GDT descriptor in memory at location zero
	movw	$(gdtEnd - gdtStart),%ax
	movw	%ax,%es:0
	lea	gdtStart,%ax
	addw	$0xFF00,%ax
	movw	%ax,%es:2
	movw	$0x000F,%ax
	movw	%ax,%es:4
	
	# load GDTR
	lgdt	%es:0

	# Switch to protected mode.
	movl	%cr0,%eax
	orb	$1, %al
	movl	%eax,%cr0

	# and do a jump to flush instruction prefetch
	jmp	romboot_pm

	hlt

	.align	4, 0xCC
gdtStart:
	/* Selector 0x00 == invalid. */
	.word	0x0000
	.word	0x0000
	.byte	0x00
	.byte	0x00
	.byte	0x00
	.byte	0x00

	/* Selector 0x08 == code. */
	.word	0xFFFF
	.word	0x0000
	.byte	0x00
	.byte	0x9B
	.byte	0xCF
	.byte	0x00

	/* Selector 0x10 == data. */
	.word	0xFFFF
	.word	0x0000
	.byte	0x00
	.byte	0x93
	.byte	0xCF
	.byte	0x00

	/* Selector 0x18 == shorter code: faults any code 
         * access 0xF0000000-0xFFFFFFFF.
         */
	.word	0xFFFF
	.word	0x0000
	.byte	0x00
	.byte	0x9B
	.byte	0xC7
	.byte	0x00

	/* Selector 0x20 == data; faults any access 0xF0000000-0xFFFFFFFF. */
	.word	0xFFFF
	.word	0x0000
	.byte	0x00
	.byte	0x93
	.byte	0xC7
	.byte	0x00

	.align	4, 0xCC
gdtEnd:

	# We arrive here in protected mode
romboot_pm:	

	# Load data selectors
	movw	$0x10, %ax
	movw	%ax, %ds
	movw	%ax, %es
	movw	%ax, %fs
	movw	%ax, %gs
	
	# jump to start of ROM, where the PM code starts	
#	ljmp	$8,$0xF0000
	.byte	0x66,0xea	# opsize + ljmp opcode
	.long	0x000F0000	# destination address
	.word	0x0008		# code selector

	.code16

	.org	0xF0
romboot_reset:		
	jmp	romboot_start

	.org	0x100
	
#------------------------------------------------------------------------------
# end of romboot.S

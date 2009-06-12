##=============================================================================
##
##      vectors.S
##
##      x86 exception vectors
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
## Author(s):   bartv
## Contributors:bartv
## Date:        2001-03-16
## Purpose:     Synthetic target startup
## Description: When running on real hardware vectors.S contains
##	        initialization code and usually the low-level interrupt and
##              exception support. On the synthetic target the latter is
##              handled by C code. The main initialization that is needed is
##              to jump into the C startup.        
##
######DESCRIPTIONEND####
##
##=============================================================================

# According the SVR4/i386 ABI, most registers are undefined. However
# there is some interesting information on the stack:   
#     %esp              argc
#     %esp+4            argv[0]
#     ...               argv[1 onwards]
#     %esp + (argc*4)   NULL
#     ...               environ[0]
#     ...               ...
#     ...               NULL                                                
#
# There are some other things that could be done, for example aligning
# the stack to a 16-byte boundary for SSE, but it is not clear which of
# those things are actually useful. The glibc source file
# sysdeps/i386/elf/start.S contains some relevant information.

#==============================================================================

        .file   "vectors.S"

	.extern	_linux_entry
        
        .data
        .global cyg_hal_sys_argc
cyg_hal_sys_argc:       
        .long   0
        .global cyg_hal_sys_argv
cyg_hal_sys_argv:
        .long   0
        .global cyg_hal_sys_environ
cyg_hal_sys_environ:
        .long 0
        
	.text
	.globl	_start
_start:
        popl    %eax
        movl    %eax, cyg_hal_sys_argc
        movl    %esp, %ebx
        movl    %ebx, cyg_hal_sys_argv
        inc     %eax
        addl    %eax, %eax
        addl    %eax, %eax
        addl    %eax, %ebx
        movl    %ebx, cyg_hal_sys_environ
        
	jmp	_linux_entry

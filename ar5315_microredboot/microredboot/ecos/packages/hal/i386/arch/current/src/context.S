##=============================================================================
##
##      context.S
##
##      i386 context switch code
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
## Contributors:jskov, pjo, nickg
## Date:        1999-01-20
## Purpose:     i386 context switch code
## Description: This file contains implementations of the thread context 
##              switch routines. It also contains the longjmp() and setjmp()
##              routines.
##              Based on PowerPC context.S, using data from SYSV ABI4, i386 
##              supplement (page 37-38)
##         http://www.sco.com/products/layered/develop/devspecs/abi386-4.pdf
##
######DESCRIPTIONEND####
##
##=============================================================================

#include <pkgconf/hal.h>

#include <cyg/hal/i386.inc>

#include <cyg/hal/arch.inc>
	
#------------------------------------------------------------------------------
# function declaration macro

#define FUNC_START(name)                        \
        .globl name;                            \
name:   

#------------------------------------------------------------------------------
# hal_thread_switch_context
# Switch thread contexts
# :     0(%esp) :     return address
# :     4(%esp) :     address of sp of next thread to execute
# :     8(%esp) :     address of sp save location of current thread
#
# %eax, %ecx, and %edx are ours to abuse.
        
FUNC_START(hal_thread_switch_context)

	# Pop the return address from the stack, but leave the
	# arguments there so that the caller can remove them
	# itself when we return.
	
	popl	%ecx		# pop return eip
	movl	0(%esp),%eax	# get next context ptr
	movl	4(%esp),%edx	# get this context ptr

	# Save context in the same format as an
	# exception
		
	pushfl			# save eflags
	pushl	%cs		# save cs
	pushl	%ecx		# save eip 
	pushl	$-1		# push fake vector
	pushal			# push general registers
	hal_fpu_push_ctx	# push FPU state
		
        # Store the context ptr
        movl    %esp,(%edx)

hal_thread_switch_context_load:
	
	# The pointer to the next context is in EAX
	
	movl	(%eax),%esp	# Point ESP at new state

	# Merge the IF bit in the saved EFLAGS with the rest of the 
	# bits currently in EFLAGS.

	movl	i386reg_eflags(%esp),%ebx # EBX = saved EFLAGS
	andl	$0x0200,%ebx	# isolate IF bit
	pushfl			# push current flags
	popl	%ecx		# pop into ECX
	btrl	$9,%ecx		# clear IF flag in current EFLAGS
	orl	%ebx,%ecx	# Or in saved IF bit
	movl	%ecx,i386reg_eflags(%esp) # Restore to saved state for use by iret

	# Now we can load the state and enter next thread

	hal_fpu_pop_ctx		# Pop FPU state
	popal			# unstack general registers
	add	$4,%esp		# skip vector number
        
        iret			# And return
        
#------------------------------------------------------------------------------
# hal_thread_load_context
# Load thread context
# : 4(%esp) = address of sp of thread to execute
#
# %eax, %ecx, and %edx are ours to abuse.
        
FUNC_START(hal_thread_load_context)

        movl    4(%esp),%eax	# get new context ptr

	# Jump into hal_thread_switch_context at the right
	# point to load this context.
	
	jmp	hal_thread_switch_context_load

#------------------------------------------------------------------------------
# HAL longjmp, setjmp implementations
# hal_setjmp saves only to callee save registers ebp, ebx, esi, edi and
# and esp+pc into buffer supplied in 4(esp)
# Note: These definitions are repeated in hal_arch.h. If changes are required
# remember to update both sets.

#define CYGARC_JMP_BUF_SP        0
#define CYGARC_JMP_BUF_EBP       1
#define CYGARC_JMP_BUF_EBX       2
#define CYGARC_JMP_BUF_ESI       3
#define CYGARC_JMP_BUF_EDI       4
#define CYGARC_JMP_BUF_PC        5

#define CYGARC_JMP_BUF_SIZE      6

FUNC_START(hal_setjmp)
        # Get jmpbuf pointer
        movl    4(%esp),%eax

        # Save regular registers
        movl    %ebp,CYGARC_JMP_BUF_EBP*4(%eax)
        movl    %ebx,CYGARC_JMP_BUF_EBX*4(%eax)
        movl    %esi,CYGARC_JMP_BUF_ESI*4(%eax)
        movl    %edi,CYGARC_JMP_BUF_EDI*4(%eax)

        # Stack and PC
        movl    %esp,CYGARC_JMP_BUF_SP*4(%eax)
        movl    0(%esp),%edx
        movl    %edx,CYGARC_JMP_BUF_PC*4(%eax)

        # Return 0
        xor     %eax,%eax
        ret

        
# hal_longjmp loads state from 4(esp) and returns to PC stored in state

FUNC_START(hal_longjmp)
        # Get return value
        movl    8(%esp),%eax

        # Get jmpbuf pointer
        movl    4(%esp),%ecx
        
        # Restore regular registers
        movl    CYGARC_JMP_BUF_EBP*4(%ecx),%ebp
        movl    CYGARC_JMP_BUF_EBX*4(%ecx),%ebx
        movl    CYGARC_JMP_BUF_ESI*4(%ecx),%esi
        movl    CYGARC_JMP_BUF_EDI*4(%ecx),%edi
        
        # Restore stack pointer
        movl    CYGARC_JMP_BUF_SP*4(%ecx),%esp

        # Put return address on stack        
        movl    CYGARC_JMP_BUF_PC*4(%ecx),%edx
        movl    %edx,0(%esp)

        ret

#-----------------------------------------------------------------------------
# End of context.S

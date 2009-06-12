##=============================================================================
##
##	i386.inc
##
##	i386 assembler header file
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
## Author(s): 	jskov
## Contributors:jskov
## Date:	1999-01-20
## Purpose:	i386 definitions.
## Description:	This file contains various definitions and macros that are
##              useful for writing assembly code for the i386
## Usage:
##		#include <cyg/hal/i386.inc>
##		...
##		
##
######DESCRIPTIONEND####
##
##=============================================================================

#------------------------------------------------------------------------------
# Exception, interrupt and thread context save area layout
# The layout of this structure is also defined in "hal_arch.h", for C
# code. Do not change this without changing that (or vice versa).

	# See SYSV ABI4, i386 supplement (page 37-38)
        # http://www.mit.edu/afs/sipb/contrib/doc/specs/software/sysv-abi/i386-psABI-4.pdf

	# Callee save registers (eax, ecx, and edx are scratch registers)

#ifdef CYGHWR_HAL_I386_FPU
#ifdef CYGHWR_HAL_I386_FPU_SWITCH_LAZY
	.equ	i386reg_fpucontext,	0
	.equ	i386reg_edi,		i386reg_fpucontext+4
#else
	.equ	i386reg_fpstate_valid,	0
	.equ	i386reg_fpstate,	i386reg_fpstate_valid+4
#ifdef CYGHWR_HAL_I386_PENTIUM_SSE
	.equ	i386reg_simd_xmm0,	i386reg_fpstate+108
	.equ	i386reg_simd_xmm1,	i386reg_simd_xmm0+16
	.equ	i386reg_simd_xmm2,	i386reg_simd_xmm1+16
	.equ	i386reg_simd_xmm3,	i386reg_simd_xmm2+16
	.equ	i386reg_simd_xmm4,	i386reg_simd_xmm3+16
	.equ	i386reg_simd_xmm5,	i386reg_simd_xmm4+16
	.equ	i386reg_simd_xmm6,	i386reg_simd_xmm5+16
	.equ	i386reg_simd_xmm7,	i386reg_simd_xmm6+16
	.equ	i386reg_simd_mxcsr,	i386reg_simd_xmm7+16
	.equ	i386reg_fpstate_size,	i386reg_simd_mxcsr+4
#else
	.equ	i386reg_fpstate_size,	i386reg_fpstate+108
#endif
	.equ	i386reg_edi,		i386reg_fpstate_size
#endif
#else
	.equ	i386reg_edi,		0
# endif
	.equ	i386reg_esi,		i386reg_edi+4
	.equ	i386reg_ebp,		i386reg_esi+4
	.equ	i386reg_esp,		i386reg_ebp+4
	.equ	i386reg_ebx,		i386reg_esp+4
	.equ	i386reg_edx,		i386reg_ebx+4
	.equ	i386reg_ecx,		i386reg_edx+4
	.equ	i386reg_eax,		i386reg_ecx+4
	.equ	i386reg_vector,		i386reg_eax+4
	.equ	i386reg_eip,		i386reg_vector+4
	.equ	i386reg_cs,		i386reg_eip+4
	.equ	i386reg_eflags,		i386reg_cs+4
	.equ	i386reg_context_size, 	i386reg_eflags+4


#if defined(CYGHWR_HAL_I386_FPU) && defined(CYGHWR_HAL_I386_FPU_SWITCH_LAZY)
	.equ	i386reg_fpucontext_valid,	0
	.equ	i386reg_fpucontext_state,	i386reg_fpucontext_valid+4
#ifdef CYGHWR_HAL_I386_PENTIUM_SSE
	.equ	i386reg_simd_xmm0,		i386reg_fpucontext_state+108
	.equ	i386reg_simd_xmm1,		i386reg_simd_xmm0+16
	.equ	i386reg_simd_xmm2,		i386reg_simd_xmm1+16
	.equ	i386reg_simd_xmm3,		i386reg_simd_xmm2+16
	.equ	i386reg_simd_xmm4,		i386reg_simd_xmm3+16
	.equ	i386reg_simd_xmm5,		i386reg_simd_xmm4+16
	.equ	i386reg_simd_xmm6,		i386reg_simd_xmm5+16
	.equ	i386reg_simd_xmm7,		i386reg_simd_xmm6+16
	.equ	i386reg_simd_mxcsr,		i386reg_simd_xmm7+16
	.equ	i386reg_fpucontext_size,	i386reg_simd_mxcsr+4
#else
	.equ	i386reg_fpucontext_size,	i386reg_fpucontext_state+108
#endif
#endif

#------------------------------------------------------------------------------
# end of i386.inc

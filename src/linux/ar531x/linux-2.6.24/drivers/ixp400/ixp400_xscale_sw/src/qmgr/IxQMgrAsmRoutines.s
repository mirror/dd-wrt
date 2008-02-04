/**
 * @file    IxQMgrAsmRoutines.s
 *
 * @author Intel Corporation
 * @date    20-Jul-2002
 *    
 * @brief   This file contains the XSCALE implementation of the fastest way to
 * to search a bit in a bitfield.
 *
 * See also: XSCALE processor instruction set documentation (clz)
 *
 * @par 
 * IXP400 SW Release Crypto version 2.4
 * 
 * -- Copyright Notice --
 * 
 * @par
 * Copyright (c) 2001-2007, Intel Corporation.
 * All rights reserved.
 * 
 * @par
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Intel Corporation nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * 
 * @par
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * 
 * @par
 * -- End of Copyright Notice --
*/


#ifndef __linux
#ifdef __XSCALE__

/*
 * System defines and include files
 */
#define _ASMLANGUAGE
#include <arch/arm/arm.h>

    .balign 4

.global ixQMgrCountLeadingZeros
/* input: r0 must contain the 32 bits bitfield where bits are to be searched.
   output: r0 contains the number of leading zero bits (or 32 if the word is nul)

   C prototype:
   unsigned int ixQMgrCountLeadingZeros(UINT32 r0);
*/
_ARM_FUNCTION(ixQMgrCountLeadingZeros)
	clz		r0, r0
	mov		pc, lr

#endif

#endif /* !__linux */


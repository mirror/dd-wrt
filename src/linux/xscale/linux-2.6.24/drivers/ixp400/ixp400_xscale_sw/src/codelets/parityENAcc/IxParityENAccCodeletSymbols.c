/**
 * @file IxParityENAccCodeletSymbols.c
 *
 * @author Intel Corporation
 *
 * @date 9 December 2004
 *
 * @brief This file declares exported symbols for linux kernel module 
 *	  builds.
 *
 * @par
 * IXP400 SW Release version 2.4
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

#ifdef __linux
#if defined (__ixp46X) || defined(__ixp43X)

/* include files */

#include <linux/module.h>
#include <linux/init.h>

#include "IxParityENAccCodelet.h"
 
BOOL multiBit = FALSE;
BOOL injectNow = FALSE;

EXPORT_SYMBOL (ixParityENAccCodeletMain);

MODULE_PARM(multiBit, "i");
MODULE_PARM_DESC(multiBit, "ECC error type\n"
		"\tFALSE - single bit ECC\n"
		"\tTRUE - multi bit ECC\n");

MODULE_PARM(injectNow, "i");
MODULE_PARM_DESC(injectNow, "When to inject ECC error\n"
		"\tFALSE - inject ECC error later\n"
		"\tTRUE - inject ECC error now\n");

static int __init parityENAccCodeletInitModule (void)
{
	printk ("Loading parityENAcc Codelet\n");
	return (ixParityENAccCodeletMain (multiBit, injectNow));
}

static void __exit parityENAccCodeletExitModule (void)
{
	ixParityENAccCodeletQuit ();
	printk ("Unload parityENAcc Codelet\n");
}

module_init (parityENAccCodeletInitModule);
module_exit (parityENAccCodeletExitModule);

#endif  /* __ixp46X || __ixp43X */
#endif	/* end of #ifdef __linux */



/**
 * @file IxTimeSyncAccCodeletSymbols.c
 *
 * @author Intel Corporation
 *
 * @date 15 December 2004
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
#if defined (__ixp46X)

/* include files */

#include <linux/module.h>
#include <linux/init.h>

#include "IxTimeSyncAccCodelet.h"

UINT32 config;

EXPORT_SYMBOL (ixTimeSyncAccCodeletMain);
EXPORT_SYMBOL (ixTimeSyncAccCodeletUninit);

MODULE_PARM(config, "i");
MODULE_PARM_DESC(config, "Choice of configuration\n"
		"\tconfiguration 0: NPE A - Slave,  NPE B - Slave,  NPE C - Master (default)\n"
		"\tconfiguration 1: NPE A - Slave,  NPE B - Master, NPE C - Slave\n"
		"\tconfiguration 2: NPE A - Master, NPE B - Slave,  NPE C - Slave\n");

PRIVATE int __init ixTimeSyncAccCodeletInitModule (void)
{
	if (IX_TIMESYNCACC_CODELET_MAX_CONFIGURATIONS <= config)
	{
		printk("Usage :\n");
		printk("# insmod ixp400_codelets_timeSyncAcc.o config=<x>\n\n");
		printk("\twhere x =\n");
		printk("\t  0 -> NPE A - Slave,  NPE B - Slave,  NPE C - Master (default)\n");  
		printk("\t  1 -> NPE A - Slave,  NPE B - Master, NPE C - Slave\n");  
		printk("\t  2 -> NPE A - Master, NPE B - Slave,  NPE C - Slave\n");  
		return -EINVAL;
	}

	printk ("Loading timeSyncAcc Codelet\n");
	return (ixTimeSyncAccCodeletMain(config));
}

PRIVATE void __exit ixTimeSyncAccCodeletExitModule (void)
{
	ixTimeSyncAccCodeletUninit ();
	printk ("TimeSyncAcc Codelet was unloaded\n");
}

module_init (ixTimeSyncAccCodeletInitModule);
module_exit (ixTimeSyncAccCodeletExitModule);

#endif  /* end of #ifdef __ixp46X */
#endif	/* end of #ifdef __linux */



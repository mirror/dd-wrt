/**
 * @file IxPerfProfAccCodeletSymbols.c
 *
 * @author Intel Corporation
 * @date 19-June-2003
 *
 * @brief This file declares exported symbols for linux kernel module builds.
 *
 * 
 * @par
 * IXP400 SW Release Crypto version 2.3
 * 
 * -- Copyright Notice --
 * 
 * @par
 * Copyright (c) 2001-2005, Intel Corporation.
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

#include <linux/autoconf.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/proc_fs.h>
#include <asm/semaphore.h>

#include "IxOsalTypes.h"
#include "IxPerfProfAccCodelet.h"
#include "IxPerfProfAcc.h"

EXPORT_SYMBOL(ixPerfProfAccCodeletMain);

int mode;
UINT32 param1;
UINT32 param2;
UINT32 param3;
UINT32 param4;
UINT32 param5;
UINT32 param6;
UINT32 param7;
UINT32 param8;
UINT32 param9;

MODULE_PARM(mode, "i");
MODULE_PARM(param1, "i");
MODULE_PARM(param2, "i");
MODULE_PARM(param3, "i");
MODULE_PARM(param4, "i");
MODULE_PARM(param5, "i");
MODULE_PARM(param6, "i");
MODULE_PARM(param7, "i");
MODULE_PARM(param8, "i");
MODULE_PARM(param9, "i");

static int __init PerfProfAccInitModule(void)
{
        printk ("Loading perfProfAcc Codelet...\n");
        create_proc_read_entry("perfProfTimeSamp", 0, NULL, 
                               ixPerfProfAccXscalePmuTimeSampCreateProcFile, NULL);
        printk ("Registered perfProfTimeSamp\n");
        create_proc_read_entry("perfProfEventSamp", 0, NULL,
                               ixPerfProfAccXscalePmuEventSampCreateProcFile, NULL);
        printk ("Registered perfProfEventSamp\n");
        ixPerfProfAccCodeletMain(mode, param1, param2, param3, param4,
                               param5, param6, param7, param8, param9);
	return 0;
}
    
static void __exit PerfProfAccExitModule(void)
{
	printk("Unloading PerfProfAcc Codelet\n");
        remove_proc_entry("perfProfTimeSamp", NULL);
        remove_proc_entry("perfProfEventSamp", NULL);
}

module_init(PerfProfAccInitModule);
module_exit(PerfProfAccExitModule);


#endif

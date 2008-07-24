/**
 * @file IxEthAal5Symbols.c
 *
 * @author Intel Corporation
 * @date 04-Oct-2002
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

#include <linux/module.h>
#include <linux/init.h>

#include "IxEthAal5App.h"
#include "IxEthAal5App_p.h"

int modeType = 0;

MODULE_PARM(modeType, "i");

static int __init ethAal5_init_module(void)
{
    printk ("Load EthAal5App Codelet \n");
    
    if (modeType < IX_EAA_UTOPIA_MODE || modeType > IX_EAA_ADSL_MODE)
    {
	printk("\nUsage :");
	printk("\n # insmod ixp400_codelets_ethAal5App.o modeType=<x>");
	printk("\n");	    
	printk("\n Where x : 1 = Utopia");
	printk("\n           2 = ADSL");
	printk("\n");

        return -EINVAL;
    }

    ixEthAal5AppCodeletMain(modeType);

    return 0;
}

static void __init ethAal5_cleanup_module(void)
{
    printk("Unload EthAal5App Codelet...");
    ixEthAal5AppShowTaskDisable();
    printk("Successful\n");
}

module_init(ethAal5_init_module);
module_exit(ethAal5_cleanup_module);

#endif /* __linux */


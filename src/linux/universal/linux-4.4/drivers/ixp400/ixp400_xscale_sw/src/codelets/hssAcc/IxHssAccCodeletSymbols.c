/**
 * @file IxHssAccCodeletSymbols.c
 *
 * @author Intel Corporation
 * @date 14-Oct-2002
 *
 * @brief This file declares exported symbols for linux kernel module builds.
 *
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

#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/sched.h>

#include "IxHssAcc.h"
#include "IxHssAccCodelet.h"

EXPORT_SYMBOL(ixHssAccCodeletMain);

int operationType = 0;
int portMode = 0 ;
int verifyMode = 0 ;

MODULE_PARM(operationType, "i");
MODULE_PARM(portMode, "i");
MODULE_PARM(verifyMode, "i");

MODULE_PARM_DESC(operationType, "HssAcc Codelet operation options (1-3)\n"
		 "\t1 - Packetised Service Only.\n"
		 "\t2 - Channelised Service Only.\n"
		 "\t3 - Packetised Service and Channelised Services.\n");

MODULE_PARM_DESC(portMode, "HssAcc Codelet port mode options (1-3)\n"
		 "\t1 - HSS Port 0 Only.\n"
		 "\t2 - HSS Port 1 Only.\n"
		 "\t3 - HSS Port 0 and 1.\n");

MODULE_PARM_DESC(verifyMode, "HssAcc Codelet verify mode options (1-2)\n"
		 "\t1 - Received traffics are verified.\n"
		 "\t2 - Received traffics are not verified.\n");


static int operation_func(void *unused)
{
    ixHssAccCodeletMain(operationType, portMode, verifyMode);
    return 0;
}

static int __init hss_init_module(void)
{
    printk ("Load Codelet: HssAcc.\n");
    
    /* Checking the input arguements' boudary */ 
    if ((operationType < IX_HSSACC_CODELET_PKT_SERV_ONLY) || 
        (operationType > IX_HSSACC_CODELET_PKT_CHAN_SERV) ||
        (portMode < IX_HSSACC_CODELET_HSS_PORT_0_ONLY) ||
        (portMode > IX_HSSACC_CODELET_DUAL_PORTS) ||   
        (verifyMode < IX_HSSACC_CODELET_VERIFY_ON) ||
        (verifyMode > IX_HSSACC_CODELET_VERIFY_OFF))
    {
	printk("\nUsage :");
	printk("\n >insmod codelets_hssAcc.o operationType=<a> portMode=<b> \
                    verifyMode=<c> \n");
	printk("\n Where a : %d = Packetised Service Only.", IX_HSSACC_CODELET_PKT_SERV_ONLY);
	printk("\n           %d = Channelised Service Only.", IX_HSSACC_CODELET_CHAN_SERV_ONLY);
	printk("\n           %d = Packetised and Channelised Services.", IX_HSSACC_CODELET_PKT_CHAN_SERV);
	printk("\n Where b : %d = HSS Port 0 Only.", IX_HSSACC_CODELET_HSS_PORT_0_ONLY);
	printk("\n           %d = HSS Port 1 Only.", IX_HSSACC_CODELET_HSS_PORT_1_ONLY);
	printk("\n           %d = HSS Port 0 and 1.", IX_HSSACC_CODELET_DUAL_PORTS);
	printk("\n Where c : %d = Received traffics are verified.", IX_HSSACC_CODELET_VERIFY_ON);
	printk("\n           %d = Received traffics are not verified. \n", IX_HSSACC_CODELET_VERIFY_OFF);
        return -EINVAL;
    }

    kernel_thread(operation_func, NULL, CLONE_SIGHAND);
    return 0;
}

static void __init hss_cleanup_module(void)
{
    printk("Unload Codelet: HSS Acc.\n");
}

module_init(hss_init_module);
module_exit(hss_cleanup_module);

#endif /* __linux */








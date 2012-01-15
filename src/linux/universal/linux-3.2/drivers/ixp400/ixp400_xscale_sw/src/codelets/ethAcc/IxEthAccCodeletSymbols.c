/**
 * @file IxEthAccCodeletSymbols.c
 *
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
#include <linux/semaphore.h>

#include "IxOsal.h"
#include "IxEthAcc.h"
#include "IxEthAcc_p.h"
#include "IxEthAccCodelet.h"
#include "IxEthAccCodelet_p.h"

EXPORT_SYMBOL(ixEthAccCodeletMain);

int operationType = 0;
MODULE_PARM(operationType, "i");
MODULE_PARM_DESC(operationType, "EthAcc Codelet operation options (1-9)\n"
		 "\t1 - Rx Sink\n"
		 "\t2 - Sw Loopback\n"
		 "\t3 - Tx Gen/Rx Sink Loopback\n"
                 "\t4 - PHY Loopback\n"
		 "\t5 - Bridge\n"
		 "\t6 - Bridge + QoS\n"
                 "\t7 - Bridge + Firewall\n"
                 "\t8 - EthDB Learning\n"
                 "\t9 - Birdge + WiFi header conversion\n");
int inPort = 0;
MODULE_PARM(inPort, "i");
MODULE_PARM_DESC(inPort, "EthAcc Codelet input port selection for tests\n");
int outPort = 1;
MODULE_PARM(outPort, "i");
MODULE_PARM_DESC(outPort, "EthAcc Codelet output port selection for tests\n");
int disableStats = 0;
MODULE_PARM(disableStats, "i");
MODULE_PARM_DESC(disableStats, "EthAcc Codelet disable statistic polling task \n"
					"\t0 - Enable\n"
					"\t1 - Disable\n");

DECLARE_MUTEX_LOCKED(__codlet_getchar);
DECLARE_MUTEX_LOCKED(__codlet_end);

IxOsalThread tid;

void ixEthAccCodelet_wait(void)
{
    /* this function waits until themutex is unlocked by
     * the cleanup function
     */
    down(&__codlet_getchar);
}

static void operation_func(void *unused)
{
    ixEthAccCodeletMain(operationType,inPort,outPort,disableStats);
    up(&__codlet_end);
}

static int __init ethAccCodelet_init_module(void)
{
    printk ("Load Codelet: EthAcc.\n");

    if ((operationType < IX_ETHACC_CODELET_RX_SINK) || 
        (operationType > IX_ETHACC_CODELET_BRIDGE_WIFI))
    {
	printk("\nUsage :");
	printk("\n # insmod codelets_ethAcc.o operationType=<x> inPort=<y> outPort=<z>");
	printk("\n");	    
	printk("\n Where x : %d = Rx Sink", IX_ETHACC_CODELET_RX_SINK);
	printk("\n           %d = Sw Loopback", IX_ETHACC_CODELET_SW_LOOPBACK);
	printk("\n           %d = Tx Gen/Rx Sink Loopback", IX_ETHACC_CODELET_TXGEN_RXSINK);
	printk("\n           %d = PHY Loopback", IX_ETHACC_CODELET_PHY_LOOPBACK);
	printk("\n           %d = Bridge", IX_ETHACC_CODELET_BRIDGE);
        printk("\n           %d = Bridge + QoS", IX_ETHACC_CODELET_BRIDGE_QOS);
        printk("\n           %d = Bridge + Firewall", IX_ETHACC_CODELET_BRIDGE_FIREWALL);
        printk("\n           %d = Eth DB Learning", IX_ETHACC_CODELET_ETH_LEARNING);
        printk("\n           %d = Bridge + WiFi header conversion", IX_ETHACC_CODELET_BRIDGE_WIFI);
        printk("\n");
        return -EINVAL;
    }

    ixOsalThreadCreate(&tid, NULL, operation_func, NULL);
    ixOsalThreadStart(&tid);

    return 0;
}

static void __exit ethAccCodelet_cleanup_module(void)
{
    ixEthAccDataPlaneShow();
    up(&__codlet_getchar);
    down(&__codlet_end);
    ixEthAccCodeletUninit();
    /* 
     * We do not need to call ixOsalThreadKill here because the thread will
     * exit on ixEthAccCodeletUninit() when the mutex unlocked.
     */
    printk("Unload Codelet: Ethernet Access Codelet.\n");
}

module_init(ethAccCodelet_init_module);
module_exit(ethAccCodelet_cleanup_module);

#endif

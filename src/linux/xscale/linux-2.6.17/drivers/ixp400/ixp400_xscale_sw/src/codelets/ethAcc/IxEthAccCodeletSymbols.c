/**
 * @file IxEthAccCodeletSymbols.c
 *
 * @date 14-Oct-2002
 *
 * @brief This file declares exported symbols for linux kernel module builds.
 *
 * 
 * @par
 * IXP400 SW Release version  2.1
 * 
 * -- Intel Copyright Notice --
 * 
 * @par
 * Copyright (c) 2002-2005 Intel Corporation All Rights Reserved.
 * 
 * @par
 * The source code contained or described herein and all documents
 * related to the source code ("Material") are owned by Intel Corporation
 * or its suppliers or licensors.  Title to the Material remains with
 * Intel Corporation or its suppliers and licensors.
 * 
 * @par
 * The Material is protected by worldwide copyright and trade secret laws
 * and treaty provisions. No part of the Material may be used, copied,
 * reproduced, modified, published, uploaded, posted, transmitted,
 * distributed, or disclosed in any way except in accordance with the
 * applicable license agreement .
 * 
 * @par
 * No license under any patent, copyright, trade secret or other
 * intellectual property right is granted to or conferred upon you by
 * disclosure or delivery of the Materials, either expressly, by
 * implication, inducement, estoppel, except in accordance with the
 * applicable license agreement.
 * 
 * @par
 * Unless otherwise agreed by Intel in writing, you may not remove or
 * alter this notice or any other notice embedded in Materials by Intel
 * or Intel's suppliers or licensors in any way.
 * 
 * @par
 * For further details, please see the file README.TXT distributed with
 * this software.
 * 
 * @par
 * -- End Intel Copyright Notice --
 */

#ifdef __linux

#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <asm/semaphore.h>

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

DECLARE_MUTEX_LOCKED(__codlet_getchar);
DECLARE_MUTEX_LOCKED(__codlet_end);

void ixEthAccCodelet_wait(void)
{
    /* this function waits until themutex is unlocked by
     * the cleanup function
     */
    down(&__codlet_getchar);
}

static int operation_func(void *unused)
{
    ixEthAccCodeletMain(operationType,inPort,outPort);
    up(&__codlet_end);
    return 0;
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
        return 1;
    }

    kernel_thread(operation_func, NULL, CLONE_SIGHAND);
    return 0;
}

static void __init ethAccCodelet_cleanup_module(void)
{
    ixEthAccDataPlaneShow();
    up(&__codlet_getchar);
    down(&__codlet_end);
    ixEthAccCodeletUninit();
    printk("Unload Codelet: Ethernet Access Codelet.\n");
}

module_init(ethAccCodelet_init_module);
module_exit(ethAccCodelet_cleanup_module);

#endif

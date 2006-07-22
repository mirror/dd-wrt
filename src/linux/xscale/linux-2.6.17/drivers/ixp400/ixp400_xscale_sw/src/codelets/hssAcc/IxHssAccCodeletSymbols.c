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
        return 1;
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








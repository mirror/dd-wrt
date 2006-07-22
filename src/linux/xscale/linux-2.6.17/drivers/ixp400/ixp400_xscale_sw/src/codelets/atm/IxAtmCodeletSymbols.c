/**
 * @file IxAtmCodeletSymbols.c
 *
 * @author Intel Corporation
 * @date 04-Oct-2002
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

#include "IxAtmCodelet.h"
#include "IxAtmCodelet_p.h"

int modeType = 0;
int aalType = 0;

MODULE_PARM(modeType, "i");
MODULE_PARM(aalType, "i");

static int __init atmdAcc_init_module(void)
{
    printk ("Load Codelet: AtmdAcc Sample.\n");
    if ((modeType < 0 || modeType > 4) || 
	(aalType <= ixAtmCodeletAalTypeInvalid || aalType >= ixAtmCodeletAalTypeMax))
    {
	printk("\nUsage :");
	printk("\n # insmod ixp400_codelets_atm.o modeType=<x> aalType=<y>");
	printk("\n");	    
	printk("\n Where x : 0 = Utopia Loopback Mode");
	printk("\n           1 = Software Loopback Mode");
	printk("\n           2 = Remote Loopback Mode");
	printk("\n           3 = F4 & F5 cells OAM Ping in UTOPIA Loopback mode");
	printk("\n           4 = F4 & F5 cells OAM Ping in Software Loopback mode");
	printk("\n");
	printk("\n Where y : 1 = AAL5");
	printk("\n           2 = AAL0_48"); 
	printk("\n           3 = AAL0_52"); 
	printk("\n");

        return 1;
    }

    ixAtmCodeletMain(modeType,aalType);

    return 0;
}

static void __init atmdAcc_cleanup_module(void)
{
    printk("Unload Codelet: AtmdAcc Sample.\n");
}


module_init(atmdAcc_init_module);
module_exit(atmdAcc_cleanup_module);

#endif /* __linux */











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

        return 1;
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


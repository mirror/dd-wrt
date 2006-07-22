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
#ifdef __ixp46X

/* include files */

#include <linux/module.h>
#include <linux/init.h>

#include "IxTimeSyncAccCodelet.h"

UINT32 config;

EXPORT_SYMBOL (ixTimeSyncAccCodeletMain);
EXPORT_SYMBOL (ixTimeSyncAccCodeletQuit);

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
		return (-1);
	}
	printk ("Loading timeSyncAcc Codelet\n");
	return (ixTimeSyncAccCodeletMain(config));
}

PRIVATE void __exit ixTimeSyncAccCodeletExitModule (void)
{
	ixTimeSyncAccCodeletQuit ();
	printk ("TimeSyncAcc Codelet was unloaded\n");
}

module_init (ixTimeSyncAccCodeletInitModule);
module_exit (ixTimeSyncAccCodeletExitModule);

#endif  /* end of #ifdef __ixp46X */
#endif	/* end of #ifdef __linux */



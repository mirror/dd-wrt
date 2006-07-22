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

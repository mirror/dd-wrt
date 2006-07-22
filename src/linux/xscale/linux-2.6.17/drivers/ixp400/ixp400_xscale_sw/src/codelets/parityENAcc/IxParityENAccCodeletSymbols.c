/**
 * @file IxParityENAccCodeletSymbols.c
 *
 * @author Intel Corporation
 *
 * @date 9 December 2004
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

#include "IxParityENAccCodelet.h"
 
BOOL multiBit = FALSE;
BOOL injectNow = FALSE;

EXPORT_SYMBOL (ixParityENAccCodeletMain);

MODULE_PARM(multiBit, "i");
MODULE_PARM_DESC(multiBit, "ECC error type\n"
		"\tFALSE - single bit ECC\n"
		"\tTRUE - multi bit ECC\n");

MODULE_PARM(injectNow, "i");
MODULE_PARM_DESC(injectNow, "When to inject ECC error\n"
		"\tFALSE - inject ECC error later\n"
		"\tTRUE - inject ECC error now\n");

static int __init parityENAccCodeletInitModule (void)
{
	printk ("Loading parityENAcc Codelet\n");
	return (ixParityENAccCodeletMain (multiBit, injectNow));
}

static void __exit parityENAccCodeletExitModule (void)
{
	ixParityENAccCodeletQuit ();
	printk ("Unload parityENAcc Codelet\n");
}

module_init (parityENAccCodeletInitModule);
module_exit (parityENAccCodeletExitModule);

#endif  /* __ixp46X */
#endif	/* end of #ifdef __linux */



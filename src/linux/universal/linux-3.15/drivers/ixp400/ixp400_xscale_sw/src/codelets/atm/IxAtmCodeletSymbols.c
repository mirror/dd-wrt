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

#include "IxAtmCodelet.h"
#include "IxAtmCodelet_p.h"

int modeType = 0;
int aalType = 0;

MODULE_PARM(modeType, "i");
MODULE_PARM(aalType, "i");

static int __init atmdAcc_init_module(void)
{
    printk ("Load Codelet: AtmdAcc Sample.\n");
    if ((modeType < 0 || modeType > 7) || 
	(aalType <= ixAtmCodeletAalTypeInvalid || aalType >= ixAtmCodeletAalTypeMax))
    {
	printk("\nUsage :");
	printk("\n # insmod ixp400_codelets_atm.o modeType=<x> aalType=<y>");
	printk("\n");	    
	printf("\n Where x : 0 = Utopia Loopback Mode 32 UBR");
	printf("\n           1 = Utopia Loopback Mode 8VBR, 8CBR, 16UBR");
	printf("\n           2 = Software Loopback Mode 32 UBR");
	printf("\n           3 = Software Loopback Mode 8VBR, 8CBR, 16UBR");
	printf("\n           4 = Remote Loopback Mode 32 UBR");
	printf("\n           5 = Remote Loopback 8VBR, 8CBR, 16UBR");
	printf("\n           6 = F4 & F5 cells OAM Ping in UTOPIA Loopback mode");
	printf("\n           7 = F4 & F5 cells OAM Ping in Software Loopback mode");
	printk("\n");
	printk("\n Where y : 1 = AAL5");
	printk("\n           2 = AAL0_48"); 
	printk("\n           3 = AAL0_52"); 
	printk("\n");

        return -EINVAL;
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











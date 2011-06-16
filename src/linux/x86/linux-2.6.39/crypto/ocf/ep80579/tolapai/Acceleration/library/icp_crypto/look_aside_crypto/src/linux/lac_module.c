/******************************************************************************
 *
 * This file is provided under a dual BSD/GPLv2 license.  When using or 
 *   redistributing this file, you may do so under either license.
 * 
 *   GPL LICENSE SUMMARY
 * 
 *   Copyright(c) 2007,2008,2009 Intel Corporation. All rights reserved.
 * 
 *   This program is free software; you can redistribute it and/or modify 
 *   it under the terms of version 2 of the GNU General Public License as
 *   published by the Free Software Foundation.
 * 
 *   This program is distributed in the hope that it will be useful, but 
 *   WITHOUT ANY WARRANTY; without even the implied warranty of 
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
 *   General Public License for more details.
 * 
 *   You should have received a copy of the GNU General Public License 
 *   along with this program; if not, write to the Free Software 
 *   Foundation, Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 *   The full GNU General Public License is included in this distribution 
 *   in the file called LICENSE.GPL.
 * 
 *   Contact Information:
 *   Intel Corporation
 * 
 *   BSD LICENSE 
 * 
 *   Copyright(c) 2007,2008,2009 Intel Corporation. All rights reserved.
 *   All rights reserved.
 * 
 *   Redistribution and use in source and binary forms, with or without 
 *   modification, are permitted provided that the following conditions 
 *   are met:
 * 
 *     * Redistributions of source code must retain the above copyright 
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright 
 *       notice, this list of conditions and the following disclaimer in 
 *       the documentation and/or other materials provided with the 
 *       distribution.
 *     * Neither the name of Intel Corporation nor the names of its 
 *       contributors may be used to endorse or promote products derived 
 *       from this software without specific prior written permission.
 * 
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * 
 *  version: Security.L.1.0.3-98
 *
 *****************************************************************************/

/**
*****************************************************************************
 * @file lac_module.c
 * 
 * This file contains the init and shutdown functions for the Look Aside
 * kernel Module. 
 * 
 *****************************************************************************/

#include <linux/kernel.h>
#include <linux/moduleparam.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/mutex.h>

#include "proc/lac_proc.h"

#include "cpa.h"
#include "icp_qat_hw.h"

#include "qat_comms_statistics.h"

#include "lac_module.h"

MODULE_DESCRIPTION("ICP Look Aside Crypto driver");
MODULE_AUTHOR("Intel Corporation");
MODULE_LICENSE("Dual BSD/GPL");

static struct mutex _mutex;

/* default value for statistics - ON by default */
#define LAC_DEFAULT_ISTAT_VALUE (ICP_CRYPTO_STATISTIC_ON)


struct lac_kparams_s icp_crypto =
{
    .statistics.dsa.istat        = LAC_DEFAULT_ISTAT_VALUE,
    .statistics.dh.istat         = LAC_DEFAULT_ISTAT_VALUE,
    .statistics.ln.istat         = LAC_DEFAULT_ISTAT_VALUE,
    .statistics.prime.istat      = LAC_DEFAULT_ISTAT_VALUE,
    .statistics.rsa.istat        = LAC_DEFAULT_ISTAT_VALUE,
    .statistics.key.istat        = LAC_DEFAULT_ISTAT_VALUE,
    .statistics.cb.istat         = LAC_DEFAULT_ISTAT_VALUE,
    .statistics.alg_chain.istat  = LAC_DEFAULT_ISTAT_VALUE,
    .statistics.random.istat     = LAC_DEFAULT_ISTAT_VALUE,
    .statistics.master.istat     = LAC_DEFAULT_ISTAT_VALUE,
    .statistics.msgs.istat       = LAC_DEFAULT_ISTAT_VALUE,
    .qatHmacMode                 = ICP_QAT_HW_AUTH_MODE1,
    .verbose                     = 0  /* verbosity level module param */
};

module_param_named(icp_crypto_qatHmacMode, icp_crypto.qatHmacMode, uint, 
                   0444);
MODULE_PARM_DESC(icp_crypto_qatHmacMode, 
              "Mode for Hmac algorithms. 1 = Precomputes  2 = No precomputes");

/* verbosity kernel param */
module_param_named(icp_crypto_verbose, icp_crypto.verbose, uint, 0);
MODULE_PARM_DESC(icp_crypto_verbose, 
              "Sets the verbosity level of the icp_crypto module: 0 - Inf");

/* The exceptional master switch for statistics: [all | none] */
module_param_string(icp_crypto_statistics_master, 
                    icp_crypto.statistics.master.sstat,
                    LAC_KPARAM_MAX_STRLEN, 0);
MODULE_PARM_DESC(icp_crypto_statistics_master, 
              "Master statistics' collection switch. [all | none]");

/* switches for statistics' collection: [on | off]*/
module_param_string(icp_crypto_statistics_dsa,
                    icp_crypto.statistics.dsa.sstat,
                    LAC_KPARAM_MAX_STRLEN, 0);
MODULE_PARM_DESC(icp_crypto_statistics_dsa, 
              "DSA statistics' collection switch. [on|off]");

module_param_string(icp_crypto_statistics_dh,
                    icp_crypto.statistics.dh.sstat,
                    LAC_KPARAM_MAX_STRLEN, 0);
MODULE_PARM_DESC(icp_crypto_statistics_dh, 
              "Diffie Hellman statistics' collection switch. [on|off]");

module_param_string(icp_crypto_statistics_ln,
                    icp_crypto.statistics.ln.sstat,
                    LAC_KPARAM_MAX_STRLEN, 0);
MODULE_PARM_DESC(icp_crypto_statistics_ln, 
              "Large Numbers statistics' collection switch. [on|off]");

module_param_string(icp_crypto_statistics_prime,
                    icp_crypto.statistics.prime.sstat,
                    LAC_KPARAM_MAX_STRLEN, 0);
MODULE_PARM_DESC(icp_crypto_statistics_prime, 
              "Prime statistics' collection switch. [on|off]");

module_param_string(icp_crypto_statistics_rsa,
                    icp_crypto.statistics.rsa.sstat,
                    LAC_KPARAM_MAX_STRLEN, 0);
MODULE_PARM_DESC(icp_crypto_statistics_rsa, 
              "RSA statistics' collection switch. [on|off]");

module_param_string(icp_crypto_statistics_key,
                    icp_crypto.statistics.key.sstat,
                    LAC_KPARAM_MAX_STRLEN, 0);
MODULE_PARM_DESC(icp_crypto_statistics_key, 
              "Key statistics' collection switch. [on|off]");

module_param_string(icp_crypto_statistics_cb,
                    icp_crypto.statistics.cb.sstat,
                    LAC_KPARAM_MAX_STRLEN, 0);
MODULE_PARM_DESC(icp_crypto_statistics_cb, 
              "Callback statistics' collection switch. [on|off]");

module_param_string(icp_crypto_statistics_alg_chain,
                    icp_crypto.statistics.alg_chain.sstat,
                    LAC_KPARAM_MAX_STRLEN, 0);
MODULE_PARM_DESC(icp_crypto_statistics_alg_chain, 
              "Algorithms' chaining statistics' collection switch. [on|off]");

module_param_string(icp_crypto_statistics_random,
                    icp_crypto.statistics.random.sstat,
                    LAC_KPARAM_MAX_STRLEN, 0);
MODULE_PARM_DESC(icp_crypto_statistics_random, 
              "Random statistics' collection switch. [on|off]");

module_param_string(icp_crypto_statistics_msgs,
                    icp_crypto.statistics.msgs.sstat,
                    LAC_KPARAM_MAX_STRLEN, 0);
MODULE_PARM_DESC(icp_crypto_statistics_msgs, 
              "Messages - <internal> statistics' collection switch. [on|off]");


static icp_crypto_statistics_t 
lac_module_strtun2int(const char * const tunable_str, 
                      icp_crypto_statistics_t * const tunable_int);

static void lac_parse_stats_tunables(void);
static void lac_print_stats_tunables(void);

static int __init Lac_ModInit( void )
{
    if (!((ICP_QAT_HW_AUTH_MODE1 == icp_crypto.qatHmacMode) || 
          (ICP_QAT_HW_AUTH_MODE2 == icp_crypto.qatHmacMode)))
    {
        printk("Invalid value for param qatHmacMode\n");
        return -1;
    }
    
    if(0 != LacProc_init())
    {
        printk( "Error creating LAC /proc filesystem files\n" );
        return -1;
    }
    else
    {
        if(icp_crypto.verbose)
            printk( "Loading LAC Module ...\n" );
    }

    mutex_init(&_mutex);

    lac_parse_stats_tunables();

    return 0;
}


static void __exit Lac_ModExit( void )
{
    LacProc_cleanup();

    mutex_lock(&_mutex);
    mutex_destroy(&_mutex);

    printk("Unloading LAC Module ...\n" ) ;
}

void Lac_SetQatHmacMode(const icp_qat_hw_auth_mode_t mode)
{
    mutex_lock(&_mutex);
    icp_crypto.qatHmacMode = mode;
    mutex_unlock(&_mutex);
}

icp_qat_hw_auth_mode_t Lac_GetQatHmacMode(void)
{
    icp_qat_hw_auth_mode_t mode = 0;
    mutex_lock(&_mutex);
    mode = icp_crypto.qatHmacMode;
    mutex_unlock(&_mutex);
    return mode;
}

/* Converts string tunable on/off/error into 1/0/-1 
   In the case of on/off the appropriate value of 1 or 0 is recorded in the *tunable
 */
static icp_crypto_statistics_t 
lac_module_strtun2int(const char * const tunable_str, 
                      icp_crypto_statistics_t * const tunable_int)
{
    if(tunable_str[0] == 'o')
    {
        if(tunable_str[1] == 'n')
        {
            return *tunable_int = ICP_CRYPTO_STATISTIC_ON;
        }
        else
        {
            if((tunable_str[1] == 'f') && (tunable_str[2] == 'f'))
            {
                return *tunable_int = ICP_CRYPTO_STATISTIC_OFF;
            }
        }
    }
    return ICP_CRYPTO_STATISTIC_ERROR; /* and leave *tunable_int untouched */
}

static void lac_parse_stats_tunables(void)
{
    lac_statistics_t *s = &icp_crypto.statistics;

    /* 
     * Now processing the master switch as a specific one 
     * icp_crypto.statistics.master=none is served as a fuse
     * and switches all statistics off.
     * Setting one to none makes all other setting ignored.
     */
    if(strncmp(s->master.sstat, "none", 4) == 0)
    {
        s->master.istat    = ICP_CRYPTO_STATISTIC_OFF;
        s->alg_chain.istat = ICP_CRYPTO_STATISTIC_OFF;
        s->cb.istat        = ICP_CRYPTO_STATISTIC_OFF;
        s->dh.istat        = ICP_CRYPTO_STATISTIC_OFF;
        s->dsa.istat       = ICP_CRYPTO_STATISTIC_OFF;
        s->dsa.istat       = ICP_CRYPTO_STATISTIC_OFF;
        s->key.istat       = ICP_CRYPTO_STATISTIC_OFF;
        s->ln.istat        = ICP_CRYPTO_STATISTIC_OFF;
        s->msgs.istat      = ICP_CRYPTO_STATISTIC_OFF;
        s->prime.istat     = ICP_CRYPTO_STATISTIC_OFF;
        s->random.istat    = ICP_CRYPTO_STATISTIC_OFF;
        s->rsa.istat       = ICP_CRYPTO_STATISTIC_OFF;

        if(icp_crypto.verbose)
                printk("statistics.master = none --> All statistics OFF\r\n");
    } /* end if "none" */

    if(strncmp(s->master.sstat, "all", 3) == 0)
    {
        /* it is the default - allow other statistics to be set manually */
        /* set the statistics to the default value  if they values are 
           neither on nor off */

        s->master.istat    = ICP_CRYPTO_STATISTIC_ON;

        if(lac_module_strtun2int(s->alg_chain.sstat, 
            &s->alg_chain.istat) == ICP_CRYPTO_STATISTIC_ERROR)
        {
            s->alg_chain.istat = LAC_DEFAULT_ISTAT_VALUE;
        }
        if(lac_module_strtun2int(s->cb.sstat, 
            &s->cb.istat) == ICP_CRYPTO_STATISTIC_ERROR)
        {
            s->cb.istat        = LAC_DEFAULT_ISTAT_VALUE;
        }
        if(lac_module_strtun2int(s->dh.sstat,
           &s->dh.istat) == ICP_CRYPTO_STATISTIC_ERROR)
        {
            s->dh.istat        = LAC_DEFAULT_ISTAT_VALUE;
        }
        if(lac_module_strtun2int(s->dsa.sstat,
            &s->dsa.istat) == ICP_CRYPTO_STATISTIC_ERROR)
        {
            s->dsa.istat       = LAC_DEFAULT_ISTAT_VALUE;
        }
        if(lac_module_strtun2int(s->key.sstat,
            &s->key.istat) == ICP_CRYPTO_STATISTIC_ERROR)
        {
            s->key.istat       = LAC_DEFAULT_ISTAT_VALUE;
        }
        if(lac_module_strtun2int(s->ln.sstat,
            &s->ln.istat) == ICP_CRYPTO_STATISTIC_ERROR)
        {
            s->ln.istat        = LAC_DEFAULT_ISTAT_VALUE;
        }
        if(lac_module_strtun2int(s->msgs.sstat,
            &s->msgs.istat) == ICP_CRYPTO_STATISTIC_ERROR)
        {
            s->msgs.istat      = LAC_DEFAULT_ISTAT_VALUE;
        }
        if(lac_module_strtun2int(s->prime.sstat,
            &s->prime.istat) == ICP_CRYPTO_STATISTIC_ERROR)
        {
            s->prime.istat     = LAC_DEFAULT_ISTAT_VALUE;
        }
        if(lac_module_strtun2int(s->random.sstat,
            &s->random.istat) == ICP_CRYPTO_STATISTIC_ERROR)
        {
            s->random.istat    = LAC_DEFAULT_ISTAT_VALUE;
        }
        if(lac_module_strtun2int(s->rsa.sstat,
            &s->rsa.istat) == ICP_CRYPTO_STATISTIC_ERROR)
        {
            s->rsa.istat       = LAC_DEFAULT_ISTAT_VALUE;
        }
        if(icp_crypto.verbose)
           printk("statistics.master = all --> All statistics ON\r\n");
    } /* end if "all" */

    /* now re-map the ICP_CRYPTO into QatComms' internal setting */
    if(ICP_CRYPTO_STATISTIC_ON == s->msgs.istat)
    {
        QatComms_setQatCollectStatistics(QAT_COMMS_STATISTICS_ON);
    }
    else
    {
        QatComms_setQatCollectStatistics(QAT_COMMS_STATISTICS_OFF);
    }

    if(icp_crypto.verbose)
        lac_print_stats_tunables();
}

static void lac_print_stats_tunables(void)
{
    printk("   Collecting statistics: \r\n");
    printk("      statistics_master    = %s\r\n",
        icp_crypto.statistics.master.istat == ICP_CRYPTO_STATISTIC_ON ?
            "all" : "none");
    printk("      statistics_dsa       = %s\r\n",
        icp_crypto.statistics.dsa.istat == ICP_CRYPTO_STATISTIC_ON ?
            "on" : "off");
    printk("      statistics_dh        = %s\r\n",
        icp_crypto.statistics.dh.istat == ICP_CRYPTO_STATISTIC_ON ?
            "on" : "off");
    printk("      statistics_ln        = %s\r\n",
        icp_crypto.statistics.ln.istat == ICP_CRYPTO_STATISTIC_ON ?
            "on" : "off");
    printk("      statistics_rsa       = %s\r\n",
        icp_crypto.statistics.rsa.istat == ICP_CRYPTO_STATISTIC_ON ?
            "on" : "off");
    printk("      statistics_key       = %s\r\n",
        icp_crypto.statistics.key.istat == ICP_CRYPTO_STATISTIC_ON ?
            "on" : "off");
    printk("      statistics_cb        = %s\r\n",
        icp_crypto.statistics.cb.istat == ICP_CRYPTO_STATISTIC_ON ?
            "on" : "off");
    printk("      statistics_alg_chain = %s\r\n",
        icp_crypto.statistics.cb.istat == ICP_CRYPTO_STATISTIC_ON ?
            "on" : "off");
    printk("      statistics_random    = %s\r\n",
        icp_crypto.statistics.random.istat == ICP_CRYPTO_STATISTIC_ON ?
            "on" : "off");
    printk("      statistics_prime     = %s\r\n",
        icp_crypto.statistics.prime.istat == ICP_CRYPTO_STATISTIC_ON ?
            "on" : "off");
    printk("      statistics_msgs      = %s\r\n",
        icp_crypto.statistics.msgs.istat == ICP_CRYPTO_STATISTIC_ON ?
            "on" : "off");
}


module_init(Lac_ModInit);
module_exit(Lac_ModExit);


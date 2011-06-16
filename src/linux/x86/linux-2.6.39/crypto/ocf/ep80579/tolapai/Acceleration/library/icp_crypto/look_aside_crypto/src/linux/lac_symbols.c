/***************************************************************************
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
 ***************************************************************************/

/**
*****************************************************************************
 * @file lac_symbols.c
 *
 * This file contains all the symbols that are exported by the Look Aside
 * kernel Module.
 *
 *****************************************************************************/

#include <linux/module.h>
#include "cpa.h"
#include "cpa_cy_sym.h"
#include "cpa_cy_common.h"
#include "cpa_cy_dh.h"
#include "cpa_cy_rand.h"
#include "cpa_cy_key.h"
#include "cpa_cy_prime.h"
#include "cpa_cy_dsa.h"
#include "cpa_cy_rsa.h"
#include "icp_lac_cfg.h"
#include "cpa_cy_ln.h"
#include "cpa_cy_im.h"
#include "icp_lac_hash_precompute.h"

/* Symbols for Symmetric */
EXPORT_SYMBOL(cpaCySymInitSession);
EXPORT_SYMBOL(cpaCySymRemoveSession);
EXPORT_SYMBOL(cpaCySymPerformOp);
EXPORT_SYMBOL(cpaCySymQueryStats);
EXPORT_SYMBOL(cpaCySymSessionCtxGetSize);

/* Symbols for Rand */
EXPORT_SYMBOL(cpaCyRandGen);
EXPORT_SYMBOL(cpaCyRandQueryStats);

/* Diffie Hellman */
EXPORT_SYMBOL(cpaCyDhKeyGenPhase1);
EXPORT_SYMBOL(cpaCyDhKeyGenPhase2Secret);
EXPORT_SYMBOL(cpaCyDhQueryStats);

/*Key Expansion and Generation*/
EXPORT_SYMBOL(cpaCyKeyGenSsl);
EXPORT_SYMBOL(cpaCyKeyGenTls);
EXPORT_SYMBOL(cpaCyKeyGenMgf);
EXPORT_SYMBOL(cpaCyKeyGenQueryStats);

/* Large Number ModExp and ModInv */
EXPORT_SYMBOL(cpaCyLnModExp);
EXPORT_SYMBOL(cpaCyLnModInv);
EXPORT_SYMBOL(cpaCyLnStatsQuery);

/* Prime */
EXPORT_SYMBOL(cpaCyPrimeTest);
EXPORT_SYMBOL(cpaCyPrimeQueryStats);

/* DSA */
EXPORT_SYMBOL(cpaCyDsaGenPParam);
EXPORT_SYMBOL(cpaCyDsaGenGParam);
EXPORT_SYMBOL(cpaCyDsaGenYParam);
EXPORT_SYMBOL(cpaCyDsaSignR);
EXPORT_SYMBOL(cpaCyDsaSignS);
EXPORT_SYMBOL(cpaCyDsaSignRS);
EXPORT_SYMBOL(cpaCyDsaVerify);
EXPORT_SYMBOL(cpaCyDsaQueryStats);

/* RSA */
EXPORT_SYMBOL(cpaCyRsaGenKey);
EXPORT_SYMBOL(cpaCyRsaEncrypt);
EXPORT_SYMBOL(cpaCyRsaDecrypt);
EXPORT_SYMBOL(cpaCyRsaQueryStats);

/* Symbols for all of LAC */
EXPORT_SYMBOL(cpaCyBufferListGetMetaSize);
EXPORT_SYMBOL(icp_AsdCfgLacInit);
EXPORT_SYMBOL(icp_AsdCfgLacStart);
EXPORT_SYMBOL(icp_AsdCfgLacStop);
EXPORT_SYMBOL(icp_AsdCfgLacShutdown);

EXPORT_SYMBOL(cpaCyGetInstances);
EXPORT_SYMBOL(cpaCyGetNumInstances);
EXPORT_SYMBOL(cpaCyGetStatusText);
EXPORT_SYMBOL(cpaCyInstanceGetInfo);
EXPORT_SYMBOL(cpaCyInstanceSetNotificationCb);

EXPORT_SYMBOL(cpaCyStartInstance);
EXPORT_SYMBOL(cpaCyStopInstance);

/* Symbols for use by other internal s/w components */
EXPORT_SYMBOL(icp_LacHashPrecomputeSynchronous);

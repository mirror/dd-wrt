/*****************************************************************************
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
 * @ingroup icp_Qatal
 *
 * @file  qatal_symbols.c
 *
 * @description
 *      This file contains all the symbols that are exported by the QAT-AL
 *      kernel Module.
 *
 * @par
 *
 *****************************************************************************/

#include <linux/module.h>
#include "cpa.h"
#include "icp_accel_handle.h"
#include "icp_asd_cfg.h"
#include "icp_qatal_cfg.h"
#include "qat_comms.h"
#include "qatal_rand.h"
#include "uclo.h"
#include "halAeApi.h"

/* Symbols for ASD interface */
EXPORT_SYMBOL(icp_AsdCfgQatalInit);
EXPORT_SYMBOL(icp_AsdCfgQatalStart);
EXPORT_SYMBOL(icp_AsdCfgQatalStop);
EXPORT_SYMBOL(icp_AsdCfgQatalShutdown);

/* Symbols for Qatal check of Firmware */
EXPORT_SYMBOL(Qatal_FWCountGet);

/* Symbols for QAT-Comms interface */
EXPORT_SYMBOL(QatComms_Init);
EXPORT_SYMBOL(QatComms_ResponseCbSet);
EXPORT_SYMBOL(QatComms_ReqHdrCreate);
EXPORT_SYMBOL(QatComms_MsgSend);
EXPORT_SYMBOL(QatComms_Shutdown);
EXPORT_SYMBOL(QatComms_MsgCountGet);
EXPORT_SYMBOL(QatComms_intr);
EXPORT_SYMBOL(QatComms_bh_handler);
EXPORT_SYMBOL(QatComms_bh_schedule_register);

EXPORT_SYMBOL(QatComms_setQatCollectStatistics);
EXPORT_SYMBOL(QatComms_getQatCollectStatistics);

/* Symbols for QAT-Rand */
EXPORT_SYMBOL(icp_QatalRandEntrophyTestRun);


EXPORT_SYMBOL(halAe_Init);
EXPORT_SYMBOL(halAe_ETRingInit);
EXPORT_SYMBOL(halAe_DelLib);

EXPORT_SYMBOL(UcLo_InitLib);
EXPORT_SYMBOL(UcLo_MapObjAddr);
EXPORT_SYMBOL(UcLo_DeleObj);
EXPORT_SYMBOL(UcLo_WriteUimageAll);


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
 * @file qat_comms_statistics.h
 *
 * @defgroup icp_QatComms  QAT Comms API
 *
 * @ingroup icp_Qatal
 *
 * @description
 *      This file documents the external interfaces for interfacing with the QAT.
 *
 *****************************************************************************/

#if !defined(__QAT_COMMS_STATISTICS_H__)
#define __QAT_COMMS_STATISTICS_H__

/**
 *****************************************************************************
 * @ingroup icp_QatComms
 *      Statistics on/off enum
 * @description
 *      Enumeration of statistics collection flip-flop states
 *      Currently two levels - ON/OFF
 *****************************************************************************/
typedef enum {
    QAT_COMMS_STATISTICS_OFF = 0,
    QAT_COMMS_STATISTICS_ON  = 1,
} qat_comms_statistics_t;

/**
 *******************************************************************************
 * @ingroup icp_QatComms
 *
 * NAME: QatComms_setQatCollectStatistics
 *
 * DESCRIPTION: This function will set the statistics collection
 *
 * @Param:  - IN collectStatistics QAT_COMMS_STATISTICS_OFF: OFF
 *                                 QAT_COMMS_STATISTICS_ON:  ON
 *
 * @Return: IX_SUCCESS
 *
 */

CpaStatus QatComms_setQatCollectStatistics(const qat_comms_statistics_t
                                           collectStatistics);

/**
 * @ingroup icp_QatComms
 *
 * NAME: QatComms_getQatCollectStatistics
 *
 * DESCRIPTION: This function will get the statistics collection seting
 *
 * @Param:  - (void)
 *
 * @Return: current value  of qatCollectStatistics:
 *                QAT_COMMS_STATISTICS_OFF: OFF
 *                QAT_COMMS_STATISTICS_ON:  ON
 *
 */

qat_comms_statistics_t QatComms_getQatCollectStatistics(void);

#endif

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

/*
 *****************************************************************************
 * Doxygen group definitions
 ****************************************************************************/

/**
 *****************************************************************************
 * @defgroup icp_Qatal QAT-AL API
 * 
 * @ingroup icp
 *
 * @description
 *      The QAT Access Layer (QATAL) provides an API which may be used to
 *      perform init and admin operations on the QAT.
 *
 *****************************************************************************/

/**
 *****************************************************************************
 * @file qatal_init_defs.h
 * 
 * @defgroup icp_QatalInitDefs QATAL Initialization parameter values.
 * 
 * @ingroup icp_Qatal
 *
 * @description
 *      This header file contains user defines which can be configured to set
 *      some QAT initialization parameters.
 *
 *****************************************************************************/

#ifndef QATAL_INIT_DEFS_H
#define QATAL_INIT_DEFS_H

/*
******************************************************************************
* Include public/global header files
******************************************************************************
*/
#include "icp_rings.h"

/* TODO: add SET_CONSTANTS and CONFIG_NRBG param values */

/* TODO: consider inverting weighting value specified here to be more user-friendly */

#define QATAL_QAT_AE_NUM (0)
/**< @ingroup icp_QatalInitDefs
 * QAT-AE AE Number */

#define QATAL_NUM_PATCHED_SYMBOLS (1)
/**< @ingroup icp_QatalInitDefs
 * Number of patched symbols */

#define QATAL_ENABLE_CONTEXT_MASK (0xFFFFFFFF) 
/**< @ingroup icp_QatalInitDefs
 * Mask for threads enabled in QAT-AE. 0xFFFFFFFF enables all threads */

#define QATAL_ADMIN_WAIT_COUNT      (100)
/**< @ingroup icp_QatalInitDefs
 * Number of times to sleep and wakeup when waaiting for admin msg response */

#define QATAL_ADMIN_TIMEOUT_IN_MS   (25)  
/**< @ingroup icp_QatalInitDefs
 * Time to sleep in ms when waiting for admin msg response */

#define QATAL_ADMIN_RING_SYMBOL_NAME "ICP_QAT_FW_INIT_RING"
/**< @ingroup icp_QatalInitDefs
 * This is the name of the QAT admin ring patched symbol */

#define QATAL_ADMIN_RING_SYMBOL_VAL (ICP_RING_QATAL_ADMIN_REQUEST)
/**< @ingroup icp_QatalInitDefs
 * This is the value of the QAT admin ring patched symbol */

#define QATAL_BULK_RINGS_MASK ((0x1<<(ICP_RING_LAC_LA_HI_REQUEST))|\
                               (0x1<<(ICP_RING_LAC_LA_LO_REQUEST))|\
                               (0x1<<(ICP_RING_QATAL_ADMIN_REQUEST)) )

/**< @ingroup icp_QatalInitDefs
 * This is the bit value for the QAT-AE bulk ring poll list */

#define QATAL_PKE_RINGS_MASK  (0x1<<(ICP_RING_LAC_PKE_REQUEST))
/**< @ingroup icp_QatalInitDefs
 * This is the bit value for the QAT-AE PKE ring poll list */

#define QATAL_ADMIN_RING_WEIGHTING 0xFF
/**< @ingroup icp_QatalInitDefs
 * This controls the weighting of the QAT Admin Ring */

#define QATAL_LAC_HIGH_PRIORITY_RING_WEIGHTING 0xFC
/**< @ingroup icp_QatalInitDefs
 * This controls the weighting of the QAT LAC High Priority Ring */

#define QATAL_LAC_LOW_PRIORITY_RING_WEIGHTING 0xFE
/**< @ingroup icp_QatalInitDefs
 * This controls the weighting of the QAT LAC Low Priority Ring */

#define QATAL_PKE_RING_WEIGHTING 0xFF
/**< @ingroup icp_QatalInitDefs
 * This controls the weighting of the QAT PKE Ring */

#define QATAL_IPSEC_RING_WEIGHTING 0xFF
/**< @ingroup icp_QatalInitDefs
 * This controls the weighting of the QAT IPSEC Ring */

#define QATAL_DEFAULT_RING_WEIGHTING 0xFF
/**< @ingroup icp_QatalInitDefs
 * This controls the weighting of the QAT IPSEC Ring */

#define QATAL_RAND_BLOCK_SIZE 2500
/**< @ingroup icp_QatalInitDefs
 * This is the size of the entrophy block in bytes 20,000bits */

#endif /* QATAL_INIT_DEFS_H */


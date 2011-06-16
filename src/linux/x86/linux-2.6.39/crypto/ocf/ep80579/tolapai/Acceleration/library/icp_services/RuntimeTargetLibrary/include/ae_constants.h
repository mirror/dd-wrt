/**
 **************************************************************************
 * @file ae_constants.h
 *
 * @description
 *      This is the header file for Hardware Abstraction Layer
 *
 * @par 
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
 **************************************************************************/ 

#ifndef __AE_CONSTANTS_H
#define __AE_CONSTANTS_H

/**
 *****************************************************************************
 * @file ae_constants.h
 * 
 * @ingroup icp_hal
 *
 * @description
 *      This header file contains the global macros used by HAL 
 *
 *****************************************************************************/

#include "icptype.h"

#define AE_NUMCLUSTR            2                   /**< number of AE cluster */
#define AE_PERCLUSTR            8                   /**< number of AEs per cluster */
#define MAX_NUM_AE              0x10                /**< number of AccelEngines */
#define MAX_CTX                 8                   /**< contexts per engine */

#define MAX_USTORE_PER_SEG      0x2000              /**< 8k ustore size per segment */

#define MAX_USTORE              MAX_USTORE_PER_SEG  /**< total ustore size */
#define UADDR_MASK              (MAX_USTORE-1)      /**< micro address mask */
#define NUM_USTORE_SEGS         1                   /**< number of ustore segments */

#define MAX_XFER_REG            128                 /**< max xfer */
#define MAX_GPR_REG             128                 /**< max gpr */
#define MAX_NN_REG              128                 /**< number of NN per AE */
#define MAX_LMEM_REG            640                 /**< number of local-memory */
#define MAX_ICP_LMEM_REG        1024                /**< number of ICP local-memory */
#define MAX_INP_STATE           16                  /**< number of inp_state */
#define MAX_CAM_REG             16                  /**< number of CAM entry */
#define MAX_FIFO_QWADDR         160                 /**< maximum fifo address */
#define AE_ALL_CTX              0xff                /**< all contexts */

#define MAX_GIGE_MAC            3                   /**< max Gige mac */
#define MAX_SSU_SHARED_RAM      (32*1024)           /**< number of SSU share ram */
#define MAX_CAP_CSR             (8*1024)            /**< number of cap csr */
#define MAX_ETRING_CSR          (4*1024)            /**< number of etring csr */
#define MAX_GIGE_CSR            128*1024            /**< number of GIGE csr */
#define MAX_HASH_CSR            0x20                /**< number of HASH csr */
#define MAX_PCI_HEAD_CSR        256                 /**< number of pci head csr */
#define MAX_MEMTGT_CSR          0x8                 /**< number of memory target csr */
#define MAX_SSU_CSR             (4*1024)            /**< number of SSU csr */

#endif  /* __AE_CONSTANTS_H */

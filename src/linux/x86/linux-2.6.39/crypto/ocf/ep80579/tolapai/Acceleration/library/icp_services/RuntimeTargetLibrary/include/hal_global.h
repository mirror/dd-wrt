/**
 **************************************************************************
 * @file hal_global.h
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

/**
 *****************************************************************************
 * @file hal_global.h
 * 
 * @ingroup icp_hal
 *
 * @description
 *      This header file contains the internal global macros used by HAL 
 *
 *****************************************************************************/
 
#ifndef __HAL_GLOBAL_H
#define __HAL_GLOBAL_H

#include "halAeApi.h"
#include "halMmap.h"
#include "IxOsal.h"

typedef enum{
    MISC_CONTROL        = 0x04,      /**< misc_control global cap CSR offset */
    ICP_RESET           = 0x0c       /**< icp_reset global cap CSR offset */
}Hal_Global_CSR_T;

typedef enum{
    CPP_AENCBASE_OFF        = 0x00,  /**< None-coherent dram base address CSR offset */
    CPP_AENCLIMIT_OFF       = 0x04   /**< None-coherent dram limit address CSR offset */
}Hal_Mem_Tgt_CSR_T;

/* global CSR offset */
#define GLOBAL_CSR      0xa00

/**< write cap csr */
#define SET_CAP_CSR(csr, val) WRITE_LWORD((Hal_cap_global_ctl_csr_virtAddr + (csr)), (val))
/**< read cap csr */
#define GET_CAP_CSR(csr) READ_LWORD((Hal_cap_global_ctl_csr_virtAddr + (csr)))

/**< write cap global csr */
#define SET_GLB_CSR(csr, val) SET_CAP_CSR(((csr) + GLOBAL_CSR), (val))
/**< read cap global csr */
#define GET_GLB_CSR(csr) GET_CAP_CSR((GLOBAL_CSR + (csr)))

/**< write memory target csr */
#define SET_MEMTGT_CSR(csr, val) WRITE_LWORD((Hal_memory_target_csr_virtAddr + (csr)), (val))
/**< read memory target csr */
#define GET_MEMTGT_CSR(csr) READ_LWORD((Hal_memory_target_csr_virtAddr + (csr)))

/* CSR: PRODUCT_ID masks - this is fake */
#define PID_MINOR_REV_BITPOS            0    /**< product ID minor revision bit postion */    
#define PID_MAJOR_REV_BITPOS            4    /**< product ID major revision bit postion */    
#define PID_MINOR_PROD_TYPE_BITPOS      8    /**< product ID minor product type bit postion */    
#define PID_MAJOR_PROD_TYPE_BITPOS      16   /**< product ID major product type  bit postion */    

#define PID_MINOR_REV                   (0xf)  /**< product ID minor revision */    
#define PID_MAJOR_REV                   (0xf << PID_MAJOR_REV_BITPOS)  /**< product ID major revision */    
#define PID_MINOR_PROD_TYPE             (0xff << PID_MINOR_PROD_TYPE_BITPOS)   /**< product ID minor product type */    
#define PID_MAJOR_PROD_TYPE             (0xff << PID_MAJOR_PROD_TYPE_BITPOS)   /**< product ID major product type */    

/* CSR: MISC_CONTROL masks */
#define MC_TIMESTAMP_ENABLE_BITPOS      7    /**< TIMESTAMP enable bit postion */    
#define MC_TIMESTAMP_ENABLE     (0x1 << MC_TIMESTAMP_ENABLE_BITPOS)  /**< TIMESTAMP enable */    

/* CSR: ICP_RESET masks */
#define ICPR_UE0_BITPOS            0    /**< AccelEngine 0 bit postion */    
#define ICPR_UE1_BITPOS            1    /**< AccelEngine 1 bit postion */      
#define ICPR_UE2_BITPOS            2    /**< AccelEngine 2 bit postion */    
#define ICPR_UE3_BITPOS            3    /**< AccelEngine 3 bit postion */    
#define ICPR_UE4_BITPOS            4    /**< AccelEngine 4 bit postion */    
#define ICPR_UE5_BITPOS            5    /**< AccelEngine 5 bit postion */    

#define ICPR_UE0                   (0x1 << ICPR_UE0_BITPOS)  /**< AccelEngine 0 */    
#define ICPR_UE1                   (0x1 << ICPR_UE1_BITPOS)  /**< AccelEngine 1 */    
#define ICPR_UE2                   (0x1 << ICPR_UE2_BITPOS)  /**< AccelEngine 2 */    
#define ICPR_UE3                   (0x1 << ICPR_UE3_BITPOS)  /**< AccelEngine 3 */    
#define ICPR_UE4                   (0x1 << ICPR_UE4_BITPOS)  /**< AccelEngine 4 */    
#define ICPR_UE5                   (0x1 << ICPR_UE5_BITPOS)  /**< AccelEngine 5 */    

#define AENC_ALIGN_SHIFT           12   /**< None-coherent address shift */    

#endif  /* __HAL_GLOBAL_H */

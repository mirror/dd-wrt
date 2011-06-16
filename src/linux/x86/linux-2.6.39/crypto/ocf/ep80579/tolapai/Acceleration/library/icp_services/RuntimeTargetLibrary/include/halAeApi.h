/**
 **************************************************************************
 * @file halAeApi.h
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

/*
 ****************************************************************************
 * Doxygen group definitions
 ****************************************************************************/

/**
 *****************************************************************************
 * @file halAeApi.h
 * 
 * @defgroup icp_hal Hardware Abstraction Layer
 *
 * @description
 *      This header file that contains the prototypes and definitions required
 *      for Hardware Abstraction Layer
 *
 *****************************************************************************/
 
#ifndef __HAL_AEAPI_H
#define __HAL_AEAPI_H

#include "core_io.h"

#define MAX_EXEC_INST           100
#define HALAE_INVALID_XADDR 0xffffffffffffffffULL

#ifdef __cplusplus
extern "C" {
#endif
HAL_DECLSPEC int          halAe_ReadMult(void* dst, uint64 vaddr_src, int size);
HAL_DECLSPEC int          halAe_WriteMult(uint64 vaddr_dst, void* src, int size);
HAL_DECLSPEC int          halAe_MemSet(uint64 vaddr_dst, int c, int size);
#ifdef __cplusplus
}
#endif

#include "hal_ae.h"
#include "ae_constants.h"

enum{
   HALAE_SUCCESS=0,         /**< the operation was successful */
   HALAE_FAIL=0x8100,       /**< the operation failed */
   HALAE_BADARG,            /**< bad function argument */
   HALAE_DUPBKP,            /**< duplicate break point */
   HALAE_NOUSTORE,          /**< not ustore available */
   HALAE_BADADDR,           /**< bad address */
   HALAE_BADLIB,            /**< bad debug library -- wasn't initialized */
   HALAE_DISABLED,          /**< acceleration engine or interrupt disabled */
   HALAE_ENABLED,           /**< acceleration engine enabled */
   HALAE_RESET,             /**< acceleration engine is in reset */
   HALAE_TIMEOUT,           /**< the operation execced th etime limit */
   HALAE_ISSET,             /**< condition/evaluation is set/true */
   HALAE_NOTSET,            /**< condition/evaluation is not set/false */
   HALAE_AEACTIVE,          /**< ae is running */
   HALAE_MEMALLOC,          /**< memory allocation error */
   HALAE_RINGFULL,          /**< ET ring full */
   HALAE_RINGEMPTY,         /**< ET ring empty */
   HALAE_NEIGHAEACTIVE      /**< neighbour ae is running */
};

enum{
    HALAE_UNINIT,           /**< HAL library uninitialized */ 
    HALAE_CLR_RST,          /**< HAL library clear reset status */  
    HALAE_RST               /**< HAL library reset status */  
};

typedef enum{
    RINGTYPE_IAPUTGET=0x0  /**< IA does both put and get operation */  
}RING_TYPE;

/* ET ring CSR offset definition */
typedef enum{
    ET_RING_CONFIG         = 0x0,       /**< ring config Reg offset */ 
    ET_RING_BASE           = 0x100,     /**< ring base Reg offset */ 
    ET_RING_HEAD_OFFSET    = 0x200,     /**< ring head Reg offset */ 
    ET_RING_TAIL_OFFSET    = 0x300,     /**< ring tail Reg offset */ 
  
    ET_RING_STAT_0           = 0x600,   /**< ring status0 Reg offset */ 
    ET_RING_STAT_1           = 0x604,   /**< ring status1 Reg offset */ 
    ET_RING_STAT_2           = 0x608,   /**< ring status2 Reg offset */ 
    ET_RING_STAT_3           = 0x60C,   /**< ring status3 Reg offset */ 
    ET_RING_STAT_4           = 0x610,   /**< ring status4 Reg offset */ 
    ET_RING_STAT_5           = 0x614,   /**< ring status5 Reg offset */ 
    ET_RING_STAT_6           = 0x618,   /**< ring status6 Reg offset */ 
    ET_RING_STAT_7           = 0x61C,   /**< ring status7 Reg offset */ 
                            
    ET_RING_UO_STAT_0        = 0x620,   /**< underflow/overflow status0 Reg */ 
    ET_RING_UO_STAT_1        = 0x624,   /**< underflow/overflow status1 Reg */ 
    ET_RING_UO_STAT_2        = 0x628,   /**< underflow/overflow status2 Reg */ 
    ET_RING_UO_STAT_3        = 0x62C,   /**< underflow/overflow status3 Reg */ 

    ET_RING_E_STAT_0         = 0x630,   /**< ring empty status0 Reg offset */ 
    ET_RING_E_STAT_1         = 0x634,   /**< ring empty status1 Reg offset */ 

    ET_RING_NE_STAT_0        = 0x640,   /**< ring near empty status0 Reg offset */ 
    ET_RING_NE_STAT_1        = 0x644,   /**< ring near empty status1 Reg offset */ 
    
    ET_RING_NF_STAT_0        = 0x650,   /**< ring near full status0 Reg offset */ 
    ET_RING_NF_STAT_1        = 0x654,   /**< ring near full status1 Reg offset */ 

    ET_RING_F_STAT_0         = 0x660,   /**< ring full status0 Reg offset */ 
    ET_RING_F_STAT_1         = 0x664,   /**< ring full status0 Reg offset */ 

    ET_RING_C_STAT_0         = 0x700,   /**< ring compelete status0 Reg offset */ 
    ET_RING_C_STAT_1         = 0x704,   /**< ring compelete status1 Reg offset */ 
    ET_RING_C_STAT_2         = 0x708,   /**< ring compelete status2 Reg offset */ 
    ET_RING_C_STAT_3         = 0x70C,   /**< ring compelete status3 Reg offset */ 
    ET_RING_C_STAT_4         = 0x710,   /**< ring compelete status4 Reg offset */ 
    ET_RING_C_STAT_5         = 0x714,   /**< ring compelete status5 Reg offset */ 
    ET_RING_C_STAT_6         = 0x718,   /**< ring compelete status6 Reg offset */ 
    ET_RING_C_STAT_7         = 0x71C,   /**< ring compelete status7 Reg offset */ 
    ET_RING_C_STAT_8         = 0x720,   /**< ring compelete status8 Reg offset */ 
    ET_RING_C_STAT_9         = 0x724,   /**< ring compelete status9 Reg offset */ 
    ET_RING_C_STAT_10        = 0x728,   /**< ring compelete status10 Reg offset */ 
    ET_RING_C_STAT_11        = 0x72C,   /**< ring compelete status11 Reg offset */ 
    ET_RING_C_STAT_12        = 0x730,   /**< ring compelete status12 Reg offset */ 
    ET_RING_C_STAT_13        = 0x734,   /**< ring compelete status13 Reg offset */ 
    ET_RING_C_STAT_14        = 0x738,   /**< ring compelete status14 Reg offset */ 
    ET_RING_C_STAT_15        = 0x73C,   /**< ring compelete status15 Reg offset */ 

    ET_RING_AUTO_PUSH_MASK_0 = 0x800,   /**< AE autopush ring mask0 Reg offset */ 
    ET_RING_AUTO_PUSH_MASK_1 = 0x804,   /**< AE autopush ring mask1 Reg offset */ 
    
    ET_RING_AUTO_PUSH_DEST_0 = 0x808,   /**< AE autopush destination configuration0 Reg offset */ 
    ET_RING_AUTO_PUSH_DEST_1 = 0x80C,   /**< AE autopush destination configuration1 Reg offset */ 
        
    ET_RING_AUTO_PUSH_DELAY_0= 0x810,   /**< AE autopush delay timer configuration0 Reg offset */ 
    ET_RING_AUTO_PUSH_DELAY_1= 0x814,   /**< AE autopush delay timer configuration1 Reg offset */ 

    ET_RING_IA_INT_EN_0      = 0x900,   /**< IA ring flag interrupt enable Reg0 offset */ 
    ET_RING_IA_INT_EN_1      = 0x904,   /**< IA ring flag interrupt enable Reg1 offset */ 

    ET_RING_IA_INT_REG_0     = 0x908,   /**< IA ring flag interrupt Reg0 offset */ 
    ET_RING_IA_INT_REG_1     = 0x90C,   /**< IA ring flag interrupt Reg1 offset */ 

    ET_RING_IA_INT_SRCSEL_0  = 0x910,   /**< IA ring flag interrupt source select Reg0 offset */ 
    ET_RING_IA_INT_SRCSEL_1  = 0x914,   /**< IA ring flag interrupt source select Reg1 offset */ 
    ET_RING_IA_INT_SRCSEL_2  = 0x918,   /**< IA ring flag interrupt source select Reg2 offset */ 
    ET_RING_IA_INT_SRCSEL_3  = 0x91C,   /**< IA ring flag interrupt source select Reg3 offset */ 
    ET_RING_IA_INT_SRCSEL_4  = 0x920,   /**< IA ring flag interrupt source select Reg4 offset */ 
    ET_RING_IA_INT_SRCSEL_5  = 0x924,   /**< IA ring flag interrupt source select Reg5 offset */ 
    ET_RING_IA_INT_SRCSEL_6  = 0x928,   /**< IA ring flag interrupt source select Reg6 offset */ 
    ET_RING_IA_INT_SRCSEL_7  = 0x92C,   /**< IA ring flag interrupt source select Reg7 offset */ 

    ET_RING_IA_INT_COL_EN_0  = 0x930,   /**< coalesced IA interrupt enable Reg0 offset */ 
    ET_RING_IA_INT_COL_EN_1  = 0x934,   /**< coalesced IA interrupt enable Reg1 offset */ 

    ET_RING_IA_INT_COL_CTL_0 = 0x938,   /**< coalesced IA interrupt control Reg0 offset */ 
    ET_RING_IA_INT_COL_CTL_1 = 0x93C,   /**< coalesced IA interrupt control Reg1 offset */ 

    ET_RING_NPE_INT_EN_0     = 0xA00,   /**< NPE ring flag interrupt enable Reg0 offset */ 
    ET_RING_NPE_INT_EN_1     = 0xA04,   /**< NPE ring flag interrupt enable Reg1 offset */ 
    
    ET_RING_NPE_INT_REG_0    = 0xA08,   /**< NPE ring flag interrupt Reg0 offset */ 
    ET_RING_NPE_INT_REG_1    = 0xA0C,   /**< NPE ring flag interrupt Reg1 offset */ 

    ET_RING_NPE_INT_SRCSEL_0 = 0xA10,   /**< NPE ring flag interrupt source select Reg0 offset */ 
    ET_RING_NPE_INT_SRCSEL_1 = 0xA14,   /**< NPE ring flag interrupt source select Reg1 offset */ 
    ET_RING_NPE_INT_SRCSEL_2 = 0xA18,   /**< NPE ring flag interrupt source select Reg2 offset */ 
    ET_RING_NPE_INT_SRCSEL_3 = 0xA1C,   /**< NPE ring flag interrupt source select Reg3 offset */ 
    ET_RING_NPE_INT_SRCSEL_4 = 0xA20,   /**< NPE ring flag interrupt source select Reg4 offset */ 
    ET_RING_NPE_INT_SRCSEL_5 = 0xA24,   /**< NPE ring flag interrupt source select Reg5 offset */ 
    ET_RING_NPE_INT_SRCSEL_6 = 0xA28,   /**< NPE ring flag interrupt source select Reg6 offset */ 
    ET_RING_NPE_INT_SRCSEL_7 = 0xA2C    /**< NPE ring flag interrupt source select Reg7 offset */ 
}EAGLETAIL_RING_CSRS;

/* ET ring near empty/full watermark definition */
typedef enum{
    ET_RING_NEAR_WATERMARK_0         = 0x00,  /**< 0 byte watermark */ 
    ET_RING_NEAR_WATERMARK_4         = 0x01,  /**< 4 bytes watermark */ 
    ET_RING_NEAR_WATERMARK_8         = 0x02,  /**< 8 bytes watermark */ 
    ET_RING_NEAR_WATERMARK_16        = 0x03,  /**< 16 bytes watermark */ 
    ET_RING_NEAR_WATERMARK_32        = 0x04,  /**< 32 bytes watermark */ 
    ET_RING_NEAR_WATERMARK_64        = 0x05,  /**< 64 bytes watermark */ 
    ET_RING_NEAR_WATERMARK_128       = 0x06,  /**< 128 bytes watermark */ 
    ET_RING_NEAR_WATERMARK_256       = 0x07,  /**< 256 bytes watermark */ 
    ET_RING_NEAR_WATERMARK_512       = 0x08,  /**< 512 bytes watermark */ 
    ET_RING_NEAR_WATERMARK_KILO_1    = 0x09,  /**< 1K bytes watermark */ 
    ET_RING_NEAR_WATERMARK_KILO_2    = 0x0A,  /**< 2K bytes watermark */ 
    ET_RING_NEAR_WATERMARK_KILO_4    = 0x0B,  /**< 4K bytes watermark */ 
    ET_RING_NEAR_WATERMARK_KILO_8    = 0x0C,  /**< 8K bytes watermark */ 
    ET_RING_NEAR_WATERMARK_KILO_16   = 0x0D,  /**< 16K bytes watermark */ 
    ET_RING_NEAR_WATERMARK_KILO_32   = 0x0E,  /**< 32K bytes watermark */     
    ET_RING_NEAR_WATERMARK_KILO_64   = 0x0F,  /**< 64K bytes watermark */     
    ET_RING_NEAR_WATERMARK_KILO_128  = 0x10,  /**< 128K bytes watermark */     
    ET_RING_NEAR_WATERMARK_KILO_256  = 0x11   /**< 256K bytes watermark */                
}EAGLETAIL_RING_NEAR_WATERMARK;

/* ET ring access mode definition */
typedef enum{
    ET_RING_MMIO_ACCESS         = 0x00,      /**< memory maped I/O mode */ 
    ET_RING_SHAREDMEM_ACCESS    = 0x01       /**< shared memory mode */ 
}EAGLETAIL_RING_IA_ACCESS_METHOD;

/* ET ring watermark definition to bytes size conversion */
#define EAGLETAIL_RING_NEAR_WATERMARK_TO_BYTES(wm) ((wm == 0) ? 0 : (4 << (wm-1)))

/* ET ring size definition */
typedef enum{
    ET_RINGSIZE_64             = 0x00,       /**< 64 bytes ET ring */ 
    ET_RINGSIZE_128            = 0x01,       /**< 128 bytes ET ring */ 
    ET_RINGSIZE_256            = 0x02,       /**< 256 bytes ET ring */ 
    ET_RINGSIZE_512            = 0x03,       /**< 512 bytes ET ring */ 
    ET_RINGSIZE_KILO_1         = 0x04,       /**< 1K bytes ET ring */ 
    ET_RINGSIZE_KILO_2         = 0x05,       /**< 2K bytes ET ring */ 
    ET_RINGSIZE_KILO_4         = 0x06,       /**< 4K bytes ET ring */ 
    ET_RINGSIZE_KILO_8         = 0x07,       /**< 8K bytes ET ring */ 
    ET_RINGSIZE_KILO_16        = 0x08,       /**< 16K bytes ET ring */ 
    ET_RINGSIZE_KILO_32        = 0x09,       /**< 32K bytes ET ring */ 
    ET_RINGSIZE_KILO_64        = 0x0A,       /**< 64K bytes ET ring */ 
    ET_RINGSIZE_KILO_128       = 0x0B,       /**< 128K bytes ET ring */ 
    ET_RINGSIZE_KILO_256       = 0x0C,       /**< 256K bytes ET ring */ 
    ET_RINGSIZE_KILO_512       = 0x0D,       /**< 512K bytes ET ring */ 
    ET_RINGSIZE_MEG_1          = 0x0E,       /**< 1M bytes ET ring */ 
    ET_RINGSIZE_MEG_2          = 0x0F,       /**< 2M bytes ET ring */ 
    ET_RINGSIZE_MEG_4          = 0x10        /**< 4M bytes ET ring */
}EAGLETAIL_RING_SIZE;

/* ET ring size definition to bytes size conversion */
#define EAGLETAIL_RING_SIZE_TO_BYTES(size) (64 << (size))

/* Define Interrupt types and type masks */
#define HALAE_INTR_ATTN_BKPT   0    /**< AE attention interrupt */                            
#define HALAE_INTR_ATTN_PARITY 1    /**< AE attention parity interrupt */
#define HALAE_INTR_THD_A       2    /**< AE thread_A interrupt */
#define HALAE_INTR_THD_B       3    /**< AE thread_B interrupt */
#define HALAE_INTR_ATTN_BKPT_MASK    (1 << HALAE_INTR_ATTN_BKPT)   /**< AE attention interrupt mask */
#define HALAE_INTR_ATTN_PARITY_MASK  (1 << HALAE_INTR_ATTN_PARITY) /**< AE attention parity interrupt mask */
#define HALAE_INTR_THD_A_MASK        (1 << HALAE_INTR_THD_A)       /**< AE thread_A interrupt mask */
#define HALAE_INTR_THD_B_MASK        (1 << HALAE_INTR_THD_B)       /**< AE thread_B interrupt mask */

#define MAX_THD_GRP 2
typedef struct Hal_IntrMasks_S {
    unsigned int attn_bkpt_mask;    /**< Mask of AEs */
    unsigned int attn_parity_mask;  /**< Mask of AEs */
    unsigned int thd_a_mask[MAX_THD_GRP];    /**< Mask of Threads */
    unsigned int thd_b_mask[MAX_THD_GRP];    /**< Mask of Threads */
} Hal_IntrMasks_T;
    
typedef void (*HalAeIntrCallback_T)(Hal_IntrMasks_T *masks, void* data);

/* Define New Page callback mechanism */
typedef enum {               /**< Argument0        Argument1       */
    START_OF_PAGE_CHANGE,    /**< N/A              N/A             */
    NEW_PAGE_LOADED,         /**< new page num     old page num    */
    END_OF_PAGE_CHANGE,      /**< N/A              N/A             */
    WAITING_FOR_PAGE_CHANGE  /**< ctx              Virt Addr or -1 */
} Hal_PageChangeReason_T;
typedef void (*HalAePageChangeCallback_T)(Hal_PageChangeReason_T reason,
                                          unsigned int           hwAeNum,
                                          unsigned int           arg0,
                                          unsigned int           arg1,
                                          void*                  user_data);

typedef enum {              /**< Argument0   Argument1            */
    UPDATE_AE_ENABLES,      /**< AeMask      ptr to AeEnablesMask */
    PAUSING_AES             /**< AeMask                           */
} Hal_UcloCallReason_T;
typedef void (*HalAe_UcloCall_T)(Hal_UcloCallReason_T reason,
                                 unsigned int         arg0,
                                 unsigned int*        arg1,
                                 void*                user_data);

#define MAX_SRAM_CHANNELS 1
#define MAX_DRAM_CONFIG 2

typedef struct Hal_SramDesc_S{
    unsigned int sramSize;      /**< size of sram */
    unsigned int sramOffset;    /**< sram offset relative to the beg of the channel */
} Hal_SramDesc_T;

typedef struct Hal_DramConfig_S{
    unsigned int dramBaseAddr;  /**< dram base address */
    unsigned int dramSize;      /**< total dram size */
    unsigned int aeDramSize;    /**< dram available to AE */
    unsigned int aeDramOffset;  /**< offset to the ae-dram relative to beg of dram */ 
} Hal_DramConfig_T;


typedef struct Hal_SysMemInfo_S {
    unsigned int        prodId;         /**< product ID and revisions as per PRM */
    unsigned int        aeClkMhz;       /**< AE clock speed in mega-hertz */
    unsigned int        strapOptions;   /**< board strap options */
    unsigned int        numDramDesc;    /**< num dram configurations */
    Hal_DramConfig_T    dramDesc[MAX_DRAM_CONFIG]; /**< dram descriptor */
    unsigned int        numSramChan;    /**< num sram channels */
    Hal_SramDesc_T      sramChan[MAX_SRAM_CHANNELS]; /**< sram descriptor */
} Hal_SysMemInfo_T;

#ifdef __cplusplus
extern "C" {
#endif

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Initializes the acceleration engines and takes them out of reset. 
 *      acceleration engines with their corresponding bit set in aeMask are initialized
 *      to the following state:
 *      1. Context mode is set to eight
 *      2. Program counters are set to zero
 *      3. Next context to run is set to zero
 *      4. All contexts are disabled
 *      5. cc_enable is set to 0x2000
 *      6. Wakeup events are set to one
 *      7. The signal event is set to zero
 *      All other acceleration engines remain untouched thought they are taken out of 
 *      reset. This function should be called prior to calling most of the HAL
 *      functions.
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param aeMask - IN The bits correspond to the accelaration engine number and address
 *                    of acceleration engines to be initialized. For example, the first
 *                    accelaration engine in the second cluster corresponds to bit 16.
 *                    write the value uWord at the address uAddr.
 *
 * @retval HALAE_SUCCESS Operation was successful
 * @retval HALAE_FAIL Operation failed
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC int 
halAe_Init(unsigned int aeMask);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Restores the system resources allocated by the halAe_Init() function. 
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param - none
 *
 * @retval - none
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC void 
halAe_DelLib(void);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Determines if the specified accelaration engine is valid, whether it is 
 *      initialized, or whether it is in or out of reset.
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param ae - IN Specifies the accelaration engine of interest
 *
 * @retval HALAE_UNINIT A bad debug library is associated with the specified
 *                      accelaration engine the library is not initialized
 * @retval HALAE_CLR_RST The accelaration engine is not in a reset state
 * @retval HALAE_RST The accelaration engine is in reset state
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC int 
halAe_GetAeState(unsigned char ae);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Set a mask of the contexts that are loaded.
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param ae - IN Specifies the accelaration engine of interest
 * @param ctxMask - IN The context or contexts to set to alive
 *
 * @retval HALAE_BADLIB Bad HAL library which wasn't initialized
 * @retval HALAE_SUCCESS Operation was successful
 * @retval HALAE_BADARG Bad function argument
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC int 
halAe_SetLiveCtx(unsigned char ae, unsigned int ctxMask);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Get a mask of the contexts that are loaded.
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param ae - IN Specifies the accelaration engine of interest
 * @param ctxMask - OUT A pointer referencec to the alive context or contexts
 *
 * @retval HALAE_BADLIB Bad HAL library which wasn't initialized
 * @retval HALAE_SUCCESS Operation was successful
 * @retval HALAE_BADARG Bad function argument
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC int 
halAe_GetLiveCtx(unsigned char ae, unsigned int *ctxMask);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Defines a region of microstore that is unused. 
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param ae - IN Specifies the accelaration engine of interest
 * @param begFreeAddr - IN Specifies the microstore address where the free 
                        region begins
 * @param size - IN Indicates the number of free microstore words
 *
 * @retval HALAE_BADLIB Bad HAL library which wasn't initialized
 * @retval HALAE_SUCCESS Operation was successful
 * @retval HALAE_BADARG Bad function argument
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC int 
halAe_SetUstoreFreeMem(unsigned char ae, 
                                        unsigned int begFreeAddr, 
                                        unsigned int size);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Returns the starting address and size of the specified accelaration engine 
 *      microstore free region
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param ae - IN Specifies the accelaration engine of interest
 * @param begFreeAddr - OUT A pointer to the location of the beginning of 
 *                          the free region for the specified accelaration engine 
 *                          microstore.                        
 * @param size - OUT A pointer to the location of the size of the free region
 *                   for the specified accelaration engine microstore
 *
 * @retval HALAE_BADLIB Bad HAL library which wasn't initialized
 * @retval HALAE_SUCCESS Operation was successful
 * @retval HALAE_BADARG Bad function argument
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC int 
halAe_GetUstoreFreeMem(unsigned char ae, 
                                        unsigned int *begFreeAddr, 
                                        unsigned int *size);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Determines if the specified accelaration engine is enabled to run and if any
 *      of its contexts are either running or waiting to run
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param ae - IN Specifies the accelaration engine of interest
 *
 * @retval HALAE_BADLIB Bad HAL library which wasn't initialized
 * @retval HALAE_ENABLED Acceleration engine enabled
 * @retval HALAE_DISABLED accelaration engine or interrupt disabled 
 * @retval HALAE_BADLIB A bad debug library is associated with the specified
                        accelaration engine the library is not initialized
 * @retval HALAE_RESET Acceleration engine is in reset
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC int 
halAe_IsAeEnabled(unsigned char ae);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Sets the wake-event to voluntary and starts the accelaration engine context 
 *      specified by the ae and startCtx parameters from that accelaration engine's 
 *      current program counter. The ctxEnMask specifies the contexts to be 
 *      enabled. If one of these acceleration engines is in reset it is taken out 
 *      of reset.
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param ae - IN Specifies the accelaration engine of interest
 * @param ctxEnMask - IN Specifies the contexts to be enabled
 *
 * @retval HALAE_BADLIB Bad HAL library which wasn't initialized
 * @retval HALAE_SUCCESS Operation was successful
 * @retval HALAE_BADLIB A bad debug library is associated with the specified
                        accelaration engine the library is not initialized
 * @retval HALAE_BADARG Bad function argument
 * 
 *****************************************************************************/

HAL_DECLSPEC int 
halAe_Start(unsigned char ae, unsigned int ctxEnMask);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Stops the accelaration engine contexts that have a corresponding bit 
 *      set in the ctxMask parameter at the next context arbitration 
 *      instruction. The context may not stop because it never executes a 
 *      context arbitration instruction. A value of HALAE_RESET is returned 
 *      if the accelaration engine is in reset.
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param ae - IN Specifies the accelaration engine of interest
 * @param ctxEnMask - IN Specifies the contexts to stop
 *
 * @retval HALAE_BADLIB Bad HAL library which wasn't initialized
 * @retval HALAE_SUCCESS Operation was successful
 * @retval HALAE_FAIL Operation failed
 * @retval HALAE_RESET Acceleration engine is in reset
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC int 
halAe_Stop(unsigned char ae, unsigned int ctxMask);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Resets the specified acceleration engines with a corresponding bit set in 
 *      aeMask. If clrReg is set then the acceleration engines are initialized to 
 *      the states described in halAe_Init().
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param aeMask - IN Specifies the acceleration engines of interest
 * @param clrReg - IN If this parameter is set, then following register
 *                    initialization is performed:
 *                    1. CTX_ENABLES are set to zero
 *                    2. CTX_ARB_CNTL is set to zero
 *                    3. CC_ENABLE is set to x2000
 *                    4. All context program counters are set to zero
 *                    5. WAKEUP_EVENTS are set to one
 *                    6. SIG_EVENTS are set to zero
 *
 * @retval - none
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC void 
halAe_Reset(unsigned int aeMask, int clrReg);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Takes the specified acceleration engines out of the reset state.
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param aeMask - IN Specifies the acceleration engines of interest
 *
 * @retval - none
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC void 
halAe_ClrReset(unsigned int aeMask);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Reads the accelaration engine context arbitration control register and returns
 *      the value in the ctxArbCtl argument.
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param ae - IN Specifies the accelaration engine of interest
 * @param ctxArbCtl - OUT A pointer to the location of the specified context 
 *                        arbitration control register
 *
 * @retval HALAE_SUCCESS Operation was successful
 * @retval HALAE_BADLIB Bad HAL library which wasn't initialized
 * @retval HALAE_BADARG Bad function argument  
 *
 * 
 *****************************************************************************/

HAL_DECLSPEC int 
halAe_GetCtxArb(unsigned char ae, unsigned int *ctxArbCtl);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Writes a longword ctxArbCtl value to the accelaration engine context 
 *      arbitration control register for the specified accelaration engine.
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param ae - IN Specifies the accelaration engine of interest
 * @param ctxArbCtl - IN The new value for the context arbitration 
 *                       control register
 *
 * @retval HALAE_SUCCESS Operation was successful
 * @retval HALAE_BADLIB Bad HAL library which wasn't initialized
 * @retval HALAE_BADARG Bad function argument  
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC int 
halAe_PutCtxArb(unsigned char ae, unsigned int ctxArbCtl);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Reads the accelaration engine control status register indicated by csr and 
 *      returns the value in the value parameter. The csr value must be a valid
 *      accelaration engine CSR offset.
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param ae - IN Specifies the accelaration engine of interest
 * @param csr - IN A valid accelaration engine CSR offset
 * @param value - OUT A pointer to the location of the requested accelaration engine 
 *                    control status register
 *
 * @retval HALAE_SUCCESS Operation was successful
 * @retval HALAE_BADLIB Bad HAL library which wasn't initialized
 * @retval HALAE_BADARG Bad function argument  
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC int 
halAe_GetAeCsr(unsigned char ae, 
               unsigned int csr, 
               unsigned int *value);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Writes the longword value to the accelaration engine CSR indicated by csr 
 *      parameter. The csr value must be a valid AE CSR offset.
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param ae - IN Specifies the accelaration engine of interest
 * @param csr - IN A valid accelaration engine CSR offset
 * @param value - IN The new value for the specified accelaration engine control status
 *
 * @retval HALAE_SUCCESS Operation was successful
 * @retval HALAE_BADLIB Bad HAL library which wasn't initialized
 * @retval HALAE_BADARG Bad function argument  
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC int 
halAe_PutAeCsr(unsigned char ae, 
               unsigned int csr, 
               unsigned int value);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Reads the PCI device' control status register indicated by csr and 
 *      returns the value in the data parameter.
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param ae - IN Specifies the accelaration engine of interest
 * @param offset - IN A valid CSR offset
 * @param numBytes - IN Specifies the number of bytes to read
 * @param data - OUT A pointer to the location of the control status register
 *
 * @retval 0 Success Operation was successful
 * @retval others Failure
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC int 
halAe_GetPciCsr(unsigned char ae,
                Hal_CSR_T type, 
                unsigned int offset, 
                unsigned int numBytes, 
                unsigned int *data);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Writes the PCI device' control status register indicated by csr.
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param ae - IN Specifies the accelaration engine of interest
 * @param offset - IN A valid CSR offset
 * @param numBytes - IN Specifies the number of bytes to read
 * @param data - IN The new value for the specified control status register
 *
 * @retval 0 Success Operation was successful
 * @retval others Failure
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC int 
halAe_PutPciCsr(unsigned char ae,
                Hal_CSR_T type, 
                unsigned int offset, 
                unsigned int numBytes, 
                unsigned int data);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Sets the local-memory mode to relative or global.
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param ae - IN Specifies the accelaration engine of interest
 * @param lmType - IN Specifies the local memory bank. 
                   This value is ICP_LMEM0 or ICP_LMEM1
 * @param mode - IN Specifies the local memory mode. This value is one of:
 *               1. zero - the memory mode is relative
 *               2. one - the memory mode is global
 *
 * @retval HALAE_SUCCESS Operation was successful
 * @retval HALAE_BADLIB Bad HAL library which wasn't initialized
 * @retval HALAE_BADARG Bad function argument  
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC int 
halAe_PutAeLmMode(unsigned char ae, 
                  icp_RegType_T lmType, 
                  unsigned char mode);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Sets the accelaration engine context mode to either four or eight
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param ae - IN Specifies the accelaration engine of interest
 * @param mode - IN Specifies the context mode. This value must be one of {4, 8}
 *
 * @retval HALAE_SUCCESS Operation was successful
 * @retval HALAE_BADLIB Bad HAL library which wasn't initialized
 * @retval HALAE_BADARG Bad function argument  
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC int 
halAe_PutAeCtxMode(unsigned char ae, unsigned char mode);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Sets the accelaration engine next-neighbor mode to either write to itself or to
 *      its neighbor.
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param ae - IN Specifies the accelaration engine of interest
 * @param mode - IN The next-neighbor mode to set. This value is one of:
 *                  1. zero - the next-neighbor mode is write to self
 *                  2. one - the next-neighbor mode is write to neighbor
 *
 * @retval HALAE_SUCCESS Operation was successful
 * @retval HALAE_BADLIB Bad HAL library which wasn't initialized
 * @retval HALAE_BADARG Bad function argument  
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC int 
halAe_PutAeNnMode(unsigned char ae, unsigned char mode);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Set the shared ustore mode on or off for the specified AE and its 
 *      neighbor's.
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param ae - IN Specifies the accelaration engine of interest
 * @param mode - IN Specifies the mode to set
 *
 * @retval HALAE_SUCCESS Operation was successful
 * @retval HALAE_BADLIB Bad HAL library which wasn't initialized
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC int 
halAe_PutAeSharedCsMode(unsigned char ae, unsigned char mode);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Determines if the specified accelaration engine is valid, whether it is 
 *      initialized, or whether it is in or out of reset.
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param ae - IN Specifies the accelaration engine of interest
 * @param mode - OUT A pointer to shared control store mode
 *
 * @retval HALAE_SUCCESS Operation was successful
 * @retval HALAE_BADLIB Bad HAL library which wasn't initialized
 * @retval HALAE_BADARG Bad function argument  
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC int 
halAe_GetAeSharedCsMode(unsigned char ae, 
                        unsigned char *mode);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Get the neighbor of a shareable-ustore pair.
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param ae - IN Specifies the accelaration engine of interest
 * @param meNeigh - OUT A pointer to neighboring accelaration engine
 *
 * @retval HALAE_SUCCESS Operation was successful
 * @retval HALAE_BADLIB Bad HAL library which wasn't initialized
 * @retval HALAE_BADARG Bad function argument  
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC int
halAe_GetSharedUstoreNeigh(unsigned char ae, 
                           unsigned char *meNeigh);


/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Reads the state of the accelaration engine context wakeup-event signals.
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param ae - IN Specifies the accelaration engine of interest
 * @param ctx - IN Specifies the accelaration engine context of interest
 * @param events - OUT A pointer to the location of the wakeup-event signals 
 *                     for the specified acceleration engines and context
 *
 * @retval HALAE_SUCCESS Operation was successful
 * @retval HALAE_BADLIB Bad HAL library which wasn't initialized
 * @retval HALAE_BADARG Bad function argument  
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC int 
halAe_GetCtxWakeupEvents(unsigned char ae, 
                         unsigned char ctx, 
                         unsigned int *events);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Writes the longword events to the accelaration engine context wakeup-events 
 *      CSR. Only the context with a corresponding bit set in ctxMask are 
 *      written; unspecified contexts remains unchanged.
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param ae - IN Specifies the accelaration engine of interest
 * @param ctx - IN Specifies the accelaration engine context of interest
 * @param events - OUT The longword specifying the new wakeup-event signals
 *
 * @retval HALAE_SUCCESS Operation was successful
 * @retval HALAE_BADLIB Bad HAL library which wasn't initialized
 * @retval HALAE_BADARG Bad function argument  
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC int 
halAe_PutCtxWakeupEvents(unsigned char ae, 
                         unsigned int ctxMask, 
                         unsigned int events);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Reads the signal events for the specified accelaration engine context
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param ae - IN Specifies the accelaration engine of interest
 * @param ctx - IN Specifies the accelaration engine context of interest.
 * @param events - IN A pointer to the location of the current signal events 
 *                    for the specified accelaration engine context
 *
 * @retval HALAE_SUCCESS Operation was successful
 * @retval HALAE_BADLIB Bad HAL library which wasn't initialized
 * @retval HALAE_BADARG Bad function argument  
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC int 
halAe_GetCtxSigEvents(unsigned char ae, 
                      unsigned char ctx, 
                      unsigned int *events);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Writes the longword value of the event parameter to the signal events 
 *      of the specified accelaration engine contexts.
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param ae - IN Specifies the accelaration engine of interest
 * @param ctxMask - IN Specifies the accelaration engine context or contexts
 * @param events - IN The longword specifying the new signal events
 *
 * @retval HALAE_SUCCESS Operation was successful
 * @retval HALAE_BADLIB Bad HAL library which wasn't initialized
 * @retval HALAE_BADARG Bad function argument  
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC int 
halAe_PutCtxSigEvents(unsigned char ae, 
                      unsigned int ctxMask, 
                      unsigned int events);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Writes the context-status CSR of the contexts specified by the ctxMask 
 *      parameter.The accelaration engine must be stopped before calling this function.
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param ae - IN Specifies the accelaration engine to write to
 * @param ctxMask - IN Specifies the context or contexts to write to.
 * @param ctxStatus - IN Specifies the longword value to write
 *
 * @retval HALAE_SUCCESS Operation was successful
 * @retval HALAE_BADLIB Bad HAL library which wasn't initialized
 * @retval HALAE_BADARG Bad function argument  
 * @retval HALAE_AEACTIVE The accelaration engine was active and the call failed
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC int 
halAe_PutCtxStatus(unsigned char ae, 
                   unsigned int ctxMask, 
                   unsigned int ctxStatus);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Reads the context-status CSR for the context specified by the ctx 
 *      parameter.The accelaration engine must be stopped before calling this function.
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param ae - IN Specifies the accelaration engine to write to
 * @param ctxMask - IN Specifies the context or contexts to write to.
 * @param ctxStatus - OUT A pointer to the location at which to return the 
 *                        longword value read
 *
 * @retval HALAE_SUCCESS Operation was successful
 * @retval HALAE_BADLIB Bad HAL library which wasn't initialized
 * @retval HALAE_BADARG Bad function argument  
 * @retval HALAE_AEACTIVE The accelaration engine was active and the call failed
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC int 
halAe_GetCtxStatus(unsigned char ae, 
                   unsigned char ctx, 
                   unsigned int *ctxStatus);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Writes a 32-bit value to the accelaration-engine context(s) indirect CSR. 
 *      It's unsafe to call this function while the AE is enabled.
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param ae - IN Specifies the accelaration engine of interest
 * @param ctxMask - IN Specifies the contexts CSR to be written
 * @param aeCsr - IN The indirect CSR to be written. Must be one of the
 *                following:
 *                1. CTX_FUTURE_COUNT_INDIRECT
 *                2. CTX_WAKEUP_EVENTS_INDIRECT
 *                3. CTX_STS_INDIRECT
 *                4. CTX_SIG_EVENTS_INDIRECT
 *                5. LM_ADDR_0_INDIRECT
 *                6. LM_ADDR_1_INDIRECT
 * @param csrVal - IN Specifies the longword value to write
 *
 * @retval HALAE_SUCCESS Operation was successful
 * @retval HALAE_BADLIB Bad HAL library which wasn't initialized
 * @retval HALAE_BADARG Bad function argument  
 * @retval HALAE_AEACTIVE The accelaration engine was active and the call failed
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC int 
halAe_PutCtxIndrCsr(unsigned char ae, 
                    unsigned int ctxMask, 
                    unsigned int aeCsr, 
                    unsigned int csrVal);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Reads a 32-bit value to the accelaration-engine context(s) indirect CSR. 
 *      It's unsafe to call this function while the AE is enabled.
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param ae - IN Specifies the accelaration engine of interest
 * @param ctxMask - IN Specifies the contexts CSR to be written
 * @param aeCsr - IN The indirect CSR to be written. Must be one of the
 *                following:
 *                1. CTX_FUTURE_COUNT_INDIRECT
 *                2. CTX_WAKEUP_EVENTS_INDIRECT
 *                3. CTX_STS_INDIRECT
 *                4. CTX_SIG_EVENTS_INDIRECT
 *                5. LM_ADDR_0_INDIRECT
 *                6. LM_ADDR_1_INDIRECT
 * @param csrVal - IN Specifies the longword value to write
 *
 * @retval HALAE_SUCCESS Operation was successful
 * @retval HALAE_BADLIB Bad HAL library which wasn't initialized
 * @retval HALAE_BADARG Bad function argument  
 * @retval HALAE_AEACTIVE The accelaration engine was active and the call failed
 * @retval HALAE_FAIL Operation failed
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC int 
halAe_GetCtxIndrCsr(unsigned char ae, 
                    unsigned char ctx, 
                    unsigned int aeCsr, 
                    unsigned int *csrVal);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Get the CPU Type and product maj/min revisions, and return
 *      the CPU type, and the combined maj and min revision.
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param prodType - IN A pointer to the location of product type
 * @param prodRev - IN A pointer to the location of product revision
 *
 * @retval HALAE_SUCCESS Operation was successful
 * @retval HALAE_BADLIB Bad HAL library which wasn't initialized
 * @retval HALAE_BADARG Bad function argument  
 * @retval HALAE_FAIL Operation failed
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC int 
halAe_GetProdInfo(unsigned int *prodType, 
                  unsigned int *prodRev);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Sets the indicated accelaration engine context program counters to the value 
 *      specified by the upc parameter.
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param ae - IN Specifies the accelaration engine of interest
 * @param ctxMask - IN Specifies the accelaration engine context or contexts
 * @param upc - IN The new program counter value
 *
 * @retval HALAE_SUCCESS Operation was successful
 * @retval HALAE_BADLIB Bad HAL library which wasn't initialized
 * @retval HALAE_BADARG Bad function argument  
 * @retval HALAE_AEACTIVE The accelaration engine was active and the call failed
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC int 
halAe_PutPC(unsigned char ae, 
            unsigned int ctxMask, 
            unsigned int upc);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Returns the program counter for the specified accelaration engine context
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param ae - IN Specifies the accelaration engine of interest
 * @param ctx - IN Specifies the accelaration engine context of interest
 * @param upc - OUT A pointer to the location of the requested program 
 *                  counter value
 *
 * @retval HALAE_SUCCESS Operation was successful
 * @retval HALAE_BADLIB Bad HAL library which wasn't initialized
 * @retval HALAE_BADARG Bad function argument  
 * @retval HALAE_AEACTIVE The accelaration engine was active and the call failed
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC int 
halAe_GetPC(unsigned char ae, 
            unsigned char ctx, 
            unsigned int *upc);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Writes a number of microword to the specified accelaration engine. The 
 *      accelaration engine must be disabled and out of reset before writing to its 
 *      microstore.
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param ae - IN Specifies the accelaration engine of interest
 * @param uWordAddr - IN The microstore address specifying the start of the 
 *                       write operation
 * @param numWords - IN The number of microwords to write
 * @param uWord - IN A pointer to the location of the microword to write
 *
 * @retval HALAE_SUCCESS Operation was successful
 * @retval HALAE_BADLIB Bad HAL library which wasn't initialized
 * @retval HALAE_BADARG Bad function argument  
 * @retval HALAE_AEACTIVE The accelaration engine was active and the call failed
 * @retval HALAE_RESET Acceleration engine is in reset
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC int
halAe_PutUwords(unsigned char ae, 
                unsigned int uWordAddr, 
                unsigned int numWords, 
                uword_T *uWord);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Reads and returns the number of microword from the specified 
 *      accelaration engine microstore. acceleration engines must be disabled before reading 
 *      from the microstore.
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param ae - IN Specifies the accelaration engine of interest
 * @param uWordAddr - IN The microstore address specifying the start of the 
 *                       write operation
 * @param numWords - IN The number of microwords to write
 * @param uWord - OUT A pointer to the location of the requested microwords
 *
 * @retval HALAE_SUCCESS Operation was successful
 * @retval HALAE_BADLIB Bad HAL library which wasn't initialized
 * @retval HALAE_BADARG Bad function argument  
 * @retval HALAE_AEACTIVE The accelaration engine was active and the call failed
 * @retval HALAE_RESET Acceleration engine is in reset
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC int 
halAe_GetUwords(unsigned char ae, 
                unsigned int uWordAddr, 
                unsigned int numWords, 
                uword_T *uWord);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      writes a number of longwords to the microstore. The accelaration engine must 
 *      be inactive, possibly under reset, before writing to the microstore.
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param ae - IN Specifies the accelaration engine of interest
 * @param uWordAddr - IN The microstore address specifying the start of the 
 *                       write operation
 * @param numWords - IN The number of microwords to write
 * @param data - IN A pointer to the location of the longwords to write
 *
 * @retval HALAE_SUCCESS Operation was successful
 * @retval HALAE_BADLIB Bad HAL library which wasn't initialized
 * @retval HALAE_BADARG Bad function argument  
 * @retval HALAE_AEACTIVE The accelaration engine was active and the call failed
 * @retval HALAE_RESET Acceleration engine is in reset
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC int 
halAe_PutUmem(unsigned char ae, 
              unsigned int uWordAddr, 
              unsigned int numWords, 
              unsigned int *data);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      reads a number of longwords to the microstore memory specified by the 
 *      uAddr word-address. The accelaration engine must be inactive before reading 
 *      from microstore.
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param ae - IN Specifies the accelaration engine of interest
 * @param uWordAddr - IN The microstore address specifying the start of the 
 *                       write operation
 * @param numWords - IN The number of microwords to write
 * @param data - OUT A pointer to the location of the requested longwords
 *
 * @retval HALAE_SUCCESS Operation was successful
 * @retval HALAE_BADLIB Bad HAL library which wasn't initialized
 * @retval HALAE_BADARG Bad function argument  
 * @retval HALAE_AEACTIVE The accelaration engine was active and the call failed
 * @retval HALAE_RESET Acceleration engine is in reset
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC int 
halAe_GetUmem(unsigned char ae, 
              unsigned int uWordAddr, 
              unsigned int numWords, 
              unsigned int *data);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Initialize a eagletail ring.
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param ringNum - IN Specifies the eagletail ring number
 * @param nfwm - IN Specifies the near full watermark to set
 * @param newm - IN Specifies the near empty watermark to set
 * @param size - IN Specifies the eagletail ring size
 * @param baseAddr - IN The ram address specifying the start of the ring
 *
 * @retval HALAE_SUCCESS Operation was successful
 * @retval HALAE_BADLIB Bad HAL library which wasn't initialized
 * @retval HALAE_BADARG Bad function argument  
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC int 
halAe_ETRingInit(unsigned int ringNum, 
                 EAGLETAIL_RING_NEAR_WATERMARK nfwm, 
                 EAGLETAIL_RING_NEAR_WATERMARK newm, 
                 EAGLETAIL_RING_SIZE size, 
                 uint64 baseAddr);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Set ring type to specified eagletail ring.
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param ringNum - IN Specifies the eagletail ring number
 * @param type - IN Specifies the eagletail ring type. Only RINGTYPE_IAPUTGET
 *                  is valid.
 *
 * @retval HALAE_SUCCESS Operation was successful
 * @retval HALAE_BADLIB Bad HAL library which wasn't initialized
 * @retval HALAE_BADARG Bad function argument  
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC int 
halAe_ETRingSetType(unsigned int ringNum, RING_TYPE type);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Put data to specified eagleTail ring
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param ringNum - IN Specifies the ring number
 * @param in_data - IN A pointer to the location of the data to write
 * @param count - IN/OUT A pointer to the location of the number of longwords 
 *                to write and write successfully within this call
 * @param mode - IN Specified the ring access mode
 * @param count_available - OUT A pointer to the location of available 
 *                          put sapce
 *
 * @retval HALAE_SUCCESS Operation was successful
 * @retval HALAE_RINGFULL Specified ring is full
 * @retval HALAE_BADLIB Bad HAL library which wasn't initialized
 * @retval HALAE_BADARG Bad function argument  
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC int 
halAe_ETRingPut(unsigned int ringNum, 
                unsigned int *in_data, 
                unsigned int *count, 
                EAGLETAIL_RING_IA_ACCESS_METHOD mode,
                unsigned int *count_available);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Get data from specified eagleTail ring
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param ringNum - IN Specifies the ring number
 * @param in_data - OUT A pointer to the location of the data to read from
 * @param count - IN/OUT A pointer to the location of the number of longwords 
 *                to read and read successfully within this call
 * @param mode - IN Specified the ring access mode
 * @param count_available - OUT A pointer to the location of available 
 *                          get sapce
 *
 * @retval HALAE_SUCCESS Operation was successful
 * @retval HALAE_RINGEMPTY Specified ring is empty
 * @retval HALAE_BADLIB Bad HAL library which wasn't initialized
 * @retval HALAE_BADARG Bad function argument  
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC int 
halAe_ETRingGet(unsigned int ringNum, 
                unsigned int *out_data, 
                unsigned int *count, 
                EAGLETAIL_RING_IA_ACCESS_METHOD mode,
                unsigned int *count_available);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Reads a longword from the relative accelaration engine general purpose register
 *      specified by the ae, ctx, regType, and regNum parameters. The 
 *      accelaration engine must be disabled before calling this function.
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param ae - IN Specifies the accelaration engine of interest
 * @param ctx - IN Specifies the context
 * @param regType - IN Specifies the type of register to read from. This must 
 *                  be one of {ICP_GPA_REL,ICP_GPB_REL, ICP_DR_RD_REL, 
 *                  ICP_SR_RD_REL, ICP_DR_WR_REL,
 *                  ICP_SR_WR_REL, ICP_NEIGH_REL}.
 * @param regNum - IN Specifies the register number to read from. The register 
 *                 number is relative to the context. Therefore, this number 
 *                 is in the range of 0 through max - 1, where max is the 
 *                 maximum number of context-relative registers for the type 
 *                 specified by regType argument and the context mode
 *
 * @param regData - OUT A pointer to the location of the data to read
 *
 * @retval HALAE_SUCCESS Operation was successful
 * @retval HALAE_FAIL Operation failed
 * @retval HALAE_BADLIB Bad HAL library which wasn't initialized
 * @retval HALAE_BADARG Bad function argument  
 * @retval HALAE_ENABLED Acceleration engine enabled
 * @retval HALAE_RESET Acceleration engine is in reset
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC int 
halAe_GetRelDataReg(unsigned char ae, 
                    unsigned char ctx, 
                    icp_RegType_T regType,
                    unsigned short regNum, 
                    unsigned int *regData);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Writes a longword to the relative accelaration engine general purpose register
 *      specified by the ae, ctx, regType, and regNum parameters. The 
 *      accelaration engine must be disabled before calling this function.
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param ae - IN Specifies the accelaration engine of interest
 * @param ctx - IN Specifies the context
 * @param regType - IN Specifies the type of register to read from. This must 
 *                  be one of {ICP_GPA_REL,ICP_GPB_REL, ICP_DR_RD_REL, 
 *                  ICP_SR_RD_REL, ICP_DR_WR_REL,
 *                  ICP_SR_WR_REL, ICP_NEIGH_REL}.
 * @param regNum - IN Specifies the register number to read from. The register 
 *                 number is relative to the context. Therefore, this number 
 *                 is in the range of 0 through max - 1, where max is the 
 *                 maximum number of context-relative registers for the type 
 *                 specified by regType argument and the context mode
 * @param regData - IN The longword to write
 *
 * @retval HALAE_SUCCESS Operation was successful
 * @retval HALAE_FAIL Operation failed
 * @retval HALAE_BADLIB Bad HAL library which wasn't initialized
 * @retval HALAE_BADARG Bad function argument  
 * @retval HALAE_ENABLED Acceleration engine enabled
 * @retval HALAE_RESET Acceleration engine is in reset
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC int 
halAe_PutRelDataReg(unsigned char ae, 
                    unsigned char ctx, 
                    icp_RegType_T regType,
                    unsigned short regNum, 
                    unsigned int regData);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Reads a longword from the accelaration engine absolute general purpose register
 *      specified by the ae, ctx, regType, and regNum parameters. The 
 *      accelaration engine must be disabled before calling this function.
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param ae - IN Specifies the accelaration engine of interest
 * @param regType - IN Specifies the type of register to read from. This must 
 *                  be one of {ICP_GPA_ABS, ICP_GPB_ABS, ICP_DR_RD_ABS, 
 *                  ICP_SR_RD_ABS, ICP_DR_WR_ABS,
 *                  ICP_SR_WR_ABS, ICP_NEIGH_ABS}.
 * @param absRegNum - IN Specifies the register number to read from
 * @param regData - OUT A pointer to the location of the data to read
 *
 * @retval HALAE_SUCCESS Operation was successful
 * @retval HALAE_FAIL Operation failed
 * @retval HALAE_BADLIB Bad HAL library which wasn't initialized
 * @retval HALAE_BADARG Bad function argument  
 * @retval HALAE_ENABLED Acceleration engine enabled
 * @retval HALAE_RESET Acceleration engine is in reset
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC int 
halAe_GetAbsDataReg(unsigned char ae, 
                    icp_RegType_T regType,
                    unsigned short absRegNum, 
                    unsigned int *regData);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Writes a longword to the accelaration engine absolute general purpose register
 *      specified by the ae, regType, and regNum parameters. The 
 *      accelaration engine must be disabled before calling this function.
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param ae - IN Specifies the accelaration engine of interest
 * @param regType - IN Specifies the type of register to read from. This must 
 *                  be one of {ICP_GPA_ABS, ICP_GPB_ABS, ICP_DR_RD_ABS, 
 *                  ICP_SR_RD_ABS, ICP_DR_WR_ABS,
 *                  ICP_SR_WR_ABS, ICP_NEIGH_ABS}.
 * @param absRegNum - IN Specifies the register number to write
 * @param regData - IN The longword to write
 *
 * @retval HALAE_SUCCESS Operation was successful
 * @retval HALAE_FAIL Operation failed
 * @retval HALAE_BADLIB Bad HAL library which wasn't initialized
 * @retval HALAE_BADARG Bad function argument  
 * @retval HALAE_ENABLED Acceleration engine enabled
 * @retval HALAE_RESET Acceleration engine is in reset
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC int 
halAe_PutAbsDataReg(unsigned char ae, 
                    icp_RegType_T regType,
                    unsigned short absRegNum, 
                    unsigned int regData);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Read a generic data register of any type.
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param ae - IN Specifies the accelaration engine of interest
 * @param ctx - IN Specifies the context
 * @param regType - IN Specifies the register type
 * @param regAddr - IN Specifies the register address offset
 * @param regData - IN A pointer to the location of the data to read
 *
 * @retval HALAE_SUCCESS Operation was successful
 * @retval HALAE_FAIL Operation failed
 * @retval HALAE_BADLIB Bad HAL library which wasn't initialized
 * @retval HALAE_BADARG Bad function argument  
 * @retval HALAE_ENABLED Acceleration engine enabled
 * @retval HALAE_RESET Acceleration engine is in reset
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC int 
halAe_GetDataReg(unsigned char hwAeNum, 
                 unsigned char ctx,
                 icp_RegType_T regType,
                 unsigned short regAddr, 
                 unsigned int *regData);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Writes a longword value to the specified accelaration engine's local memory 
 *      location specified by the lmaddr word address.
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param ae - IN Specifies the accelaration engine of interest
 * @param lmAddr - IN Local memory word address
 * @param value - IN The longword to write
 *
 * @retval HALAE_SUCCESS Operation was successful
 * @retval HALAE_BADLIB Bad HAL library which wasn't initialized
 * @retval HALAE_BADARG Bad function argument  
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC int
halAe_PutLM(unsigned char ae, 
            unsigned short lmAddr, 
            unsigned int value);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Reads a longword value from the specified accelaration engine local memory 
 *      location specified by the lmAddr word address.
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param ae - IN Specifies the accelaration engine of interest
 * @param lmAddr - IN Local memory word address
 * @param value - IN A pointer to the location of location of the longword 
 *                read from the specified register
 *
 * @retval HALAE_SUCCESS Operation was successful
 * @retval HALAE_BADLIB Bad HAL library which wasn't initialized
 * @retval HALAE_BADARG Bad function argument  
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC int 
halAe_GetLM(unsigned char ae, 
            unsigned short lmAddr, 
            unsigned int *value);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Writes the longword data to the specified accelaration engine CAM entry and 
 *      marks it as the most recently used. The entry and state must be in 
 *      the range of zero through fifteen.
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param ae - IN Specifies the accelaration engine of interest
 * @param entry - IN A pointer to the location of the CAM entry ID
 * @param data - IN The CAM data to write
 * @param state - IN The value of the state for the specified CAM entry
 *
 * @retval HALAE_SUCCESS Operation was successful
 * @retval HALAE_BADLIB Bad HAL library which wasn't initialized
 * @retval HALAE_BADARG Bad function argument  
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC int 
halAe_PutCAM(unsigned char ae, 
             unsigned char entry, 
             unsigned int data,
             unsigned char state);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Reads the entire contents of the accelaration engine CAM, that is, sixteen CAM 
 *      entries and stores the tag value and status in the appropriate parameters.
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param ae - IN Specifies the accelaration engine of interest
 * @param tags - OUT An array of 16 unsigned integers to receive the CAM 
 *                   tag-data of each CAM entry
 * @param state - OUT An array of 16 chars to receive the CAM state-bits of 
 *                    each CAM entry
 * @param lru - OUT A pointer to the location of the array describing the 
 *                  order of use of the CAM entries with values of zero through 
 *                  fifteen. The entry with a zero value is the Least Recently Used. 
 *                  The entry with the value of fifteen is the Most Recently Used.
 *
 * @retval HALAE_SUCCESS Operation was successful
 * @retval HALAE_BADLIB Bad HAL library which wasn't initialized
 * @retval HALAE_BADARG Bad function argument  
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC int 
halAe_GetCAMState(unsigned char ae, 
                  unsigned int *tags,
                  unsigned char *state, 
                  unsigned char *lru);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Determines if the accelaration engine is in the state specified by inpState 
 *      and returns a value indicating whether it is set, not set, or an error 
 *      occurred. There are 16 values for inpState, and their meaning is 
 *      specific to the chip implementation. 
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param ae - IN Specifies the accelaration engine of interest
 * @param inpState - IN A chip-specific value denoting a accelaration engine state
 * 
 * @retval HALAE_ISSET The value is set
 * @retval HALAE_NOTSET The value is not set
 * @retval HALAE_BADLIB Bad HAL library which wasn't initialized
 * @retval HALAE_BADARG Bad function argument  
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC int 
halAe_IsInpStateSet(unsigned char ae, unsigned char inpState);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Checks that the value specified by the ae parameter is a valid 
 *      accelaration engine number. The library must be initialized before calling 
 *      this function.
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param ae - IN Specifies the accelaration engine of interest
 *
 * @retval HALAE_SUCCESS Operation was successful
 * @retval HALAE_BADLIB Bad HAL library which wasn't initialized
 * @retval HALAE_BADARG Bad function argument  
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC int 
halAe_VerifyAe(unsigned char ae);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Checks that the value specified by the ae parameter is a valid 
 *      accelaration engine number. The library must be initialized before calling 
 *      this function.
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param aeMask - IN Specifies the acceleration engines of interest
 *
 * @retval HALAE_SUCCESS Operation was successful
 * @retval HALAE_BADLIB Bad HAL library which wasn't initialized
 * @retval HALAE_BADARG Bad function argument  
 * 
 * 
 *****************************************************************************/
HAL_DECLSPEC int 
halAe_VerifyAeMask(unsigned int aeMask);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Stops the timestamp clock and zeroes the timestamps of all specified 
 *      acceleration engines then restarts the timestamp clock.
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param aeMask - IN Specifies one or more acceleration engines
 *
 * @retval HALAE_SUCCESS Operation was successful
 * @retval HALAE_BADLIB Bad HAL library which wasn't initialized
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC int 
halAe_ResetTimestamp(unsigned int aeMask);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Returns the system product Id, memory size, and other system 
 *      information in the Hal_SysMemInfo_T structure.
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param sysMemInfo - OUT A pointer to Hal_SysMemInfo_T structure storing
 *                         system information
 *
 * @retval HALAE_SUCCESS Operation was successful
 * @retval HALAE_BADLIB Bad HAL library which wasn't initialized
 * @retval HALAE_BADARG Bad function argument  
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC int 
halAe_GetSysMemInfo(Hal_SysMemInfo_T *sysMemInfo);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Waits until one or more of the interrupts specified by type_mask 
 *      occurs, or returns immediately if there are any outstanding interrupts 
 *      of the specified type. Interrupts must be enabled by halAe_IntrEnable() 
 *      or else any attempt to poll returns HALAE_DISABLED.
 *        
 *      Upon return, the mask parameter contains a bit mask of the acceleration engines 
 *      or threads which have an interrupt of the type or types specified in 
 *      the poll request. The masks may be zero in the case of a 
 *      halAe_IntrDisable() request. The interrupt is cleared before the call 
 *      returns.

 *      In the event that multiple threads were blocked on the same interrupt, 
 *      when that interrupt occurs, only one poll call completes. There is no 
 *      guarantee which of the calls completes.
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param type_mask - IN Specifies the mask type must be one or more of the 
 *                       following values {HALAE_INTR_ATTN_BKPT_MASK, 
 *                       HALAE_INTR_ATTN_PARITY_MASK,
 *                       HALAE_INTR_THD_A_MASK, 
 *                       HALAE_INTR_THD_B_MASK}
 * @param masks - OUT On input specifies the location where the mask is to be
                     returned. On output the returned mask
 *
 * @retval HALAE_SUCCESS Operation was successful
 * @retval HALAE_DISABLED accelaration engine or interrupt disabled 
 * @retval HALAE_FAIL Operation failed
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC int 
halAe_IntrPoll(unsigned int type_mask,      
               Hal_IntrMasks_T  *masks);    

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Enables breakpoint, parity, or thread interrupts, and enables interrupt 
 *      processing by the HAL. If the HAL interrupt processing is not enabled, 
 *      then the application is free to manage the particular interrupts in 
 *      an OS dependent manner.
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param type_mask - IN Specifies the mask type must be one or more of the 
 *                       following values {HALAE_INTR_ATTN_BKPT_MASK, 
 *                       HALAE_INTR_ATTN_PARITY_MASK,
 *                       HALAE_INTR_THD_A_MASK, 
 *                       HALAE_INTR_THD_B_MASK}
 *
 * @retval HALAE_SUCCESS Operation was successful
 * @retval system error number
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC int 
halAe_IntrEnable(unsigned int type_mask);          

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Disables breakpoint, parity, or thread interrupts.
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param type_mask - IN Specifies the mask type must be one or more of the 
 *                       following values {HALAE_INTR_ATTN_BKPT_MASK, 
 *                       HALAE_INTR_ATTN_PARITY_MASK,
 *                       HALAE_INTR_THD_A_MASK, 
 *                       HALAE_INTR_THD_B_MASK}
 *
 * @retval HALAE_SUCCESS Operation was successful
 * @retval system error number
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC int 
halAe_IntrDisable(unsigned int type_mask);         

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      This function is similar to halAe_IntrPoll() except that it does not 
 *      block waiting for the specified interrupts. Instead, it returns 
 *      immediately and invokes the specified callback function for subsequent
 *      occurrences of the interrupt or interrupts. The callback data are 
 *      passed to the callback function on invocation. A handle associated 
 *      with  this call is returned and this handle can be used to terminate
 *      the request and disregard the specified interrupts.
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param type_mask - IN Specifies the mask type must be one or more of the 
 *                       following values {HALAE_INTR_ATTN_BKPT_MASK, 
 *                       HALAE_INTR_ATTN_PARITY_MASK,
 *                       HALAE_INTR_THD_A_MASK, 
 *                       HALAE_INTR_THD_B_MASK}
 * @param callback_func - IN A pointer to the location of a callback function.
 * @param callback_data - IN A pointer to the location of application-specific 
 *                           data. This pointer is passed to the callback 
 *                           function when it is invoked
 * @param thd_priority - IN The thread priority
 * @param handle - IN A handle associated with and returned by this call. 
 *                    This handle can be used to terminate the request and 
 *                    disregard the specified interrupts
 *
 * @retval HALAE_SUCCESS Operation was successful
 * @retval HALAE_FAIL Operation failed
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC int 
halAe_SpawnIntrCallbackThd(unsigned int type_mask,
                           HalAeIntrCallback_T callback_func,
                           void*               callback_data,
                           int                 thd_priority,
                           void*              *handle);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      This function is similar to halAe_IntrPoll() except that it does not 
 *      block waiting for the specified interrupts. Instead, it returns 
 *      immediately and invokes the specified callback function for subsequent
 *      occurrences of the interrupt or interrupts. The callback data are 
 *      passed to the callback function on invocation. A handle associated 
 *      with  this call is returned and this handle can be used to terminate
 *      the request and disregard the specified interrupts.
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param type_mask - IN Specifies the mask type must be one or more of the 
 *                       following values {HALAE_INTR_ATTN_BKPT_MASK, 
 *                       HALAE_INTR_ATTN_PARITY_MASK,
 *                       HALAE_INTR_THD_A_MASK, 
 *                       HALAE_INTR_THD_B_MASK}
 * @param callback_func - IN A pointer to the location of a callback function
 * @param callback_data - IN A pointer to the location of application-specific 
 *                           data. This pointer is passed to the callback 
 *                           function when it is invoked
 * @param thd_priority - IN The thread priority
 * @param callback_priority - IN The callback priority
 * @param handle - IN A handle associated with and returned by this call. 
 *                    This handle can be used to terminate the request and 
 *                    disregard the specified interrupts
 *
 * @retval HALAE_SUCCESS Operation was successful
 * @retval HALAE_FAIL Operation failed
 * 
 * 
 *****************************************************************************/

/* halAe_SpawnIntrCallbackThdEx() is used when chaining interrupts.
   Higher priority callbacks are made first */
HAL_DECLSPEC int 
halAe_SpawnIntrCallbackThdEx(unsigned int type_mask,
                             HalAeIntrCallback_T callback_func,
                             void*               callback_data,
                             int                 thd_priority,
                             unsigned int        callback_priority,
                             void*               *handle);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Cancels a request initiated by the halAe_SpawnIntrCallbackThd() 
 *      function.
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param handle - IN The handle returned by a call to 
 *                    halAe_SpawnIntrCallbackThd(). This handle
 *                    specifies which request to terminate.
 *
 * @retval HALAE_SUCCESS Operation was successful
 * @retval HALAE_FAIL Operation failed
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC int 
halAe_TerminateCallbackThd(void* handle);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Register new page callback function.
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param callback_func - IN A pointer to the location of a callback function
 * @param user_data - IN A pointer to user input callback function data
 *
 * @retval - none
 * 
 * 
 *****************************************************************************/

/* The New Page Callback is defined globally and not on a per-chip basis
   (when running under simulation) */
HAL_DECLSPEC void
halAe_DefineNewPageCallback(HalAePageChangeCallback_T callback_func,
                            void*                     user_data);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Calls new page callback function.
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param reason - IN Specifies the reason why callback function is invoked. 
 *                    Must be one of the following values
 *                    {START_OF_PAGE_CHANGE, NEW_PAGE_LOADED,
 *                     END_OF_PAGE_CHANGE, WAITING_FOR_PAGE_CHANGE} 
 *                      
 * @param hwAeNum - IN Specifies the accelaration engine of interest
 * @param new_page_num - IN Specifies the new page number
 * @param old_page_num - IN Specifies the old page number
 *
 * @retval - none
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC void 
halAe_CallNewPageCallback(Hal_PageChangeReason_T reason,
                          unsigned int           hwAeNum,
                          unsigned int           new_page_num,
                          unsigned int           old_page_num);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Defines UCLO callback function.
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param callback_func - IN A pointer to the location of a callback function
 * @param user_data - IN A pointer to user input callback function data
 *
 * @retval - none
 * 
 * 
 *****************************************************************************/

/* The UCLO callbacks can be defined multiple times. Each one is called in an
   arbitrary order */
HAL_DECLSPEC void
halAe_DefineUcloCallback(HalAe_UcloCall_T callback_func,
                         void*            user_data);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Deletes UCLO callback function.
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param callback_func - IN A pointer to the location of a callback function
 * @param user_data - IN A pointer to user input callback function data
 *
 * @retval - none
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC void
halAe_DeleteUcloCallback(HalAe_UcloCall_T callback_func,
                         void*            user_data);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Calls UCLO callback function.
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param reason - IN Specifies the reason why callback function is invoked. 
 *                    Must be one of the following values
 *                    {UPDATE_AE_ENABLES, PAUSING_AES,
 *                     END_OF_PAGE_CHANGE, WAITING_FOR_PAGE_CHANGE} 
 * @param arg0 - IN Callback function argument 
 * @param arg1 - IN Callback function argument 
 *
 * @retval - none
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC void 
halAe_CallUcloCallback(Hal_UcloCallReason_T reason,
                       unsigned int         arg0,
                       unsigned int*        arg1);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Convert from a virtual to a physical uword address.
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param hwAeNum - IN Specifies the accelaration engine of interest
 * @param v_addr - IN Specifies the uword virtual address
 * @param p_addr - OUT A pointer to uwrod physical address
 * @param loaded - OUT A pointer to indicate wether or not the address is 
 *                     currently in ustore
 *
 * @retval HALAE_SUCCESS Operation was successful
 * @retval HALAE_BADADDR Bad address
 * 
 * 
 *****************************************************************************/

/* In halAe_MapVirtToPhysUaddr(), the value returned in loaded is a boolean
   flag, either zero or non-zero */
HAL_DECLSPEC int 
halAe_MapVirtToPhysUaddr(unsigned char hwAeNum,
                         unsigned int  v_addr,
                         unsigned int *p_addr,
                         unsigned int *loaded);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Convert from a physical to a virtual uword address.
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param hwAeNum - IN Specifies the accelaration engine of interest
 * @param p_addr - IN Specifies the uword physical address
 * @param v_addr - OUT A pointer to uwrod virtual address
 * @param loaded - OUT A pointer to indicate wether or not the address is 
 *                     currently in ustore
 *
 * @retval HALAE_SUCCESS Operation was successful
 * @retval HALAE_BADADDR Bad address
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC int 
halAe_MapPhysToVirtUaddr(unsigned char hwAeNum,
                         unsigned int  p_addr,
                         unsigned int *v_addr);

/* halAe_SetNumPages() and halAe_SetPageData() are designed to be used
   internally by the loader */
int 
halAe_SetNumPages(unsigned char hwAeNum, unsigned int numPages);

int 
halAe_SetPageData(unsigned char hwAeNum, 
                  unsigned int page,
                  unsigned int vaddr, 
                  unsigned int paddr, 
                  unsigned int size);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Convert from a virtual to a physical uword address.
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param hwAeNum - IN Specifies the accelaration engine of interest
 * @param v_addr - IN Specifies the uword virtual address
 * @param p_addr - OUT A pointer to uwrod physical address
 * @param loaded - OUT A pointer to indicate wether or not the address is 
 *                     currently in ustore
 * @param page_num - OUT A pointer to indicate page number associated with 
 *                       the physical address
 *
 * @retval HALAE_SUCCESS Operation was successful
 * @retval HALAE_BADADDR Bad address
 * 
 * 
 *****************************************************************************/

int halAe_MapVirtToPhysUaddrEx(unsigned char hwAeNum,
                               unsigned int  v_addr,
                               unsigned int *p_addr,
                               unsigned int *loaded,
                               unsigned int *page_num);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Translates an IA physical address to a virtual address.
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param phyAddr - IN Specifies the physical address
 *
 * @retval the virtual address or 0xffffffff if the address cannot be translated
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC unsigned int 
halAe_GetVirAddr(unsigned int phyAddr);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Return the starting virtual address of the specified physical 
 *      region (phyXaddr+phySize) if it's mapped by the HAL, otherwise, an 
 *      invalid address (0xffffffffffffffff) is returned.
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param phyAddr - IN Specifies the physical start address of region
 * @param phyAddr - IN Specifies the size of region
 *
 * @retval The virtual address, or 0xffffffffffffffff if the address is not 
 *         mapped by the HAL library
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC uint64 
halAe_GetVirXaddr(uint64 phyXaddr, unsigned int phySize);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Return the starting physical address of the specified virtual region 
 *      (virtXaddr+virtSize) if it was mapped by the HAL, otherwise, an invalid 
 *      address (0xffffffffffffffff) is returned.
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param virXaddr - IN Specifies the virtual start address of region
 * @param virtSize - IN Specifies the size of region
 *
 * @retval The virtual address, or 0xffffffffffffffff if the address is not 
 *         mapped by the HAL library
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC uint64 
halAe_GetPhyXaddr(uint64 virXaddr, unsigned int virtSize);

/* internal use only */
HAL_DECLSPEC int 
halAe_SetReloadUstore(unsigned char ae, 
                      unsigned int reloadSize, 
                      int sharedMode, 
                      unsigned int ustoreDramBuf);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Write a number of long-words to micro-store memory specified such that
 *      the even address goes to the even numbered AE, and the odd address 
 *      goes to the odd numbered AE.
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param ae - IN Specifies the accelaration engine of interest
 * @param uAddr - IN Specifies the microstore address to start of the 
 *                   write operation
 * @param numWords - IN Specifies The number of microwords to write
 * @param uWord - IN A pointer to the location of the microwords to write
 *
 * @retval HALAE_SUCCESS Operation was successful
 * @retval HALAE_BADLIB Bad HAL library which wasn't initialized
 * @retval HALAE_BADARG Bad function argument
 * @retval HALAE_AEACTIVE The accelaration engine was active and the call failed
 * @retval HALAE_RESET Acceleration engine is in reset 
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC int 
halAe_PutCoalesceUwords(unsigned char ae, 
                        unsigned int uAddr,
                        unsigned int numWords, 
                        uword_T *uWord);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Read a number of long-words from micro-store memory such that the 
 *      ven address is taken from even numbered AE, and the odd address is 
 *      taken from the odd numbered AE.
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param ae - IN Specifies the accelaration engine of interest
 * @param uAddr - IN Specifies the microstore address to start of the 
 *                   write operation
 * @param numWords - IN Specifies The number of microwords to write
 * @param uWord - OUT A pointer to the location of the microwords to read
 *
 * @retval HALAE_SUCCESS Operation was successful
 * @retval HALAE_BADLIB Bad HAL library which wasn't initialized
 * @retval HALAE_BADARG Bad function argument
 * @retval HALAE_AEACTIVE The accelaration engine was active and the call failed
 * @retval HALAE_RESET Acceleration engine is in reset 
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC int 
halAe_GetCoalesceUwords(unsigned char ae, 
                        unsigned int uAddr,
                        unsigned int numWords, 
                        uword_T *uWord);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Reads a longword value from specified Gige control and status register.
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param mac - IN Specifies the MAC number
 * @param csr - IN Specified Gige CSR offset
 * @param value - OUT A pointer to the location of the data to read
 *
 * @retval HALAE_SUCCESS Operation was successful
 * @retval HALAE_BADLIB Bad HAL library which wasn't initialized
 * @retval HALAE_BADARG Bad function argument  
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC int 
halAe_GetGigeCsr(unsigned char ae,
                 unsigned int mac, 
                 unsigned int csr, 
                 unsigned int *value);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Writes a longword value to specified Gige control and status register.
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param mac - IN Specifies the MAC number
 * @param csr - IN Specified Gige CSR offset
 * @param value - IN A data to to write
 *
 * @retval HALAE_SUCCESS Operation was successful
 * @retval HALAE_BADLIB Bad HAL library which wasn't initialized
 * @retval HALAE_BADARG Bad function argument  
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC int 
halAe_PutGigeCsr(unsigned char ae,
                 unsigned int mac, 
                 unsigned int csr, 
                 unsigned int value);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Reads a long-word value from the SSU Share Ram location. It's unsafe to 
 *      call this function while the AE is enabled.
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param ae - IN Specifies the accelaration engine of interest
 * @param addr - IN Specified shared ram address index
 * @param value - OUT A point to the location of data to read
 *
 * @retval HALAE_SUCCESS Operation was successful
 * @retval HALAE_BADLIB Bad HAL library which wasn't initialized
 * @retval HALAE_BADARG Bad function argument  
 * @retval HALAE_AEACTIVE The accelaration engine was active and the call failed
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC int 
halAe_GetSharedRam(unsigned char ae, 
                   unsigned int addr, 
                   unsigned int *value);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Write a long-word value to the SSU Share Ram location. It's unsafe to 
 *      call this function while the AE is enabled.
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param ae - IN Specifies the accelaration engine of interest
 * @param addr - IN Specified shared ram address index
 * @param value - IN A data to write
 *
 * @retval HALAE_SUCCESS Operation was successful
 * @retval HALAE_BADLIB Bad HAL library which wasn't initialized
 * @retval HALAE_BADARG Bad function argument  
 * @retval HALAE_AEACTIVE The accelaration engine was active and the call failed
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC int
halAe_PutSharedRam(unsigned char ae, 
                   unsigned int addr, 
                   unsigned int value);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Get coherent dram physical base address.
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param baseAddr - OUT A point to the location of base address to read
 *
 * @retval HALAE_SUCCESS Operation was successful
 * @retval HALAE_FAIL Operation failed
 * @retval HALAE_UNINIT A bad debug library is associated with the specified
                        accelaration engine the library is not initialized
 * @retval HALAE_CLR_RST The accelaration engine is not in a reset state
 * @retval HALAE_RST The accelaration engine is in reset state
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC int 
halAe_GetCDramBaseAddr(unsigned int *baseAddr);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Get non-coherent dram physical base address.
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param baseAddr - OUT A point to the location of base address to read
 *
 * @retval HALAE_SUCCESS Operation was successful
 * @retval HALAE_FAIL Operation failed
 * @retval HALAE_UNINIT A bad debug library is associated with the specified
                        accelaration engine the library is not initialized
 * @retval HALAE_CLR_RST The accelaration engine is not in a reset state
 * @retval HALAE_RST The accelaration engine is in reset state
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC int
halAe_GetNCDramBaseAddr(unsigned int *baseAddr);

/* internal used */
HAL_DECLSPEC int 
halAe_SetMemoryStartOffset(unsigned int scratchOffset, 
                           unsigned int sramOffset, 
                           unsigned int ncdramOffset, 
                           unsigned int cdramOffset);

/* internal used */
HAL_DECLSPEC int 
halAe_GetMemoryStartOffset(unsigned int *scratchOffset, 
                           unsigned int *sramOffset, 
                           unsigned int *ncdramOffset, 
                           unsigned int *cdramOffset);

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Get the error description string for provided error code.
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param errCode - IN Specifies the error code
 *
 * @retval error description string
 * 
 * 
 *****************************************************************************/

HAL_DECLSPEC char 
*halAe_GetErrorStr(int errCode);

#ifdef __cplusplus
}
#endif

#endif          /* __HAL_AEAPI_H */

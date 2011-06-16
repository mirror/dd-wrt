/**
 **************************************************************************
 * @file halAeDrv.hxx
 *
 * @description
 *      This file provides implementation of Ucode AE Library 
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

#ifndef __HALAEDRV_HXX
#define __HALAEDRV_HXX

#include "halAe_platform.h"
#include "halMmap.hxx"
#include "halMmap.h"
#include "hal_global.h"
#include "IxOsal.h"
#include "halAeApi.h"

/* memory size definition */
#define KILO_1    (0x400)              /* 1K bytes size */
#define KILO_2    (KILO_1 << 1)        /* 2K bytes size */
#define KILO_4    (KILO_1 << 2)        /* 4K bytes size */
#define KILO_8    (KILO_1 << 3)        /* 8K bytes size */
#define KILO_16   (KILO_1 << 4)        /* 16K bytes size */
#define KILO_32   (KILO_1 << 5)        /* 32K bytes size */
#define KILO_64   (KILO_1 << 6)        /* 64K bytes size */
#define KILO_128  (KILO_1 << 7)        /* 128K bytes size */
#define KILO_256  (KILO_1 << 8)        /* 256K bytes size */

#define MEG_1     (0x100000)           /* 1M bytes size */
#define MEG_2     (MEG_1 << 1)         /* 2M bytes size */
#define MEG_4     (MEG_1 << 2)         /* 4M bytes size */
#define MEG_8     (MEG_1 << 3)         /* 8M bytes size */
#define MEG_16    (MEG_1 << 4)         /* 16M bytes size */
#define MEG_32    (MEG_1 << 5)         /* 32M bytes size */
#define MEG_64    (MEG_1 << 6)         /* 64M bytes size */
#define MEG_128   (MEG_1 << 7)         /* 128M bytes size */
#define MEG_256   (MEG_1 << 8)         /* 256M bytes size */
#define MEG_192   (0x0c000000)         /* 192M bytes size */

#define MAX_SRAM_CHAN 1                /* maximum sram channels */
#define MAX_DRAM_CHAN 2                /* maximum dram channels */

#define DEFAULT_ICP_FREQUENCY  400         /* default AE frequency */
#define SRAM_BASE_ADDRESS      0xFFFC0000  /* sram base address */

/* ICP_DEVICE_CONFIG CSR offset */
#define ICP_DEVICE_CONFIG          0x40
/* SWSKU CSR offset */
#define SWSKU                      0x44
/* bit position in ICP_DEVICE_CONFIG CSR */
#define AESPD_BITPOS               31
#define AESPDFUSE_BITPOS           30

typedef struct aeDrv_sramDesc_S{
    unsigned int chipSize;      /**< chip size */
    unsigned int numChips;      /**< numpber of sram chips */
    unsigned int sramSize;      /**< Size of SRAM */
    unsigned int sramBase;      /**< sram base address */
} aeDrv_srChanDesc_T;

typedef struct aeDrv_dramDesc_S{
    unsigned int dramBaseAddr;  /**< dram base address */
    unsigned int aeDramTotSize; /**< Total Size of DRAM */
    unsigned int aeDramSize;    /**< Size of DRAM chunk owned by AEs */
    unsigned int aeDramBase;    /**< Offset from start of DRAM to area owned by AEs */
} aeDrv_drChanDesc_T;


typedef struct aeDrv_SysMemInfo_S {
    unsigned int prodId;        /**< product ID */
    unsigned int valid;         /**< valid flag */
    unsigned int aeClkMhz;      /**< AE clock speed in mega-hertz */
    unsigned int strapOptions;  /**< board strap options */

    int osDramSize;             /**< Size of DRAM owned by OS */
    int totDramSize;            /**< Size of OS and AE DRAM chunks added */

    int numDramChan;            /**< Number of DRAM channels */
    aeDrv_drChanDesc_T dramChan[MAX_DRAM_CHAN]; /**< DRAM channel descriptors */

    int numSramChan;                            /**< Number of SRAM channels */
    aeDrv_srChanDesc_T sramChan[MAX_SRAM_CHAN]; /**< SRAM channel descriptors */
} aeDrv_SysMemInfo_T;

typedef struct{
    uint64 base_addr;  /**< base address */
    unsigned int size; /**< memory region size */
    uint64 virt_addr;  /**< base virtual address */       
}pci_memory_region_T;

typedef struct{
    SPINLOCK_T lock; 
   
    pci_memory_region_T ae_cluster_cap_bridge_reg_region; /**< AE_CLUSTER CAP bar descriptor */
    pci_memory_region_T ae_cluster_scratch_mem_region;    /**< AE_CLUSTER SCRATCH bar descriptor */
    pci_memory_region_T ae_cluster_ae_tran_reg_region;    /**< AE_CLUSTER AE bar descriptor */
    pci_memory_region_T ae_cluster_ssu_reg_region;        /**< AE_CLUSTER SSU bar descriptor */
    pci_memory_region_T ring_cntl_reg_region;             /**< RING_CONTROLLER REG bar descriptor */
    pci_memory_region_T ring_cntl_get_put_region;         /**< RING_CONTROLLER IO bar descriptor */ 
    pci_memory_region_T ring_cntl_sram_region;            /**< RING_CONTROLLER SRAM bar descriptor */
    pci_memory_region_T ring_cntl_cdram_region;           /**< RING_CONTROLLER CDRAM bar descriptor */
    pci_memory_region_T ring_cntl_ncdram_region;          /**< RING_CONTROLLER NCDRAM bar descriptor */
    pci_memory_region_T ae_cplx_cntl_pmu_region;          /**< AE_CPLX PMU bar descriptor */
    pci_memory_region_T ae_fastaccess_reg_region;         /**< FAST_ACCESS REG bar descriptor */
}pci_device_bar_T;

/* CAP CSR offset definition */
#define CAP_IA_INTERRUPT_A_OFFSET      0xB20                     /* IA interrupt A csr offset */
#define CAP_IA_INTERRUPT_B_OFFSET      0xB24                     /* IA interrupt A csr offset */
#define CAP_THD_INTERRUPT_A_OFFSET     0xB30                     /* thread interrupt A csr offset */ 
#define CAP_THD_INTERRUPT_B_OFFSET     0xB50                     /* thread interrupt B csr offset */   
#define CAP_THD_ENABLE_A_OFFSET        0xB70                     /* thread interrupt A enable csr offset */ 
#define CAP_THD_ENABLE_B_OFFSET        0xB90                     /* thread interrupt B enable csr offset */ 
#define CAP_THD_ENABLE_SET_A_OFFSET    CAP_THD_ENABLE_A_OFFSET   /* thread interrupt A enable set csr offset */ 
#define CAP_THD_ENABLE_SET_B_OFFSET    CAP_THD_ENABLE_B_OFFSET   /* thread interrupt B enable set csr offset */ 
#define CAP_THD_ENABLE_CLR_A_OFFSET    0xBB0                     /* thread interrupt A enable clear csr offset */ 
#define CAP_THD_ENABLE_CLR_B_OFFSET    0xBD0                     /* thread interrupt B enable set csr offset */ 
#define CAP_RAW_ATTN_STATUS            0xA88                     /* raw attention interrupt csr offset */ 
#define CAP_ATTN_MASK                  0xA8C                     /* attention mask csr offset */ 
#define CAP_ATTN_MASK_SET              CAP_ATTN_MASK             /* attention mask set csr offset */ 
#define CAP_ATTN_MASK_CLR              0xA90                     /* attention mask clear csr offset */ 

#define THD_CSR(grp) (Hal_cap_global_ctl_csr_virtAddr + ((grp) << 2))
#define THD_CSR_ADDR(grp, csr) (THD_CSR(grp)+(0xfff & (csr)))

#define GET_THD_CSR(grp, csr) (READ_LWORD(THD_CSR_ADDR((grp),(csr))))
#define SET_THD_CSR(grp, csr, val) (WRITE_LWORD(THD_CSR_ADDR((grp),(csr)),(val)))

/* Define interrupt vector ID */
#define IRQ_THD_A    0     /* thread A interrupt ID */   
#define IRQ_THD_B    1     /* thread B interrupt ID */   
#define IRQ_AE_ATTN   2     /* AE attention interrupt ID */   
#define MAX_AE_INT    0x10  /* maximum interrupt ID */   

/* AE thread A interrupt bits */
#define AE_THREAD_A_ALLBITS        0xFFFFFFFF
/* AE thread B interrupt bits */
#define AE_THREAD_B_ALLBITS        0xFFFFFFFF
/* All AE' attention interrupt bits */
#define AE_ATTN_ALLBITS            0xFFFFFFFF

/* Define parity error bit position */
#define CE_BREAKPOINT_BITPOS              27    /* breakpoint bit position */   
#define CE_CNTL_STORE_PARITY_ERROR_BITPOS 29    /* control store parrity error bit position */   
#define CE_BREAKPOINT_BIT              (1 << CE_BREAKPOINT_BITPOS)   /* breakpoint bit */      
#define CE_CNTL_STORE_PARITY_ERROR_BIT (1 << CE_CNTL_STORE_PARITY_ERROR_BITPOS)  /* control store parrity error bit */   

#define HWID_ICP        0x80       /* ICP hardward ID  */
#define EP80579_A_MAJOR_REV  0x0   /* EP80579 A Stepping Major Revision ID */
#define EP80579_B_MAJOR_REV  0x1   /* EP80579 B Stepping Major Revision ID */
#define EP80579_A0_RID  0x00       /* EP80579 A0 Revision ID */
#define EP80579_B0_RID  0x01       /* EP80579 B0 Revision ID */
#define EP80579_B1_RID  0x02       /* EP80579 B1 Revision ID */
#define EP80579_B2_RID  0x03       /* EP80579 B2 Revision ID */

#define HALAE_VERIFY_LIB() if(!HALAE_LIBINIT) { return (HALAE_BADLIB); } /* verify HAL library */

#define HALAE_CLUSTR(ae) (((ae) & 0x10) >> 4)           /* cluster number of AE */  
#define HALAE_NUM(ae) ((ae) & AeMask)                   /* AE number within cluster */    
#define AE(ae) AEs[HALAE_CLUSTR(ae)][HALAE_NUM(ae)]     /* AE control */  

#define BITS_IN_WORD                 32                /* bits number in a word */        

typedef struct{
   unsigned int state;                     /* AE state */
   unsigned int uStoreSize;                /* micro-store size */
   unsigned int freeAddr;                  /* free micro-store address */
   unsigned int freeSize;                  /* free micro-store size */
   unsigned int liveCtxMask;               /* live context mask */
   unsigned int ustoreDramAddr;            /* micro-store dram address */
   unsigned int reloadSize;                /* reloadable code size */
   SPINLOCK_T   aeLock;                    /* AE lock */
}Ae_T; 

static const unsigned int clrMasks[BITS_IN_WORD] ={
    0xfffffffe, 0xfffffffd, 0xfffffffb, 0xfffffff7,
    0xffffffef, 0xffffffdf, 0xffffffbf, 0xffffff7f,
    0xfffffeff, 0xfffffdff, 0xfffffbff, 0xfffff7ff,
    0xffffefff, 0xffffdfff, 0xffffbfff, 0xffff7fff,
    0xfffeffff, 0xfffdffff, 0xfffbffff, 0xfff7ffff,
    0xffefffff, 0xffdfffff, 0xffbfffff, 0xff7fffff,
    0xfeffffff, 0xfdffffff, 0xfbffffff, 0xf7ffffff,
    0xefffffff, 0xdfffffff, 0xbfffffff, 0x7fffffff};

static const unsigned int setMasks[BITS_IN_WORD] ={
    0x00000001, 0x00000002, 0x00000004, 0x00000008,
    0x00000010, 0x00000020, 0x00000040, 0x00000080,
    0x00000100, 0x00000200, 0x00000400, 0x00000800,
    0x00001000, 0x00002000, 0x00004000, 0x00008000,
    0x00010000, 0x00020000, 0x00040000, 0x00080000,
    0x00100000, 0x00200000, 0x00400000, 0x00800000,
    0x01000000, 0x02000000, 0x04000000, 0x08000000,
    0x10000000, 0x20000000, 0x40000000, 0x80000000};

#define VERIFY_ARGPTR(arg) if((arg) == NULL) { return (HALAE_BADARG); }    /* verify argument pointer */
#define VERIFY_AE(ae) if(((ae) > AeMaxNum) || ((ae) >= BITS_IN_WORD) || \
                          (AeBadMask & setMasks[(ae)]) || \
                          (AE((ae)).state == HALAE_UNINIT)) \
                          return (HALAE_BADARG)                            /* verify AE */
#define VERIFY_CTX(ctx) if(ctx >= MAX_CTX) { return (HALAE_BADARG); }      /* verify context */
 
#define SPIN_LOCK_AE(ae) SPIN_LOCK(AE(ae).aeLock)                          /* lock AE resource */
#define SPIN_UNLOCK_AE(ae) SPIN_UNLOCK(AE(ae).aeLock)                      /* unlock AE resource */
    
#define GET_LIVECTXMASK(ae) AE((ae)).liveCtxMask                           /* get live context mask */
#define SET_LIVECTXMASK(ae, ctxMask) AE((ae)).liveCtxMask = ctxMask        /* set live context mask */

#define IS_RST(ae) (AE((ae)).state == HALAE_RST)                           /* check if AE in reset status */
#define SET_RST(ae) AE((ae)).state = HALAE_RST                             /* set AE in reset status */
#define CLR_RST(ae) AE((ae)).state = HALAE_CLR_RST                         /* clear AE in reset status */
#define GET_USTORE_SIZE(ae) AE((ae)).uStoreSize                            /* get micro-store size */

struct HalAeCallback_S;
typedef struct HalAeCallbackChain_S {
    struct HalAeCallbackChain_S *next;
    unsigned int                   priority;      /**< higher value called first */
    HalAeIntrCallback_T            callback_func; /**< callback function pointer */
    void*                          callback_data; /**< callback data */
    struct HalAeCallback_S         *thread_data;  /**< back pointer */
} HalAeCallbackChain_T;

typedef struct HalAeCallback_S {
    struct HalAeCallback_S  *next;                /**< next callback */  
    unsigned int            type_mask;            /**< type mask */
    IxOsalSemaphore         terminate_sem;        /**< termination semaphore */
    IxOsalThread            threadID;             /**< thread ID */
    HalAeCallbackChain_T    callback_chain;       /**< When chained, this first
                                                  CallbackChain structure is 
                                                  just used as a pointer to 
                                                  the chain */
} HalAeCallback_T;

struct PageData_S;

typedef struct Hal_ChipSpecificState_S {
    unsigned int  HalAe_LibInit;         /**< HAL initializaion flag */
    unsigned int  UofChecksum;           /**< .uof check sum */
    SPINLOCK_T    hal_GlobalLock;        /**< HAL global lock */ 
    SPINLOCK_T    hal_CallbackThdLock;   /**< HAL callback thread lock*/
    SPINLOCK_T    hal_InterruptLock;     /**< HAL interrupt lock */
    HalAeCallback_T *ISR_callback_root;  /**< ISR callback root */
    struct PageData_S *pageData;         /**< page data pointer */
} Hal_ChipSpecificState_T;

extern Hal_ChipSpecificState_T css;
#define HALAE_LIBINIT      (css.HalAe_LibInit)
#define UOFCHECKSUM        (css.UofChecksum)
#define HAL_GLOBALLOCK     (css.hal_GlobalLock)
#define HAL_CALLBACKTHDLOCK (css.hal_CallbackThdLock)
#define HAL_INTERRUPTLOCK  (css.hal_InterruptLock)
#define ISR_CALLBACK_ROOT  (css.ISR_callback_root)
#define PAGEDATA           (css.pageData)


/* compute AE number:
   0x00 -> 0x00
   ...
   0x07 -> 0x07
   0x08 -> 0x10
   ...
   0x0F -> 0x17
*/
#define AE_NUM(index) ((index) + ((index) & 0x08))

typedef struct aeDrv_OsMemUsage_S {
    unsigned int start; /* first byte used by OS */
    unsigned int end;   /**< first byte after region used by OS */
} aedrv_OsMemUsage_T[MAX_DRAM_CHAN];



#ifdef __cplusplus
extern "C" {
#endif

int halAeDrvInit(void);
int mapMemIo(HalMmapIo_T *pMemIo);
int unmapMemIo(HalMmapIo_T *pMemIo);
int halAe_ringPut(uint64 baseAddr, unsigned int ringSize, unsigned int tailptr, unsigned int *in_data, unsigned int count);
int halAe_ringGet(uint64 baseAddr, unsigned int ringSize, unsigned int headptr, unsigned int *out_data, unsigned int count);

#ifdef __cplusplus
}
#endif

#define PHYS_ADDR_T unsigned long int

#endif            /* __HALAEDRV_HXX leave next blank line */

/**
 **************************************************************************
 * @file halMmapKern.c
 *
 * @description
 *      This file provides implementation of MEv2 HAL library
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

#include "IxOsal.h"
#include "halMmap.hxx"
#include "hal_global.h"
#include "halAeDrv.hxx"

extern unsigned int hal_ae_ncdram_size;
extern unsigned int hal_ae_cdram_size;
extern unsigned int hal_ae_ncdram_base;
extern unsigned int hal_ae_cdram_base;
extern pci_device_bar_T pci_device_bar;

HalMemMap_T *halAe_ConfigIcpTab(HalMemMap_T *mTab);
HalMemMap_T *halAe_InitMapTable(HalMemMap_T *mTab);
void halAe_DrvMmap(HalMemMap_T *mTab);
void halAe_ConfigIcpTab_Common(HalMemMap_T *mTab);

/*-----------------------------------------------------------------------------
   Function:    halAe_InitMapTable
   Description: Initialize memory map table
   Returns:    
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
HalMemMap_T *
halAe_InitMapTable(HalMemMap_T *mTab)
{
    ixOsalMemSet(mTab, 0, sizeof(HalMemMap_T));
    return (halAe_ConfigIcpTab(mTab));
}

/*-----------------------------------------------------------------------------
   Function:    halAe_DrvMmap
   Description: Map the physical address to virtual address
   Returns:    
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
void halAe_DrvMmap(HalMemMap_T *mTab)
{
    /* Export Local CSR virtual address in this function */
    Hal_cap_ae_local_csr_virtAddr = SADD(mTab->cap_ae_xfer_csr.virt, 0x800);
}

/*-----------------------------------------------------------------------------
   Function:    halAe_ConfigIcpTab_Common
   Description: Configurate ICP chip memmory map table for common entries
   Returns:    
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
void halAe_ConfigIcpTab_Common(HalMemMap_T *mTab)
{
    /* NCDram/CDram base address and size have been retrieved from system driver */    
    if(mTab)
    {
        mTab->mm_scheme = MM_SCHEME_DEFAULT;              
        
        /* None-coherent DRAM managed by AE driver */
        INIT_MTAB(mTab->dram_ch0, hal_ae_ncdram_base,
                                               hal_ae_ncdram_size,
                                               0, 0);    
        /* coherent DRAM managed by AE driver */        
        INIT_MTAB(mTab->dram_ch1, hal_ae_cdram_base,
                                               hal_ae_cdram_size,
                                               0, 1);
        /* SRAM channel 0 */
        INIT_MTAB(mTab->sram_ch0, 
                               pci_device_bar.ring_cntl_sram_region.base_addr,
                               pci_device_bar.ring_cntl_sram_region.size,
                               pci_device_bar.ring_cntl_sram_region.virt_addr, 0);
        /* Global Control CSR */        
        INIT_MTAB(mTab->cap_global_ctl_csr,
                               pci_device_bar.ae_cluster_cap_bridge_reg_region.base_addr,
                               pci_device_bar.ae_cluster_cap_bridge_reg_region.size,
                               pci_device_bar.ae_cluster_cap_bridge_reg_region.virt_addr, 0);
        /* AE Xfer CSR */                               
        INIT_MTAB(mTab->cap_ae_xfer_csr,
                               pci_device_bar.ae_cluster_ae_tran_reg_region.base_addr,
                               pci_device_bar.ae_cluster_ae_tran_reg_region.size, 
                               pci_device_bar.ae_cluster_ae_tran_reg_region.virt_addr, 0);
        /* Scratch read write swap */        
        INIT_MTAB(mTab->scratch_rd_wr_swap,
                               pci_device_bar.ae_cluster_scratch_mem_region.base_addr,
                               pci_device_bar.ae_cluster_scratch_mem_region.size,
                               pci_device_bar.ae_cluster_scratch_mem_region.virt_addr, 0);
        /* SSU CSR */
        INIT_MTAB(mTab->ssu_csr,
                  pci_device_bar.ae_cluster_ssu_reg_region.base_addr,
                  pci_device_bar.ae_cluster_ssu_reg_region.size,
                  pci_device_bar.ae_cluster_ssu_reg_region.virt_addr, 0);
        /* ET ring get/put access */                  
        INIT_MTAB(mTab->eagletail_ring_access, 
                  pci_device_bar.ring_cntl_get_put_region.base_addr, 
                  pci_device_bar.ring_cntl_get_put_region.size,
                  pci_device_bar.ring_cntl_get_put_region.virt_addr, 0);
        /* memory target CSR */                
        INIT_MTAB(mTab->memory_target_csr,
                  pci_device_bar.ring_cntl_reg_region.base_addr,
                  KILO_8, pci_device_bar.ring_cntl_reg_region.virt_addr, 0);
        /* THE END OF TABLE */
        INIT_MTAB(mTab->the_end, 0x0, 0x0, 0, 0);
    }
}

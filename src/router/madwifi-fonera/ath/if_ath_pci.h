/*-
 * Copyright (c) 2002-2005 Sam Leffler, Errno Consulting
 * Copyright (c) 2004-2005 Atheros Communications, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer,
 *    without modification.
 * 2. Redistributions in binary form must reproduce at minimum a disclaimer
 *    similar to the "NO WARRANTY" disclaimer below ("Disclaimer") and any
 *    redistribution must be conditioned upon including a substantially
 *    similar Disclaimer requirement for further binary redistribution.
 * 3. Neither the names of the above-listed copyright holders nor the names
 *    of any contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 *
 * NO WARRANTY
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF NONINFRINGEMENT, MERCHANTIBILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES.
 *
 * $Id: if_ath_pci.h 1441 2006-02-06 16:03:21Z mrenzmann $
 */
 
#ifndef _DEV_ATH_PCI_H_
#define _DEV_ATH_PCI_H_

#include <linux/pci.h>
#define bus_map_single		pci_map_single
#define bus_unmap_single	pci_unmap_single

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,9)
#define bus_dma_sync_single	pci_dma_sync_single_for_cpu
#define	PCI_SAVE_STATE(a,b)	pci_save_state(a)
#define	PCI_RESTORE_STATE(a,b)	pci_restore_state(a)
#else
#define bus_dma_sync_single	pci_dma_sync_single
#define	PCI_SAVE_STATE(a,b)	pci_save_state(a,b)
#define	PCI_RESTORE_STATE(a,b)	pci_restore_state(a,b)
#endif

#define bus_alloc_consistent	pci_alloc_consistent
#define bus_free_consistent	pci_free_consistent
#define BUS_DMA_FROMDEVICE	PCI_DMA_FROMDEVICE
#define BUS_DMA_TODEVICE	PCI_DMA_TODEVICE

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,11)
#define pm_message_t u32
#endif

#ifndef PCI_D0
#define PCI_D0		0
#endif

#ifndef PCI_D3hot
#define PCI_D3hot	3
#endif

#endif   /* _DEV_ATH_PCI_H_ */

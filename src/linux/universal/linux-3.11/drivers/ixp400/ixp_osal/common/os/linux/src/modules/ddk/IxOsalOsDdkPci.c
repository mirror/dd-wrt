/**
 * @file IxOsalOsDdkPci.c (linux)
 *
 * @brief Implementation for PCI functionality. 
 * 
 * 
 * @par
 * IXP400 SW Release version 2.4
 * 
 * -- Copyright Notice --
 * 
 * @par
 * Copyright (c) 2001-2007, Intel Corporation.
 * All rights reserved.
 * 
 * @par
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Intel Corporation nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * 
 * @par
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * 
 * @par
 * -- End of Copyright Notice --
 */

#include <linux/pci.h>

#include "IxOsal.h"

PUBLIC
IxOsalPciDev ixOsalPciDeviceFind(UINT32 vendor_id,UINT32 device_id,IxOsalPciDev pci_dev)
{
	struct pci_dev *pdev = (struct pci_dev *)pci_dev;

	pdev = pci_get_device(vendor_id, device_id, pdev);
	
	return (IxOsalPciDev)pdev;
}

PUBLIC
INT32 ixOsalPciSlotAddress(IxOsalPciDev pci_dev,UINT32 *bus,UINT32 *slot,UINT32 *func)
{
	struct pci_dev *pdev = (struct pci_dev *)pci_dev;

	/* Ensure PCI device handle is not NULL */
	if(!pdev) {
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
        		   IX_OSAL_LOG_DEV_STDOUT,
            		   "ixOsalPciSlotAddress: Null PCI handle \n", 0, 0, 0, 0, 0, 0);
        	return IX_FAIL; 
	}

	if(bus) {
		*bus = pdev->bus->number;
	}
	if(slot) {
		*slot = PCI_SLOT(pdev->devfn);
	}
	if(func) {
		*func = PCI_FUNC(pdev->devfn);
	}
	return IX_SUCCESS;
}

PUBLIC
INT32 ixOsalPciConfigReadByte(IxOsalPciDev pci_dev,UINT32 offset,UINT8* val)
{
	struct pci_dev *pdev = (struct pci_dev *)pci_dev;

	/* Ensure PCI device handle is not NULL */
	if(!pdev) {
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
        		   IX_OSAL_LOG_DEV_STDOUT,
            		   "ixOsalPciConfigReadByte: Null PCI handle \n", 0, 0, 0, 0, 0, 0);
        	return IX_FAIL; 
	}
	
	return pci_read_config_byte(pdev, offset, val);
}

PUBLIC
INT32 ixOsalPciConfigReadShort(IxOsalPciDev pci_dev,UINT32 offset,UINT16* val)
{
	struct pci_dev *pdev = (struct pci_dev *)pci_dev;

	/* Ensure PCI device handle is not NULL */
	if(!pdev) {
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
        		   IX_OSAL_LOG_DEV_STDOUT,
            		   "ixOsalPciConfigReadShort: Null PCI handle \n", 0, 0, 0, 0, 0, 0);
        	return IX_FAIL; 
	}
	
	return pci_read_config_word(pdev, offset, val);
}

PUBLIC
INT32 ixOsalPciConfigReadLong(IxOsalPciDev pci_dev,UINT32 offset,UINT32* val)
{
	struct pci_dev *pdev = (struct pci_dev *)pci_dev;
	
	/* Ensure PCI device handle is not NULL */
	if(!pdev) {
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
        		   IX_OSAL_LOG_DEV_STDOUT,
            		   "ixOsalPciConfigReadLong: Null PCI handle \n", 0, 0, 0, 0, 0, 0);
        	return IX_FAIL; 
	}
	
	return pci_read_config_dword(pdev, offset, val);
}

PUBLIC
INT32 ixOsalPciConfigWriteByte(IxOsalPciDev pci_dev,UINT32 offset,UINT8 val)
{
	struct pci_dev *pdev = (struct pci_dev *)pci_dev;

	/* Ensure PCI device handle is not NULL */
	if(!pdev) {
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
        		   IX_OSAL_LOG_DEV_STDOUT,
            		   "ixOsalPciConfigWriteByte: Null PCI handle \n", 0, 0, 0, 0, 0, 0);
        	return IX_FAIL; 
	}
	
	return pci_write_config_byte(pdev, offset, val);
}

PUBLIC
INT32 ixOsalPciConfigWriteShort(IxOsalPciDev pci_dev,UINT32 offset,UINT16 val)
{
	struct pci_dev *pdev = (struct pci_dev *)pci_dev;

	/* Ensure PCI device handle is not NULL */
	if(!pdev) {
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
        		   IX_OSAL_LOG_DEV_STDOUT,
            		   "ixOsalPciConfigWriteShort: Null PCI handle \n", 0, 0, 0, 0, 0, 0);
        	return IX_FAIL; 
	}

	return pci_write_config_word(pdev, offset, val);
}

PUBLIC
INT32 ixOsalPciConfigWriteLong(IxOsalPciDev pci_dev,UINT32 offset,UINT32 val)
{
	struct pci_dev *pdev = (struct pci_dev *)pci_dev;

	/* Ensure PCI device handle is not NULL */
	if(!pdev) {
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
        		   IX_OSAL_LOG_DEV_STDOUT,
            		   "ixOsalPciConfigWriteLong: Null PCI handle \n", 0, 0, 0, 0, 0, 0);
        	return IX_FAIL; 
	}
	
	return pci_write_config_dword(pdev, offset, val);	
}

PUBLIC
void ixOsalPciDeviceFree(IxOsalPciDev pci_dev)
{
	struct pci_dev *pdev = (struct pci_dev *)pci_dev;

	/* Ensure PCI device handle is not NULL */
	if(!pdev) {
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
        		   IX_OSAL_LOG_DEV_STDOUT,
            		   "ixOsalPciDeviceFree: Null PCI handle \n", 0, 0, 0, 0, 0, 0);
   		return;
	}

	/* 
	 * this takes care of decrementing reference count so that
	 * the kernel may free the space when the count becomes
	 * zero.
	 */
	pci_dev_put(pdev);
}

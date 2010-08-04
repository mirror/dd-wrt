/*******************************************************************************
 *
 *  Copyright (c) 2008 Cavium Networks 
 * 
 *  This file is free software; you can redistribute it and/or modify 
 *  it under the terms of the GNU General Public License, Version 2, as 
 *  published by the Free Software Foundation. 
 *
 *  This file is distributed in the hope that it will be useful, 
 *  but AS-IS and WITHOUT ANY WARRANTY; without even the implied warranty of 
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, TITLE, or 
 *  NONINFRINGEMENT.  See the GNU General Public License for more details. 
 *
 *  You should have received a copy of the GNU General Public License 
 *  along with this file; if not, write to the Free Software 
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA or 
 *  visit http://www.gnu.org/licenses/. 
 *
 *  This file may also be available under a different license from Cavium. 
 *  Contact Cavium Networks for more information
 *
 ******************************************************************************/

#ifndef	_CNS3XXX_PCIE_H_
#define	_CNS3XXX_PCIE_H_

#include "mach/board.h"

#define PCIE0_IO_SPACE_START                   (CNS3XXX_PCIE0_IO_BASE)
#define PCIE0_IO_SPACE_SIZE                    0x01000000      /* 16MB */
#define PCIE0_IO_SPACE_END                     (CNS3XXX_PCIE0_IO_BASE + PCIE0_IO_SPACE_SIZE - 1)

#define PCIE0_MEM_SPACE_START                  (CNS3XXX_PCIE0_MEM_BASE)
#define PCIE0_MEM_SPACE_SIZE                   0x01000000 /* 176MB */
#define PCIE0_MEM_SPACE_END                    (CNS3XXX_PCIE0_MEM_BASE + PCIE0_MEM_SPACE_SIZE - 1)

#define PCIE1_IO_SPACE_START                   (CNS3XXX_PCIE1_IO_BASE)
#define PCIE1_IO_SPACE_SIZE                    0x01000000      /* 16MB */
#define PCIE1_IO_SPACE_END                     (CNS3XXX_PCIE1_IO_BASE + PCIE1_IO_SPACE_SIZE - 1)

#define PCIE1_MEM_SPACE_START           	(CNS3XXX_PCIE1_MEM_BASE)
#define PCIE1_MEM_SPACE_SIZE                   0x01000000 /* 16MB */
#define PCIE1_MEM_SPACE_END                    (CNS3XXX_PCIE1_MEM_BASE + PCIE1_MEM_SPACE_SIZE - 1)

#define	PCIB_MEM_MAP_VALUE(base, reg_offset)	(*((u32 volatile *)(SYSVA_PCI_BRIDGE_##base##_ADDR + reg_offset)))

/*
 * define access macros
 */
#define	PCI_BRIDGE_CONFIG_DATA			PCIB_MEM_MAP_VALUE(CONFIG_DATA_BASE, 0x2C)
#define	PCI_BRIDGE_CONFIG_ADDR			PCIB_MEM_MAP_VALUE(CONFIG_ADDR_BASE, 0x28)

#define PCI_BRIDGE_CONFIG_DATA_REG_OFFSET	0x2C
#define PCI_BRIDGE_CONFIG_ADDR_REG_OFFSET	0x28


/* PCIe MISC 0 Register */
#define CNS3XXX_PCIEPHY0_CMCTL0			(CNS3XXX_MISC_BASE_VIRT + 0x900)
#define CNS3XXX_PCIEPHY0_CMCTL1			(CNS3XXX_MISC_BASE_VIRT + 0x904)
#define CNS3XXX_PCIEPHY0_CTL1			(CNS3XXX_MISC_BASE_VIRT + 0x940)
#define CNS3XXX_PCIE0_AXIS_AWMISC		(CNS3XXX_MISC_BASE_VIRT + 0x944)
#define CNS3XXX_PCIE0_AXIS_ARMISC		(CNS3XXX_MISC_BASE_VIRT + 0x948)
#define CNS3XXX_PCIE0_AXIS_RMISC		(CNS3XXX_MISC_BASE_VIRT + 0x94C)
#define CNS3XXX_PCIE0_AXIS_BMISC		(CNS3XXX_MISC_BASE_VIRT + 0x950)
#define CNS3XXX_PCIE0_AXIM_RMISC		(CNS3XXX_MISC_BASE_VIRT + 0x954)
#define CNS3XXX_PCIE0_AXIM_BMISC		(CNS3XXX_MISC_BASE_VIRT + 0x958)
#define CNS3XXX_PCIE0_CTRL			(CNS3XXX_MISC_BASE_VIRT + 0x95C)
#define CNS3XXX_PCIE0_PM_DEBUG			(CNS3XXX_MISC_BASE_VIRT + 0x960)
#define CNS3XXX_PCIE0_RFC_DEBUG			(CNS3XXX_MISC_BASE_VIRT + 0x964)
#define CNS3XXX_PCIE0_CXPL_DEBUGL		(CNS3XXX_MISC_BASE_VIRT + 0x968)
#define CNS3XXX_PCIE0_CXPL_DEBUGH		(CNS3XXX_MISC_BASE_VIRT + 0x96C)
#define CNS3XXX_PCIE0_DIAG			(CNS3XXX_MISC_BASE_VIRT + 0x970)
#define CNS3XXX_PCIE0_INT_STATUS		(CNS3XXX_MISC_BASE_VIRT + 0x974)
#define CNS3XXX_PCIE0_INT_MASK			(CNS3XXX_MISC_BASE_VIRT + 0x978)


/* PCIe MISC 1 Register */
#define CNS3XXX_PCIEPHY1_CMCTL0			(CNS3XXX_MISC_BASE_VIRT + 0xA00)
#define CNS3XXX_PCIEPHY1_CMCTL1			(CNS3XXX_MISC_BASE_VIRT + 0xA04)
#define CNS3XXX_PCIEPHY1_CTL1			(CNS3XXX_MISC_BASE_VIRT + 0xA40)
#define CNS3XXX_PCIE1_AXIS_AWMISC		(CNS3XXX_MISC_BASE_VIRT + 0xA44)
#define CNS3XXX_PCIE1_AXIS_ARMISC		(CNS3XXX_MISC_BASE_VIRT + 0xA48)
#define CNS3XXX_PCIE1_AXIS_RMISC		(CNS3XXX_MISC_BASE_VIRT + 0xA4C)
#define CNS3XXX_PCIE1_AXIS_BMISC		(CNS3XXX_MISC_BASE_VIRT + 0xA50)
#define CNS3XXX_PCIE1_AXIM_RMISC		(CNS3XXX_MISC_BASE_VIRT + 0xA54)
#define CNS3XXX_PCIE1_AXIM_BMISC		(CNS3XXX_MISC_BASE_VIRT + 0x958)
#define CNS3XXX_PCIE1_CTRL			(CNS3XXX_MISC_BASE_VIRT + 0xA5C)
#define CNS3XXX_PCIE1_PM_DEBUG			(CNS3XXX_MISC_BASE_VIRT + 0xA60)
#define CNS3XXX_PCIE1_RFC_DEBUG			(CNS3XXX_MISC_BASE_VIRT + 0xA64)
#define CNS3XXX_PCIE1_CXPL_DEBUGL		(CNS3XXX_MISC_BASE_VIRT + 0xA68)
#define CNS3XXX_PCIE1_CXPL_DEBUGH		(CNS3XXX_MISC_BASE_VIRT + 0xA6C)
#define CNS3XXX_PCIE1_DIAG			(CNS3XXX_MISC_BASE_VIRT + 0xA70)
#define CNS3XXX_PCIE1_INT_STATUS		(CNS3XXX_MISC_BASE_VIRT + 0xA74)
#define CNS3XXX_PCIE1_INT_MASK			(CNS3XXX_MISC_BASE_VIRT + 0xA78)


/*
 * define constants macros
 */

#define	PCIB_DEVICE_ID			     0x3400
#define	PCIB_VENDOR_ID			     0x177D
#define	PCIB_CLASS_CODE			     0xFF0000
#define	PCIB_REVISION_ID		     0x00
#define	PCIB_BAR0_MEMORY_SPACE_BASE	     0x20000000
#define	PCIB_BAR1_IO_SPACE_BASE		     0x20000000
#define	PCI_MEMORY_SPACE_BASE		     0xB0000000
#define	PCI_IO_SPACE_BASE		     0xA8000000
#define	PCI_MAX_BUS_NUM			     0x01
#define	PCI_MAX_DEVICE_NUM		     0x14
#define	PCI_MAX_FUNCTION_NUM		     0x01
#define	PCI_MAX_REG_NUM			     0x3C

#define	PCI_MAX_DEVICE_TYPE_NUM		     0x13
#define	PCI_MAX_BAR_NUM			     0x06

#define	PCI_CSH_VENDOR_ID_REG_ADDR	     0x00
#define	PCI_CSH_DEVICE_ID_REG_ADDR	     0x02
#define	PCI_CSH_COMMAND_REG_ADDR	     0x04
#define	PCI_CSH_STATUS_REG_ADDR		     0x06
#define	PCI_CSH_REVISION_CLASS_REG_ADDR	     0x08
#define	PCI_CSH_CACHE_LINE_SIZE_REG_ADDR     0x0C
#define	PCI_CSH_LATENCY_TIMER_REG_ADDR	     0x0D
#define	PCI_CSH_HEADER_TYPE_REG_ADDR	     0x0E
#define	PCI_CSH_BIST_REG_ADDR		     0x0F
#define	PCI_CSH_BAR_REG_ADDR		     0x10


#define	PCI_IO_SPACE_SIZE_1M		     0x00
#define	PCI_IO_SPACE_SIZE_2M		     0x01
#define	PCI_IO_SPACE_SIZE_4M		     0x02
#define	PCI_IO_SPACE_SIZE_8M		     0x03
#define	PCI_IO_SPACE_SIZE_16M		     0x04
#define	PCI_IO_SPACE_SIZE_32M		     0x05
#define	PCI_IO_SPACE_SIZE_64M		     0x06
#define	PCI_IO_SPACE_SIZE_128M		     0x07
#define	PCI_IO_SPACE_SIZE_256M		     0x08
#define	PCI_IO_SPACE_SIZE_512M		     0x09
#define	PCI_IO_SPACE_SIZE_1G		     0x0A
#define	PCI_IO_SPACE_SIZE_2G		     0x0B


struct pcie_dbgfs_reg{
	char *name;
	u32 *addr;
};

#endif	/* end of #ifndef _STAR_PCIE_H_ */


/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifndef __ASM_ARCH_HARDWARE_H
#define __ASM_ARCH_HARDWARE_H

#include <asm/sizes.h>
#include "../arch/arm/mach-feroceon-orion/config/mvSysHwConfig.h"


#define PCI_MEMORY_VADDR        mv_pci_mem_base_get(0)/*PEX0_MEM_BASE*/	/* PCI MEM Vaddr */
#define PCI_IO_VADDR            mv_pci_io_base_get(0)/*PEX0_IO_BASE*/	/* PCI IO Vadder */

#define pcibios_assign_all_busses()     1	/* assign a bus number over the bridge while scanning it */
                                                                                                                             
#define PCIBIOS_MIN_IO          0x1000		/* min IO allocate for PCI dev */
#define PCIBIOS_MIN_MEM         0x01000000	/* min MEM allocate for PCI dev */

#define PCIMEM_BASE             PCI_MEMORY_VADDR /* mem base for VGA use*/

#endif


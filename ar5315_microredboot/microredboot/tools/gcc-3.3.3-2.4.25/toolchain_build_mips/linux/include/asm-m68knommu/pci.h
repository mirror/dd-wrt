#ifndef M68KNOMMU_PCI_H
#define	M68KNOMMU_PCI_H

#include <asm-m68k/pci.h>

/*
 *	These are pretty much arbitary with the CoMEM implementation.
 *	We have the whole address space to ourselves.
 */
#define PCIBIOS_MIN_IO		0x100
#define PCIBIOS_MIN_MEM		0x00010000

#endif

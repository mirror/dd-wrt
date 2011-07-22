/*
 * Copyright 2008 Cavium Networks
 * Copyright 2003 ARM Limited
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, Version 2, as
 * published by the Free Software Foundation.
 */
#ifndef __MACH_IO_H
#define __MACH_IO_H

#include "cns3xxx.h"

#define IO_SPACE_LIMIT 0xffffffff

static inline void __iomem *__io(unsigned long addr)
{
	return (void __iomem *)((addr - CNS3XXX_PCIE0_IO_BASE)
		+ CNS3XXX_PCIE0_IO_BASE_VIRT);
}

#define __io(a)			__io(a)
#define __mem_pci(a)		(a)

#endif

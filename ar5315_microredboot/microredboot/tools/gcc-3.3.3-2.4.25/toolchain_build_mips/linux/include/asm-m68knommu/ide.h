#ifndef _M68KNOMMU_IDE_H
#define _M68KNOMMU_IDE_H
#ifdef __KERNEL__
/****************************************************************************/
/*
 *  linux/include/asm-m68knommu/ide.h
 *
 *  Copyright (C) 2001-2003  David McCullough <davidm@snapgear.com>
 */
/****************************************************************************/

#include <linux/config.h>

#include <asm/setup.h>
#include <asm/io.h>
#include <asm/irq.h>

/****************************************************************************/

#undef SUPPORT_SLOW_DATA_PORTS
#define SUPPORT_SLOW_DATA_PORTS 0

#undef SUPPORT_VLB_SYNC
#define SUPPORT_VLB_SYNC 0

#ifndef MAX_HWIFS
#define MAX_HWIFS 4
#endif

#define IDE_ARCH_ACK_INTR 1
#define ide_ack_intr(hwif) ((hwif)->hw.ack_intr?(hwif)->hw.ack_intr(hwif):1)

/****************************************************************************/
/* these guys are not used,  the uclinux.c driver does it all for us        */

static __inline__ int ide_default_irq(ide_ioreg_t base) { return 0; }
static __inline__ ide_ioreg_t ide_default_io_base(int index) { return 0; }
static __inline__ void ide_init_default_hwifs(void) { }

static __inline__ void ide_init_hwif_ports(hw_regs_t *hw,
		                       ide_ioreg_t data_port, ide_ioreg_t ctrl_port,
							   int *irq)
{
}

/****************************************************************************/
/* some bits needed for parts of the IDE subsystem to compile               */

#define __ide_mm_insw(port, addr, n)	insw((u16 *)port, addr, n)
#define __ide_mm_insl(port, addr, n)	insl((u32 *)port, addr, n)
#define __ide_mm_outsw(port, addr, n)	outsw((u16 *)port, addr, n)
#define __ide_mm_outsl(port, addr, n)	outsl((u32 *)port, addr, n)

/****************************************************************************/
#endif /* __KERNEL__ */
#endif /* _M68KNOMMU_IDE_H */

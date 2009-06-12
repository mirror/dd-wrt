/*
 * linux/include/asm-armnommu/arch-ta7s/io.h
 *
 * Copyright (C) 1997-1999 Russell King
 *
 * Modifications:
 *  06-12-1997	RMK	Created.
 *  07-04-1999	RMK	Major cleanup
 *  02-19-2001  gjm     Leveraged for armnommu/dsc21
 *  05-30-2002  csh     Modified for armnommu/arch-ta7s
 */
#ifndef __ASM_ARM_ARCH_IO_H
#define __ASM_ARM_ARCH_IO_H

/*
 * kernel/resource.c uses this to initialize the global ioport_resource struct
 * which is used in all calls to request_resource(), allocate_resource(), etc.
 * --gmcnutt
 */
#define IO_SPACE_LIMIT 0xffffffff

/*
 * If we define __io then asm/io.h will take care of most of the inb & friends
 * macros. It still leaves us some 16bit macros to deal with ourselves, though.
 * We don't have PCI or ISA on the dsc21 so I dropped __mem_pci & __mem_isa.
 * --gmcnutt
 */
#define PCIO_BASE 0
#define __io(a) (PCIO_BASE + (a))
#define __arch_getw(a) (*(volatile unsigned short *)(a))
#define __arch_putw(v,a) (*(volatile unsigned short *)(a) = (v))

/*
 * Defining these two gives us ioremap for free. See asm/io.h.
 * --gmcnutt
 */
#define iomem_valid_addr(iomem,sz) (1)
#define iomem_to_phys(iomem) (iomem)

/*
 * These functions are needed for mtd/maps/physmap.c
 * --rp
 */ 
#ifndef memset_io
#define memset_io(a,b,c)		_memset_io((a),(b),(c))
#endif
#ifndef memcpy_fromio
#define memcpy_fromio(a,b,c)		_memcpy_fromio((a),(b),(c))
#endif
#ifndef memcpy_toio
#define memcpy_toio(a,b,c)		_memcpy_toio((a),(b),(c))
#endif

#ifndef __mem_pci
/* Implement memory read/write functions (needed for mtd) */
#define readb(addr)			__arch_getb(addr)
#define readw(addr)			__arch_getw(addr)
#define readl(addr)			__arch_getl(addr)
#define writeb(v,addr)			__arch_putb(v,addr)
#define writew(v,addr)			__arch_putw(v,addr)
#define writel(v,addr)			__arch_putl(v,addr)

#define eth_io_copy_and_sum(a,b,c,d)	__readwrite_bug("eth_io_copy_and_sum")

#define check_signature(io,sig,len)	(0)

#endif

#endif


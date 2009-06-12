/*
 * uclinux/include/asm-armnommu/arch-S3C44B0X/io.h
 *
 * Copyright (C) 2003 Thomas Eschenbacher <eschenbacher@sympat.de>
 *
 * Copyright (C) 1997-1999 Russell King
 *
 * Modifications:
 *  06-12-1997	RMK	Created.
 *  07-04-1999	RMK	Major cleanup
 *  02-19-2001  gjm     Leveraged for armnommu/dsc21
 *  08-13-2003  the     copied to S3C44B0X arch
 *
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
 * These functions are needed for mtd/maps/physmap.c
 * (copied from include/asm/arch-atmel/io.h)
 * --the
 */
#ifndef memset_io
#define memset_io(a,b,c)                _memset_io((a),(b),(c))
#endif
#ifndef memcpy_fromio
#define memcpy_fromio(a,b,c)            _memcpy_fromio((a),(b),(c))
#endif
#ifndef memcpy_toio
#define memcpy_toio(a,b,c)              _memcpy_toio((a),(b),(c))
#endif

/* maybe someone needs this in far future... */
#define PCIO_BASE 0

/*
 * These macros were copied from arch/armnommu/io.h and are used instead
 * of the definitions found there, because we want to do 16/32 bit i/o
 * without byte swapping.
 * --the
 */
#define __arch_getw(a)    (*(volatile unsigned short *)(a))
#define __arch_putw(v,a)  (*(volatile unsigned short *)(a) = (v))

#ifndef __iob
  #define __iob(a) __io(a)
#endif

#define outb(v,p)	__raw_writeb(v, p)
#define outw(v,p)	__raw_writew(v, p)
#define outl(v,p)	__raw_writel(v, p)

#define inb(p)		({ unsigned int __v = __raw_readb(p); __v; })
#define inw(p)		({ unsigned int __v = __raw_readw(p); __v; })
#define inl(p)		({ unsigned int __v = __raw_readl(p); __v; })

#define outsb(p,d,l)	__raw_writesb(p, d, l)
#define outsw(p,d,l)	__raw_writesw(p, d, l)
#define outsl(p,d,l)	__raw_writesl(p, d, l)

#define insb(p,d,l)	__raw_readsb(p, d, l)
#define insw(p,d,l)	__raw_readsw(p, d, l)
#define insl(p,d,l)	__raw_readsl(p, d, l)

/*
 * Defining these two gives us ioremap for free. See asm/io.h.
 * --gmcnutt
 */
#define iomem_valid_addr(iomem,sz) (1)
#define iomem_to_phys(iomem) (iomem)

#endif /* __ASM_ARM_ARCH_IO_H */

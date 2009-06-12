#ifndef _FRIO_IO_H
#define _FRIO_IO_H

#ifdef __KERNEL__

#include <linux/config.h>
#include <asm/virtconvert.h>

/* Chang Junxiao, June 25, 2003.
 * This macro is base of PCI IO address window of AD21535 */
#define AD21535_PCIIO_BASE 0xeefe0000

/*
 * These are for ISA/PCI shared memory _only_ and should never be used
 * on any other type of memory, including Zorro memory. They are meant to
 * access the bus in the bus byte order which is little-endian!.
 *
 * readX/writeX() are used to access memory mapped devices. On some
 * architectures the memory mapped IO stuff needs to be accessed
 * differently. On the m68k architecture, we just read/write the
 * memory location directly.
 */
/* ++roman: The assignments to temp. vars avoid that gcc sometimes generates
 * two accesses to memory, which may be undesireable for some devices.
 */
#define readb(addr) ({ unsigned char __v = (*(volatile unsigned char *) (addr));asm("ssync;"); __v; })
#define readw(addr) ({ unsigned short __v = (*(volatile unsigned short *) (addr)); asm("ssync;");__v; })
#define readl(addr) ({ unsigned int __v = (*(volatile unsigned int *) (addr));asm("ssync;"); __v; })

#define writeb(b,addr) {((*(volatile unsigned char *) (addr)) = (b)); asm("ssync;");}
#define writew(b,addr) {((*(volatile unsigned short *) (addr)) = (b)); asm("ssync;");}
#define writel(b,addr) {((*(volatile unsigned int *) (addr)) = (b)); asm("ssync;");}

#define memset_io(a,b,c)	memset((void *)(a),(b),(c))
#define memcpy_fromio(a,b,c)	memcpy((a),(void *)(b),(c))
#define memcpy_toio(a,b,c)	memcpy((void *)(a),(b),(c))

#define inb_p(addr) readb((addr) + AD21535_PCIIO_BASE)
#define inb(addr) readb((addr) + AD21535_PCIIO_BASE)

#define outb(x,addr) writeb(x, (addr) + AD21535_PCIIO_BASE)
#define outb_p(x,addr) outb(x, (addr) + AD21535_PCIIO_BASE)

#define inw(addr) readw((addr) + AD21535_PCIIO_BASE)
#define inl(addr) readl((addr) + AD21535_PCIIO_BASE)

#define outw(x,addr) writew(x, (addr) + AD21535_PCIIO_BASE)
#define outl(x,addr) writel(x, (addr) + AD21535_PCIIO_BASE)

#define insb(port, addr, count) memcpy((void*)addr, (void*)(AD21535_PCIIO_BASE + port), count)
#define insw(port, addr, count) memcpy((void*)addr, (void*)(AD21535_PCIIO_BASE + port), (2*count))
#define insl(port, addr, count) memcpy((void*)addr, (void*)(AD21535_PCIIO_BASE + port), (4*count))

#define outsb(port, addr, count) memcpy((void*)(AD21535_PCIIO_BASE + port), (void*)addr, count)
#define outsw(port, addr, count) memcpy((void*)(AD21535_PCIIO_BASE + port), (void*)addr, (2*count))
#define outsl(port, addr, count) memcpy((void*)(AD21535_PCIIO_BASE + port), (void*)addr, (4*count))

#define IO_SPACE_LIMIT 0xffff


/* Values for nocacheflag and cmode */
#define IOMAP_FULL_CACHING		0
#define IOMAP_NOCACHE_SER		1
#define IOMAP_NOCACHE_NONSER		2
#define IOMAP_WRITETHROUGH		3

extern void *__ioremap(unsigned long physaddr, unsigned long size, int cacheflag);
extern void __iounmap(void *addr, unsigned long size);

extern inline void *ioremap(unsigned long physaddr, unsigned long size)
{
	return __ioremap(physaddr, size, IOMAP_NOCACHE_SER);
}
extern inline void *ioremap_nocache(unsigned long physaddr, unsigned long size)
{
	return __ioremap(physaddr, size, IOMAP_NOCACHE_SER);
}
extern inline void *ioremap_writethrough(unsigned long physaddr, unsigned long size)
{
	return __ioremap(physaddr, size, IOMAP_WRITETHROUGH);
}
extern inline void *ioremap_fullcache(unsigned long physaddr, unsigned long size)
{
	return __ioremap(physaddr, size, IOMAP_FULL_CACHING);
}

extern void iounmap(void *addr);

/* Nothing to do */


extern void blkfin_inv_cache_all(void);
#define dma_cache_inv(_start,_size) do { blkfin_inv_cache_all();} while (0)
#define dma_cache_wback(_start,_size) do { } while (0)
#define dma_cache_wback_inv(_start,_size) do { blkfin_inv_cache_all();} while (0)

#endif /* __KERNEL__ */

#endif /* _FRIO_IO_H */

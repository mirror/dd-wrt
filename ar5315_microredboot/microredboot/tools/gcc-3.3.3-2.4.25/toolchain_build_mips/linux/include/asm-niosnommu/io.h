#ifndef __NIOS_IO_H
#define __NIOS_IO_H

#include <linux/kernel.h>

#include <asm/page.h>      /* IO address mapping routines need this */
#include <asm/system.h>

extern void insw(unsigned long port, void *dst, unsigned long count);
extern void outsw(unsigned long port, void *src, unsigned long count);


/*
 * readX/writeX() are used to access memory mapped devices. On some
 * architectures the memory mapped IO stuff needs to be accessed
 * differently. On the Nios architecture, we just read/write the
 * memory location directly.
 */

#define readb(addr) (*(volatile unsigned char *)  (addr))
#define readw(addr) (*(volatile unsigned short *) (addr))
#define readl(addr) (*(volatile unsigned int *)   (addr))

#define writeb(b,addr) (*(volatile unsigned char *)  (addr) = (b))
#define writew(b,addr) (*(volatile unsigned short *) (addr) = (b))
#define writel(b,addr) (*(volatile unsigned int *)   (addr) = (b))


/*
 *	make the short names macros so specific devices
 *	can override them as required
 */

#define inb(addr)    readb(addr)
#define inw(addr)    readw(addr)
#define inl(addr)    readl(addr)

#define outb(x,addr) ((void) writeb(x,addr))
#define outw(x,addr) ((void) writew(x,addr))
#define outl(x,addr) ((void) writel(x,addr))

#define inb_p(addr)    inb(addr)
#define inw_p(addr)    inw(addr)
#define inl_p(addr)    inl(addr)

#define outb_p(x,addr) outb(x,addr)
#define outw_p(x,addr) outw(x,addr)
#define outl_p(x,addr) outl(x,addr)



extern inline void insb(unsigned long port, void *dst, unsigned long count)
{
	while (count--)
		*(((unsigned char *)dst)++) = inb(port);
}

/* See arch/niosnommu/io.c for optimized version */
extern inline void _insw(unsigned long port, void *dst, unsigned long count)
{
	while (count--)
		*(((unsigned short *)dst)++) = inw(port);
}

extern inline void insl(unsigned long port, void *dst, unsigned long count)
{
	while (count--)
		*(((unsigned long *)dst)++) = inl(port);
}

extern inline void outsb(unsigned long port, void *src, unsigned long count)
{
	while (count--) 
        outb( *(((unsigned char *)src)++), port );
}

/* See arch/niosnommu/io.c for optimized version */
extern inline void _outsw(unsigned long port, void *src, unsigned long count)
{
	while (count--) 
        outw( *(((unsigned short *)src)++), port );
}

extern inline void outsl(unsigned long port, void *src, unsigned long count)
{
	while (count--) 
        outl( *(((unsigned long *)src)++), port );
}



extern inline void mapioaddr(unsigned long physaddr, unsigned long virt_addr,
			     int bus, int rdonly)
{
	return;
}

//vic - copied from m68knommu

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

#define IO_SPACE_LIMIT 0xffff
#endif /* !(__NIOS_IO_H) */


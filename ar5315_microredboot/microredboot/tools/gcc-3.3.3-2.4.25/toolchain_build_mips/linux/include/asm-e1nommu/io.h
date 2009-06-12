#ifndef _HYPERSTONE_NOMMU_IO_H
#define _HYPERSTONE_NOMMU_IO_H

#ifdef __KERNEL__

#include <linux/config.h>
#include <asm/virtconvert.h>

#define IORegAddress	13
#define IOWait			11
#define IOSetupTime		8
#define IOAccessTime	5
#define IOHoldTime		3

#ifdef CONFIG_E132XS_BOARD
#define NR_CS	15
#define SLOW_IO_ACCESS ( 0x3 << IOSetupTime | 0x0 << IOWait | 7 << IOAccessTime | 3 << IOHoldTime )
#else
/* Yannis Fixme !! */
#define NR_CS	4
#endif

extern unsigned int io_periph[NR_CS];

/*
 * swap functions are sometimes needed to interface little-endian hardware
 */
static inline unsigned short _swapw(volatile unsigned short v)
{
    return ((v << 8) | (v >> 8));
}

static inline unsigned int _swapl(volatile unsigned long v)
{
    return ((v << 24) | ((v & 0xff00) << 8) | ((v & 0xff0000) >> 8) | (v >> 24));
}

#define  hy_inpw(addr)  						\
	({   register unsigned long dummy, dummy1; 			\
		 dummy  = addr; 						\
		 asm volatile  ("LDW.IOD   %1, %0, 0" 	\
				 		: "=l" (dummy1) 		\
					   	: "l" (dummy)); dummy1; })


#define  hy_outpw(x, addr)						  \
	({   register unsigned long dummy0,dummy1; \
	 	 dummy0 = addr; 					  \
		 dummy1 = x;						  \
		 asm volatile  ("STW.IOD   %1, %0, 0" \
						 : "=l" (dummy1) 	  \
						 : "l"(dummy0), "l" (dummy1)); dummy1; })

#ifndef CONFIG_IO_ACCESS 
#define readb(addr)	(hy_inpw(addr))
#define readw(addr)	(hy_inpw(addr))
#define readl(addr)	(hy_inpw(addr))

#define writeb(b,addr) (void)(hy_outpw(b,addr))
#define writew(b,addr) (void)(hy_outpw(b,addr))
#define writel(b,addr) (void)(hy_outpw(b,addr))

#else

#define readb(addr)	({ unsigned char  __v = inregb(addr); __v; })
#define readw(addr)	({ unsigned short __v = inregw(addr); __v; })
#define readl(addr)	({ unsigned long  __v = inregl(addr); __v; })

#define writeb(b,addr) (void)(outreg(b, addr))
#define writew(b,addr) (void)(outreg(b, addr))
#define writel(b,addr) (void)(outreg(b, addr))

#endif

static inline unsigned long common_io_access(unsigned long addr)
{
#ifdef CONFIG_IO_ACCESS
	return io_periph[(addr & 0x03C00000) >> 22];
#else 
	return 	addr;
#endif
}

static inline volatile unsigned char inregb(volatile unsigned long reg)
{
	unsigned char val;

/* FIXME: ask Christoph for the masking of the "reg" value. In the manual
 * it appears to be 3 bit, but in the schematic they use 4 bits. So we mask
 * 4 bits
 */
	
	val = hy_inpw(common_io_access(reg) | ((0xf & reg) << IORegAddress)); 
	return val;
}

static inline volatile unsigned short inregw(volatile unsigned long reg)
{
	unsigned short val;

	val = hy_inpw(common_io_access(reg) | ((0xf & reg) << IORegAddress)); 
	return val;
}

static inline volatile unsigned long inregl(volatile unsigned long reg)
{
	unsigned long val;

	val = hy_inpw(common_io_access(reg) | ((0xf & reg) << IORegAddress)); 
	return val;
}


static inline void outreg(volatile unsigned long val, volatile unsigned long reg)
{
		
	hy_outpw(val, (common_io_access(reg) | ((0xf & reg) << IORegAddress)));
}

static inline void io_outsb(unsigned int addr, void *buf, int len)
{
	unsigned long tmp;
	unsigned char *bp = (unsigned char *) buf;

	tmp = (common_io_access(addr)) | ((0xf & addr) << IORegAddress);

	while (len--){
		hy_outpw(_swapw(*bp++), tmp);
	}
}

static inline void io_outsw(volatile unsigned int addr, void *buf, int len)
{
	unsigned long tmp;
	unsigned short *bp = (unsigned short *) buf;
	
	tmp = (common_io_access(addr)) | ((0xf & addr) << IORegAddress);

	while (len--){
		hy_outpw(_swapw(*bp++), tmp);
	}
}

static inline void io_outsl(volatile unsigned int addr, void *buf, int len)
{
	unsigned long tmp;
	unsigned int *bp = (unsigned int *) buf;
		
	tmp = (common_io_access(addr)) | ((0xf & addr) << IORegAddress);

	while (len--){
		hy_outpw(_swapl(*bp++), tmp);
	}
}

static inline void io_insb(volatile unsigned int addr, void *buf, int len)
{
	unsigned long tmp;
	unsigned char *bp = (unsigned char *) buf;

	tmp = (common_io_access(addr)) | ((0xf & addr) << IORegAddress);

	while (len--)
		*bp++ = hy_inpw((unsigned char) tmp);
	
}

static inline void io_insw(unsigned int addr, void *buf, int len)
{
	unsigned long tmp;
	unsigned short *bp = (unsigned short *) buf;

	tmp = (common_io_access(addr)) | ((0xf & addr) << IORegAddress);

	while (len--)
		*bp++ = _swapw((unsigned short)hy_inpw(tmp));

}

static inline void io_insl(unsigned int addr, void *buf, int len)
{
	unsigned long tmp;
	unsigned int *bp = (unsigned int *) buf;

	tmp = (common_io_access(addr)) | ((0xf & addr) << IORegAddress);

	while (len--)
		*bp++ = _swapl((unsigned int)hy_inpw(tmp));
}

/*
 *	make the short names macros so specific devices
 *	can override them as required
 */

#define memset_io(a,b,c)	memset((void *)(a),(b),(c))
#define memcpy_fromio(a,b,c)	memcpy((a),(void *)(b),(c))
#define memcpy_toio(a,b,c)	memcpy((void *)(a),(b),(c))

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

#define outsb(a,b,l) io_outsb(a,b,l)
#define outsw(a,b,l) io_outsw(a,b,l)
#define outsl(a,b,l) io_outsl(a,b,l)

#define insb(a,b,l) io_insb(a,b,l)
#define insw(a,b,l) io_insw(a,b,l)
#define insl(a,b,l) io_insl(a,b,l)


#define __io_virt(x)            ((unsigned long)x)
#define __io_phys(x)            ((unsigned long)x)

/* FIXME */
#define IO_SPACE_LIMIT 0xffffffff

#define MAX_NR_EXTERNAL_DEVICES		8

//#define iounmap(addr)				((void)0)
#define ioremap(physaddr, size)			(physaddr)
#define ioremap_nocache(physaddr, size)		(physaddr)
#define ioremap_writethrough(physaddr, size)	(physaddr)
#define ioremap_fullcache(physaddr, size)	(physaddr)


#define page_to_phys(page)    ((page - mem_map) << PAGE_SHIFT)

#endif /* __KERNEL__ */

#endif /* _HYPERSTONE_NOMMU_IO_H */

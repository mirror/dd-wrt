/*
 * linux/include/asm-armnommu/arch-c5471/io.h
 */

#ifndef __ASM_ARM_ARCH_IO_H
#define __ASM_ARM_ARCH_IO_H

/* kernel/resource.c uses this to initialize the global ioport_resource struct
 * which is used in all calls to request_resource(), allocate_resource(), etc.
 */

#define IO_SPACE_LIMIT 0xffffffff

/* If we define __io then asm/io.h will take care of most of the inb & friends
 * macros. It still leaves us some 16bit macros to deal with ourselves, though.
 * We don't have PCI or ISA, no  __mem_pci & __mem_isa.
 */

#define PCIO_BASE 0
#define __io(a) (PCIO_BASE + (a))

/* Some compiler options will convert short loads and stores into byte loads
 * and stores.  We don't want this to happen for IO reads and writes!
 */

static inline unsigned short __arch_getw(unsigned int addr)
{
  unsigned short retval;
 __asm__ __volatile__("\tldrh %0, [%1]\n\t" : "=r"(retval) : "r"(addr));
  return retval;
}
static inline void __arch_putw(unsigned short val, unsigned int addr)
{
 __asm__ __volatile__("\tstrh %0, [%1]\n\t": : "r"(val), "r"(addr));
}

/* Defining these two gives us ioremap for free. See asm/io.h. */

#define iomem_valid_addr(iomem,sz) (1)
#define iomem_to_phys(iomem) (iomem)

#endif

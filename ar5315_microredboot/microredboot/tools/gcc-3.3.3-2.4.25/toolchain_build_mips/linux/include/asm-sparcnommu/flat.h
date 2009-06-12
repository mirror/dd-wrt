/*
 * include/asm-sparcnommu/flat.h -- uClinux flat-format executables
 */

#ifndef __SPARCNOMMU_FLAT_H__
#define __SPARCNOMMU_FLAT_H__

/*
 * Sparc requires stacks to start on 8byte boundary
 */

#define	flat_stack_align(sp)		\
	(sp) = (unsigned long *) (((unsigned long) (sp)) & 0xfffffff8); \
	--sp; put_user(0,--sp)

#define flat_argvp_envp_on_stack()	0
#define	flat_old_ram_flag(flags)	(flags)
#define	flat_reloc_valid(reloc, size)	((reloc) <= (size))

/*
 * For SPARC architecture we need to handle the funky HI22 and LO10
 * addressing modes. The relocations occur inside of an instruction,
 * and so we need to preserve the instruction bits but also perform
 * the relocation on the address component.
 */

static inline unsigned long
flat_get_relocate_addr(unsigned long rel)
{
	return(rel & 0x3fffffff);
}

static inline unsigned long
flat_get_addr_from_rp(unsigned long *rp, unsigned long relval)
{
	unsigned long addr = get_unaligned (rp);
	if (relval & 0x80000000)
		addr &= 0x003fffff;
	else if (relval & 0x40000000)
		addr &= 0x000003ff;
	return(addr);
}

static inline unsigned long
flat_put_addr_at_rp(unsigned long *rp,unsigned long addr,unsigned long relval)
{
	if (relval & 0xC0000000) {
		unsigned long old_val = get_unaligned (rp);
		if (relval & 0x80000000)
			addr = (old_val & 0xffc00000) | (addr >> 10);
		else
			addr = (old_val & 0xfffffc00) | (addr & 0x3ff);
	}
	put_unaligned (addr, rp);
}

#endif /* __SPARCNOMMU_FLAT_H__ */

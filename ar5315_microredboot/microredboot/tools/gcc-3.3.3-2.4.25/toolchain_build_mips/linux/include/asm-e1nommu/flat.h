/*
 * include/asm-e1nommu/flat.h -- uClinux flat-format executables
 */

#ifndef __E1NOMMU_FLAT_H__
#define __E1NOMMU_FLAT_H__

#include <asm/unaligned.h>

#define	flat_stack_align(sp)			/* nothing needed */
#define	flat_argvp_envp_on_stack()		0 
#define	flat_old_ram_flag(flags)		(flags)
#define	flat_reloc_valid(reloc, size)		((reloc) <= (size))

static inline unsigned long	
flat_get_addr_from_rp(unsigned long *rp, unsigned long relval)
{	
	unsigned long offset = get_unaligned (rp);
	if (relval & 0x80000000)
		offset &= 0x3fFFffFF; // 30 bits
	else if (relval & 0x40000000)
		offset &= 0x0fFFffFF; // 28 bits

	return(offset);
}

static inline void
flat_put_addr_at_rp(unsigned long *rp, unsigned long addr, unsigned long relval)
{
	unsigned long val = get_unaligned (rp);
	if (relval & 0x80000000)
		val = (val & 0xc0000000) | addr ;
	else if (relval & 0x40000000)
		val = (val & 0xf0000000) | addr ;
	else
		val = addr;
	
		put_unaligned (val, rp);
	
}

static inline unsigned long flat_get_relocate_addr(unsigned long relval)
{ 
	return ( relval & 0x3fFFffFF );
}
#endif 

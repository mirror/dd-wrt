/*
 * include/asm-niosnommu/flat.h -- uClinux flat-format executables
 *
 * this file is yet to be completed: davidm@snapgear.com
 */

#ifndef __NIOSNOMMU_FLAT_H__
#define __NIOSNOMMU_FLAT_H__

#define	flat_stack_align(sp)			XXXX
#define	flat_argvp_envp_on_stack()		XXXX
#define	flat_old_ram_flag(flags)		(flags)
#define	flat_reloc_valid(reloc, size)		((reloc) <= (size))
#define	flat_get_addr_from_rp(rp, relval)	XXXX
#define	flat_put_addr_at_rp(rp, val, relval)	XXXX
#define	flat_get_relocate_addr(rel)		XXXX

#endif /* __NIOSNOMMU_FLAT_H__ */

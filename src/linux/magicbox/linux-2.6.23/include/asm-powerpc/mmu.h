#ifndef _ASM_POWERPC_MMU_H_
#define _ASM_POWERPC_MMU_H_
#ifdef __KERNEL__

#ifdef CONFIG_PPC64
/* 64-bit classic hash table MMU */
#  include <asm/mmu-hash64.h>
#elif defined(CONFIG_PPC_STD_MMU)
/* 32-bit classic hash table MMU */
#  include <asm/mmu-hash32.h>
#elif defined(CONFIG_44x)
/* 44x-style software loaded TLB */
#  include <asm/mmu-44x.h>
#elif defined(CONFIG_FSL_BOOKE)
/* Freescale Book-E software loaded TLB */
#  include <asm/mmu-fsl-booke.h>
#elif defined (CONFIG_PPC_8xx)
/* Motorola/Freescale 8xx software loaded TLB */
#  include <asm/mmu-8xx.h>
#endif

#ifndef __ASSEMBLY__
typedef unsigned long long phys_addr_t;

#ifdef CONFIG_PPC64
typedef unsigned long mm_context_id_t;

typedef struct {
	mm_context_id_t id;
	u16 user_psize;		/* page size index */

#ifdef CONFIG_PPC_MM_SLICES
	u64 low_slices_psize;	/* SLB page size encodings */
	u64 high_slices_psize;  /* 4 bits per slice for now */
#else
	u16 sllp;		/* SLB page size encoding */
#endif
	unsigned long vdso_base;
} mm_context_t;

#else /* !CONFIG_PPC64 */

typedef struct {
	unsigned long id;
	unsigned long vdso_base;
} mm_context_t;

#endif /* CONFIG_PPC64 */
#endif /* !__ASSEMBLY__ */

#endif /* __KERNEL__ */
#endif /* _ASM_POWERPC_MMU_H_ */

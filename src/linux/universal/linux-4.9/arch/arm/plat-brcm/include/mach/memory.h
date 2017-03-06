/*
 * Platform memory layout definitions
 *
 * Note: due to dependencies in common architecture code
 * some mappings go in other header files.
 */
#ifndef __ASM_ARCH_MEMORY_H
#define __ASM_ARCH_MEMORY_H

/*
 * Main memory base address and size
 * are defined from the board-level configuration file
 */

#define PLAT_PHYS_OFFSET             UL(CONFIG_DRAM_BASE)

#define PADDR_ACP_AX		(0x80000000)
#define PADDR_ACP_BX		(0x40000000)
#define PADDR_ACE_BCM53573	(0x00000000)
#define PADDR_ACE1_BCM53573	(0x80000000)

/*
 * Different coherence setting for coherence_flag
 */
#define COHERENCE_NONE		0
#define COHERENCE_ACP_WAR	1
#define COHERENCE_ACP		2
#define COHERENCE_ACE		4
#define COHERENCE_ACP_ACE	(COHERENCE_ACP | COHERENCE_ACE)
#define COHERENCE_MASK		(COHERENCE_ACP_WAR | COHERENCE_ACP_ACE)
 



/*
 * DMA memory
 *
 * The PCIe-to-AXI mapping (PAX) has a window of 128 MB alighed at 1MB
 * we should make the DMA-able DRAM at least this large.
 * Will need to use CONSISTENT_BASE and CONSISTENT_SIZE macros
 * to program the PAX inbound mapping registers.
 */
#define CONSISTENT_DMA_SIZE     SZ_128M
#define PHYS_OFFSET2             UL(0x8000000) /* Default value for NS */

#if !defined(__ASSEMBLY__)
extern unsigned int coherence_flag;
#define ACP_WAR_ENAB()		0 //((coherence_flag & COHERENCE_ACP_WAR) != 0)

//extern unsigned int ddr_phys_offset2_va;
//#define PHYS_OFFSET2   ((unsigned long)ddr_phys_offset2_va)

extern unsigned int coherence_win_sz;
//#define ACP_WIN_SIZE			(coherence_win_sz)
//#define ACP_WIN_LIMIT			(PHYS_OFFSET + ACP_WIN_SIZE)
#else
#endif

#if !defined(__ASSEMBLY__) && defined(CONFIG_ZONE_DMA)
extern void bcm47xx_adjust_zones(unsigned long *size, unsigned long *hole);
#define arch_adjust_zones(size, hole) \
	bcm47xx_adjust_zones(size, hole)

#define ISA_DMA_THRESHOLD	(PHYS_OFFSET + SZ_128M - 1)
//#define MAX_DMA_ADDRESS		(PAGE_OFFSET + SZ_128M)
#endif

#ifdef CONFIG_SPARSEMEM

#define MAX_PHYSMEM_BITS	32
#define SECTION_SIZE_BITS	27

/* bank page offsets */
#define PAGE_OFFSET1	(PAGE_OFFSET + SZ_128M)

#define __phys_to_virt(phys)						\
	((phys) >= PHYS_OFFSET2 ? (phys) - PHYS_OFFSET2 + PAGE_OFFSET1 :	\
	 (phys) + PAGE_OFFSET)

#define __virt_to_phys(virt)						\
	 ((virt) >= PAGE_OFFSET1 ? (virt) - PAGE_OFFSET1 + PHYS_OFFSET2 :	\
	  (virt) - PAGE_OFFSET)

#endif


#endif

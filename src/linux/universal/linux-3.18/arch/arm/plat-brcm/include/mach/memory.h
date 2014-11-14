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

#ifdef __ASSEMBLY__
#ifndef PHYS_OFFSET
#define PHYS_OFFSET             UL(CONFIG_DRAM_BASE)
#endif
#endif


/*
 * DMA memory
 *
 * The PCIe-to-AXI mapping (PAX) has a window of 128 MB alighed at 1MB
 * we should make the DMA-able DRAM at least this large.
 * Will need to use CONSISTENT_BASE and CONSISTENT_SIZE macros
 * to program the PAX inbound mapping registers.
 */
#define CONSISTENT_DMA_SIZE     SZ_128M

#endif

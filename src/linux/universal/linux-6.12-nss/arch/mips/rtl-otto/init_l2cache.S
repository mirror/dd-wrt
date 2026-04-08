#include <asm/asm.h>
#include <asm/asmmacro.h>
#include <asm/cacheops.h>
#include <asm/cpu.h>
#include <asm/regdef.h>
#include <asm/mipsregs.h>
#include <asm/stackframe.h>
#include <asm/addrspace.h>
#include <asm/hazards.h>

#define CP0_CONFIG2	$16,2
#define CP0_TAGLO2	$28,4
#define CP0_DATALO2	$28,5
#define CP0_DATAHI2	$29,5
#define GCR_CONFIG_ADDR     0xbfbf8000  // KSEG0 address of the GCR registers


/*
 * ILEAF - declare leaf routine
 */

#define IEND(name)  END(name) 

#define ILEAF(symbol)					\
		CFI_SECTIONS;				\
		.globl	symbol;				\
		.section        ".init.text","ax";      \
		.align	2;				\
		.type	symbol, @function;		\
		.ent	symbol, 0;			\
symbol:		.frame	sp, 0, ra;			\
		.cfi_startproc;				\
		.insn


	.set	push
	.set	noreorder
ILEAF(init_l2_cache)
#ifdef CONFIG_MIPS_CPU_SCACHE
        sync   0x3 //memory sync , avoid some unfinish request in buffer
#if 1
        li     t0, GCR_CONFIG_ADDR  //GCR BASE ADDR
        lw     t1, 0x0008(t0)    // GCR_BASE
        ext    t0, t1, 4, 4                      // Extract CCA Override 
        li     t1, 0x5                           //UC=0x02, CCA_Override =1, 
        bne    t0, t1, done_l23
        nop
#endif
1:	
        // L2 Cache initialization routine
        // Check L2 cache size
        mfc0    t7, CP0_CONFIG2          // C0_Config2

        // Isolate L2$ Line Size
        ext         t6, t7, 4, 4                // extract SL

        // Skip ahead if No L2$
        beq         t6, zero, done_l2cache
        nop

        li          a2, 2
        sllv    t6, a2, t6              // Now have true L2$ line size in bytes

        // Isolate L2$ Sets per Way
        ext         a0, t7, 8, 4                // extract SS
        li          a2, 64
        sllv    a0, a2, a0              // L2$ Sets per way

        // Isolate L2$ Associativity
        // L2$ Assoc (-1)
        ext         a1, t7, 0, 4                // extract SA
        add         a1, 1

        mul         a0, a0, a1          // Get total number of sets

        lui         a2, 0x8000          // Get a KSeg0 address for cacheops

        // Clear L23TagLo/L23DATALo/L23DATAHi registers
        mtc0    zero, CP0_TAGLO2
        mtc0    zero, CP0_DATALO2
        mtc0    zero, CP0_DATAHI2
        
        move    a3, a0

        // L2$ Index Store Tag Cache Op
        // Will invalidate the tag entry, clear the lock bit, and clear the LRF bit
next_L2cache_tag:
        /*  Write Tag using index store tag */
        cache   Index_Store_Tag_SD, 0(a2)    
        add         a3, -1                                              // Decrement set counter

        bne         a3, zero, next_L2cache_tag  // Done yet?
        add         a2, t6                                              // Get next line address

done_l2cache:
        sync     0x3                 //To ensure cache_op finish!
#if 1
        li      t0, GCR_CONFIG_ADDR  //GCR BASE ADDR
        lw      t1, 0x0008(t0)    // GCR_BASE
        ins     t1, zero, 0, 8                      // CCA Override disabled
        sw      t1, 0x0008(t0)    // GCR_BASE
#else
        li      t0, GCR_CONFIG_ADDR  //GCR BASE ADDR
        lw      t1, 0x0008(t0)    // GCR_BASE
        li      t2, 0xF0         // Enable CCA and set to uncached
        ins     t1, t2, 0, 8    // Insert bits
        sw      t1, 0x0008(t0)  // Write GCR_BASE
#endif
	sync     0x3                 //Ensure CCA effective!
#else  //Not define CONFIG_MIPS_CPU_SCACHE
        mfc0    t7, CP0_CONFIG2
        li      t0, 1
        ins     t7, t0, 12, 1
        mtc0    t7, CP0_CONFIG2
#endif
done_l23:
        jr.hb      ra
         nop
IEND(init_l2_cache)
	.set	pop

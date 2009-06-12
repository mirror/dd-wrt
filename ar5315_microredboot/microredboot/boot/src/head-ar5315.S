/*
 * lzma_misc.c
 * originally written for xscale based linux kernel decompression
 * now adapted for AR531X based redboot stub and kernel loader
 * copyright 2009 Sebastian Gottschall / NewMedia-NET GmbH / DD-WRT.COM
 * licensed under GPL conditions
 * based on Vxworks sources from Atheros Communications, Inc.
 *
 */


#include <linux/autoconf.h>
#include <linux/init.h>
#include <linux/threads.h>

#include <asm/addrspace.h>
#include <asm/asm.h>
#include <asm/asmmacro.h>
#include <asm/irqflags.h>
#include <asm/regdef.h>
#include <asm/page.h>
#include <asm/mipsregs.h>
#include <asm/stackframe.h>

#include "mips4Kx.h"
#include "ar531xPlusreg.h"
#include "ar531xSio.h"



#define AR2316_SDRAM0           0x00000000      /* DRAM */
#define AR2316_SPI_READ         0x08000000      /* SPI FLASH */
#define AR2316_WLAN0            0xB0000000      /* Wireless MMR */
#define AR2316_PCI              0xB0100000      /* PCI MMR */
#define AR2316_SDRAMCTL         0xB0300000      /* SDRAM MMR */
#define AR2316_LOCAL            0xB0400000      /* LOCAL BUS MMR */
#define AR2316_ENET0            0xB0500000      /* ETHERNET MMR */
#define AR2316_DSLBASE          0xB1000000      /* RESET CONTROL MMR */
#define AR2316_UART0            0xB1100003      /* UART MMR */
#define AR2316_SPI              0xB1300000      /* SPI FLASH MMR */
#define AR2316_FLASHBT          0xBfc00000      /* ro boot alias to FLASH */
#define AR2316_RAM1             0x40000000      /* ram alias */
#define AR2316_PCIEXT           0x80000000      /* pci external */
#define AR2316_RAM2             0xc0000000      /* ram alias */
#define AR2316_RAM3             0xe0000000      /* ram alias */

#define AR2316_RESET            (AR2316_DSLBASE + 0x0004)

#if defined(COBRA_EMUL)
#define AR2316_AMBA_CLOCK_RATE  20000000
#define AR2316_CPU_CLOCK_RATE   40000000
#else
#if defined(DEFAULT_PLL)
#define AR2316_AMBA_CLOCK_RATE  40000000
#define AR2316_CPU_CLOCK_RATE   40000000
#else
#define AR2316_AMBA_CLOCK_RATE  92000000
#define AR2316_CPU_CLOCK_RATE   184000000
#endif /* ! DEFAULT_PLL */
#endif /* ! COBRA_EMUL */

#define AR2316_UART_CLOCK_RATE  AR2316_AMBA_CLOCK_RATE
#define AR2316_SDRAM_CLOCK_RATE AR2316_AMBA_CLOCK_RATE

#define AR2316_GPIO_DI          (AR2316_DSLBASE + 0x0088)
#define AR2316_GPIO_DO          (AR2316_DSLBASE + 0x0090)
#define AR2316_GPIO_CR          (AR2316_DSLBASE + 0x0098)
#define AR2316_GPIO_INT         (AR2316_DSLBASE + 0x00a0)

#define AR2316_POWER_GPIO_BITMAP  0x80






/*
 * WARNING:
 * This code must be Position Independent Code (PIC) as it runs in
 * the prom address space (0xbfc00000) even though it is linked
 * in memory.  Only call routines in romLib.s and romInit.s with
 * bal.
 */
#ifdef CONFIG_BOOTLOADER

	.section	".start"

#define RVECENT(f,n) \
	b f; nop
#define XVECENT(f,bev) \
	b f; li k0,bev


	/*
	 * Only for Flash based booting - bootloader mode goes straight
	 * to the decompressor, relying on the bootloader to have
	 * initialised the memory controller etc
	 * 
	 * Also use this for JTAG mode.
	 */

	.set	noreorder

__AR531X_start:
	RVECENT(__AR531X_init,0)	/* PROM entry point */
	RVECENT(__AR531X_reboot,1)	/* software reboot */
	RVECENT(romReserved,2)
	RVECENT(romReserved,3)
	RVECENT(romReserved,4)
	RVECENT(romReserved,5)
	RVECENT(romReserved,6)
	RVECENT(romReserved,7)
	RVECENT(romReserved,8)
	RVECENT(romReserved,9)
	RVECENT(romReserved,10)
	RVECENT(romReserved,11)
	RVECENT(romReserved,12)
	RVECENT(romReserved,13)
	RVECENT(romReserved,14)
	RVECENT(romReserved,15)
	RVECENT(romReserved,16)
	RVECENT(romReserved,17)
	RVECENT(romReserved,18)
	RVECENT(romReserved,19)
	RVECENT(romReserved,20)
	RVECENT(romReserved,21)
	RVECENT(romReserved,22)
	RVECENT(romReserved,23)
	RVECENT(romReserved,24)
	RVECENT(romReserved,25)
	RVECENT(romReserved,26)
	RVECENT(romReserved,27)
	RVECENT(romReserved,28)
	RVECENT(romReserved,29)
	RVECENT(romReserved,30)
	RVECENT(romReserved,31)
	RVECENT(romReserved,32)
	RVECENT(romReserved,33)
	RVECENT(romReserved,34)
	RVECENT(romReserved,35)
	RVECENT(romReserved,36)
	RVECENT(romReserved,37)
	RVECENT(romReserved,38)
	RVECENT(romReserved,39)
	RVECENT(romReserved,40)
	RVECENT(romReserved,41)
	RVECENT(romReserved,42)
	RVECENT(romReserved,43)
	RVECENT(romReserved,44)
	RVECENT(romReserved,45)
	RVECENT(romReserved,46)
	RVECENT(romReserved,47)
	RVECENT(romReserved,48)
	RVECENT(romReserved,49)
	RVECENT(romReserved,50)
	RVECENT(romReserved,51)
	RVECENT(romReserved,52)
	RVECENT(romReserved,53)
	RVECENT(romReserved,54)
	RVECENT(romReserved,55)
	RVECENT(romReserved,56)
	RVECENT(romReserved,57)
	RVECENT(romReserved,58)
	RVECENT(romReserved,59)
	RVECENT(romReserved,60)
	RVECENT(romReserved,61)
	RVECENT(romReserved,62)
	RVECENT(romReserved,63)
	XVECENT(romExcHandle,0x200)	/* tlbmiss: for 4Kc */
	RVECENT(romReserved,65)
	RVECENT(romReserved,66)
	RVECENT(romReserved,67)
	RVECENT(romReserved,68)
	RVECENT(romReserved,69)
	RVECENT(romReserved,70)
	RVECENT(romReserved,71)
	RVECENT(romReserved,72)
	RVECENT(romReserved,73)
	RVECENT(romReserved,74)
	RVECENT(romReserved,75)
	RVECENT(romReserved,76)
	RVECENT(romReserved,77)
	RVECENT(romReserved,78)
	RVECENT(romReserved,79)
	XVECENT(romExcHandle,0x280)	/* xtlbmiss: for 4Kc */
	RVECENT(romReserved,81)
	RVECENT(romReserved,82)
	RVECENT(romReserved,83)
	RVECENT(romReserved,84)
	RVECENT(romReserved,85)
	RVECENT(romReserved,86)
	RVECENT(romReserved,87)
	RVECENT(romReserved,88)
	RVECENT(romReserved,89)
	RVECENT(romReserved,90)
	RVECENT(romReserved,91)
	RVECENT(romReserved,92)
	RVECENT(romReserved,93)
	RVECENT(romReserved,94)
	RVECENT(romReserved,95)
	XVECENT(romExcHandle,0x300)	/* cache error: should not happen */
	RVECENT(romReserved,97)
	RVECENT(romReserved,98)
	RVECENT(romReserved,99)
	RVECENT(romReserved,100)
	RVECENT(romReserved,101)
	RVECENT(romReserved,102)
	RVECENT(romReserved,103)
	RVECENT(romReserved,104)
	RVECENT(romReserved,105)
	RVECENT(romReserved,106)
	RVECENT(romReserved,107)
	RVECENT(romReserved,108)
	RVECENT(romReserved,109)
	RVECENT(romReserved,110)
	RVECENT(romReserved,111)
	XVECENT(romExcHandle,0x380)	/* bfc00380: R4000 general vector */
	RVECENT(romReserved,113)
	RVECENT(romReserved,114)
	RVECENT(romReserved,115)
	RVECENT(romReserved,116)
	RVECENT(romReserved,116)
	RVECENT(romReserved,118)
	RVECENT(romReserved,119)
	RVECENT(romReserved,120)
	RVECENT(romReserved,121)
	RVECENT(romReserved,122)
	RVECENT(romReserved,123)
	RVECENT(romReserved,124)
	RVECENT(romReserved,125)
	RVECENT(romReserved,126)
	RVECENT(romReserved,127)
	XVECENT(romExcHandle,0x400)	/* 0xbfc00400: 4Kx fast interrupt */
	RVECENT(romReserved,129)
	RVECENT(romReserved,130)
	RVECENT(romReserved,131)
	RVECENT(romReserved,132)
	RVECENT(romReserved,133)
	RVECENT(romReserved,134)
	RVECENT(romReserved,135)
	RVECENT(romReserved,136)
	RVECENT(romReserved,137)
	RVECENT(romReserved,138)
	RVECENT(romReserved,139)
	RVECENT(romReserved,140)
	RVECENT(romReserved,141)
	RVECENT(romReserved,142)
	RVECENT(romReserved,143)
	XVECENT(romExcHandle,0x480)	/* 0xbfc00480: 4Kx debug interrupt */
	RVECENT(romReserved,145)
	RVECENT(romReserved,146)
	RVECENT(romReserved,147)
	RVECENT(romReserved,148)
	RVECENT(romReserved,149)
	RVECENT(romReserved,150)
	RVECENT(romReserved,151)
	RVECENT(romReserved,152)
	RVECENT(romReserved,153)
	RVECENT(romReserved,154)
	RVECENT(romReserved,155)
	RVECENT(romReserved,156)
	RVECENT(romReserved,157)
	RVECENT(romReserved,158)
	RVECENT(romReserved,159)

	.set reorder

	/* Want this fairly early in output file */
	.asciz "MicroRedBoot Copyright 2009 NewMedia-NET GmbH / DD-WRT.COM"
	.align 4

/*
 * romExcHandle - rom hook to forward exceptions to sysConsoleDump()
 * to dump the system state to the console and panic.  Not for general
 * use.
 */
romExcHandle:
	b	__AR531X_init

/*
 * romReserved - Handle a jump to an unknown vector.
 * void romReserved(void);
 */
romReserved:
	b	__AR531X_init		# just start over

/*
 * romTlbInit - set all TLB entries to invalid on the 4Kc.  This
 * is not strictly needed on cold reset as the tlb shutdown condition
 * is avoided, but it does not discuss warm reset.  Better to be safe.
 */

.macro		romTlbInit

	MFC0_T0_CONFIG1
	and	t0, CFG1_MMUSIZE
	srl	t0, CFG1_MMUSIZE_S
	beqz	t0, 6f			# no TLB entries is the 4Kp
	mtc0	zero, CP0_ENTRYLO0		# set-up invalid TLB entry
	mtc0	zero, CP0_ENTRYLO1
	li	t1, 0x2000		# first page
	mtc0	zero, CP0_PAGEMASK	# 4KB pages
	.set	push
	.set	noreorder
5:
	mtc0	t1, CP0_ENTRYHI		# each entry needs a unique vaddr
	nop
	mtc0	t0, CP0_INDEX		# set TLB index for tlbwr
	nop
	tlbwi
	addi	t0, -1			# decr loop counter
	addi	t1, 0x2000		# next page * 2
	bgez	t0, 5b			# loop
	nop				# BDSLOT
	.set	pop
6:
	nop
.endm

        .macro  lar     reg,addr
        .set    noat
        move    $at,ra                  # save ra
        la      \reg,\addr              # get address into register
        la      ra,x\@                  # get linked address of label
        subu    \reg,\reg,ra            # subtract it from value
        bal     x\@                     # branch and link to label
        nop                             #  to get current actual address
x\@:
        addu    \reg,\reg,ra            # add actual address
        move    ra,$at                  # restore ra
        .set    at
        .endm


#define DCACHE_SIZE(ways, linesize, depth)	\
	MFC0_T0_CONFIG1;			\
	and	ways, t0, CFG1_DA;		\
	srl	ways, CFG1_DA_S;		\
	add	ways, 1;			\
	and	linesize, t0, CFG1_DL;		\
	srl	linesize, CFG1_DL_S;		\
	addi	linesize, 1;			\
	and	depth, t0, CFG1_DS;		\
	srl	depth, CFG1_DS_S;		\
	li	t0, 64;				\
	sll	depth, t0, depth;		\
	li	t0, 1;				\
	sll	linesize, t0, linesize

#define ICACHE_SIZE(ways, linesize, depth)	\
	MFC0_T0_CONFIG1;			\
	and	ways, t0, CFG1_IA;		\
	srl	ways, CFG1_IA_S;		\
	add	ways, 1;			\
	and	linesize, t0, CFG1_IL;		\
	srl	linesize, CFG1_IL_S;		\
	and	depth, t0, CFG1_IS;		\
	srl	depth, CFG1_IS_S;		\
	addi	linesize, 1;			\
	li	t0, 64;				\
	sll	depth, t0, depth;		\
	li	t0, 1;				\
	sll	linesize, t0, linesize

/*
 * romCacheInit - Low level init of the (primary) caches via index
 *                invalidate cache ops.  We do not have to worry
 *                about parity like the R4000.
 */
.macro ramCacheInit
	/* Inval data cache tags: t1=ways, t2=linesize, t3=depth */
  	mfc0	t1, CP0_TAGLO
 	li	t2, 0xfffffC00
  	and	t1, t1, t2
  	mtc0	t1, CP0_TAGLO

	DCACHE_SIZE(t1, t2, t3)
	beqz	t3, 2f			# any d$?
	mul	a1, t1, t3		# number of lines (ways * depth)
	li	a0, KSEG0
1:
	cache	9, 0(a0)		# invalidate data line
	add	a0, t2			# p += linesize
	add	a1, -1			# number of lines - 1
	bgtz	a1, 1b			# any left?
2:

	/* Inval instruction cache tags: t1=ways, t2=linesize, t3=depth */
	ICACHE_SIZE(t1, t2, t3)
	beqz	t3, 2f			# any i$?
	mul	a1, t1, t3		# number of lines
	li	a0, KSEG0
1:
	cache	0, 0(a0)		# invalidate instr line
	add	a0, t2			# p += linesize
	add	a1, -1			# number of lines - 1
	bgtz	a1, 1b			# any left?
2:
	nop
.endm


/*
 * romSizeMemory - initialize SDRAM configuration.
 *   - The 256Mb mode is not supported.
 *   - Only 4 bank devices are supported.
 *
 * MEMDELAY is slow as it's running uncached - not perfect but good enough.
 */

#define MEMDELAY(count, reg)	\
	li	reg, count;	\
9:	addi	reg, -1;	\
	bgtz	reg, 9b

.macro romSizeMemory

	li	a2, KSEG1 | AR531XPLUS_RST_MEMCTL
	lw	t0, 0(a2)
	and	t0, ~RST_MEMCTL_EXT_FB
	or	t0, RST_MEMCTL_EXT_FB
	sw	t0, 0(a2)
	sync

	/*
	**  Setup Memory for 2-2-2 configuration
	*/
	li	a2, KSEG1 | AR531XPLUS_MEM_STMG0R
	li	t0, 0x2265655
	sw	t0, 0(a2)
	sync
	
	li	a2, KSEG1 | AR531XPLUS_MEM_CFG

	lw	t0, 0(a2)		# MEM_CFG
	and	t0, ~(SDRAM_DATA_WIDTH_M | SDRAM_COL_WIDTH_M | \
	         SDRAM_ROW_WIDTH_M|SDRAM_BANKADDR_BITS_M)
	
#if AR531XPLUS_SDRAM_DDR_SDRAM
	li	t1, (((AR531XPLUS_SDRAM_DATA_WIDTH / 8) - 1) << SDRAM_DATA_WIDTH_S)
#else
	li	t1, (((AR531XPLUS_SDRAM_DATA_WIDTH / 8) - 2) << SDRAM_DATA_WIDTH_S)
#endif
    
	/* Column Width */
	or  t1, ((AR531XPLUS_SDRAM_COL_WIDTH - 1) << SDRAM_COL_WIDTH_S)

	/* Row Width */
	or  t1, ((AR531XPLUS_SDRAM_ROW_WIDTH - 1) << SDRAM_ROW_WIDTH_S)
    
	/* Bank Address Bits are 2 */
	or  t1, (1 << SDRAM_BANKADDR_BITS_S)
	
	or  t0, t1
	sw	t0, 0(a2)		# MEM_CFG
	sync
	
	/* Setup Memory Refresh value */
	li	a2, KSEG1 | AR531XPLUS_MEM_REF
	li	t0, AR531XPLUS_SDRAM_MEMORY_REFRESH_VALUE

	sw	t0, 0(a2)		# MEM_REF
	sync
         
	li	a2, KSEG1 | AR531XPLUS_MEM_STMG0R
	lw	t0, 0(a2)		# MEM_TIMING
	and	t0, ~0x3
	ori	t0, 0x1
	
	sw	t0, 0(a2)		# MEM_TIMING
	sync

	/* Place SDRAM into Auto Initialize state */
	li	a2, KSEG1 | AR531XPLUS_MEM_CTRL
	li	t0, 0x3089

	sw	t0, 0(a2)		# MEM_CTRL
	sync
	
1:	
	lw	t0, 0(a2)		# MEM_CTRL
	andi	t1,t0,0x1
	bnez	t1, 1b
	
	nop
.endm

__AR531X_init:
		
#ifndef DEFAULT_PLL

	/* Configure RF LDO to output 1.9 Volts */
	/* BEGIN LDO PROGRAMMING */
#if defined(HAVE_ap61) || defined(HAVE_ap65)	
    /* Power LED enable */
    li	a0, KSEG1|AR2316_GPIO_CR
    lw      t0, 0(a0)
    or      t0, AR2316_POWER_GPIO_BITMAP
    sw      t0, 0(a0)
    sync

    li	a0, KSEG1|AR2316_GPIO_DO
    lw      t0, 0(a0)
    or      t0, AR2316_POWER_GPIO_BITMAP
    sw      t0, 0(a0)
    sync

    /* set ETH warm RESET */
    li	a0, KSEG1|AR2316_RESET
    lw	t0, 0(a0)
    and t0, ~(RESET_EPHY0 | RESET_ENET0)
    sw	t0, 0(a0)
    sync
    
#endif

	li	a0, KSEG1|AR531XPLUS_SPI_CTL
	lw	t0, 0(a0)
	and	t0, ~SPI_CTL_CLK_SEL_MASK
	or	t0, 1 << 24
	sw	t0, 0(a0)
	sync 
	nop
	nop
	nop


#ifdef HAVE_ap51
	/* Enable arbitration */
	li	a0, KSEG1|AR531XPLUS_AHB_ARB_CTL
	lw	t0, 0(a0)
	or	t0, ARB_WLAN
	sw	t0, 0(a0)
	sync
	
	/* Enable global swapping so this looks like a normal BE system */
	li	a0, KSEG1|AR531XPLUS_ENDIAN_CTL
	lw	t0, 0(a0)
	or	t0, CONFIG_WLAN
	sw	t0, 0(a0)
	sync

	/* Wakeup the WMAC via the Sleep Control register in PCI Addr space */
	li	a0, KSEG1|PCI_MAC_SCR
	lw	t0, 0(a0)
	and	t0, ~PCI_MAC_SCR_SLMODE_M
	or	t0, (PCI_MAC_SCR_SLM_FWAKE << PCI_MAC_SCR_SLMODE_S)
	sw	t0, 0(a0)
	sync

	/* Delay for WMAC to stabilize */
	MEMDELAY(10, t1)

	/* Bring WMAC out of RESET */
	li	a0, KSEG1|AR531XPLUS_RESET
	lw	t0, 0(a0)
	or	t0, RESET_WARM_WLAN0_MAC | RESET_WARM_WLAN0_BB
	sw	t0, 0(a0)
	sync

	/* Delay for WMAC to stabilize */
	MEMDELAY(10, t1)

	and	t0, ~(RESET_WARM_WLAN0_MAC | RESET_WARM_WLAN0_BB)
	sw	t0, 0(a0)
	sync

	/* Delay for WMAC to stabilize */
	MEMDELAY(10, t1)

	/* Set disable_a2_warm_reset in BB reg #2 (0x9808) */
	li	a0, KSEG1|(AR531XPLUS_WLANPHY_BASE + (0x2<<2))
	li	t0, 0x00004000
	sw	t0, 0(a0)
	sync
		
	/* 
	** Initiate RF Bank 6 write to configure LDO to 1.9 Volts
	**
	** A Bank 6 register is comprised of 320 bits, hence, the 
	** following series of store words to Bank 6.
	*/ 
	li	a0, KSEG1| (AR531XPLUS_WLANPHY_BASE + (0x27<<2))
	sw	zero, 0(a0)     # 0x00000000
	sw	zero, 0(a0)     # 0x00000000
	sw	zero, 0(a0)     # 0x00000000
	li	t0, 0x02000000  # 0x02000000
	sw	t0, 0(a0)
	sw	zero, 0(a0)     # 0x00000000
	sw	zero, 0(a0)     # 0x00000000
	sw	zero, 0(a0)     # 0x00000000
	sw	zero, 0(a0)     # 0x00000000
	sw	zero, 0(a0)     # 0x00000000
	sw	zero, 0(a0)     # 0x00000000
	li	t0, 0x80150000
	sw	t0, 0(a0)       # 0x80150000
	li	t0, 0x01000000
	sw	t0, 0(a0)       # 0x01000000
	sw	zero, 0(a0)     # 0x00000000
	li	t0, 0x00080000
	sw	t0, 0(a0)       # 0x00080000
	li	t0, 0x00450000
	sw	t0, 0(a0)       # 0x00450000
	li	t0, 0x00060000
	sw	t0, 0(a0)       # 0x00060000
	li	t0, 0x001c0000
	sw	t0, 0(a0)       # 0x001c0000
	li	t0, 0x00c60000
	sw	t0, 0(a0)       # 0x00c60000
	li	t0, 0x008b0000
	sw	t0, 0(a0)       # 0x008b0000
	li	t0, 0x10080000
	sw	t0, 0(a0)       # 0x10080000
	li	t0, 0x00830000
	sw	t0, 0(a0)       # 0x00830000
	li	t0, 0x00a00080
	sw	t0, 0(a0)       # 0x00a00080
	li	t0, 0x0040000d
	sw	t0, 0(a0)       # 0x0040000d
	li	t0, 0x001104eb
	sw	t0, 0(a0)       # 0x001104eb
	li	t0, 0x0000006f
	sw	t0, 0(a0)       # 0x0000006f
	li	t0, 0x00d80031
	sw	t0, 0(a0)       # 0x00d80031
	li	t0, 0x00920b00
	sw	t0, 0(a0)       # 0x00920b00
	li	t0, 0x00810bc0
	sw	t0, 0(a0)       # 0x00810bc0

	li	a0, KSEG1| (AR531XPLUS_WLANPHY_BASE + (0x30<<2))
	sw	zero, 0(a0)

	/* Put the WMAC to sleep via the Sleep Control register */
	li	a0, KSEG1|PCI_MAC_SCR
	lw	t0, 0(a0)
	and	t0, ~PCI_MAC_SCR_SLMODE_M
	or	t0, (PCI_MAC_SCR_SLM_FSLEEP << PCI_MAC_SCR_SLMODE_S)
	sw	t0, 0(a0)
	sync

	/* END of LDO PROGRAMMING */
#endif
		
 	/* Configure PLLC for 184 MHz CPU and 92 MHz AMBA */

	/* Set PLLC_CTL */
	li	a0, KSEG1|AR531XPLUS_PLLC_CTL
	lw	t0, 0(a0)

	/* Choose Ref Div to 5 hence val to be 3 */
	and	t0, ~PLLC_REF_DIV_M
	or	t0, 0x3 << PLLC_REF_DIV_S
         
	/* Choose Div value to 23 */
	and	t0, ~PLLC_FDBACK_DIV_M
	or	t0, 0x17 << PLLC_FDBACK_DIV_S
         
	/* Choose Divby2 value to be 0 */
	and	t0, ~PLLC_ADD_FDBACK_DIV_M
	or	t0, 0 << PLLC_ADD_FDBACK_DIV_S
         
	/* Choose clkc value to be 368 % 2 = 184 Mhz */
	and	t0, ~PLLC_CLKC_DIV_M
	or	t0, 0 << PLLC_CLKC_DIV_S
         
	/* Choose clkm value to be 368 % 2 = 184 Mhz */
	and	t0, ~PLLC_CLKM_DIV_M
	or	t0, 0 << PLLC_CLKM_DIV_S
         
	/* 2c25 for 180 -- 2c5f for 184 */
	/* Store the PLLc Control to be 40/5 * 2 * (0 + 1) * 23= 368 Mhz */
	sw	t0, 0(a0)
	sync 
	
	nop
	nop
	nop
	nop

		
	/* Set CPUCLK_CTL to use clkm / 1 = 184 */
	li	a0, KSEG1|AR531XPLUS_CPUCLK
	lw	t0, 0(a0)
	
	# Choose CLKm                                   
	and	t0, ~CPUCLK_CLK_SEL_M
	or	t0, 0  << CPUCLK_CLK_SEL_S  
         
	and	t0, ~CPUCLK_CLK_DIV_M
	or	t0, 0 << CPUCLK_CLK_DIV_S   # Choose div % 1 
         
	sw	t0, 0(a0)
	sync 
	
	nop
	nop
	nop
	nop
	
	/* Set AMBACLK_CTL to use clkm / 2 = 92MHz */
	li	a0, KSEG1|AR531XPLUS_AMBACLK
	lw	t0, 0(a0)
	
	and	t0, ~AMBACLK_CLK_SEL_M
	or	t0, 0 << AMBACLK_CLK_SEL_S 
         
	and	t0, ~AMBACLK_CLK_DIV_M
	or	t0, 1 << AMBACLK_CLK_DIV_S
         
	sw	t0, 0(a0)
	sync 
	
	/* Set PLL to By pass Mode */
	li	a0, KSEG1|AR531XPLUS_MISCCLK
	li	t0, 0x0	
	sw	t0, 0(a0)
	sync 


	nop
	nop
	nop
	nop
#endif
__AR531X_reboot:

	.set	nomove
	/* Disable all interrupts, select prom exception handlers */
	li	t1, CAUSE_IV
	mtc0	t1, CP0_CAUSE
	nop
	li	t0, ST0_CU0 | ST0_BEV
	mtc0	t0, CP0_STATUS

	/* Clear software interrupts or lurking watchpoints */
	mtc0	zero, CP0_WATCHLO
	mtc0	zero, CP0_WATCHHI
	
	/* Set Interface Control Register */
	/* Disable PCI and Local Bus */
	li	a0, KSEG1|AR531XPLUS_IF_CTL
	li	t0, IF_DISABLED
	sw	t0, 0(a0)
	
	/* Clear any prior AHB errors by writing reset bit */
	li	a0, KSEG1|AR531XPLUS_AHB_ERR0
	li	t0, AHB_ERROR_DET
	sw	t0, 0(a0)

	.set	move
	
#ifdef COBRA_EMUL         
	/* 
	 * Work around for ethernet not working in bootloader 
	 * It seems that ethernet needs to be brought out of
	 * reset for emulation board to work 
	 */
	li	a0, KSEG1|AR531XPLUS_RESET
	and	t0, ~(RESET_ENET0 | RESET_EPHY0)
	sw	t0, 0(a0)
#endif

	/* Give us as long as possible before a clock interrupt */
	li	t0, 1
	mtc0	t0, CP0_COUNT
	mtc0	zero, CP0_COMPARE



	/* Set watchpoint for low memory to debug null pointers */
	li	t0, 0x40000ff8		# G + Ignore 11:3 in check
	mtc0	t0, CP0_WATCHHI
	li	t0, 7			# watch rwx
	mtc0	t0, CP0_WATCHLO

	/*
	 * Enable the HW watchdog early.  Check for cookie left by
	 * the ice to disable the watchdog.
	 */

	li	a0, KSEG1|AR531XPLUS_WDC
	li      a2, KSEG1|AR531XPLUS_SCRATCH0
	lw	t1, 0(a2)
	li	t2, 0x05551212
	bne	t1, t2, 1f
	b	2f
1:	li	a1, KSEG1|AR531XPLUS_WD

	/*
	**  WAR - Bug #14407 - AR5315 watchdog timer CANNOT perform
	**        NMI or system reset.  Therefore, use
	**        Watchdog Interrupt to provide some
	**        watchdog capability
	*/
	
	li	t0, WDC_AHB_INTR
	li	t1, AR531XPLUS_WATCHDOG_TIME
	sw	t1, 0(a1)               # Write new count value
	sync                            # Flush new WD COUNT to hardware
	sw	t0, 0(a0)		# Now enable interrupt
	sync
2:
	romSizeMemory
	romTlbInit

        /* Disable NMI callback after memory is ready */
        li      a0, 0xa0000410
        sw      zero, 0(a0)

	
	/*
	 * Must have cache enabled before cache init.  At this point
	 * we are running in bfc00000 so the text is uncached.
	 */
	mfc0	t0, CP0_CONFIG
	and	t1, t0, CFG_MT		# check for TLB or FM
	sub	t1, CFG_MT_TLB
	beqz	t1, 1f
	/* 4Kp init all fixed mapping modes -- leave U and 23 uncached */
	and	t0, ~(CFG_KU | CFG_K23)
	or	t0, (CONF_CM_UNCACHED << CFG_KU_S) | \
		    (CONF_CM_UNCACHED << CFG_K23_S)
1:
	and	t0, ~CFG_K0MASK
	mtc0	t0, CP0_CONFIG
	nop
	nop
	ramCacheInit		# init/invalidate cache tags
	nop
	li	a0, KSEG1|AR2316_RESET
	lw	t0, 0(a0)
	or	t0, RESET_UART0		# AR2316 uart only has warm reset
	sw	t0, 0(a0)
	lw	zero, 0(a0)		# flush

	and	t0, ~RESET_UART0
	sw	t0, 0(a0)
	lw	zero, 0(a0)		# flush

	li	a0, AR2316_UART_CLOCK_RATE

	div	a0, 115200  # baudrate
	div	a0, 16
	srl	t2, a0, 8
	andi	t1, a0, 0xff		# AR531X_NS16550_DLL_VALUE
	andi	t2, 0xff		# AR531X_NS16550_DLM_VALUE

	/* Init UART @ 38400 baud */
	li	a0, KSEG1|AR2316_UART0
	li	t0, LCR_DLAB		# access divisor latch
	sb	t0, LCR<<2(a0)
	sb	t1, DLL<<2(a0)		# divisor low
	sb	t2, DLM<<2(a0)		# divisor high
	li	t0, CHAR_LEN_8		# 8b mode
	sb	t0, LCR<<2(a0)
	sb	zero, IER<<2(a0)	# disable interrupts

# For 5312, 5315, FIFO length > 1 (lenght = 16 actually)
        li      t0, FCR_EN|FCR_RXCLR|FCR_TXCLR

# otherwise, use the following
#	li	t0, FCR_EN|FCR_RXCLR|FCR_TXCLR */

	sb	t0, FCR<<2(a0)		# reset and enable fifos

	li	t0, MCR_DTR | MCR_RTS 
	sb	t0, MCR<<2(a0)		# no flow ctrl
	nop
	
        la      t0,_text               # dest addr
#       la      t1,_text          # source addr
#	lar	t1,__AR531X_start
        la      t1,_text-0x80400000+BOOTBASE          # source addr
        la      t3,_text+0x10000       # end dest addr
3:      
        lw      v0,0(t1)                # get word
        sw      v0,0(t0)                # write word
        addiu   t1,t1,4
        addiu   t0,t0,4
        bne     t0,t3,3b
        nop
        la      v0,4f                   # RAM address to go to
        jr      v0
        nop
4:       
	nop
	j	main
	nop




#endif	/* ! CONFIG_BOOTLOADER */

#ifndef CYGONCE_HAL_PLATFORM_INC
#define CYGONCE_HAL_PLATFORM_INC
##=============================================================================
##
##	platform.inc
##
##	AR2316 assembler header file
##
##=============================================================================
#####ECOSGPLCOPYRIGHTBEGIN####
## -------------------------------------------
## This file is part of eCos, the Embedded Configurable Operating System.
## Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
##
## eCos is free software; you can redistribute it and/or modify it under
## the terms of the GNU General Public License as published by the Free
## Software Foundation; either version 2 or (at your option) any later version.
##
## eCos is distributed in the hope that it will be useful, but WITHOUT ANY
## WARRANTY; without even the implied warranty of MERCHANTABILITY or
## FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
## for more details.
##
## You should have received a copy of the GNU General Public License along
## with eCos; if not, write to the Free Software Foundation, Inc.,
## 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
##
## As a special exception, if other files instantiate templates or use macros
## or inline functions from this file, or you compile this file and link it
## with other works to produce a work based on this file, this file does not
## by itself cause the resulting work to be covered by the GNU General Public
## License. However the source code for this file must still be made available
## in accordance with section (3) of the GNU General Public License.
##
## This exception does not invalidate any other reasons why a work based on
## this file might be covered by the GNU General Public License.
##
## Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
## at http://sources.redhat.com/ecos/ecos-license/
## -------------------------------------------
#####ECOSGPLCOPYRIGHTEND####
##=============================================================================
#######DESCRIPTIONBEGIN####
##
## Author(s): 	
## Contributors:
## Date:	2003-10-20
## Purpose:	AR2316 SOC definitions.
## Description:	This file contains various definitions and macros that are
##              useful for writing assembly code for the AR2316 SOC
## Usage:
##		#include <cyg/hal/platform.inc>
##		...
##		
##
######DESCRIPTIONEND####
##
##=============================================================================


#include <cyg/hal/mips.inc>

#include <cyg/hal/hal_arch.h>
#include <cyg/hal/plf_io.h>

#include "ar2316reg.h"
#include "ar531xSio.h"

# MEMDELAY is slow as it's running uncached - not perfect but good enough.
#define MEMDELAY(count, reg)	\
	li	reg, count;	\
9:	addi	reg, -1;	\
	bgtz	reg, 9b;	\
	nop

#define K1BASE 0xa0000000
#define K0BASE 0x80000000

#define CFG_MT          0x00000380              /* MMU type */
#define CFG_MT_TLB      0x00000080              /* standard TLB (4Kc) */
#define CFG_KU          0x0e000000              /* k/useg cacheability */
#define CFG_K23         0x70000000              /* kseg2/3 cacheability */
#define CFG_KU_S        25
#define CFG_K0MASK      0x00000007              /* KSEG0 coherency algorithm */
#define CFG_K23_S       28	
#define CFG_C_UNCACHED   2
#define C0_TAGLO        $28             /* Low-order portion of cache tag interface */
#define MFC0_T0_CONFIG1 .word 0x40088001        /* Not built into gas */
#define CFG1_DA_S       7
#define CFG1_DA         0x00000380              /* Num of d$ ways - 1 */
#define CFG1_DL         0x00001c00              /* d$ line size */
#define CFG1_DL_S       10
#define CFG1_DS         0x0000e000              /* Num of d$ sets per way */
#define CFG1_DS_S       13
#define CFG1_IA         0x00070000              /* Num of i$ ways - 1 */
#define CFG1_IA_S       16
#define CFG1_IL         0x00380000              /* i$ line size */
#define CFG1_IL_S       19
#define CFG1_IS         0x01c00000              /* Num of i$ sets per way */
#define CFG1_IS_S       22

	
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
        
#if defined(CYG_HAL_STARTUP_ROMRAM)
#define CYGPKG_HAL_MIPS_LAR_DEFINED
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
#endif        

#------------------------------------------------------------------------------
# Macro for copying vectors to RAM if necessary.
#if !defined(CYGSEM_HAL_USE_ROM_MONITOR)

	.macro	hal_vectors_init
        # If we don~t play nice with a ROM monitor, copy the required
        # vectors into the proper location.
	la	t0,0x80000000		# dest addr
	la	t1,utlb_vector		# source addr
	la	t3,utlb_vector_end	# end dest addr
1:	
	lw	v0,0(t1)		# get word
	addi	t1,t1,4
	sw	v0,0(t0)		# write word
	addi	t0,t0,4
	bne	t1,t3,1b
	 nop

	la	t0,0x80000180		# dest addr
	la	t1,other_vector		# source addr
	la	t3,other_vector_end	# end dest addr
1:	
	lw	v0,0(t1)		# get word
	addi	t1,t1,4
	sw	v0,0(t0)		# write word
	addi	t0,t0,4
	bne	t1,t3,1b
	 nop

        .set mips3                      # Set ISA to MIPS 3 to allow cache insns
	# Now clear the region in the caches
	la	t0,0x80000000		# dest addr
	ori	t1,t0,0x200		# source addr
1:	cache	0x01,0x0000(t0)	        # Flush word from data cache
	cache	0x01,0x1000(t0)
	cache	0x01,0x2000(t0)
	cache	0x01,0x3000(t0)
	nop
	cache	0x00,0x0000(t0)		# Invalidate icache for word
	cache	0x00,0x1000(t0)
	cache	0x00,0x2000(t0)
	cache	0x00,0x3000(t0)
	nop
	addi	t0,t0,0x20
	bne	t0,t1,1b
	 nop
        .set mips0                      # reset ISA to default

	.endm

#else

	.macro	hal_vectors_init
	.endm

#endif

	.macro hal_serial_init

	li	a0, K1BASE|AR2316_RESET
	lw	t0, 0(a0)
	or	t0, RESET_UART0		# AR2316 uart only has warm reset
	sw	t0, 0(a0)
	lw	zero, 0(a0)		# flush

	and	t0, ~RESET_UART0
	sw	t0, 0(a0)
	lw	zero, 0(a0)		# flush

	li	a0, AR2316_UART_CLOCK_RATE

	div	a0, DESIRED_BAUD_RATE
	div	a0, 16
	srl	t2, a0, 8
	andi	t1, a0, 0xff		# AR531X_NS16550_DLL_VALUE
	andi	t2, 0xff		# AR531X_NS16550_DLM_VALUE

	/* Init UART @ 38400 baud */
	li	a0, K1BASE|AR2316_UART0
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
	.endm	

#------------------------------------------------------------------------------
# Monitor initialization.
	
#ifndef CYGPKG_HAL_MIPS_MON_DEFINED

#if defined(CYG_HAL_STARTUP_ROM) || defined(CYG_HAL_STARTUP_ROMRAM)
	# If we are using a ROM version, include code to
	# initialize PLLs and SDRAM.

#define CYGPKG_HAL_RESET_VECTOR_FIRST_CODE
	.macro	hal_reset_vector_first_code
#ifdef CYGPKG_HAL_MIPS_AP61	
    /* Power LED enable */
    li	a0, K1BASE|AR2316_GPIO_CR
    lw      t0, 0(a0)
    or      t0, AR2316_POWER_GPIO_BITMAP
    sw      t0, 0(a0)
    sync

    li	a0, K1BASE|AR2316_GPIO_DO
    lw      t0, 0(a0)
    or      t0, AR2316_POWER_GPIO_BITMAP
    sw      t0, 0(a0)
    sync

    /* set ETH warm RESET */
    li	a0, K1BASE|AR2316_RESET
    lw	t0, 0(a0)
    and t0, ~(RESET_EPHY0 | RESET_ENET0)
    sw	t0, 0(a0)
    sync
    
#endif

    li	a0, K1BASE|AR2316_SPI_CTL
    lw	t0, 0(a0)
    and	t0, ~SPI_CTL_CLK_SEL_MASK
    or	t0, 1 << 24
    sw	t0, 0(a0)
    sync 
    nop
    nop
    nop

/** AR5315 and AR5316 requires this work around. */
#ifdef CYGNUM_USE_LDO_WORK_AROUND
	/* Configure RF LDO to output 1.9 Volts */
	/* BEGIN LDO PROGRAMMING */

	/* Enable arbitration */
	li	a0, K1BASE|AR2316_AHB_ARB_CTL
	lw	t0, 0(a0)
	or	t0, ARB_WLAN
	sw	t0, 0(a0)
	sync
	
	/* Enable global swapping so this looks like a normal BE system */
	li	a0, K1BASE|AR2316_ENDIAN_CTL
	lw	t0, 0(a0)
	or	t0, CONFIG_WLAN
	sw	t0, 0(a0)
	sync

	/* Wakeup the WMAC via the Sleep Control register in PCI Addr space */
	li	a0, K1BASE|PCI_MAC_SCR
	lw	t0, 0(a0)
	and	t0, ~PCI_MAC_SCR_SLMODE_M
	or	t0, (PCI_MAC_SCR_SLM_FWAKE << PCI_MAC_SCR_SLMODE_S)
	sw	t0, 0(a0)
	sync

	/* Delay for WMAC to stabilize */
	MEMDELAY(10, t1)

	/* Bring WMAC out of RESET */
	li	a0, K1BASE|AR2316_RESET
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
	li	a0, K1BASE|(AR2316_WLANPHY_BASE + (0x2<<2))
	li	t0, 0x00004000
	sw	t0, 0(a0)
	sync
		
	/* 
	** Initiate RF Bank 6 write to configure LDO to 1.9 Volts
	**
	** A Bank 6 register is comprised of 320 bits, hence, the 
	** following series of store words to Bank 6.
	*/ 
	li	a0, K1BASE| (AR2316_WLANPHY_BASE + (0x27<<2))
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

	li	a0, K1BASE| (AR2316_WLANPHY_BASE + (0x30<<2))
	sw	zero, 0(a0)

	/* Put the WMAC to sleep via the Sleep Control register */
	li	a0, K1BASE|PCI_MAC_SCR
	lw	t0, 0(a0)
	and	t0, ~PCI_MAC_SCR_SLMODE_M
	or	t0, (PCI_MAC_SCR_SLM_FSLEEP << PCI_MAC_SCR_SLMODE_S)
	sw	t0, 0(a0)
	sync
	/* END of LDO PROGRAMMING */
#endif

 	/* Configure PLLC for 184 MHz CPU and 92 MHz AMBA */

	/* Set PLLC_CTL */
	li	a0, K1BASE|AR2316_PLLC_CTL
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
         
	/* Store the PLLc Control to be 40/5 * 2 * (0 + 1) * 23= 368 Mhz */
	sw	t0, 0(a0)
	sync 
	
	nop
	nop
	nop
	nop

		
	/* Set CPUCLK_CTL to use clkm / 1 = 184 */
	li	a0, K1BASE|AR2316_CPUCLK
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
	
	/* Set AMBACLK_CTL to use clkc / 2 = 92MHz */
	li	a0, K1BASE|AR2316_AMBACLK
	lw	t0, 0(a0)
	
	and	t0, ~AMBACLK_CLK_SEL_M
	or	t0, 0 << AMBACLK_CLK_SEL_S
         
	and	t0, ~AMBACLK_CLK_DIV_M
	or	t0, 1 << AMBACLK_CLK_DIV_S
         
	sw	t0, 0(a0)
	sync 
	
	/* Set PLL to By pass Mode */
	li	a0, K1BASE|AR2316_MISCCLK
	li	t0, 0x0	
	sw	t0, 0(a0)
	sync 
	
	nop
	nop
	nop
	nop

	/* disable PCI & Local Bus Interface on 2316 by default */
	li	a0, K1BASE|AR2316_IF_CTL
	li	t0, IF_DISABLED
	sw	t0, 0(a0)
	sync

	
	nop
	nop
	nop
	nop

	.endm

#define CYGPKG_HAL_MIPS_MEMC_DEFINED

	# Initialize SDRAM configuration.
	#   - The 256Mb mode is not supported.
	#   - Only 4 bank devices are supported.
	#
	.macro	hal_memc_init
	/*
	 *  Setup Memory for 2-2-2 configuration
	 */
	li	a2, K1BASE | AR2316_MEM_STMGOR
	li	t0, 0x2265655
	sw	t0, 0(a2)
	sync

	li	a2, K1BASE | AR2316_MEM_CFG

	lw	t0, 0(a2)		# MEM_CFG
	and	t0, ~(SDRAM_DATA_WIDTH_M | SDRAM_COL_WIDTH_M | \
	         SDRAM_ROW_WIDTH_M|SDRAM_BANKADDR_BITS_M)
	
#if AR2316_SDRAM_DDR_SDRAM
	li	t1, (((AR2316_SDRAM_DATA_WIDTH / 8) - 1) << SDRAM_DATA_WIDTH_S)
#else
	li	t1, (((AR2316_SDRAM_DATA_WIDTH / 8) - 2) << SDRAM_DATA_WIDTH_S)
#endif
    
	/* Column Width */
	or  t1, ((AR2316_SDRAM_COL_WIDTH - 1) << SDRAM_COL_WIDTH_S)

	/* Row Width */
	or  t1, ((AR2316_SDRAM_ROW_WIDTH - 1) << SDRAM_ROW_WIDTH_S)
    
	/* Bank Address Bits are 2 */
	or  t1, (1 << SDRAM_BANKADDR_BITS_S)
	
	or  t0, t1
	sw	t0, 0(a2)		# MEM_CFG
	sync
	
	/* Setup Memory Refresh value */
	li	a2, K1BASE | AR2316_MEM_REF
	li	t0, AR2316_SDRAM_MEMORY_REFRESH_VALUE

	sw	t0, 0(a2)		# MEM_REF
	sync

	/* Place SDRAM into Auto Initialize state */
	li	a2, K1BASE | AR2316_MEM_CTRL
	li	t0, 0x3089

	sw	t0, 0(a2)		# MEM_CTRL
	sync
	
1:	
	lw	t0, 0(a2)		# MEM_CTRL
	andi	t1,t0,0x1
	bnez	t1, 1b

#if defined(CYG_HAL_STARTUP_ROMRAM)
        # Having got the RAM working, we must now relocate the Entire
        # ROM into it and then continue execution from RAM.

        la      t0,reset_vector         # dest addr
        lar     t1,reset_vector         # source addr
        la      t3,__ram_data_end       # end dest addr
1:      
        lw      v0,0(t1)                # get word
        sw      v0,0(t0)                # write word
        addiu   t1,t1,4
        bne     t0,t3,1b
        addiu   t0,t0,4
        nop
        la      v0,2f                   # RAM address to go to
        jr      v0
        nop
2:      
        # We are now executing out of RAM!
#endif
			
	.endm
#endif /* CYG_HAL_STARTUP_ROM */

#if	defined(CYG_HAL_STARTUP_ROM) ||			\
        defined(CYG_HAL_STARTUP_ROMRAM) ||              \
	(	defined(CYG_HAL_STARTUP_RAM) &&		\
		!defined(CYGSEM_HAL_USE_ROM_MONITOR))
	
	# If we are starting up from ROM, or we are starting in
	# RAM and NOT using a ROM monitor, initialize the VSR table.

	.macro	hal_mon_init

	la	a0,__default_interrupt_vsr
	la	a1,__default_exception_vsr
	la	a3,hal_vsr_table
	sw	a0,0(a3)
	sw	a1,1*4(a3)
	sw	a1,2*4(a3)
	sw	a1,3*4(a3)
	sw	a1,4*4(a3)
	sw	a1,5*4(a3)
	sw	a1,6*4(a3)
	sw	a1,7*4(a3)
	sw	a1,8*4(a3)
	sw	a1,9*4(a3)
	sw	a1,10*4(a3)
	sw	a1,11*4(a3)
	sw	a1,12*4(a3)
	sw	a1,13*4(a3)
	sw	a1,14*4(a3)
	sw	a1,15*4(a3)

	sw	a1,32*4(a3)
	sw	a1,33*4(a3)
	sw	a1,34*4(a3)
	.endm

#elif defined(CYG_HAL_STARTUP_RAM) && defined(CYGSEM_HAL_USE_ROM_MONITOR)

	# Initialize the VSR table entries
	# We only take control of the interrupt vector,
	# the rest are left to the ROM for now...

	.macro	hal_mon_init
	la	a0,__default_interrupt_vsr
	la	a3,hal_vsr_table
	sw	a0,0(a3)
	.endm

#else

	.macro	hal_mon_init
	.endm

#endif

#if defined(CYGPKG_HAL_MIPS_CACHE_DEFINED)
	.macro	hal_cache_init
    #enable cache
    mfc0	t0, config0
	and	t1, t0, CFG_MT		# check for TLB or FM
	sub	t1, CFG_MT_TLB
	beqz	t1, 1f
	/* 4Kp init all fixed mapping modes -- leave U and 23 uncached */
	and	t0, ~(CFG_KU | CFG_K23)
	or	t0, (CFG_C_UNCACHED << CFG_KU_S) | \
		    (CFG_C_UNCACHED << CFG_K23_S)
1:
	and	t0, ~CFG_K0MASK
	/* for 4Kec  cache mode CFG_C_UNCACHED is write back but 0 is write through */
	mtc0	t0, config0
	nop
	nop
	/* Inval data cache tags: t1=ways, t2=linesize, t3=depth */
  	mfc0	t1, C0_TAGLO
 	li	t2, 0xfffffC00
  	and	t1, t1, t2
  	mtc0	t1, C0_TAGLO

	DCACHE_SIZE(t1, t2, t3)
	beqz	t3, 2f			# any d$?
	mul	a1, t1, t3		# number of lines (ways * depth)
	li	a0, K0BASE
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
	li	a0, K0BASE
1:
	cache	0, 0(a0)		# invalidate instr line
	add	a0, t2			# p += linesize
	add	a1, -1			# number of lines - 1
	bgtz	a1, 1b			# any left?
2:
	nop
	nop
	sync

	.endm
#endif


#define CYGPKG_HAL_MIPS_MON_DEFINED

#endif	

#------------------------------------------------------------------------------
#endif // ifndef CYGONCE_HAL_PLATFORM_INC
# end of platform.inc

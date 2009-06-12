#ifndef CYGONCE_HAL_IDT79RC233X_H
#define CYGONCE_HAL_IDT79RC233X_H
//==========================================================================
//
//      idt79rc233x.h
//
//      IDT 79RC233x platform definitions
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    Tim Michals
// Contributors: nickg
// Date:         2003-02-13
// Purpose:      IDT 79RC233x platform definitions
// Description:  
// Usage:        
//####DESCRIPTIONEND####
//==========================================================================

#define	K0BASE		0x80000000
#define	K0SIZE		0x20000000
#define	K1BASE		0xA0000000
#define	K1SIZE		0x20000000
#define	K2BASE		0xC0000000
#define	K2SIZE		0x40000000

/*
* Exception vectors
*/

#define	T_VEC	       	 K0BASE			/* tlbmiss vector */
#define	C_VEC		(K1BASE+0x100)		/* cache exception vector */
#define	E_VEC		(K0BASE+0x180)		/* exception vector */
#define	R_VEC		(K1BASE+0x1fc00000)	/* reset vector */

/*
 * Address conversion macros
 */

#define	K0_TO_K1(x)	((unsigned)(x)|0xA0000000)	/* kseg0 to kseg1 */
#define	K1_TO_K0(x)	((unsigned)(x)&0x9FFFFFFF)	/* kseg1 to kseg0 */
#define	K0_TO_PHYS(x)	((unsigned)(x)&0x1FFFFFFF)	/* kseg0 to physical */
#define	K1_TO_PHYS(x)	((unsigned)(x)&0x1FFFFFFF)	/* kseg1 to physical */
#define	PHYS_TO_K0(x)	((unsigned)(x)|0x80000000)	/* physical to kseg0 */
#define	PHYS_TO_K1(x)	((unsigned)(x)|0xA0000000)	/* physical to kseg1 */

/*
* Address predicates
*/

#define	IS_KSEG0(x)	((unsigned)(x) >= K0BASE && (unsigned)(x) < K1BASE)
#define	IS_KSEG1(x)	((unsigned)(x) >= K1BASE && (unsigned)(x) < K2BASE)
#define	IS_KUSEG(x)	((unsigned)(x) < K0BASE)

/*
* Cache size constants
*/

#define	MINCACHE	+(1*1024)	/* leading plus for mas's benefit */
#define	MAXCACHE	+(256*1024)	/* leading plus for mas's benefit */

/*
* Cache alignment macros
*
* NOTE: These definitions may migrate to vxWorks.h in a future release.
*/

#define	CACHE_ROUND_UP(x)	ROUND_UP(x, _CACHE_ALIGN_SIZE)
#define	CACHE_ROUND_DOWN(x)	ROUND_DOWN(x, _CACHE_ALIGN_SIZE)

/*
* Cause bit definitions
*/

#define	CAUSE_BD	0x80000000	/* Branch delay slot */
#define	CAUSE_CEMASK	0x30000000	/* coprocessor error */
#define	CAUSE_CESHIFT	28

#define	CAUSE_IP8	0x00008000	/* External level 8 pending */
#define	CAUSE_IP7	0x00004000	/* External level 7 pending */
#define	CAUSE_IP6	0x00002000	/* External level 6 pending */
#define	CAUSE_IP5	0x00001000	/* External level 5 pending */
#define	CAUSE_IP4	0x00000800	/* External level 4 pending */
#define	CAUSE_IP3	0x00000400	/* External level 3 pending */
#define	CAUSE_SW2	0x00000200	/* Software level 2 pending */
#define	CAUSE_SW1	0x00000100	/* Software level 1 pending */

#define	CAUSE_IPMASK	0x0000FF00	/* Pending interrupt mask */
#define	CAUSE_IPSHIFT	8

#define	CAUSE_EXCMASK	0x0000007C	/* Cause code bits */
#define	CAUSE_EXCSHIFT	2

/*
* Status definition bits
*/

#define	SR_CUMASK	0xf0000000	/* coproc usable bits */
#define	SR_CU3		0x80000000	/* Coprocessor 3 usable */
#define	SR_CU2		0x40000000	/* Coprocessor 2 usable */
#define	SR_CU1		0x20000000	/* Coprocessor 1 usable */
#define	SR_CU0		0x10000000	/* Coprocessor 0 usable */
#define SR_NBL		0x08000000      /* Enables Non Blocking Load */
#define SR_RE		0x02000000      /* Reverse endian in user mode */
#define SR_DL		0x01000000      /* Data Cache Lock Enable      */ 
#define SR_IL		0x00800000      /* Instruction Cache Lock Enable */

#define	SR_BEV		0x00400000	/* use boot exception vectors */
#define SR_SR		0x00100000	/* soft reset occurred */
#define	SR_CE		0x00020000	/* use ECC reg */
#define	SR_DE		0x00010000	/* disable cache errors */
#define	SR_IMASK	0x0000ff00	/* Interrupt mask */
#define	SR_IMASK8	0x00000000	/* mask level 8 */
#define	SR_IMASK7	0x00008000	/* mask level 7 */
#define	SR_IMASK6	0x0000c000	/* mask level 6 */
#define	SR_IMASK5	0x0000e000	/* mask level 5 */
#define	SR_IMASK4	0x0000f000	/* mask level 4 */
#define	SR_IMASK3	0x0000f800	/* mask level 3 */
#define	SR_IMASK2	0x0000fc00	/* mask level 2 */
#define	SR_IMASK1	0x0000fe00	/* mask level 1 */
#define	SR_IMASK0	0x0000ff00	/* mask level 0 */

#define	SR_IBIT8	0x00008000	/* bit level 8 */
#define	SR_IBIT7	0x00004000	/* bit level 7 */
#define	SR_IBIT6	0x00002000	/* bit level 6 */
#define	SR_IBIT5	0x00001000	/* bit level 5 */
#define	SR_IBIT4	0x00000800	/* bit level 4 */
#define	SR_IBIT3	0x00000400	/* bit level 3 */
#define	SR_IBIT2	0x00000200	/* bit level 2 */
#define	SR_IBIT1	0x00000100	/* bit level 1 */

#define SR_KSU_K	0x00000000	/* kernel mode */
#define SR_KSU_U	0x00000010	/* user mode */
#define	SR_ERL		0x00000004	/* Error Level */
#define	SR_EXL		0x00000002	/* Exception Level */
#define	SR_IE		0x00000001	/* interrupt enable, 1 => enable */

#define	SR_IMASKSHIFT	8

/*
* tlb definitions
*/

#define	TLB_ENTRIES		16
#define	TLBLO_PFNMASK		0x03ffffc0
#define	TLBLO_PFNSHIFT		6
#define TLBLO_CMASK		0x00000038
#define TLBLO_NC		0x00000010 /* uncached */
#define TLBLO_NONC		0x00000018 /* cacheable non-coherent */
#define	TLBLO_D			0x4		/* writeable */
#define	TLBLO_V			0x2		/* valid bit */
#define	TLBLO_G			0x1		/* global bit */

#define	TLBHI_VPN2MASK		0xffffe000
#define	TLBHI_VPN2SHIFT		13
#define	TLBHI_PIDMASK		0xff
#define	TLBHI_PIDSHIFT		0
#define	TLBHI_NPID		256
#define	TLBINX_PROBE		0x80000000
#define	TLBINX_INXMASK		0x0000003f
#define	TLBINX_INXSHIFT		0
#define	TLBRAND_RANDMASK	0x0000000f
#define	TLBRAND_RANDSHIFT	0
#define	TLBCTXT_BASEMASK	0xff800000
#define	TLBCTXT_BASESHIFT	23
#define	TLBCTXT_VPN2MASK	0x007ffff0
#define	TLBCTXT_VPN2SHIFT	4

/*
 * RC32364 Config Register 
 */
#define CFG_ICE		0x80000000	/* In Circuit Emulator existence */
#define CFG_ECMASK	0x70000000	/* System Clock Ratio */
#define CFG_ECBY2	0x00000000 	/* divide by 2 */
#define CFG_ECBY3	0x10000000 	/* divide by 3 */
#define CFG_ECBY4	0x20000000 	/* divide by 4 */
#define CFG_NBL         0x00800000      /* Non Blocking load */
#define CFG_BE		0x00008000	/* Big Endian */
#define CFG_ICMASK	0x00000e00	/* Instruction cache size */
#define CFG_ICSHIFT	9
#define CFG_DCMASK	0x000001c0	/* Data cache size */
#define CFG_DCSHIFT	6
#define CFG_IB		0x00000020	/* Instruction cache block size */
#define CFG_DB		0x00000010	/* Data cache block size */
#define CFG_K0MASK	0x00000007	/* KSEG0 coherency algorithm */

/*
 * Primary cache mode
 */
#define CFG_C_UNCACHED		2
#define CFG_C_NONCOHERENT	3

/* 
 * Primary Cache TagLo 
 */
#define TAG_PTAG_MASK           0x7fffff00      /* Primary Tag */
#define TAG_PTAG_SHIFT          0x00000008
#define TAG_PSTATE_MASK         0x000000c0      /* Primary Cache State */
#define TAG_PSTATE_SHIFT        0x00000006
#define TAG_LOCK_BIT            0x00000004      /* Cache line lock bit */
#define TAG_LOCK_BIT_SHIFT      0x00000002
#define TAG_FIFO_REFILL         0x00000002      /* Fifo refill         */
#define TAG_FIFO_REFILL_SHIFT   0x00000001
#define TAG_PARITY_MASK         0x00000001      /* Primary Tag Parity */
#define TAG_PARITY_SHIFT        0x00000000



/*
 * CacheErr register
 */
#define CACHEERR_TYPE		0x80000000	/* reference type: 
						   0=Instr, 1=Data */
#define CACHEERR_LEVEL		0x40000000	/* cache level:
						   0=Primary, 1=reserved */
#define CACHEERR_DATA		0x20000000	/* data field:
						   0=No error, 1=Error */
#define CACHEERR_TAG		0x10000000	/* tag field:
						   0=No error, 1=Error */
#define CACHEERR_BOTH		0x02000000	/* Data & Instruction error:
						   0=No, 1=Yes */
#define CACHEERR_SIDX_MASK	0x003ffff8	/* PADDR(21..3) */
#define CACHEERR_SIDX_SHIFT		 3
#define CACHEERR_PIDX_MASK	0x00000003	/* VADDR(13..12) */
#define CACHEERR_PIDX_SHIFT	        12 	


/*
 * Cache operations
 */
#define Index_Invalidate_I               0x0         /* 0       0 */
#define Index_Writeback_Inv_D            0x1         /* 0       1 */
#define Index_Load_Tag_I                 0x4         /* 1       0 */
#define Index_Load_Tag_D                 0x5         /* 1       1 */
#define Index_Store_Tag_I                0x8         /* 2       0 */
#define Index_Store_Tag_D                0x9         /* 2       1 */
#define Create_Dirty_Exc_D               0xD         /* 3       1 */
#define Hit_Invalidate_I                 0x10        /* 4       0 */
#define Hit_Invalidate_D                 0x11        /* 4       1 */
#define Hit_Writeback_Inv_D              0x15        /* 5       1 */
#define Fill_I                           0x14        /* 5       0 */
#define Hit_Writeback_D                  0x19        /* 6       1 */
#define Hit_Writeback_I                  0x18        /* 6       0 */

/*
* Coprocessor 0 operations
*/

#define	C0_READI  0x1		/* read ITLB entry addressed by C0_INDEX */
#define	C0_WRITEI 0x2		/* write ITLB entry addressed by C0_INDEX */
#define	C0_WRITER 0x6		/* write ITLB entry addressed by C0_RAND */
#define	C0_PROBE  0x8		/* probe for ITLB entry addressed by TLBHI */
#define	C0_ERET	  0x18		/* restore for exception */
#define FP_EXC_MASK     (FP_EXC_I|FP_EXC_U|FP_EXC_O|FP_EXC_Z|FP_EXC_V|FP_EXC_E)
#define FP_EXC_I        0x1000          /* inexact operation */
#define FP_EXC_U        0x2000          /* underflow */
#define FP_EXC_O        0x4000          /* overflow */
#define FP_EXC_Z        0x8000          /* divide by zero */
#define FP_EXC_V        0x10000         /* invalid operation */
#define FP_EXC_E        0x20000         /* unimplemented operation */

							  
#define C0_INX        $0              /* Index into TLB Array - 4Kc core */
#define C0_RANDOM       $1              /* Randomly generated index into TLB Array - 4Kc core */
#define C0_TLBLO0     $2              /* Low-order portion of the TLB entry for even-numbered virtual pages - 4Kc core */
#define C0_TLBLO1     $3              /* Low-order portion of the TLB entry for odd-numbered virtual pages - 4Kc core */
#define C0_PAGEMASK     $5              /* Pointer to page table entry in memory - 4Kc core */
#define C0_WIRED        $6              /* Number of fixed TLB entries - 4Kc core */
#define C0_TLBHI      $10             /* High-order portion of the TLB entry - 4Kc core */
#define C0_PRId         $15             /* Processor Identification and Revision */
#define C0_CONFIG       $16             /* Configuration Register */
#define C0_LLADDR       $17             /* Load linked address */
#define C0_LLADDR       $17             /* Load linked address */
#define C0_DEBUG        $23             /* Debug control and exception status */
#define C0_DEPC         $24             /* Program counter at last debug exception */
#define C0_TAGLO        $28             /* Low-order portion of cache tag interface */
#define C0_TAGHI        $29             /* High-order portion of cache tag interface (not implemented in 4K cores */
#define C0_DESAVE       $31             /* Debug handler scratch pad register */




#define TARGET_S334
#define BUS		                 0

#define PORT_WIDTH_CNTL_REG     0xffffe200
#define BUS_TURN_AROUND_REG     0xffffe204
#define BUS_TURN_AROUND_CNTRL_REG     0xb8000000
#define BUS_TURN_AROUND_VAL     0x00000000 

#define ADDRESS_LATCH_TIMING_REG	0xB8000004
#define ADDRESS_LATCH_TIMING_VAL	0x00000007

#define PORT_WIDTH_CNTL_VAL     0xaa822aaa
#define SDRAM_TEST_PATTERN      0xaa55aa55

/* RC32134 Register Settings               */
#define MEM_BASE_BASE           0xb8000080
#define MBA_REG0                0x1fc00000
#define MBM_REG0                0xffC00000


#define MEM_CTL_BASE            0xb8000200
#define MCR_CS0_BS              0x23ef23ef
#define MCR_CS1_BS              0x28632863
#define MCR_CS2_BS              0x60e760e7
#define MCR_CS3_BS              0x60e760e7  /* NVRAM    */
#define MCR_CS4_BS              0x60e760e7  /* S334 LED */
#define MCR_CS5_BS              0x60e760e7


#define RHEA_IREG_BASE          0xb8000000
#define SODIMM                     1

#define DRAM_BNK0_BASE          0x00000000
#define DRAM_BNK1_BASE          0x01000000
#define DRAM_BNK2_BASE          0x02000000
#define DRAM_BNK3_BASE          0x03000000
#define DRAM_BNK0_MASK          0xff000000
#define DRAM_BNK1_MASK          0xff000000
#define DRAM_BNK2_MASK          0xff000000
#define DRAM_BNK3_MASK          0xff000000
#define MBA_REG1                0x04000000
#define MBM_REG1                0xffff0000
#define SDRAM_CR_BS             0x8955c0ff
#define SDRAM_PC_VAL            0x8955c0a0
#define SDRAM_RFRSH_CMD         0x8955c090
#define SDRAM_MODE_REG          0x8955c080
#define SDRAM_CSEL_PARK         0x8955c0ff
#define TIMER_BASE              0xb8000700
#define DRAM_RF_CMPR_BS         0x00000040
#define DRAM_RF_CMPR_SE_BS      0x00000200
#define CPU_BERR_BS             0xff
#define IP_BERR_BS              0xff
#define DISABLE_TIMER           0x0
#define ENABLE_TIMER            0x1

#define CPU_CLOCK_RATE	        75000000


/* define macro so drivers will call sysWbFlush() */

#define SYS_WB_FLUSH

/* task default status register */

#define INT_LVL_PCI        INT_LVL_IORQ1 
#define INT_LVL_SR_IMASK  (INT_LVL_PCI | INT_LVL_IORQ3 |\
                           INT_LVL_SW0 | INT_LVL_SW1 )

#define RC32364_SR        (SR_CU0| INT_LVL_SR_IMASK |\
                           INT_LVL_TIMER | SR_IE)

/* interrupt priority */

#define INT_PRIO_MSB       TRUE            /* interrupt priority msb highest */




/* Miscellaneous */

#define PIO_DATA_REG0         0xb8000600
#define PIO_FUNC_SELECT_REG0  0xb8000608
#define PIO_DATA_REG1         0xb8000610
#define PIO_DIRCNTL_REG1      0xb8000614
#define PIO_FUNC_SELECT_REG1  0xb8000618
#define CYG_MGMT_LED_MASK     0x00000008
#define CYG_STATUS_LED_MASK   0x00000003
#define CYG_TEST_LED1_MASK    0x00000002
#define CYG_TEST_LED2_MASK    0x00000004
#define CYG_STATUS_LED_GREEN  0x00000001
#define CYG_STATUS_LED_ORANGE 0x00000002




/* PIO definition for Internal Uart   */

#define PIO_DIRCNTL_REG         0xb8000604
#define PIO_FUNCSEL_MASK        0xf0
#define PIO_DIRCNTL_MASK        0xffffff0f
#define PIO_DIRCNTL_VAL         0x50


/* Serial grouping */

#define SERIAL_PORT0_GROUP		5
#define SERIAL_PORT1_GROUP		6

/* Rc32134 Interrupt controller settings for Uart */
#define INTR_STATUS_PTR         0xb8000500
#define INTR_MASK_REG           0xb8000504
#define INTR_CLEAR_REG          0xb8000508

#define INTR_COM0_REG           0xb8000554
#define INTR_COM1_REG           0xb8000564

#define INTR_CLEAR_COM0         0xb8000558
#define INTR_PEND_COM0          0xb8000550
#define INTR_CLEAR_COM1         0xb8000568
#define INTR_PEND_COM1          0xb8000560

#define INTR_CLEAR_MASTER       0xb8000508
#define INTR_PEND_MASTER        0xb8000500

#define INTR_MASTERMASK_COM1    0x0020
#define INTR_MASTERMASK_COM2    0x0040
#define INTR_MASTERMASK_UART    ( INTR_MASTERMASK_COM1 | INTR_MASTERMASK_COM2 )

/* Rc32134 Timer0(used as Auxiliary clock)interrupts */
#define AUX_TIMER_INTR_PEND     0xb8000540
#define AUX_TIMER_INTR_MASK     0xb8000544
#define AUX_TIMER_INTR_CLEAR    0xb8000548
#define INTR_MASTERMASK_TIMER0  0x0010
#define AUX_TIMER_CNTL_REG      0xb8000700
#define AUX_TIMER_CNT_REG       0xb8000704
#define AUX_TIMER_CMP_REG       0xb8000708

#define AUX_CLOCK_FREQ          (2 * NS16550_XTAL_FREQ)   

/* Rc32364 Tlb attributes for PCI transactions */
#define PCI_MMU_PAGEMASK         0x00000fff
#define MMU_PAGE_UNCACHED        0x00000010
#define MMU_PAGE_DIRTY           0x00000004
#define MMU_PAGE_VALID           0x00000002
#define MMU_PAGE_GLOBAL          0x00000001
#define PCI_MMU_PAGEATTRIB       (MMU_PAGE_UNCACHED|MMU_PAGE_DIRTY|\
                                  MMU_PAGE_VALID|MMU_PAGE_GLOBAL)
#define PCI_MEMORY_SPACE1        0x40000000
#define PCI_MEMORY_SPACE2        0x60000000
#define PCI_IO_SPACE             0x18000000
#define PCI_PAGE_SIZE            0x01000000
#define TLB_HI_MASK              0xffffe000
#define TLB_LO_MASK              0x3fffffff
#define PAGEMASK_SHIFT                   13
#define TLB_LO_SHIFT                      6


/* RC32134 PCI definitions */
#define PCI_CONFIG_ADDR_REG      0xb8002cf8
#define PCI_CONFIG_DATA_REG      0xb8002cfc

/* Rhea's Configuration Address*/

#define RHEA_CONFIG0_ADDR        0x80000000
#define RHEA_CONFIG1_ADDR        0x80000004
#define RHEA_CONFIG2_ADDR        0x80000008
#define RHEA_CONFIG3_ADDR        0x8000000c
#define RHEA_CONFIG4_ADDR        0x80000010
#define RHEA_CONFIG5_ADDR        0x80000014
#define RHEA_CONFIG6_ADDR        0x80000018
#define RHEA_CONFIG7_ADDR        0x8000001c
#define RHEA_CONFIG8_ADDR        0x80000020
#define RHEA_CONFIG9_ADDR        0x80000024
#define RHEA_CONFIG10_ADDR       0x80000028
#define RHEA_CONFIG11_ADDR       0x8000002c
#define RHEA_CONFIG12_ADDR       0x80000030
#define RHEA_CONFIG13_ADDR       0x80000034
#define RHEA_CONFIG14_ADDR       0x80000038
#define RHEA_CONFIG15_ADDR       0x8000003c
#define RHEA_CONFIG16_ADDR       0x80000040

/* Rhea's configuration Header */

#define RHEA_PCI_CONFIG0       0x032410b5   /* Device ID & Vendor ID                        */
#define RHEA_PCI_CONFIG1       0x02a00157   /* Status & Command                             */
#define RHEA_PCI_CONFIG2       0x06800001   /* Class Code & Revision ID                     */
#define RHEA_PCI_CONFIG3       0x0000ff04   /* BIST, Header Type, Latency, & Cacheline Size */
#define RHEA_PCI_CONFIG4       0xa0000000   /* PCI Memory Address that Rhea responds to.    */
#define RHEA_PCI_CONFIG5       0x60000000   /* PCI Dual Cycle Address that Rhea responds to.*/
#define RHEA_PCI_CONFIG6       0x00800001   /* PCI I/O Address that Rhea responds to.       */
#define RHEA_PCI_CONFIG7       0x00000000
#define RHEA_PCI_CONFIG8       0x00000000
#define RHEA_PCI_CONFIG9       0x00000000
#define RHEA_PCI_CONFIG10      0x00000000
#define RHEA_PCI_CONFIG11      0x013410b5
#define RHEA_PCI_CONFIG12      0x00000000
#define RHEA_PCI_CONFIG13      0x00000000
#define RHEA_PCI_CONFIG14      0x00000000
#define RHEA_PCI_CONFIG15      0x38080101

/* Because of an errata in Rc32134 Pci Bridge, Scanning does not work properly.
   The device number is selected based on which pci slot on S134 board is being
   used */

#define PCI_BUS                         0
#define PCI_DEVICE_U28                  2      /* PCI Slot U28 */
#define PCI_DEVICE_U29                  3      /* PCI Slot U29 */
#define PCI_DEVICE_U20                  4      /* PCI Slot U20 */
#define PCI_FUNC                        0

/* Latency for the Pci/Ethernet Card   */

#define PCI_DEVICE_MAX_LATENCY          0x0000ff00
/* BusErrCntReg is used to disable/Enable BusError thrown on PCI
   bus on scanning */
#define BUS_ERR_CNTL_REG_ADDR           0xb8000010




/* FEI PCI bus resources */

#define FEI_IO_MAP_USE
#define FEI_OFFSET_ADD
#define FEI0_MEMBASE0           0x40800000      /* memory base for CSR */
#define FEI0_MEMSIZE0           0x00001000      /* memory size for CSR, 4KB */
#define FEI0_MEMBASE1           0x40a00000      /* memory base for Flash */
#define FEI0_MEMSIZE1           0x00100000      /* memory size for Flash, 1MB */
#define FEI0_IOBASE0            0x18800000      /* IO base for CSR, 32Bytes */
#define FEI0_INT_LVL            0x1             /* IRQ 1                    */

#define PCI_CFG_TYPE  		PCI_CFG_FORCE

/* Redefine PCI_CONFIG_ADDR & PCI_CONFIG_DATA */

#define CPU_TO_PCI_MEM_BASE	0x40000001
#define CPU_TO_PCI_IO_BASE	0x18800001
#define PCI_TO_CPU_MEM_BASE	0x00000000
#define PCI_TO_CPU_IO_BASE      0x00800001

#define IDT134_PCI_BASE		0xb8000000

#define IDT134_PCI_MEM_BAR1    	(IDT134_PCI_BASE + 0x20B0)
#define IDT134_PCI_MEM_BAR2 	(IDT134_PCI_BASE + 0x20B8)
#define IDT134_PCI_MEM_BAR3	(IDT134_PCI_BASE + 0x20C0)
#define IDT134_PCI_IO_BAR	(IDT134_PCI_BASE + 0x20C8)
#define IDT134_PCI_ARB_REG	(IDT134_PCI_BASE + 0x20E0)
#define IDT134_PCI_CPU_BAR1	(IDT134_PCI_BASE + 0x20E8)
#define IDT134_PCI_CPU_IO_BAR	(IDT134_PCI_BASE + 0x2100)
#define IDT134_PCI_CONFIG_ADDR  (IDT134_PCI_BASE + 0x2CF8)
#define IDT134_PCI_CONFIG_DATA  (IDT134_PCI_BASE + 0x2CFC)

#define IDT134_BAR_MEM_SWAP	0x00000001

							  
#endif /* CYGONCE_HAL_IDT79RC233X_H */
/*---------------------------------------------------------------------------*/
/* end of idt79rc233x.h                                                      */

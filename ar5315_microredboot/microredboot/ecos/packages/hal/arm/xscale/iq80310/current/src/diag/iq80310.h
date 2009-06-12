#ifndef CYGONCE_IQ80310_H
#define CYGONCE_IQ80310_H
//=============================================================================
//
//      iq80310.h - Cyclone Diagnostics
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003 Red Hat, Inc.
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
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   Scott Coulter, Jeff Frazier, Eric Breeden
// Contributors:
// Date:        2001-01-25
// Purpose:     
// Description: 
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

/******************************************************************************/
/* iq80310.h - Header file for Cyclone IQ80310 Evaluation Board				  */
/*																			  */
/* modification history														  */
/* --------------------														  */
/* 07sep00, ejb, Written for IQ80310 Cygmon diagnostics						  */
/* 18dec00  jwf                                                               */
/* 09feb01  jwf                                                               */
/******************************************************************************/

#ifndef NULL
#define NULL ((void *)0)
#endif

#ifndef ERROR
#define ERROR -1
#endif

#ifndef OK
#define OK 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#include <cyg/infra/diag.h>
#define printf diag_printf

#define RAM_FUNC_SECT

/* 02/09/01 jwf */
/* Specify the operating system for version information retrieval */
#define CYGNUS_CYGMON_OS			FALSE
#if CYGNUS_CYGMON_OS
	#define REDHAT_REDBOOT_OS		FALSE
#else
	#define REDHAT_REDBOOT_OS		TRUE
#endif

typedef int		STATUS;
typedef unsigned char	UCHAR;
typedef unsigned char	UINT8;
typedef unsigned short	USHORT;
typedef unsigned short	UINT16;
typedef unsigned long	ULONG;
typedef unsigned int	UINT;
typedef unsigned int	UINT32;
typedef int 		(*INTFUNCPTR) (int);
typedef int 		(*FUNCPTR) (void);
typedef void 		(*VOIDFUNCPTR) (int);

/* board specific definitions */


#define MEMBASE_DRAM		0xa0000000


/* UART definitions */
#define	SCALE		   		0x10000		/* distance between port addresses */
#define	TERMINAL		   	0xfe800000	/* Terminal base address */
#define ACCESS_DELAY		5
#define DFLTPORT			0			/* channel 2 on 16C552 */
#define XTAL				1843200		/* frequency of baud rate generation crystal */

/* Backplane Detect Register */
//#define BACKPLANE_DET_REG		(volatile unsigned char *)0xfe870000
#define BP_HOST_BIT				0x1 

/* PAL-based external timer definitions */
//#define TIMER_LA0_REG_ADDR		(volatile unsigned char *)0xfe880000
//#define TIMER_LA1_REG_ADDR		(volatile unsigned char *)0xfe890000
//#define TIMER_LA2_REG_ADDR		(volatile unsigned char *)0xfe8a0000
//#define TIMER_LA3_REG_ADDR		(volatile unsigned char *)0xfe8b0000
//#define TIMER_ENABLE_REG_ADDR	(volatile unsigned char *)0xfe8c0000

#define TIMER_COUNT_MASK		0x5f	/* 6 bits of timer data with the MSB in bit 6 not bit 5 */
#define TIMER_CNT_ENAB			0x1
#define TIMER_INT_ENAB			0x2
#define EXT_TIMER_CLK_FREQ		33000000	/* external timer runs at 33 MHz */
#define TICKS_10MSEC			100			/* 10msec = 100 ticks/sec */
#define EXT_TIMER_10MSEC_COUNT	(EXT_TIMER_CLK_FREQ / TICKS_10MSEC)
#define TICKS_5MSEC				200			/* 5msec = 200 ticks/sec */
#define EXT_TIMER_5MSEC_COUNT	(EXT_TIMER_CLK_FREQ / TICKS_5MSEC)

#define EXT_TIMER_CNT_ENAB()		(*TIMER_ENABLE_REG_ADDR |= TIMER_CNT_ENAB)
#define EXT_TIMER_CNT_DISAB()		(*TIMER_ENABLE_REG_ADDR &= ~TIMER_CNT_ENAB)
#define EXT_TIMER_INT_ENAB()		(*TIMER_ENABLE_REG_ADDR |= TIMER_INT_ENAB)
#define EXT_TIMER_INT_DISAB()		(*TIMER_ENABLE_REG_ADDR &= ~TIMER_INT_ENAB)

/* 80312 Interrupt Status Registers */
#define X3ISR_ADDR	0xfe820000	/* XINT3 (external interrupts) Status Register */
#define X3MASK_ADDR 0xfe860000	/* XINT3 Mask Register */


/* 12/18/00 jwf */
/* CPLD Read only Registers */
#define BOARD_REV_REG_ADDR	(volatile unsigned char *)0xfe830000	/* Board Revision Register, xxxxbbbb=0x2<-->Rev B Board, Note: This was not implemented in the CPLD for board revisions A,B,C and D. */
#define BOARD_REV_E			(unsigned char)0x5						/* BOARD REV E */
#define BOARD_REV_MASK		(unsigned char)0xf						/* use only b0-b3 */
#define CPLD_REV_REG_ADDR	(volatile unsigned char *)0xfe840000	/* CPLD Revision Register, data examples: xxxxbbbb=0x3<-->Rev C CPLD(used on PCI-700 Rev D Board), xxxxbbbb=0x4<-->Rev D CPLD(used on PCI-700 Rev E Board) */
#define SINT_REG_ADDR		(volatile unsigned char *)0xfe850000	/* SINTA-SINTC secondary PCI interrupt status register */
/* SINT_REG_ADDR Register Interrupt Status bit definitions */
#define SINTA_INT			(unsigned char)0x1						/* b0=1, Secondary PCI (S_INTA) Interrupt Pending */
#define SINTB_INT			(unsigned char)0x2						/* b1=1, Secondary PCI (S_INTB) Interrupt Pending */
#define SINTC_INT			(unsigned char)0x4						/* b2=1, Secondary PCI (S_INTC) Interrupt Pending */
#define SINT_MASK			(unsigned char)0x7						/* isolate bits b0-b2 */
#define RI_MASK				(unsigned char)0x40						/* use to isolate bit 6, Ring Indicator, of MSR in UART 2 */

/* Intel 28F640J3A Strata Flash Memory Definitions */
#define NUM_FLASH_BANKS		1					/* number of flash banks, there is only 1 flash memory chip on the pci-700 board */
#define FLASH_WIDTH			1					/* width of flash in bytes */
#define FLASH_BASE_ADDR		0x00000000 			/* base address of flash block 0, avoid this area, vectors and cygmon code occupy addresses 0x2000h-0x28000h */
#define DEV_CODE_ADDR		(0x00000001 << 1)	/* address of Device Code in Flash memory, note that address bit A0 is not used, must shift 0x00000001<<1=0x00000002 */

/* 10/17/00 jwf */
#define FLASH_BLK4_BASE_ADDR		0x80000

#define FLASH_TOP_ADDR		0x7fffff			/* last address of last block of flash memory */
#define FLASH_ADDR			0x00000000 			/* base address of flash block 0, avoid this area, vectors and cygmon code occupy addresses 0x2000h-0x28000h */
#define FLASH_ADDR_INCR		0x00020000			/* address offset of each flash block, 128K block, byte-wide (X8) mode, device address range 0-7fffff */
#define VALID_FLASH_ADDR	0x00000000 			/* base address of flash block 0 */ 
#define FLASH_TIME_ADJUST	1					/* delay adjustment factor for delay times */

/* 10/17/00 jwf */
#define RESERVED_AREA1		0x0			/* 0h-1ffffh is partially occupied by Cygnus Cygmon monitor and debug code */
#define RESERVED_AREA2		0x20000		/* 20000h-3ffffh is partially occupied by Cygnus Cygmon monitor and debug code */   
#define RESERVED_AREA3		0x40000		/* 40000h-5ffffh is partially occupied by Cygnus Cygmon debug code */   
#define RESERVED_AREA4		0x60000		/* 60000h-7ffffh is partially occupied by Cygnus Cygmon debug code */   
#define RESERVED_AREA_SIZE  0x80000		/* 20000h * 4h */

/* Definitions for Battery Backup SDRAM memory test */
#define	SDRAM_BATTERY_TEST_BASE		0xA1FFFFF0		/* base address of last 16 memory locations in 32MB SDRAM */
/* #define BATTERY_TEST_PATTERN		0xBAEBAEBA */
#define BATTERY_TEST_PATTERN		0x55555555


/* 02/09/01 jwf */
/* Use a base address set to the fourth memory location from the last memory location */
/* in a 32MB SDRAM DIMM to store the 80200 Coyanosa ID 32 bit data */
#define	COYANOSA_ID_BASE_ADDR		0xA1FFFFFC

/* Definitions for data types and constants used in Flash.c */
typedef unsigned long ADDR;			
#define NO_ADDR ((ADDR)0x800000)	/* last address of Flash memory + 1 */
#ifndef ERR
#define ERR	-1					
#endif
/* Error code Constants */
#define	E_EEPROM_ADDR	12		
#define	E_EEPROM_PROG	13			
#define	E_EEPROM_FAIL	14		
#define E_NO_FLASH	29				


/* 10/17/00 jwf */
#define BREEZE_BLOCK_0			0x0 	
#define NUM_BREEZE_BLOCKS		  4		
#define NUM_FLASH_BLOCKS		 64	






/* 80310 IRQ Interrupt Identifiers (used for connecting and disconnecting ISRs) */
#define DMA0_INT_ID		0
#define DMA1_INT_ID		1
#define DMA2_INT_ID		2
#define PM_INT_ID		3
#define AA_INT_ID		4
#define I2C_INT_ID		5
#define MU_INT_ID		6
#define PATU_INT_ID		7	
#define TIMER_INT_ID	8
#define ENET_INT_ID		9
#define UART1_INT_ID	10
#define UART2_INT_ID	11
#define SINTA_INT_ID	12
#define SINTB_INT_ID	13
#define SINTC_INT_ID	14
#define SINTD_INT_ID	15



/* XINT3 External Interrupt Status and Mask bit definitions */
#define TIMER_INT		(1 << 0)	/* Timer Interrupt Pending */
#define ENET_INT		(1 << 1)	/* Ethernet Interrupt Pending */
#define UART1_INT		(1 << 2)	/* UART1 Interrupt Pending */
#define UART2_INT		(1 << 3)	/* UART2 Interrupt Pending */
#define SINTD_INT		(1 << 4)	/* Secondary PCI (S_INTD) Interrupt Pending */

/* XINT6 Interrupt Status bit definitions */
#define DMA0_INT		(1 << 0)	/* DMA Channel 0 Interrupt Pending */
#define DMA1_INT		(1 << 2)	/* DMA Channel 1 Interrupt Pending */
#define DMA2_INT		(1 << 3)	/* DMA Channel 2 Interrupt Pending */
#define PM_INT			(1 << 5)	/* Performance Monitoring Unit Interrupt Pending */
#define AA_INT			(1 << 6)	/* Application Accelerator Interrupt Pending */

/* XINT7 Interrupt Status bit definitions */
#define I2C_INT			(1 << 1)	/* I2C Interrupt Pending */
#define MU_INT			(1 << 2)	/* Messaging Unit Interrupt Pending */
#define PATU_INT		(1 << 3)	/* Primary ATU / BIST Start Interrupt Pending */

/* NISR bit definitions */
#define MCU_ERROR		(1 << 0)		/* 80960 core Error within internal memory controller */
#define PATU_ERROR		(1 << 1)		/* Primary ATU Error (PCI or local bus error) */
#define SATU_ERROR		(1 << 2)		/* Secondary ATU Error (PCI or local bus error) */
#define PBRIDGE_ERROR	(1 << 3)		/* Primary Bridge Interface Error */
#define SBRIDGE_ERROR	(1 << 4)		/* Secondary Bridge Interface Error */
#define DMA_0_ERROR		(1 << 5)		/* DMA Channel 0 Error (PCI or local bus error) */
#define DMA_1_ERROR		(1 << 6)		/* DMA Channel 1 Error (PCI or local bus error) */
#define DMA_2_ERROR		(1 << 7)		/* DMA Channel 2 Error (PCI or local bus error) */
#define MU_ERROR		(1 << 8)		/* Messaging Unit NMI interrupt */
#define AAU_ERROR		(1 << 10)		/* Application Accelerator Unit Error */
#define BIU_ERROR		(1 << 11)		/* Bus Interface Unit Error */



/* macros to clear (S/P PCI Status register bits) */
#define CLEAR_PATU_STATUS()		(*(volatile UINT16 *)PATUSR_ADDR |= 0xf900)
#define CLEAR_SATU_STATUS()		(*(volatile UINT16 *)SATUSR_ADDR |= 0xf900)
#define CLEAR_PBRIDGE_STATUS()	(*(volatile UINT16 *)PSR_ADDR |= 0xf900)
#define CLEAR_SBRIDGE_STATUS()	(*(volatile UINT16 *)SSR_ADDR |= 0xf900)







/*** Yavapai Registers ***/

/* PCI-to-PCI Bridge Unit 0000 1000H through 0000 10FFH */
#define VIDR_ADDR	0x00001000
#define DIDR_ADDR	0x00001002
#define PCR_ADDR	0x00001004
#define PSR_ADDR	0x00001006
#define RIDR_ADDR	0x00001008
#define CCR_ADDR	0x00001009
#define CLSR_ADDR	0x0000100C
#define PLTR_ADDR	0x0000100D
#define HTR_ADDR	0x0000100E
/* Reserved 0x0000100F through  0x00001017 */
#define PBNR_ADDR	0x00001018
#define SBNR_ADDR	0x00001019
#define SUBBNR_ADDR	0x0000101A
#define SLTR_ADDR	0x0000101B
#define IOBR_ADDR	0x0000101C
#define IOLR_ADDR	0x0000101D
#define SSR_ADDR	0x0000101E
#define MBR_ADDR	0x00001020
#define MLR_ADDR	0x00001022
#define PMBR_ADDR	0x00001024
#define PMLR_ADDR	0x00001026
/* Reserved 0x00001028 through 0x00001033 */
#define BSVIR_ADDR	0x00001034
#define BSIR_ADDR	0x00001036
/* Reserved 0x00001038 through 0x0000103D */
#define BCR_ADDR	0x0000103E
#define EBCR_ADDR	0x00001040
#define	SISR_ADDR	0x00001042
#define PBISR_ADDR	0x00001044
#define SBISR_ADDR	0x00001048
#define SACR_ADDR	0x0000104C
#define PIRSR_ADDR	0x00001050
#define SIOBR_ADDR	0x00001054
#define SIOLR_ADDR	0x00001055
#define SCCR_ADDR	0x00001056		/* EAS inconsistent */
#define SMBR_ADDR	0x00001058
#define SMLR_ADDR	0x0000105A
#define SDER_ADDR	0x0000105C
#define QCR_ADDR	0x0000105E
#define CDTR_ADDR	0x00001060		/* EAS inconsistent */
/* Reserved 0x00001064 through 0x000010FFH */

/* Performance Monitoring Unit 0000 1100H through 0000 11FFH */
#define GMTR_ADDR	0x00001100
#define ESR_ADDR	0x00001104
#define EMISR_ADDR	0x00001108
/* Reserved 0x0000110C */			/* EAS inconsistent */
#define GTSR_ADDR	0x00001110		/* EAS inconsistent */
#define PECR1_ADDR	0x00001114		/* EAS inconsistent */
#define PECR2_ADDR	0x00001118		/* EAS inconsistent */
#define PECR3_ADDR	0x0000111C		/* EAS inconsistent */
#define PECR4_ADDR	0x00001120		/* EAS inconsistent */
#define PECR5_ADDR	0x00001124		/* EAS inconsistent */
#define PECR6_ADDR	0x00001128		/* EAS inconsistent */
#define PECR7_ADDR	0x0000112C		/* EAS inconsistent */
#define PECR8_ADDR	0x00001130		/* EAS inconsistent */
#define PECR9_ADDR	0x00001134		/* EAS inconsistent */
#define PECR10_ADDR	0x00001138		/* EAS inconsistent */
#define PECR11_ADDR	0x0000113C		/* EAS inconsistent */
#define PECR12_ADDR	0x00001140		/* EAS inconsistent */
#define PECR13_ADDR	0x00001144		/* EAS inconsistent */
#define PECR14_ADDR	0x00001148		/* EAS inconsistent */
/* Reserved 0x0000104C through 0x000011FFH */	/* EAS inconsistent */

/* Address Translation Unit 0000 1200H through 0000 12FFH */
#define ATUVID_ADDR		0x00001200
#define ATUDID_ADDR		0x00001202
#define PATUCMD_ADDR	0x00001204
#define PATUSR_ADDR		0x00001206
#define ATURID_ADDR		0x00001208
#define ATUCCR_ADDR		0x00001209
#define ATUCLSR_ADDR	0x0000120C
#define ATULT_ADDR		0x0000120D
#define ATUHTR_ADDR		0x0000120E
#define ATUBISTR_ADDR	0x0000120F
#define PIABAR_ADDR		0x00001210
/* Reserved 0x00001214 through 0x0000122B */
#define ASVIR_ADDR		0x0000122C
#define ASIR_ADDR		0x0000122E
#define ERBAR_ADDR		0x00001230
/* Reserved 0x00001234 */
/* Reserved 0x00001238 */
#define ATUILR_ADDR		0x0000123C
#define ATUIPR_ADDR		0x0000123D
#define ATUMGNT_ADDR	0x0000123E
#define ATUMLAT_ADDR	0x0000123F
#define PIALR_ADDR		0x00001240
#define PIATVR_ADDR		0x00001244
#define SIABAR_ADDR		0x00001248
#define SIALR_ADDR		0x0000124C
#define SIATVR_ADDR		0x00001250
#define POMWVR_ADDR		0x00001254
/* Reserved 0x00001258 */
#define POIOWVR_ADDR	0x0000125C
#define PODWVR_ADDR		0x00001260
#define POUDR_ADDR		0x00001264
#define SOMWVR_ADDR		0x00001268
#define SOIOWVR_ADDR	0x0000126C
/* Reserved 0x00001270 */
#define ERLR_ADDR		0x00001274
#define ERTVR_ADDR		0x00001278
/* Reserved 0x0000127C */
/* Reserved 0x00001280 */
/* Reserved 0x00001284 */
#define ATUCR_ADDR		0x00001288
/* Reserved 0x0000128C */
#define PATUISR_ADDR	0x00001290
#define SATUISR_ADDR	0x00001294
#define SATUCMD_ADDR	0x00001298
#define SATUSR_ADDR		0x0000129A
#define SODWVR_ADDR		0x0000129C
#define SOUDR_ADDR		0x000012A0
#define POCCAR_ADDR		0x000012A4
#define SOCCAR_ADDR		0x000012A8
#define POCCDR_ADDR		0x000012AC
#define SOCCDR_ADDR		0x000012B0
#define PAQCR_ADDR		0x000012B4
#define SAQCR_ADDR		0x000012B8
#define PAIMR_ADDR		0x000012BC
#define SAIMR_ADDR		0x000012C0
/* Reserved 0x000012C4 through 0x000012FF */

/* Messaging Unit 0000 1300H through 0000 130FH */
#define IMR0_ADDR		0x00001310
#define IMR1_ADDR		0x00001314
#define OMR0_ADDR		0x00001318
#define OMR1_ADDR		0x0000131C
#define IDR_ADDR		0x00001320
#define IISR_ADDR		0x00001324
#define IIMR_ADDR		0x00001328
#define ODR_ADDR		0x0000132C
#define OISR_ADDR		0x00001330
#define OIMR_ADDR		0x00001334
/* Reserved 0x00001338 through 0x0000134F */
#define MUCR_ADDR		0x00001350
#define QBAR_ADDR		0x00001354
/* Reserved 0x00001358 */
/* Reserved 0x0000135C */
#define IFHPR_ADDR		0x00001360
#define IFTPR_ADDR		0x00001364
#define IPHPR_ADDR		0x00001368
#define IPTPR_ADDR		0x0000136C
#define OFHPR_ADDR		0x00001370
#define OFTPR_ADDR		0x00001374
#define OPHPR_ADDR		0x00001378
#define OPTPR_ADDR		0x0000137C
#define IAR_ADDR		0x00001380
/* Reserved 0x00001384 through 0x000013FF */

/* DMA Controller 0000 1400H through 0000 14FFH */
#define	CCR0_ADDR		0x00001400
#define CSR0_ADDR		0x00001404
/* Reserved 0x00001408 */
#define DAR0_ADDR		0x0000140C
#define NDAR0_ADDR		0x00001410
#define PADR0_ADDR		0x00001414
#define PUADR0_ADDR		0x00001418
#define LADR0_ADDR		0x0000141C
#define BCR0_ADDR		0x00001420
#define DCR0_ADDR		0x00001424
/* Reserved 0x00001428 through 0x0000143F */
#define CCR1_ADDR		0x00001440
#define CSR1_ADDR		0x00001444
/* Reserved 0x00001448 */
#define DAR1_ADDR		0x0000144C
#define NDAR1_ADDR		0x00001450
#define PADR1_ADDR		0x00001454
#define PUADR1_ADDR		0x00001458
#define LADR1_ADDR		0x0000145C
#define BCR1_ADDR		0x00001460
#define DCR1_ADDR		0x00001464
/* Reserved 0x00001468 through 0x0000147F */
#define CCR2_ADDR		0x00001480
#define CSR2_ADDR		0x00001484
/* Reserved 0x00001488 */
#define DAR2_ADDR		0x0000148C
#define NDAR2_ADDR		0x00001490
#define PADR2_ADDR		0x00001494
#define PUADR2_ADDR		0x00001498
#define LADR2_ADDR		0x0000149C
#define BCR2_ADDR		0x000014A0
#define DCR2_ADDR		0x000014A4
/* Reserved 0x000014A8 through 0x000014FF */

/* Memory Controller 0000 1500H through 0000 15FFH */
#define SDIR_ADDR		0x00001500
#define SDCR_ADDR		0x00001504
#define SDBR_ADDR		0x00001508
#define SBR0_ADDR		0x0000150C
#define SBR1_ADDR		0x00001510
#define SDPR0_ADDR		0x00001514
#define SDPR1_ADDR		0x00001518
#define SDPR2_ADDR		0x0000151C
#define SDPR3_ADDR		0x00001520
#define SDPR4_ADDR		0x00001524
#define SDPR5_ADDR		0x00001528
#define SDPR6_ADDR		0x0000152C
#define SDPR7_ADDR		0x00001530
#define ECCR_ADDR		0x00001534
#define ELOG0_ADDR		0x00001538
#define ELOG1_ADDR		0x0000153C
#define ECAR0_ADDR		0x00001540
#define ECAR1_ADDR		0x00001544
#define ECTST_ADDR		0x00001548
#define FEBR0_ADDR		0x0000154C
#define FEBR1_ADDR		0x00001550
#define FBSR0_ADDR		0x00001554
#define FBSR1_ADDR		0x00001558
#define FWSR0_ADDR		0x0000155C
#define FWSR1_ADDR		0x00001560
#define MCISR_ADDR		0x00001564
#define RFR_ADDR		0x00001568
/* Reserved 0x0000156C through 0x000015FF */

/* Arbitration Control Unit 0000 1600H through 0000 167FH */
#define IACR_ADDR		0x00001600
#define MLTR_ADDR		0x00001604
#define MTTR_ADDR		0x00001608
/* Reserved 0x0000160C through 0x0000163F */

/* Bus Interface Control Unit 0000 1640H through 0000 167FH */
#define BIUCR_ADDR		0x00001640
#define BIUISR_ADDR		0x00001644
/* Reserved 0x00001648 through 0x0000167F */

/* I2C Bus Interface Unit 0000 1680H through 0000 16FFH */
#define ICR_ADDR		0x00001680
#define ISR_ADDR		0x00001684
#define ISAR_ADDR		0x00001688
#define IDBR_ADDR		0x0000168C
#define ICCR_ADDR		0x00001690
#define IBMR_ADDR		0x00001694
/* Reserved 0x00001698 through 0x000016FF */

/* PCI And Peripheral Interrupt Controller 0000 1700H through 0000 17FFH */
#define NISR_ADDR		0x00001700
#define X7ISR_ADDR		0x00001704
#define X6ISR_ADDR		0x00001708
#define PDIDR_ADDR		0x00001710		/* EAS inconsistent */
/* Reserved 0x00001714 through 0x0000177F */

/* Application Accelerator Unit 0000 1800H through 0000 18FFH */
#define ACR_ADDR		0x00001800
#define ASR_ADDR		0x00001804
#define ADAR_ADDR		0x00001808
#define ANDAR_ADDR		0x0000180C
#define SAR1_ADDR		0x00001810
#define SAR2_ADDR		0x00001814
#define SAR3_ADDR		0x00001818
#define SAR4_ADDR		0x0000181C
#define DAR_ADDR		0x00001820
#define ABCR_ADDR		0x00001824
#define ADCR_ADDR		0x00001828
#define SAR5_ADDR		0x0000182C
#define SAR6_ADDR		0x00001830
#define SAR7_ADDR		0x00001834
#define SAR8_ADDR		0x00001838

/* Reserved 0x0000183C through 0x000018FF */


#endif /* CYGONCE_IQ80310_H */

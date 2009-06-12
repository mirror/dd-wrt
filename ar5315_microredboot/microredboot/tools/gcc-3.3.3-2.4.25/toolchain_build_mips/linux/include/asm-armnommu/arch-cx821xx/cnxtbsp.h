/****************************************************************************
*
*	Name:			cnxtbsp.h
*
*	Description:	
*
*	Copyright:		(c) 2002 Conexant Systems Inc.
*
*****************************************************************************

  This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2 of the License, or (at your option)
any later version.

  This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
more details.

  You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc., 59
Temple Place, Suite 330, Boston, MA 02111-1307 USA

****************************************************************************
*  $Author: gerg $
*  $Revision: 1.1 $
*  $Modtime: 11/08/02 12:50p $
****************************************************************************/

/*
This file contains I/O address and related constants for the ARM PID board.
*/

#ifndef CNXTBSP_H
#define CNXTBSP_H

#define BULK_MAX_PACKET_SIZE     64

/*******************************************************************
* MMU Control Register layout.
* BIT
*	0 M 0 MMU disabled
*	1 A 0 Address alignment fault disabled, initially
*	2 C 0 Data cache disabled
*	3 W 0/1 Write Buffer disabled (or Should Be One)
*	4 P 1 PROG32
*	5 D 1 DATA32
*	6 L 1 Should Be One (Late abort on earlier CPUs)
*	7 B ? Endianness (1 => big)
*	8 S 0 System bit to zero } Modifies MMU protections, not really
*	9 R 1 ROM bit to one     } relevant until MMU switched on later.
*  10 F 0 Should Be Zero
*  11 Z 0 Should Be Zero (Branch prediction control on 810)
*  12 I 0 Instruction cache control, (or Should Be Zero)
********************************************************************/

#define MMUCR_M_ENABLE	 (1<<0)  /* MMU enable */
#define MMUCR_A_ENABLE	 (1<<1)  /* Address alignment fault enable */
#define MMUCR_C_ENABLE	 (1<<2)  /* (data) cache enable */
#define MMUCR_W_ENABLE	 (1<<3)  /* write buffer enable */
#define MMUCR_PROG32	    (1<<4)  /* PROG32 */
#define MMUCR_DATA32	    (1<<5)  /* DATA32 */
#define MMUCR_L_ENABLE	 (1<<6)  /* Late abort on earlier CPUs */
#define MMUCR_BIGEND	    (1<<7)  /* Big-endian (=1), little-endian (=0) */
#define MMUCR_SYSTEM	    (1<<8)  /* System bit, modifies MMU protections */
#define MMUCR_ROM		    (1<<9)  /* ROM bit, modifies MMU protections */
#define MMUCR_F			 (1<<10) /* Should Be Zero */
#define MMUCR_Z_ENABLE	 (1<<11) /* Branch prediction enable on 810 */
#define MMUCR_I_ENABLE	 (1<<12) /* Instruction cache enable */
/* Extra 940T bits */
#define MMUCR_FB_DISABLE (1<<30) /* nFastBus bit */
#define MMUCR_ASYNC_BIT	 (1<<31) /* Async bit*/
#define MMUCR_SYNC		 (1<<30) /* Synchronous mode */
#define MMUCR_ASYNC		 (3<<30) /* Asynchronous mode */
#define MMUCR_FB		 0
/* W bit is Should Be One */
#define MMU_INIT_VALUE ( MMUCR_PROG32 | MMUCR_DATA32 | MMUCR_L_ENABLE | MMUCR_W_ENABLE)


/* Internal memory addresses */
#define ASB_RAM_ADRS 0x00180000

#define  SRAMSIZE    0x8000
#if (SDRAM_SIZE == 8)
	#define SDRAMSIZE  0x00800000
#else
	#if (SDRAM_SIZE == 4)
  		#define SDRAMSIZE  0x00400000
	#else
		#define SDRAMSIZE  0x00200000
	#endif
#endif

/* Internal SRAM location during Run map */
#define  SRAMSTART      0x1000
#define  SRAMEND        0x8000
#define  HOST_SIGNATURE 0x500

#define  SDRAMSTART     0x800000
#define  SDRAMEND       SDRAMSTART + SDRAMSIZE


/* PLL control bit definitions */
#define PLL_CR_SLOW     BIT28
#define PLL_LOCK_STATUS BIT27

/* PLL definitions */
#define P52_MAX_NUM_BLCK         3
#define RUSHMORE_MAX_NUM_BLCK    5

/* Define how often USB interface should poll DMA Ready bit */
#define USB_POLL_125    ((0.0005 * PLL_FAST_125MHZ)/2) & 0x000007FF
#define USB_POLL_100    ((0.0005 * PLL_FAST_100MHZ)/2) & 0x000007FF
#define USB_POLL_75     ((0.0005 * PLL_FAST_75MHZ)/2)  & 0x000007FF


#define EXT_MEM_CTL     0x350010
#define EXT_SDRAM_ENA   0x1
#define EXT_SRAM_ENA    0x2

#define LPMR		      0x350014

/*
 * interrupt control stuff
 * Note: FIQ is not handled within VxWorks so this is just IRQ
 */

#define IC_BASE         0x00350000

#define EXT_MEM_CTL     0x350010
#define EXT_SDRAM_ENA   0x1
#define EXT_SRAM_ENA    0x2

/* PLL Registers */

/* ##################################### */
#define DEVICE_ID_REG		 0x350060

/* This memory location and value determine if it is
   Rushmore or Rushmore X4	*/
#define ROM_DEVICE_ID_REG		0x187ffc
#define RUSH_X45_ROM_VALUE		0x00003354
#define RUSH_X10_45_DEV_ID		0x08230000
#define RUSH_X50_DEV_ID			0x08230001

#define P5200X10_DEV_ID		 0x50000000     /* Mask 1st[3:0] and last[31:28] bytes */
#define P5200X180_DEV_ID     0x50000001     /* Mask 1st[3:0] and last[31:28] bytes */
#define P5300_DEV_ID		 0x50000002     /* Mask 1st[3:0] and last[31:28] bytes */

/* Interrupt Registers */
#ifdef DEVICE_YELLOWSTONE
//************************************************
// 		Define Offset for I/O and memory map 
//		Incase of APTIX system with ARM SCM board  
//************************************************

#ifdef ARM_SCM_USE
   #define S_OFFSET     0xC0000000
#else
   #define S_OFFSET     0x00000000
#endif

//************************************************
// 		Interrupt Controller I/O register map
//************************************************

#define PIC_BASE			            (0x00600800 + S_OFFSET)

#define PIC_INT_ENABLE					(PIC_BASE + 0)						// Not Use 
#define PIC_TOP_ASSIGN_POLLED			(PIC_BASE + 0x4)		
#define PIC_TOP_ASSIGN_FIQ				(PIC_BASE + 0x8)		
#define PIC_TOP_ISR_POLLED				(PIC_BASE + 0xC)		 
#define PIC_TOP_IMR_FIQ 				(PIC_BASE + 0x10)		  
#define PIC_TOP_ISR_FIQ					(PIC_BASE + 0x14)		  
#define PIC_TOP_MISR_FIQ 				(PIC_BASE + 0x18)		  
#define PIC_TOP_IMR_IRQ					(PIC_BASE + 0x1C)		  
#define PIC_TOP_ISR_IRQ					(PIC_BASE + 0x20)		  
#define PIC_TOP_MISR_IRQ				(PIC_BASE + 0x24)		  
#define PIC_PRIORITY_3_0				(PIC_BASE + 0x28)		  
#define PIC_PRIORITY_7_4				(PIC_BASE + 0x2C)		  
#define PIC_PRIORITY_11_8				(PIC_BASE + 0x30)		  
#define PIC_PRIORITY_15_12			   (PIC_BASE + 0x34)		  
#define PIC_PRIORITY_19_16			   (PIC_BASE + 0x38)		  
#define PIC_PRIORITY_23_20			   (PIC_BASE + 0x3C)		  
#define PIC_PRIORITY_27_24			   (PIC_BASE + 0x40)		  
#define PIC_PRIORITY_31_28			   (PIC_BASE + 0x44)		  
#define PIC_CALC_FRM_STAT_WR			(PIC_BASE + 0x48)		  
#define PIC_CALC_FRM_STAT_RD_INDEX	(PIC_BASE + 0x4C)		  
#define PIC_CALC_FRM_STAT_RD_MSK		(PIC_BASE + 0x50)		  
#define PIC_CALC_FRM_INDEX_WR			(PIC_BASE + 0x54)		  
#define PIC_CALC_FRM_INDEX_RD_MSK	(PIC_BASE + 0x58)		  

#else
#define PIC_TOP_ISR_IRQ             (IC_BASE+0x44)
#define INT_SET_STAT                (IC_BASE+0x48)
#define PIC_TOP_IMR_IRQ             (IC_BASE+0x4C)

#define IRQ_LEVEL       (IC_BASE)       /* Read/Write */
#define IRQ_STATUS      (IC_BASE+0x44)  /* Read */
#define IRQ_SET_STATUS  (IC_BASE+0x48)  /* Write */
#define IRQ_ENABLE      (IC_BASE+0x4C)  /* Read/Write */
#define IRQ_DISABLE     (IC_BASE+0x4C)  /* Write */
#define MASK_STATUS     (IC_BASE+0x50)  /* Read */

#define INT_STAT        (IC_BASE+0x44)
#define INT_SET_STAT    (IC_BASE+0x48)
#define INT_MASK        (IC_BASE+0x4C)
#define INT_MSTAT       (IC_BASE+0x90)

#endif
/*
 * All ARM 940T BSPs must define a variable sysCacheUncachedAdrs: a
 * pointer to a word that is uncached and is safe to read (i.e. has no
 * side effects).  This is used by the cacheLib code to perform a read
 * (only) to drain the write-buffer. Clearly this address must be present
 * within one of the regions created within sysPhysMemDesc, where it must
 * be marked as non-cacheable. There are many such addresses we could use
 * on the PID board, but we choose to use an address here that will be
 * mapped in on just about all configurations: a safe address within the
 * interrupt controller: the IRQ Enabled status register. This saves us
 * from having to define a region just for this pointer. This constant
 * defined here is used to initialise sysCacheUncachedAdrs in sysLib.c
 * and is also used by the startup code in sysALib.s and romInit.s in
 * draining the write-buffer.
 */

#define SYSTEM_CACHE_UNCACHED_ADRS	PIC_TOP_ISR_IRQ

#define CNXT_INT_NUM_LEVELS        32
#define CNXT_INT_CSR_PEND          MASK_STATUS
#define CNXT_INT_CSR_ENB           IRQ_ENABLE
#define CNXT_INT_CSR_DIS           IRQ_DISABLE
#define CNXT_INT_CSR_MASK          0xFFFFFFFF

#define IRQ_OFF         0
#define IRQ_ON          1

#ifdef DEVICE_YELLOWSTONE
#define PIC_DISABLE_MASK   0xFFFFFFFF
#else
#define PIC_DISABLE_MASK   0x0
#endif

/* interrupt levels */
#define INT_LVL_TIMER_1            0       /* Timer 1 interrupt */
#define INT_LVL_TIMER_2            1       /* Timer 2 interrupt */
#define INT_LVL_TIMER_3            2       /* Timer 3 interrupt */
#define INT_LVL_TIMER_4            3       /* Timer 3 interrupt */
#define INT_LVL_USB                4       /* USB */
#define INT_LVL_RES0               5       /* */
#define INT_LVL_HOST               6       /* External Host interrupt */
#define INT_LVL_HOST_ERR           7       /* Host Bus error detect */
#define INT_LVL_DMA8               8       /* M2M */
#define INT_LVL_RES1               9       /* */
#define INT_LVL_DMA6              10       /* ADSL Rx */
#define INT_LVL_DMA5              11       /* ADSL Tx */
#define INT_LVL_DMA4              12       /* EMAC2 Rx */
#define INT_LVL_DMA3              13       /* EMAC2 Tx */
#define INT_LVL_DMA2              14       /* EMAC1 Rx */
#define INT_LVL_DMA1              15       /* EMAC1 Tx*/
#define INT_LVL_RES2              16       /* */
#define INT_LVL_RES3              17       /* */
#define INT_LVL_DMAC_ERR          18       /* DMAC error */
#define INT_LVL_EMAC2_ERR         19       /* EMAC2 error */
#define INT_LVL_EMAC1_ERR         20       /* EMAC1 error */
#define INT_LVL_ADSL_ERR          21       /* ADSL error */
#define INT_LVL_RES4              22       /* */
#define INT_LVL_RES5              23       /* */
#define INT_LVL_GPIO              24       /* GPIO input*/
#define INT_LVL_RES6              25       /* */
#define INT_LVL_COMMTX            26       /* ARM9 Comms Xmit */
#define INT_LVL_COMMRX            27       /* ARM9 Comms Rcv */
#define INT_LVL_SW0               28       /* Software interrupt 0 */
#define INT_LVL_SW1               29       /* Software interrupt 1 */
#define INT_LVL_SW2               30       /* Software interrupt 2 */
#define INT_LVL_SW3               31       /* Software interrupt 3 */

/* interrupt vectors */
#define INT_VEC_TIMER_1   IVEC_TO_INUM(INT_LVL_TIMER_1)  /* Timer 1 interrupt */
#define INT_VEC_TIMER_2   IVEC_TO_INUM(INT_LVL_TIMER_2)  /* Timer 2 interrupt */
#define INT_VEC_TIMER_3   IVEC_TO_INUM(INT_LVL_TIMER_3)  /* Timer 3 interrupt */
#define INT_VEC_TIMER_4   IVEC_TO_INUM(INT_LVL_TIMER_4)  /* Timer 4 interrupt */
#define INT_VEC_USB       IVEC_TO_INUM(INT_LVL_USB)      /* USB */
#define INT_VEC_RES0      IVEC_TO_INUM(INT_LVL_RES0)     /* */
#define INT_VEC_HOST      IVEC_TO_INUM(INT_LVL_HOST)     /* External Host interrupt */
#define INT_VEC_HOST_ERR  IVEC_TO_INUM(INT_LVL_HOST_ERR) /* Host Bus error detect */
#define INT_VEC_DMA8      IVEC_TO_INUM(INT_LVL_DMA8)     /* M2M */
#define INT_VEC_RES1      IVEC_TO_INUM(INT_LVL_RES1)     /* */
#define INT_VEC_DMA6      IVEC_TO_INUM(INT_LVL_DMA6)     /* ADSL Rx */
#define INT_VEC_DMA5      IVEC_TO_INUM(INT_LVL_DMA5)     /* ADSL Tx */
#define INT_VEC_DMA4      IVEC_TO_INUM(INT_LVL_DMA4)     /* EMAC2 Rx */
#define INT_VEC_DMA3      IVEC_TO_INUM(INT_LVL_DMA3)     /* EMAC2 Tx */
#define INT_VEC_DMA2      IVEC_TO_INUM(INT_LVL_DMA2)     /* EMAC1 Rx */
#define INT_VEC_DMA1      IVEC_TO_INUM(INT_LVL_DMA1)     /* EMAC1 Tx */
#define INT_VEC_RES2      IVEC_TO_INUM(INT_LVL_RES2)     /* */
#define INT_VEC_RES3      IVEC_TO_INUM(INT_LVL_RES3)     /* */
#define INT_VEC_DMAC_ERR  IVEC_TO_INUM(INT_LVL_DMAC_ERR) /* DMAC BERROR */
#define INT_VEC_EMAC1_ERR IVEC_TO_INUM(INT_LVL_EMAC1_ERR)/* EMAC1 error */
#define INT_VEC_EMAC2_ERR IVEC_TO_INUM(INT_LVL_EMAC2_ERR)/* EMAC2 error */
#define INT_VEC_ADSL_ERR  IVEC_TO_INUM(INT_LVL_ADSL_ERR) /* Upstream CMAC error */
#define INT_VEC_RES4      IVEC_TO_INUM(INT_LVL_RES4)     /* */
#define INT_VEC_RES5      IVEC_TO_INUM(INT_LVL_RES5)     /* */
#define INT_VEC_GPIO      IVEC_TO_INUM(INT_LVL_GPIO)     /* GPIO input */
#define INT_VEC_RES6      IVEC_TO_INUM(INT_LVL_RES6)     /* */
#define INT_VEC_COMMTX    IVEC_TO_INUM(INT_LVL_COMMTX)   /* ARM9 Comms Xmit */
#define INT_VEC_COMMRX    IVEC_TO_INUM(INT_LVL_COMMRX)   /* ARM9 Comms Rcv */
#define INT_VEC_SW0       IVEC_TO_INUM(INT_LVL_SW0)      /* Software interrupt 0 */
#define INT_VEC_SW1       IVEC_TO_INUM(INT_LVL_SW1)      /* Software interrupt 1 */
#define INT_VEC_SW2       IVEC_TO_INUM(INT_LVL_SW2)      /* Software interrupt 2 */
#define INT_VEC_SW3       IVEC_TO_INUM(INT_LVL_SW3)      /* Software interrupt 3 */

/* definition for UART structure */
#define UART_REG_ADDR_INTERVAL  2               /* registers 2 bytes apart */
/*
 * definitions for the CNXT Timer:
 * two timers clocked from same source and with the same reload overhead
 */
#define CNXT_TIMER_BASE             0x00350020  /* Address of base of timer */
#define CNXT_TIMER_SYS_TC_DISABLE   0x0         /* 0 Written to TIM_LIM1 shuts off timer    */
/* Paul: Must revisit */
#define CNXT_TIMER_SYS_TC_ENABLE    0x411A      /* TIM_LIM1 sets 16.7 ms timer  */
#define CNXT_TIMER_AUX_TC_DISABLE   0x0         /* 0 Written to TIM_LIM2 shuts off timer    */
#define CNXT_TIMER_AUX_TC_ENABLE    0x2710      /* TIM_LIM2 sets 10 ms timer  */

#define CNXT_CLK_SPEED   1000000                /* Timer speed is 1 microsecond*/
#define SYS_TIMER_CLK   (CNXT_CLK_SPEED)        /* Frequency of counter/timer*/
#define AUX_TIMER_CLK   (CNXT_CLK_SPEED)        /* Frequency of counter/timer*/

#define CNXT_RELOAD_TICKS       3               /* three ticks to reload timer*/

#define SYS_TIMER_CLEAR(x)      (CNXT_TIMER_T1CLEAR(x)) /* sys Clk is timer 1 */
#define SYS_TIMER_CTRL(x)       (CNXT_TIMER_T1CTRL(x))
#define SYS_TIMER_LOAD(x)       (CNXT_TIMER_T1LOAD(x))
#define SYS_TIMER_VALUE(x)      (CNXT_TIMER_T1VALUE(x))
#define CNXT_TIMER_VALUE_MASK   0xFFFF
#define AUX_TIMER_CLEAR(x)      (CNXT_TIMER_T2CLEAR(x)) /* aux Clk is timer 2 */
#define AUX_TIMER_CTRL(x)       (CNXT_TIMER_T2CTRL(x))
#define AUX_TIMER_LOAD(x)       (CNXT_TIMER_T2LOAD(x))
#define AUX_TIMER_VALUE(x)      (CNXT_TIMER_T2VALUE(x))


/*
*   Added data defines for CNXT registers
*/

/*  Bit Defines         */
#define BIT0             0x00000001
#define BIT1             0x00000002
#define BIT2             0x00000004
#define BIT3             0x00000008
#define BIT4             0x00000010
#define BIT5             0x00000020
#define BIT6             0x00000040
#define BIT7             0x00000080
#define BIT8             0x00000100
#define BIT9             0x00000200
#define BIT10            0x00000400
#define BIT11            0x00000800
#define BIT12            0x00001000
#define BIT13            0x00002000
#define BIT14            0x00004000
#define BIT15            0x00008000
#define BIT16            0x00010000
#define BIT17            0x00020000
#define BIT18            0x00040000
#define BIT19            0x00080000
#define BIT20            0x00100000
#define BIT21            0x00200000
#define BIT22            0x00400000
#define BIT23            0x00800000
#define BIT24            0x01000000
#define BIT25            0x02000000
#define BIT26            0x04000000
#define BIT27            0x08000000
#define BIT28            0x10000000
#define BIT29            0x20000000
#define BIT30            0x40000000
#define BIT31            0x80000000

/*  Interrupts          */
#define INT_SW3         BIT31   /*  Software interrupt  */
#define INT_SW2         BIT30   /*  Software interrupt  */
#define INT_SW1         BIT29   /*  Software interrupt  */
#define INT_SW0         BIT28   /*  Software interrupt  */
#define INT_COMMRX      BIT27   /*  ARM9 Comms Receive buffer contains data  */
#define INT_COMMTX      BIT26   /*  ARM9 Comms Transmit buffer is empty      */
#define INT_RES6        BIT25   /* */
#define INT_GPIO        BIT24   /*  GPIO input interrupt  */
#define INT_RES5        BIT23   /* */
#define INT_RES4        BIT22   /* */
#define INT_ADSL_ERR    BIT21   /*  ADSL error   */
#define INT_EMAC1_ERR   BIT20   /*  EMAC1 error detected */
#define INT_EMAC2_ERR   BIT19   /*  EMAC2 error detected */
#define INT_DMAC_ERR    BIT18   /*  DMAC error detected  */
#define INT_RES3        BIT17   /* */
#define INT_RES2        BIT16   /* */
#define INT_DMA1        BIT15   /*  EMAC2 Tx  */
#define INT_DMA2        BIT14   /*  EMAC2 Rx  */
#define INT_DMA3        BIT13   /*  EMAC1 Tx  */
#define INT_DMA4        BIT12   /*  EMAC1 Rx  */
#define INT_DMA5        BIT11   /*  ADSL Tx   */
#define INT_DMA6        BIT10   /*  ADSL Rx   */
#define INT_RES1        BIT9    /* */
#define INT_DMA8        BIT8    /*  M2M Dst DMA xfer complete           */
#define INT_HOST_ERR    BIT7    /*  External host encountered bus error */
#define INT_HOST        BIT6    /*  External host writes to H_INT       */
#define INT_RES0        BIT5    /* */
#define INT_USB         BIT4    /*  USB interrupt */

#define INT_TM4         BIT3    /*  Reserved                            */
#define INT_TM3         BIT2    /*  Timer 3 interrupt                   */
#define INT_TM2         BIT1    /*  Timer 2 interrupt                   */
#define INT_TM1         BIT0    /*  Timer 1 interrupt                   */

#define HW_REG_READ(x,result) \
 	     ((result) = *(volatile UINT32 *)(x))

#define HW_REG_WRITE(x,data) \
	     (*((volatile UINT32 *)(x)) = (data))

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

#endif  /* CNXTBSP_H */

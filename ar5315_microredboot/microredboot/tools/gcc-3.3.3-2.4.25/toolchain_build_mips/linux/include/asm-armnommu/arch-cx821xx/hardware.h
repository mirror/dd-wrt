/****************************************************************************
*
*	Name:			hardware.h
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
*  $Modtime: 2/28/02 9:54a $
****************************************************************************/

#ifndef __ASM_ARCH_HARDWARE_H
#define __ASM_ARCH_HARDWARE_H


#define HARD_RESET_NOW()

/*
 * emac
 */

#define E_DMA_1			0x310000
#define E_NA_1			0x310004
#define E_Stat_1		0x310008
#define E_IE_1			0x31000c
#define E_LP_1			0x310010
#define E_MII_1			0x310018
#define ET_DMA_1		0x310020

#define E_DMA_2			0x320000
#define E_NA_2			0x320004
#define E_Stat_2		0x320008
#define E_IE_2			0x32000c
#define E_LP_2			0x320010
#define E_MII_2			0x320018
#define ET_DMA_2		0x320020


/*
 * GPIO
 */

#define CX821xx_GPIO_ISM1  (volatile unsigned long*)0x3500a0
#define CX821xx_GPIO_ISM2  (volatile unsigned long*)0x3500a4
#define CX821xx_GPIO_ISM3  (volatile unsigned long*)0x3500a8

#define CX821xx_GPIO_OE1   (volatile unsigned long*)0x3500b4
#define CX821xx_GPIO_OE2   (volatile unsigned long*)0x3500b8
#define CX821xx_GPIO_OE3   (volatile unsigned long*)0x3500bc

#define CX821xx_GPIO_DIN1  (volatile unsigned long*)0x3500c0
#define CX821xx_GPIO_DIN2  (volatile unsigned long*)0x3500c4
#define CX821xx_GPIO_DIN3  (volatile unsigned long*)0x3500c8

#define CX821xx_GPIO_DOUT1 (volatile unsigned long*)0x3500cc
#define CX821xx_GPIO_DOUT2 (volatile unsigned long*)0x3500d0
#define CX821xx_GPIO_DOUT3 (volatile unsigned long*)0x3500d4

#define CX821xx_GPIO_ISR1  (volatile unsigned long*)0x3500d8
#define CX821xx_GPIO_ISR2  (volatile unsigned long*)0x3500dc
#define CX821xx_GPIO_ISR3  (volatile unsigned long*)0x3500e0

#define CX821xx_GPIO_IER1  (volatile unsigned long*)0x3500e4
#define CX821xx_GPIO_IER2  (volatile unsigned long*)0x3500e8
#define CX821xx_GPIO_IER3  (volatile unsigned long*)0x3500ec

#define CX821xx_GPIO_IPC1  (volatile unsigned long*)0x3500f0
#define CX821xx_GPIO_IPC2  (volatile unsigned long*)0x3500f4
#define CX821xx_GPIO_IPC3  (volatile unsigned long*)0x3500f8

/* 
 * interrupts 
 */
#ifdef __ASSEMBLY__
#define	CNXT_INT_STATUS   0x350044
#define	CNXT_INT_MASK     0x35004C 
#define CNXT_INT_STATUS_M 0x350090
#else
#define	CNXT_INT_STATUS   (volatile unsigned long*)0x350044
#define	CNXT_INT_MASK     (volatile unsigned long*)0x35004C 
#define CNXT_INT_STATUS_M (volatile unsigned long*)0x350090
#endif

/*
 * timers
 */

#define CX821xx_TIMBase_Addr     0x350020

#define TM_Cnt1          0x00
#define TM_Cnt2          0x04
#define TM_Cnt3          0x08
#define TM_Cnt4          0x0C

#define TM_Lmt1          0x10
#define TM_Lmt2          0x14
#define TM_Lmt3          0x18
#define TM_Lmt4          0x1C

#define TIM_SET_RATE(tmr, value) *((volatile unsigned long *)(CX821xx_TIMBase_Addr+tmr)) = value


#define ASB_RAM_ADRS      0x00180000
#define CX821xx_CLK_SPEED     1000000         /* Timer speed is 1 microsecond*/

#endif  /* _ASM_ARCH_HARDWARE_H */



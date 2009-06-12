/****************************************************************************
*
*	Name:			irqs.h
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
*  $Modtime: 2/28/02 9:55a $
****************************************************************************/

#ifndef __ASM_ARCH_IRQS_H__
#define __ASM_ARCH_IRQS_H__

#define CNXT_INT_LVL_TIMER_1            0       /* Timer 0 interrupt */
#define CNXT_INT_LVL_TIMER_2            1       /* Timer 1 interrupt */
#define CNXT_INT_LVL_TIMER_3            2       /* Timer 2 interrupt */
#define CNXT_INT_LVL_TIMER_4            3       /* Timer 3 interrupt */
#define CNXT_INT_LVL_USB                4       /* USB interrupt */
#define CNXT_INT_LVL_RSVD1              5       /* Reserved */
#define CNXT_INT_LVL_HOST               6       /* External Host interrupt */
#define CNXT_INT_LVL_HOST_ERR           7       /* Host Bus error detect */
#define CNXT_INT_LVL_DMA8               8       /* M2M */
#define CNXT_INT_LVL_RSVD2              9       /* Reserved */
#define CNXT_INT_LVL_DMA6              10       /* DSL Rx */
#define CNXT_INT_LVL_DMA5              11       /* DSL Tx */
#define CNXT_INT_LVL_DMA4              12       /* Ethernet 2 Receive */
#define CNXT_INT_LVL_DMA3              13       /* Ethernet 2 Transmit */
#define CNXT_INT_LVL_DMA2              14       /* Ethernet 1 Receive */
#define CNXT_INT_LVL_DMA1              15       /* Ethernet 1 Transmit */
#define CNXT_INT_LVL_RSVD3             16       /* Reserved */
#define CNXT_INT_LVL_RSVD4             17       /* Reserved */
#define CNXT_INT_LVL_DMA_ERR           18       /* DMA Errort */
#define CNXT_INT_LVL_E1_ERR            19       /* EMAC 1 error */
#define CNXT_INT_LVL_E2_ERR            20       /* EMAC 2 error */
#define CNXT_INT_LVL_DSL               21       /* DSL intr*/
#define CNXT_INT_LVL_RSVD5             22       /* Reserved */
#define CNXT_INT_LVL_RSVD6             23       /* Reserved */
#define CNXT_INT_LVL_GPIO              24       /* GPIO interrupts */
#define CNXT_INT_LVL_RSVD7             25       /* Reserved */
#define CNXT_INT_LVL_COMMTX            26       /* ARM9 Comms Xmit */
#define CNXT_INT_LVL_COMMRX            27       /* ARM9 Comms Rcv */
#define CNXT_INT_LVL_SW1               28       /* Software interrupt 1 */
#define CNXT_INT_LVL_SW2               29       /* Software interrupt 2 */
#define CNXT_INT_LVL_SW3               30       /* Software interrupt 3 */
#define CNXT_INT_LVL_SW4               31       /* Software interrupt 4 */

#define CNXT_INT_MASK_TIMER_1        1
#define CNXT_INT_MASK_TIMER_2        (1 << 1)
#define CNXT_INT_MASK_TIMER_3        (1 << 2)
#define CNXT_INT_MASK_TIMER_4        (1 << 3)
#define CNXT_INT_MASK_USB            (1 << 4)
#define CNXT_INT_MASK_RSVD1          (1 << 5)
#define CNXT_INT_MASK_HOST           (1 << 6)
#define CNXT_INT_MASK_HOST_ERR       (1 << 7)
#define CNXT_INT_MASK_DMA8           (1 << 8)
#define CNXT_INT_MASK_RSVD2          (1 << 9)
#define CNXT_INT_MASK_DMA6           (1 << 10)
#define CNXT_INT_MASK_DMA5           (1 << 11)
#define CNXT_INT_MASK_DMA4           (1 << 12)
#define CNXT_INT_MASK_DMA3           (1 << 13)
#define CNXT_INT_MASK_DMA2           (1 << 14)
#define CNXT_INT_MASK_DMA1           (1 << 15)
#define CNXT_INT_MASK_RSVD3          (1 << 16)
#define CNXT_INT_MASK_RSVD4          (1 << 17)
#define CNXT_INT_MASK_DMA_ERR        (1 << 18)
#define CNXT_INT_MASK_E1_ERR         (1 << 19)
#define CNXT_INT_MASK_E2_ERR         (1 << 20)
#define CNXT_INT_MASK_DSL            (1 << 21)
#define CNXT_INT_MASK_RSVD5          (1 << 22)
#define CNXT_INT_MASK_RSVD6          (1 << 23)
#define CNXT_INT_MASK_GPIO           (1 << 24)
#define CNXT_INT_MASK_RSVD7          (1 << 25)
#define CNXT_INT_MASK_COMMTX         (1 << 26)
#define CNXT_INT_MASK_COMMRX         (1 << 27)
#define CNXT_INT_MASK_SW1            (1 << 28)
#define CNXT_INT_MASK_SW2            (1 << 29)
#define CNXT_INT_MASK_SW3            (1 << 30)
#define CNXT_INT_MASK_SW4            (1 << 31)

#define NR_IRQS           32

/* alias */
#define IRQ_TIMER         CNXT_INT_LVL_TIMER_1

/* flags for request_irq() */
#define IRQ_FLG_LOCK      (0x0001)  /* handler is not replaceable   */
#define IRQ_FLG_REPLACE   (0x0002)  /* replace existing handler     */
#define IRQ_FLG_FAST      (0x0004)
#define IRQ_FLG_SLOW      (0x0008)
#define IRQ_FLG_STD       (0x8000)  /* internally used              */

#endif /* __ASM_ARCH_IRQS_H__ */

/*
 *
 * BRIEF MODULE DESCRIPTION
 *      Helpfile for banyan.c
 *
 * Copyright 2002 MontaVista Software Inc.
 * Author: MontaVista Software, Inc.
 *         	stevel@mvista.com or source@mvista.com
 *
 * Heavily modified from original version by:
 *
 * (C) 2001, IDT Inc.
 *
 * ########################################################################
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 *  THIS  SOFTWARE  IS PROVIDED   ``AS  IS'' AND   ANY  EXPRESS OR IMPLIED
 *  WARRANTIES,   INCLUDING, BUT NOT  LIMITED  TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 *  NO  EVENT  SHALL   THE AUTHOR  BE    LIABLE FOR ANY   DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED   TO, PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS OF
 *  USE, DATA,  OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  You should have received a copy of the  GNU General Public License along
 *  with this program; if not, write  to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * ########################################################################
 */

#ifndef ETHACACIA_H
#define ETHACACIA_H

#include  "types.h"
#include  "dma_v.h"
#include  "eth_v.h"

#define ETH0_DMA_RX_IRQ   GROUP1_IRQ_BASE + 2
#define ETH0_DMA_TX_IRQ   GROUP1_IRQ_BASE + 3
#define ETH0_RX_OVR_IRQ   GROUP3_IRQ_BASE + 12
#define ETH1_DMA_RX_IRQ   GROUP1_IRQ_BASE + 4
#define ETH1_DMA_TX_IRQ   GROUP1_IRQ_BASE + 5
#define ETH1_RX_OVR_IRQ   GROUP3_IRQ_BASE + 15


/* cgg - the following must be powers of two */
#define ACACIA_NUM_RDS    32    /* number of receive descriptors */
#define ACACIA_NUM_TDS    32    /* number of transmit descriptors */

#define ACACIA_RBSIZE     1536  /* size of one resource buffer = Ether MTU */
#define TX_RX_BUF_SIZE    2048

#define ACACIA_RDS_MASK   (ACACIA_NUM_RDS-1)
#define ACACIA_TDS_MASK   (ACACIA_NUM_TDS-1)
#define RD_RING_SIZE (ACACIA_NUM_RDS * sizeof(struct DMAD_s))
#define TD_RING_SIZE (ACACIA_NUM_TDS * sizeof(struct DMAD_s))

#define ACACIA_TX_TIMEOUT HZ/4

#define rc32438_eth0_regs ((ETH_t)(ETH0_VirtualAddress);
#define rc32438_eth1_regs ((ETH_t)(ETH1_VirtualAddress);

typedef struct TxRxBuf_s {
	char                 data[TX_RX_BUF_SIZE];
} TxRxBuf_t;

/* Information that need to be kept for each board. */
struct acacia_local {
	ETH_t  eth_regs;
	DMA_Chan_t  rx_dma_regs;
	DMA_Chan_t  tx_dma_regs;
 	volatile DMAD_t   td_ring;  /* transmit descriptor ring */ 
	volatile DMAD_t   rd_ring;  /* receive descriptor ring  */

	U8*     rba;               /* start of rx buffer areas */  
 	TxRxBuf_t *tx_buf[ACACIA_NUM_TDS]; /* buffs for pkt to trans */
 	TxRxBuf_t *rx_buf[ACACIA_NUM_RDS]; /* buffs for pkt to trans */

	int     rx_next_out;
	int     tx_next_in;    	   /* next trans packet */
	int     tx_next_out;       /* last packet processed by ISR */
	int     tx_count;          /* current # of pkts waiting to be sent */

	int     tx_full;

	int     rx_irq;
	int     tx_irq;
	int     ovr_irq;

	int dma_halt_cnt;    U32 halt_tx_count;
	int dma_collide_cnt; U32 collide_tx_count;
	int dma_run_cnt;     U32 run_tx_count;
	int dma_race_cnt;    U32 race_tx_count;
};

#define IDT_LONG_WR(addr, value)				\
	{							\
		*((volatile U32 *)(addr)) = (U32)(value);	\
	}

#endif /* ETHACACIA_H */

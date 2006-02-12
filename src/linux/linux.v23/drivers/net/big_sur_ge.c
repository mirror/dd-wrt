/*
 * drivers/net/big_sur_ge.c - Driver for PMC-Sierra Big Sur
 * ethernet ports
 *
 * Copyright (C) 2003, 2004 PMC-Sierra Inc.
 * Author : Manish Lachwani (lachwani@pmc-sierra.com)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

/*
 * This driver also includes the PHY related stuff. The device
 * is a fast ethernet device. But, there is a Gigabit unit
 * in the works. When that is ready, this driver will support
 * it. 
 *
 * Basic Operation
 * ================
 * The device operates in following modes:
 * -> Polled mode, no DMA (FIFO)
 * -> Polled mode, Simple DMA
 * -> Interrupt mode, Simple DMA
 * -> Interrupt mode, Scatter Gather DMA
 *
 * Scatter Gather DMA does not work. So, we make use of Simple DMA
 * mode here. There is no implementation of ring descriptors here
 * since it is not working as yet. Note that there could be an enhancement
 * whereby the driver can support FIFO mode or Simple DMA. However,
 * this version only supports Simple DMA.
 *
 * Receive Operation
 * ==================
 * The device DMA's sends an interrupt to the processor. The driver 
 * is invoked and it determines if the packet is to be received. If 
 * yes, it allocates a new skb and has the DMA controller transfer the 
 * data into the newly allocated skb. The receive side handles one 
 * packet at a time. The Rx FIFO on the device is 2000 bytes.
 *
 * Send Operation
 * ===============
 * The device Transmit FIFO is 2000 bytes long, i.e. one packet. When
 * sending, the driver makes sure that the packet is aligned. Once
 * done, it asks the device to do a DMA transfer from memory to the 
 * onboard FIFO. The device is stopped till the transfer completes since
 * it can DMA only one packet at a time.
 *
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/mii.h>
#include <linux/interrupt.h>
#include <asm/io.h>
#include <asm/irq.h>

#include "big_sur_ge.h"

MODULE_AUTHOR("Manish Lachwani <lachwani@pmc-sierra.com>");
MODULE_DESCRIPTION("PMC-Sierra Big Sur Ethernet MAC Driver");
MODULE_LICENSE("GPL");

#define TX_TIMEOUT (60 * HZ)      /* Transmit timeout */

typedef enum DUPLEX { UNKNOWN, HALF_DUPLEX, FULL_DUPLEX } DUPLEX;

/* Big Sur Ethernet MAC structure */
struct big_sur_ge_enet {
        struct net_device_stats *stats;	/* Statistics for this device */
        struct timer_list phy_timer;    /* PHY monitoring timer */
        u32 index;             		/* Which interface is this */
        u32 save_base_address; 		/* Saved physical base address */
        struct sk_buff *saved_skb;      /* skb being transmitted */
        spinlock_t lock;            	/* For atomic access to saved_skb */
        u8 mii_addr;        		/* The MII address of the PHY */
        big_sur_ge *emac;               /* GE driver structure */
	struct tasklet_struct big_sur_tasklet;	/* Tasklet structure */
};

extern unsigned char big_sur_mac_addr_base[6];

/*
 * Function Prototypes 
 */
unsigned long big_sur_ge_dma_control(xdma_channel *);
void big_sur_ge_dma_reset(xdma_channel *);
static void handle_intr(struct net_device *, big_sur_ge *);
void big_sur_ge_check_fifo_recv_error(struct net_device *, big_sur_ge *);
void big_sur_ge_check_fifo_send_error(struct net_device *, big_sur_ge *);
static int big_sur_ge_config_fifo(big_sur_ge *);
static int big_sur_ge_config_dma(big_sur_ge *);
void big_sur_ge_enet_reset(big_sur_ge *);
void big_sur_ge_check_mac_error(big_sur_ge *, unsigned long);
static void big_sur_receive(struct net_device *);
static void big_sur_tx_free_skb(struct net_device *);
big_sur_ge_config *big_sur_ge_get_config(int);
static void big_sur_ge_reset(struct net_device *,DUPLEX);

/*
 * DMA Channel Initialization. In case of Simple DMA,
 * not much to do here. However, in case of SG DMA, we
 * need to intialize the descriptor pointers 
 */
int big_sur_ge_dma_init(xdma_channel *dma, unsigned long base_address) 
{
	dma->reg_base_address = base_address;
	dma->ready = 1; 

	big_sur_ge_dma_reset(dma);
	return 0;
}

/*
 * Perform the self test on the DMA channel 
 */
#define BIG_SUR_GE_CONTROL_REG_RESET_MASK	0x98000000

int big_sur_ge_dma_self_test(xdma_channel *dma)
{
	unsigned long	reg_data;

	big_sur_ge_dma_reset(dma);
	
	reg_data = big_sur_ge_dma_control(dma);
	if (reg_data != BIG_SUR_GE_CONTROL_REG_RESET_MASK) {
		printk(KERN_ERR "DMA Channel Self Test Failed \n");
		return -1;
	}
	
	return 0;
}

/*
 * Reset the DMA channel
 */
void big_sur_ge_dma_reset(xdma_channel *dma)
{
	BIG_SUR_GE_WRITE(dma->reg_base_address + BIG_SUR_GE_RST_REG_OFFSET,
						BIG_SUR_GE_RESET_MASK);
}

/*
 * Get control register from the DMA channel
 */
unsigned long big_sur_ge_dma_control(xdma_channel *dma)
{
	return BIG_SUR_GE_READ(dma->reg_base_address + BIG_SUR_GE_DMAC_REG_OFFSET);
}

/*
 * Set control register of the DMA channel
 */
void big_sur_ge_set_dma_control(xdma_channel *dma, unsigned long control)
{
	BIG_SUR_GE_WRITE(dma->reg_base_address + BIG_SUR_GE_DMAC_REG_OFFSET, control);
}

/*
 * Get the status of the DMA channel 
 */
unsigned long big_sur_ge_dma_status(xdma_channel *dma)
{
	return BIG_SUR_GE_READ(dma->reg_base_address + BIG_SUR_GE_DMAS_REG_OFFSET);
}

/*
 * Transfer the data over the DMA channel
 */
void big_sur_ge_dma_transfer(xdma_channel *dma, unsigned long *source,
				unsigned long *dest, unsigned long length)
{
	/* Source Address */
	BIG_SUR_GE_WRITE(dma->reg_base_address + BIG_SUR_GE_SA_REG_OFFSET, 
				(unsigned long)source);

	/* Destination Address */
	BIG_SUR_GE_WRITE(dma->reg_base_address + BIG_SUR_GE_DA_REG_OFFSET,
				(unsigned long)dest);

	/* Length of the data */
	BIG_SUR_GE_WRITE(dma->reg_base_address + BIG_SUR_GE_LEN_REG_OFFSET,
				length);
}

/*
 * Init the packet fifo only in the FIFO mode
 */
int packet_fifo_init(packet_fifo *fifo, u32 reg, u32 data)
{
	fifo->reg_base_addr = reg;
	fifo->data_base_address = data;
	fifo->ready_status = 1;

	BIG_SUR_GE_FIFO_RESET(fifo);

	return 0;
}

/*
 * Write to the packet FIFO, 32-bit at a time. 
 */
static int packet_fifo_write(packet_fifo * fifo, int *buffer, int len)
{
	unsigned long fifo_count, word_count, extra_byte;
	unsigned long *buffer_data = (unsigned long *) buffer;

	fifo_count =
	    BIG_SUR_GE_READ(fifo->reg_base_addr +
			    BIG_SUR_GE_FIFO_WIDTH_BYTE_COUNT);
	fifo_count &= BIG_SUR_GE_COUNT_MASK;

	word_count = len / BIG_SUR_GE_FIFO_WIDTH_BYTE_COUNT;
	extra_byte = len % BIG_SUR_GE_FIFO_WIDTH_BYTE_COUNT;

	if (extra_byte > 0)
		if (fifo_count > (word_count + 1)) {
			printk(KERN_ERR
			       "No room in the packet send fifo \n");
			return -1;
		}

	for (fifo_count = 0; fifo_count < word_count; fifo_count++)
		BIG_SUR_GE_WRITE(fifo->data_base_address,
				 buffer_data[fifo_count]);

	if (extra_byte > 0) {
		unsigned long last_word = 0;
		int *extra_buffer_data =
		    (int *) (buffer_data + word_count);

		if (extra_byte == 1)
			last_word = extra_buffer_data[0] << 24;
		else if (extra_byte == 2)
			last_word = (extra_buffer_data[0] << 24 |
				     extra_buffer_data[1] << 16);

		else if (extra_byte == 3)
			last_word = (extra_buffer_data[0] << 24 |
				     extra_buffer_data[1] << 16 |
				     extra_buffer_data[2] << 8);


		BIG_SUR_GE_WRITE(fifo->data_base_address, last_word);
	}

	return 0;
}

/*
 * Start transmitting the packet 
 */
int big_sur_tx(big_sur_ge *emac, u8 *buffer, unsigned long byte_cnt)
{
	unsigned long int_status, reg_data, status_reg;

	if ( (!emac->started) && (emac->polled) &&
		(emac->dma_sg) ) 
		return -1;

	/* There is space in the FIFO */
	int_status = BIG_SUR_GE_READ(emac->base_address + XIIF_V123B_IISR_OFFSET);
	if (int_status & BIG_SUR_GE_EIR_XMIT_LFIFO_FULL_MASK) {
		printk(KERN_ERR "Tx FIFO error: Queue is Full \n");
		return -1;
	}

	/*
	 * Check if there is enough space for this packet
	 */
	if ((BIG_SUR_GE_GET_COUNT(&emac->send_fifo) * sizeof(unsigned long)) < byte_cnt) {
		printk(KERN_ERR "Send FIFO on chip is full \n");
		return -1;
	}

	if (emac->has_dma == 0) {
		/* Write to the Send FIFO */
		if (packet_fifo_write(&emac->send_fifo, buffer, byte_cnt) == -1) {
			printk(KERN_ERR "Error : Could not write to FIFO \n");
			return -1;
		}

		/* Write the MSB of the length */
		BIG_SUR_GE_WRITE(emac->base_address + 0x1FF4, (byte_cnt & 0xff00));

		/* Write the LSB of the length */
		BIG_SUR_GE_WRITE(emac->base_address + 0x1FF8, (byte_cnt & 0x00ff));

		/* Write the Status bit */
		BIG_SUR_GE_WRITE(emac->base_address + 0x1FFC, 0x80000000);
	
		/* Make sure MAC has done transmitting */	
		status_reg = BIG_SUR_GE_READ(emac->base_address + 0x1FFC);
		while (!(status_reg & 0x80000000)) {
			status_reg = BIG_SUR_GE_READ(emac->base_address + 0x1FFC);
			if (!(status_reg & 0x80000000))
				break;
		}

	}
	else {
		/* DMA Engine is not buzy */
		if (big_sur_ge_dma_status(&emac->send_channel) & BIG_SUR_GE_DMASR_BUSY_MASK) {
			printk(KERN_ERR "Send channel FIFO engine busy \n");
			return -1;
		}

		/* Get ready to transfer the data */
		big_sur_ge_set_dma_control(&emac->send_channel, 
					BIG_SUR_GE_DMACR_SOURCE_INCR_MASK |
					BIG_SUR_GE_DMACR_DEST_LOCAL_MASK  |
					BIG_SUR_GE_DMACR_SG_DISABLE_MASK);

		big_sur_ge_dma_transfer(&emac->send_channel, (unsigned long *)buffer, 
					(unsigned long *)(emac->base_address + 
					BIG_SUR_GE_PFIFO_TXDATA_OFFSET), byte_cnt);

		/* Check the DMA Engine status */	
		reg_data = big_sur_ge_dma_status(&emac->send_channel);
		while (reg_data & BIG_SUR_GE_DMASR_BUSY_MASK) {
                	reg_data = big_sur_ge_dma_status(&emac->recv_channel);
	                if (!(reg_data & BIG_SUR_GE_DMASR_BUSY_MASK))
        	                break;
        	}

		/* Check for any DMA errors */
		if ( (reg_data & BIG_SUR_GE_DMASR_BUS_ERROR_MASK) || 
			(reg_data & BIG_SUR_GE_DMASR_BUS_TIMEOUT_MASK)) {
				printk(KERN_ERR "Send side DMA error \n");
				return -1;
		}

		/* Write the packet length */
		BIG_SUR_GE_WRITE(emac->base_address + BIG_SUR_GE_TPLR_OFFSET, byte_cnt);
	}

	/* Good Send */
	return 0;
}

/*
 * Read the packet FIFO
 */
static int packet_fifo_read(packet_fifo * fifo, u8 * buffer, unsigned int len)
{
	unsigned long fifo_count, word_count, extra_byte;
	unsigned long *buffer_data = (unsigned long *) buffer;

	fifo_count =
	    BIG_SUR_GE_READ(fifo->reg_base_addr +
			    BIG_SUR_GE_FIFO_WIDTH_BYTE_COUNT);
	fifo_count &= BIG_SUR_GE_COUNT_MASK;

	if ((fifo_count * BIG_SUR_GE_FIFO_WIDTH_BYTE_COUNT) < len)
		return -1;

	word_count = len / BIG_SUR_GE_FIFO_WIDTH_BYTE_COUNT;
	extra_byte = len % BIG_SUR_GE_FIFO_WIDTH_BYTE_COUNT;

	for (fifo_count = 0; fifo_count < word_count; fifo_count++)
		buffer_data[fifo_count] =
		    BIG_SUR_GE_READ(fifo->reg_base_addr);

	if (extra_byte > 0) {
		unsigned long last_word;
		int *extra_buffer_data =
		    (int *) (buffer_data + word_count);

		last_word = BIG_SUR_GE_READ(fifo->data_base_address);
		if (extra_byte == 1)
			extra_buffer_data[0] = (int) (last_word << 24);
		else if (extra_byte == 2) {
			extra_buffer_data[0] = (int) (last_word << 24);
			extra_buffer_data[1] = (int) (last_word << 16);
		} else if (extra_byte == 3) {
			extra_buffer_data[0] = (int) (last_word << 24);
			extra_buffer_data[1] = (int) (last_word << 16);
			extra_buffer_data[2] = (int) (last_word << 8);
		}
	}

	return 0;
}

/*
 * FIFO receive for Simple DMA case
 */
int big_sur_rx(big_sur_ge *emac, u8 *buffer, unsigned long *byte_cnt)
{
	unsigned long int_status, reg_data, packet_length;

        if ( (!emac->started) && (emac->polled) &&
		(emac->dma_sg) ) {
                return -1;
	}

	/* Not enough space in the current buffer */	
	if (*byte_cnt < BIG_SUR_GE_MAX_FRAME_SIZE)
		return -1;

	int_status = BIG_SUR_GE_READ(emac->base_address + XIIF_V123B_IISR_OFFSET);

	/* FIFO is empty */
	if (int_status & BIG_SUR_GE_EIR_RECV_LFIFO_EMPTY_MASK) {
		BIG_SUR_GE_WRITE(emac->base_address + XIIF_V123B_IISR_OFFSET, 
					BIG_SUR_GE_EIR_RECV_LFIFO_EMPTY_MASK);
		return -1;
	}

	packet_length = BIG_SUR_GE_READ(emac->base_address + BIG_SUR_GE_RPLR_OFFSET);

	if (emac->has_dma == 0) {
		reg_data = BIG_SUR_GE_READ(emac->base_address + 0x3FFC);
		if (reg_data & 0x80000000) {
			if (packet_fifo_read(&emac->recv_fifo, buffer, packet_length) == -1) {
				printk(KERN_ERR "Could not read the packet fifo \n");
				return -1;
			}

			BIG_SUR_GE_WRITE(emac->base_address + 0x3FFC, 0x0);
		}
	}
	else {	
		/* Rx side DMA engine */
		if (big_sur_ge_dma_status(&emac->recv_channel) & BIG_SUR_GE_DMASR_BUSY_MASK) {
			printk(KERN_ERR "Rx side DMA Engine busy \n");
			return -1;
		}

		if (packet_length == 0) {
			printk(KERN_ERR "MAC has the FIFO packet length 0 \n");
			return -1;
		}

		/* For the simple DMA case only */
		big_sur_ge_set_dma_control(&emac->recv_channel, 
							BIG_SUR_GE_DMACR_DEST_INCR_MASK |
							BIG_SUR_GE_DMACR_SOURCE_LOCAL_MASK |
							BIG_SUR_GE_DMACR_SG_DISABLE_MASK);

		big_sur_ge_dma_transfer(&emac->recv_channel, (unsigned long *)
					(emac->base_address + 
					BIG_SUR_GE_PFIFO_RXDATA_OFFSET), (unsigned long *)
					buffer, packet_length);

		reg_data = big_sur_ge_dma_status(&emac->recv_channel);
		while (reg_data & BIG_SUR_GE_DMASR_BUSY_MASK) {
			reg_data = big_sur_ge_dma_status(&emac->recv_channel);
			if (!(reg_data & BIG_SUR_GE_DMASR_BUSY_MASK))
					break;
		}	

		if ( (reg_data & BIG_SUR_GE_DMASR_BUS_ERROR_MASK) || 
			(reg_data & BIG_SUR_GE_DMASR_BUS_TIMEOUT_MASK)) {
				printk(KERN_ERR "DMA Bus Error \n");
				return -1;
		}
	}

	*byte_cnt = packet_length;
	
	return 0;
}

/*
 * Main FIFO Interrupt Handler
 */
void big_sur_ge_fifo_intr(unsigned long data)
{
	struct net_device *netdev = (struct net_device *)data;
	struct big_sur_ge_enet *lp = (struct big_sur_ge_enet *)netdev->priv;
	big_sur_ge *emac = (big_sur_ge *)lp->emac;

	/* Read the interrupt status */
	unsigned long int_status = BIG_SUR_GE_READ(emac->base_address +
							XIIF_V123B_DIPR_OFFSET);

	/* Handle Rx and Tx */
	if (int_status & BIG_SUR_GE_IPIF_EMAC_MASK)
		handle_intr(netdev, emac);

	/* Handle Receive Error */
	if (int_status & BIG_SUR_GE_IPIF_RECV_FIFO_MASK)
		big_sur_ge_check_fifo_recv_error(netdev, emac);

	/* Handle Transmit Error */
	if (int_status & BIG_SUR_GE_IPIF_SEND_FIFO_MASK)
		big_sur_ge_check_fifo_send_error(netdev, emac);

	/* Clear the errors */
	if (int_status & XIIF_V123B_ERROR_MASK)
		BIG_SUR_GE_WRITE(emac->base_address + XIIF_V123B_DISR_OFFSET, 
							XIIF_V123B_ERROR_MASK);
}

/*
 * Tasklet function to invoke interrupt
 */
void big_sur_tasklet_schedule(void *data)
{
	struct net_device *netdev = (struct net_device *)data;
	struct big_sur_ge_enet *lp = (struct big_sur_ge_enet *)netdev->priv;

	tasklet_schedule(&lp->big_sur_tasklet);
	
	return;
}

/*
 * Main intr handler 
 */
static void handle_intr(struct net_device *netdev, big_sur_ge *emac)
{
	unsigned long int_status = BIG_SUR_GE_READ(emac->base_address + 
						XIIF_V123B_IISR_OFFSET);

	/* Process the Rx side */
	if (int_status & BIG_SUR_GE_EIR_RECV_DONE_MASK) {
		big_sur_receive(netdev);
		BIG_SUR_GE_WRITE(emac->base_address + XIIF_V123B_IISR_OFFSET, 
						BIG_SUR_GE_EIR_RECV_DONE_MASK);
	}

	/* Process the Tx side */
	if (int_status & BIG_SUR_GE_EIR_XMIT_DONE_MASK) {
		big_sur_tx_free_skb(netdev);
		BIG_SUR_GE_WRITE(emac->base_address + XIIF_V123B_IISR_OFFSET,
                                                BIG_SUR_GE_EIR_XMIT_DONE_MASK);
	}

	big_sur_ge_check_mac_error(emac, int_status);
}
/*
 * For now, the MAC address errors dont trigger a update of the 
 * stats. There is no stats framework in place. Hence, we just
 * check for the errors below and do a reset if needed. 
 */
void big_sur_ge_check_mac_error(big_sur_ge *emac, unsigned long int_status)
{
	if (int_status & (BIG_SUR_GE_EIR_RECV_DFIFO_OVER_MASK | 
				BIG_SUR_GE_EIR_RECV_LFIFO_OVER_MASK |
				BIG_SUR_GE_EIR_RECV_LFIFO_UNDER_MASK |
				BIG_SUR_GE_EIR_RECV_ERROR_MASK |
				BIG_SUR_GE_EIR_RECV_MISSED_FRAME_MASK |
				BIG_SUR_GE_EIR_RECV_COLLISION_MASK |
				BIG_SUR_GE_EIR_RECV_FCS_ERROR_MASK |
				BIG_SUR_GE_EIR_RECV_LEN_ERROR_MASK |
				BIG_SUR_GE_EIR_RECV_SHORT_ERROR_MASK |
				BIG_SUR_GE_EIR_RECV_LONG_ERROR_MASK |
				BIG_SUR_GE_EIR_RECV_ALIGN_ERROR_MASK |
				BIG_SUR_GE_EIR_XMIT_SFIFO_OVER_MASK |
				BIG_SUR_GE_EIR_XMIT_LFIFO_OVER_MASK |
				BIG_SUR_GE_EIR_XMIT_SFIFO_UNDER_MASK |
				BIG_SUR_GE_EIR_XMIT_LFIFO_UNDER_MASK) ) {

		BIG_SUR_GE_WRITE(emac->base_address + XIIF_V123B_IIER_OFFSET, 0);
		big_sur_ge_enet_reset(emac);
	}
}

/*
 * Check for FIFO Recv errors
 */
void big_sur_ge_check_fifo_recv_error(struct net_device *netdev, big_sur_ge *emac)
{
	if (BIG_SUR_GE_IS_DEADLOCKED(&emac->recv_fifo)) {
		unsigned long intr_enable;

		/*
		 * The only way to ack this interrupt is to reset the 
		 * device. However, before we reset, we make sure this
		 * interrupt is disabled
		 */
		intr_enable = BIG_SUR_GE_READ(emac->base_address + XIIF_V123B_DIER_OFFSET);
		BIG_SUR_GE_WRITE(emac->base_address + XIIF_V123B_DIER_OFFSET,
					(intr_enable & ~(BIG_SUR_GE_IPIF_RECV_FIFO_MASK)));

		/* Reset the device */
		big_sur_ge_reset(netdev, UNKNOWN);

		/* Turn the interrupts back on */
		BIG_SUR_GE_WRITE(emac->base_address + XIIF_V123B_DIER_OFFSET,
					(intr_enable | BIG_SUR_GE_IPIF_RECV_FIFO_MASK));

	}
}

/*
 * Check for FIFO Send errors
 */
void big_sur_ge_check_fifo_send_error(struct net_device *netdev, big_sur_ge *emac)
{
        if (BIG_SUR_GE_IS_DEADLOCKED(&emac->send_fifo)) {
		unsigned long intr_enable;

		/* Disable the interrupt first */
		intr_enable = BIG_SUR_GE_READ(emac->base_address + XIIF_V123B_DIER_OFFSET);
		BIG_SUR_GE_WRITE(emac->base_address + XIIF_V123B_DIER_OFFSET,
                                        (intr_enable & ~(BIG_SUR_GE_IPIF_SEND_FIFO_MASK)));

		/* Reset the device */
		big_sur_ge_reset(netdev, UNKNOWN);

		/* Turn the interrupts back on */
		BIG_SUR_GE_WRITE(emac->base_address + XIIF_V123B_DIER_OFFSET,
					(intr_enable | BIG_SUR_GE_IPIF_SEND_FIFO_MASK));
	}
}

/*
 * GE unit init
 */
int big_sur_ge_enet_init(big_sur_ge *emac, unsigned int device_id)
{
	big_sur_ge_config *config;
	int err;
	
	/* Assume that the device has been stopped */
	config = big_sur_ge_get_config(device_id);
	if (config == NULL)
		return -1;

	emac->ready = 0;
	emac->started = 0;
	emac->dma_sg = 0; 
	emac->has_mii = config->has_mii;
	emac->has_mcast_hash_table = 0;
	emac->dma_config = config->dma_config;
	emac->base_address = config->base_address;

	if (big_sur_ge_config_dma(emac) == -1)
		return -1;

	if (emac->has_dma == 0) {
		err = big_sur_ge_config_fifo(emac);
		if (err == -1)
			return err;
	}

	/* Now, we know that the FIFO initialized successfully. So, set the ready flag */
	emac->ready = 1;

	/* Do we need a PHY reset here also. It did cause problems on some boards */	
	big_sur_ge_enet_reset(emac);

	/* PHY reset code. Remove if causes a problem on the board */
	big_sur_ge_reset_phy(emac->base_address);

	return 0;
}

/*
 * Start the GE unit for Tx, Rx and Interrupts
 */
int big_sur_ge_start(big_sur_ge *emac)
{
	unsigned long		reg_data;
	
	/*
	 * Basic mode of operation is polled and interrupt mode. 
	 * We disable the polled mode for good. We may use the 
	 * polled mode for Rx NAPI but that does not require all 
	 * the interrupts to be disabled
	 */
	emac->polled = 0;

	/*
 	 * DMA: Three modes of operation - simple, FIFO, SG. 
	 * SG is surely not working and so is kept off using the 
	 * dma_sg flag. Simple and FIFO work. But, we may not use FIFO 
	 * at all. So, we enable the interrupts below
	 */
	BIG_SUR_GE_WRITE(emac->base_address + XIIF_V123B_DIER_OFFSET, 
				BIG_SUR_GE_IPIF_FIFO_DFT_MASK | XIIF_V123B_ERROR_MASK);

	BIG_SUR_GE_WRITE(emac->base_address + XIIF_V123B_IIER_OFFSET,
				BIG_SUR_GE_EIR_DFT_FIFO_MASK);

	/* Toggle the started flag */
	emac->started = 1;

	/* Start the Tx and Rx units respectively */
	reg_data = BIG_SUR_GE_READ(emac->base_address + BIG_SUR_GE_ECR_OFFSET);
	reg_data &= ~(BIG_SUR_GE_ECR_XMIT_RESET_MASK | BIG_SUR_GE_ECR_RECV_RESET_MASK);
	reg_data |= (BIG_SUR_GE_ECR_XMIT_ENABLE_MASK | BIG_SUR_GE_ECR_RECV_ENABLE_MASK);
	
	BIG_SUR_GE_WRITE(emac->base_address + BIG_SUR_GE_ECR_OFFSET, reg_data);

	return 0;
}

/*
 * Stop the GE unit
 */
int big_sur_ge_stop(big_sur_ge *emac)
{
	unsigned long	reg_data;

	/* We assume that the device is not already stopped */
	if (!emac->started)
		return 0;

	/* Disable the Tx and Rx unit respectively */
	reg_data = BIG_SUR_GE_READ(emac->base_address + BIG_SUR_GE_ECR_OFFSET);
	reg_data &= ~(BIG_SUR_GE_ECR_XMIT_ENABLE_MASK | BIG_SUR_GE_ECR_RECV_ENABLE_MASK);
	BIG_SUR_GE_WRITE(emac->base_address + BIG_SUR_GE_ECR_OFFSET, reg_data);

	/* Disable the interrupts */
	BIG_SUR_GE_WRITE(emac->base_address + XIIF_V123B_DGIER_OFFSET, 0);

	/* Toggle the started flag */
	emac->started = 0;

	return 0;
}

/*
 * Reset the GE MAC unit 
 */
void big_sur_ge_enet_reset(big_sur_ge *emac)
{
	unsigned long reg_data;

	(void) big_sur_ge_stop(emac);

	BIG_SUR_GE_WRITE(emac->base_address + XIIF_V123B_RESETR_OFFSET, 
			XIIF_V123B_RESET_MASK);
	
	/*
         * For now, configure the receiver to not strip off 
	 * FCS and padding since this is not currently supported. 
	 * In the future, just take the default and provide the option 
	 * for the user to change this behavior.
         */
	reg_data = BIG_SUR_GE_READ(emac->base_address + BIG_SUR_GE_ECR_OFFSET);
	reg_data &= ~(BIG_SUR_GE_ECR_RECV_PAD_ENABLE_MASK | BIG_SUR_GE_ECR_RECV_FCS_ENABLE_MASK);
	reg_data &= ~(BIG_SUR_GE_ECR_RECV_STRIP_ENABLE_MASK);
	BIG_SUR_GE_WRITE(emac->base_address + BIG_SUR_GE_ECR_OFFSET, reg_data);
}

/*
 * Set the MAC address of the GE mac unit 
 */
int big_sur_ge_set_mac_address(big_sur_ge *emac, unsigned char *addr)
{
	unsigned long	mac_addr = 0;

	/* Device is started and so mac address must be set */
	if (emac->started == 1)
		return 0;

	/* Address High */
	mac_addr = ( (addr[0] << 8) | addr[1]);
	BIG_SUR_GE_WRITE(emac->base_address + BIG_SUR_GE_SAH_OFFSET, mac_addr);

	/* Address Low */
	mac_addr |= ( (addr[2] << 24) | (addr[3] << 16) | 
			(addr[4] << 8) | addr[5]);
	BIG_SUR_GE_WRITE(emac->base_address + BIG_SUR_GE_SAL_OFFSET, mac_addr);
	
	return 0;
}

/*
 * Get the MAC address of the GE MAC unit
 */
void big_sur_ge_get_mac_unit(big_sur_ge *emac, unsigned int *addr)
{
	unsigned long	mac_addr_hi, mac_addr_lo;

	mac_addr_hi = BIG_SUR_GE_READ(emac->base_address + BIG_SUR_GE_SAH_OFFSET);
	mac_addr_lo = BIG_SUR_GE_READ(emac->base_address + BIG_SUR_GE_SAL_OFFSET);

	addr[0] = (mac_addr_hi >> 8);
	addr[1] = mac_addr_hi;

	addr[2] = (mac_addr_lo >> 24);
	addr[3] = (mac_addr_lo >> 16);
	addr[4] = (mac_addr_lo >> 8);
	addr[5] = mac_addr_lo;
}

/*
 * Configure the GE MAC for DMA capabilities, only Simple
 */
static int big_sur_ge_config_dma(big_sur_ge *emac)
{
	/* Supports Simple DMA */
	emac->has_dma = 1;

	if (big_sur_ge_dma_init(&emac->recv_channel, emac->base_address +
				BIG_SUR_GE_DMA_RECV_OFFSET) == -1) {
		printk(KERN_ERR "Could not initialize the DMA unit  \n");
		return -1;
	}

	if (big_sur_ge_dma_init(&emac->send_channel, emac->base_address +
                                BIG_SUR_GE_DMA_SEND_OFFSET) == -1) {
		printk(KERN_ERR "Could not initialize the DMA unit  \n");
		return -1;
	}

	return 0;
}

/*
 * Configure the FIFO for simple DMA
 */
static int big_sur_ge_config_fifo(big_sur_ge *emac)
{
	int err = 0;

	/* Receive side packet FIFO */
	err = packet_fifo_init(&emac->recv_fifo, emac->base_address + 
				BIG_SUR_GE_PFIFO_RXREG_OFFSET, emac->base_address +
				BIG_SUR_GE_PFIFO_RXDATA_OFFSET);

	if (err == -1) {
		printk(KERN_ERR "Could not initialize Rx packet FIFO for Simple DMA \n");
		return err;
	}

	/* Send side Packet FIFO */
	err = packet_fifo_init(&emac->send_fifo, emac->base_address +
				BIG_SUR_GE_PFIFO_TXREG_OFFSET, emac->base_address +
				BIG_SUR_GE_PFIFO_TXDATA_OFFSET);

	if (err == -1) {
		printk(KERN_ERR "Could not initialize Tx packet FIFO for Simple DMA \n");
	}

	return err;
}

typedef struct {
        unsigned long   option;
        unsigned long   mask;
} option_map;

static option_map option_table[] = {
        {BIG_SUR_GE_UNICAST_OPTION, BIG_SUR_GE_ECR_UNICAST_ENABLE_MASK},
        {BIG_SUR_GE_BROADCAST_OPTION, BIG_SUR_GE_ECR_BROAD_ENABLE_MASK},
        {BIG_SUR_GE_PROMISC_OPTION, BIG_SUR_GE_ECR_PROMISC_ENABLE_MASK},
        {BIG_SUR_GE_FDUPLEX_OPTION, BIG_SUR_GE_ECR_FULL_DUPLEX_MASK},
        {BIG_SUR_GE_LOOPBACK_OPTION, BIG_SUR_GE_ECR_LOOPBACK_MASK},
        {BIG_SUR_GE_MULTICAST_OPTION, BIG_SUR_GE_ECR_MULTI_ENABLE_MASK},
        {BIG_SUR_GE_FLOW_CONTROL_OPTION, BIG_SUR_GE_ECR_PAUSE_FRAME_MASK},
        {BIG_SUR_GE_INSERT_PAD_OPTION, BIG_SUR_GE_ECR_XMIT_PAD_ENABLE_MASK},
        {BIG_SUR_GE_INSERT_FCS_OPTION, BIG_SUR_GE_ECR_XMIT_FCS_ENABLE_MASK},
        {BIG_SUR_GE_INSERT_ADDR_OPTION, BIG_SUR_GE_ECR_XMIT_ADDR_INSERT_MASK},
        {BIG_SUR_GE_OVWRT_ADDR_OPTION, BIG_SUR_GE_ECR_XMIT_ADDR_OVWRT_MASK},
        {BIG_SUR_GE_STRIP_PAD_OPTION, BIG_SUR_GE_ECR_RECV_PAD_ENABLE_MASK},
        {BIG_SUR_GE_STRIP_FCS_OPTION, BIG_SUR_GE_ECR_RECV_FCS_ENABLE_MASK},
        {BIG_SUR_GE_STRIP_PAD_FCS_OPTION, BIG_SUR_GE_ECR_RECV_STRIP_ENABLE_MASK}
};

#define BIG_SUR_GE_NUM_OPTIONS          (sizeof(option_table) / sizeof(option_map))

/*
 * Set the options for the GE
 */
int big_sur_ge_set_options(big_sur_ge *emac, unsigned long option_flag)
{
        unsigned long reg_data;
        unsigned int index;

        /* Assume that the device is stopped before calling this function */

        reg_data = BIG_SUR_GE_READ(emac->base_address + BIG_SUR_GE_ECR_OFFSET);
        for (index = 0; index < BIG_SUR_GE_NUM_OPTIONS; index++) {
                if (option_flag & option_table[index].option)
                        reg_data |= option_table[index].mask;
                else
                        reg_data &= ~(option_table[index].mask);

        }

        BIG_SUR_GE_WRITE(emac->base_address + BIG_SUR_GE_ECR_OFFSET, reg_data);

        /* No polled option */
        emac->polled = 0;
        return 0;
}

/*
 * Get the options from the GE
 */
unsigned long big_sur_ge_get_options(big_sur_ge *emac)
{
        unsigned long option_flag = 0, reg_data;
        unsigned int index;

        reg_data = BIG_SUR_GE_READ(emac->base_address + BIG_SUR_GE_ECR_OFFSET);

        for (index = 0; index < BIG_SUR_GE_NUM_OPTIONS; index++) {
                if (option_flag & option_table[index].option)
                        reg_data |= option_table[index].mask;
        }

        return option_flag;
}

/*
 * Set the Inter frame gap
 */
int big_sur_ge_set_frame_gap(big_sur_ge *emac, int part1, int part2)
{
        unsigned long config;

        /* Assume that the device is stopped before calling this */

        config = ( (part1 << BIG_SUR_GE_IFGP_PART1_SHIFT) |
                        (part2 << BIG_SUR_GE_IFGP_PART2_SHIFT) );

        BIG_SUR_GE_WRITE(emac->base_address + BIG_SUR_GE_IFGP_OFFSET, config);

        return 0;
}

/*
 * Get the Inter frame gap
 */
void big_sur_ge_get_frame_gap(big_sur_ge *emac, int *part1, int *part2)
{
        unsigned long config;

        config = BIG_SUR_GE_READ(emac->base_address + BIG_SUR_GE_IFGP_OFFSET);
        *part1 = ((config & BIG_SUR_GE_IFGP_PART1_SHIFT) >> BIG_SUR_GE_IFGP_PART1_SHIFT);
        *part2 = ((config & BIG_SUR_GE_IFGP_PART2_SHIFT) >> BIG_SUR_GE_IFGP_PART2_SHIFT);
}

/*
 * PHY specific functions for the MAC
 */
#define	BIG_SUR_GE_MAX_PHY_ADDR		32
#define	BIG_SUR_GE_MAX_PHY_REG		32

/*
 * Read the PHY reg
 */
int big_sur_ge_phy_read(big_sur_ge *emac, unsigned long addr, 
				unsigned long reg_num, unsigned int *data)
{
	unsigned long mii_control, mii_data;

	if (!emac->has_mii)
		return -1;

	mii_control = BIG_SUR_GE_READ(emac->base_address + BIG_SUR_GE_MGTCR_OFFSET);
	if (mii_control & BIG_SUR_GE_MGTCR_START_MASK) {
		printk(KERN_ERR "PHY busy \n");
		return -1;
	}

	mii_control = (addr << BIG_SUR_GE_MGTCR_PHY_ADDR_SHIFT);
	mii_control |= (reg_num << BIG_SUR_GE_MGTCR_REG_ADDR_SHIFT);
	mii_control |= (BIG_SUR_GE_MGTCR_RW_NOT_MASK | BIG_SUR_GE_MGTCR_START_MASK |
				BIG_SUR_GE_MGTCR_MII_ENABLE_MASK);

	BIG_SUR_GE_WRITE(emac->base_address + BIG_SUR_GE_MGTCR_OFFSET, mii_control);

	while (mii_control & BIG_SUR_GE_MGTCR_START_MASK) 
		if (!(mii_control & BIG_SUR_GE_MGTCR_START_MASK))
			break;

	mii_data = BIG_SUR_GE_READ(emac->base_address + BIG_SUR_GE_MGTDR_OFFSET);
	*data = (unsigned int) mii_data;

	return 0;
}

/*
 * Write to the PHY register
 */
int big_sur_ge_phy_write(big_sur_ge *emac, unsigned long addr,
				unsigned long reg_num, unsigned int data)
{
	unsigned long		mii_control;

	if (!emac->has_mii)
                return -1;

	mii_control = BIG_SUR_GE_READ(emac->base_address + BIG_SUR_GE_MGTCR_OFFSET);
	if (mii_control & BIG_SUR_GE_MGTCR_START_MASK) {
                printk(KERN_ERR "PHY busy \n");
                return -1;
        }

	BIG_SUR_GE_WRITE(emac->base_address + BIG_SUR_GE_MGTDR_OFFSET, (unsigned long)data);
	
	mii_control = (addr << BIG_SUR_GE_MGTCR_PHY_ADDR_SHIFT);
        mii_control |= (reg_num << BIG_SUR_GE_MGTCR_REG_ADDR_SHIFT);
        mii_control |= (BIG_SUR_GE_MGTCR_START_MASK | BIG_SUR_GE_MGTCR_MII_ENABLE_MASK);

	BIG_SUR_GE_WRITE(emac->base_address + BIG_SUR_GE_MGTCR_OFFSET, mii_control);

	while (mii_control & BIG_SUR_GE_MGTCR_START_MASK)
                if (!(mii_control & BIG_SUR_GE_MGTCR_START_MASK))
                        break;

	return 0;
}

/*
 * Reset the GE system
 */
static void big_sur_ge_reset(struct net_device *netdev, DUPLEX duplex)
{
	struct big_sur_ge_enet *lp = (struct big_sur_ge_enet *)netdev->priv;
	struct sk_buff *skb;
	unsigned long options;
	int ifcfg1, ifcfg2;

	/* Stop the queue */
	netif_stop_queue(netdev);

	big_sur_ge_get_frame_gap(lp->emac, &ifcfg1, &ifcfg2);
	options = big_sur_ge_get_options(lp->emac);
	switch (duplex) {
		case HALF_DUPLEX:
			options &= ~(BIG_SUR_GE_FDUPLEX_OPTION);
			break;

		case FULL_DUPLEX:
			options |= BIG_SUR_GE_FDUPLEX_OPTION;
			break;

		case UNKNOWN:
			break;
	}

	big_sur_ge_enet_reset(lp->emac);

	/* Set the necessary options for the MAC unit */
	big_sur_ge_set_mac_address(lp->emac, netdev->dev_addr);
	big_sur_ge_set_frame_gap(lp->emac, ifcfg1, ifcfg2);
	big_sur_ge_set_options(lp->emac, options);

	(void) big_sur_ge_start(lp->emac);

	spin_lock_irq(lp->lock);
	skb = lp->saved_skb;
	lp->saved_skb = NULL;
	spin_unlock_irq(lp->lock);

	if (skb) 
		dev_kfree_skb(skb);

	/* Start the queue, in case it was stopped */
	netif_wake_queue(netdev);
}

/*
 * Get the PHY status and then configure the 
 * speed, duplex, link status etc.
 */
static int big_sur_ge_get_phy_status(struct net_device *netdev, 
					DUPLEX *duplex, int *linkup)
{
	struct big_sur_ge_enet  *lp = netdev->priv;
	unsigned int		reg_data;
	int			err = 0;

	err = big_sur_ge_phy_read(lp->emac, lp->mii_addr, MII_BMCR, &reg_data);
	if (err == -1) {
		printk(KERN_ERR "%s: Could not read PHY control register", netdev->name);
		return err;
	}

	if (!(reg_data & BMCR_ANENABLE)) {
		if (reg_data & BMCR_FULLDPLX) 
			*duplex = FULL_DUPLEX;
		else
			*duplex = HALF_DUPLEX;
	}
	else {
		unsigned int advertise, partner, neg;
	
		err = big_sur_ge_phy_read(lp->emac, lp->mii_addr, MII_ADVERTISE, &advertise);
		if (err == -1) {
	                printk(KERN_ERR "%s: Could not read PHY control register", netdev->name);
        	        return err;
		}

		err = big_sur_ge_phy_read(lp->emac, lp->mii_addr, MII_LPA, &partner);
		if (err == -1) {
                        printk(KERN_ERR "%s: Could not read PHY control register", netdev->name);
                        return err;
                }

		neg = advertise & partner & ADVERTISE_ALL;
		if (neg & ADVERTISE_100FULL)
                        *duplex = FULL_DUPLEX;
                else if (neg & ADVERTISE_100HALF)
                        *duplex = HALF_DUPLEX;
                else if (neg & ADVERTISE_10FULL)
                        *duplex = FULL_DUPLEX;
                else
                        *duplex = HALF_DUPLEX;

		err = big_sur_ge_phy_read(lp->emac, lp->mii_addr, MII_BMSR, &reg_data);
		if (err == -1) {
                        printk(KERN_ERR "%s: Could not read PHY control register", netdev->name);
                        return err;
                }

		*linkup = (reg_data & BMSR_LSTATUS) != 0;

	}
	return 0;
}

/*
 * Poll the MII for duplex and link status 
 */
static void big_sur_ge_poll_mii(unsigned long data)
{
	struct net_device *netdev = (struct net_device *) data;	
	struct big_sur_ge_enet* lp = netdev->priv;
	unsigned long options;
	DUPLEX mac_duplex, phy_duplex;
	int phy_carrier, netif_carrier;

	if (big_sur_ge_get_phy_status(netdev, &phy_duplex, &phy_carrier) == -1) {
		printk(KERN_ERR "%s: Terminating link monitoring.\n", netdev->name);
		return;
	}

	options = big_sur_ge_get_options(lp->emac);
	if (options & BIG_SUR_GE_FDUPLEX_OPTION)
		mac_duplex = FULL_DUPLEX;
	else
		mac_duplex = HALF_DUPLEX;

	if (mac_duplex != phy_duplex) {
		tasklet_disable(&lp->big_sur_tasklet);
		big_sur_ge_reset(netdev, phy_duplex);
		tasklet_enable(&lp->big_sur_tasklet);
	}

	netif_carrier = netif_carrier_ok(netdev) != 0;

        if (phy_carrier != netif_carrier) {
                if (phy_carrier) {
                        printk(KERN_INFO "%s: Link carrier restored.\n",
                               netdev->name);
                        netif_carrier_on(netdev);
                } else {
                        printk(KERN_INFO "%s: Link carrier lost.\n", netdev->name);
                        netif_carrier_off(netdev);
                }
        }

        /* Set up the timer so we'll get called again in 2 seconds. */
        lp->phy_timer.expires = jiffies + 2 * HZ;
        add_timer(&lp->phy_timer);
}

/*
 * Open the network interface
 */
static int big_sur_ge_open(struct net_device *netdev)
{
	struct big_sur_ge_enet *lp = netdev->priv;
	unsigned long options;
	DUPLEX phy_duplex, mac_duplex;
	int phy_carrier;

	(void) big_sur_ge_stop(lp->emac);

	if (big_sur_ge_set_mac_address(lp->emac, netdev->dev_addr) == -1) {
		printk(KERN_ERR "%s: Could not set MAC address.\n", netdev->name);
		return -EIO;
	}

	options = big_sur_ge_get_options(lp->emac);

	/*
	 * This MAC unit has no support for Interrupts. So, we initialize
	 * a tasklet here that will be scheduled from the timer
	 * interrupt handler. Note that we cannot run the receive
	 * and the transmit functions as part of an interrupt handler. Since,
	 * this will be disastrous for the timer under load. Hence, tasklet
	 * is the best way to go
	 */
	tasklet_init(&lp->big_sur_tasklet, big_sur_ge_fifo_intr, (unsigned long)netdev);

	if (!(big_sur_ge_get_phy_status(netdev, &phy_duplex, &phy_carrier))) {
		if (options & BIG_SUR_GE_FDUPLEX_OPTION)
			mac_duplex = FULL_DUPLEX;
		else
			mac_duplex = HALF_DUPLEX;

		if (mac_duplex != phy_duplex) {
			switch (phy_duplex) {
				case HALF_DUPLEX: 
					options &= ~(BIG_SUR_GE_FDUPLEX_OPTION); 
					break;
				case FULL_DUPLEX:
					options |= BIG_SUR_GE_FDUPLEX_OPTION;
					break;
				case UNKNOWN:
					break;
			}

			big_sur_ge_set_options(lp->emac, options);
		}
	}

	if (big_sur_ge_start(lp->emac) == -1) {
		printk(KERN_ERR "%s: Could not start device.\n", netdev->name);
		tasklet_kill(&lp->big_sur_tasklet);
		return -EBUSY;
	}

	MOD_INC_USE_COUNT;
	netif_start_queue(netdev);

	lp->phy_timer.expires = jiffies + 2*HZ;
        lp->phy_timer.data = (unsigned long)netdev;
        lp->phy_timer.function = &big_sur_ge_poll_mii;
	add_timer(&lp->phy_timer);

	return 0;
}

/*
 * Close the network device interface
 */
static int big_sur_ge_close(struct net_device *netdev)
{
	struct big_sur_ge_enet  *lp = netdev->priv;
	
	del_timer_sync(&lp->phy_timer);
	netif_stop_queue(netdev);

	if (big_sur_ge_stop(lp->emac) == -1) {
		printk(KERN_ERR "%s: Could not stop device.\n", netdev->name);
		return -EBUSY;
        }

	tasklet_kill(&lp->big_sur_tasklet);

        MOD_DEC_USE_COUNT;
        return 0;
}

/*	
 * Get the network device stats. 
 */
static struct net_device_stats *big_sur_ge_get_stats(struct net_device *netdev)
{
	struct big_sur_ge_enet *lp = netdev->priv;
	
	return lp->stats;
}

/*
 * FIFO send for a packet that needs to be transmitted
 */
static int big_sur_start_xmit(struct sk_buff *orig_skb, struct net_device *netdev)
{
	struct big_sur_ge_enet *lp = netdev->priv;
	struct sk_buff *new_skb;
	unsigned int len, align;
	struct net_device_stats *stats = lp->stats;

	/* 
	 * The FIFO takes a single request at a time. Stop the queue to
	 * accomplish this.  We'll wake the queue in the transmit
	 * routine below or in the timeout routine
	 */
	netif_stop_queue(netdev);
        len = orig_skb->len;

	/*
	 * Align the packet for the FIFO
	 */
	if (!(new_skb = dev_alloc_skb(len + 4))) {
                dev_kfree_skb(orig_skb);
                printk(KERN_ERR "%s: Could not allocate transmit buffer.\n",
                       netdev->name);
                netif_wake_queue(netdev);
                return -EBUSY;
        }

	align = 4 - ((unsigned long) new_skb->data & 3);
        if (align != 4)
                skb_reserve(new_skb, align);

	skb_put(new_skb, len);
        memcpy(new_skb->data, orig_skb->data, len);

	dev_kfree_skb(orig_skb);
	lp->saved_skb = new_skb;

	/* Do the actual transmit */
	if (big_sur_tx(lp->emac, (u8 *) new_skb->data, len) == -1) {
		spin_lock_irq(&lp->lock);
                new_skb = lp->saved_skb;
                lp->saved_skb = NULL;
                spin_unlock_irq(&lp->lock);

                dev_kfree_skb(new_skb);
                printk(KERN_ERR "%s: Could not transmit buffer.\n", netdev->name);
                netif_wake_queue(netdev);
                return -EIO;
        }

	stats->tx_bytes += len;
	stats->tx_packets++;

        return 0;
}

/*
 * Free the skb
 */
static void big_sur_tx_free_skb(struct net_device *netdev)
{
	struct big_sur_ge_enet *lp = netdev->priv;
	struct sk_buff *skb;

	spin_lock_irq(&lp->lock);
	skb = lp->saved_skb;
        lp->saved_skb = NULL;
        spin_unlock_irq(&lp->lock);
	
	if (skb)
		dev_kfree_skb(skb);

	/* Start the queue since we know that packet has been transmitted */
        netif_wake_queue(netdev);
}

/*
 * Handle the timeout of the ethernet device 
 */
static void big_sur_ge_tx_timeout(struct net_device *netdev)
{
	struct big_sur_ge_enet *lp = (struct big_sur_ge_enet *)netdev;

	printk("%s: Exceeded transmit timeout of %lu ms.  Resetting mac.\n",
			netdev->name, TX_TIMEOUT * 1000UL / HZ);

	tasklet_disable(&lp->big_sur_tasklet);
	big_sur_ge_reset(netdev, UNKNOWN);
	tasklet_enable(&lp->big_sur_tasklet);
}

/*
 * Receive the packets
 */
static void big_sur_receive(struct net_device *netdev)
{
	struct big_sur_ge_enet *lp = (struct big_sur_ge_enet *)netdev->priv;
	struct sk_buff *skb;
	unsigned long len = BIG_SUR_GE_MAX_FRAME_SIZE;
	unsigned int align;
	struct net_device_stats *stats = lp->stats;
	
	if (!(skb = dev_alloc_skb(len + 4))) {
		printk(KERN_ERR "%s: Could not allocate receive buffer.\n",
			netdev->name);
		return;
	}

	align = 4 - ((unsigned long) skb->data & 3);
        if (align != 4)
                skb_reserve(skb, align);

	if (big_sur_rx(lp->emac, (u8 *)skb->data, &len) == -1) {
		dev_kfree_skb(skb);
		
		printk(KERN_ERR "%s: Could not receive buffer \n", netdev->name);
		netdev->tx_timeout = NULL;
		big_sur_ge_reset(netdev, UNKNOWN);
		netdev->tx_timeout = big_sur_ge_tx_timeout;
	}

	skb_put(skb, len);
        skb->dev = netdev;
        skb->protocol = eth_type_trans(skb, netdev);

	stats->rx_packets++;
	stats->rx_bytes += len;

	/* Good Rx, send the packet upstream. */
	netif_rx(skb);
}

/*
 * Set the Multicast Hash list
 */
static void big_sur_ge_set_multi(struct net_device *netdev)
{
	struct big_sur_ge_enet *lp = (struct big_sur_ge_enet *)netdev->priv;
	unsigned long options;

	tasklet_disable(&lp->big_sur_tasklet);

	(void) big_sur_ge_stop(lp->emac);
	options = big_sur_ge_get_options(lp->emac);
	options &= ~(BIG_SUR_GE_PROMISC_OPTION | BIG_SUR_GE_MULTICAST_OPTION);

	if (netdev->flags & IFF_PROMISC) 
		options |= BIG_SUR_GE_PROMISC_OPTION;
	
	(void) big_sur_ge_start(lp->emac);

	tasklet_enable(&lp->big_sur_tasklet);
}	

/*
 * IOCTL support
 */
static int big_sur_ge_ioctl(struct net_device *netdev, struct ifreq *rq, int cmd)
{
	struct big_sur_ge_enet *lp = netdev->priv;
	struct mii_ioctl_data *data = (struct mii_ioctl_data *) &rq->ifr_data;

	switch(cmd) {
		case SIOCGMIIPHY:       	/* Get address of MII PHY in use. */
	        case SIOCDEVPRIVATE:    	/* for binary compat, remove in 2.5 */
                	data->phy_id = lp->mii_addr;

        	case SIOCGMIIREG:   	    	/* Read MII PHY register. */
	        case SIOCDEVPRIVATE + 1: 	/* for binary compat, remove in 2.5 */
                	if (data->phy_id > 31 || data->reg_num > 31)
                        	return -ENXIO;

                	del_timer_sync(&lp->phy_timer);

			if (big_sur_ge_phy_read(lp->emac, data->phy_id, 
						data->reg_num, &data->val_out) == -1) {
				printk(KERN_ERR "%s: Could not read from PHY", netdev->name);
				return -EBUSY;
			}

			lp->phy_timer.expires = jiffies + 2*HZ;
	                add_timer(&lp->phy_timer);
				
			return 0;

		case SIOCSMIIREG:       	/* Write MII PHY register. */
	        case SIOCDEVPRIVATE + 2:        /* for binary compat, remove in 2.5 */
			if (data->phy_id > 31 || data->reg_num > 31)
				return -ENXIO;
	
			del_timer_sync(&lp->phy_timer);

			if (big_sur_ge_phy_write(lp->emac, data->phy_id, data->reg_num,
							data->val_in) == -1) {
				printk(KERN_ERR "%s: Could not write to PHY", netdev->name);
				return -EBUSY;
			}

			lp->phy_timer.expires = jiffies + 2*HZ;
			add_timer(&lp->phy_timer);

			return 0;

		default:
			return -EOPNOTSUPP;
	}
}

/*
 * Get the config from the config table 
 */
big_sur_ge_config *big_sur_ge_get_config(int index)
{	
	/* For port 0 only */
	big_sur_ge_config *config;

	config->device_id = 0;
	config->base_address = BIG_SUR_GE_BASE; /* Base Address of the MAC */
	config->has_counters = 0;
	config->has_sg_dma = 0;
	config->dma_config = 0;
	config->has_mii = 1;

	return (big_sur_ge_config *)config;
}

/*
 * Release the network device structure
 */
static void big_sur_ge_remove_dev(struct net_device *netdev)
{
	struct big_sur_ge_enet  *lp = (struct big_sur_ge_enet *)netdev;
	big_sur_ge_config	*config;

	config = big_sur_ge_get_config(lp->index);
	config->base_address = lp->save_base_address;
	
	if (lp->saved_skb)
                dev_kfree_skb(lp->saved_skb);
        kfree(lp);

        unregister_netdev(netdev);
        kfree(netdev);
}

/*
 * Initial Function to probe the network interface 
 */
static int __init big_sur_ge_probe(int index)
{
	struct net_device *netdev;
	struct big_sur_ge_enet *lp;
	big_sur_ge_config *config;
	unsigned long maddr;

	config = big_sur_ge_get_config(index);
	if (!config)
		return -ENODEV;

	netdev = alloc_etherdev(sizeof(struct big_sur_ge_enet));

	if (!netdev) {
		printk(KERN_ERR "Could not allocate Big Sur Ethernet device %d.\n",index);
		return -ENOMEM;
	}

	SET_MODULE_OWNER(netdev);

	lp = (struct big_sur_ge_enet *)netdev->priv;
	memset(lp, 0, sizeof(struct big_sur_ge_enet));
	spin_lock_init(&lp->lock);
	
	/* Use KSEG1 address */
	lp->save_base_address = config->base_address;
	
	if (big_sur_ge_enet_init(lp->emac, config->device_id) == -1) {
		printk(KERN_ERR "%s: Could not initialize device.\n", netdev->name);
		big_sur_ge_remove_dev(netdev);
		return -ENODEV;
	}

	memcpy(netdev->dev_addr, big_sur_mac_addr_base, 6);
	if (big_sur_ge_set_mac_address(lp->emac, netdev->dev_addr) == -1) {
		printk(KERN_ERR "%s: Could not set MAC address.\n", netdev->name);
		big_sur_ge_remove_dev(netdev);
                return -EIO;
        }

	/* Check the PHY */	
	lp->mii_addr = 0xff;
	for (maddr = 0; maddr < 31; maddr++) {
		unsigned int	reg_data;

		if (big_sur_ge_phy_read(lp->emac, maddr, MII_BMCR, &reg_data) == 0) {
			lp->mii_addr = maddr;
                        break;
                }
	}

	if (lp->mii_addr == 0xff) {
                lp->mii_addr = 0;
                printk(KERN_WARNING
                       "%s: No PHY detected.  Assuming a PHY at address %d.\n",
					netdev->name, lp->mii_addr);
	}

	netdev->open = big_sur_ge_open;
	netdev->stop = big_sur_ge_close;
	netdev->get_stats = big_sur_ge_get_stats; 
	netdev->do_ioctl = big_sur_ge_ioctl;
	netdev->tx_timeout = big_sur_ge_tx_timeout;
	netdev->watchdog_timeo = TX_TIMEOUT;
	netdev->hard_start_xmit = big_sur_start_xmit;
	netdev->set_multicast_list = big_sur_ge_set_multi;

	printk(KERN_INFO
               "%s: PMC-Sierra Big Sur Ethernet Device %d  at 0x%08X mapped to 0x%08X\n",
		netdev->name, index,
               lp->save_base_address, config->base_address);

        return 0;
}
	
static int __init big_sur_ge_init(void)
{
	int	index = 0;

	while (big_sur_ge_probe(index++) == 0);

	return (index > 1) ? 0 : -ENODEV;
}

static void __init big_sur_ge_cleanup_module(void)
{
	/* Nothing to do here */
}

module_init(big_sur_ge_init);
module_exit(big_sur_ge_cleanup_module);


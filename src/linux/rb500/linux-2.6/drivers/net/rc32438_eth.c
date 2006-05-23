/**************************************************************************
 *
 *  BRIEF MODULE DESCRIPTION
 *     Driver for the IDT RC32438 on-chip ethernet controller.
 *
 *  Copyright 2004 IDT Inc. (rischelp@idt.com)
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
 *
 **************************************************************************
 * May 2004 P. Sadik
 *
 * Based on the driver developed by B. Maruthanayakam, H. Kou and others.
 *
 * Aug 2004 P. Sadik
 * Added NAPI support.
 *
 **************************************************************************
 */
#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/timer.h>
#include <linux/errno.h>
#include <linux/proc_fs.h>
#include <linux/in.h>
#include <linux/ioport.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/ethtool.h>
#include <linux/skbuff.h>
#include <linux/delay.h>
#include <linux/ctype.h>
#include <linux/mii.h>

#include <asm/irq.h>
#include <asm/bitops.h>
#include <asm/io.h>
#include <asm/uaccess.h>

#include "rc32438_eth.h"
#define DRIVER_VERSION "(July04)"

#define DRIVER_NAME "rc32438 Ethernet driver. " DRIVER_VERSION


#define STATION_ADDRESS_HIGH(dev) (((dev)->dev_addr[0] << 8) | \
			           ((dev)->dev_addr[1]))
#define STATION_ADDRESS_LOW(dev)  (((dev)->dev_addr[2] << 24) | \
				   ((dev)->dev_addr[3] << 16) | \
				   ((dev)->dev_addr[4] << 8)  | \
				   ((dev)->dev_addr[5]))

#define MII_CLOCK 1250000 				/* no more than 2.5MHz */
static char mac0[18] = "08:00:06:05:40:01";
static char mac1[18] = "08:00:06:05:50:01";

MODULE_PARM(mac0, "c18");
MODULE_PARM(mac1, "c18");
MODULE_PARM_DESC(mac0, "MAC address for RC32438 ethernet0");
MODULE_PARM_DESC(mac1, "MAC address for RC32438 ethernet1");

static struct rc32438_if_t {
	char *name;
	struct net_device *dev;
	int weight;
	char* mac_str;
	u32 iobase;
	u32 rxdmabase;
	u32 txdmabase;
	int rx_dma_irq;
	int tx_dma_irq;
	int rx_ovr_irq;
	int tx_und_irq;
} rc32438_iflist[] =
{
	{
		"rc32438_eth0",
		NULL,
		300,
		mac0,
		ETH0_PhysicalAddress,
		ETH0_RX_DMA_ADDR,
		ETH0_TX_DMA_ADDR,
		ETH0_DMA_RX_IRQ,
		ETH0_DMA_TX_IRQ,
		ETH0_RX_OVR_IRQ,
		ETH0_TX_UND_IRQ
	},
	{
		"rc32438_eth1",
		NULL,
		300,
		mac1,
		ETH1_PhysicalAddress,
		ETH1_RX_DMA_ADDR,
		ETH1_TX_DMA_ADDR,
		ETH1_DMA_RX_IRQ,
		ETH1_DMA_TX_IRQ,
		ETH1_RX_OVR_IRQ,
		ETH1_TX_UND_IRQ
	}
};


static int parse_mac_addr(struct net_device *dev, char* macstr)
{
	int i, j;
	unsigned char result, value;

	for (i=0; i<6; i++)
	{
		result = 0;
		if (i != 5 && *(macstr+2) != ':')
		{
			ERR("invalid mac address format: %d %c\n",
			    i, *(macstr+2));
			return -EINVAL;
		}
		for (j=0; j<2; j++)
		{
			if (isxdigit(*macstr) && (value = isdigit(*macstr) ? *macstr-'0' :
						  toupper(*macstr)-'A'+10) < 16)
			{
				result = result*16 + value;
				macstr++;
			}
			else
			{
				ERR("invalid mac address "
				    "character: %c\n", *macstr);
				return -EINVAL;
			}
		}

		macstr++;
		dev->dev_addr[i] = result;
	}

	return 0;
}

static inline void rc32438_abort_tx(struct net_device *dev)
{
	struct rc32438_local *lp = netdev_priv(dev);

	rc32438_abort_dma(dev, lp->tx_dma_regs);

}

static inline void rc32438_abort_rx(struct net_device *dev)
{
	struct rc32438_local *lp = netdev_priv(dev);

	rc32438_abort_dma(dev, lp->rx_dma_regs);

}

static inline void rc32438_start_tx(struct rc32438_local *lp,  volatile DMAD_t td)
{
	rc32438_start_dma(lp->tx_dma_regs, CPHYSADDR(td));
}

static inline void rc32438_start_rx(struct rc32438_local *lp, volatile DMAD_t rd)
{
	rc32438_start_dma(lp->rx_dma_regs, CPHYSADDR(rd));
}

static inline void rc32438_chain_tx(struct rc32438_local *lp, volatile DMAD_t td)
{
	rc32438_chain_dma(lp->tx_dma_regs, CPHYSADDR(td));
}
static inline void rc32438_chain_rx(struct rc32438_local *lp, volatile DMAD_t rd)
{
	rc32438_chain_dma(lp->rx_dma_regs, CPHYSADDR(rd));
}

#ifdef RC32438_PROC_DEBUG
static int rc32438_read_proc(char *buf, char **start, off_t fpos,
			     int length, int *eof, void *data)
{
	struct net_device *dev = (struct net_device *)data;

	struct rc32438_local *lp = netdev_priv(dev);

	int len = 0;

	/* print out header */
	len += sprintf(buf + len, "\n\tRC32438 Ethernet Debug\n\n");

 	len += sprintf (buf + len,
			"DMA halt count  = %10d, DMA ovr count  = %10d\n",
			lp->dma_halt_cnt,lp->dma_ovr_count);


        len += sprintf (buf + len,
			"note_done_cnt   = %10d, DMA halt_nd_count = %10d\n",
			lp->not_done_cnt,lp->dma_halt_nd_cnt);

        len += sprintf (buf + len,
			"budget          = %10d, quota             = %10d\n",
			lp->ibudget,lp->iquota);

        len += sprintf (buf + len,
			"tx_stopped          = %10d, quota             = %10d\n",
			lp->tx_stopped,lp->iquota);

	if (fpos >= len)
	{
		*start = buf;
		*eof = 1;
		return 0;
	}
	*start = buf + fpos;

	if ((len -= fpos) > length)
		return length;
	*eof = 1;

	return len;

}
#endif

/*
 * Restart the RC32438 ethernet controller.
 */
static int rc32438_restart(struct net_device *dev)
{
	struct rc32438_local *lp = netdev_priv(dev);

	/*
	 * Disable interrupts
	 */
	disable_irq(lp->rx_irq);
	disable_irq(lp->tx_irq);

	/* Mask F E bit in Tx DMA */
	rc32438_writel(rc32438_readl(&lp->tx_dma_regs->dmasm) | DMASM_f_m | DMASM_e_m, &lp->tx_dma_regs->dmasm);
	/* Mask D H E bit in Rx DMA */
	rc32438_writel(rc32438_readl(&lp->rx_dma_regs->dmasm) | DMASM_d_m | DMASM_h_m | DMASM_e_m, &lp->rx_dma_regs->dmasm);

	rc32438_init(dev);
	rc32438_multicast_list(dev);

	enable_irq(lp->tx_irq);
	enable_irq(lp->rx_irq);

	return 0;
}

int rc32438_init_module(void)
{
	int retval;

	printk(KERN_INFO DRIVER_NAME " \n");

	retval  = rc32438_probe(0);
	retval |= rc32438_probe(1);

	return retval;
}

static int rc32438_probe(int port_num)
{
	struct rc32438_if_t *bif = &rc32438_iflist[port_num];
	struct rc32438_local *lp = NULL;
	struct net_device *dev = NULL;
	int i, retval,err;

	dev = alloc_etherdev(sizeof(struct rc32438_local));
	if(!dev)
	{
		ERR("rc32438_eth: alloc_etherdev failed\n");
		return -1;
	}

	SET_MODULE_OWNER(dev);

	bif->dev = dev;

	if ((retval = parse_mac_addr(dev, bif->mac_str)))
	{
		ERR("MAC address parse failed\n");
		free_netdev(dev);
		return -1;
	}

	/* Initialize the device structure. */
	if (dev->priv == NULL)
	{
		lp = (struct rc32438_local *)kmalloc(sizeof(*lp), GFP_KERNEL);
		memset(lp, 0, sizeof(struct rc32438_local));
	}
	else
	{
		lp = (struct rc32438_local *)dev->priv;
	}

	lp->rx_irq = bif->rx_dma_irq;
	lp->tx_irq = bif->tx_dma_irq;

	lp->weight = bif->weight;

	lp->eth_regs = ioremap_nocache(bif->iobase, sizeof(*lp->eth_regs));

	if (!lp->eth_regs)
	{
		ERR("Can't remap eth registers\n");
		retval = -ENXIO;
		goto probe_err_out;
	}

	lp->rx_dma_regs = ioremap_nocache(bif->rxdmabase, sizeof(struct DMA_Chan_s));

	if (!lp->rx_dma_regs)
	{
		ERR("Can't remap Rx DMA registers\n");
		retval = -ENXIO;
		goto probe_err_out;
	}
	lp->tx_dma_regs = ioremap_nocache(bif->txdmabase,sizeof(struct DMA_Chan_s));

	if (!lp->tx_dma_regs)
	{
		ERR("Can't remap Tx DMA registers\n");
		retval = -ENXIO;
		goto probe_err_out;
	}

#ifdef RC32438_PROC_DEBUG
	lp->ps = create_proc_read_entry (bif->name, 0, proc_net,
					 rc32438_read_proc, dev);
#endif

	lp->td_ring =	(DMAD_t)kmalloc(TD_RING_SIZE + RD_RING_SIZE, GFP_KERNEL);
	if (!lp->td_ring)
	{
		ERR("Can't allocate descriptors\n");
		retval = -ENOMEM;
		goto probe_err_out;
	}

	dma_cache_inv((unsigned long)(lp->td_ring), TD_RING_SIZE + RD_RING_SIZE);

	/* now convert TD_RING pointer to KSEG1 */
	lp->td_ring = (DMAD_t )KSEG1ADDR(lp->td_ring);
	lp->rd_ring = &lp->td_ring[RC32438_NUM_TDS];

#ifdef CONFIG_SMP
	spin_lock_init(&lp->lock);
#endif
	dev->base_addr = bif->iobase;
	/* just use the rx dma irq */
	dev->irq = bif->rx_dma_irq;

	dev->priv = lp;

	dev->open = rc32438_open;
	dev->stop = rc32438_close;
	dev->hard_start_xmit = rc32438_send_packet;
	dev->get_stats	= rc32438_get_stats;
	dev->set_multicast_list = &rc32438_multicast_list;
	dev->tx_timeout = rc32438_tx_timeout;
	dev->watchdog_timeo = RC32438_TX_TIMEOUT;

	dev->poll = rc32438_poll;
	dev->weight = lp->weight;

	lp->tx_tasklet = kmalloc(sizeof(struct tasklet_struct), GFP_KERNEL);
	tasklet_init(lp->tx_tasklet, rc32438_tx_tasklet, (unsigned long)dev);

#ifdef CONFIG_RC32438_REVISION_ZA
	lp->ovr_und_tasklet = kmalloc(sizeof(struct tasklet_struct), GFP_KERNEL);
	tasklet_init(lp->ovr_und_tasklet, rc32438_ovr_und_tasklet, (unsigned long)dev);
#endif

	if ((err = register_netdev(dev))) {
		printk(KERN_ERR "rc32438 ethernet. Cannot register net device %d\n", err);
		free_netdev(dev);
		retval = -EINVAL;
		goto probe_err_out;
	}

	INFO("Rx IRQ %d, Tx IRQ %d, TDS %d RDS %d weight %d ",
	     bif->rx_dma_irq, bif->tx_dma_irq,RC32438_NUM_TDS, RC32438_NUM_RDS,dev->weight);
	for (i = 0; i < 6; i++)
	{
		printk("%2.2x", dev->dev_addr[i]);
		if (i<5)
			printk(":");
	}
	printk("\n");

	return 0;

 probe_err_out:
	rc32438_cleanup_module();
	ERR(" failed.  Returns %d\n", retval);
	return retval;

}


static void rc32438_cleanup_module(void)
{
	int i;

	for (i = 0; rc32438_iflist[i].iobase; i++)
	{
		struct rc32438_if_t * bif = &rc32438_iflist[i];
		if (bif->dev != NULL)
		{
			struct rc32438_local *lp = (struct rc32438_local *)bif->dev->priv;
			if (lp != NULL)
			{
				if (lp->eth_regs)
					iounmap((void*)lp->eth_regs);
				if (lp->rx_dma_regs)
					iounmap((void*)lp->rx_dma_regs);
				if (lp->tx_dma_regs)
					iounmap((void*)lp->tx_dma_regs);
				if (lp->td_ring)
					kfree((void*)KSEG0ADDR(lp->td_ring));

#ifdef RC32438_PROC_DEBUG
				if (lp->ps)
				{
					remove_proc_entry(bif->name, proc_net);
				}
#endif
				kfree(lp);
			}

			unregister_netdev(bif->dev);
			free_netdev(bif->dev);
			kfree(bif->dev);
		}
	}
}

/*
 * Open/initialize the RC32438 controller.
 *
 * This routine should set everything up anew at each open, even
 *  registers that "should" only need to be set once at boot, so that
 *  there is non-reboot way to recover if something goes wrong.
 */

static int rc32438_open(struct net_device *dev)
{
	struct rc32438_local *lp = netdev_priv(dev);

	/* Initialize */
	if (rc32438_init(dev))
	{
		ERR("Erroe: cannot open the Ethernet device\n");
		return -EAGAIN;
	}

	/* Install the interrupt handler that handles the Done Finished Ovr and Und Events */
	if (request_irq(lp->rx_irq, &rc32438_rx_dma_interrupt,
			SA_INTERRUPT,
			"rc32438 ethernet Rx", dev))
	{
		ERR(": unable to get Rx DMA IRQ %d\n",
		    lp->rx_irq);
		return -EAGAIN;
	}
	if (request_irq(lp->tx_irq, &rc32438_tx_dma_interrupt,
			SA_INTERRUPT,
			"rc32438 ethernet Tx", dev))
	{
		ERR(": unable to get Tx DMA IRQ %d\n",
		    lp->tx_irq);
		free_irq(lp->rx_irq, dev);
		return -EAGAIN;
	}

	/*Start MII-PHY Timer*/
	//Not enabled this feature at this time.
	/*
	  init_timer(&lp->mii_phy_timer);
	  lp->mii_phy_timer.expires = jiffies + 10 * HZ;
	  lp->mii_phy_timer.data = (unsigned long)dev;
	  lp->mii_phy_timer.function	 = rc32438_mii_handler;
	  add_timer(&lp->mii_phy_timer);
	*/

#ifdef RC32438_PROC_DEBUG
	lp->dma_halt_cnt = 0;
      	lp->dma_halt_nd_cnt = 0;
	lp->not_done_cnt = 0;
        lp->ibudget = 0;
        lp->iquota  = 0;
        lp->done_cnt    =0;
        lp->firstTime = 0;
	lp->tx_stopped = 0;
#endif
	return 0;
}

/*
 * Close the RC32438 device
 */
static int rc32438_close(struct net_device *dev)
{
	struct rc32438_local *lp = netdev_priv(dev);
	u32 tmp;

	/* Disable interrupts */
	disable_irq(lp->rx_irq);
	disable_irq(lp->tx_irq);

	tmp = rc32438_readl(&lp->tx_dma_regs->dmasm);
	tmp = tmp | DMASM_f_m | DMASM_e_m;
	rc32438_writel(tmp, &lp->tx_dma_regs->dmasm);

	tmp = rc32438_readl(&lp->rx_dma_regs->dmasm);
	tmp = tmp | DMASM_d_m | DMASM_h_m | DMASM_e_m;
	rc32438_writel(tmp, &lp->rx_dma_regs->dmasm);

	free_irq(lp->rx_irq, dev);
	free_irq(lp->tx_irq, dev);

	//Not enabled this feature at this time.
	//del_timer(&lp->mii_phy_timer);

	return 0;
}


/* transmit packet */
static int rc32438_send_packet(struct sk_buff *skb, struct net_device *dev)
{
	struct rc32438_local *lp = netdev_priv(dev);
	unsigned long 			flags;
	u32				length;
	volatile DMAD_t				td;

#ifdef CONFIG_SMP
	spin_lock_irqsave(&lp->lock, flags);
#else
	local_irq_save(flags);
#endif
	td = &lp->td_ring[lp->tx_chain_tail];

	/* stop queue when full, drop pkts if queue already full */
	if(lp->tx_count >= (RC32438_NUM_TDS - 2))
	{
		lp->tx_full = 1;

		if(lp->tx_count == (RC32438_NUM_TDS - 2))
		{
			/* this pkt is about to fill the queue*/
			lp->tx_stopped++;
			netif_stop_queue(dev);
		}
		else
		{
			/* this pkt cannot be added to the full queue */
			printk("Tx ring full, packet dropped\n");
			lp->stats.tx_dropped++;
			dev_kfree_skb_any(skb);
#ifdef CONFIG_SMP
			spin_unlock_irqrestore(&lp->lock, flags);
#else
			local_irq_restore(flags);
#endif
			return 1;
		}
	}

	lp->tx_count ++;

	lp->tx_skb[lp->tx_chain_tail] = skb;

	length = skb->len;

	/* Setup the transmit descriptor. */
	td->ca = CPHYSADDR(skb->data);

	if(rc32438_readl(&(lp->tx_dma_regs->dmandptr)) == 0)
	{
		if( lp->tx_chain_status == empty )
		{
			td->control = DMA_COUNT(length) |DMAD_cof_m |DMAD_iof_m;                                /*  Update tail      */
			lp->tx_chain_tail = (lp->tx_chain_tail + 1) & RC32438_TDS_MASK;                          /*   Move tail       */
			rc32438_writel(CPHYSADDR(&lp->td_ring[lp->tx_chain_head]), &(lp->tx_dma_regs->dmandptr)); /* Write to NDPTR    */
			lp->tx_chain_head = lp->tx_chain_tail;                                                  /* Move head to tail */
		}
		else
		{
			td->control = DMA_COUNT(length) |DMAD_cof_m|DMAD_iof_m;                                 /* Update tail */
			lp->td_ring[(lp->tx_chain_tail-1)& RC32438_TDS_MASK].control &=  ~(DMAD_cof_m);          /* Link to prev */
			lp->td_ring[(lp->tx_chain_tail-1)& RC32438_TDS_MASK].link =  CPHYSADDR(td);              /* Link to prev */
			lp->tx_chain_tail = (lp->tx_chain_tail + 1) & RC32438_TDS_MASK;                          /* Move tail */
			rc32438_writel(CPHYSADDR(&lp->td_ring[lp->tx_chain_head]), &(lp->tx_dma_regs->dmandptr)); /* Write to NDPTR */
			lp->tx_chain_head = lp->tx_chain_tail;                                                  /* Move head to tail */
			lp->tx_chain_status = empty;
		}
	}
	else
	{
		if( lp->tx_chain_status == empty )
		{
			td->control = DMA_COUNT(length) |DMAD_cof_m |DMAD_iof_m;                                /* Update tail */
			lp->tx_chain_tail = (lp->tx_chain_tail + 1) & RC32438_TDS_MASK;                          /* Move tail */
			lp->tx_chain_status = filled;
		}
		else
		{
			td->control = DMA_COUNT(length) |DMAD_cof_m |DMAD_iof_m;                                /* Update tail */
			lp->td_ring[(lp->tx_chain_tail-1)& RC32438_TDS_MASK].control &=  ~(DMAD_cof_m);          /* Link to prev */
			lp->td_ring[(lp->tx_chain_tail-1)& RC32438_TDS_MASK].link =  CPHYSADDR(td);              /* Link to prev */
			lp->tx_chain_tail = (lp->tx_chain_tail + 1) & RC32438_TDS_MASK;                          /* Move tail */
		}
	}

	dev->trans_start = jiffies;

#ifdef CONFIG_SMP
	spin_unlock_irqrestore(&lp->lock, flags);
#else
	local_irq_restore(flags);
#endif

	return 0;
}


/* Ethernet MII-PHY Handler */
static void rc32438_mii_handler(unsigned long data)
{
	struct net_device *dev = (struct net_device *)data;
	struct rc32438_local *lp = netdev_priv(dev);
	unsigned long 	flags;
	unsigned long duplex_status;
	int port_addr = (lp->rx_irq == 0x2c? 1:0) << 8;

#ifdef CONFIG_SMP
	spin_lock_irqsave(&lp->lock, flags);
#else
	local_irq_save(flags);
#endif

	/* Two ports are using the same MII, the difference is the PHY address */
	rc32438_writel(0, &rc32438_eth0_regs->miimcfg);
	rc32438_writel(0, &rc32438_eth0_regs->miimcmd);
	rc32438_writel(port_addr |0x05, &rc32438_eth0_regs->miimaddr);
	rc32438_writel(MIIMCMD_scn_m, &rc32438_eth0_regs->miimcmd);
	while(rc32438_readl(&rc32438_eth0_regs->miimind) & MIIMIND_nv_m);

	ERR("irq:%x		port_addr:%x	RDD:%x\n",
	    lp->rx_irq, port_addr, rc32438_readl(&rc32438_eth0_regs->miimrdd));
	duplex_status = (rc32438_readl(&rc32438_eth0_regs->miimrdd) & 0x140)? ETHMAC2_fd_m: 0;
	if(duplex_status != lp->duplex_mode)
	{
		ERR("The MII-PHY is Auto-negotiated to %s-Duplex mode for Eth-%x\n", duplex_status? "Full":"Half", lp->rx_irq == 0x2c? 1:0);
		lp->duplex_mode = duplex_status;
		rc32438_restart(dev);
	}

	lp->mii_phy_timer.expires = jiffies + 10 * HZ;
	add_timer(&lp->mii_phy_timer);

#ifdef CONFIG_SMP
	spin_unlock_irqrestore(&lp->lock, flags);
#else
	local_irq_restore(flags);
#endif

}

#ifdef CONFIG_RC32438_REVISION_ZA
static void rc32438_ovr_und_tasklet(unsigned long dev_id)
{
	struct net_device *dev = (struct net_device *)dev_id;
	struct rc32438_local *lp = netdev_priv(dev);

	unsigned int status;
	unsigned long flags;

	ASSERT(dev != NULL);
#ifdef CONFIG_SMP
	spin_lock_irqsave(&lp->lock,flags);
#else
	local_irq_save(flags);
#endif
	status = rc32438_readl(&lp->eth_regs->ethintfc);

	lp->dma_ovr_count++;
	if(status & (ETHINTFC_und_m | ETHINTFC_ovr_m) )
	{
		netif_stop_queue(dev);

		/* clear OVR bit */
		rc32438_writel((status & ~(ETHINTFC_und_m | ETHINTFC_ovr_m)), &lp->eth_regs->ethintfc);

		/* Restart interface */
		rc32438_restart(dev);
	}
#ifdef CONFIG_SMP
	spin_unlock_irqrestore(&lp->lock,flags);
#else
	local_irq_restore(flags);
#endif
}
#endif
/* Ethernet Rx DMA interrupt */
static irqreturn_t
rc32438_rx_dma_interrupt(int irq, void *dev_id, struct pt_regs * regs)
{
	struct net_device *dev = (struct net_device *)dev_id;
	struct rc32438_local *lp = netdev_priv(dev);
	volatile u32 dmas,dmasm;
	irqreturn_t retval = IRQ_NONE;

	ASSERT(dev != NULL);

#ifdef CONFIG_SMP
	spin_lock(&lp->lock);
#endif
	dmas = rc32438_readl(&lp->rx_dma_regs->dmas);
	if(dmas & (DMAS_d_m|DMAS_h_m|DMAS_e_m))
	{
		/* Mask D H E bit in Rx DMA */
		dmasm = rc32438_readl(&lp->rx_dma_regs->dmasm);
		rc32438_writel(dmasm | (DMASM_d_m | DMASM_h_m | DMASM_e_m), &lp->rx_dma_regs->dmasm);

		if(netif_rx_schedule_prep(dev))
			__netif_rx_schedule(dev);

		if (dmas & DMAS_e_m)
			ERR(": DMA error\n");

		retval = IRQ_HANDLED;
	}
#ifdef CONFIG_SMP
	spin_unlock(&lp->lock);
#endif
	return retval;
}

static int rc32438_poll(struct net_device *dev, int *budget)
{
	struct rc32438_local* lp = netdev_priv(dev);
	volatile DMAD_t  rd;
	u32 rx_next_done;
	struct sk_buff *skb, *skb_new;
	u8* pkt_buf;
	u32 count, pkt_len, pktuncrc_len;
	volatile u32 dmas,devcs;
#ifdef CONFIG_RC32438_REVISION_ZA
	volatile u32 ovr_und;
#endif
	u32 received = 0;
	int rx_work_limit = 0;

	rx_next_done  = lp->rx_next_done;
	rd = &lp->rd_ring[rx_next_done];

	rx_work_limit = min(*budget,dev->quota);

        while ( (count = RC32438_RBSIZE - (u32)DMA_COUNT(rd->control)) != 0)
        {
                if(--rx_work_limit <0)
                {
#ifdef RC32438_PROC_DEBUG
                        if(lp->firstTime == 0)
                        {
                                lp->ibudget = *budget;
                                lp->iquota  = dev->quota;
                                lp->firstTime = 1;
                        }
#endif
			break;
                }
                /* init the var. used for the later operations within the while loop */
                skb_new = NULL;
                devcs = rd->devcs;
                pkt_len = RCVPKT_LENGTH(devcs);
		skb = lp->rx_skb[rx_next_done];

		if ((devcs & ( ETHRX_ld_m)) !=	ETHRX_ld_m)
		{
			/* check that this is a whole packet */
			/* WARNING: DMA_FD bit incorrectly set in Rc32438 (errata ref #077) */
			lp->stats.rx_errors++;
			lp->stats.rx_dropped++;
		}
		else if (pkt_len < 64)
		{
			lp->stats.rx_errors++;
			lp->stats.rx_dropped++;
		}
		else if ( (devcs & ETHRX_rok_m)  )
		{
			/* must be the (first and) last descriptor then */
			pkt_buf = (u8*)lp->rx_skb[rx_next_done]->data;

			pktuncrc_len = pkt_len - 4;
			/* invalidate the cache */
			dma_cache_inv((unsigned long)pkt_buf, pktuncrc_len);

			/* Malloc up new buffer. */
			skb_new = dev_alloc_skb(RC32438_RBSIZE + 2);

			if (skb_new != NULL)
			{
				/* Make room */
				skb_put(skb, pktuncrc_len);

				skb->protocol = eth_type_trans(skb, dev);

				/* pass the packet to upper layers */
				netif_receive_skb(skb);
				dev->last_rx = jiffies;
				lp->stats.rx_packets++;
				lp->stats.rx_bytes += pktuncrc_len;

				if (IS_RCV_MP(devcs))
					lp->stats.multicast++;

				/* 16 bit align */
				skb_reserve(skb_new, 2);

				skb_new->dev = dev;
				lp->rx_skb[rx_next_done] = skb_new;
				received++;
			}
			else
			{
				ERR("no memory, dropping rx packet.\n");
				lp->stats.rx_errors++;
				lp->stats.rx_dropped++;
			}
		}
		else
		{
			/* This should only happen if we enable accepting broken packets */
			lp->stats.rx_errors++;
			lp->stats.rx_dropped++;

			/* add statistics counters */
			if (IS_RCV_CRC_ERR(devcs))
			{
				DBG(2, "RX CRC error\n");
				lp->stats.rx_crc_errors++;
			}
			else if (IS_RCV_LOR_ERR(devcs))
			{
				DBG(2, "RX LOR error\n");
				lp->stats.rx_length_errors++;
			}
			else if (IS_RCV_LE_ERR(devcs))
			{
				DBG(2, "RX LE error\n");
				lp->stats.rx_length_errors++;
			}
			else if (IS_RCV_OVR_ERR(devcs))
			{
				/*
				 * The overflow errors are handled through
				 * an interrupt handler.
				 */
				lp->stats.rx_over_errors++;
			}
			else if (IS_RCV_CV_ERR(devcs))
			{
				/* code violation */
				DBG(2, "RX CV error\n");
				lp->stats.rx_frame_errors++;
			}
			else if (IS_RCV_CES_ERR(devcs))
			{
				DBG(2, "RX Preamble error\n");
			}
		}

		rd->devcs = 0;

		/* restore descriptor's curr_addr */
		if(skb_new)
			rd->ca = CPHYSADDR(skb_new->data);
		else
			rd->ca = CPHYSADDR(skb->data);

		rd->control = DMA_COUNT(RC32438_RBSIZE) |DMAD_cod_m |DMAD_iod_m;

		/* There is a race condition here. See below */
		lp->rd_ring[(rx_next_done-1)& RC32438_RDS_MASK].control &=  ~(DMAD_cod_m);

		rx_next_done = (rx_next_done + 1) & RC32438_RDS_MASK;
		rd = &lp->rd_ring[rx_next_done];
		rc32438_writel(~DMAS_d_m, &lp->rx_dma_regs->dmas);
	}

	dev->quota -= received;
	*budget =- received;

	lp->rx_next_done = rx_next_done;
	if(rx_work_limit < 0)
		goto not_done;

	dmas = rc32438_readl(&lp->rx_dma_regs->dmas);

	if(dmas & DMAS_h_m)
	{
#ifdef CONFIG_RC32438_REVISION_ZA
		ovr_und = rc32438_readl(&lp->eth_regs->ethintfc);
		if(ovr_und & (ETHINTFC_ovr_m | ETHINTFC_und_m))
			goto over_under_flow;
#endif

#ifdef RC32438_PROC_DEBUG
		lp->dma_halt_cnt++;
#endif

/***********************************************************************************
  There is a race condition here. Imagine software completed processing 0 descriptor
  and is updating the control field of N-1 descriptor. At the same time, DMA 
  processed N-1 descriptor and is also updating the descriptor. If Software is
  updating the descriptor after DMA, the D bit in the descriptor gets cleared,
  though DMA has set it and halted. Now, when Software arrives to N-1, it sees
  D bit not being set. However, it finds the DMA halted. The Software, in order
  to start the DMA again, loads this descriptor. Though the control field of
  this descriptor is fine, the devcs and ca fields are wrong. Hence, the software
  needs to update those fields before loading DMA.
***********************************************************************************/
		rd->devcs = 0;
		skb = lp->rx_skb[rx_next_done];
		rd->ca = CPHYSADDR(skb->data);
		rc32438_chain_rx(lp,rd);
	}
	rc32438_writel( ~(DMAS_h_m | DMAS_e_m), &lp->rx_dma_regs->dmas);

	netif_rx_complete(dev);
	/* Enable D H E bit in Rx DMA */
	rc32438_writel(rc32438_readl(&lp->rx_dma_regs->dmasm) & ~(DMASM_d_m | DMASM_h_m |DMASM_e_m), &lp->rx_dma_regs->dmasm);

	return 0;

 not_done:
#ifdef RC32438_PROC_DEBUG
	lp->not_done_cnt++;
#endif
	return 1;

#ifdef CONFIG_RC32438_REVISION_ZA
 over_under_flow:
 	netif_rx_complete(dev);
	tasklet_hi_schedule(lp->ovr_und_tasklet);
	return 0;
#endif
}

/* Ethernet Tx DMA interrupt */
static irqreturn_t
rc32438_tx_dma_interrupt(int irq, void *dev_id, struct pt_regs * regs)
{
	struct net_device *dev = (struct net_device *)dev_id;
	struct rc32438_local *lp = netdev_priv(dev);
	volatile u32 dmas,dmasm;
	irqreturn_t retval = IRQ_NONE;

	ASSERT(dev != NULL);

//	spin_lock(&lp->lock);

	dmas = rc32438_readl(&lp->tx_dma_regs->dmas);

	if (dmas & (DMAS_f_m | DMAS_e_m))
	{
		dmasm = rc32438_readl(&lp->tx_dma_regs->dmasm);
		/* Mask F E bit in Tx DMA */
		rc32438_writel(dmasm | (DMASM_f_m | DMASM_e_m), &lp->tx_dma_regs->dmasm);

		tasklet_schedule(lp->tx_tasklet);

		if(lp->tx_chain_status == filled && (rc32438_readl(&(lp->tx_dma_regs->dmandptr)) == 0))
		{
			rc32438_writel(CPHYSADDR(&lp->td_ring[lp->tx_chain_head]), &(lp->tx_dma_regs->dmandptr));
			lp->tx_chain_status = empty;
			lp->tx_chain_head = lp->tx_chain_tail;
			dev->trans_start = jiffies;
		}

		if (dmas & DMAS_e_m)
			ERR(": DMA error\n");

		retval = IRQ_HANDLED;
	}

//	spin_unlock(&lp->lock);

	return retval;
}

static void rc32438_tx_tasklet(unsigned long tx_data_dev)
{
        struct net_device *dev = (struct net_device *)tx_data_dev;
	struct rc32438_local *lp = netdev_priv(dev);
        volatile DMAD_t td = &lp->td_ring[lp->tx_next_done];
        unsigned long   flags;
        volatile u32 dmas;
        volatile u32 devcs;

	u32 tx_next_done = lp->tx_next_done;
#ifdef CONFIG_SMP
        spin_lock_irqsave(&lp->lock, flags);
#else
	local_irq_save(flags);
#endif

        /* process all desc that are done */
        while(IS_DMA_FINISHED(td->control))
        {
                if(lp->tx_full == 1)
                {
                        netif_wake_queue(dev);
                        lp->tx_full = 0;
                }

                devcs = lp->td_ring[tx_next_done].devcs;
                if ((devcs & (ETHTX_fd_m | ETHTX_ld_m)) != (ETHTX_fd_m | ETHTX_ld_m))
                {
                        lp->stats.tx_errors++;
                        lp->stats.tx_dropped++;

                        /* should never happen */
                        DBG(1, __FUNCTION__ ": split tx ignored\n");
                }
                else if (IS_TX_TOK(devcs))
                {
                        /* transmit OK */
                        lp->stats.tx_packets++;
                }
                else
                {
                        lp->stats.tx_errors++;
                        lp->stats.tx_dropped++;

                        /* underflow */
                        if (IS_TX_UND_ERR(devcs))
                                lp->stats.tx_fifo_errors++;

                        /* oversized frame */
                        if (IS_TX_OF_ERR(devcs))
                                lp->stats.tx_aborted_errors++;

                        /* excessive deferrals */
                        if (IS_TX_ED_ERR(devcs))
                                lp->stats.tx_carrier_errors++;

                        /* collisions: medium busy */
                        if (IS_TX_EC_ERR(devcs))
                                lp->stats.collisions++;

                        /* late collision */
                        if (IS_TX_LC_ERR(devcs))
                                lp->stats.tx_window_errors++;

                }

                /* We must always free the original skb */
                if (lp->tx_skb[tx_next_done] != NULL)
                {
                        dev_kfree_skb_any(lp->tx_skb[tx_next_done]);
                        lp->tx_skb[tx_next_done] = NULL;
                }

                lp->td_ring[tx_next_done].control = DMAD_iof_m;
                lp->td_ring[tx_next_done].devcs = ETHTX_fd_m | ETHTX_ld_m;
                lp->td_ring[tx_next_done].link = 0;
                lp->td_ring[tx_next_done].ca = 0;
                lp->tx_count --;

                /* go on to next transmission */
                tx_next_done = (tx_next_done + 1) & RC32438_TDS_MASK;
                td = &lp->td_ring[tx_next_done];

        }
	lp->tx_next_done = tx_next_done;
        dmas = rc32438_readl(&lp->tx_dma_regs->dmas);
	rc32438_writel( ~dmas, &lp->tx_dma_regs->dmas);

        /* Enable F E bit in Tx DMA */
        rc32438_writel(rc32438_readl(&lp->tx_dma_regs->dmasm) & ~(DMASM_f_m | DMASM_e_m), &lp->tx_dma_regs->dmasm);
#ifdef CONFIG_SMP
        spin_unlock_irqrestore(&lp->lock, flags);
#else
	local_irq_restore(flags);
#endif
}
/*
 * Get the current statistics.
 * This may be called with the device open or closed.
 */
static struct net_device_stats * rc32438_get_stats(struct net_device *dev)
{
	struct rc32438_local *lp = netdev_priv(dev);
	return &lp->stats;
}


/*
 * Set or clear the multicast filter for this adaptor.
 */
static void rc32438_multicast_list(struct net_device *dev)
{
	/* listen to broadcasts always and to treat 	*/
	/*       IFF bits independantly	*/

	struct rc32438_local *lp = netdev_priv(dev);
	unsigned long flags;
	u32 recognise = ETHARC_ab_m; 		/* always accept broadcasts */

	if (dev->flags & IFF_PROMISC)         		/* set promiscuous mode */
		recognise |= ETHARC_pro_m;

	if ((dev->flags & IFF_ALLMULTI) || (dev->mc_count > 15))
		recognise |= ETHARC_am_m;    	  	/* all multicast & bcast */
	else if (dev->mc_count > 0)
	{
		DBG(2, __FUNCTION__ ": mc_count %d\n", dev->mc_count);
		recognise |= ETHARC_am_m;    	  	/* for the time being */
	}

#ifdef CONFIG_SMP
	spin_lock_irqsave(&lp->lock, flags);
#else
	local_irq_save(flags);
#endif
	rc32438_writel(recognise, &lp->eth_regs->etharc);
#ifdef CONFIG_SMP
	spin_unlock_irqrestore(&lp->lock, flags);
#else
	local_irq_restore(flags);
#endif
}


static void rc32438_tx_timeout(struct net_device *dev)
{
	unsigned long flags;

#ifdef CONFIG_SMP
	struct rc32438_local *lp = netdev_priv(dev);
	spin_lock_irqsave(&lp->lock, flags);
#else
	local_irq_save(flags);
#endif
	rc32438_restart(dev);
#ifdef CONFIG_SMP
	spin_unlock_irqrestore(&lp->lock, flags);
#else
	local_irq_restore(flags);
#endif

}


/*
 * Initialize the RC32438 ethernet controller.
 */
static int rc32438_init(struct net_device *dev)
{
	struct rc32438_local *lp = netdev_priv(dev);
	int i, j;

	/* Disable DMA */
	rc32438_abort_tx(dev);
	rc32438_abort_rx(dev);

	/* reset ethernet logic */
	rc32438_writel(0, &lp->eth_regs->ethintfc);
	while((rc32438_readl(&lp->eth_regs->ethintfc) & ETHINTFC_rip_m))
		dev->trans_start = jiffies;

	/* Enable Ethernet Interface */
	rc32438_writel(ETHINTFC_en_m, &lp->eth_regs->ethintfc);
#ifdef CONFIG_RC32438_REVISION_ZA
	tasklet_disable(lp->ovr_und_tasklet);
#endif
	tasklet_disable(lp->tx_tasklet);

	/* Initialize the transmit Descriptors */
	for (i = 0; i < RC32438_NUM_TDS; i++)
	{
		lp->td_ring[i].control = DMAD_iof_m;
		lp->td_ring[i].devcs = ETHTX_fd_m | ETHTX_ld_m;
		lp->td_ring[i].ca = 0;
		lp->td_ring[i].link = 0;
		if (lp->tx_skb[i] != NULL)
		{
			/* free dangling skb */
			dev_kfree_skb_any(lp->tx_skb[i]);
			lp->tx_skb[i] = NULL;
		}
	}
	lp->tx_next_done = lp->tx_chain_head = lp->tx_chain_tail = 	lp->tx_full = lp->tx_count = 0;
	lp->tx_chain_status = empty;

	/*
	 * Initialize the receive descriptors so that they
	 * become a circular linked list, ie. let the last
	 * descriptor point to the first again.
	 */
	for (i=0; i<RC32438_NUM_RDS; i++)
	{
		struct sk_buff *skb = lp->rx_skb[i];

		if (lp->rx_skb[i] == NULL)
		{
			skb = dev_alloc_skb(RC32438_RBSIZE + 2);
			if (skb == NULL)
			{
				ERR("No memory in the system\n");
				for (j = 0; j < RC32438_NUM_RDS; j ++)
					if (lp->rx_skb[j] != NULL)
						dev_kfree_skb_any(lp->rx_skb[j]);

				return 1;
			}
			else
			{
				skb->dev = dev;
				skb_reserve(skb, 2);
				lp->rx_skb[i] = skb;
				lp->rd_ring[i].ca = CPHYSADDR(skb->data);

			}
		}
		lp->rd_ring[i].control =	DMAD_iod_m | DMA_COUNT(RC32438_RBSIZE);
		lp->rd_ring[i].devcs = 0;
		lp->rd_ring[i].ca = CPHYSADDR(skb->data);
		lp->rd_ring[i].link = CPHYSADDR(&lp->rd_ring[i+1]);

	}
	/* loop back */
	lp->rd_ring[RC32438_NUM_RDS-1].link = CPHYSADDR(&lp->rd_ring[0]);
	lp->rx_next_done   = 0;

	lp->rd_ring[RC32438_NUM_RDS-1].control |= DMAD_cod_m;
	lp->rx_chain_head = 0;
	lp->rx_chain_tail = 0;
	lp->rx_chain_status = empty;

	rc32438_writel(0, &lp->rx_dma_regs->dmas);
	/* Start Rx DMA */
	rc32438_start_rx(lp, &lp->rd_ring[0]);

	/* Enable F E bit in Tx DMA */
	rc32438_writel(rc32438_readl(&lp->tx_dma_regs->dmasm) & ~(DMASM_f_m | DMASM_e_m), &lp->tx_dma_regs->dmasm);
	/* Enable D H E bit in Rx DMA */
	rc32438_writel(rc32438_readl(&lp->rx_dma_regs->dmasm) & ~(DMASM_d_m | DMASM_h_m | DMASM_e_m), &lp->rx_dma_regs->dmasm);

	/* Accept only packets destined for this Ethernet device address */
	rc32438_writel(ETHARC_ab_m, &lp->eth_regs->etharc);

	/* Set all Ether station address registers to their initial values */
	rc32438_writel(STATION_ADDRESS_LOW(dev), &lp->eth_regs->ethsal0);
	rc32438_writel(STATION_ADDRESS_HIGH(dev), &lp->eth_regs->ethsah0);

	rc32438_writel(STATION_ADDRESS_LOW(dev), &lp->eth_regs->ethsal1);
	rc32438_writel(STATION_ADDRESS_HIGH(dev), &lp->eth_regs->ethsah1);

	rc32438_writel(STATION_ADDRESS_LOW(dev), &lp->eth_regs->ethsal2);
	rc32438_writel(STATION_ADDRESS_HIGH(dev), &lp->eth_regs->ethsah2);

	rc32438_writel(STATION_ADDRESS_LOW(dev), &lp->eth_regs->ethsal3);
	rc32438_writel(STATION_ADDRESS_HIGH(dev), &lp->eth_regs->ethsah3);


	/* Frame Length Checking, Pad Enable, CRC Enable, Full Duplex set */
	rc32438_writel(ETHMAC2_pe_m | ETHMAC2_cen_m | ETHMAC2_fd_m, &lp->eth_regs->ethmac2);
	//ETHMAC2_flc_m		ETHMAC2_fd_m	lp->duplex_mode

	/* Back to back inter-packet-gap */
	rc32438_writel(0x15, &lp->eth_regs->ethipgt);
	/* Non - Back to back inter-packet-gap */
	rc32438_writel(0x12, &lp->eth_regs->ethipgr);

	/* Management Clock Prescaler Divisor */
	/* Clock independent setting */
	rc32438_writel(((idt_cpu_freq)/MII_CLOCK+1) & ~1,
		       &lp->eth_regs->ethmcp);

	/* don't transmit until fifo contains 48b */
	rc32438_writel(48, &lp->eth_regs->ethfifott);

	rc32438_writel(ETHMAC1_re_m, &lp->eth_regs->ethmac1);

#ifdef CONFIG_RC32438_REVISION_ZA
	tasklet_enable(lp->ovr_und_tasklet);
#endif
	tasklet_enable(lp->tx_tasklet);

	netif_start_queue(dev);


	return 0;

}


#ifndef MODULE

static int __init rc32438_setup(char *options)
{
	/* no options yet */
	return 1;
}

static int __init rc32438_setup_ethaddr0(char *options)
{
	memcpy(mac0, options, 17);
	mac0[17]= '\0';
	return 1;
}

static int __init rc32438_setup_ethaddr1(char *options)
{
	memcpy(mac1, options, 17);
	mac1[17]= '\0';
	return 1;
}

__setup("rc32438eth=", rc32438_setup);
__setup("ethaddr0=", rc32438_setup_ethaddr0);
__setup("ethaddr1=", rc32438_setup_ethaddr1);


#endif /* MODULE */

module_init(rc32438_init_module);
module_exit(rc32438_cleanup_module);















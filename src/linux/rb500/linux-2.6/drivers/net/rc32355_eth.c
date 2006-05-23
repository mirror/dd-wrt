/**************************************************************************
 *
 *  BRIEF MODULE DESCRIPTION
 *     Driver for the IDT RC32355 on-chip ethernet controller.
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
 * May 2004 rkt
 *
 *  Based on the driver written by B. Maruthanayakam, H. Kou and others.
 *
 *
 **************************************************************************
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/ctype.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/interrupt.h>
#include <linux/ptrace.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/proc_fs.h>
#include <linux/in.h>
#include <linux/slab.h> 
#include <linux/string.h>
#include <linux/delay.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/errno.h>
#include <asm/bootinfo.h>
#include <asm/system.h>
#include <asm/bitops.h>
#include <asm/pgtable.h>
#include <asm/segment.h>
#include <asm/io.h>
#include <asm/dma.h>

#include "rc32355_eth.h"

#define DRIVER_VERSION "(apr2904)"
#define DRIVER_NAME "rc32355 Ethernet driver. " DRIVER_VERSION
#define STATION_ADDRESS_HIGH(dev) (((dev)->dev_addr[0] << 8) | \
				   ((dev)->dev_addr[1]))
#define STATION_ADDRESS_LOW(dev)  (((dev)->dev_addr[2] << 24) | \
				   ((dev)->dev_addr[3] << 16) | \
				   ((dev)->dev_addr[4] << 8)  | \
				   ((dev)->dev_addr[5]))

#define MII_CLOCK 1250000 					/* no more than 2.5MHz */
#define NINTFC		2 						/* number of interface */
static char mac[18] = "08:00:06:05:40:01"; 

MODULE_AUTHOR ("IDT Inc");
MODULE_DESCRIPTION ("rc32365 Ethernet driver");
MODULE_LICENSE("GPL");
MODULE_PARM(mac, "c18");
MODULE_PARM_DESC(mac0, "MAC address for RC32355 ethernet");

static struct rc32355_if_t 
{
	struct net_device *dev;
  	char* mac_str;
  	u32 iobase;
  	int rx_dma_irq;
  	int tx_dma_irq;
  	int rx_ovr_irq;
  	int tx_und_irq;		
} rc32355_iflist[] = 
{
	{NULL, mac, RC32355_ETH_BASE, ETH_DMA_RX_IRQ, ETH_DMA_TX_IRQ, ETH_RX_OVR_IRQ, ETH_TX_UND_IRQ},
	{NULL, NULL, 0, 0,0,0,0}
};


static int parse_mac_addr(struct net_device *dev, char* macstr)
{
  	int i, j;
  	unsigned char result, value;
	
  	for (i=0; i<6; i++) 
	{
    		result = 0;
    		if (i != 5 && *(macstr+2) != ':') {
			ERR(__FILE__ "invalid mac address format: %d %c\n", i, *(macstr+2));
      			return -EINVAL;
    		}
		
	    	for (j=0; j<2; j++) {
			if (isxdigit(*macstr) && (value = isdigit(*macstr) ? *macstr-'0' : toupper(*macstr)-'A'+10) < 16) {
				result = result*16 + value;
				macstr++;
			} 
			else {
				ERR(__FILE__ "invalid mac address character: %c\n", *macstr);
				return -EINVAL;
			}
		}
		
    		macstr++;
    		dev->dev_addr[i] = result;
  	}
	
  	return 0;
}

static inline void rc32355_abort_tx(struct net_device *dev)
{
	struct rc32355_local *lp = (struct rc32355_local *)dev->priv;
	rc32355_abort_dma(dev, lp->tx_dma_regs);
	
}

static inline void rc32355_abort_rx(struct net_device *dev)
{
	struct rc32355_local *lp = (struct rc32355_local *)dev->priv;
	rc32355_abort_dma(dev, lp->rx_dma_regs);
	
}

static inline void rc32355_halt_tx(struct net_device *dev)
{
	struct rc32355_local *lp = (struct rc32355_local *)dev->priv;
	if (rc32355_halt_dma(lp->tx_dma_regs))
		ERR(__FUNCTION__ ": timeout!\n");
}

static inline void rc32355_halt_rx(struct net_device *dev)
{
	struct rc32355_local *lp = (struct rc32355_local *)dev->priv;
	if (rc32355_halt_dma(lp->rx_dma_regs))
		ERR(__FUNCTION__ ": timeout!\n");
}

static inline void rc32355_start_tx(struct rc32355_local *lp, volatile rc32355_dma_desc_t* td)
{
printk("inside start_tx\n");
	rc32355_start_dma(lp->tx_dma_regs, CPHYSADDR(td));
}

static inline void rc32355_start_rx(struct rc32355_local *lp, volatile rc32355_dma_desc_t* rd)
{
printk("inside start_rx\n");
	rc32355_start_dma(lp->rx_dma_regs, CPHYSADDR(rd));
}

static inline void rc32355_chain_tx(struct rc32355_local *lp, volatile rc32355_dma_desc_t* td)
{
	rc32355_chain_dma(lp->tx_dma_regs, CPHYSADDR(td));
}

static inline void rc32355_chain_rx(struct rc32355_local *lp, volatile rc32355_dma_desc_t* rd)
{
	rc32355_chain_dma(lp->rx_dma_regs, CPHYSADDR(rd));
}

#ifdef RC32355_PROC_DEBUG
static int rc32355_read_proc(char *buf, char **start, off_t fpos,
			     int length, int *eof, void *data)
{
  	struct net_device *dev = (struct net_device *)data;
  	struct rc32355_local *lp = (struct rc32355_local *)dev->priv;
  	int len = 0;
	
  	/* print out header */
  	len += sprintf(buf + len, "\n\tRC32355 Ethernet Debug\n\n");
	
  	len += sprintf (buf + len,
			"DMA halt count      = %10d, total pkt cnt = %10d\n",
			lp->dma_halt_cnt, lp->halt_tx_count);
  	len += sprintf (buf + len,
			"DMA run count       = %10d, total pkt cnt = %10d\n",
			lp->dma_run_cnt, lp->run_tx_count);
  	len += sprintf (buf + len,
			"DMA race count      = %10d, total pkt cnt = %10d\n",
			lp->dma_race_cnt, lp->race_tx_count);
  	len += sprintf (buf + len,
			"DMA collision count = %10d, total pkt cnt = %10d\n",
			lp->dma_collide_cnt, lp->collide_tx_count);
	
  	if (fpos >= len) {
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
 * Restart the RC32355 ethernet controller. Hold a spin lock
 * before calling.
 */
static int rc32355_restart(struct net_device *dev)
{
  	struct rc32355_local *lp = (struct rc32355_local *)dev->priv;
	
  	/*
	 * Disable interrupts
	 */
  	disable_irq(lp->rx_irq);
  	disable_irq(lp->tx_irq);
  	disable_irq(lp->ovr_irq);
    	disable_irq(lp->und_irq);
	
  	/* Mask F bit in Tx DMA */
  	local_writel(local_readl(&lp->tx_dma_regs->dmasm) | DMAS_F, &lp->tx_dma_regs->dmasm);
    	/* Mask D bit in Rx DMA */
  	local_writel(local_readl(&lp->rx_dma_regs->dmasm) | DMAS_D, &lp->rx_dma_regs->dmasm);
	
	rc32355_init(dev);
	rc32355_multicast_list(dev);
	
  	enable_irq(lp->und_irq);
  	enable_irq(lp->ovr_irq);
  	enable_irq(lp->tx_irq);
  	enable_irq(lp->rx_irq);
	
  	return 0;
}

int rc32355_init_module(void)
{
  	int retval=0;
	
	printk(KERN_INFO DRIVER_NAME " \n");
	retval = rc32355_probe(0);
	return retval;
}

static int rc32355_probe(int port_num)
{
  	struct rc32355_local *lp = NULL;
  	struct rc32355_if_t *bif = NULL;
  	struct net_device *dev = NULL;
  	int i, retval,err;
	
	bif = &rc32355_iflist[port_num];
	dev = alloc_etherdev(sizeof(struct rc32355_local));
	if(!dev)
	{ 
		ERR("rc32438_eth: alloc_etherdev failed\n");
		free_netdev(dev);
		return -1;
	}
	
	SET_MODULE_OWNER(dev);
	
	bif->dev = dev;
	
	
  	if ((retval = parse_mac_addr(dev, bif->mac_str))) {
		ERR(__FUNCTION__ ": MAC address parse failed\n");
		retval = -EINVAL;
		goto probe1_err_out;
	}
	
	
	/* Initialize the device structure. */
  	if (dev->priv == NULL) {
		lp = (struct rc32355_local *)kmalloc(sizeof(*lp), GFP_KERNEL);
		memset(lp, 0, sizeof(struct rc32355_local));
	} 
	else {
		lp = (struct rc32355_local *)dev->priv;
	}
  	lp->rx_irq = bif->rx_dma_irq;
  	lp->tx_irq = bif->tx_dma_irq;
    	lp->ovr_irq = bif->rx_ovr_irq;
  	lp->und_irq = bif->tx_und_irq;
	
  	lp->eth_regs = ioremap_nocache(bif->iobase, sizeof(*lp->eth_regs));
	
  	if (!lp->eth_regs) {
		ERR("Can't remap eth registers\n");
		retval = -ENXIO;
		goto probe1_err_out;
	}
	
	lp->rx_dma_regs =
		ioremap_nocache(RC32355_DMA_BASE + 9*DMA_CHAN_OFFSET,
				sizeof(rc32355_dma_ch_t));
	if (!lp->rx_dma_regs) {
		ERR("Can't remap Rx DMA registers\n");
		retval = -ENXIO;
		goto probe1_err_out;
	}
	
	lp->tx_dma_regs =
		ioremap_nocache(RC32355_DMA_BASE + 10*DMA_CHAN_OFFSET,
				sizeof(rc32355_dma_ch_t));
	if (!lp->tx_dma_regs) {
		ERR("Can't remap Tx DMA registers\n");
		retval = -ENXIO;
		goto probe1_err_out;
	}
	
  	lp->td_ring = (rc32355_dma_desc_t*)kmalloc(TD_RING_SIZE + RD_RING_SIZE, GFP_KERNEL);
  	
	if (!lp->td_ring) {
		ERR("Can't allocate descriptors\n");
		retval = -ENOMEM;
		goto probe1_err_out;
	}
	
  	dma_cache_inv((unsigned long)(lp->td_ring), TD_RING_SIZE + RD_RING_SIZE);
	
  	/* now convert TD_RING pointer to KSEG1 */
  	lp->td_ring = (rc32355_dma_desc_t *)KSEG1ADDR(lp->td_ring);
  	lp->rd_ring = &lp->td_ring[RC32355_NUM_TDS];
	
  	spin_lock_init(&lp->lock);
	dev->base_addr = bif->iobase;
	/* just use the rx dma irq */
	
	dev->irq = bif->rx_dma_irq; 
	dev->priv = lp;
  	dev->open = rc32355_open;
  	dev->stop = rc32355_close;
  	dev->hard_start_xmit = rc32355_send_packet;
  	dev->get_stats	= rc32355_get_stats;
  	dev->set_multicast_list = &rc32355_multicast_list;
  	dev->tx_timeout = rc32355_tx_timeout;
  	dev->watchdog_timeo = RC32355_TX_TIMEOUT;
	
	lp->rx_tasklet = kmalloc(sizeof(struct tasklet_struct), GFP_KERNEL);
	tasklet_init(lp->rx_tasklet, rc32355_rx_tasklet, (unsigned long)dev);
	lp->tx_tasklet = kmalloc(sizeof(struct tasklet_struct), GFP_KERNEL);
	tasklet_init(lp->tx_tasklet, rc32355_tx_tasklet, (unsigned long)dev);
	
	
#ifdef RC32355_PROC_DEBUG
  	lp->ps = create_proc_read_entry ("net/rc32355", 0, NULL,
					 rc32355_read_proc, dev);
#endif
	
	
	if ((err = register_netdev(dev))) {
		printk(KERN_ERR "rc32355 ethernet. Cannot register net device %d\n", err);
		free_netdev(dev);
		retval = -EINVAL;
		goto probe1_err_out;
	}
	
	INFO("HW Address ");
  	for (i = 0; i < 6; i++) 
	{
		printk("%2.2x", dev->dev_addr[i]);
		if (i<5)
			printk(":");
	}
  	printk("\n");
	INFO("Rx IRQ %d, Tx IRQ %d\n", bif->rx_dma_irq, bif->tx_dma_irq);
	
	/* Fill in the fields of the device structure with ethernet values. */
  	ether_setup(dev);
  	return 0;
	
 probe1_err_out:
  	rc32355_cleanup_module();
  	ERR(__FUNCTION__ " failed.  Returns %d\n", retval);
  	return retval;
	
}


/*
 * Open/initialize the RC32355 controller.
 *
 * This routine should set everything up anew at each open, even
 *  registers that "should" only need to be set once at boot, so that
 *  there is non-reboot way to recover if something goes wrong.
 */
static int rc32355_open(struct net_device *dev)
{
	struct rc32355_local *lp = (struct rc32355_local *)dev->priv;
	
    	/*
	 * Initialize
	 */
  	if (rc32355_init(dev)) {
		ERR("Error: cannot open the Ethernet device\n");
    		return -EAGAIN;
  	}
	
	/*
	 * Install the interrupt handler that handles the dma Done and
	 * Finished Events.
	 */
    	if (request_irq(lp->rx_irq, &rc32355_rx_dma_interrupt,
			SA_SHIRQ | SA_INTERRUPT,
			"rc32355 ethernet Rx", dev)) {
		
    		ERR(__FUNCTION__ ": unable to get Rx DMA IRQ %d\n", lp->rx_irq);
		return -EAGAIN;
  	}
  	if (request_irq(lp->tx_irq, &rc32355_tx_dma_interrupt,
			SA_SHIRQ | SA_INTERRUPT,
			"rc32355 ethernet Tx", dev)) {
		ERR(__FUNCTION__ ": unable to get Tx DMA IRQ %d\n", lp->tx_irq);
    		free_irq(lp->rx_irq, dev);
		return -EAGAIN;
  	}
	
  	/* Install handler for overrun error. */
  	if (request_irq(lp->ovr_irq, &rc32355_ovr_interrupt,
			SA_SHIRQ | SA_INTERRUPT,
			"rc32355 ethernet Overflow", dev)) {
		ERR(__FUNCTION__ ": unable to get OVR IRQ %d\n",	lp->ovr_irq);
    		free_irq(lp->rx_irq, dev);
    		free_irq(lp->tx_irq, dev);
		return -EAGAIN;
  	}
	
  	/* Install handler for underflow error. */
	if (request_irq(lp->und_irq, &rc32355_und_interrupt,
			SA_SHIRQ | SA_INTERRUPT,
			"rc32355 ethernet Underflow", dev)) {
		ERR(__FUNCTION__ ": unable to get UND IRQ %d\n",
		    lp->und_irq);
		free_irq(lp->rx_irq, dev);
		free_irq(lp->tx_irq, dev);
		free_irq(lp->ovr_irq, dev);
		return -EAGAIN;
	}
	
  	return 0;
}



/*
 * Close the RC32355 device
 */
static int rc32355_close(struct net_device *dev)
{
  	struct rc32355_local *lp = (struct rc32355_local *)dev->priv;
  	u32 tmp;
	
	/*
	 * Disable interrupts
	 */
  	disable_irq(lp->rx_irq);
  	disable_irq(lp->tx_irq);
	disable_irq(lp->ovr_irq);
    	disable_irq(lp->und_irq);
	
  	tmp = local_readl(&lp->tx_dma_regs->dmasm);
  	tmp = tmp | DMAS_F | DMAS_E;
  	local_writel(tmp, &lp->tx_dma_regs->dmasm);
	
  	tmp = local_readl(&lp->rx_dma_regs->dmasm);
	tmp = tmp | DMAS_D | DMAS_H | DMAS_E;
	local_writel(tmp, &lp->rx_dma_regs->dmasm);
	
  	free_irq(lp->rx_irq, dev);
  	free_irq(lp->tx_irq, dev);
  	free_irq(lp->ovr_irq, dev);
    	free_irq(lp->und_irq, dev);	

  	return 0;
}


/* transmit packet */
static int rc32355_send_packet(struct sk_buff *skb, struct net_device *dev)
{
	struct rc32355_local	*lp = (struct rc32355_local *)dev->priv;
	unsigned long 	flags;
	u32			length;
	volatile rc32355_dma_desc_t * td;
	
	spin_lock_irqsave(&lp->lock, flags);
	
	td = &lp->td_ring[lp->tx_chain_tail];
	
	//stop queue when full, drop pkts if queue already full
	if(lp->tx_count >= (RC32355_NUM_TDS - 2)){
		lp->tx_full = 1;
		
		if(lp->tx_count == (RC32355_NUM_TDS - 2)) {
			//this pkt is about to fill the queue
			//ERR("Tx Ring now full, queue stopped.\n");			
			netif_stop_queue(dev);
		}else{
			//this pkt cannot be added to the full queue
			//ERR("Tx ring full, packet dropped\n");
			lp->stats.tx_dropped++;
			dev_kfree_skb_any(skb);
			spin_unlock_irqrestore(&lp->lock, flags);
			return 1;
		}	   
	}	   
	lp->tx_count ++;
	
	/* make sure payload gets written to memory */
	dma_cache_wback_inv((unsigned long)skb->data, skb->len);
	
	if (lp->tx_skb[lp->tx_chain_tail] != NULL)
		dev_kfree_skb_any(lp->tx_skb[lp->tx_chain_tail]);
	
	lp->tx_skb[lp->tx_chain_tail] = skb;
	
	length = skb->len; 
	
	//Setup the transmit descriptor.
	td->curr_addr = CPHYSADDR(skb->data);
	
	/* Using the NDPTR to handl the DMA Race Condition */
	if(local_readl(&(lp->tx_dma_regs->dmandptr)) == 0) {
		if( lp->tx_chain_status == empty ) {
			td->cmdstat = DMA_COUNT(length) |DMADESC_COF |DMADESC_IOF; /* Update tail */
			lp->tx_chain_tail = (lp->tx_chain_tail + 1) & RC32355_TDS_MASK; /* Move tail */
			local_writel(CPHYSADDR(&lp->td_ring[lp->tx_chain_head]), &(lp->tx_dma_regs->dmandptr)); /* Write to NDPTR */
			lp->tx_chain_head = lp->tx_chain_tail; /* Move head to tail */
		}
		else
		{
			td->cmdstat = DMA_COUNT(length) |DMADESC_COF|DMADESC_IOF; /* Update tail */
			lp->td_ring[(lp->tx_chain_tail-1)& RC32355_TDS_MASK].cmdstat &=  ~(DMADESC_COF); /* Update prev */
			lp->td_ring[(lp->tx_chain_tail-1)& RC32355_TDS_MASK].link =  CPHYSADDR(td); /* Link prev to this one */
			lp->tx_chain_tail = (lp->tx_chain_tail + 1) & RC32355_TDS_MASK; /* Move tail */
			local_writel(CPHYSADDR(&lp->td_ring[lp->tx_chain_head]), &(lp->tx_dma_regs->dmandptr)); /* Write to NDPTR */
			lp->tx_chain_head = lp->tx_chain_tail;  /* Move head to tail */
			lp->tx_chain_status = empty;  
		}
	}
	else
	{
		if( lp->tx_chain_status == empty ) {
			td->cmdstat = DMA_COUNT(length) |DMADESC_COF|DMADESC_IOF; /* Update tail */
			lp->tx_chain_tail = (lp->tx_chain_tail + 1) & RC32355_TDS_MASK; /* Move tail */
			lp->tx_chain_status = filled;		
		}
		else {
			td->cmdstat = DMA_COUNT(length) |DMADESC_COF |DMADESC_IOF; /* Update tail */
			lp->td_ring[(lp->tx_chain_tail-1)& RC32355_TDS_MASK].cmdstat &=  ~(DMADESC_COF);   /* Update prev */
			lp->td_ring[(lp->tx_chain_tail-1)& RC32355_TDS_MASK].link =  CPHYSADDR(td);	 /* Link prev to this one*/
			lp->tx_chain_tail = (lp->tx_chain_tail + 1) & RC32355_TDS_MASK; /* Move tail */
		}		
	}
	
	
	dev->trans_start = jiffies;
	
     	spin_unlock_irqrestore(&lp->lock, flags);
	
	
  	return 0;
}



/* Ethernet Rx Overflow interrupt */
static irqreturn_t rc32355_ovr_interrupt(int irq, void *dev_id, struct pt_regs * regs)
{
  	struct net_device *dev = (struct net_device *)dev_id;
  	struct rc32355_local *lp;
  	unsigned int i;
	irqreturn_t retval = IRQ_NONE;
	
	ASSERT(dev != NULL);
	
  	lp = (struct rc32355_local *)dev->priv;
	netif_stop_queue(dev);
	
	//ERR("Rx overflow\n");
	
  	spin_lock(&lp->lock);
	
  	//clear OVR int (sticky bit)
  	i = local_readl(&lp->eth_regs->ethintfc);
  	i &= ~ETHERDMA_IN_OVR;
  	local_writel(i, &lp->eth_regs->ethintfc);
	
  	// Restart interface
  	rc32355_restart(dev);	  
	retval = IRQ_HANDLED;
  	spin_unlock(&lp->lock);
	
	return retval;
	
}

/* Ethernet Tx Underflow interrupt */
static irqreturn_t rc32355_und_interrupt(int irq, void *dev_id, struct pt_regs * regs)
{
	struct net_device *dev = (struct net_device *)dev_id;
	struct rc32355_local *lp;
	unsigned int i;
	irqreturn_t retval = IRQ_NONE;
	
	ASSERT(dev != NULL);
	
	printk(__FILE__" %d\n",__LINE__);
	netif_stop_queue(dev);	
	lp = (struct rc32355_local *)dev->priv;
	
	//ERR("Tx underflow - i/f reset\n");
	spin_lock(&lp->lock);
	
	i = local_readl(&lp->eth_regs->ethintfc);
  	i &= ~ETHERDMA_OUT_UND;
	local_writel(i, &lp->eth_regs->ethintfc);
	
	/* Restart interface */
	rc32355_restart(dev);     
	retval = IRQ_HANDLED;
	spin_unlock(&lp->lock);
	return retval;
	
}

/* Ethernet Rx DMA interrupt */
static irqreturn_t
rc32355_rx_dma_interrupt(int irq, void *dev_id, struct pt_regs * regs)
{
  	struct net_device *dev = (struct net_device *)dev_id;
  	struct rc32355_local *lp;
  	volatile u32 dmas;
	irqreturn_t retval = IRQ_NONE;
	
	ASSERT(dev != NULL);
	
  	lp = (struct rc32355_local *)dev->priv;
	
  	spin_lock(&lp->lock);
	/* Mask D H & E bit in Rx DMA so that no interrupts are generated. The rx_tasklet
	   will take care of un-masking them.	*/
	
	local_writel(local_readl(&lp->rx_dma_regs->dmasm) | DMAS_D | DMAS_E |DMAS_H, &lp->rx_dma_regs->dmasm);
	
  	dmas = local_readl(&lp->rx_dma_regs->dmas);
	/* If DMA has halted, deal with it immediately. Else, process the packets in tasklet */
	if(dmas & DMAS_H)
		lp->rx_tasklet->func((unsigned long)dev);
	
  	else if(dmas & (DMAS_D|DMAS_E)) {
		tasklet_hi_schedule(lp->rx_tasklet);
		if(dmas & DMAS_E) {
			lp->stats.rx_errors++;
		}
  	}
	retval = IRQ_HANDLED;
  	spin_unlock(&lp->lock);
	return retval;
}


static void rc32355_rx_tasklet(unsigned long rx_data_dev)
{
	struct net_device *dev = (struct net_device *)rx_data_dev;	
  	struct rc32355_local* lp = (struct rc32355_local *)dev->priv;
  	volatile rc32355_dma_desc_t*  rd = &lp->rd_ring[lp->rx_next_done];
  	struct sk_buff *skb, *skb_new;
	u8* pkt_buf;
  	u32 devcs, count, pkt_len;
  	unsigned long 	flags;
	volatile u32 dmas;
	
	spin_lock_irqsave(&lp->lock, flags);
	
  	/* keep going while we have received into more descriptors */
        while ( (count = RC32355_RBSIZE - (u32)DMA_COUNT(rd->cmdstat)) != 0)
    	{
		
		/* init the var. used for the later operations within the while loop */
		skb_new = NULL;
		devcs = rd->devcs;
		pkt_len = RCVPKT_LENGTH(devcs);
		skb = lp->rx_skb[lp->rx_next_done];
		
		//count = RC32355_RBSIZE - (u32)DMA_COUNT(rd->cmdstat);
		
		if( count != pkt_len) {
			/*
			 * Due to a bug in rc32355 processor, the packet length
			 * given by devcs field and count field sometimes differ.
			 * If that is the case, report Error.
			 */				
			lp->stats.rx_errors++;
			lp->stats.rx_dropped++;
			
		}
		else if (count < 64) {
			lp->stats.rx_errors++;
			lp->stats.rx_dropped++;
		} 
		else if ((devcs & ( ETHERDMA_IN_LD)) !=	ETHERDMA_IN_LD) {
			/* Check that this is a whole packet */
			/* WARNING: DMA_FD bit incorrectly set in rc32355 (errata ref #077) */
			lp->stats.rx_errors++;
			lp->stats.rx_dropped++;
		} 
		else if (devcs & ETHERDMA_IN_ROK) {
					
			/* must be the (first and) last descriptor then */
			pkt_buf = (u8*)lp->rx_skb[lp->rx_next_done]->data;
			
			/* invalidate the cache before copying the buffer */
			dma_cache_inv((unsigned long)pkt_buf, (pkt_len-4));
			
			/* Malloc up new buffer. */					  
			skb_new = dev_alloc_skb(RC32355_RBSIZE + 2);							
			
			if (skb_new != NULL) {
							
				skb_new->dev = dev;
				
				/* Make room */
				skb_put(skb_new, (pkt_len-4));		    
				
				eth_copy_and_sum(skb_new, skb->data, pkt_len-4, 0);
				
				skb_new->protocol = eth_type_trans(skb_new, dev);
				
				/* pass the packet to upper layers */
				netif_rx(skb_new);
				
				dev->last_rx = jiffies;
				lp->stats.rx_packets++;
				lp->stats.rx_bytes += (pkt_len-4);
				
				if (IS_RCV_MP(devcs))
					lp->stats.multicast++;
			}
			else {
				//ERR("no memory, dropping rx packet.\n");
				lp->stats.rx_errors++;				
				lp->stats.rx_dropped++;				
			}
			
		}				
		else  { // Not O.K!
			/* This should only happen if we enable accepting broken packets */
			lp->stats.rx_errors++;
			lp->stats.rx_dropped++;
			
			/* added statistics counters */
			if (IS_RCV_CRC_ERR(devcs)) {
				DBG(2, "RX CRC error\n");
				lp->stats.rx_crc_errors++;
			}
			else if (IS_RCV_LOR_ERR(devcs)) {
				DBG(2, "RX LOR error\n");
				lp->stats.rx_length_errors++;
			}
			else if (IS_RCV_LE_ERR(devcs)) {
				DBG(2, "RX LE error\n");
				lp->stats.rx_length_errors++;
			}
			else if (IS_RCV_OVR_ERR(devcs)) {
				/*
				 * The overflow errors are handled through
				 * an interrupt handler.
				 */
				
				lp->stats.rx_over_errors++;
			}
			else if (IS_RCV_CV_ERR(devcs)) {
				/* code violation */
				DBG(2, "RX CV error\n");
				lp->stats.rx_errors++;					
			}
			else if (IS_RCV_CES_ERR(devcs)) {
				DBG(2, "RX Preamble error\n");
				lp->stats.rx_errors++;					
			}
		}
		
		/* Restore current descriptor */
		rd->devcs = 0;
		rd->curr_addr = CPHYSADDR(skb->data);
		rd->cmdstat = DMA_COUNT(RC32355_RBSIZE) |DMADESC_COD |DMADESC_IOD;
		
		lp->rd_ring[(lp->rx_next_done-1)& RC32355_RDS_MASK].cmdstat &=  ~(DMADESC_COD); 	
		
		lp->rx_next_done = (lp->rx_next_done + 1) & RC32355_RDS_MASK;
		rd = &lp->rd_ring[lp->rx_next_done];
		
	}	
	
        dmas = local_readl(&lp->rx_dma_regs->dmas);
        if(dmas & DMAS_H) {
		rc32355_start_rx(lp,rd);
	}
        local_writel(~dmas, &lp->rx_dma_regs->dmas);
    	/* Enable D bit in Rx DMA */
  	local_writel(local_readl(&lp->rx_dma_regs->dmasm) & ~(DMAS_D | DMAS_E | DMAS_H), &lp->rx_dma_regs->dmasm); 
   	spin_unlock_irqrestore(&lp->lock, flags);
}



/* Ethernet Tx DMA interrupt */
static irqreturn_t
rc32355_tx_dma_interrupt(int irq, void *dev_id, struct pt_regs * regs)
{
  	struct net_device	*dev = (struct net_device *)dev_id;
  	struct rc32355_local	*lp;
  	volatile u32			dmas;
	irqreturn_t retval = IRQ_NONE;
	
	ASSERT(dev != NULL);
   	lp = (struct rc32355_local *)dev->priv;
	
  	spin_lock(&lp->lock);
	/* Mask F & E bit in Tx DMA */
	local_writel(local_readl(&lp->tx_dma_regs->dmasm) | DMAS_F |DMAS_E, &lp->tx_dma_regs->dmasm);
	
	
  	dmas = local_readl(&lp->tx_dma_regs->dmas);
  	if (dmas & DMAS_F){
		tasklet_hi_schedule(lp->tx_tasklet);
	}
  	if (dmas & DMAS_E)
    		ERR(__FUNCTION__ ": DMA error\n");
	
	
  	local_writel(~dmas, &lp->tx_dma_regs->dmas);
	
   	if(lp->tx_chain_status == filled && (local_readl(&(lp->tx_dma_regs->dmandptr)) == 0)) {
		local_writel(CPHYSADDR(&lp->td_ring[lp->tx_chain_head]), &(lp->tx_dma_regs->dmandptr));			
		lp->tx_chain_status = empty;
		lp->tx_chain_head = lp->tx_chain_tail;
	}
	
	retval = IRQ_HANDLED;
  	spin_unlock(&lp->lock);
	return retval;
	
}

static void rc32355_tx_tasklet(unsigned long tx_data_dev)
{
	struct net_device *dev = (struct net_device *)tx_data_dev;	
  	struct rc32355_local* lp = (struct rc32355_local *)dev->priv;
  	volatile rc32355_dma_desc_t* td = &lp->td_ring[lp->tx_next_done];
    	u32			dmas, devcs;
    	unsigned long 	flags;
	
	
	spin_lock_irqsave(&lp->lock, flags);
	
  	dmas = local_readl(&lp->tx_dma_regs->dmas);
	
  	//process all desc that are done
  	while(IS_DMA_FINISHED(td->cmdstat)) {

  		if (	lp->tx_full == 1){
			netif_wake_queue(dev);
			lp->tx_full = 0;
		}
		
		devcs = lp->td_ring[lp->tx_next_done].devcs;    
		if ((devcs & (ETHERDMA_OUT_FD | ETHERDMA_OUT_LD)) != (ETHERDMA_OUT_FD | ETHERDMA_OUT_LD)) {
			lp->stats.tx_errors++;
	      		lp->stats.tx_dropped++;				
			
	      		/* should never happen */
	      		DBG(1, __FUNCTION__ ": split tx ignored\n");
	    	} 
		else if (IS_TX_TOK(devcs)) {
			
	      		/* transmit OK */
	      		lp->stats.tx_packets++;
	    	} 
		else {
			
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
	    	if (lp->tx_skb[lp->tx_next_done] != NULL) {
			dev_kfree_skb_irq(lp->tx_skb[lp->tx_next_done]);
	      		lp->tx_skb[lp->tx_next_done] = NULL;
	    	}
		
		lp->td_ring[lp->tx_next_done].cmdstat = DMADESC_IOF;
		lp->td_ring[lp->tx_next_done].devcs = ETHERDMA_OUT_FD | ETHERDMA_OUT_LD;
		lp->td_ring[lp->tx_next_done].link = 0;
		lp->td_ring[lp->tx_next_done].curr_addr = 0;
	    	lp->tx_count --;
		
	    	/* go on to next transmission */
	    	lp->tx_next_done = (lp->tx_next_done + 1) & RC32355_TDS_MASK;
	    	td = &lp->td_ring[lp->tx_next_done];
		
  	}
	
   	spin_unlock_irqrestore(&lp->lock, flags);
	
  	/* Enable F bit in Tx DMA */
  	local_writel(local_readl(&lp->tx_dma_regs->dmasm) & ~(DMAS_F | DMAS_E), &lp->tx_dma_regs->dmasm); 
	
}	

/*
 * Get the current statistics.
 * This may be called with the device open or closed.
 */
static struct net_device_stats *
rc32355_get_stats(struct net_device *dev)
{
  	struct rc32355_local *lp = (struct rc32355_local *)dev->priv;
	
	return &lp->stats;
}


/*
 * Set or clear the multicast filter for this adaptor.
 */
static void
rc32355_multicast_list(struct net_device *dev)
{   	
	/* changed to listen to broadcasts always and to treat	*/
  	/*	   IFF bits independantly	*/
  	struct rc32355_local *lp = (struct rc32355_local *)dev->priv;
  	unsigned long flags;
  	u32 recognise = ETHERARC_AB; 			/* always accept broadcasts */
	
  	if (dev->flags & IFF_PROMISC) 				/* set promiscuous mode */
    		recognise |= ETHERARC_PRO;
	
  	if ((dev->flags & IFF_ALLMULTI) || (dev->mc_count > 15))
    		recognise |= ETHERARC_AM;		  	/* all multicast & bcast */
	
  	else if (dev->mc_count > 0) {
		DBG(2, __FUNCTION__ ": mc_count %d\n", dev->mc_count);
		
    		recognise |= ETHERARC_AM;		  	/* for the time being */
  	}
	
  	spin_lock_irqsave(&lp->lock, flags);
	
  	local_writel(recognise, &lp->eth_regs->etharc);
	
  	spin_unlock_irqrestore(&lp->lock, flags);
	
}


static void
rc32355_tx_timeout(struct net_device *dev)
{
  	struct rc32355_local *lp = (struct rc32355_local *)dev->priv;
  	unsigned long flags;
	
  	spin_lock_irqsave(&lp->lock, flags);
	printk(__FILE__" %d\n",__LINE__);
  	rc32355_restart(dev);
  	spin_unlock_irqrestore(&lp->lock, flags);
	
}


/*
 * Initialize the RC32355 ethernet controller.
 */
static int rc32355_init(struct net_device *dev)
{
	struct rc32355_local *lp = (struct rc32355_local *)dev->priv;
	int i, j;
	
	/* Disable DMA */       
	rc32355_abort_tx(dev);
	rc32355_abort_rx(dev); 
	
	/* reset ethernet logic */ 
	local_writel(0, &lp->eth_regs->ethintfc);
	while((local_readl(&lp->eth_regs->ethintfc) & ETHERINTFC_RIP))
		dev->trans_start = jiffies;		
	/* Enable Ethernet Interface */ 
	local_writel(ETHERINTFC_EN, &lp->eth_regs->ethintfc); 
	
	tasklet_disable(lp->rx_tasklet);
	tasklet_disable(lp->tx_tasklet);
	
	/* Initialize the transmit Descriptors */
	for (i = 0; i < RC32355_NUM_TDS; i++) {
    		lp->td_ring[i].cmdstat = DMADESC_IOF;
    		lp->td_ring[i].devcs = ETHERDMA_OUT_FD | ETHERDMA_OUT_LD;
    		lp->td_ring[i].curr_addr = 0;
    		lp->td_ring[i].link = 0;
    		if (lp->tx_skb[i] != NULL) {
			/* free dangling skb */
      			dev_kfree_skb_any(lp->tx_skb[i]);
      			lp->tx_skb[i] = NULL;
    		}
	}
    	lp->tx_next_done = lp->tx_chain_head = lp->tx_chain_tail = 
    		lp->tx_count = lp->tx_full = 0;
	lp->	tx_chain_status = empty;
	
	/*
	 * Initialize the receive descriptors so that they
	 * become a circular linked list, ie. let the last
	 * descriptor point to the first again.
	 */
  	for (i=0; i<RC32355_NUM_RDS; i++) {
		struct sk_buff *skb = lp->rx_skb[i];
		
		if (lp->rx_skb[i] == NULL) {
			skb = dev_alloc_skb(RC32355_RBSIZE + 2);
			if (skb == NULL) {
				//ERR("No memory in the system\n");
				for (j = 0; j < RC32355_NUM_RDS; j ++)
					if (lp->rx_skb[j] != NULL) 
						dev_kfree_skb_any(lp->rx_skb[j]);
				
				return 1;
			}
			else
			{
				skb->dev = dev;
				lp->rx_skb[i] = skb;
			}
		}
		lp->rd_ring[i].cmdstat =	DMADESC_IOD | DMA_COUNT(RC32355_RBSIZE);
		lp->rd_ring[i].devcs = 0;
		lp->rd_ring[i].curr_addr = CPHYSADDR(skb->data);
		lp->rd_ring[i].link = CPHYSADDR(&lp->rd_ring[i+1]);
  	}
    	/* loop back */
  	lp->rd_ring[RC32355_NUM_RDS-1].link = CPHYSADDR(&lp->rd_ring[0]);
    	lp->rx_next_done   = 0;
	
#ifdef RX_DMA_CHAIN				
  	lp->rd_ring[RC32355_NUM_RDS-1].cmdstat |= DMADESC_COD;
  	lp->rx_chain_head = 0;
  	lp->rx_chain_tail = 0;
  	lp->rx_chain_status = empty;
#endif
	
	local_writel(0, &lp->rx_dma_regs->dmas);
	/* Start Rx DMA */
	rc32355_start_rx(lp, &lp->rd_ring[0]);
	
  	/* Enable F bit in Tx DMA */
  	local_writel(local_readl(&lp->tx_dma_regs->dmasm) & ~(DMAS_F | DMAS_E), &lp->tx_dma_regs->dmasm); 
    	/* Enable D bit in Rx DMA */
  	local_writel(local_readl(&lp->rx_dma_regs->dmasm) & ~(DMAS_D | DMAS_E), &lp->rx_dma_regs->dmasm); 
	
	
	/* Accept only packets destined for this Ethernet device address */
	local_writel(ETHERARC_AB, &lp->eth_regs->etharc); 
	
	/* Set all Ether station address registers to their initial values */ 
	local_writel(STATION_ADDRESS_LOW(dev), &lp->eth_regs->ethsal0); 
	local_writel(STATION_ADDRESS_HIGH(dev), &lp->eth_regs->ethsah0);
	
	local_writel(STATION_ADDRESS_LOW(dev), &lp->eth_regs->ethsal1); 
	local_writel(STATION_ADDRESS_HIGH(dev), &lp->eth_regs->ethsah1);
	
	local_writel(STATION_ADDRESS_LOW(dev), &lp->eth_regs->ethsal2); 
	local_writel(STATION_ADDRESS_HIGH(dev), &lp->eth_regs->ethsah2);
	
	local_writel(STATION_ADDRESS_LOW(dev), &lp->eth_regs->ethsal3); 
	local_writel(STATION_ADDRESS_HIGH(dev), &lp->eth_regs->ethsah3); 
	
	/* Frame Length Checking, Pad Enable, CRC Enable, Full Duplex set */ 
	local_writel( ETHERMAC2_PE | ETHERMAC2_CEN | ETHERMAC2_FD,
		&lp->eth_regs->ethmac2);  
	//ETHERMAC2_FLC	lp->duplex_mode	ETHERMAC2_FD, 
	
	/* Back to back inter-packet-gap */ 
	local_writel(0x15, &lp->eth_regs->ethipgt); 
	/* Non - Back to back inter-packet-gap */ 
	local_writel(0x12, &lp->eth_regs->ethipgr); 
	
	/* Management Clock Prescaler Divisor */
	local_writel(((idt_cpu_freq)/MII_CLOCK+1) & ~1, &lp->eth_regs->ethmcp);	
	
	/* don't transmit until fifo contains 48b */
	local_writel(48, &lp->eth_regs->ethfifott);
	
	local_writel(ETHERMAC1_RE, &lp->eth_regs->ethmac1);
	
	tasklet_enable(lp->rx_tasklet);
	tasklet_enable(lp->tx_tasklet);
	
	netif_start_queue(dev);
	
	/* Enable Ethernet Interface */ 
	local_writel(ETHERINTFC_EN, &lp->eth_regs->ethintfc); 
	
	
	return 0; 
	
}


static void rc32355_cleanup_module(void)
{
  	int i;
	
  	for (i = 0; rc32355_iflist[i].iobase; i++) {
		struct rc32355_if_t * bif = &rc32355_iflist[i];
    		if (bif->dev != NULL) {
			struct rc32355_local *lp = (struct rc32355_local *)bif->dev->priv;
      			if (lp != NULL) {
				if (lp->eth_regs)
					iounmap((void*)lp->eth_regs);
				if (lp->rx_dma_regs)
					iounmap((void*)lp->rx_dma_regs);
				if (lp->tx_dma_regs)
					iounmap((void*)lp->tx_dma_regs);
				if (lp->td_ring)
	  				kfree((void*)KSEG0ADDR(lp->td_ring));
				
#ifdef RC32355_PROC_DEBUG
				if (lp->ps)
	  				remove_proc_entry("net/rc32355", NULL);
#endif
				kfree(lp);
      			}
			
      			unregister_netdev(bif->dev);
			free_netdev(bif->dev);
     			kfree(bif->dev);
		}
  	}
}


#ifndef MODULE

static int __init rc32355_setup(char *options)
{
  	/* no options yet */
  	return 1;
}

static int __init rc32355_setup_ethaddr(char *options)
{
  	memcpy(mac, options, 17);
  	mac[17]= '\0';
  	return 1;
}

__setup("rc3255eth=", rc32355_setup);

__setup("ethaddr=", rc32355_setup_ethaddr);

#endif /* !MODULE */

module_init(rc32355_init_module);
module_exit(rc32355_cleanup_module);


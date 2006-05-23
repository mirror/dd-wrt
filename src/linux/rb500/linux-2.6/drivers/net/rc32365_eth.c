/**************************************************************************
 *
 *  BRIEF MODULE DESCRIPTION
 *     Driver for the IDT RC32365 on-chip ethernet controller.
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
 *  Based on the driver written by B. Maruthanayakam, H. Kou and others.
 *
 *
 **************************************************************************
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/moduleparam.h>
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

#include "rc32365_eth.h"

#define CONFIG_IDT_NUM_ETH_PORTS 2

#if CONFIG_IDT_NUM_ETH_PORTS > 2
#error "Only 2 ports are available."
#endif

#define DRIVER_VERSION 

#define DRIVER_NAME "rc32365 Ethernet driver" DRIVER_VERSION


#define STATION_ADDRESS_HIGH(dev) (((dev)->dev_addr[0] << 8) | \
				   ((dev)->dev_addr[1]))
#define STATION_ADDRESS_LOW(dev)  (((dev)->dev_addr[2] << 24) | \
				   ((dev)->dev_addr[3] << 16) | \
				   ((dev)->dev_addr[4] << 8)  | \
				   ((dev)->dev_addr[5]))

#define MII_CLOCK 1250000 			/* no more than 2.5MHz */
static char mac0[18] = "08:00:06:05:40:01"; 
static char mac1[18] = "08:00:06:05:50:01"; 

MODULE_AUTHOR ("IDT Inc");
MODULE_DESCRIPTION ("rc32365 Ethernet driver");
MODULE_LICENSE("GPL");

MODULE_PARM(mac0, "c18");
MODULE_PARM_DESC(mac0, "MAC address for RC32365 ethernet port 0");
MODULE_PARM(mac1, "c18");
MODULE_PARM_DESC(mac1, "MAC address for RC32365 ethernet port 1");

static void rc32365_remove(int);

static struct rc32365_if_t 
{
	struct net_device *dev;
	char *name;
	char* mac_str;
	u32 iobase;
	u32 rxdmabase;
	u32 txdmabase;
	int rx_dma_irq;
	int tx_dma_irq;
	int rx_ovr_irq;
	int tx_und_irq;
	u32 ipabmc;
} rc32365_iflist[] = 
{
	{
		NULL,
		"rc32365_eth0",
		mac0, 
		ETH0_PhysicalAddress, 
		ETH0_RX_DMA_ADDR,
		ETH0_TX_DMA_ADDR,
		ETH0_DMA_RX_IRQ, 
		ETH0_DMA_TX_IRQ, 
		ETH0_RX_OVR_IRQ, 
		ETH0_TX_UND_IRQ,
		ETH0_IPABMC_PhysicalAddress
	},
	{
		NULL, 
		"rc32365_eth1", 
		mac1, 
		ETH1_PhysicalAddress, 
		ETH1_RX_DMA_ADDR,
		ETH1_TX_DMA_ADDR,
		ETH1_DMA_RX_IRQ, 
		ETH1_DMA_TX_IRQ, 
		ETH1_RX_OVR_IRQ, 
		ETH1_TX_UND_IRQ,
		ETH1_IPABMC_PhysicalAddress
	}
};

static int parse_mac_addr(struct net_device *dev, char* macstr)
{
	int i, j;
	unsigned char result, value;
	
	for (i=0; i<6; i++) {
		result = 0;
		if (i != 5 && *(macstr+2) != ':') {
			ERR("invalid mac address format: %d %c\n", i, *(macstr+2));
			return -EINVAL;
		}
		
		for (j=0; j<2; j++) {
			if (isxdigit(*macstr) && (value = isdigit(*macstr) ? 
						  *macstr-'0' : toupper(*macstr)-'A'+10) < 16) {
				result = result*16 + value;
				macstr++;
			} 
			else {
				ERR("invalid mac address character: %c\n", *macstr);
				return -EINVAL;
			}
		}
		
		macstr++;
		dev->dev_addr[i] = result;
	}
	
	return 0;
}

static inline void rc32365_abort_tx(struct net_device *dev)
{
	struct rc32365_local *lp = (struct rc32365_local *)dev->priv;
	rc32365_abort_dma(dev, lp->tx_dma_regs,&lp->ipabmc->ipabmctx);
}

static inline void rc32365_abort_rx(struct net_device *dev)
{
	struct rc32365_local *lp = (struct rc32365_local *)dev->priv;
	rc32365_abort_dma(dev, lp->rx_dma_regs,&lp->ipabmc->ipabmcrx);
}

static inline void rc32365_start_tx(struct rc32365_local *lp, volatile DMAD_t td)
{
	rc32365_start_dma(lp->tx_dma_regs, CPHYSADDR(td));
}
static inline void rc32365_start_rx(struct rc32365_local *lp, volatile DMAD_t rd)
{
	rc32365_start_dma(lp->rx_dma_regs, CPHYSADDR(rd));
}

static inline void rc32365_chain_tx(struct rc32365_local *lp, volatile DMAD_t td)
{
	rc32365_chain_dma(lp->tx_dma_regs, CPHYSADDR(td));
}
static inline void rc32365_chain_rx(struct rc32365_local *lp, volatile DMAD_t rd)
{
	rc32365_chain_dma(lp->rx_dma_regs, CPHYSADDR(rd));
}

#ifdef RC32365_PROC_DEBUG
static int rc32365_read_proc(char *buf, char **start, off_t fpos,
			     int length, int *eof, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	struct rc32365_local *lp = (struct rc32365_local *)dev->priv;
	int len = 0;
	
	/* print out header */
	
	len += sprintf(buf + len, "\n\t rc32365 Ethernet Debug\n\n");
	
	len += sprintf (buf + len,
			"DMA halt count      = %10d, DMA run count = %10d\n",
			lp->dma_halt_cnt, lp->dma_run_cnt);
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
 * Restart the RC32365 ethernet controller. 
 */
static int rc32365_restart(struct net_device *dev)
{
	struct rc32365_local *lp = (struct rc32365_local *)dev->priv;
	
	/*
	 * Disable interrupts
	 */
	disable_irq(lp->rx_irq);
	disable_irq(lp->tx_irq);
	if (cedar_za)
		disable_irq(lp->ovr_irq);
	disable_irq(lp->und_irq);
	
	/* Mask F Ebit in Tx DMA */
	local_writel(local_readl(&lp->tx_dma_regs->dmasm) | DMASM_f_m | DMASM_e_m, &lp->tx_dma_regs->dmasm);
	/* Mask D H E bit in Rx DMA */
	local_writel(local_readl(&lp->rx_dma_regs->dmasm) 
		     | DMASM_d_m |DMASM_h_m | DMASM_e_m, &lp->rx_dma_regs->dmasm);
	
	rc32365_init(dev);
	rc32365_multicast_list(dev);

	enable_irq(lp->und_irq);
	if (cedar_za)
		enable_irq(lp->ovr_irq);
	enable_irq(lp->tx_irq);
	enable_irq(lp->rx_irq);
	
	return 0;
}

static int __init rc32365_init_module(void)
{
	int i;
	for(i=0;i<CONFIG_IDT_NUM_ETH_PORTS;i++)
		if (rc32365_probe(i))
			printk(KERN_ERR DRIVER_NAME " Port 0 load failed.\n");
	return 0;
}

static int rc32365_probe(int port_num)
{
	struct rc32365_if_t *bif = &rc32365_iflist[port_num];
	struct rc32365_local *lp = NULL;
	struct net_device *dev = NULL;
	int i, retval;
	
	dev = alloc_etherdev(sizeof(struct rc32365_local));
	if(!dev){
		printk(KERN_ERR DRIVER_NAME "alloc_etherdev failed\n");
		return -ENODEV;
	}
	
	SET_MODULE_OWNER(dev);
  
	bif->dev = dev;
  
	if ((retval = parse_mac_addr(dev, bif->mac_str))) {
		printk(KERN_ERR DRIVER_NAME" MAC address parse failed\n");
		free_netdev(dev);
		bif->dev = NULL;
		return(-ENODEV);
	}
	/* Initialize the device structure. */
	lp = (struct rc32365_local *)dev->priv;
	
	lp->rx_irq = bif->rx_dma_irq;
	lp->tx_irq = bif->tx_dma_irq;
	lp->ovr_irq = bif->rx_ovr_irq;
	lp->und_irq = bif->tx_und_irq;
	
	lp->eth_regs = ioremap_nocache(bif->iobase, sizeof(*lp->eth_regs));
	
	if (!lp->eth_regs) {
		printk(KERN_ERR DRIVER_NAME" Can't remap eth registers\n");
		retval = -ENXIO;
		goto probe_err_out;
	}
	
	lp->ipabmc = ioremap_nocache(bif->ipabmc, sizeof(*lp->ipabmc));
	
	if (!lp->ipabmc){
		printk(KERN_ERR DRIVER_NAME" Cannot map ipabmc registers.\n");
		retval = -ENXIO;
		goto probe_err_out;
	}
	
	lp->rx_dma_regs = ioremap_nocache(bif->rxdmabase, sizeof(struct DMA_Chan_s));
	
	if (!lp->rx_dma_regs) {
		printk(KERN_ERR DRIVER_NAME" Can't remap Rx DMA registers\n");
		retval = -ENXIO;
		goto probe_err_out;
	}		

	lp->tx_dma_regs = ioremap_nocache(bif->txdmabase,sizeof(struct DMA_Chan_s));
	
	if (!lp->tx_dma_regs) {
		printk(KERN_ERR DRIVER_NAME" Can't remap Tx DMA registers\n");
		retval = -ENXIO;
		goto probe_err_out;
	}
	
#ifdef RC32365_PROC_DEBUG
	lp->ps = create_proc_read_entry (bif->name, 0, proc_net,
					 rc32365_read_proc, dev);
#endif
	
	lp->td_ring = (DMAD_t)kmalloc(TD_RING_SIZE + RD_RING_SIZE, GFP_KERNEL);
	if (!lp->td_ring) {
		printk(KERN_ERR DRIVER_NAME" Can't allocate descriptors\n");
		retval = -ENOMEM;
		goto probe_err_out;
	}
	dma_cache_inv((unsigned long)(lp->td_ring), TD_RING_SIZE + RD_RING_SIZE);
  
  	/* now convert TD_RING pointer to KSEG1 */
	lp->td_ring = (DMAD_t )KSEG1ADDR(lp->td_ring);
	lp->rd_ring = &lp->td_ring[RC32365_NUM_TDS];
	
	spin_lock_init(&lp->lock);
  
	/* Fill in the 'dev' fields. */
	dev->base_addr = bif->iobase;
	/* just use the rx dma irq */
	dev->irq = bif->rx_dma_irq; 
	
	dev->priv = lp;
	
	dev->open = rc32365_open;
	dev->stop = rc32365_close;
	dev->hard_start_xmit = rc32365_send_packet;
	dev->get_stats	= rc32365_get_stats;
	dev->set_multicast_list = &rc32365_multicast_list;
	dev->tx_timeout = rc32365_tx_timeout;
	dev->watchdog_timeo = RC32365_TX_TIMEOUT;
  
	lp->rx_tasklet = kmalloc(sizeof(struct tasklet_struct), GFP_KERNEL);
	if(lp->rx_tasklet == NULL){
		printk(KERN_ERR DRIVER_NAME" Cannot allocate memory for rx_tasklet\n");
		retval = -ENOMEM;
		goto probe_err_out;
	}
	lp->tx_tasklet = kmalloc(sizeof(struct tasklet_struct), GFP_KERNEL);
	if(lp->tx_tasklet == NULL){
		printk(KERN_ERR DRIVER_NAME" Cannot allocate memory for tx_tasklet\n");
		retval = -ENOMEM;
		goto probe_err_out;
	}

	tasklet_init(lp->rx_tasklet, rc32365_rx_tasklet, (unsigned long)dev);
	tasklet_init(lp->tx_tasklet, rc32365_tx_tasklet, (unsigned long)dev);
	
	if (register_netdev(dev)) {
		printk(KERN_ERR DRIVER_NAME" Cannot register_netdev\n");
		retval = -EINVAL;
		goto probe_err_out;
	}

	INFO("Rx IRQ %d, Tx IRQ %d, ", bif->rx_dma_irq, bif->tx_dma_irq);
	for (i = 0; i < 6; i++) {
		printk("%2.2x", dev->dev_addr[i]);
		if (i<5)
			printk(":");
	}
	printk("\n");
	return 0;
	
 probe_err_out:
	rc32365_remove(port_num);
	printk(KERN_ERR DRIVER_NAME " failed.  Returns %d\n", retval);
	return retval;
}

/*
 * Open/initialize the RC32365 Ethernet device.
 *
 * This routine should set everything up new at each open, even
 *  registers that "should" only need to be set once at boot, so that
 *  there is non-reboot way to recover if something goes wrong.
 */
static int rc32365_open(struct net_device *dev)
{
  struct rc32365_local *lp = (struct rc32365_local *)dev->priv;
  
  /*
   * Initialize
   */
  if (rc32365_init(dev)) 
    {
      ERR("Erroe: cannot open the Ethernet device\n");
      return -EAGAIN;
    }

  if (request_irq(lp->rx_irq, &rc32365_rx_dma_interrupt,
		  SA_SHIRQ | SA_INTERRUPT,
		  "rc32365 ethernet Rx", dev)) 
    {
      INFO("unable to get Rx DMA IRQ %d\n", lp->rx_irq);
      return -EAGAIN;
    }

  if (request_irq(lp->tx_irq, &rc32365_tx_dma_interrupt,
		  SA_SHIRQ | SA_INTERRUPT,
		  "rc32365 ethernet Tx", dev)) 
    {
      INFO("unable to get Tx DMA IRQ %d\n", lp->tx_irq);
      free_irq(lp->rx_irq, dev);
      return -EAGAIN;
    }

  /* Install handler for overrun error. */
  if (cedar_za)
    if (request_irq(lp->ovr_irq, &rc32365_ovr_interrupt,
		    SA_SHIRQ | SA_INTERRUPT,
		    "Ethernet Overflow", dev)) 
      {
        INFO("unable to get OVR IRQ %d\n",	lp->ovr_irq);
        free_irq(lp->rx_irq, dev);
        free_irq(lp->tx_irq, dev);
        return -EAGAIN;
      }

  /* Install handler for underflow error. */
  if (request_irq(lp->und_irq, &rc32365_und_interrupt,
		  SA_SHIRQ | SA_INTERRUPT,
		  "Ethernet Underflow", dev)) 
    {
      INFO("unable to get UND IRQ %d\n",	lp->und_irq);
      free_irq(lp->rx_irq, dev);
      free_irq(lp->tx_irq, dev);
      if (cedar_za)
        free_irq(lp->ovr_irq, dev);		
      return -EAGAIN;
    }

  /*
    init_timer(&lp->mii_phy_timer);
    lp->mii_phy_timer.expires = jiffies + 10 * HZ;	
    lp->mii_phy_timer.data = (unsigned long)dev;
    lp->mii_phy_timer.function	 = rc32365_mii_handler;
    add_timer(&lp->mii_phy_timer);
  */
  
  return 0;
}

/*
 * Close the RC32365 ethernet device.
 */
static int rc32365_close(struct net_device *dev)
{
  struct rc32365_local *lp = (struct rc32365_local *)dev->priv;
  u32 tmp;
  
  /*
   * Disable interrupts
   */
  disable_irq(lp->rx_irq);
  disable_irq(lp->tx_irq);
  if (cedar_za)
    disable_irq(lp->ovr_irq);
  disable_irq(lp->und_irq);
  
  tmp = local_readl(&lp->tx_dma_regs->dmasm);
  tmp = tmp | DMASM_f_m | DMASM_e_m;
  local_writel(tmp, &lp->tx_dma_regs->dmasm);

  tmp = local_readl(&lp->rx_dma_regs->dmasm);
  tmp = tmp | DMASM_d_m | DMASM_h_m | DMASM_e_m;
  local_writel(tmp, &lp->rx_dma_regs->dmasm);
  
  free_irq(lp->rx_irq, dev);
  free_irq(lp->tx_irq, dev);
  if (cedar_za)
    free_irq(lp->ovr_irq, dev);
  free_irq(lp->und_irq, dev);

  //Not enabled this feature at this time.
  //del_timer(&lp->mii_phy_timer);
  return 0;
}


/* transmit packet */
static int rc32365_send_packet(struct sk_buff *skb, struct net_device *dev)
{
  struct rc32365_local	*lp = (struct rc32365_local *)dev->priv;
  unsigned long 	flags;
  u32			length;
  DMAD_t		td;

  spin_lock_irqsave(&lp->lock, flags);

  td = &lp->td_ring[lp->tx_chain_tail];

  /* stop queue when full, drop pkts if queue already full */
  if(lp->tx_count >= (RC32365_NUM_TDS - 2))
    {
      lp->tx_full = 1;
      
      if(lp->tx_count == (RC32365_NUM_TDS - 2))
	{
	  /* this pkt is about to fill the queue */
	  //	  ERR("Tx Ring now full, queue stopped.\n");
	  netif_stop_queue(dev);
	}
      else{
	/* this pkt cannot be added to the full queue */
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

  lp->tx_skb[lp->tx_chain_tail] = skb;
  
  length = skb->len;

  /* Setup the transmit descriptor. */
  td->ca = CPHYSADDR(skb->data);

  if(local_readl(&(lp->tx_dma_regs->dmandptr)) == 0) 		
    {
      if( lp->tx_chain_status == empty ) 
	{
	  td->control = DMA_COUNT(length) |DMAD_cof_m |DMAD_iof_m;                                /*  Update tail      */
	  lp->tx_chain_tail = (lp->tx_chain_tail + 1) & RC32365_TDS_MASK;                           /*   Move tail       */
	  local_writel(CPHYSADDR(&lp->td_ring[lp->tx_chain_head]), &(lp->tx_dma_regs->dmandptr)); /* Write to NDPTR    */
	  lp->tx_chain_head = lp->tx_chain_tail;                                                  /* Move head to tail */
	}
      else
	{
	  td->control = DMA_COUNT(length) |DMAD_cof_m|DMAD_iof_m;                                 /* Update tail */
	  lp->td_ring[(lp->tx_chain_tail-1)& RC32365_TDS_MASK].control &=  ~(DMAD_cof_m);           /* Link to prev */
	  lp->td_ring[(lp->tx_chain_tail-1)& RC32365_TDS_MASK].link =  CPHYSADDR(td);               /* Link to prev */
	  lp->tx_chain_tail = (lp->tx_chain_tail + 1) & RC32365_TDS_MASK;                           /* Move tail */
	  local_writel(CPHYSADDR(&lp->td_ring[lp->tx_chain_head]), &(lp->tx_dma_regs->dmandptr)); /* Write to NDPTR */
	  lp->tx_chain_head = lp->tx_chain_tail;                                                  /* Move head to tail */
	  lp->tx_chain_status = empty;  
	}
    }
  else
    {
      if( lp->tx_chain_status == empty )
	{
	  td->control = DMA_COUNT(length) |DMAD_cof_m |DMAD_iof_m;                                /* Update tail */
	  lp->tx_chain_tail = (lp->tx_chain_tail + 1) & RC32365_TDS_MASK;                           /* Move tail */
	  lp->tx_chain_status = filled;		
	}
      else
	{
	  td->control = DMA_COUNT(length) |DMAD_cof_m |DMAD_iof_m;                                /* Update tail */
	  lp->td_ring[(lp->tx_chain_tail-1)& RC32365_TDS_MASK].control &=  ~(DMAD_cof_m);           /* Link to prev */
	  lp->td_ring[(lp->tx_chain_tail-1)& RC32365_TDS_MASK].link =  CPHYSADDR(td);	          /* Link to prev */
	  lp->tx_chain_tail = (lp->tx_chain_tail + 1) & RC32365_TDS_MASK;                           /* Move tail */
	}		
    }

  dev->trans_start = jiffies;
  
  spin_unlock_irqrestore(&lp->lock, flags);
  
  return 0;
}


/* Ethernet MII-PHY Handler */
static void rc32365_mii_handler(unsigned long data)
{
  struct net_device *dev = (struct net_device *)data;		
  struct rc32365_local *lp = (struct rc32365_local *)dev->priv;
  unsigned long 	flags;
  unsigned long duplex_status;
  int port_addr = (lp->rx_irq == 0x2a? 1:0) << 8;

  spin_lock_irqsave(&lp->lock, flags);

  /* Two ports are using the same MII, the difference is the PHY address */
  local_writel(0, &rc32365_eth0_regs->miimcfg);  
  local_writel(0, &rc32365_eth0_regs->miimcmd);  
  local_writel(port_addr |0x05, &rc32365_eth0_regs->miimaddr);  
  local_writel(MIIMCMD_scn_m, &rc32365_eth0_regs->miimcmd);  
  while(local_readl(&rc32365_eth0_regs->miimind) & MIIMIND_nv_m);

  ERR("irq:%x		port_addr:%x	RDD:%x\n", 
      lp->rx_irq, port_addr, local_readl(&rc32365_eth0_regs->miimrdd));
  duplex_status = (local_readl(&rc32365_eth0_regs->miimrdd) & 0x140)? ETHMAC2_fd_m: 0;
  if(duplex_status != lp->duplex_mode)
    {
      ERR("The MII-PHY is Auto-negotiated to %s-Duplex mode for Eth-%x\n", 
	  duplex_status? "Full":"Half", lp->rx_irq == 0x2a? 1:0);		
      lp->duplex_mode = duplex_status;
      rc32365_restart(dev);		
    }

  lp->mii_phy_timer.expires = jiffies + 10 * HZ;	
  add_timer(&lp->mii_phy_timer);
  
  spin_unlock_irqrestore(&lp->lock, flags);

}

/* Ethernet Rx Overflow interrupt */
static irqreturn_t rc32365_ovr_interrupt(int irq, void *dev_id, struct pt_regs * regs)
{
  struct net_device *dev = (struct net_device *)dev_id;
  struct rc32365_local *lp;
  unsigned int ovr;
  irqreturn_t retval = IRQ_NONE;
	
  ASSERT(dev != NULL);
  
  lp = (struct rc32365_local *)dev->priv;
  spin_lock(&lp->lock);
  ovr = local_readl(&lp->eth_regs->ethintfc);

  if(ovr & ETHINTFC_ovr_m)
    {
      netif_stop_queue(dev);

      /* clear OVR bit */
      local_writel((ovr & ~ETHINTFC_ovr_m), &lp->eth_regs->ethintfc);

      /* Restart interface */
      rc32365_restart(dev);	  
      retval = IRQ_HANDLED;
    }
  spin_unlock(&lp->lock);

  return retval;
}

/* Ethernet Tx Underflow interrupt */
static irqreturn_t rc32365_und_interrupt(int irq, void *dev_id, struct pt_regs * regs)
{
  struct net_device *dev = (struct net_device *)dev_id;
  struct rc32365_local *lp;
  unsigned int und;
  irqreturn_t retval = IRQ_NONE;
	
  ASSERT(dev != NULL);
	
  lp = (struct rc32365_local *)dev->priv;
  
  spin_lock(&lp->lock);

  und = local_readl(&lp->eth_regs->ethintfc);

  if(und & ETHINTFC_und_m)
    {
      netif_stop_queue(dev);	
      
      local_writel((und & ~ETHINTFC_und_m), &lp->eth_regs->ethintfc);

      /* Restart interface */
      rc32365_restart(dev);    
      retval = IRQ_HANDLED;
    }
	
  spin_unlock(&lp->lock);
	
  return retval;

}


/* Ethernet Rx DMA interrupt */
static irqreturn_t
rc32365_rx_dma_interrupt(int irq, void *dev_id, struct pt_regs * regs)
{
  struct net_device *dev = (struct net_device *)dev_id;
  struct rc32365_local* lp;
  volatile u32 dmas,dmasm;
  irqreturn_t retval = IRQ_NONE;

  ASSERT(dev != NULL);

  lp = (struct rc32365_local *)dev->priv;

  spin_lock(&lp->lock);
  dmas = local_readl(&lp->rx_dma_regs->dmas);
  if(dmas & (DMAS_d_m|DMAS_h_m|DMAS_e_m))
    { 
      if (cedar_za)
        local_writel(~dmas, &lp->rx_dma_regs->dmas);

      /* Mask D H E bit in Rx DMA */
      dmasm = local_readl(&lp->rx_dma_regs->dmasm);
      local_writel(dmasm | (DMASM_d_m | DMASM_h_m | DMASM_e_m), &lp->rx_dma_regs->dmasm);
      tasklet_hi_schedule(lp->rx_tasklet);

      if (dmas & DMAS_e_m)
	ERR(": DMA error\n");

      retval = IRQ_HANDLED;
    }

  spin_unlock(&lp->lock);
  return retval;
}

static void rc32365_rx_tasklet(unsigned long rx_data_dev)
{
  struct net_device *dev = (struct net_device *)rx_data_dev;	
  struct rc32365_local* lp = (struct rc32365_local *)dev->priv;
  volatile DMAD_t  rd = &lp->rd_ring[lp->rx_next_done];
  struct sk_buff *skb, *skb_new;
  u8* pkt_buf;
  volatile u32 devcs;
  volatile u32 dmas;
  volatile u32 dmasm;
  volatile u32 count;
  volatile u32 pkt_len;
  unsigned long 	flags;

  spin_lock_irqsave(&lp->lock, flags);
  /* keep going while we have received into more descriptors */
  while ( (count = RC32365_RBSIZE - (u32)DMA_COUNT(rd->control)) != 0)
    {
      /* init the var. used for the operations within the while loop later */
      skb_new = NULL;
      devcs = rd->devcs;
      pkt_len = RCVPKT_LENGTH(devcs);
      skb = lp->rx_skb[lp->rx_next_done];

      if( count != pkt_len) 
	{	
	  lp->stats.rx_errors++;
	  lp->stats.rx_dropped++;
	}
      else if (count < 64) 
	{
	  lp->stats.rx_errors++;
	  lp->stats.rx_dropped++;
	}
      else if ((devcs & ( ETHRX_ld_m)) !=	ETHRX_ld_m)
	{
	  lp->stats.rx_errors++; 
	}
      else if (devcs & ETHRX_rok_m) 
	{				
	  pkt_buf = (u8*)lp->rx_skb[lp->rx_next_done]->data;
	      
	  /* invalidate the cache before copying the buffer */
	  dma_cache_inv((unsigned long)pkt_buf, (pkt_len-4));
	      
	  /* Malloc up new buffer. */					  
	  skb_new = dev_alloc_skb(RC32365_RBSIZE + 2);

	  if (skb_new != NULL)
	    {
	      /* Make room */
	      skb_put(skb, (pkt_len-4));		    
	      
	      skb->protocol = eth_type_trans(skb, dev);
	      /* pass the packet to upper layers */
	      netif_rx(skb);
	      dev->last_rx = jiffies;
	      lp->stats.rx_packets++;
	      lp->stats.rx_bytes += (pkt_len-4);
	      
	      if (IS_RCV_MP(devcs))
		lp->stats.multicast++;

	      /* 16 bit align */
	      skb_reserve(skb_new, 2);	

	      skb_new->dev = dev;
	      lp->rx_skb[lp->rx_next_done] = skb_new;
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
	  
	  /* added statistics counters */
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
	      lp->stats.rx_errors++;					
	    }
	  else if (IS_RCV_CES_ERR(devcs)) 
	    {
	      DBG(2, "RX Preamble error\n");
	      lp->stats.rx_errors++;					
	    }
	}
      
      /*
       * clear the bits that let us see whether this
       * descriptor has been used or not & reset reception
       * length.
       */		 
      rd->devcs = 0;
      /* restore descriptor's curr_addr */
      if (skb_new)
	rd->ca = CPHYSADDR(skb_new->data); 
      else
	rd->ca = CPHYSADDR(skb->data);

      rd->control = DMA_COUNT(RC32365_RBSIZE) |DMAD_cod_m |DMAD_iod_m;

      lp->rd_ring[(lp->rx_next_done-1)& RC32365_RDS_MASK].control &=  ~(DMAD_cod_m); 	

      lp->rx_next_done = (lp->rx_next_done + 1) & RC32365_RDS_MASK;
      rd = &lp->rd_ring[lp->rx_next_done];

    }	
  if (!cedar_za) {
  dmas = local_readl(&lp->rx_dma_regs->dmas);
  if(dmas & DMAS_h_m)
    {
#ifdef RC32365_PROC_DEBUG
      lp->dma_halt_cnt++;
#endif
      rd->devcs = 0;
      skb = lp->rx_skb[lp->rx_next_done];
      rd->ca = CPHYSADDR(skb->data);
      rc32365_chain_rx(lp,rd);
    }
  }
  local_writel(~dmas, &lp->rx_dma_regs->dmas);
  /* Enable D H E bit in Rx DMA */
  dmasm = local_readl(&lp->rx_dma_regs->dmasm);
  local_writel(dmasm & ~(DMASM_d_m | DMASM_h_m | DMASM_e_m), &lp->rx_dma_regs->dmasm);
  spin_unlock_irqrestore(&lp->lock, flags);
}



/* Ethernet Tx DMA interrupt */
static irqreturn_t
rc32365_tx_dma_interrupt(int irq, void *dev_id, struct pt_regs * regs)
{
  struct net_device *dev = (struct net_device *)dev_id;
  struct rc32365_local *lp;
  volatile u32 dmas,dmasm;
  irqreturn_t retval = IRQ_NONE;

  ASSERT(dev != NULL);

  lp = (struct rc32365_local *)dev->priv;

  spin_lock(&lp->lock);

  dmas = local_readl(&lp->tx_dma_regs->dmas);

  if (dmas & (DMAS_f_m | DMAS_e_m))
    {
      dmasm = local_readl(&lp->tx_dma_regs->dmasm);
      /* Mask F E bit in Tx DMA */
      local_writel(dmasm | (DMASM_f_m | DMASM_e_m), &lp->tx_dma_regs->dmasm);

      tasklet_hi_schedule(lp->tx_tasklet);

      if(lp->tx_chain_status == filled && (local_readl(&(lp->tx_dma_regs->dmandptr)) == 0))
	{ 
	  local_writel(CPHYSADDR(&lp->td_ring[lp->tx_chain_head]), &(lp->tx_dma_regs->dmandptr));
	  lp->tx_chain_status = empty;
	  lp->tx_chain_head = lp->tx_chain_tail;
	  dev->trans_start = jiffies;
	}

      if (dmas & DMAS_e_m)
	ERR(": DMA error\n");

      retval = IRQ_HANDLED;
    }

  spin_unlock(&lp->lock);

  return retval;
}


static void rc32365_tx_tasklet(unsigned long tx_data_dev)
{
  struct net_device *dev = (struct net_device *)tx_data_dev;	
  struct rc32365_local* lp = (struct rc32365_local *)dev->priv;
  volatile DMAD_t td = &lp->td_ring[lp->tx_next_done];
  volatile u32	devcs;
  unsigned long 	flags;
  volatile u32 dmasm;

  spin_lock_irqsave(&lp->lock, flags);

  /* process all desc that are done */
  while(IS_DMA_FINISHED(td->control)) 
    {
      if(lp->tx_full == 1)
	{
	  netif_wake_queue(dev);
	  lp->tx_full = 0;
	}

      devcs = lp->td_ring[lp->tx_next_done].devcs;    
      if ((devcs & (ETHTX_fd_m | ETHTX_ld_m)) != (ETHTX_fd_m | ETHTX_ld_m)) 
	{
	  lp->stats.tx_errors++;
	  lp->stats.tx_dropped++;				
	  
	  /* should never happen */
	  INFO("split tx ignored\n");
	} 
      else if (IS_TX_TOK(devcs)) 
	{
	  /* transmit OK */
	  lp->stats.tx_packets++;
	} 
      else 
	{
	  
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
      if (lp->tx_skb[lp->tx_next_done] != NULL) 
	{
	  dev_kfree_skb_any(lp->tx_skb[lp->tx_next_done]);
	  lp->tx_skb[lp->tx_next_done] = NULL;
	}

      lp->td_ring[lp->tx_next_done].control = DMAD_iof_m;
      lp->td_ring[lp->tx_next_done].devcs = ETHTX_fd_m | ETHTX_ld_m;	
      lp->td_ring[lp->tx_next_done].link = 0;
      lp->td_ring[lp->tx_next_done].ca = 0;

      lp->tx_count--;

      /* go on to next transmission */
      lp->tx_next_done = (lp->tx_next_done + 1) & RC32365_TDS_MASK;
      td = &lp->td_ring[lp->tx_next_done];
      
    }

  /* Enable F E bit in Tx DMA */
  dmasm = local_readl(&lp->tx_dma_regs->dmasm);
  local_writel(dmasm & ~(DMASM_f_m | DMASM_e_m),&lp->tx_dma_regs->dmasm);
	
  spin_unlock_irqrestore(&lp->lock, flags);

}

/*
 * Get the current statistics.
 * This may be called with the device open or closed.
 */
static struct net_device_stats *
rc32365_get_stats(struct net_device *dev)
{
  struct rc32365_local *lp = (struct rc32365_local *)dev->priv;
  
  return &lp->stats;
}

/*
 * Set or clear the multicast filter for this adaptor.
 */
static void
rc32365_multicast_list(struct net_device *dev)
{   	
  struct rc32365_local *lp = (struct rc32365_local *)dev->priv;
  unsigned long flags;
  u32 recognise = ETHARC_ab_m; 		/* always accept broadcasts */
  
  if (dev->flags & IFF_PROMISC) 			/* set promiscuous mode */
    recognise |= ETHARC_pro_m;
  
  if ((dev->flags & IFF_ALLMULTI) || (dev->mc_count > 15))
    recognise |= ETHARC_am_m;		/* all multicast & bcast */
  
  else if (dev->mc_count > 0) 
    {
      DBG(2, __FUNCTION__ ": mc_count %d\n", dev->mc_count);
      
      recognise |= ETHARC_am_m;		/* for the time being */
    }
  
  spin_lock_irqsave(&lp->lock, flags);
  
  local_writel(recognise, &lp->eth_regs->etharc);
	
  spin_unlock_irqrestore(&lp->lock, flags);
	
}


static void
rc32365_tx_timeout(struct net_device *dev)
{
  struct rc32365_local *lp = (struct rc32365_local *)dev->priv;
  unsigned long flags;
  
  spin_lock_irqsave(&lp->lock, flags);
  dev->trans_start = jiffies;
  rc32365_restart(dev);
  spin_unlock_irqrestore(&lp->lock, flags);
	
}


/*
 * Initialize the RC32365 ethernet controller.
 */
static int rc32365_init(struct net_device *dev)
{
  struct rc32365_local *lp = (struct rc32365_local *)dev->priv;
  int i, j;

  /* Disable DMA */       
  rc32365_abort_rx(dev);	
  rc32365_abort_tx(dev);

  /* reset ethernet logic */ 
  local_writel(0, &lp->eth_regs->ethintfc);
  while((local_readl(&lp->eth_regs->ethintfc) & ETHINTFC_rip_m))
    dev->trans_start = jiffies;
	
  /* Enable Ethernet Interface */ 
  local_writel(ETHINTFC_en_m, &lp->eth_regs->ethintfc); 

  tasklet_disable(lp->rx_tasklet);
  tasklet_disable(lp->tx_tasklet);

  /* Initialize the transmit Descriptors */
  for (i = 0; i < RC32365_NUM_TDS; i++) {
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
  lp->tx_next_done = lp->tx_chain_head = lp->tx_chain_tail = 
    lp->tx_count = lp->tx_full = 0;
  lp->tx_chain_status = empty;
	
  /*
   * Initialize the receive descriptors so that they
   * become a circular linked list, ie. let the last
   * descriptor point to the first again.
   */
  for (i=0; i<RC32365_NUM_RDS; i++) 
    {
      struct sk_buff *skb = lp->rx_skb[i];

      if (lp->rx_skb[i] == NULL) 
	{
	  skb = dev_alloc_skb(RC32365_RBSIZE + 2);
	  if (skb == NULL)
	    {
	      printk("No memory in the system\n");
	      for (j = 0; j < RC32365_NUM_RDS; j ++)
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
      lp->rd_ring[i].control =	DMAD_iod_m | DMA_COUNT(RC32365_RBSIZE);
      lp->rd_ring[i].devcs = 0;
      lp->rd_ring[i].ca = CPHYSADDR(skb->data);
      lp->rd_ring[i].link = CPHYSADDR(&lp->rd_ring[i+1]);
    }
  /* loop back */
  lp->rd_ring[RC32365_NUM_RDS-1].link = CPHYSADDR(&lp->rd_ring[0]);
  lp->rx_next_done   = 0;
  lp->rd_ring[RC32365_NUM_RDS-1].control |= DMAD_cod_m;
  lp->rx_chain_head = 0;
  lp->rx_chain_tail = 0;
  lp->rx_chain_status = empty;

  local_writel(0, &lp->rx_dma_regs->dmas);
  /* Start Rx DMA */
  rc32365_start_rx(lp, &lp->rd_ring[0]);
	
  /* Enable F E bit in Tx DMA */
  local_writel(local_readl(&lp->tx_dma_regs->dmasm) & ~(DMASM_f_m | DMASM_e_m), &lp->tx_dma_regs->dmasm); 
  /* Enable D H E bit in Rx DMA */
  local_writel(local_readl(&lp->rx_dma_regs->dmasm) & ~(DMASM_d_m |DMASM_h_m| DMASM_e_m), &lp->rx_dma_regs->dmasm); 

  /* Accept only packets destined for this Ethernet device address */
  local_writel(ETHARC_ab_m, &lp->eth_regs->etharc); 
 
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
  local_writel( ETHMAC2_pe_m | ETHMAC2_cen_m | ETHMAC2_fd_m,&lp->eth_regs->ethmac2);  

  /* Back to back inter-packet-gap */ 
  local_writel(0x15, &lp->eth_regs->ethipgt); 
  /* Non - Back to back inter-packet-gap */ 
  local_writel(0x12, &lp->eth_regs->ethipgr); 
     
  /* Management Clock Prescaler Divisor */
  local_writel((idt_cpu_freq/MII_CLOCK+1) & ~1, &lp->eth_regs->ethmcp);

  /* don't transmit until fifo contains 48b */
  local_writel(48, &lp->eth_regs->ethfifott);
  tasklet_enable(lp->rx_tasklet);
  tasklet_enable(lp->tx_tasklet);

  netif_start_queue(dev);

  local_writel(ETHMAC1_re_m, &lp->eth_regs->ethmac1);

  return 0; 

}
static void rc32365_remove(int port_num)
{
	struct rc32365_if_t * bif = &rc32365_iflist[port_num];
	if (bif->dev != NULL) {
		struct rc32365_local *lp = (struct rc32365_local *)bif->dev->priv;
		if (lp != NULL) {
			if (lp->rx_tasklet) {
				tasklet_disable(lp->rx_tasklet);
				tasklet_kill(lp->rx_tasklet);
				kfree(lp->rx_tasklet);
			}
			if (lp->tx_tasklet) {
				tasklet_disable(lp->tx_tasklet);
				tasklet_kill(lp->tx_tasklet);
				kfree(lp->tx_tasklet);
			}
			if (lp->eth_regs)
				iounmap((void*)lp->eth_regs);
			if (lp->ipabmc)
				iounmap((void*)lp->ipabmc);
			if (lp->rx_dma_regs)
				iounmap((void*)lp->rx_dma_regs);
			if (lp->tx_dma_regs)
				iounmap((void*)lp->tx_dma_regs);
			if (lp->td_ring)
				kfree((void*)KSEG0ADDR(lp->td_ring));
			
#ifdef RC32365_PROC_DEBUG
			if (lp->ps)
				remove_proc_entry(bif->name, proc_net);
#endif
		}
		unregister_netdev(bif->dev);
		free_netdev(bif->dev);
		bif->dev = NULL;
	}
}
static void __exit rc32365_cleanup_module(void)
{
	int i;
	printk(KERN_ERR " Removing.\n");
	for(i=0;i<CONFIG_IDT_NUM_ETH_PORTS;i++)
		rc32365_remove(i);
	printk(KERN_INFO DRIVER_NAME " Un-loaded.\n");
}


#ifndef MODULE

static int __init rc32365_setup(char *options)
{
  /* no options yet */
  return 1;
}

static int __init rc32365_setup_ethaddr0(char *options)
{
  memcpy(mac0, options, 17);
  mac0[17]= '\0';
  return 1;
}

static int __init rc32365_setup_ethaddr1(char *options)
{
  memcpy(mac1, options, 17);
  mac1[17]= '\0';
  return 1;
}

__setup("rc32365eth=", rc32365_setup);

__setup("ethaddr0=", rc32365_setup_ethaddr0);
__setup("ethaddr1=", rc32365_setup_ethaddr1);

#endif /* !MODULE */

module_init(rc32365_init_module);
module_exit(rc32365_cleanup_module);


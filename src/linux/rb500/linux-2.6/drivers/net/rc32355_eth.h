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


#ifndef RC32355_H
#define RC32355_H

#include  <asm/idt-boards/rc32300/rc32355_eth.h> 
#include  <asm/idt-boards/rc32300/rc32355_dma.h> 
#include  <asm/idt-boards/rc32300/rc32355.h> 


#define RC32355_DEBUG	2
#define RC32355_PROC_DEBUG

#undef	RC32355_DEBUG
#if 0
#undef	RC32355_PROC_DEBUG
#endif

#ifdef RC32355_DEBUG

/* use 0 for production, 1 for verification, >2 for debug */
static int rc32355_debug = RC32355_DEBUG;

/*ASSERT*/
#define ASSERT(expr) \
	if(!(expr)) {	\
		printk( "Assertion failed! %s,%s,%s,line=%d\n",	\
		#expr,__FILE__,__FUNCTION__,__LINE__);		}

/*DBG*/
#define DBG(lvl, format, arg...) if (rc32355_debug > lvl) printk(KERN_INFO "%s: " format, dev->name , ## arg)
#define ERR(format, arg...) printk(KERN_ERR "%s: " format, dev->name , ## arg)
#define WARN(format, arg...) printk(KERN_WARNING "%s: " format, dev->name , ## arg)		

#else

#define ASSERT(expr) do {} while (0)
#define DBG(lvl, format, arg...) do {} while (0)
#define ERR(format, arg...) do {} while (0)
#define WARN(format, arg...) do {} while (0)

#endif

/* INFO */
#define INFO(format, arg...) printk(KERN_INFO "%s: " format, dev->name , ## arg)



#define ETH_DMA_RX_IRQ   GROUP1_IRQ_BASE + 9
#define ETH_DMA_TX_IRQ   GROUP1_IRQ_BASE + 10
#define ETH_RX_OVR_IRQ   GROUP3_IRQ_BASE + 22
#define ETH_TX_UND_IRQ   GROUP3_IRQ_BASE + 23

extern unsigned int idt_cpu_freq;

/* Index to functions, as function prototypes. */
static int rc32355_open(struct net_device *dev);
static int rc32355_send_packet(struct sk_buff *skb, struct net_device *dev);
static void rc32355_mii_handler(unsigned long data);
static irqreturn_t rc32355_ovr_interrupt(int irq, void *dev_id,
					 struct pt_regs * regs);
static irqreturn_t rc32355_und_interrupt(int irq, void *dev_id,
					 struct pt_regs * regs);
static irqreturn_t rc32355_rx_dma_interrupt(int irq, void *dev_id,
					    struct pt_regs * regs);
static irqreturn_t rc32355_tx_dma_interrupt(int irq, void *dev_id,
					    struct pt_regs * regs);
static void rc32355_rx(struct net_device *dev);
static void rc32355_tx(struct net_device *dev);
static int  rc32355_close(struct net_device *dev);
static struct net_device_stats *rc32355_get_stats(struct net_device *dev);
static void rc32355_multicast_list(struct net_device *dev);
static int  rc32355_init(struct net_device *dev);
static void rc32355_tx_timeout(struct net_device *dev);
static void rc32355_tx_tasklet(unsigned long tx_data_dev);
static void rc32355_rx_tasklet(unsigned long rx_data_dev);
static void rc32355_cleanup_module(void);
static int rc32355_probe(int port_num);
int rc32355_init_module(void);


/* the following must be powers of two */
#define RC32355_NUM_RDS	 128    	/* number of receive descriptors */
#define RC32355_NUM_TDS	 128    	/* number of transmit descriptors */

#define RC32355_RBSIZE	 1536  		/* size of one resource buffer = Ether MTU */
#define RC32355_RDS_MASK	 (RC32355_NUM_RDS-1)
#define RC32355_TDS_MASK	 (RC32355_NUM_TDS-1)
#define RD_RING_SIZE (RC32355_NUM_RDS * sizeof(rc32355_dma_desc_t))
#define TD_RING_SIZE (RC32355_NUM_TDS * sizeof(rc32355_dma_desc_t))

#define RC32355_TX_TIMEOUT HZ * 100

enum status	{ filled,	empty};
#define RX_DMA_CHAIN		


/* Information that need to be kept for each board. */
struct rc32355_local {
	rc32355_eth_regs_t* eth_regs;
	rc32355_dma_ch_t* rx_dma_regs;
	rc32355_dma_ch_t* tx_dma_regs;
 	volatile rc32355_dma_desc_t * td_ring;  		/* transmit descriptor ring */ 
	volatile rc32355_dma_desc_t * rd_ring;  		/* receive descriptor ring  */

	struct sk_buff* tx_skb[RC32355_NUM_TDS]; 	/* skbuffs for pkt to trans */
	struct sk_buff* rx_skb[RC32355_NUM_RDS]; 	/* skbuffs for pkt to trans */

	struct tasklet_struct * rx_tasklet;
	struct tasklet_struct * tx_tasklet;
	
	int	rx_next_done;
	int	rx_chain_head;
	int	rx_chain_tail;
	enum status	rx_chain_status;
	
	int	tx_next_done;
	int	tx_chain_head;
	int	tx_chain_tail;
	enum status	tx_chain_status;
	int	tx_count;	   						/* current # of pkts waiting to be sent */
	int	tx_full;
	
	struct timer_list    mii_phy_timer;
	unsigned long duplex_mode;
	
	int	rx_irq;
	int	tx_irq;
	int  	ovr_irq;
	int  	und_irq;		
	
	struct net_device_stats stats;
	spinlock_t lock; 							/* Serialise access to device */
	
	/* debug /proc entry */
	struct proc_dir_entry *ps;
	int dma_halt_cnt;    u32 halt_tx_count;
	int dma_collide_cnt; u32 collide_tx_count;
	int dma_run_cnt;     u32 run_tx_count;
	int dma_race_cnt;    u32 race_tx_count;
};


static inline void rc32355_abort_dma(struct net_device *dev, rc32355_dma_ch_t* ch)
{
	
	if (readl(&ch->dmac) & DMAC_RUN) 
	{
		writel(0x10, &ch->dmac); 	
		while (!(readl(&ch->dmas) & DMAS_H))
			dev->trans_start = jiffies;			
		writel(0, &ch->dmas);  
	}
	
	writel(0, &ch->dmadptr); 
	writel(0, &ch->dmandptr); 
	
}


#endif /* RC32355_H */














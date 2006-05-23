/**************************************************************************
 *
 *  BRIEF MODULE DESCRIPTION
 *     Definitions for IDT RC32365 on-chip ethernet controller.
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
 *   Based on the driver written by B. Maruthanayakam, H. Kou and others.
 *
 *
 **************************************************************************
 */

#include <linux/config.h>
#include  <asm/idt-boards/rc32300/rc32365_dma_v.h>
#include  <asm/idt-boards/rc32300/rc32365_eth_v.h>
#include  <asm/idt-boards/rc32300/rc32365_eth.h> 
#include  <asm/idt-boards/rc32300/rc32365_dma.h> 
#include  <asm/idt-boards/rc32300/rc32365.h> 

#define RC32365_DEBUG	2
#define RC32365_PROC_DEBUG
#undef	RC32365_DEBUG
//#undef	RC32365_PROC_DEBUG

#ifdef RC32365_DEBUG

static int rc32365_debug = RC32365_DEBUG;
#define ASSERT(expr) \
	if(!(expr)) {	\
		printk( "Assertion failed! %s,%s,%s,line=%d\n",	\
		#expr,__FILE__,__FUNCTION__,__LINE__);		}
#define DBG(lvl, format, arg...) \
         if (rc32365_debug > lvl) printk(KERN_INFO "%s: " format, dev->name , ## arg)
#else
#define ASSERT(expr) do {} while (0)
#define DBG(lvl, format, arg...) do {} while (0)
#endif

#define ERR(format, arg...) printk(KERN_ERR "%s: " format, dev->name , ## arg)
#define WARN(format, arg...) printk(KERN_WARNING "%s: " format, dev->name , ## arg)		
#define INFO(format, arg...) printk(KERN_INFO "%s: " format, dev->name , ## arg)

#define ETH0_DMA_RX_IRQ   (GROUP1_IRQ_BASE + 0)
#define ETH0_DMA_TX_IRQ   (GROUP1_IRQ_BASE + 1)
#define ETH0_RX_OVR_IRQ   (GROUP3_IRQ_BASE + 4)
#define ETH0_TX_UND_IRQ   (GROUP3_IRQ_BASE + 5)
#define ETH1_DMA_RX_IRQ   (GROUP1_IRQ_BASE + 2)
#define ETH1_DMA_TX_IRQ   (GROUP1_IRQ_BASE + 3)
#define ETH1_RX_OVR_IRQ   (GROUP3_IRQ_BASE + 7)
#define ETH1_TX_UND_IRQ   (GROUP3_IRQ_BASE + 8)

#define ETH0_RX_DMA_ADDR  (DMA0_PhysicalAddress + 0*DMA_CHAN_OFFSET)
#define ETH0_TX_DMA_ADDR  (DMA0_PhysicalAddress + 1*DMA_CHAN_OFFSET)
#define ETH1_RX_DMA_ADDR  (DMA0_PhysicalAddress + 2*DMA_CHAN_OFFSET)
#define ETH1_TX_DMA_ADDR  (DMA0_PhysicalAddress + 3*DMA_CHAN_OFFSET)

#define RC32365_NUM_RDS	 128    /* number of receive descriptors */
#define RC32365_NUM_TDS	 128    /* number of transmit descriptors */

#define RC32365_RBSIZE	 1536  /* size of one resource buffer = Ether MTU */
#define RC32365_RDS_MASK (RC32365_NUM_RDS-1)
#define RC32365_TDS_MASK (RC32365_NUM_TDS-1)
#define RD_RING_SIZE	 (RC32365_NUM_RDS * sizeof(struct DMAD_s))
#define TD_RING_SIZE	 (RC32365_NUM_TDS * sizeof(struct DMAD_s))

//#define RC32365_TX_TIMEOUT HZ/4
#define RC32365_TX_TIMEOUT HZ * 100

#define rc32365_eth0_regs ((ETH_t)(ETH0_VirtualAddress))
#define rc32365_eth1_regs ((ETH_t)(ETH1_VirtualAddress))

enum status	{ filled,	empty};

extern unsigned int idt_cpu_freq;


/* Index to functions, as function prototypes. */
static int rc32365_open(struct net_device *dev);
static int rc32365_send_packet(struct sk_buff *skb, struct net_device *dev);
static void rc32365_mii_handler(unsigned long data);
static irqreturn_t rc32365_ovr_interrupt(int irq, void *dev_id, struct pt_regs * regs);
static irqreturn_t rc32365_und_interrupt(int irq, void *dev_id, struct pt_regs * regs);
static irqreturn_t rc32365_rx_dma_interrupt(int irq, void *dev_id, struct pt_regs * regs);
static irqreturn_t rc32365_tx_dma_interrupt(int irq, void *dev_id, struct pt_regs * regs);
static int  rc32365_close(struct net_device *dev);
static struct net_device_stats *rc32365_get_stats(struct net_device *dev);
static void rc32365_multicast_list(struct net_device *dev);
static int  rc32365_init(struct net_device *dev);
static void rc32365_tx_timeout(struct net_device *dev);
static void rc32365_rx_tasklet(unsigned long rx_data_dev);
static void rc32365_tx_tasklet(unsigned long tx_data_dev);
static void rc32365_cleanup_module(void);
static int rc32365_probe(int port_num);


/* Information that need to be kept for each board. */
struct rc32365_local {
	ETH_t  eth_regs;
	DMA_Chan_t  rx_dma_regs;
	DMA_Chan_t  tx_dma_regs;
	IPABM_ETH_t ipabmc;
	
	volatile DMAD_t   td_ring; 			/* transmit descriptor ring */ 
	volatile DMAD_t   rd_ring; 			/* receive descriptor ring  */
	
	struct sk_buff* tx_skb[RC32365_NUM_TDS]; 	/* skbuffs for pkt to trans */
	struct sk_buff* rx_skb[RC32365_NUM_RDS]; 	/* skbuffs for pkt to trans */
	
	struct tasklet_struct * rx_tasklet;
	struct tasklet_struct * tx_tasklet;
	struct tasklet_struct * ovr_tasklet;
	
	int	rx_next_done;
	int	rx_chain_head;
	int	rx_chain_tail;
	enum status	rx_chain_status;
	
	int	tx_next_done;
	int	tx_chain_head;
	int	tx_chain_tail;
	enum status	tx_chain_status;
	int	tx_count;	                        /* current # of pkts waiting to be sent */
	int	tx_full;
	struct timer_list    mii_phy_timer;
	unsigned long duplex_mode;
	
	int	rx_irq;
	int	tx_irq;
	int	ovr_irq;
	int	und_irq;
	
	struct net_device_stats stats;
	spinlock_t lock; 				
	
	/* debug /proc entry */
	struct proc_dir_entry *ps;
	int dma_halt_cnt;  int dma_run_cnt;
};

static inline void rc32365_abort_dma(struct net_device *dev, DMA_Chan_t ch, volatile u32 *ipabmcx)
{
	volatile u32 ipabmc;
#ifdef RC32365_PROC_DEBUG
	struct rc32365_local *lp = (struct rc32365_local *)dev->priv;
#endif
	if (local_readl(&ch->dmac) & DMAC_run_m) 
	{
		ipabmc = local_readl(ipabmcx);
		local_writel(ipabmc|0x00004000,ipabmcx);
		local_writel(0x10, &ch->dmac); 	
		local_writel(ipabmc,ipabmcx);
		while (!(local_readl(&ch->dmas) & DMAS_h_m))
			dev->trans_start = jiffies;			
		local_writel(0, &ch->dmas);  
#ifdef RC32365_PROC_DEBUG
		lp->dma_run_cnt++;
#endif
	}
	local_writel(0, &ch->dmandptr); 
	local_writel(0, &ch->dmadptr); 
}

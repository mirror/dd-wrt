/**************************************************************************
 *
 *  BRIEF MODULE DESCRIPTION
 *     Definitions for IDT RC32438 on-chip ethernet controller.
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
 *  THIS SOFTWARE, EVEN IF ADVISED OF THEg POSSIBILITY OF SUCH DAMAGE.
 *
 *  You should have received a copy of the  GNU General Public License along
 *  with this program; if not, write  to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 **************************************************************************
 * May 2004 P. Sadik
 *
 * Initial Release
 *
 * Aug 2004 P. Sadik
 *
 * Added NAPI support
 **************************************************************************
 */

#include  <asm/idt-boards/rc32438/rc32438.h>
#include  <asm/idt-boards/rc32438/rc32438_dma_v.h>
#include  <asm/idt-boards/rc32438/rc32438_eth_v.h>

#define RC32438_PROC_DEBUG
//#define RC32438_DEBUG	2

#ifdef RC32438_DEBUG

/* use 0 for production, 1 for verification, >2 for debug */
static int rc32438_debug = RC32438_DEBUG;
#define ASSERT(expr) \
	if(!(expr)) {	\
		printk( "Assertion failed! %s,%s,%s,line=%d\n",	\
		#expr,__FILE__,__FUNCTION__,__LINE__);		}
#define DBG(lvl, format, arg...) if (rc32438_debug > lvl) printk(KERN_INFO "%s: " format, dev->name , ## arg)
#else
#define ASSERT(expr) do {} while (0)
#define DBG(lvl, format, arg...) do {} while (0)
#endif

#define INFO(format, arg...) printk(KERN_INFO "%s: " format, dev->name , ## arg)
#define ERR(format, arg...) printk(KERN_ERR "%s: " format, dev->name , ## arg)
#define WARN(format, arg...) printk(KERN_WARNING "%s: " format, dev->name , ## arg)

#define ETH0_DMA_RX_IRQ   GROUP1_IRQ_BASE + 2
#define ETH0_DMA_TX_IRQ   GROUP1_IRQ_BASE + 3
#define ETH0_RX_OVR_IRQ   GROUP3_IRQ_BASE + 12
#define ETH0_TX_UND_IRQ   GROUP3_IRQ_BASE + 13
#define ETH1_DMA_RX_IRQ   GROUP1_IRQ_BASE + 4
#define ETH1_DMA_TX_IRQ   GROUP1_IRQ_BASE + 5
#define ETH1_RX_OVR_IRQ   GROUP3_IRQ_BASE + 15
#define ETH1_TX_UND_IRQ   GROUP3_IRQ_BASE + 16

#define ETH0_RX_DMA_ADDR  (DMA0_PhysicalAddress + 2*DMA_CHAN_OFFSET)
#define ETH0_TX_DMA_ADDR  (DMA0_PhysicalAddress + 3*DMA_CHAN_OFFSET)
#define ETH1_RX_DMA_ADDR  (DMA0_PhysicalAddress + 4*DMA_CHAN_OFFSET)
#define ETH1_TX_DMA_ADDR  (DMA0_PhysicalAddress + 5*DMA_CHAN_OFFSET)

/* the following must be powers of two */
#define RC32438_NUM_RDS    64    		/* number of receive descriptors */
#define RC32438_NUM_TDS    64    		/* number of transmit descriptors */

#define RC32438_RBSIZE     1536  		/* size of one resource buffer = Ether MTU */
#define RC32438_RDS_MASK   (RC32438_NUM_RDS-1)
#define RC32438_TDS_MASK   (RC32438_NUM_TDS-1)
#define RD_RING_SIZE (RC32438_NUM_RDS * sizeof(struct DMAD_s))
#define TD_RING_SIZE (RC32438_NUM_TDS * sizeof(struct DMAD_s))

#define RC32438_TX_TIMEOUT HZ * 100

#define rc32438_eth0_regs ((ETH_t)(ETH0_VirtualAddress))
#define rc32438_eth1_regs ((ETH_t)(ETH1_VirtualAddress))

enum status	{ filled,	empty};
#define IS_DMA_FINISHED(X)   (((X) & (DMAD_f_m)) != 0)
#define IS_DMA_DONE(X)   (((X) & (DMAD_d_m)) != 0)


/* Information that need to be kept for each board. */
struct rc32438_local {
	ETH_t  eth_regs;
	DMA_Chan_t  rx_dma_regs;
	DMA_Chan_t  tx_dma_regs;
	volatile DMAD_t   td_ring;			/* transmit descriptor ring */
	volatile DMAD_t   rd_ring;			/* receive descriptor ring  */

	struct sk_buff* tx_skb[RC32438_NUM_TDS]; 	/* skbuffs for pkt to trans */
	struct sk_buff* rx_skb[RC32438_NUM_RDS]; 	/* skbuffs for pkt to trans */

	int weight;
#ifdef CONFIG_RC32438_REVISION_ZA
	struct tasklet_struct * ovr_und_tasklet;
#endif
	struct tasklet_struct * tx_tasklet;

	int	rx_next_done;
	int	rx_chain_head;
	int	rx_chain_tail;
	enum status	rx_chain_status;

	int	tx_next_done;
	int	tx_chain_head;
	int	tx_chain_tail;
	enum status	tx_chain_status;
	int tx_count;
	int	tx_full;

	struct timer_list    mii_phy_timer;
	unsigned long duplex_mode;

	int   	rx_irq;
	int    tx_irq;
	int    ovr_irq;
	int    und_irq;

	struct net_device_stats stats;
#ifdef CONFIG_SMP
	spinlock_t lock;
#endif

	/* debug /proc entry */
	struct proc_dir_entry *ps;
	int dma_halt_cnt;
        int ibudget;
        int iquota;
        int done_cnt;
        int firstTime;
        int not_done_cnt;
        int dma_halt_nd_cnt;
        int dma_ovr_count;
        int dma_und_count;
	int tx_stopped;
};

extern unsigned int idt_cpu_freq;

/* Index to functions, as function prototypes. */
static int rc32438_open(struct net_device *dev);
static int rc32438_send_packet(struct sk_buff *skb, struct net_device *dev);
static void rc32438_mii_handler(unsigned long data);
static irqreturn_t rc32438_rx_dma_interrupt(int irq, void *dev_id, struct pt_regs * regs);
static irqreturn_t rc32438_tx_dma_interrupt(int irq, void *dev_id, struct pt_regs * regs);
static int rc32438_poll(struct net_device *dev, int *budget);
static int  rc32438_close(struct net_device *dev);
static struct net_device_stats *rc32438_get_stats(struct net_device *dev);
static void rc32438_multicast_list(struct net_device *dev);
static int  rc32438_init(struct net_device *dev);
static void rc32438_tx_timeout(struct net_device *dev);
static void rc32438_tx_tasklet(unsigned long dev_id);
#ifdef CONFIG_RC32438_REVISION_ZA
static void rc32438_ovr_und_tasklet(unsigned long dev_id);
#endif
static void rc32438_cleanup_module(void);
static int rc32438_probe(int port_num);
int rc32438_init_module(void);

static inline void rc32438_abort_dma(struct net_device *dev, DMA_Chan_t ch)
{
	if (rc32438_readl(&ch->dmac) & DMAC_run_m)
	{
		rc32438_writel(0x10, &ch->dmac);

		while (!(rc32438_readl(&ch->dmas) & DMAS_h_m))
			dev->trans_start = jiffies;

		rc32438_writel(0, &ch->dmas);
	}

	rc32438_writel(0, &ch->dmadptr);
	rc32438_writel(0, &ch->dmandptr);
}

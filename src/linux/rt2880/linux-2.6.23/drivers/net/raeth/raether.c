#include <linux/module.h>
#include <linux/version.h>

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/interrupt.h>
#include <linux/ptrace.h>
#include <linux/ioport.h>
#include <linux/in.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/pci.h>
#include <linux/socket.h>

#include <asm/system.h>
#include <asm/io.h>
#include <asm/bitops.h>
#include <asm/io.h>
#include <asm/dma.h>
#include <asm/atomic.h>
#include <asm/uaccess.h>
#include <asm/rt2880/surfboardint.h>

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
#include <asm/rt2880/rt_mmap.h>
#endif

#include <linux/errno.h>
#include <linux/init.h>
#include <linux/mca.h>

#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/if_vlan.h>
#include <linux/if_ether.h>
#include <linux/ethtool.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
#include <linux/libata-compat.h>
#endif

#include "ra2882ethreg.h"
#include "raether.h"
#include "ra_mac.h"
#include "ra_ioctl.h"

#include "ra_netlink.h"
#if defined (CONFIG_RAETH_QOS)
#include "ra_qos.h"
#endif

#if defined (CONFIG_RA_HW_NAT) || defined (CONFIG_RA_HW_NAT_MODULE)
#include "../../../net/nat/hw_nat/ra_nat.h"
#endif

static int raeth_mtu = 0;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
MODULE_PARM(raeth_mtu, "i");
MODULE_PARM_DESC(raeth_mtu, "RT2880 RaEther MTU");
#endif

#ifdef CONFIG_RAETH_NAPI
static int raeth_clean(struct net_device *dev, int *budget);
static int rt2880_eth_recv(struct net_device* dev, int *work_done, int work_to_do);
#else
static int rt2880_eth_recv(struct net_device* dev);
#endif

#if !defined(CONFIG_RA_NAT_NONE) 
/* bruce+
 */
extern int (*ra_sw_nat_hook_rx)(struct sk_buff *skb);
extern int (*ra_sw_nat_hook_tx)(struct sk_buff *skb, int gmac_no);
#endif

#if defined(CONFIG_RALINK_RT3052_MP) || defined(CONFIG_RALINK_RT3052_MP2)
int32_t mcast_rx(struct sk_buff * skb);
int32_t mcast_tx(struct sk_buff * skb);
#endif

/* gmac driver feature set config */
#define	HEADER_ALIGNED	1

#ifdef CONFIG_RAETH_NAPI
#undef DELAY_INT
//#define DELAY_INT	1
#else
#define DELAY_INT	1
#endif

//#define DSP_EN
#undef	DSP_EN
/* end of config */

#ifdef CONFIG_RAETH_JUMBOFRAME
#define	MAX_RX_LENGTH	4096
#else
#define	MAX_RX_LENGTH	1500
#endif

#define RX_DESC_SIZE	(sizeof(struct PDMA_rxdesc))

static struct sk_buff	*netrx_skbuf[NUM_RX_DESC];

static struct net_device *dev_raether;

static int rx_dma_owner_idx0;     /* Point to the next RXD DMA wants to use in RXD Ring#0.  */
static int rx_wants_alloc_idx0;   /* Point to the next RXD CPU wants to allocate to RXD Ring #0. */

static struct PDMA_rxdesc *rx_ring;
static unsigned int phy_rx_ring;

#if defined (CONFIG_ETHTOOL) && ( defined (CONFIG_RAETH_ROUTER) || defined (CONFIG_RT_3052_ESW) )
extern struct ethtool_ops ra_ethtool_ops;
#endif

#ifdef CONFIG_RALINK_VISTA_BASIC
int is_switch_175c = 1;
#endif
END_DEVICE *ra_ei_local;
//bruce debug
#if 0
void skb_dump(struct sk_buff* sk) {
        unsigned int i;

        printk("skb_dump: from %s with len %d (%d) headroom=%d tailroom=%d\n",
                sk->dev?sk->dev->name:"ip stack",sk->len,sk->truesize,
                skb_headroom(sk),skb_tailroom(sk));

        for(i=(unsigned int)sk->head;i<=(unsigned int)sk->tail;i++) {
                if((i % 20) == 0)
                        printk("\n");
                if(i==(unsigned int)sk->data) printk("{");
                if(i==(unsigned int)sk->h.raw) printk("#");
                if(i==(unsigned int)sk->nh.raw) printk("|");
                if(i==(unsigned int)sk->mac.raw) printk("*");
                printk("%02x",*((unsigned char*)i));
                if(i==(unsigned int)sk->tail) printk("}");
        }
        printk("\n");
}
#endif

#if defined (CONFIG_GIGAPHY) || defined (CONFIG_P5_MAC_TO_PHY_MODE)
int isMarvellGigaPHY(void)
{
        u32 phy_id0,phy_id1;

        if( ! mii_mgr_read(31, 2,&phy_id0)){
                printk("\n Read PhyID 0 is Fail!!\n");
                phy_id0 =0;
        }

        if( ! mii_mgr_read(31, 3,&phy_id1)){
                printk("\n Read PhyID 1 is Fail!!\n");
                phy_id1 = 0;
        }

        if((phy_id0 == EV_MARVELL_PHY_ID0) && (phy_id1 == EV_MARVELL_PHY_ID1))
                return 1;

        return 0;
}

int isMarvellGigaPHY2(void)
{
        u32 phy_id0,phy_id1;

        if( ! mii_mgr_read(0x14, 2,&phy_id0)){
                printk("\n Read PhyID 0 is Fail!!\n");
                phy_id0 =0;
        }

        if( ! mii_mgr_read(0x14, 3,&phy_id1)){
                printk("\n Read PhyID 1 is Fail!!\n");
                phy_id1 = 0;
        }
        
        if((phy_id0 == EV_MARVELL_PHY_ID0) && (phy_id1 == EV_MARVELL_PHY_SENAO_ID1))
                return 1;

        return 0;
}

#endif	

/*
 * Set the hardware MAC address.
 */
static int ei_set_mac_addr(struct net_device *dev, void *p)
{
	END_DEVICE *ei_local=netdev_priv(dev);
	MAC_INFO *macinfo = (MAC_INFO*)ei_local->MACInfo;
	struct sockaddr *addr = p;

	memcpy(dev->dev_addr, addr->sa_data, dev->addr_len);
	
	if(netif_running(dev))
		return -EBUSY;

        ra2880MacAddressSet(macinfo, addr->sa_data);
	return 0;
}

#ifdef CONFIG_PSEUDO_SUPPORT
static int ei_set_mac2_addr(struct net_device *dev, void *p)
{
	END_DEVICE *ei_local=netdev_priv(dev);
	MAC_INFO *macinfo = (MAC_INFO*)ei_local->MACInfo;
	struct sockaddr *addr = p;

	memcpy(dev->dev_addr, addr->sa_data, dev->addr_len);
	
	if(netif_running(dev))
		return -EBUSY;

        ra2880Mac2AddressSet(macinfo, p);
	return 0;
}
#endif

void set_fe_pdma_glo_cfg(void)
{
        int fe_glo_cfg=0;
        int pdma_glo_cfg=0;

	pdma_glo_cfg = sysRegRead(PDMA_GLO_CFG);
	pdma_glo_cfg |= (RT2880_TX_WB_DDONE | RT2880_RX_DMA_EN | RT2880_TX_DMA_EN | PDMA_BT_SIZE_4DWORDS);
	sysRegWrite(PDMA_GLO_CFG, pdma_glo_cfg);

	//set 1us timer count in unit of clock cycle
	fe_glo_cfg = sysRegRead(FE_GLO_CFG);
	fe_glo_cfg &= ~(0xff << 8); //clear bit8-bit15

	/* Unit = MHz */
	fe_glo_cfg |= ( (get_surfboard_sysclk()/1000000) << 8); 

	sysRegWrite(FE_GLO_CFG, fe_glo_cfg);
}

int forward_config(struct net_device *dev)
{
	unsigned int	regVal, regCsg;
	regVal = sysRegRead(GDMA1_FWD_CFG);
	regCsg = sysRegRead(CDMA_CSG_CFG);

	//set unicast/multicast/broadcast frame to cpu
	regVal &= ~0xFFFF; 

#ifdef CONFIG_RAETH_CHECKSUM_OFFLOAD
	//enable ipv4 header checksum check
	regVal |= RT2880_GDM1_ICS_EN;
	regCsg |= RT2880_ICS_GEN_EN;
	
	//enable tcp checksum check
	regVal |= RT2880_GDM1_TCS_EN;
	regCsg |= RT2880_TCS_GEN_EN;
	
	//enable udp checksum check
	regVal |= RT2880_GDM1_UCS_EN;
	regCsg |= RT2880_UCS_GEN_EN;
 
	dev->features |= NETIF_F_IP_CSUM;
#else
 	//disable ipv4 header checksum check
 	regVal &= ~RT2880_GDM1_ICS_EN;
	regCsg &= ~RT2880_ICS_GEN_EN;
 	
 	//disable tcp checksum check
 	regVal &= ~RT2880_GDM1_TCS_EN;
	regCsg &= ~RT2880_TCS_GEN_EN;
 	
 	//disable udp checksum check
 	regVal &= ~RT2880_GDM1_UCS_EN;
	regCsg &= ~RT2880_UCS_GEN_EN;
#endif

#ifdef CONFIG_RAETH_JUMBOFRAME
	// enable jumbo frame
	regVal |= RT2880_GDM1_JMB_EN;
#endif

	// test, if disable bit 16
	// regVal &= ~RT2880_GDM1_STRPCRC;

	sysRegWrite(GDMA1_FWD_CFG, regVal);
	sysRegWrite(CDMA_CSG_CFG, regCsg);

/*
 * 	PSE_FQ_CFG register definition -
 *
 * 	Define max free queue page count in PSE. (31:24)
 *	RT2883 - 0xff908000 (255 pages)
 *	RT3052 - 0x80504000 (128 pages)
 *	RT2880 - 0x80504000 (128 pages)
 *
 * 	In each page, there are 128 bytes in each page.
 *
 *	23:16 - free queue flow control release threshold
 *	15:8  - free queue flow control assertion threshold
 *	7:0   - free queue empty threshold
 *
 *	The register affects QOS correctness in frame engine!
 */

#if defined(CONFIG_RALINK_RT2883)
	sysRegWrite(PSE_FQ_CFG, cpu_to_le32(INIT_VALUE_OF_RT2883_PSE_FQ_CFG));
#else
	sysRegWrite(PSE_FQ_CFG, cpu_to_le32(INIT_VALUE_OF_RT2880_PSE_FQFC_CFG));
#endif

/*
 *	FE_RST_GLO register definition -
 *
 *	Bit 0: PSE Rest
 *	Reset PSE after re-programming PSE_FQ_CFG.
 */
	regVal = 0x1;
	sysRegWrite(FE_RST_GL, regVal);
	sysRegWrite(FE_RST_GL, 0);	// update for RSTCTL issue

	regVal = sysRegRead(GDMA1_FWD_CFG);
	printk("GDMA1_FWD_CFG = %0X\n",regVal);

#ifdef CONFIG_PSEUDO_SUPPORT
	regVal = sysRegRead(GDMA2_FWD_CFG);
	printk("GDMA2_FWD_CFG = %0X\n",regVal);
#endif

	return 1;
}

static int rt2880_eth_setup(struct net_device *dev)
{

	int	i;
	unsigned int	regVal;
	END_DEVICE* ei_local = netdev_priv(dev);

	while(1)
	{
		regVal = sysRegRead(PDMA_GLO_CFG);	
		if((regVal & RT2880_RX_DMA_BUSY))
		{
			printk("\n  RT2880_RX_DMA_BUSY !!! ");
			continue;
		}
		if((regVal & RT2880_TX_DMA_BUSY))
		{
			printk("\n  RT2880_TX_DMA_BUSY !!! ");
			continue;
		}
		break;
	}

#if defined (CONFIG_RAETH_QOS)
	int j=0;
	for (i=0;i<NUM_TX_RINGS;i++){
		for (j=0;j<NUM_TX_DESC;j++){
			ei_local->skb_free[i][j]=0;
		}
                ei_local->free_idx[i]=0;
	}
	/*
	 * RT2880: 2 x TX_Ring, 1 x Rx_Ring
	 * RT2883: 4 x TX_Ring, 1 x Rx_Ring
	 * RT3052: 4 x TX_Ring, 1 x Rx_Ring
	 */
	fe_tx_desc_init(dev, 0, 3, 1);
	if (ei_local->tx_ring0 == NULL) {
		printk("RAETH: tx ring0 allocation failed\n");
		return 0;
	}

	fe_tx_desc_init(dev, 1, 3, 1);
	if (ei_local->tx_ring1 == NULL) {
		printk("RAETH: tx ring1 allocation failed\n");
		return 0;
	}

#if defined (CONFIG_RALINK_RT2883) || defined (CONFIG_RALINK_RT3052)
	fe_tx_desc_init(dev, 2, 3, 1);
	if (ei_local->tx_ring2 == NULL) {
		printk("RAETH: tx ring2 allocation failed\n");
		return 0;
	}
	
	fe_tx_desc_init(dev, 3, 3, 1);
	if (ei_local->tx_ring3 == NULL) {
		printk("RAETH: tx ring3 allocation failed\n");
		return 0;
	}
#endif	
#else
	for (i=0;i<NUM_TX_DESC;i++){
		ei_local->skb_free[i]=0;
	}
	ei_local->free_idx =0;
    	ei_local->tx_ring0 = pci_alloc_consistent(NULL, NUM_TX_DESC * sizeof(struct PDMA_txdesc), &ei_local->phy_tx_ring0);
#endif // CONFIG_RAETH_QOS
	rx_ring = pci_alloc_consistent(NULL, NUM_RX_DESC * sizeof(struct PDMA_rxdesc), &phy_rx_ring);

#if defined (CONFIG_RAETH_QOS)
	printk("\nphy_tx_ring0 = %08x, tx_ring0 = %p, size: %d bytes\n", ei_local->phy_tx_ring0, ei_local->tx_ring0, sizeof(struct PDMA_txdesc));
	printk("\nphy_tx_ring1 = %08x, tx_ring1 = %p, size: %d bytes\n", ei_local->phy_tx_ring1, ei_local->tx_ring1, sizeof(struct PDMA_txdesc));
#if defined (CONFIG_RALINK_RT2883) || defined (CONFIG_RALINK_RT3052)
	printk("\nphy_tx_ring2 = %08x, tx_ring2 = %p, size: %d bytes\n", ei_local->phy_tx_ring2, ei_local->tx_ring2, sizeof(struct PDMA_txdesc));
	printk("\nphy_tx_ring3 = %08x, tx_ring3 = %p, size: %d bytes\n", ei_local->phy_tx_ring3, ei_local->tx_ring3, sizeof(struct PDMA_txdesc));
#endif
#else
 	printk("\nphy_tx_ring = 0x%08x, tx_ring = 0x%p, size: %d bytes\n", ei_local->phy_tx_ring0, ei_local->tx_ring0, sizeof(struct PDMA_txdesc));
#endif
	printk("\nphy_rx_ring = 0x%08x, rx_ring = 0x%p, size: %d bytes\n",phy_rx_ring,rx_ring, sizeof(struct PDMA_rxdesc));
 
	for (i = 0; i < NUM_RX_DESC; i++) {
		// dma_cache_inv((unsigned long*)&rx_ring[i], sizeof(struct PDMA_rxdesc));
		memset(&rx_ring[i],0,16);
	    	rx_ring[i].rxd_info2.DDONE_bit = 0;
		rx_ring[i].rxd_info2.LS0 = 1;
#ifdef 	HEADER_ALIGNED	 
		rx_ring[i].rxd_info1.PDP0 = dma_map_single(NULL, skb_put(netrx_skbuf[i], 2), MAX_RX_LENGTH+2, PCI_DMA_FROMDEVICE);
#else
		rx_ring[i].rxd_info1.PDP0 = dma_map_single(NULL, netrx_skbuf[i]->data, MAX_RX_LENGTH, PCI_DMA_FROMDEVICE);
#endif			
	}
#ifndef DSP_VIA_NONCACHEABLE
	dma_cache_wback_inv((unsigned long)rx_ring, NUM_RX_DESC*(sizeof(struct PDMA_rxdesc)));
#endif

#if! defined (CONFIG_RAETH_QOS)
	for (i=0; i < NUM_TX_DESC; i++) {

#ifndef DSP_VIA_NONCACHEABLE
		dma_cache_inv((unsigned long)&ei_local->tx_ring0[i], sizeof(struct PDMA_txdesc));	
#endif
		memset(&ei_local->tx_ring0[i],0,16);
		ei_local->tx_ring0[i].txd_info2.LS0_bit = 1;
		ei_local->tx_ring0[i].txd_info2.DDONE_bit = 1;

	}
#endif

#ifndef DSP_VIA_NONCACHEABLE
#if! defined (CONFIG_RAETH_QOS)
	dma_cache_wback_inv((unsigned long)ei_local->tx_ring0, NUM_TX_DESC*(sizeof(struct PDMA_txdesc)));
#endif
#endif

	rx_dma_owner_idx0 = 0;
	rx_wants_alloc_idx0 = (NUM_RX_DESC - 1);
	regVal = sysRegRead(PDMA_GLO_CFG);
	regVal &= 0x000000FF;
   	sysRegWrite(PDMA_GLO_CFG, regVal);
	regVal=sysRegRead(PDMA_GLO_CFG);

	/* Tell the adapter where the TX/RX rings are located. */
#if !defined (CONFIG_RAETH_QOS)
        sysRegWrite(TX_BASE_PTR0, phys_to_bus((u32) ei_local->phy_tx_ring0));
	sysRegWrite(TX_MAX_CNT0, cpu_to_le32((u32) NUM_TX_DESC));
	sysRegWrite(TX_CTX_IDX0, 0);
	/* TX_TDX_IDX should be a RO register, use PDMA_RST_CFG instead */
	//sysRegWrite(TX_DTX_IDX0, 0); 	
	sysRegWrite(PDMA_RST_CFG, RT2880_PST_DTX_IDX0);
	/* Reset ring2 IDX if we use it */
	//sysRegWrite(PDMA_RST_CFG, RT2880_PST_DTX_IDX1); 
#endif

	sysRegWrite(RX_BASE_PTR0, phys_to_bus((u32) phy_rx_ring));
	sysRegWrite(RX_MAX_CNT0,  cpu_to_le32((u32) NUM_RX_DESC));
	sysRegWrite(RX_CALC_IDX0, cpu_to_le32((u32) (NUM_RX_DESC - 1)));
	/* RX_DRX_IDX should be RO registers, use PDMA_RST_CFG instead */
	//sysRegWrite(RX_DRX_IDX0,  cpu_to_le32((u32) 0));
	sysRegWrite(PDMA_RST_CFG, RT2880_PST_DRX_IDX0);

#if defined (CONFIG_RAETH_QOS)
	set_scheduler_weight();
	set_schedule_pause_condition();
#endif
	set_fe_pdma_glo_cfg();

	return 1;
}

#if! defined (CONFIG_RAETH_QOS)
static inline int rt2880_eth_send(struct net_device* dev, struct sk_buff *skb, int gmac_no)
{
	unsigned int	length=skb->len;
	END_DEVICE*	ei_local = netdev_priv(dev);
	unsigned long	tx_cpu_owner_idx0 = sysRegRead(TX_CTX_IDX0);

	while(ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info2.DDONE_bit == 0)
	{
		printk(KERN_ERR "%s: TX DMA is Busy !! TX desc is Empty!\n", dev->name);
		ei_local->stat.tx_errors++;
	}

	ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info1.SDP0 = virt_to_phys(skb->data);
	ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info2.SDL0 = length;
	ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info4.PN = gmac_no; 
	ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info4.QN = 3; 
	ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info2.DDONE_bit = 0;

#ifdef CONFIG_RAETH_CHECKSUM_OFFLOAD
	ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info4.TCO = 1; 
	ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info4.UCO = 1; 
	ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info4.ICO = 1; 
#endif

#if defined (CONFIG_RA_HW_NAT) || defined (CONFIG_RA_HW_NAT_MODULE) 
	if(FOE_MAGIC_TAG(skb) == FOE_MAGIC_PPE) {
	    ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info4.PN = 6; /* PPE */
	} 

	ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info4.RXIF = FOE_ALG_RXIF(skb); /* 0: WLAN, 1: PCI */
#endif	

    	tx_cpu_owner_idx0 = (tx_cpu_owner_idx0+1) % NUM_TX_DESC;
	sysRegWrite(TX_CTX_IDX0, cpu_to_le32((u32)tx_cpu_owner_idx0));
  	
	ei_local->stat.tx_packets++;
	ei_local->stat.tx_bytes += length;
#ifdef CONFIG_RAETH_NAPI
	if ( ei_local->tx_full == 1) {
		ei_local->tx_full = 0;
		netif_wake_queue(dev);
	}
#endif

	return length;
}
#endif

#ifdef CONFIG_RAETH_NAPI
static int rt2880_eth_recv(struct net_device* dev, int *work_done, int work_to_do)
#else
static int rt2880_eth_recv(struct net_device* dev)
#endif
{
	struct sk_buff *skb, *rx_skb;
	unsigned int	length = 0;
	unsigned long	RxProcessed;
	int bReschedule = 0;
	END_DEVICE* 	ei_local = netdev_priv(dev);

	RxProcessed = 0;

	for ( ; ; ) { 

#ifdef CONFIG_RAETH_NAPI
                if(*work_done >= work_to_do)
                        break;
                (*work_done)++;
#else
		if (RxProcessed++ > NUM_RX_MAX_PROCESS)
                {
                        // need to reschedule rx handle
                        bReschedule = 1;
                        break;
                }                                      
#endif
		/* Update to Next packet point that was received.
		 */ 
		rx_dma_owner_idx0 = (sysRegRead(RX_CALC_IDX0) + 1) % NUM_RX_DESC;
	
		if (rx_ring[rx_dma_owner_idx0].rxd_info2.DDONE_bit == 0) 
			break;


		length = rx_ring[rx_dma_owner_idx0].rxd_info2.PLEN0;

/* skb processing */
		rx_skb = netrx_skbuf[rx_dma_owner_idx0];
		rx_skb->len 	= length;
		rx_skb->data	= netrx_skbuf[rx_dma_owner_idx0]->data;
		//rx_skb->tail 	= rx_skb->data + length + 2;
		rx_skb->tail 	= rx_skb->data + length;

#ifdef CONFIG_PSEUDO_SUPPORT
    if(rx_ring[rx_dma_owner_idx0].rxd_info4.SP == 2) {
		if(ei_local->PseudoDev!=NULL) {
		    rx_skb->dev 	  = ei_local->PseudoDev;
		    rx_skb->protocol  = eth_type_trans(rx_skb,ei_local->PseudoDev);
		}else {
		    printk("ERROR: PseudoDev is still not initialize but receive packet from GMAC2\n");
		}
    }else{
		rx_skb->dev 	  = dev;
		rx_skb->protocol  = eth_type_trans(rx_skb,dev);
    }
#else
		rx_skb->dev 	  = dev;
		rx_skb->protocol  = eth_type_trans(rx_skb,dev);
#endif

#ifdef CONFIG_RAETH_CHECKSUM_OFFLOAD
		rx_skb->ip_summed = CHECKSUM_UNNECESSARY; 
#else
		rx_skb->ip_summed = CHECKSUM_NONE; 
#endif

#ifdef CONFIG_RT2880_BRIDGING_ONLY
		rx_skb->cb[22]=0xa8;
#endif

#if defined (CONFIG_RA_HW_NAT)  || defined (CONFIG_RA_HW_NAT_MODULE)
	FOE_MAGIC_TAG(rx_skb)= FOE_MAGIC_GE;
	memcpy(rx_skb->head+2,&rx_ring[rx_dma_owner_idx0].rxd_info4,
		sizeof(PDMA_RXD_INFO4_T));
#endif

#if !defined(CONFIG_RA_NAT_NONE) 
/* bruce+
 * ra_sw_nat_hook_rx return 1 --> continue
 * ra_sw_nat_hook_rx return 0 --> FWD & without netif_rx
 */
	 unsigned long flags;
         if(ra_sw_nat_hook_rx!= NULL)
         {
	   spin_lock_irqsave(&ei_local->page_lock, flags);
           if(ra_sw_nat_hook_rx(rx_skb)) {
#if defined(CONFIG_RALINK_RT3052_MP) || defined(CONFIG_RALINK_RT3052_MP2)
	       if(mcast_rx(rx_skb)==0) {
		   kfree_skb(rx_skb);
	       }else 
#endif
#ifdef CONFIG_RAETH_NAPI
                netif_receive_skb(rx_skb);
#else
                netif_rx(rx_skb);
#endif
	   }
	   spin_unlock_irqrestore(&ei_local->page_lock, flags);
         } else {
#if defined(CONFIG_RALINK_RT3052_MP) || defined(CONFIG_RALINK_RT3052_MP2)
	     if(mcast_rx(rx_skb)==0) {
		 kfree_skb(rx_skb);
	     }else 
#endif
#ifdef CONFIG_RAETH_NAPI
                netif_receive_skb(rx_skb);
#else
                netif_rx(rx_skb);
#endif
	 }
#else

#if defined(CONFIG_RALINK_RT3052_MP) || defined(CONFIG_RALINK_RT3052_MP2)
	if(mcast_rx(rx_skb)==0) {
		kfree_skb(rx_skb);
	}else 
#endif
#ifdef CONFIG_RAETH_NAPI
                netif_receive_skb(rx_skb);
#else
                netif_rx(rx_skb);
#endif


#endif 
		skb = __dev_alloc_skb(MAX_RX_LENGTH + 2, GFP_DMA | GFP_ATOMIC);
		if (skb == NULL)
		{
			printk(KERN_ERR "skb not available...\n");
			ei_local->stat.rx_dropped++;
                        bReschedule = 1;
			break;
			//return;
		}
		skb_reserve(skb, 2);
		rx_ring[rx_dma_owner_idx0].rxd_info2.DDONE_bit = 0;	
		netrx_skbuf[rx_dma_owner_idx0] = skb;
		rx_ring[rx_dma_owner_idx0].rxd_info1.PDP0 = dma_map_single(NULL, skb->data, MAX_RX_LENGTH+2, PCI_DMA_FROMDEVICE);

		dma_cache_wback_inv((unsigned long)&rx_ring[rx_dma_owner_idx0], sizeof(struct PDMA_rxdesc));	

		/*  Move point to next RXD which wants to alloc*/
		sysRegWrite(RX_CALC_IDX0, rx_dma_owner_idx0);	
		ei_local->stat.rx_packets++;
		ei_local->stat.rx_bytes += length;	
	}	/* for */

	//printk("%s(): %d\n",__FUNCTION__, __LINE__);
	return bReschedule;
}



///////////////////////////////////////////////////////////////////
/////
///// ra_get_stats - gather packet information for management plane
/////
///// Pass net_device_stats to the upper layer.
///// 
/////
///// RETURNS: pointer to net_device_stats
///////////////////////////////////////////////////////////////////

struct net_device_stats *ra_get_stats(struct net_device *dev)
{
	END_DEVICE *ei_local = netdev_priv(dev);
	return &ei_local->stat;
}

///////////////////////////////////////////////////////////////////
/////
///// ra2880Recv - process the next incoming packet
/////
///// Handle one incoming packet.  The packet is checked for errors and sent
///// to the upper layer.
/////
///// RETURNS: OK on success or ERROR.
///////////////////////////////////////////////////////////////////

void ei_receive(unsigned long unused)  // device structure 
{
	unsigned long flags;
	struct net_device *dev = dev_raether;
	END_DEVICE *ei_local = netdev_priv(dev);

	//printk("ei_receive called\n");
	spin_lock_irqsave(&(ei_local->page_lock), flags);
#ifndef CONFIG_RAETH_NAPI
	unsigned long reg_int_mask=0;
	int bReschedule = rt2880_eth_recv(dev);
	if(bReschedule)
        {
		tasklet_schedule(&ei_local->rx_tasklet);
        }else{
    		reg_int_mask=sysRegRead(FE_INT_ENABLE);
#if defined(DELAY_INT)
    		sysRegWrite(FE_INT_ENABLE, reg_int_mask|RT2880_RX_DLY_INT);
#else
    		sysRegWrite(FE_INT_ENABLE, reg_int_mask|RT2880_RX_DONE_INT0);
#endif
        }
#endif
	spin_unlock_irqrestore(&(ei_local->page_lock), flags);
}

#ifdef CONFIG_RAETH_NAPI
static int
raeth_clean(struct net_device *netdev, int *budget)
{
	END_DEVICE *ei_local =netdev_priv(netdev);
	//unsigned int reg_int_val;
        int work_to_do = min(*budget, netdev->quota);
        //int tx_cleaned;
        int work_done = 0;
	unsigned long reg_int_mask=0;

	ei_xmit_housekeeping(0);
	rt2880_eth_recv(netdev, &work_done, work_to_do);

#if 1 /* this could control when to re-enable interrupt, 0-> mean never enable interrupt*/
        *budget -= work_done;
        netdev->quota -= work_done;
        /* if no Tx and not enough Rx work done, exit the polling mode */
        if(( (work_done < work_to_do)) || !netif_running(netdev)) {
                netif_rx_complete(netdev);
		atomic_dec_and_test(&ei_local->irq_sem);

		sysRegWrite(FE_INT_STATUS, FE_INT_ALL);		// ack all fe interrupts
    		reg_int_mask=sysRegRead(FE_INT_ENABLE);

#ifdef DELAY_INT
		sysRegWrite(FE_INT_ENABLE, reg_int_mask |FE_INT_DLY_INIT);  // init delay interrupt only
#else
		//sysRegWrite(FE_INT_ENABLE, FE_INT_ALL);		// enable all fe interrupts
	    sysRegWrite(FE_INT_ENABLE,reg_int_mask|RT2880_RX_DONE_INT0 \
			    	      		|RT2880_TX_DONE_INT0 \
				      		|RT2880_TX_DONE_INT1 \
				      		|RT2880_TX_DONE_INT2 \
				      		|RT2880_TX_DONE_INT3);
#endif
                return 0;
        }
#endif

        return 1;
}

#endif


/**
 * ei_interrupt - handle controler interrupt
 *
 * This routine is called at interrupt level in response to an interrupt from
 * the controller.
 *
 * RETURNS: N/A.
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,21)
static irqreturn_t ei_interrupt(int irq, void *dev_id)
#else
static irqreturn_t ei_interrupt(int irq, void *dev_id, struct pt_regs * regs)
#endif
{
#if !defined(CONFIG_RAETH_NAPI)
	unsigned long reg_int_val;
	unsigned long reg_int_mask=0;
	unsigned int recv = 0;
	unsigned int transmit = 0;
	unsigned long flags; 
#endif 

	struct net_device *dev = (struct net_device *) dev_id;
	END_DEVICE *ei_local = netdev_priv(dev);

	if (dev == NULL) 
	{
		printk (KERN_ERR "net_interrupt(): irq %x for unknown device.\n", RT2880_IRQ_ENET0);
		return IRQ_NONE;
	}

#ifdef CONFIG_RAETH_NAPI
        if(netif_rx_schedule_prep(dev)) {
                atomic_inc(&ei_local->irq_sem);
		sysRegWrite(FE_INT_ENABLE, 0);
                __netif_rx_schedule(dev);
        }
#else

	spin_lock_irqsave(&(ei_local->page_lock), flags);
	reg_int_val = sysRegRead(FE_INT_STATUS);

#if defined (DELAY_INT)
	if((reg_int_val & RT2880_RX_DLY_INT))
		recv = 1;

	if (reg_int_val & RT2880_TX_DLY_INT)
		transmit = 1;
#else
	if((reg_int_val & RT2880_RX_DONE_INT0))
		recv = 1;

	if (reg_int_val & RT2880_TX_DONE_INT0)
		transmit |= RT2880_TX_DONE_INT0;
#if defined (CONFIG_RAETH_QOS)
	if (reg_int_val & RT2880_TX_DONE_INT1)
		transmit |= RT2880_TX_DONE_INT1;
	if (reg_int_val & RT2880_TX_DONE_INT2)
		transmit |= RT2880_TX_DONE_INT2;
	if (reg_int_val & RT2880_TX_DONE_INT3)
		transmit |= RT2880_TX_DONE_INT3;
#endif //CONFIG_RAETH_QOS

#endif //DELAY_INT

	sysRegWrite(FE_INT_STATUS, 0xFFFFFFFF);	

	if( recv == 1)
	{
		reg_int_mask = sysRegRead(FE_INT_ENABLE);	
#if defined (DELAY_INT)
		sysRegWrite(FE_INT_ENABLE, reg_int_mask& ~(RT2880_RX_DLY_INT));
#else
		sysRegWrite(FE_INT_ENABLE, reg_int_mask& ~(RT2880_RX_DONE_INT0));
#endif //DELAY_INT
		tasklet_schedule(&ei_local->rx_tasklet);
		//recv = 0;
	}	

	if ( transmit != 0)  
		ei_xmit_housekeeping(0);
	
	spin_unlock_irqrestore(&(ei_local->page_lock), flags);
#endif
	
	return IRQ_HANDLED;
}

#if defined (CONFIG_RALINK_RT3052)

#define PORT0_QUEUE_FULL		(1<<14) //port0 queue full
#define PORT1_QUEUE_FULL		(1<<15) //port1 queue full
#define PORT2_QUEUE_FULL		(1<<16) //port2 queue full
#define PORT3_QUEUE_FULL		(1<<17) //port3 queue full
#define PORT4_QUEUE_FULL		(1<<18) //port4 queue full
#define PORT5_QUEUE_FULL		(1<<19) //port5 queue full
#define PORT6_QUEUE_FULL		(1<<20) //port6 queue full
#define QUEUE_EXHAUSTED			(1<<24) //global queue is used up and all packets are dropped
#define BC_STROM			(1<<25) //the device is undergoing broadcast storm
#define PORT_ST_CHG			(1<<26) //Port status change
static irqreturn_t esw_interrupt(int irq, void *dev_id)
{
	unsigned long flags; 
	unsigned long reg_int_val;
	struct net_device *dev = (struct net_device *) dev_id;
	END_DEVICE *ei_local = netdev_priv(dev);

	spin_lock_irqsave(&(ei_local->page_lock), flags);
	reg_int_val = (*((volatile u32 *)(RALINK_ETH_SW_BASE))); //Interrupt Status Register

	if (reg_int_val & PORT_ST_CHG) {
	    printk("RT305x_ESW: Link Status Changed\n");
	}

	*((volatile u32 *)(RALINK_ETH_SW_BASE))=0xFFFFFFFF; //write one clear

	spin_unlock_irqrestore(&(ei_local->page_lock), flags);
	return IRQ_HANDLED;
}
#endif

static int ei_start_xmit(struct sk_buff* skb, struct net_device *dev, int gmac_no)
{
	END_DEVICE *ei_local = netdev_priv(dev);
	unsigned long flags;
	unsigned long tx_cpu_owner_idx;
	unsigned int tx_cpu_owner_idx_next;
#if	!defined(CONFIG_RAETH_QOS)
	struct PDMA_txdesc* tx_desc;
#endif
#ifdef CONFIG_RALINK_VISTA_BASIC
	struct vlan_ethhdr *veth;
#endif

#if !defined(CONFIG_RA_NAT_NONE)
/* bruce+
 */
         if(ra_sw_nat_hook_tx!= NULL)
         {
	   spin_lock_irqsave(&ei_local->page_lock, flags);
           if(ra_sw_nat_hook_tx(skb, gmac_no)==1){
	   	spin_unlock_irqrestore(&ei_local->page_lock, flags);
	   }else{
	        kfree_skb(skb);
	   	spin_unlock_irqrestore(&ei_local->page_lock, flags);
	   	return 0;
	   }
         }
#endif

#if defined(CONFIG_RALINK_RT3052_MP) || defined(CONFIG_RALINK_RT3052_MP2)
	mcast_tx(skb);
#endif

#if defined (CONFIG_RT_3052_ESW)
#define MIN_PKT_LEN  64
	 if (skb->len < MIN_PKT_LEN) {
	     if (skb_padto(skb, MIN_PKT_LEN)) {
		 printk("raeth: skb_padto failed\n");
		 kfree_skb(skb);
		 return 0;
	     }
	     skb_put(skb, MIN_PKT_LEN - skb->len);
	 }
#endif //CONFIG_RT_3052_ESW

	dev->trans_start = jiffies;	/* save the timestamp */
	spin_lock_irqsave(&ei_local->page_lock, flags);
#if defined( CONFIG_RT2880_ENHANCE) || defined (CONFIG_RT2880_BRIDGING_ONLY)
	if ((unsigned char)skb->cb[22] == 0xa9)
		dma_cache_wback_inv((unsigned long)skb->data, 60);
	else if ((unsigned char)skb->cb[22] == 0xa8) {
		dma_cache_wback_inv((unsigned long)skb->data, 16);
	}
	else
		dma_cache_wback_inv((unsigned long)skb->data, skb->len);
#else
	dma_cache_wback_inv((unsigned long)skb->data, skb->len);
#endif
#ifdef CONFIG_RALINK_VISTA_BASIC
	veth = (struct vlan_ethhdr *)(skb->data);
	if (is_switch_175c && veth->h_vlan_proto == __constant_htons(ETH_P_8021Q)) {
		if ((veth->h_vlan_TCI & __constant_htons(VLAN_VID_MASK)) == 0) {
			veth->h_vlan_TCI |= htons(VLAN_DEV_INFO(dev)->vlan_id);
		}
	}
#endif

#if defined (CONFIG_RAETH_QOS)
	int ring_no, queue_no, port_no;
	if(pkt_classifier(skb, gmac_no, &ring_no, &queue_no, &port_no)) {
		get_tx_ctx_idx(ring_no, &tx_cpu_owner_idx);

	if(tx_cpu_owner_idx== NUM_TX_DESC-1)
		tx_cpu_owner_idx_next = 0;
	else
		tx_cpu_owner_idx_next = tx_cpu_owner_idx +1;


	  if(((ei_local->skb_free[ring_no][tx_cpu_owner_idx]) ==0) && (ei_local->skb_free[ring_no][tx_cpu_owner_idx_next]==0)){
	    fe_qos_packet_send(dev, skb, ring_no, queue_no, port_no);
	  }else{
	    ei_local->stat.tx_dropped++;
	    kfree_skb(skb);
	    spin_unlock_irqrestore(&ei_local->page_lock, flags);
	    return 0;	
	  }
	} 
#else
	tx_cpu_owner_idx = *(unsigned long*)TX_CTX_IDX0;
	if(tx_cpu_owner_idx== NUM_TX_DESC-1)
		tx_cpu_owner_idx_next = 0;
	else
		tx_cpu_owner_idx_next = tx_cpu_owner_idx +1;

	if(((ei_local->skb_free[tx_cpu_owner_idx]) ==0) && (ei_local->skb_free[tx_cpu_owner_idx_next]==0))
		rt2880_eth_send(dev, skb, gmac_no);
	else {
	    ei_local->stat.tx_dropped++;
	    kfree_skb(skb);
	    spin_unlock_irqrestore(&ei_local->page_lock, flags);
	    return 0;	
	}
	tx_desc = ei_local->tx_ring0;
	ei_local->skb_free[tx_cpu_owner_idx] = skb;	

	*(unsigned long*)TX_CTX_IDX0 = ((tx_cpu_owner_idx+1) % NUM_TX_DESC);
#endif
	spin_unlock_irqrestore(&ei_local->page_lock, flags);
	return 0;	
}

static int ei_start_xmit_fake(struct sk_buff* skb, struct net_device *dev)
{
	return ei_start_xmit(skb, dev, 1);
}


int ei_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
#if defined(CONFIG_RT_3052_ESW)
	esw_reg reg;
#endif
	ra_mii_ioctl_data mii;
	switch (cmd) {
		case RAETH_MII_READ:
			copy_from_user(&mii, ifr->ifr_data, sizeof(mii));
			mii_mgr_read(mii.phy_id, mii.reg_num, &mii.val_out);
			//printk("phy %d, reg %d, val 0x%x\n", mii.phy_id, mii.reg_num, mii.val_out);
			copy_to_user(ifr->ifr_data, &mii, sizeof(mii));
			break;

		case RAETH_MII_WRITE:
			copy_from_user(&mii, ifr->ifr_data, sizeof(mii));
			//printk("phy %d, reg %d, val 0x%x\n", mii.phy_id, mii.reg_num, mii.val_in);
			mii_mgr_write(mii.phy_id, mii.reg_num, mii.val_in);
			break;
#if defined(CONFIG_RT_3052_ESW)
#define _ESW_REG(x)	(*((volatile u32 *)(RALINK_ETH_SW_BASE + x)))
		case RAETH_ESW_REG_READ:
			copy_from_user(&reg, ifr->ifr_data, sizeof(reg));
			if (reg.off > REG_ESW_MAX)
				return -EINVAL;
			reg.val = _ESW_REG(reg.off);
			//printk("read reg off:%x val:%x\n", reg.off, reg.val);
			copy_to_user(ifr->ifr_data, &reg, sizeof(reg));
			break;
		case RAETH_ESW_REG_WRITE:
			copy_from_user(&reg, ifr->ifr_data, sizeof(reg));
			if (reg.off > REG_ESW_MAX)
				return -EINVAL;
			_ESW_REG(reg.off) = reg.val;
			//printk("write reg off:%x val:%x\n", reg.off, reg.val);
			break;
#endif // CONFIG_RT_3052_ESW
	}
	return 0;
}

/*
 * Set new MTU size
 * Change the mtu of Raeth Ethernet Device
 */
static int ei_change_mtu(struct net_device *dev, int new_mtu)
{
	unsigned long flags;
	END_DEVICE *ei_local = netdev_priv(dev);  // get priv ei_local pointer from net_dev structure

	if ( ei_local == NULL ) {
		printk(KERN_EMERG "%s: ei_change_mtu passed a non-existent private pointer from net_dev!\n", dev->name);
		return -ENXIO;
	}

	spin_lock_irqsave(&ei_local->page_lock, flags);

	if ( (new_mtu > 4096) || (new_mtu < 64)) {
		spin_unlock_irqrestore(&ei_local->page_lock, flags);
		return -EINVAL;
	}

#ifndef CONFIG_RAETH_JUMBOFRAME
	if ( new_mtu > 1500 ) {
		spin_unlock_irqrestore(&ei_local->page_lock, flags);
		return -EINVAL;
	}
#endif

	raeth_mtu = dev->mtu = new_mtu;

	spin_unlock_irqrestore(&ei_local->page_lock, flags);
	return 0;
}

void ra2880_setup_dev_fptable(struct net_device *dev)
{
	RAETH_PRINT(__FUNCTION__ "is called!\n");
	dev->open		= ei_open;
	dev->stop		= ei_close;
	dev->hard_start_xmit	= ei_start_xmit_fake;
	dev->tx_timeout		= ei_tx_timeout;
	dev->get_stats		= ra_get_stats;
	dev->set_mac_address	= ei_set_mac_addr;
	dev->change_mtu		= ei_change_mtu;

	if ((raeth_mtu < MAX_RX_LENGTH ) && (raeth_mtu > 0))
		dev->mtu = raeth_mtu;
	else
		dev->mtu = MAX_RX_LENGTH;

#ifdef CONFIG_RAETH_NAPI
        dev->poll = &raeth_clean;
#if defined (CONFIG_RAETH_ROUTER)
	dev->weight = 32;
#elif defined (CONFIG_RT_3052_ESW)
	dev->weight = 32;
#else
	dev->weight = 128;
#endif
#endif

#if defined (CONFIG_ETHTOOL) && ( defined (CONFIG_RAETH_ROUTER) || defined (CONFIG_RT_3052_ESW) )
	dev->ethtool_ops	= &ra_ethtool_ops;
#endif
	dev->do_ioctl		= ei_ioctl;
}


#define TX_TIMEOUT (20*HZ/100)
void ei_tx_timeout(struct net_device *dev)
{
	END_DEVICE* ei_local = netdev_priv(dev); 

	tasklet_schedule(&ei_local->tx_tasklet);
	return;
}

void setup_statistics(END_DEVICE* ei_local)
{
	ei_local->stat.tx_packets	= 0;
	ei_local->stat.tx_bytes 	= 0;
	ei_local->stat.tx_dropped 	= 0;
	ei_local->stat.tx_errors	= 0;
	ei_local->stat.tx_aborted_errors= 0;
	ei_local->stat.tx_carrier_errors= 0;
	ei_local->stat.tx_fifo_errors	= 0;
	ei_local->stat.tx_heartbeat_errors = 0;
	ei_local->stat.tx_window_errors	= 0;
	
	ei_local->stat.rx_packets	= 0;
	ei_local->stat.rx_bytes 	= 0;
	ei_local->stat.rx_dropped 	= 0;	
	ei_local->stat.rx_errors	= 0;
	ei_local->stat.rx_length_errors = 0;
	ei_local->stat.rx_over_errors	= 0;
	ei_local->stat.rx_crc_errors	= 0;
	ei_local->stat.rx_frame_errors	= 0;
	ei_local->stat.rx_fifo_errors	= 0;
	ei_local->stat.rx_missed_errors	= 0;

	ei_local->stat.collisions	= 0;
#if defined (CONFIG_RAETH_QOS)
	ei_local->tx3_full = 0;
	ei_local->tx2_full = 0;
	ei_local->tx1_full = 0;
	ei_local->tx0_full = 0;
#else
	ei_local->tx_full = 0;
#endif
#ifdef CONFIG_RAETH_NAPI
	atomic_set(&ei_local->irq_sem, 1);
#endif
	
}

/**
 * rather_probe - pick up ethernet port at boot time
 * @dev: network device to probe
 *
 * This routine probe the ethernet port at boot time.
 *
 *
 */

int __init rather_probe(struct net_device *dev)
{
	int i;
        unsigned int regValue = 0;
        END_DEVICE *ei_local = netdev_priv(dev);
	struct sockaddr addr;
	unsigned char *mac_addr;
	unsigned char zero[6]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

	SET_MODULE_OWNER(dev);
	ra_ei_local = ei_local;

	dev->base_addr = RA2882ETH_BASE;
        regValue |= FE_RESET_BIT;
        sysRegWrite(FE_RESET, regValue);
        sysRegWrite(FE_RESET, 0);

        /* receiving packet buffer allocation - NUM_RX_DESC x MAX_RX_LENGTH */
        for ( i = 0; i < NUM_RX_DESC; i++)
        {      		
                netrx_skbuf[i] = NULL;
        }	// kmalloc

	
	//Get mac0 address from flash
	mac_addr = (unsigned char *)GMAC0_ADDR;
	memcpy(addr.sa_data, mac_addr, 6); 

	//If mac0 is empty, random generate mac address
	if (memcmp(addr.sa_data, zero, 6) == 0) //mac0 address is empty
	{
          unsigned char mac_addr01234[5]={ 0x00, 0x0C, 0x43, 0x28, 0x80};
          net_srandom(jiffies);
	  memcpy(addr.sa_data, mac_addr01234, 5);
          addr.sa_data[5] = net_random()&0xFF;
	}

	ei_set_mac_addr(dev, &addr);
	spin_lock_init(&ei_local->page_lock);	
	ether_setup(dev);
	ra2880_setup_dev_fptable(dev);


	dev->tx_timeout = ei_tx_timeout;
	dev->watchdog_timeo = TX_TIMEOUT;

	setup_statistics(ei_local);

	for ( i = 0; i < NUM_RX_DESC; i++)
		netrx_skbuf[i] = NULL;

	return 0;
}

void ei_xmit_housekeeping(unsigned long unused)
{
    struct net_device *dev = dev_raether;
    END_DEVICE *ei_local = netdev_priv(dev);
    struct PDMA_txdesc *tx_desc;
    unsigned long skb_free_idx;
    unsigned long tx_dtx_idx;

#ifdef CONFIG_RAETH_QOS
    int i;
    for (i=0;i<NUM_TX_RINGS;i++){
        skb_free_idx = ei_local->free_idx[i];
    	if((ei_local->skb_free[i][skb_free_idx])==0){
		continue;
	}

	get_tx_desc_and_dtx_idx(ei_local, i, &tx_dtx_idx, &tx_desc);

	while(tx_desc[skb_free_idx].txd_info2.DDONE_bit==1 && (ei_local->skb_free[i][skb_free_idx])!=0 ){
	
	    dev_kfree_skb_irq((ei_local->skb_free[i][skb_free_idx]));
	    ei_local->skb_free[i][skb_free_idx]=0;
	    skb_free_idx++;
	    if(skb_free_idx >= NUM_TX_DESC)
                    skb_free_idx =0;
	}
	ei_local->free_idx[i] = skb_free_idx;
    }
#else
	tx_dtx_idx = *(unsigned long*)TX_DTX_IDX0;
	tx_desc = ei_local->tx_ring0;
	skb_free_idx = ei_local->free_idx;
	if ((ei_local->skb_free[skb_free_idx]) != 0) {
		while(tx_desc[skb_free_idx].txd_info2.DDONE_bit==1 && (ei_local->skb_free[skb_free_idx])!=0 ){
			dev_kfree_skb_irq((ei_local->skb_free[skb_free_idx]));
			ei_local->skb_free[skb_free_idx]=0;
			skb_free_idx++;
			if(skb_free_idx >= NUM_TX_DESC)
       				skb_free_idx =0;
		}
		ei_local->free_idx = skb_free_idx;
	}  /* if skb_free != 0 */
#endif	

#ifndef CONFIG_RAETH_NAPI
    unsigned long reg_int_mask=0;
    reg_int_mask=sysRegRead(FE_INT_ENABLE);
#if defined (DELAY_INT)
    sysRegWrite(FE_INT_ENABLE, reg_int_mask|RT2880_TX_DLY_INT);
#else
    
    sysRegWrite(FE_INT_ENABLE, reg_int_mask|RT2880_TX_DONE_INT0	\
		    			   |RT2880_TX_DONE_INT1 \
					   |RT2880_TX_DONE_INT2 \
					   |RT2880_TX_DONE_INT3);
#endif
#endif //CONFIG_RAETH_NAPI//
}

#if 0 //moved to scripts
#ifdef CONFIG_WAN_AT_P4
void config_ICPLUS_175C_eva(void)
{
	//MII0 doesn't strip the tag of an outgoing packet.
	//MII0 adds a tag to an outgoing packet.
	//Port0~4 strip the tag of an outgoing packet.
	//Port0~4 doesn't add a tag to an outgoing packet.
	mii_mgr_write(29, 23, 0x07c2);
	//setup port 0, 1 vlan group
	//mii_mgr_write(29, 19, 0x8f8f);
	//setup port 2, 3 vlan group
	//mii_mgr_write(29, 20, 0x8f8f);
	//setup port 4 vlan group
	//mii_mgr_write(29, 21, 0x8000);
	//setup default vlan tag value port0~4
	//force cpu port is 100F
	mii_mgr_write(29, 22, 0x8420);
	mii_mgr_write(29, 24, 0x1);
	mii_mgr_write(29, 25, 0x1);
	mii_mgr_write(29, 26, 0x1);
	mii_mgr_write(29, 27, 0x1);
	mii_mgr_write(29, 28, 0x2);

	//for unknow mac flooding to all ports.
	//this will make flooding to the same PVID ports
	mii_mgr_write(30, 9, 0x1081); 
	mii_mgr_write(30, 1, 0x2f00);
	mii_mgr_write(30, 2, 0x0030);
	printk("ICPLUS 175C config LAN_WAN finished (evaluation board)\n");
}
#endif

#ifdef CONFIG_WAN_AT_P0
void config_ICPLUS_175C_demo(void)
{
	//MII0 doesn't strip the tag of an outgoing packet.
	//MII0 adds a tag to an outgoing packet.
	//Port0~4 strip the tag of an outgoing packet.
	//Port0~4 doesn't add a tag to an outgoing packet.
	mii_mgr_write(29, 23, 0x07c2);
	//force cpu port is 100F
	mii_mgr_write(29, 22, 0x8420);
	//set port 0 to vlan 2 (wan)
	mii_mgr_write(29, 24, 0x2);
	//set port 1~4 to vlan 1 (lan)
	mii_mgr_write(29, 25, 0x1);
	mii_mgr_write(29, 26, 0x1);
	mii_mgr_write(29, 27, 0x1);
	mii_mgr_write(29, 28, 0x1);

	//for unknow mac flooding to all ports.
	//this will make flooding to the same PVID ports
	mii_mgr_write(30, 9, 0x0181); 
	mii_mgr_write(30, 1, 0x3e00);
	mii_mgr_write(30, 2, 0x0021);
	printk("ICPLUS 175C config LAN_WAN finished (demo board)\n");
}
#endif
#endif

#ifdef CONFIG_PSEUDO_SUPPORT
int VirtualIF_ioctl(struct net_device * net_dev,
		    struct ifreq * rq, int cmd)
{
	return 0;
}

int VirtualIF_open(struct net_device * dev)
{
    PSEUDO_ADAPTER *pPesueoAd = dev->priv;


    *(unsigned long *)MDIO_CFG2= 0x3F000000; //auto-polling
    printk("%s: ===> VirtualIF_open\n", dev->name);

    netif_start_queue(pPesueoAd->PseudoDev);

    return 0;
}

int VirtualIF_close(struct net_device * dev)
{
    PSEUDO_ADAPTER *pPesueoAd = dev->priv;

    printk("%s: ===> VirtualIF_close\n", dev->name);

    netif_stop_queue(pPesueoAd->PseudoDev);

    return 0;
}

int VirtualIFSendPackets(struct sk_buff * pSkb,
			 struct net_device * dev)
{
    PSEUDO_ADAPTER *pPesueoAd = dev->priv;

    //printk("VirtualIFSendPackets --->\n");

    if (!(pPesueoAd->RaethDev->flags & IFF_UP)) {
	dev_kfree_skb_any(pSkb);
	return 0;
    }
    //pSkb->cb[40]=0x5a;
    pSkb->dev = pPesueoAd->RaethDev;
    ei_start_xmit(pSkb, pPesueoAd->RaethDev, 2);
    return 0;
}


// Register pseudo interface
void RAETH_Init_PSEUDO(pEND_DEVICE pAd, struct net_device *net_dev)
{
    int index;
    struct net_device *dev;
    PSEUDO_ADAPTER *pPseudoAd;
    int i = 0;
    char slot_name[16];
    struct net_device *device;
    unsigned char *mac_addr;
    struct sockaddr addr;
    unsigned char zero[6]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

    for (index = 0; index < MAX_PSEUDO_ENTRY; index++) {

	dev = alloc_etherdev(sizeof(PSEUDO_ADAPTER));

	{			// find available 
	    for (i = 3; i < 32; i++) {
		sprintf(slot_name, "eth%d", i);

		for (device = dev_base; device != NULL;
		     device = device->next) {
		    if (strncmp(device->name, slot_name, 4) == 0) {
			break;
		    }
		}
		if (device == NULL)
		    break;
	    }

	    if (i != 32) {
		sprintf(dev->name, "eth%d", i);
	    } else {
		printk("Ethernet interface number overflow...\n");
		break;
	    }
	}

	//Get mac2 address from flash
	mac_addr = (unsigned char *)GMAC2_ADDR;
	memcpy(addr.sa_data, mac_addr, 6); 

	//If mac2 is empty, random generate mac address
	if (memcmp(addr.sa_data, zero, 6) == 0) //mac2 address is empty
	{
          unsigned char mac_addr01234[5]={ 0x00, 0x0C, 0x43, 0x28, 0x80};
          net_srandom(jiffies);
	  memcpy(addr.sa_data, mac_addr01234, 5);
          addr.sa_data[5] = net_random()&0xFF;
	}

	ei_set_mac2_addr(dev, &addr);
	ether_setup(dev);
	pPseudoAd = dev->priv;

	pPseudoAd->PseudoDev = dev;
	pPseudoAd->RaethDev = net_dev;
	pAd->PseudoDev = dev;

	memcpy(&dev->dev_addr, &net_dev->dev_addr, 6);

	dev->hard_start_xmit = VirtualIFSendPackets;
	dev->stop = VirtualIF_close;
	dev->open = VirtualIF_open;
	dev->do_ioctl = VirtualIF_ioctl;
	dev->set_mac_address	= ei_set_mac2_addr;

	// Register this device
	register_netdevice(dev);
    }
}
#endif



/**
 * ei_open - Open/Initialize the ethernet port.
 * @dev: network device to initialize
 *
 * This routine goes all-out, setting everything
 * up a new at each open, even though many of these registers should only need to be set once at boot.
 */
int ei_open(struct net_device *dev)
{
	int i;
	unsigned long flags;
	END_DEVICE *ei_local;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)
	if (!try_module_get(THIS_MODULE))
	{
		printk("%s: Cannot reserve module\n", __FUNCTION__);
		return -1;
	}
#else
	MOD_INC_USE_COUNT;
#endif

  	ei_local = netdev_priv(dev); // get device pointer from System
	// unsigned int flags;

	if (ei_local == NULL)
	{
		printk(KERN_EMERG "%s: ei_open passed a non-existent device!\n", dev->name);
		return -ENXIO;
	}

        /* receiving packet buffer allocation - NUM_RX_DESC x MAX_RX_LENGTH */
        for ( i = 0; i < NUM_RX_DESC; i++)
        {
                netrx_skbuf[i] = dev_alloc_skb(MAX_RX_LENGTH+2);
                if (netrx_skbuf[i] == NULL )
                        printk("rx skbuff buffer allocation failed!");
#ifdef HEADER_ALIGNED
                skb_reserve(netrx_skbuf[i], 2);
#endif
        }       // kmalloc

	spin_lock_irqsave(&(ei_local->page_lock), flags);
	request_irq( dev->irq, ei_interrupt, SA_INTERRUPT, dev->name, dev);	// try to fix irq in open
	rt2880_eth_setup(dev);

#if defined (CONFIG_RALINK_RT3052)
	*((volatile u32 *)(RALINK_INTCL_BASE + 0x34)) = (1<<17); //INTENA: Interrupt enabled for ESW
	*((volatile u32 *)(RALINK_ETH_SW_BASE + 0x04)) &= ~(PORT_ST_CHG); //IMR: Port status change
	request_irq(SURFBOARDINT_ESW, esw_interrupt, SA_INTERRUPT, "Ralink_ESW", NULL);	
#endif


#ifdef DELAY_INT
#ifdef CONFIG_RAETH_NAPI 
	sysRegWrite(DLY_INT_CFG, DELAY_INT_INIT_NAPI);
#else /*CONFIG_RAETH_NAPI*/
	sysRegWrite(DLY_INT_CFG, DELAY_INT_INIT);
#endif /*CONFIG_RAETH_NAPI*/
    	sysRegWrite(FE_INT_ENABLE, FE_INT_DLY_INIT);
#else
    	sysRegWrite(FE_INT_ENABLE, FE_INT_ALL);
#endif

	tasklet_init(&ei_local->tx_tasklet, ei_xmit_housekeeping , 0);
	tasklet_init(&ei_local->rx_tasklet, ei_receive, 0);

	netif_start_queue(dev);

#ifdef CONFIG_RAETH_NAPI
	atomic_dec(&ei_local->irq_sem);
        netif_poll_enable(dev);
#endif

	spin_unlock_irqrestore(&(ei_local->page_lock), flags);
#ifdef CONFIG_PSEUDO_SUPPORT 
	if(ei_local->PseudoDev==NULL) {
	    RAETH_Init_PSEUDO(ei_local, dev);
	}

	VirtualIF_open(ei_local->PseudoDev);
#endif
	forward_config(dev);
	return 0;
}

/**
 * ei_close - shut down network device
 * @dev: network device to clear
 *
 * This routine shut down network device.
 *
 *
 */
int ei_close(struct net_device *dev)
{
	int i;
	END_DEVICE *ei_local = netdev_priv(dev);	// device pointer
	unsigned int flags;
	spin_lock_irqsave(&(ei_local->page_lock), flags);

#ifdef CONFIG_PSEUDO_SUPPORT 
	VirtualIF_close(ei_local->PseudoDev);
#endif

	netif_stop_queue(dev);
	ra2880stop(ei_local);
	
	tasklet_kill(&ei_local->tx_tasklet);
	tasklet_kill(&ei_local->rx_tasklet);

        for ( i = 0; i < NUM_RX_DESC; i++)
        {
                if (netrx_skbuf[i] != NULL) {
                        dev_kfree_skb(netrx_skbuf[i]);
			netrx_skbuf[i] = NULL;
		}
        }       // dev_kfree_skb


#if defined (CONFIG_RAETH_QOS)
       if (ei_local->tx_ring0 != NULL) {
	   pci_free_consistent(NULL, NUM_TX_DESC*sizeof(struct PDMA_txdesc), ei_local->tx_ring0, ei_local->phy_tx_ring0);
       }

       if (ei_local->tx_ring1 != NULL) {
	   pci_free_consistent(NULL, NUM_TX_DESC*sizeof(struct PDMA_txdesc), ei_local->tx_ring1, ei_local->phy_tx_ring1);
       }
       
#if defined (CONFIG_RALINK_RT2883) || defined (CONFIG_RALINK_RT3052)
       if (ei_local->tx_ring2 != NULL) {
	   pci_free_consistent(NULL, NUM_TX_DESC*sizeof(struct PDMA_txdesc), ei_local->tx_ring2, ei_local->phy_tx_ring2);
       }
       
       if (ei_local->tx_ring3 != NULL) {
	   pci_free_consistent(NULL, NUM_TX_DESC*sizeof(struct PDMA_txdesc), ei_local->tx_ring3, ei_local->phy_tx_ring3);
       }
#endif
        pci_free_consistent(NULL, NUM_RX_DESC*sizeof(struct PDMA_rxdesc), rx_ring, phy_rx_ring);
	printk("Free TX/RX Ring Memory!\n");
#endif
	free_irq(dev->irq, dev);
	//rt2880_gmac_hard_reset();

#ifdef CONFIG_RAETH_NAPI
	atomic_inc(&ei_local->irq_sem);
        netif_poll_disable(dev);
#endif
	spin_unlock_irqrestore(&(ei_local->page_lock), flags);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)
	module_put(THIS_MODULE);
#else
	MOD_DEC_USE_COUNT;
#endif
	return 0;
}

#if defined (CONFIG_RALINK_RT3052) 
void rt305x_esw_init(void)
{
	u32 i=0;
	u32 my_val;

        *(unsigned long *)(0xb0110008) = 0xC8A07850;
        *(unsigned long *)(0xb01100E4) = 0x00000000;
        *(unsigned long *)(0xb0110014) = 0x00405555;
        *(unsigned long *)(0xb0110050) = 0x00002001;
        *(unsigned long *)(0xb0110090) = 0x00007f7f;
        *(unsigned long *)(0xb0110098) = 0x00007f3f; //disable VLAN
        *(unsigned long *)(0xb01100CC) = 0x00d6500c;
        *(unsigned long *)(0xb011009C) = 0x0008a301; //hashing algorithm=XOR48, aging interval=300sec
        *(unsigned long *)(0xb011008C) = 0x02404040;
#if defined (CONFIG_RT3052_ASIC)
        *(unsigned long *)(0xb01100C8) = 0x3f502b28; //Change polling Ext PHY Addr=0x1F
        *(unsigned long *)(0xb0110084) = 0x00000000;
#elif defined (CONFIG_RT3052_FPGA)
        *(unsigned long *)(0xb01100C8) = 0x20f02b28; //Change polling Ext PHY Addr=0x0
        *(unsigned long *)(0xb0110084) = 0xffdf1f00;

	/* In order to use 10M/Full on FPGA board. We configure phy capable to
	 * 10M Full/Half duplex, so we can use auto-negotiation on PC side */
        for(i=0;i<4;i++){
	    mii_mgr_write(i, 4, 0x0461);   //Capable of 10M Full/Half Duplex, flow control on/off
	    mii_mgr_write(i, 0, 0xB100);   //reset all digital logic, except phy_reg
	}
#endif // RT3052_ASIC_BOARD

        mii_mgr_write(0, 31, 0x8000);   //---> select local register
        for(i=0;i<5;i++){
                mii_mgr_write(i, 26, 0x1600);   //TX10 waveform coefficient
                mii_mgr_write(i, 29, 0x7058);   //TX100/TX10 AD/DA current bias
                mii_mgr_write(i, 30, 0x0018);   //TX100 slew rate control
        }
        /* PHY IOT */
        //mii_mgr_write(0, 31, 0x0);      //select global register
        //mii_mgr_write(0, 22, 0x052f);   //tune TP_IDL tail and head waveform
        //mii_mgr_write(0, 17, 0x0fe0);   //set TX10 signal amplitude threshold to minimum
        //mii_mgr_write(0, 18, 0x40ba);   //set squelch amplitude to higher threshold
        //mii_mgr_write(0, 14, 0x65);     //longer TP_IDL tail length
        //mii_mgr_write(0, 31, 0x8000);   //select local register

         mii_mgr_write(0, 31, 0x0);   //select global register
        mii_mgr_write(0, 1, 0x4a40); //enlarge agcsel threshold 3 and threshold 2
        mii_mgr_write(0, 2, 0x6254); //enlarge agcsel threshold 5 and threshold 4
        mii_mgr_write(0, 3, 0xa17f); //enlarge agcsel threshold 6
        mii_mgr_write(0, 12, 0x7eaa); //100% link down power saving & tx10 link up 50%
        mii_mgr_write(0, 14, 0x65);   //longer TP_IDL tail length
        mii_mgr_write(0, 16, 0x0684);  //increased squelch pulse count threshold.
        mii_mgr_write(0, 17, 0x0fe0); //set TX10 signal amplitude threshold to minimum
        mii_mgr_write(0, 18, 0x40ba); //set squelch amplitude to higher threshold
        mii_mgr_write(0, 22, 0x252f); //tune TP_IDL tail and head waveform, enable power down slew rate control
        mii_mgr_write(0, 27, 0x2fc3); //set PLL/Receive bias current are calibrated(RT3350)
        mii_mgr_write(0, 28, 0xc410); //change PLL/Receive bias current to internal(RT3350)
        mii_mgr_write(0, 29, 0x598b); //change PLL bias current to internal(RT3052_MP3)
        mii_mgr_write(0, 31, 0x8000); //select local register
        for(i=0;i<5;i++){
                //LSB=1 enable PHY
                mii_mgr_read(i, 26, &my_val);
                my_val |= 0x0001;
                mii_mgr_write(i, 26, my_val);
        }
#if defined (CONFIG_P5_RGMII_TO_MAC_MODE)
	*(unsigned long *)(0xb0000060) &= ~(1 << 9); //set RGMII to Normal mode
        *(unsigned long *)(0xb01100C8) &= ~(1<<29); //disable port 5 auto-polling
        *(unsigned long *)(0xb01100C8) |= 0x3fff; //force 1000M full duplex
        *(unsigned long *)(0xb01100C8) &= ~(0xf<<20); //rxclk_skew, txclk_skew = 0
#elif defined (CONFIG_P5_MII_TO_MAC_MODE)
	*(unsigned long *)(0xb0000060) &= ~(1 << 9); //set RGMII to Normal mode
        *(unsigned long *)(0xb01100C8) &= ~(1<<29); //disable port 5 auto-polling
        *(unsigned long *)(0xb01100C8) |= 0x3ffd; //force 100M full duplex
#elif defined (CONFIG_P5_MAC_TO_PHY_MODE)
	*(unsigned long *)(0xb0000060) &= ~(1 << 9); //set RGMII to Normal mode
	enable_auto_negotiate();
        if (isMarvellGigaPHY()) {
                printk("\n MARVELL Phy\n");
                mii_mgr_write(31,20,0x0ce0);
                mii_mgr_write(31,0,0x9140);
        }

#elif defined (CONFIG_P5_RMII_TO_MAC_MODE)
	*(unsigned long *)(0xb0000060) &= ~(1 << 9); //set RGMII to Normal mode
	/* reserved */
#else // Port 5 Disabled //
        *(unsigned long *)(0xb0000060) |= (1 << 9); //set RGMII to GPIO mode (GPIO41-GPIO50)
        *(unsigned long *)(0xb0000674) = 0xFFF; //GPIO41-GPIO50 output mode
        *(unsigned long *)(0xb0000670) = 0x0; //GPIO41-GPIO50 output low
#endif // CONFIG_P5_RGMII_TO_MAC_MODE //

}
#endif

/**
 * ra2882eth_init - Module Init code
 *
 * Called by kernel to register net_device
 *
 */
int __init ra2882eth_init(void)
{
	int ret;
	struct net_device *dev = alloc_etherdev(sizeof(END_DEVICE));

	if (!dev)
		return -ENOMEM;

#if defined (CONFIG_RTL8366_SWITCH)
extern int rtl_smi_init(void);
        rtl_smi_init();
        udelay(500);
#endif


	strcpy(dev->name, DEV_NAME);
	dev->irq  = RT2880_IRQ_ENET0;
	dev->addr_len = 6;
	dev->base_addr = RA2882_ENET0;	

	dev->init =  rather_probe;
	//dev->mtu  = MAX_RX_LENGTH; --> use default MTU

	/* net_device structure Init */
	hard_init(dev);
	printk("Ralink APSoC Ethernet Driver Initilization. %s  %d rx/tx descriptors allocated, mtu = %d!\n", RAETH_VERSION, NUM_RX_DESC, dev->mtu);
#ifdef CONFIG_RAETH_NAPI
	printk("NAPI enable, weight = %d, Tx Ring = %d, Rx Ring = %d\n", dev->weight, NUM_TX_DESC, NUM_RX_DESC);
#endif

	/* Register net device for the driver */
	if ( register_netdev(dev) != 0) {
        	printk(KERN_WARNING " " __FILE__ ": No ethernet port found.\n");
        	return -ENXIO;
	}

#ifdef CONFIG_RAETH_NETLINK
	csr_netlink_init();
#endif
	ret = debug_proc_init();

// RT2880 + GigaPhy
#if defined (CONFIG_GIGAPHY) 
        unsigned int regValue = 0;


        if (isMarvellGigaPHY2()) {
        mii_mgr_read(0x14,0, &regValue);
        regValue &= ~(1<<11); //power up
        mii_mgr_write(0x14,0, regValue);

	u32 regValue = sysRegRead(MDIO_CFG);
        u32 addr = 0x14;
        regValue &= 0xe0ff7fff;                 // clear auto polling related field:
                                                // (MD_PHY1ADDR & GP1_FRC_EN).
        regValue |= 0x20000000;                 // force to enable MDC/MDIO auto polling.
        regValue |= (addr << 24);               // setup PHY address for auto polling.
        
	sysRegWrite(MDIO_CFG, regValue);

    	wait_linkup();
    	wait_an_completed();
	rt2880_mdio_cfg(0,15,1);
	mii_mgr_write(0x14,22,0x0003);
	mii_mgr_write(0x14,16,0x1011);
	mii_mgr_write(0x14,22,0x0000);
	
	}

        if (isMarvellGigaPHY()) {
    		enable_auto_negotiate();
                printk("\n Reset MARVELL phy\n");
                mii_mgr_read(31,20, &regValue);
                regValue |= 1<<7; //Add delay to RX_CLK for RXD Outputs
                mii_mgr_write(31,20, regValue);

                mii_mgr_read(31,0, &regValue);
                regValue |= 1<<15; //PHY Software Reset
                mii_mgr_write(31,0, regValue);
        }

// RT3052 + EmbeddedSW
#elif defined (CONFIG_RT_3052_ESW) 
	rt305x_esw_init();
// RT2880 + GigaSW
#elif defined (CONFIG_MAC_TO_MAC_MODE)
        // force cpu port is 1000F
	sysRegWrite(MDIO_CFG, 0x1F01DC01);

// RT2880 + 100PHY
#elif defined (CONFIG_RTL8366_SWITCH)

        printk("Rtl8366 Phy Init...\n");
        sysRegWrite(MDIO_CFG, 0x0000dc01);

#elif defined (CONFIG_RAETH_ROUTER) || defined (CONFIG_ICPLUS_PHY)

	sysRegWrite(MDIO_CFG, INIT_VALUE_OF_ICPLUS_PHY_INIT_VALUE);

#if defined (CONFIG_RAETH_ROUTER)
#ifdef CONFIG_RALINK_VISTA_BASIC
	int sw_id=0;
	mii_mgr_read(29, 31, &sw_id);
	if (sw_id == 0x175c) {
	    is_switch_175c = 1;
	} else {
	    is_switch_175c = 0;
	}
#endif // CONFIG_RALINK_VISTA_BASIC

        // due to the flaws of RT2880 GMAC implementation (or IC+ SW ?) we use the
        // fixed capability instead of auto-polling.
        // force cpu port is 100F
        mii_mgr_write(29, 22, 0x8420);
#endif // CONFIG_RAETH_ROUTER //
#endif // CONFIG_GIGAPHY //  
	
	dev_raether = dev;
	return ret;
}

/**
 * ra2882eth_cleanup_module - Module Exit code
 *
 * Cmd 'rmmod' will invode the routine to exit the module
 *
 */
void ra2882eth_cleanup_module(void)
{
	int i;
	struct net_device *dev = dev_raether;
	END_DEVICE *ei_local;

	if (dev->priv != NULL)
	{
		ei_local = netdev_priv(dev);
		if ( ei_local->MACInfo != NULL )
		{
			RAETH_PRINT("Free MACInfo...\n");
			kfree(ei_local->MACInfo);

		} 
		else
			RAETH_PRINT("MACInfo is null\n");	

#ifdef CONFIG_PSEUDO_SUPPORT
		kfree(ei_local->PseudoDev->priv);
		unregister_netdev(ei_local->PseudoDev);
#endif
		kfree(dev->priv);
		unregister_netdev(dev);
		RAETH_PRINT("Free ei_local and unregister netdev...\n");
  	} /* dev->priv */
        for ( i = 0; i < NUM_RX_DESC; i++)
        {
                if (netrx_skbuf[i] != NULL) {
                        dev_kfree_skb(netrx_skbuf[i]);
                        netrx_skbuf[i] = NULL;
                }
        }       // dev_kfree_skb

	free_netdev(dev);
	debug_proc_exit();
#ifdef CONFIG_RAETH_NETLINK
	csr_netlink_end();
#endif
}

late_initcall(ra2882eth_init);
module_exit(ra2882eth_cleanup_module);
MODULE_LICENSE("GPL");

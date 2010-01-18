/**************************************************************************
* Copyright 2006 StorLink Semiconductors, Inc.  All rights reserved.
*--------------------------------------------------------------------------
* Name			: sl351x_toe.c
* Description	:
*		Provide TOE routines for SL351x
*
* History
*
*	Date		Writer		Description
*----------------------------------------------------------------------------
*				Xiaochong
*
****************************************************************************/

#include <linux/pci.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/tcp.h>
#include <linux/slab.h>
#include <linux/etherdevice.h>
#include <asm/io.h>
#include <linux/sysctl_storlink.h>
#include <net/tcp.h>
#include <linux/if_ether.h>
#include <asm/arch/sl351x_gmac.h>
#include <asm/arch/sl351x_toe.h>
#include <asm/arch/sl351x_hash_cfg.h>
#include <asm/arch/sl351x_nat_cfg.h>

static int in_toe_isr;
static int toe_initialized=0;

static struct toe_conn	toe_connections[TOE_TOE_QUEUE_NUM];
EXPORT_SYMBOL(toe_connections);
static __u32 toe_connection_bits[TOE_TOE_QUEUE_NUM/32] __attribute__ ((aligned(16)));
struct sk_buff* gen_pure_ack(struct toe_conn* connection, TOE_QHDR_T* toe_qhdr, INTR_QHDR_T *intr_curr_desc);

extern struct storlink_sysctl	storlink_ctl;
extern TOE_INFO_T toe_private_data;
extern spinlock_t gmac_fq_lock;
extern void mac_write_dma_reg(int mac, unsigned int offset, u32 data);
extern int mac_set_rule_reg(int mac, int rule, int enabled, u32 reg0, u32 reg1, u32 reg2);
extern int hash_add_toe_entry(HASH_ENTRY_T *entry);
extern void toe_gmac_fill_free_q(void);

#define _DEBUG_SKB_		1
#ifdef _DEBUG_SKB_
/*---------------------------------------------------------------------------
 * _debug_skb
 *-------------------------------------------------------------------------*/
static inline void _debug_skb(struct sk_buff *skb, GMAC_RXDESC_T *toe_curr_desc, u32 data)
{
	if ((u32)skb < 0x1000)
	{
		printk("%s skb=%x\n", __func__, (u32)skb);
		while(1);
	}
	REG32(__va(toe_curr_desc->word2.buf_adr)-SKB_RESERVE_BYTES) = data;
}
#else
#define _debug_skb(x, y, z)
#endif

/*---------------------------------------------------------------------------
 * get_connection_seq_num
 *-------------------------------------------------------------------------*/
u32 get_connection_seq_num(unsigned short qid)
{
	TOE_QHDR_T	*toe_qhdr;

	toe_qhdr = (TOE_QHDR_T*)TOE_TOE_QUE_HDR_BASE;
	toe_qhdr += qid;
	return (u32)toe_qhdr->word3.seq_num;
}
EXPORT_SYMBOL(get_connection_seq_num);

/*---------------------------------------------------------------------------
 * get_connection_ack_num
 *-------------------------------------------------------------------------*/
u32 get_connection_ack_num(unsigned short qid)
{
	TOE_QHDR_T	*toe_qhdr;

	toe_qhdr = (TOE_QHDR_T*)TOE_TOE_QUE_HDR_BASE;
	toe_qhdr += qid;
	return (u32)toe_qhdr->word4.ack_num;
}
EXPORT_SYMBOL(get_connection_ack_num);

/*---------------------------------------------------------------------------
 * dump_toe_qhdr
 *-------------------------------------------------------------------------*/
void dump_toe_qhdr(TOE_QHDR_T *toe_qhdr)
{
	printk("TOE w1 %x, w2 %x, w3 %x\n", toe_qhdr->word1.bits32,
		toe_qhdr->word2.bits32, toe_qhdr->word3.bits32);
	printk("w4 %x, w5 %x, w6 %x\n", toe_qhdr->word4.bits32,
		toe_qhdr->word5.bits32, toe_qhdr->word6.bits32);
}

/*---------------------------------------------------------------------------
 * dump_intrq_desc
 *-------------------------------------------------------------------------*/
void dump_intrq_desc(INTR_QHDR_T *intr_curr_desc)
{
	printk("INTR w0 %x, w1 %x, seq %x\n", intr_curr_desc->word0.bits32,
		intr_curr_desc->word1.bits32, intr_curr_desc->word2.bits32);
	printk("ack %x, w4 %x\n", intr_curr_desc->word3.bits32,
		intr_curr_desc->word4.bits32);
}

/*---------------------------------------------------------------------------
 * This routine will initialize a TOE matching rule
 * called by SL351x GMAC driver.
 *-------------------------------------------------------------------------*/
void sl351x_toe_init(void)
{
	GMAC_MRxCR0_T	mrxcr0;
	GMAC_MRxCR1_T	mrxcr1;
	GMAC_MRxCR2_T	mrxcr2;
	int	rule, rc;

	if (toe_initialized)
		return;

	toe_initialized = 1;

#ifndef CONFIG_SL351x_NAT
	mrxcr0.bits32 = 0;
	mrxcr1.bits32 = 0;
	mrxcr2.bits32 = 0;
	mrxcr0.bits.l3 = 1;
	mrxcr0.bits.l4 = 1;
	mrxcr1.bits.sip = 1;
	mrxcr1.bits.dip = 1;
	mrxcr1.bits.l4_byte0_15 = 0x0f;
	mrxcr0.bits.sprx = 1;
	rule = 0;
	rc = mac_set_rule_reg(0, rule, 1, mrxcr0.bits32, mrxcr1.bits32,
						mrxcr2.bits32);
	if (rc<0) {
		printk("%s::Set MAC 0 rule fail!\n", __func__);
	}
	rc = mac_set_rule_reg(1, rule, 1, mrxcr0.bits32, mrxcr1.bits32,
	     					mrxcr2.bits32);
	if (rc<0) {
		printk("%s::Set MAC 1 rule fail!\n", __func__);
	}
#endif // CONFIG_SL351x_NAT
}

/*---------------------------------------------------------------------------
 * dump_intrq_desc
 * assign an interrupt queue number to a give tcp queue
 *-------------------------------------------------------------------------*/
int get_interrupt_queue_id(int tcp_qid)
{
	return (int)(tcp_qid & 0x0003);
}

/*---------------------------------------------------------------------------
 * reset_connection_index
 * reset the connection bit by given index
 *-------------------------------------------------------------------------*/
void reset_connection_index(__u8 index)
{
	__u32 mask = ~(0xffffffff & (1<< (index&0x1f)));
	toe_connection_bits[index>>5] = toe_connection_bits[index>>5] & mask;
}

/*---------------------------------------------------------------------------
 * update_timer
 *-------------------------------------------------------------------------*/
void update_timer(struct toe_conn* connection)
{
//	if (time_before(jiffies, connection->last_rx_jiffies+3))
//	if ((jiffies + 0xffffffff - connection->last_rx_jiffies) & 0x3)
//	if (connection->last_rx_jiffies > jiffies)
//		printk("%s::jif %g, last_rx_jif %g\n", __func__, jiffies, connection->last_rx_jiffies);
/*	if ((long)(jiffies + 2)< 3) { // overflow...
		printk("%s::jiffies %x\n", __func__, jiffies);
	} */
//	if ((long)(jiffies - connection->last_rx_jiffies)< 2)
//		return;
	connection->last_rx_jiffies = jiffies;
	// gary chen mod_timer(&connection->rx_timer, jiffies+2);
	connection->rx_timer.expires = jiffies + 2;
	add_timer(&connection->rx_timer);
//	printk("%s::nt %x, lj %x\n", __func__, (jiffies+2), connection->last_rx_jiffies);
}

/*---------------------------------------------------------------------------
 * gen_pure_ack
 *-------------------------------------------------------------------------*/
struct sk_buff* gen_pure_ack(struct toe_conn* connection, TOE_QHDR_T* toe_qhdr,
INTR_QHDR_T *intr_curr_desc)
{
	struct sk_buff	*skb;
	struct iphdr	*ip_hdr;
	struct tcphdr	*tcp_hdr;
	struct ethhdr	*eth_hdr;

	if ((skb= dev_alloc_skb(RX_BUF_SIZE))==NULL) {
		printk("%s::alloc pure ack fail!\n", __func__);
		return NULL;
	}
	skb_reserve(skb, RX_INSERT_BYTES);
	memset(skb->data, 0, 60);

	eth_hdr = (struct ethhdr*)&(skb->data[0]);
	memcpy(eth_hdr, &connection->l2_hdr, sizeof(struct ethhdr));

	ip_hdr = (struct iphdr*)&(skb->data[14]);
	ip_hdr->version = connection->ip_ver;
	ip_hdr->ihl = 20>>2;
	ip_hdr->tot_len = ntohs(40);
	ip_hdr->frag_off = htons(IP_DF);
	ip_hdr->ttl = 128;
	ip_hdr->protocol = 0x06;
	ip_hdr->saddr = connection->saddr[0];
	ip_hdr->daddr = connection->daddr[0];
//	printk("%s ip sa %x, da %x\n",
//		__func__, ntohl(ip_hdr->saddr), ntohl(ip_hdr->daddr));

	tcp_hdr = (struct tcphdr*)&(skb->data[34]);
	tcp_hdr->source = connection->source;
	tcp_hdr->dest = connection->dest;
	if (intr_curr_desc) {
		tcp_hdr->seq = htonl(intr_curr_desc->word2.seq_num);
		tcp_hdr->ack_seq = htonl(intr_curr_desc->word3.ack_num);
		tcp_hdr->window = htons(intr_curr_desc->word0.bits.win_size);
	} else {
		tcp_hdr->seq = htonl(toe_qhdr->word3.seq_num);
		tcp_hdr->ack_seq = htonl(toe_qhdr->word4.ack_num);
		tcp_hdr->window = htons(toe_qhdr->word6.bits.WinSize);
	}
	tcp_hdr->ack = 1;
	tcp_hdr->doff = 20 >> 2;
#if 0
	if (!intr_curr_desc) {
		unsigned char byte;
		for (i=0; i<20; i++) {
			byte = skb->data[34+i];
			printk("%x ", byte);
		}
		printk("\n");
	}
#endif
	TCP_SKB_CB(skb)->connection = connection;
	return skb;
}

/*---------------------------------------------------------------------------
 * connection_rx_timer
 *-------------------------------------------------------------------------*/
void connection_rx_timer(unsigned long *data)
{
	struct toe_conn	*connection = (struct toe_conn*)data;
	unsigned int	tcp_qid, toeq_wptr;
	unsigned int	pkt_size, desc_count;
	struct sk_buff	*skb;
	GMAC_RXDESC_T	*toe_curr_desc;
	TOE_QHDR_T	*toe_qhdr;
	struct net_device	*dev;
	unsigned long	conn_flags;
	DMA_RWPTR_T		toeq_rwptr;
	unsigned short	timeout_descs;

	if (in_toe_isr)
		printk("%s::in_toe_isr=%d!\n", __func__, in_toe_isr);

	if (connection) {
		/* should we disable gmac interrupt first? */
		if (!connection->gmac)
			printk("%s::conn gmac %x!\n", __func__, (u32)connection->gmac);
		local_irq_save(conn_flags);
		if (!spin_trylock(&connection->conn_lock)) {
			local_irq_restore(conn_flags);
			// timer should be updated by the toeq isr. So no need to update here.
			printk("%s::conn_lock is held by ISR!\n", __func__);
			return;
		}
		disable_irq(connection->gmac->irq);

		/* disable hash entry and get toeq desc. */
		hash_set_valid_flag(connection->hash_entry_index, 0);
		do{} while(0);	/* wait until HW finish */

		dev = connection->dev;
		if (!dev)
			printk("%s::conn dev NULL!\n", __func__);
		tcp_qid = connection->qid;
		toe_qhdr = (TOE_QHDR_T *)(TOE_TOE_QUE_HDR_BASE +
		              tcp_qid * sizeof(TOE_QHDR_T));
		toeq_rwptr.bits32 = readl(&toe_qhdr->word1);
		toeq_wptr = toe_qhdr->word1.bits.wptr;
		timeout_descs = toeq_wptr - toeq_rwptr.bits.rptr;

		if (toeq_rwptr.bits.rptr == toeq_wptr) {
			if (toe_qhdr->word5.bits32) {
				// shall we check toe_qhdr->word2.bits?
				skb = gen_pure_ack(connection, toe_qhdr, (INTR_QHDR_T *)NULL);
				skb_put(skb, 54);
				skb->dev = connection->dev;
				skb->ip_summed = CHECKSUM_UNNECESSARY;
				skb->protocol = eth_type_trans(skb, connection->dev);
				netif_rx(skb);
				connection->dev->last_rx = jiffies;
			}
		} else {
			while (toeq_rwptr.bits.rptr != toeq_rwptr.bits.wptr) {
				/* we just simply send those packets to tcp? */
				toe_curr_desc = (GMAC_RXDESC_T*)(toe_private_data.toe_desc_base[tcp_qid]
					+ toeq_rwptr.bits.rptr * sizeof(GMAC_RXDESC_T));
				connection->curr_desc = toe_curr_desc;
				if (toe_curr_desc->word3.bits.ctrl_flag) {
					printk("%s::ctrl flag! %x, conn rptr %d, to %d, jif %x, conn_jif %x\n",
						__func__, toe_curr_desc->word3.bits32,
						connection->toeq_rwptr.bits.rptr, timeout_descs,
						(u32)jiffies, (u32)connection->last_rx_jiffies);
				}
				desc_count = toe_curr_desc->word0.bits.desc_count;
				pkt_size = toe_curr_desc->word1.bits.byte_count;
				consistent_sync((void*)__va(toe_curr_desc->word2.buf_adr), pkt_size,
					PCI_DMA_FROMDEVICE);
				skb = (struct sk_buff*)(REG32(__va(toe_curr_desc->word2.buf_adr)-
					SKB_RESERVE_BYTES));
				_debug_skb(skb, (GMAC_RXDESC_T *)toe_curr_desc, 0x02);
				connection->curr_rx_skb = skb;
				skb_reserve(skb, RX_INSERT_BYTES);
				skb_put(skb, pkt_size);
				skb->dev = dev;
				skb->protocol = eth_type_trans(skb, dev);
				{
					struct iphdr* ip_hdr = (struct iphdr*)&(skb->data[0]);
					if (toe_curr_desc->word3.bits.ctrl_flag)
						printk("%s::ip id %x\n", __func__, ntohs(ip_hdr->id));
				}
				skb->ip_summed = CHECKSUM_UNNECESSARY;

				netif_rx(skb);
				dev->last_rx = jiffies;
#if 0
				if ((skb = dev_alloc_skb(RX_BUF_SIZE)) == NULL) {
					printk("%s::alloc buf fail!\n", __func__);
				}
				*(unsigned int*)(skb->data) = (unsigned int)skb;
				connection->curr_rx_skb = skb;
				skb_reserve(skb, SKB_RESERVE_BYTES);
				spin_lock_irqsave(&connection->gmac->rx_mutex, flags);
				fq_rwptr.bits32 = readl(TOE_GLOBAL_BASE + GLOBAL_SWFQ_RWPTR_REG);
				if (toe_private_data.fq_rx_rwptr.bits.wptr != fq_rwptr.bits.wptr) {
					mac_stop_txdma((struct net_device*)connection->dev);
					spin_unlock_irqrestore(&connection->gmac->rx_mutex, flags);
					while(1);
				}
				fq_desc = (GMAC_RXDESC_T*)toe_private_data.swfq_desc_base + fq_rwptr.bits.wptr;
				fq_desc->word2.buf_adr = (unsigned int)__pa(skb->data);
				fq_rwptr.bits.wptr = RWPTR_ADVANCE_ONE(fq_rwptr.bits.wptr, TOE_SW_FREEQ_DESC_NUM);
				SET_WPTR(TOE_GLOBAL_BASE+GLOBAL_SWFQ_RWPTR_REG, fq_rwptr.bits.wptr);
				toe_private_data.fq_rx_rwptr.bits32 = fq_rwptr.bits32;
				spin_unlock_irqrestore(&connection->gmac->rx_mutex, flags);
#endif
//				spin_lock_irqsave(&connection->gmac->rx_mutex, flags);
				toeq_rwptr.bits.rptr = RWPTR_ADVANCE_ONE(toeq_rwptr.bits.rptr, TOE_TOE_DESC_NUM);
				SET_RPTR(&toe_qhdr->word1, toeq_rwptr.bits.rptr);
//				spin_unlock_irqrestore(&connection->gmac->rx_mutex, flags);
				connection->toeq_rwptr.bits32 = toeq_rwptr.bits32;
			}
			toeq_rwptr.bits32 = readl(&toe_qhdr->word1);
//			toe_gmac_fill_free_q();
		}
		connection->last_rx_jiffies = jiffies;
		if (connection->status != TCP_CONN_CLOSED)
			mod_timer(&connection->rx_timer, jiffies+2);
		if (connection->status != TCP_CONN_ESTABLISHED)
			printk("%s::conn status %x\n", __func__, connection->status);
		hash_set_valid_flag(connection->hash_entry_index, 1);
		enable_irq(connection->gmac->irq);
		// Gary Chen spin_unlock_irqrestore(&connection->conn_lock, conn_flags);
	}
}

/*---------------------------------------------------------------------------
 * free_toeq_descs
 *-------------------------------------------------------------------------*/
void free_toeq_descs(int qid, TOE_INFO_T *toe)
{
	void	*desc_ptr;

	desc_ptr = (void*)toe->toe_desc_base[qid];
	pci_free_consistent(NULL, TOE_TOE_DESC_NUM*sizeof(GMAC_RXDESC_T), desc_ptr,
	   (dma_addr_t)toe->toe_desc_base_dma[qid]);
	toe->toe_desc_base[qid] = 0;
}

/*---------------------------------------------------------------------------
 * set_toeq_hdr
 *-------------------------------------------------------------------------*/
void set_toeq_hdr(struct toe_conn*	connection, TOE_INFO_T* toe, struct net_device *dev)
{
	volatile TOE_QHDR_T	*toe_qhdr;
	volatile unsigned int	toeq_wptr; // toeq_rptr
	volatile GMAC_RXDESC_T	*toe_curr_desc;
	struct sk_buff	*skb;
	unsigned int	pkt_size;
	DMA_RWPTR_T	toeq_rwptr;

	if (connection->status == TCP_CONN_CLOSING) {
		connection->status = TCP_CONN_CLOSED;
		hash_set_valid_flag(connection->hash_entry_index, 0);
		// remove timer first.
		// del_timer_sync(&(connection->rx_timer));
		// check if any queued frames last time.
		toe_qhdr = (volatile TOE_QHDR_T*)TOE_TOE_QUE_HDR_BASE;
		toe_qhdr += connection->qid;
		toeq_rwptr.bits32 = readl(&toe_qhdr->word1);

		//toeq_rptr = toe_qhdr->word1.bits.rptr;
		toeq_wptr = toe_qhdr->word1.bits.wptr;
		while (toeq_rwptr.bits.rptr != toeq_wptr) {
			printk("%s::pending frames in TOE Queue before closing!\n", __func__);
			toe_curr_desc = (GMAC_RXDESC_T*)(toe->toe_desc_base[connection->qid] +
				toe_qhdr->word1.bits.rptr*sizeof(GMAC_RXDESC_T));
			connection->curr_desc = (GMAC_RXDESC_T *)toe_curr_desc;
			pkt_size = toe_curr_desc->word1.bits.byte_count;
			consistent_sync((void*)__va(toe_curr_desc->word2.buf_adr), pkt_size,
				PCI_DMA_FROMDEVICE);
			skb = (struct sk_buff*)(REG32(__va(toe_curr_desc->word2.buf_adr) -
				SKB_RESERVE_BYTES));
			_debug_skb(skb, (GMAC_RXDESC_T *)toe_curr_desc, 0x03);
			connection->curr_rx_skb = skb;
			skb_reserve(skb, RX_INSERT_BYTES);
			skb_put(skb, pkt_size);
			skb->dev = connection->dev;
			skb->protocol = eth_type_trans(skb, connection->dev);
			skb->ip_summed = CHECKSUM_UNNECESSARY;
			netif_rx(skb);
			connection->dev->last_rx = jiffies;

			toeq_rwptr.bits.rptr = RWPTR_ADVANCE_ONE(toeq_rwptr.bits.rptr, TOE_TOE_DESC_NUM);
			SET_RPTR(&toe_qhdr->word1, toeq_rwptr.bits.rptr);
		}
		free_toeq_descs(connection->qid, toe);
		// shall we re-fill free queue?

		reset_connection_index(connection->qid);
		//memset(connection, 0, sizeof(struct toe_conn));
		printk(" del timer and close connection %x, qid %d\n", (u32)connection, connection->qid);
		return;
	}
	/* enable or setup toe queue header */
	if (connection->status == TCP_CONN_CONNECTING && storlink_ctl.rx_max_pktsize) {
		volatile TOE_QHDR_T	*qhdr;
		int iq_id;
		connection->status = TCP_CONN_ESTABLISHED;
		qhdr = (volatile TOE_QHDR_T*)((unsigned int)TOE_TOE_QUE_HDR_BASE +
		               connection->qid * sizeof(TOE_QHDR_T));

		iq_id = get_interrupt_queue_id(connection->qid);
		connection->dev = dev;
		connection->gmac = dev->priv;
		connection->toeq_rwptr.bits32 = 0;

//		qhdr->word6.bits.iq_num = iq_id;
		qhdr->word6.bits.MaxPktSize = (connection->max_pktsize)>>2; // in word.
		qhdr->word7.bits.AckThreshold = connection->ack_threshold;
		qhdr->word7.bits.SeqThreshold = connection->seq_threshold;

		// init timer.
#if 1
		init_timer(&connection->rx_timer);
		connection->rx_timer.expires = jiffies + 5;
		connection->rx_timer.data = (unsigned long)connection;
		connection->rx_timer.function = (void *)&connection_rx_timer;
		add_timer(&connection->rx_timer);
		connection->last_rx_jiffies = jiffies;
		printk("init_timer %x\n", (u32)jiffies);
#endif
		hash_set_valid_flag(connection->hash_entry_index, 1);
		return;
	} else {
		printk("%s::conn status %x, rx_pktsize %d\n",
			__func__, connection->status, storlink_ctl.rx_max_pktsize);
	}
}

/*---------------------------------------------------------------------------
 * get_connection_index
 * get_connection_index will find an available index for the connection,
 * when allocate a new connection is needed.
 * we find available Qid from AV bits and write to hash_table, so that when RxTOE
 * packet is received, sw_id from ToeQ descriptor is also the Qid of conneciton Q.
 *-------------------------------------------------------------------------*/
int get_connection_index(void)
{
	int i=0, j=0, index=-1;
	__u32	connection_bits;

	for (i = 0; i< TOE_TOE_QUEUE_NUM/32; i++) {
		connection_bits = ~(toe_connection_bits[i]);
		if (connection_bits == 0)
			// all 32 bits are used.
			continue;

		for (j=0; j<32; j++) {
			if (connection_bits & 0x01) {
				index = i*32 + j;
				return index;
			}
			connection_bits = connection_bits >> 1;
		}
	}
	return index;
}

/*---------------------------------------------------------------------------
 * set_toe_connection
 *-------------------------------------------------------------------------*/
void set_toe_connection(int index, int val)
{
	if (val) {
		toe_connection_bits[index/32] |= (1<<(index%32));
	} else {
		toe_connection_bits[index/32] &= (~(1<<(index%32)));
	}
}

/*---------------------------------------------------------------------------
 * sl351x_get_toe_conn_flag
 *-------------------------------------------------------------------------*/
int sl351x_get_toe_conn_flag(int index)
{
	if (index < TOE_TOE_QUEUE_NUM)
		return (toe_connection_bits[index/32] & (1 << (index %32)));
	else
		return 0;
}

/*---------------------------------------------------------------------------
 * sl351x_get_toe_conn_info
 *-------------------------------------------------------------------------*/
struct toe_conn * sl351x_get_toe_conn_info(int index)
{
	if (index < TOE_TOE_QUEUE_NUM)
		return (struct toe_conn *)&toe_connections[index];
	else
		return NULL;
}

/*---------------------------------------------------------------------------
 * create_sw_toe_connection
 *-------------------------------------------------------------------------*/
struct toe_conn* create_sw_toe_connection(int qid, int ip_ver, void* ip_hdr,
	struct tcphdr* tcp_hdr)
{
	struct toe_conn*	connection =  &(toe_connections[qid]);

	connection->ip_ver = (__u8)ip_ver;
	connection->qid = (__u8)qid;
	connection->source = (__u16)tcp_hdr->source;
	connection->dest = (__u16)tcp_hdr->dest;
	if (ip_ver == 4) {
		struct iphdr* iph = (struct iphdr*) ip_hdr;
		connection->saddr[0] = (__u32)iph->saddr;
		connection->daddr[0] = (__u32)iph->daddr;
//		printk("%s::saddr %x, daddr %x\n", __func__,
//			ntohl(connection->saddr[0]), ntohl(connection->daddr[0]));
	} else if (ip_ver == 6) {
		struct ipv6hdr *iph = (struct ipv6hdr*)ip_hdr;
		int i=0;
		for (i=0; i<4; i++) {
			connection->saddr[i] = (__u32)iph->saddr.in6_u.u6_addr32[i];
			connection->daddr[i] = (__u32)iph->daddr.in6_u.u6_addr32[i];
		}
	}
	connection->status = TCP_CONN_CREATION;
	return connection;
}

/*---------------------------------------------------------------------------
 * fill_toeq_buf
 *-------------------------------------------------------------------------*/
int fill_toeq_buf(int index, TOE_INFO_T* toe)
{
	volatile TOE_QHDR_T	*qhdr;
	//struct toe_conn* connection;
	GMAC_RXDESC_T	*desc_ptr;

	if (!toe->toe_desc_base[index]) {
		// first time. init.
		desc_ptr = (GMAC_RXDESC_T*)(pci_alloc_consistent(NULL, TOE_TOE_DESC_NUM
		            *sizeof(GMAC_RXDESC_T), (dma_addr_t*)&toe->toe_desc_base_dma[index]));

		toe->toe_desc_num = TOE_TOE_DESC_NUM;
		toe->toe_desc_base[index] = (unsigned int)desc_ptr;
	}
	qhdr = (volatile TOE_QHDR_T*)((unsigned int)TOE_TOE_QUE_HDR_BASE +
									index*sizeof(TOE_QHDR_T));
	//connection = (struct toe_conn*)&(toe_connections[index]);

	qhdr->word0.base_size = ((unsigned int)toe->toe_desc_base_dma[index]&TOE_QHDR0_BASE_MASK)
					| TOE_TOE_DESC_POWER;
	qhdr->word1.bits32 = 0;
	qhdr->word2.bits32 = 0;
	qhdr->word3.bits32 = 0;
	qhdr->word4.bits32 = 0;
	qhdr->word5.bits32 = 0;
	return 1;
}

/*---------------------------------------------------------------------------
 * create_toe_hash_entry_smb
 * add SMB header in hash entry.
 *-------------------------------------------------------------------------*/
int create_toe_hash_entry_smb(int ip_ver, void* ip_hdr, struct tcphdr* tcp_hdr,
	int sw_id)
{
	HASH_ENTRY_T	hash_entry, *entry;
	int	hash_entry_index;
	int i;

	entry = (HASH_ENTRY_T*)&hash_entry;
	memset((void*)entry, 0, sizeof(HASH_ENTRY_T));
	entry->rule = 0;

	/* enable fields of hash key */
	entry->key_present.ip_protocol = 1;
	entry->key_present.sip = 1;
	entry->key_present.dip = 1;
	entry->key_present.l4_bytes_0_3 = 1;	// src port and dest port
	entry->key_present.l7_bytes_0_3 = 0;	// do we need to enable NETBIOS? how?
	entry->key_present.l7_bytes_4_7 = 1;	// "SMB" header

	/* hash key */
	entry->key.ip_protocol = IPPROTO_TCP;
	if (ip_ver == 4) {
		struct iphdr *iph = (struct iphdr*)ip_hdr;
		memcpy(entry->key.sip, &iph->saddr, 4);
		memcpy(entry->key.dip, &iph->daddr, 4);
	} else if (ip_ver == 6) {
		struct ipv6hdr *iph = (struct ipv6hdr*)ip_hdr;
		for (i=0; i<4; i++) {
			memcpy(&(entry->key.sip[i*4]), &(iph->saddr.in6_u.u6_addr32[i]), 4);
			memcpy(&(entry->key.dip[i*4]), &(iph->daddr.in6_u.u6_addr32[i]), 4);
		}
	}
	*(__u16*)&entry->key.l4_bytes[0] = tcp_hdr->source;
	*(__u16*)&entry->key.l4_bytes[2] = tcp_hdr->dest;

	entry->key.l7_bytes[4] = 0xff;
	entry->key.l7_bytes[5] = 0x53;
	entry->key.l7_bytes[6] = 0x4d;
	entry->key.l7_bytes[7] = 0x42;

	/* action of hash entry match */
	entry->action.sw_id = 1;
	entry->action.dest_qid = (__u8)TOE_TOE_QID(sw_id);
	entry->action.srce_qid = 0;
	hash_entry_index = hash_add_toe_entry(entry);

	return hash_entry_index;
}

// best performance of tcp streaming.
/*---------------------------------------------------------------------------
 * create_toe_hash_entry_smb
 * add SMB header in hash entry.
 *-------------------------------------------------------------------------*/
int create_toe_hash_entry_ftp(int ip_ver, void* ip_hdr, struct tcphdr* tcphdr)
{
	return 0;
}

// is hash entry for nfs needed?

/*
 * Create a TOE hash entry by given ip addresses and tcp port numbers.
 * hash entry index will be saved in sw connection.
 */
/*---------------------------------------------------------------------------
 * create_toe_hash_entry
 *-------------------------------------------------------------------------*/
int create_toe_hash_entry(int ip_ver, void* ip_hdr, struct tcphdr* tcp_hdr, int sw_id)
{
	HASH_ENTRY_T	hash_entry, *entry;
//	unsigned long	hash_key[HASH_MAX_DWORDS];
	int	hash_entry_index;

	entry = (HASH_ENTRY_T*) &hash_entry;
	memset((void*)entry, 0, sizeof(HASH_ENTRY_T));
	entry->rule = 0;
	/* enable fields of hash key */
	entry->key_present.ip_protocol = 1;
	entry->key_present.sip = 1;
	entry->key_present.dip = 1;
	entry->key_present.l4_bytes_0_3 = 1;	// src port and dest port

	/* hash key */
	entry->key.ip_protocol = IPPROTO_TCP;
	if (ip_ver == 4) {
		// key of ipv4
		struct iphdr* iph = (struct iphdr*)ip_hdr;
		memcpy(entry->key.sip, &iph->saddr, 4);
		memcpy(entry->key.dip, &iph->daddr, 4);
	} else if (ip_ver == 6) {
		// key of ipv6
		int i=0;
		struct ipv6hdr *iph = (struct ipv6hdr*)ip_hdr;
		for (i=0; i<4; i++) {
			memcpy(&(entry->key.sip[i*4]), &(iph->saddr.in6_u.u6_addr32[i]), 4);
			memcpy(&(entry->key.dip[i*4]), &(iph->daddr.in6_u.u6_addr32[i]), 4);
		}
	}
	*(__u16*)&entry->key.l4_bytes[0] = tcp_hdr->source;
	*(__u16*)&entry->key.l4_bytes[2] = tcp_hdr->dest;
	// is it necessary to write ip version to hash key?

	/* action of hash entry match */
	entry->action.sw_id = 1;
	entry->action.dest_qid = (__u8)TOE_TOE_QID(sw_id);
	entry->action.srce_qid = 0;	// 0 for SW FreeQ. 1 for HW FreeQ.
	hash_entry_index = hash_add_toe_entry(entry);
//	printk("\n%s. sw_id %d, hash_entry index %x\n",
//		__func__, TOE_TOE_QID(sw_id), hash_entry_index);
	return hash_entry_index;
}

/*---------------------------------------------------------------------------
 * init_toeq
 * 1. Reserve a TOE Queue id first, to get the sw toe_connection.
 * 2. Setup the hash entry with given iphdr and tcphdr, save hash entry index
 *    in sw toe_connection.
 * 3. Prepare sw toe_connection and allocate buffers.
 * 4. Validate hash entry.
 *-------------------------------------------------------------------------*/
struct toe_conn* init_toeq(int ipver, void* iph, struct tcphdr* tcp_hdr,
	TOE_INFO_T* toe, unsigned char* l2hdr)
{
//	printk("\t*** %s, ipver %d\n", __func__, ipver);
	int qid=-1;
	struct toe_conn* connection;
	int hash_entry_index;
	// int i=0;
	unsigned short	dest_port = ntohs(tcp_hdr->dest);

	if (dest_port == 445) {
		printk("%s::SMB/CIFS connection\n", __func__);
	} else if (dest_port == 20) {
		printk("%s::ftp-data connection\n", __func__);
	} else if (dest_port == 2049) {
		printk("%s::nfs daemon connection\n", __func__);
	}
	qid = get_connection_index();
	if (qid<0)
		return 0;	// setup toeq failure
	set_toe_connection(qid, 1); // reserve this sw toeq.

	//connection = (struct toe_conn*)&(toe_connections[qid]);
	hash_entry_index = create_toe_hash_entry(ipver, iph, tcp_hdr, qid);
	if (hash_entry_index <0) {
		printk("%s::release toe hash entry!\n", __func__);
		set_toe_connection(qid, 0); // release this sw toeq.
		return 0;
	}
	connection = create_sw_toe_connection(qid, ipver, iph, tcp_hdr);
	connection->hash_entry_index = (__u16) hash_entry_index;

	fill_toeq_buf(qid, toe);
	memcpy(&connection->l2_hdr, l2hdr, sizeof(struct ethhdr));
	spin_lock_init(&connection->conn_lock);

	return connection;
}

#if 0
/*----------------------------------------------------------------------
*   toe_init_toe_queue
*   (1) Initialize the TOE Queue Header
*       Register: TOE_TOE_QUE_HDR_BASE (0x60003000)
*   (2) Initialize Descriptors of TOE Queues
*----------------------------------------------------------------------*/
void toe_init_toe_queue(TOE_INFO_T* toe)
{
}
EXPORT_SYMBOL(toe_init_toe_queue);
#endif

/*---------------------------------------------------------------------------
 * dump_jumbo_skb
 *-------------------------------------------------------------------------*/
void dump_jumbo_skb(struct jumbo_frame *jumbo_skb)
{
	if (jumbo_skb->skb0) {
//		printk("%s. jumbo skb %x, len %d\n",
//			__func__, jumbo_skb->skb0->data, jumbo_skb->skb0->len);
		netif_rx(jumbo_skb->skb0);
	}
	jumbo_skb->skb0 = 0;
	jumbo_skb->tail = 0;
	jumbo_skb->iphdr0 = 0;
	jumbo_skb->tcphdr0 = 0;
}

/* ---------------------------------------------------------------------
 * Append skb to skb0. skb0 is the jumbo frame that will be passed to
 * kernel tcp.
 * --------------------------------------------------------------------*/
void rx_append_skb(struct jumbo_frame *jumbo_skb, struct sk_buff* skb, int payload_len)
{
	struct iphdr* iphdr0 = (struct iphdr*)&(skb->data[0]);
	int ip_hdrlen = iphdr0->ihl << 2;
	struct tcphdr* tcphdr0 = (struct tcphdr*)&(skb->data[ip_hdrlen]);

	if (!jumbo_skb->skb0) {
		// head of the jumbo frame.
		jumbo_skb->skb0 = skb;
		jumbo_skb->tail = 0;
		jumbo_skb->iphdr0 = iphdr0;
		jumbo_skb->tcphdr0 = tcphdr0;
	} else {
		if (!jumbo_skb->tail)
			skb_shinfo(jumbo_skb->skb0)->frag_list = skb;
		else
			(jumbo_skb->tail)->next = skb;
		jumbo_skb->tail = skb;

		// do we need to change truesize as well?
		jumbo_skb->skb0->len += payload_len;
		jumbo_skb->skb0->data_len += payload_len;

		jumbo_skb->iphdr0->tot_len = htons(ntohs(jumbo_skb->iphdr0->tot_len)+payload_len);
		jumbo_skb->tcphdr0->ack_seq = tcphdr0->ack_seq;
		jumbo_skb->tcphdr0->window = tcphdr0->window;

		skb->len += payload_len;
		skb->data_len = 0;
		skb->data += ntohs(iphdr0->tot_len) - payload_len;
	}
}

/*----------------------------------------------------------------------
* toe_gmac_handle_toeq
* (1) read interrupt Queue to get TOE Q.
* (2) get packet fro TOE Q and send to upper layer handler.
* (3) allocate new buffers and put to TOE Q. Intr Q buffer is recycled.
*----------------------------------------------------------------------*/
void toe_gmac_handle_toeq(struct net_device *dev, GMAC_INFO_T* tp, __u32 status)
{
	//volatile INTRQ_INFO_T	*intrq_info;
	//TOEQ_INFO_T		*toeq_info;
	volatile NONTOE_QHDR_T	*intr_qhdr;
	volatile TOE_QHDR_T		*toe_qhdr;
	volatile INTR_QHDR_T	*intr_curr_desc;
	TOE_INFO_T	*toe = &toe_private_data;

	volatile GMAC_RXDESC_T	*toe_curr_desc; // , *fq_desc;// *tmp_desc;
	volatile DMA_RWPTR_T	intr_rwptr, toeq_rwptr;  // fq_rwptr;

	unsigned int 	pkt_size, desc_count, tcp_qid;
	volatile unsigned int	toeq_wptr;
	struct toe_conn*		connection;
	int		i, frag_id = 0;
	// unsigned long	toeq_flags;
	struct jumbo_frame	jumbo_skb;
	struct sk_buff	*skb;
	__u32	interrupt_status;

	in_toe_isr++;

	interrupt_status = status >> 24;
	// get interrupt queue header
	intr_qhdr = (volatile NONTOE_QHDR_T*)TOE_INTR_Q_HDR_BASE;
	memset(&jumbo_skb, 0, sizeof(struct jumbo_frame));

	for (i=0; i<TOE_INTR_QUEUE_NUM; i++, intr_qhdr++) {
		if (!(interrupt_status & 0x0001)) {
			// no interrupt of this IntQ
			interrupt_status = interrupt_status >> 1;
			continue;
		}
		interrupt_status = interrupt_status >> 1;
		intr_rwptr.bits32 = readl(&intr_qhdr->word1);

		while ( intr_rwptr.bits.rptr != intr_rwptr.bits.wptr) {
			int max_pktsize = 1;
			// get interrupt queue descriptor.
			intr_curr_desc = (INTR_QHDR_T*)toe->intr_desc_base +
				i* TOE_INTR_DESC_NUM + intr_rwptr.bits.rptr;
//			printk("%s::int %x\n", __func__, intr_curr_desc->word1.bits32);
			// get toeq id
			tcp_qid = (u8)intr_curr_desc->word1.bits.tcp_qid - (u8)TOE_TOE_QID(0);
			// get toeq queue header
			toe_qhdr = (volatile TOE_QHDR_T*) TOE_TOE_QUE_HDR_BASE;
			toe_qhdr += tcp_qid;
			connection = &toe_connections[tcp_qid];
			del_timer(&connection->rx_timer);
			// Gary Chen spin_lock_irqsave(&connection->conn_lock, toeq_flags);
			// handling interrupts of this TOE Q.
			if (intr_curr_desc->word1.bits.ctl || intr_curr_desc->word1.bits.osq ||
				intr_curr_desc->word1.bits.abn)
				max_pktsize = 0;
			if (!max_pktsize || intr_curr_desc->word1.bits.TotalPktSize) {
				desc_count=0;
				// wptr in intl queue is where this TOE interrupt should stop.
				toeq_rwptr.bits32 = readl(&toe_qhdr->word1);
				toeq_wptr = intr_curr_desc->word0.bits.wptr;
				if (connection->toeq_rwptr.bits.rptr != toeq_rwptr.bits.rptr)
					printk("conn rptr %d, hw rptr %d\n",
						connection->toeq_rwptr.bits.rptr, toeq_rwptr.bits.rptr);

				if (intr_curr_desc->word1.bits.ctl &&
					(toeq_rwptr.bits.rptr == toeq_wptr)) {
					printk("\nctrl frame, but not in TOE queue! conn rptr %d, hw wptr %d\n",
						connection->toeq_rwptr.bits.rptr, toeq_wptr);
//					dump_toe_qhdr(toe_qhdr);
//					dump_intrq_desc(intr_curr_desc);
				}
				// while (toeq_rwptr.bits.rptr != intr_curr_desc->word0.bits.wptr) {
				while (toe_qhdr->word1.bits.rptr != intr_curr_desc->word0.bits.wptr) {
					frag_id++;
					toe_curr_desc = (volatile GMAC_RXDESC_T *)(toe->toe_desc_base[tcp_qid]
						+ toe_qhdr->word1.bits.rptr *sizeof(GMAC_RXDESC_T));
					connection->curr_desc = (GMAC_RXDESC_T *)toe_curr_desc;
					desc_count = toe_curr_desc->word0.bits.desc_count;
					pkt_size = toe_curr_desc->word1.bits.byte_count;
					consistent_sync((void*)__va(toe_curr_desc->word2.buf_adr), pkt_size,
						PCI_DMA_FROMDEVICE);
					skb = (struct sk_buff*)(REG32(__va(toe_curr_desc->word2.buf_adr)-
						SKB_RESERVE_BYTES));
					_debug_skb(skb, (GMAC_RXDESC_T *)toe_curr_desc, 0x01);
					connection->curr_rx_skb = skb;
					skb_reserve(skb, RX_INSERT_BYTES);
					if ((skb->len + pkt_size) > (1514+16))
					{
						printk("skb->len=%d, pkt_size=%d\n",skb->len, pkt_size);
						while(1);
					}

					skb_put(skb, pkt_size);
					skb->dev = dev;
					skb->protocol = eth_type_trans(skb, dev);
					skb->ip_summed = CHECKSUM_UNNECESSARY;

					if (toe_curr_desc->word3.bits32 & 0x1b000000)
						dump_jumbo_skb(&jumbo_skb);

					rx_append_skb(&jumbo_skb, skb, pkt_size-toe_curr_desc->word3.bits.l7_offset);
//					spin_lock_irqsave(&gmac_fq_lock, flags);
					toeq_rwptr.bits.rptr = RWPTR_ADVANCE_ONE(toeq_rwptr.bits.rptr, TOE_TOE_DESC_NUM);
					SET_RPTR(&toe_qhdr->word1, toeq_rwptr.bits.rptr);
//					spin_unlock_irqrestore(&gmac_fq_lock, flags);
					if (storlink_ctl.fqint_threshold)
						continue;
#if 0
//#if (HANDLE_FREEQ_METHOD == HANDLE_FREEQ_INDIVIDUAL)
					if ((skb = dev_alloc_skb(RX_BUF_SIZE)) == NULL) {
						printk("%s::toe queue alloc buffer ", __func__);
					}
					*(unsigned int*)(skb->data) = (unsigned int)skb;
					connection->curr_rx_skb = skb;
					skb_reserve(skb, SKB_RESERVE_BYTES);

					spin_lock_irqsave(&gmac_fq_lock, flags);
					fq_rwptr.bits32 = readl(TOE_GLOBAL_BASE + GLOBAL_SWFQ_RWPTR_REG);
					if (toe->fq_rx_rwptr.bits.wptr != fq_rwptr.bits.wptr) {
						printk("%s::fq_rx_rwptr %x\n", __func__, toe->fq_rx_rwptr.bits32);
						mac_stop_txdma((struct net_device*) tp->dev);
						spin_unlock_irqrestore(&gmac_fq_lock, flags);
						while(1);
					}
					fq_desc = (GMAC_RXDESC_T*)toe->swfq_desc_base + fq_rwptr.bits.wptr;
					fq_desc->word2.buf_adr = (unsigned int)__pa(skb->data);

					fq_rwptr.bits.wptr = RWPTR_ADVANCE_ONE(fq_rwptr.bits.wptr, TOE_SW_FREEQ_DESC_NUM);
					SET_WPTR(TOE_GLOBAL_BASE + GLOBAL_SWFQ_RWPTR_REG, fq_rwptr.bits.wptr);
					toe->fq_rx_rwptr.bits32 = fq_rwptr.bits32;
					spin_unlock_irqrestore(&gmac_fq_lock, flags);
#endif
				} // end of this multi-desc.
				dump_jumbo_skb(&jumbo_skb);
				dev->last_rx = jiffies;
				connection->toeq_rwptr.bits32 = toeq_rwptr.bits32;
			} else if (intr_curr_desc->word1.bits.sat) {
				toeq_rwptr.bits32 = readl(&toe_qhdr->word1);
				toeq_wptr = intr_curr_desc->word0.bits.wptr;
				if (connection->toeq_rwptr.bits.rptr != toeq_rwptr.bits.rptr)
					printk("SAT. conn rptr %d, hw rptr %d\n",
						connection->toeq_rwptr.bits.rptr, toeq_rwptr.bits.rptr);
/*
					printk("%s::SAT int!, ackcnt %x, seqcnt %x, rptr %d, wptr %d, ack %x, qhack %x\n",
 						__func__, intr_curr_desc->word4.bits.AckCnt, intr_curr_desc->word4.bits.SeqCnt,
						toeq_rptr, toeq_wptr, intr_curr_desc->word3.ack_num, toe_qhdr->word4.ack_num);*/
				/* pure ack */
				if (toeq_rwptr.bits.rptr == toeq_wptr) {
					if (intr_curr_desc->word4.bits32) {
						skb = gen_pure_ack(connection, (TOE_QHDR_T *)toe_qhdr, (INTR_QHDR_T *)intr_curr_desc);
						skb_put(skb, 60);
						skb->dev = connection->dev;
						skb->ip_summed = CHECKSUM_UNNECESSARY;
						skb->protocol = eth_type_trans(skb, connection->dev);
						netif_rx(skb);
					} else
						printk("%s::SAT Interrupt!. But cnt is 0!\n", __func__);
				} else {
					// while (toeq_rwptr.bits.rptr != toeq_wptr) {
					while (toe_qhdr->word1.bits.rptr != intr_curr_desc->word0.bits.wptr) {
						toe_curr_desc = (volatile GMAC_RXDESC_T*)(toe->toe_desc_base[tcp_qid]
							+ toe_qhdr->word1.bits.rptr * sizeof(GMAC_RXDESC_T));
						connection->curr_desc = (GMAC_RXDESC_T *)toe_curr_desc;
						desc_count = toe_curr_desc->word0.bits.desc_count;
						pkt_size = toe_curr_desc->word1.bits.byte_count;
						consistent_sync((void*)__va(toe_curr_desc->word2.buf_adr), pkt_size,
							PCI_DMA_FROMDEVICE);
						// if ( ((toeq_rwptr.bits.rptr +1)&(TOE_TOE_DESC_NUM-1)) == toeq_wptr) {
						if ( RWPTR_ADVANCE_ONE(toe_qhdr->word1.bits.rptr, TOE_TOE_DESC_NUM) == toeq_wptr) {
							skb = (struct sk_buff*)(REG32(__va(toe_curr_desc->word2.buf_adr) -
								SKB_RESERVE_BYTES));
							_debug_skb(skb, (GMAC_RXDESC_T *)toe_curr_desc, 0x04);
							connection->curr_rx_skb = skb;
							skb_reserve(skb, RX_INSERT_BYTES);
							skb_put(skb, pkt_size);
							skb->dev = dev;
							skb->protocol = eth_type_trans(skb, dev);
							skb->ip_summed = CHECKSUM_UNNECESSARY;
							// printk("toeq_rptr %d, wptr %d\n", toeq_rptr, toeq_wptr);
							netif_rx(skb);
							dev->last_rx = jiffies;
/*
							if ((skb = dev_alloc_skb(RX_BUF_SIZE)) == NULL) {

							}
							*(unsigned int*)(skb->data) = (unsigned int) skb;
							skb_reserve(skb, SKB_RESERVE_BYTES); */
						} else {
							// reuse this skb, append to free queue..
							skb = (struct sk_buff*)(REG32(__va(toe_curr_desc->word2.buf_adr)-
								SKB_RESERVE_BYTES));
							_debug_skb(skb, (GMAC_RXDESC_T *)toe_curr_desc, 0x05);
							connection->curr_rx_skb = skb;
							dev_kfree_skb_irq(skb);
						}
#if 0
						spin_lock_irqsave(&gmac_fq_lock, flags);
						fq_rwptr.bits32 = readl(TOE_GLOBAL_BASE + GLOBAL_SWFQ_RWPTR_REG);
/*						if (toe->fq_rx_rwptr.bits.wptr != fq_rwptr.bits.wptr) {
							printk("%s::fq_rx_rwptr %x\n", __func__, toe->fq_rx_rwptr.bits32);
							mac_stop_txdma((struct net_device*) tp->dev);
							spin_unlock_irqrestore(&gmac_fq_lock, flags);
							while(1);
						} */
						fq_desc = (GMAC_RXDESC_T*)toe->swfq_desc_base + fq_rwptr.bits.wptr;
						fq_desc->word2.buf_adr = (unsigned int)__pa(skb->data);

						fq_rwptr.bits.wptr = RWPTR_ADVANCE_ONE(fq_rwptr.bits.wptr, TOE_SW_FREEQ_DESC_NUM);
						SET_WPTR(TOE_GLOBAL_BASE + GLOBAL_SWFQ_RWPTR_REG, fq_rwptr.bits.wptr);
						toe->fq_rx_rwptr.bits32 = fq_rwptr.bits32;
	//					spin_unlock_irqrestore(&gmac_fq_lock, flags);
#endif
//						spin_lock_irqsave(&gmac_fq_lock, flags);
						toeq_rwptr.bits.rptr = RWPTR_ADVANCE_ONE(toeq_rwptr.bits.rptr, TOE_TOE_DESC_NUM);
						SET_RPTR(&toe_qhdr->word1, toeq_rwptr.bits.rptr);
//						spin_unlock_irqrestore(&gmac_fq_lock, flags);
					}
				} // end of ACK with options.
				connection->toeq_rwptr.bits32 = toeq_rwptr.bits32;
				// Gary Chen spin_unlock_irqrestore(&connection->conn_lock, toeq_flags);
//				}
			};
			update_timer(connection);
			// any protection against interrupt queue header?
			intr_rwptr.bits.rptr = RWPTR_ADVANCE_ONE(intr_rwptr.bits.rptr, TOE_INTR_DESC_NUM);
			SET_RPTR(&intr_qhdr->word1, intr_rwptr.bits.rptr);
			intr_rwptr.bits32 = readl(&intr_qhdr->word1);
			toe_gmac_fill_free_q();
		} // end of this interrupt Queue processing.
	} // end of all interrupt Queues.

	in_toe_isr = 0;
}



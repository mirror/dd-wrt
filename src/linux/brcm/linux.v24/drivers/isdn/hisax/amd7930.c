/* $Id: amd7930.c,v 1.8 2001/09/24 13:22:55 kai Exp $
 *
 * HiSax ISDN driver - chip specific routines for AMD 7930
 *
 * Author       Brent Baccala
 * Copyright    by Brent Baccala <baccala@FreeSoft.org>
 *
 * This software may be used and distributed according to the terms
 * of the GNU General Public License, incorporated herein by reference.
 *
 *    - Existing ISDN HiSax driver provides all the smarts
 *    - it compiles, runs, talks to an isolated phone switch, connects
 *      to a Cisco, pings go through
 *    - AMD 7930 support only (no DBRI yet)
 *    - no US NI-1 support (may not work on US phone system - untested)
 *    - periodic packet loss, apparently due to lost interrupts
 *    - ISDN sometimes freezes, requiring reboot before it will work again
 *
 * The code is unreliable enough to be consider alpha
 *
 * This file is (c) under GNU General Public License
 *
 * Advanced Micro Devices' Am79C30A is an ISDN/audio chip used in the
 * SparcStation 1+.  The chip provides microphone and speaker interfaces
 * which provide mono-channel audio at 8K samples per second via either
 * 8-bit A-law or 8-bit mu-law encoding.  Also, the chip features an
 * ISDN BRI Line Interface Unit (LIU), I.430 S/T physical interface,
 * which performs basic D channel LAPD processing and provides raw
 * B channel data.  The digital audio channel, the two ISDN B channels,
 * and two 64 Kbps channels to the microprocessor are all interconnected
 * via a multiplexer.
 *
 * This driver interfaces to the Linux HiSax ISDN driver, which performs
 * all high-level Q.921 and Q.931 ISDN functions.  The file is not
 * itself a hardware driver; rather it uses functions exported by
 * the AMD7930 driver in the sparcaudio subsystem (drivers/sbus/audio),
 * allowing the chip to be simultaneously used for both audio and ISDN data.
 * The hardware driver does _no_ buffering, but provides several callbacks
 * which are called during interrupt service and should therefore run quickly.
 *
 * D channel transmission is performed by passing the hardware driver the
 * address and size of an skb's data area, then waiting for a callback
 * to signal successful transmission of the packet.  A task is then
 * queued to notify the HiSax driver that another packet may be transmitted.
 *
 * D channel reception is quite simple, mainly because of:
 *   1) the slow speed of the D channel - 16 kbps, and
 *   2) the presence of an 8- or 32-byte (depending on chip version) FIFO
 *      to buffer the D channel data on the chip
 * Worst case scenario of back-to-back packets with the 8 byte buffer
 * at 16 kbps yields an service time of 4 ms - long enough to preclude
 * the need for fancy buffering.  We queue a background task that copies
 * data out of the receive buffer into an skb, and the hardware driver
 * simply does nothing until we're done with the receive buffer and
 * reset it for a new packet.
 *
 * B channel processing is more complex, because of:
 *   1) the faster speed - 64 kbps,
 *   2) the lack of any on-chip buffering (it interrupts for every byte), and
 *   3) the lack of any chip support for HDLC encapsulation
 *
 * The HiSax driver can put each B channel into one of three modes -
 * L1_MODE_NULL (channel disabled), L1_MODE_TRANS (transparent data relay),
 * and L1_MODE_HDLC (HDLC encapsulation by low-level driver).
 * L1_MODE_HDLC is the most common, used for almost all "pure" digital
 * data sessions.  L1_MODE_TRANS is used for ISDN audio.
 *
 * HDLC B channel transmission is performed via a large buffer into
 * which the skb is copied while performing HDLC bit-stuffing.  A CRC
 * is computed and attached to the end of the buffer, which is then
 * passed to the low-level routines for raw transmission.  Once
 * transmission is complete, the hardware driver is set to enter HDLC
 * idle by successive transmission of mark (all 1) bytes, waiting for
 * the ISDN driver to prepare another packet for transmission and
 * deliver it.
 *
 * HDLC B channel reception is performed via an X-byte ring buffer
 * divided into N sections of X/N bytes each.  Defaults: X=256 bytes, N=4.
 * As the hardware driver notifies us that each section is full, we
 * hand it the next section and schedule a background task to peruse
 * the received section, bit-by-bit, with an HDLC decoder.  As
 * packets are detected, they are copied into a large buffer while
 * decoding HDLC bit-stuffing.  The ending CRC is verified, and if
 * it is correct, we alloc a new skb of the correct length (which we
 * now know), copy the packet into it, and hand it to the upper layers.
 * Optimization: for large packets, we hand the buffer (which also
 * happens to be an skb) directly to the upper layer after an skb_trim,
 * and alloc a new large buffer for future packets, thus avoiding a copy.
 * Then we return to HDLC processing; state is saved between calls.
 * 
 */

#define __NO_VERSION__
#include "hisax.h"
#include "../../sbus/audio/amd7930.h"
#include "isac.h"
#include "isdnl1.h"
#include "rawhdlc.h"
#include <linux/interrupt.h>

static const char *amd7930_revision = "$Revision: 1.8 $";

#define RCV_BUFSIZE	1024	/* Size of raw receive buffer in bytes */
#define RCV_BUFBLKS	4	/* Number of blocks to divide buffer into
				 * (must divide RCV_BUFSIZE) */

static void Bchan_fill_fifo(struct BCState *, struct sk_buff *);

static void
Bchan_xmt_bh(struct BCState *bcs)
{
	struct sk_buff *skb;

	if (bcs->hw.amd7930.tx_skb != NULL) {
		dev_kfree_skb(bcs->hw.amd7930.tx_skb);
		bcs->hw.amd7930.tx_skb = NULL;
	}

	if ((skb = skb_dequeue(&bcs->squeue))) {
		Bchan_fill_fifo(bcs, skb);
	} else {
		clear_bit(BC_FLG_BUSY, &bcs->Flag);
		bcs->event |= 1 << B_XMTBUFREADY;
		queue_task(&bcs->tqueue, &tq_immediate);
		mark_bh(IMMEDIATE_BH);
	}
}

static void
Bchan_xmit_callback(struct BCState *bcs)
{
	queue_task(&bcs->hw.amd7930.tq_xmt, &tq_immediate);
	mark_bh(IMMEDIATE_BH);
}

/* B channel transmission: two modes (three, if you count L1_MODE_NULL)
 *
 * L1_MODE_HDLC - We need to do HDLC encapsulation before transmiting
 * the packet (i.e. make_raw_hdlc_data).  Since this can be a
 * time-consuming operation, our completion callback just schedules
 * a bottom half to do encapsulation for the next packet.  In between,
 * the link will just idle
 *
 * L1_MODE_TRANS - Data goes through, well, transparent.  No HDLC encap,
 * and we can't just let the link idle, so the "bottom half" actually
 * gets called during the top half (it's our callback routine in this case),
 * but it's a lot faster now since we don't call make_raw_hdlc_data
 */

static void
Bchan_fill_fifo(struct BCState *bcs, struct sk_buff *skb)
{
	struct IsdnCardState *cs = bcs->cs;
	int len;

	if ((cs->debug & L1_DEB_HSCX) || (cs->debug & L1_DEB_HSCX_FIFO)) {
		char tmp[1024];
		char *t = tmp;

		t += sprintf(t, "amd7930_fill_fifo %c cnt %d",
			     bcs->channel ? 'B' : 'A', skb->len);
		if (cs->debug & L1_DEB_HSCX_FIFO)
			QuickHex(t, skb->data, skb->len);
		debugl1(cs, tmp);
	}

	if (bcs->mode == L1_MODE_HDLC) {
		len = make_raw_hdlc_data(skb->data, skb->len,
					 bcs->hw.amd7930.tx_buff, RAW_BUFMAX);
		if (len > 0)
			amd7930_bxmit(0, bcs->channel,
				      bcs->hw.amd7930.tx_buff, len,
				      (void *) &Bchan_xmit_callback,
				      (void *) bcs);
		dev_kfree_skb(skb);
	} else if (bcs->mode == L1_MODE_TRANS) {
		amd7930_bxmit(0, bcs->channel,
			      bcs->hw.amd7930.tx_buff, skb->len,
			      (void *) &Bchan_xmt_bh,
			      (void *) bcs);
		bcs->hw.amd7930.tx_skb = skb;
	} else {
		dev_kfree_skb(skb);
	}
}

static void
Bchan_mode(struct BCState *bcs, int mode, int bc)
{
	struct IsdnCardState *cs = bcs->cs;

	if (cs->debug & L1_DEB_HSCX) {
		char tmp[40];
		sprintf(tmp, "AMD 7930 mode %d bchan %d/%d",
			mode, bc, bcs->channel);
		debugl1(cs, tmp);
	}
	bcs->mode = mode;
}

/* Bchan_l2l1 is the entry point for upper layer routines that want to
 * transmit on the B channel.  PH_DATA_REQ is a normal packet that
 * we either start transmitting (if idle) or queue (if busy).
 * PH_PULL_REQ can be called to request a callback message (PH_PULL_CNF)
 * once the link is idle.  After a "pull" callback, the upper layer
 * routines can use PH_PULL_IND to send data.
 */

static void
Bchan_l2l1(struct PStack *st, int pr, void *arg)
{
	struct sk_buff *skb = arg;

	switch (pr) {
		case (PH_DATA_REQ):
			if (test_bit(BC_FLG_BUSY, &st->l1.bcs->Flag)) {
				skb_queue_tail(&st->l1.bcs->squeue, skb);
			} else {
				test_and_set_bit(BC_FLG_BUSY, &st->l1.bcs->Flag);
				Bchan_fill_fifo(st->l1.bcs, skb);
			}
			break;
		case (PH_PULL_IND):
			if (test_bit(BC_FLG_BUSY, &st->l1.bcs->Flag)) {
				printk(KERN_WARNING "amd7930: this shouldn't happen\n");
				break;
			}
			test_and_set_bit(BC_FLG_BUSY, &st->l1.bcs->Flag);
			Bchan_fill_fifo(st->l1.bcs, skb);
			break;
		case (PH_PULL_REQ):
			if (!test_bit(BC_FLG_BUSY, &st->l1.bcs->Flag)) {
				clear_bit(FLG_L1_PULL_REQ, &st->l1.Flags);
				st->l1.l1l2(st, PH_PULL_CNF, NULL);
			} else
				set_bit(FLG_L1_PULL_REQ, &st->l1.Flags);
			break;
	}
}

/* Receiver callback and bottom half - decodes HDLC at leisure (if
 * L1_MODE_HDLC) and passes newly received skb on via bcs->rqueue.  If
 * a large packet is received, stick rv_skb (the buffer that the
 * packet has been decoded into) on the receive queue and alloc a new
 * (large) skb to act as buffer for future receives.  If a small
 * packet is received, leave rv_skb alone, alloc a new skb of the
 * correct size, and copy the packet into it
 */

static void
Bchan_recv_callback(struct BCState *bcs)
{
	struct amd7930_hw *hw = &bcs->hw.amd7930;

	hw->rv_buff_in += RCV_BUFSIZE/RCV_BUFBLKS;
	hw->rv_buff_in %= RCV_BUFSIZE;

	if (hw->rv_buff_in != hw->rv_buff_out) {
		amd7930_brecv(0, bcs->channel,
			      hw->rv_buff + hw->rv_buff_in,
			      RCV_BUFSIZE/RCV_BUFBLKS,
			      (void *) &Bchan_recv_callback, (void *) bcs);
	}

	queue_task(&hw->tq_rcv, &tq_immediate);
	mark_bh(IMMEDIATE_BH);
}

static void
Bchan_rcv_bh(struct BCState *bcs)
{
	struct IsdnCardState *cs = bcs->cs;
	struct amd7930_hw *hw = &bcs->hw.amd7930;
	struct sk_buff *skb;
	int len;

	if (cs->debug & L1_DEB_HSCX) {
		char tmp[1024];

		sprintf(tmp, "amd7930_Bchan_rcv (%d/%d)",
			hw->rv_buff_in, hw->rv_buff_out);
		debugl1(cs, tmp);
		QuickHex(tmp, hw->rv_buff + hw->rv_buff_out,
			 RCV_BUFSIZE/RCV_BUFBLKS);
		debugl1(cs, tmp);
	}

	do {
		if (bcs->mode == L1_MODE_HDLC) {
			while ((len = read_raw_hdlc_data(hw->hdlc_state,
							 hw->rv_buff + hw->rv_buff_out, RCV_BUFSIZE/RCV_BUFBLKS,
							 hw->rv_skb->tail, HSCX_BUFMAX))) {
				if (len > 0 && (cs->debug & L1_DEB_HSCX_FIFO)) {
					char tmp[1024];
					char *t = tmp;

					t += sprintf(t, "amd7930_Bchan_rcv %c cnt %d", bcs->channel ? 'B' : 'A', len);
					QuickHex(t, hw->rv_skb->tail, len);
					debugl1(cs, tmp);
				}

				if (len > HSCX_BUFMAX/2) {
					/* Large packet received */

					if (!(skb = dev_alloc_skb(HSCX_BUFMAX))) {
						printk(KERN_WARNING "amd7930: receive out of memory");
					} else {
						skb_put(hw->rv_skb, len);
						skb_queue_tail(&bcs->rqueue, hw->rv_skb);
						hw->rv_skb = skb;
						bcs->event |= 1 << B_RCVBUFREADY;
						queue_task(&bcs->tqueue, &tq_immediate);
					}
				} else if (len > 0) {
					/* Small packet received */

					if (!(skb = dev_alloc_skb(len))) {
						printk(KERN_WARNING "amd7930: receive out of memory\n");
					} else {
						memcpy(skb_put(skb, len), hw->rv_skb->tail, len);
						skb_queue_tail(&bcs->rqueue, skb);
						bcs->event |= 1 << B_RCVBUFREADY;
						queue_task(&bcs->tqueue, &tq_immediate);
						mark_bh(IMMEDIATE_BH);
					}
				} else {
					/* Reception Error */
					/* printk("amd7930: B channel receive error\n"); */
				}
			}
		} else if (bcs->mode == L1_MODE_TRANS) {
			if (!(skb = dev_alloc_skb(RCV_BUFSIZE/RCV_BUFBLKS))) {
				printk(KERN_WARNING "amd7930: receive out of memory\n");
			} else {
				memcpy(skb_put(skb, RCV_BUFSIZE/RCV_BUFBLKS),
				       hw->rv_buff + hw->rv_buff_out,
				       RCV_BUFSIZE/RCV_BUFBLKS);
				skb_queue_tail(&bcs->rqueue, skb);
				bcs->event |= 1 << B_RCVBUFREADY;
				queue_task(&bcs->tqueue, &tq_immediate);
				mark_bh(IMMEDIATE_BH);
			}
		}

		if (hw->rv_buff_in == hw->rv_buff_out) {
			/* Buffer was filled up - need to restart receiver */
			amd7930_brecv(0, bcs->channel,
				      hw->rv_buff + hw->rv_buff_in,
				      RCV_BUFSIZE/RCV_BUFBLKS,
				      (void *) &Bchan_recv_callback,
				      (void *) bcs);
		}

		hw->rv_buff_out += RCV_BUFSIZE/RCV_BUFBLKS;
		hw->rv_buff_out %= RCV_BUFSIZE;

	} while (hw->rv_buff_in != hw->rv_buff_out);
}

static void
Bchan_close(struct BCState *bcs)
{
	struct sk_buff *skb;

	Bchan_mode(bcs, 0, 0);
	amd7930_bclose(0, bcs->channel);

	if (test_bit(BC_FLG_INIT, &bcs->Flag)) {
		skb_queue_purge(&bcs->rqueue);
		skb_queue_purge(&bcs->squeue);
	}
	test_and_clear_bit(BC_FLG_INIT, &bcs->Flag);
}

static int
Bchan_open(struct BCState *bcs)
{
	struct amd7930_hw *hw = &bcs->hw.amd7930;

	if (!test_and_set_bit(BC_FLG_INIT, &bcs->Flag)) {
		skb_queue_head_init(&bcs->rqueue);
		skb_queue_head_init(&bcs->squeue);
	}
	test_and_clear_bit(BC_FLG_BUSY, &bcs->Flag);

	amd7930_bopen(0, bcs->channel, 0xff);
	hw->rv_buff_in = 0;
	hw->rv_buff_out = 0;
	hw->tx_skb = NULL;
	init_hdlc_state(hw->hdlc_state, 0);
	amd7930_brecv(0, bcs->channel,
		      hw->rv_buff + hw->rv_buff_in, RCV_BUFSIZE/RCV_BUFBLKS,
		      (void *) &Bchan_recv_callback, (void *) bcs);

	bcs->event = 0;
	bcs->tx_cnt = 0;
	return (0);
}

static void
Bchan_init(struct BCState *bcs)
{
	if (!(bcs->hw.amd7930.tx_buff = kmalloc(RAW_BUFMAX, GFP_ATOMIC))) {
		printk(KERN_WARNING
		       "HiSax: No memory for amd7930.tx_buff\n");
		return;
	}
	if (!(bcs->hw.amd7930.rv_buff = kmalloc(RCV_BUFSIZE, GFP_ATOMIC))) {
		printk(KERN_WARNING
		       "HiSax: No memory for amd7930.rv_buff\n");
		return;
	}
	if (!(bcs->hw.amd7930.rv_skb = dev_alloc_skb(HSCX_BUFMAX))) {
		printk(KERN_WARNING
		       "HiSax: No memory for amd7930.rv_skb\n");
		return;
	}
	if (!(bcs->hw.amd7930.hdlc_state = kmalloc(sizeof(struct hdlc_state),
						   GFP_ATOMIC))) {
		printk(KERN_WARNING
		       "HiSax: No memory for amd7930.hdlc_state\n");
		return;
	}

	bcs->hw.amd7930.tq_rcv.sync = 0;
	bcs->hw.amd7930.tq_rcv.routine = (void (*)(void *)) &Bchan_rcv_bh;
	bcs->hw.amd7930.tq_rcv.data = (void *) bcs;

	bcs->hw.amd7930.tq_xmt.sync = 0;
	bcs->hw.amd7930.tq_xmt.routine = (void (*)(void *)) &Bchan_xmt_bh;
	bcs->hw.amd7930.tq_xmt.data = (void *) bcs;
}

static void
Bchan_manl1(struct PStack *st, int pr,
	  void *arg)
{
	switch (pr) {
		case (PH_ACTIVATE_REQ):
			test_and_set_bit(BC_FLG_ACTIV, &st->l1.bcs->Flag);
			Bchan_mode(st->l1.bcs, st->l1.mode, st->l1.bc);
			st->l1.l1man(st, PH_ACTIVATE_CNF, NULL);
			break;
		case (PH_DEACTIVATE_REQ):
			if (!test_bit(BC_FLG_BUSY, &st->l1.bcs->Flag))
				Bchan_mode(st->l1.bcs, 0, 0);
			test_and_clear_bit(BC_FLG_ACTIV, &st->l1.bcs->Flag);
			break;
	}
}

int
setstack_amd7930(struct PStack *st, struct BCState *bcs)
{
	if (Bchan_open(bcs))
		return (-1);
	st->l1.bcs = bcs;
	st->l2.l2l1 = Bchan_l2l1;
	st->ma.manl1 = Bchan_manl1;
	setstack_manager(st);
	bcs->st = st;
	return (0);
}


static void
amd7930_drecv_callback(void *arg, int error, unsigned int count)
{
	struct IsdnCardState *cs = (struct IsdnCardState *) arg;
	static struct tq_struct task;
	struct sk_buff *skb;

        /* NOTE: This function is called directly from an interrupt handler */

	if (1) {
		if (!(skb = alloc_skb(count, GFP_ATOMIC)))
			printk(KERN_WARNING "HiSax: D receive out of memory\n");
		else {
			memcpy(skb_put(skb, count), cs->rcvbuf, count);
			skb_queue_tail(&cs->rq, skb);
		}

		task.routine = (void *) DChannel_proc_rcv;
		task.data = (void *) cs;
		queue_task(&task, &tq_immediate);
		mark_bh(IMMEDIATE_BH);
	}

	if (cs->debug & L1_DEB_ISAC_FIFO) {
		char tmp[128];
		char *t = tmp;

		t += sprintf(t, "amd7930 Drecv cnt %d", count);
		if (error) t += sprintf(t, " ERR %x", error);
		QuickHex(t, cs->rcvbuf, count);
		debugl1(cs, tmp);
	}

	amd7930_drecv(0, cs->rcvbuf, MAX_DFRAME_LEN,
		      &amd7930_drecv_callback, cs);
}

static void
amd7930_dxmit_callback(void *arg, int error)
{
	struct IsdnCardState *cs = (struct IsdnCardState *) arg;
	static struct tq_struct task;

        /* NOTE: This function is called directly from an interrupt handler */

	/* may wish to do retransmission here, if error indicates collision */

	if (cs->debug & L1_DEB_ISAC_FIFO) {
		char tmp[128];
		char *t = tmp;

		t += sprintf(t, "amd7930 Dxmit cnt %d", cs->tx_skb->len);
		if (error) t += sprintf(t, " ERR %x", error);
		QuickHex(t, cs->tx_skb->data, cs->tx_skb->len);
		debugl1(cs, tmp);
	}

	cs->tx_skb = NULL;

	task.routine = (void *) DChannel_proc_xmt;
	task.data = (void *) cs;
	queue_task(&task, &tq_immediate);
	mark_bh(IMMEDIATE_BH);
}

static void
amd7930_Dchan_l2l1(struct PStack *st, int pr, void *arg)
{
	struct IsdnCardState *cs = (struct IsdnCardState *) st->l1.hardware;
	struct sk_buff *skb = arg;
	char str[64];

	switch (pr) {
		case (PH_DATA_REQ):
			if (cs->tx_skb) {
				skb_queue_tail(&cs->sq, skb);
#ifdef L2FRAME_DEBUG		/* psa */
				if (cs->debug & L1_DEB_LAPD)
					Logl2Frame(cs, skb, "PH_DATA Queued", 0);
#endif
			} else {
				if ((cs->dlogflag) && (!(skb->data[2] & 1))) {
					/* I-FRAME */
					LogFrame(cs, skb->data, skb->len);
					sprintf(str, "Q.931 frame user->network tei %d", st->l2.tei);
					dlogframe(cs, skb->data+4, skb->len-4,
						  str);
				}
				cs->tx_skb = skb;
				cs->tx_cnt = 0;
#ifdef L2FRAME_DEBUG		/* psa */
				if (cs->debug & L1_DEB_LAPD)
					Logl2Frame(cs, skb, "PH_DATA", 0);
#endif
				amd7930_dxmit(0, skb->data, skb->len,
					      &amd7930_dxmit_callback, cs);
			}
			break;
		case (PH_PULL_IND):
			if (cs->tx_skb) {
				if (cs->debug & L1_DEB_WARN)
					debugl1(cs, " l2l1 tx_skb exist this shouldn't happen");
				skb_queue_tail(&cs->sq, skb);
				break;
			}
			if ((cs->dlogflag) && (!(skb->data[2] & 1))) {	/* I-FRAME */
				LogFrame(cs, skb->data, skb->len);
				sprintf(str, "Q.931 frame user->network tei %d", st->l2.tei);
				dlogframe(cs, skb->data + 4, skb->len - 4,
					  str);
			}
			cs->tx_skb = skb;
			cs->tx_cnt = 0;
#ifdef L2FRAME_DEBUG		/* psa */
			if (cs->debug & L1_DEB_LAPD)
				Logl2Frame(cs, skb, "PH_DATA_PULLED", 0);
#endif
			amd7930_dxmit(0, cs->tx_skb->data, cs->tx_skb->len,
				      &amd7930_dxmit_callback, cs);
			break;
		case (PH_PULL_REQ):
#ifdef L2FRAME_DEBUG		/* psa */
			if (cs->debug & L1_DEB_LAPD)
				debugl1(cs, "-> PH_REQUEST_PULL");
#endif
			if (!cs->tx_skb) {
				test_and_clear_bit(FLG_L1_PULL_REQ, &st->l1.Flags);
				st->l1.l1l2(st, PH_PULL_CNF, NULL);
			} else
				test_and_set_bit(FLG_L1_PULL_REQ, &st->l1.Flags);
			break;
	}
}

int
setDstack_amd7930(struct PStack *st, struct IsdnCardState *cs)
{
	st->l2.l2l1 = amd7930_Dchan_l2l1;
	if (! cs->rcvbuf) {
		printk("setDstack_amd7930: No cs->rcvbuf!\n");
	} else {
		amd7930_drecv(0, cs->rcvbuf, MAX_DFRAME_LEN,
			      &amd7930_drecv_callback, cs);
	}
	return (0);
}

static void
manl1_msg(struct IsdnCardState *cs, int msg, void *arg) {
	struct PStack *st;

	st = cs->stlist;
	while (st) {
		st->ma.manl1(st, msg, arg);
		st = st->next;
	}
}

static void
amd7930_new_ph(struct IsdnCardState *cs)
{
	switch (amd7930_get_liu_state(0)) {
	        case 3:
			manl1_msg(cs, PH_POWERUP_CNF, NULL);
                        break;

	        case 7:
			manl1_msg(cs, PH_I4_P8_IND, NULL);
			break;

	        case 8:
			manl1_msg(cs, PH_RSYNC_IND, NULL);
			break;
	}
}

/* amd7930 LIU state change callback */

static void
amd7930_liu_callback(struct IsdnCardState *cs)
{
	static struct tq_struct task;

	if (!cs)
		return;

	if (cs->debug & L1_DEB_ISAC) {
		char tmp[32];
		sprintf(tmp, "amd7930_liu state %d", amd7930_get_liu_state(0));
		debugl1(cs, tmp);
	}

	task.sync = 0;
	task.routine = (void *) &amd7930_new_ph;
	task.data = (void *) cs;
	queue_task(&task, &tq_immediate);
	mark_bh(IMMEDIATE_BH);
}

void
amd7930_l1cmd(struct IsdnCardState *cs, int msg, void *arg)
{
	u_char val;
	char tmp[32];
	
	if (cs->debug & L1_DEB_ISAC) {
		char tmp[32];
		sprintf(tmp, "amd7930_l1cmd msg %x", msg);
		debugl1(cs, tmp);
	}

	switch(msg) {
		case PH_RESET_REQ:
			if (amd7930_get_liu_state(0) <= 3)
				amd7930_liu_activate(0,0);
			else
				amd7930_liu_deactivate(0);
			break;
		case PH_ENABLE_REQ:
			break;
		case PH_INFO3_REQ:
			amd7930_liu_activate(0,0);
			break;
		case PH_TESTLOOP_REQ:
			break;
		default:
			if (cs->debug & L1_DEB_WARN) {
				sprintf(tmp, "amd7930_l1cmd unknown %4x", msg);
				debugl1(cs, tmp);
			}
			break;
	}
}

static void init_amd7930(struct IsdnCardState *cs)
{
	Bchan_init(&cs->bcs[0]);
	Bchan_init(&cs->bcs[1]);
	cs->bcs[0].BC_SetStack = setstack_amd7930;
	cs->bcs[1].BC_SetStack = setstack_amd7930;
	cs->bcs[0].BC_Close = Bchan_close;
	cs->bcs[1].BC_Close = Bchan_close;
	Bchan_mode(cs->bcs, 0, 0);
	Bchan_mode(cs->bcs + 1, 0, 0);
}

void
release_amd7930(struct IsdnCardState *cs)
{
}

static int
amd7930_card_msg(struct IsdnCardState *cs, int mt, void *arg)
{
	switch (mt) {
		case CARD_RESET:
			return(0);
		case CARD_RELEASE:
			release_amd7930(cs);
			return(0);
		case CARD_INIT:
			cs->l1cmd = amd7930_l1cmd;
			amd7930_liu_init(0, &amd7930_liu_callback, (void *)cs);
			init_amd7930(cs);
			return(0);
		case CARD_TEST:
			return(0);
	}
	return(0);
}

int __init
setup_amd7930(struct IsdnCard *card)
{
	struct IsdnCardState *cs = card->cs;
	char tmp[64];

	strcpy(tmp, amd7930_revision);
	printk(KERN_INFO "HiSax: AMD7930 driver Rev. %s\n", HiSax_getrev(tmp));
	if (cs->typ != ISDN_CTYPE_AMD7930)
		return (0);

        cs->irq = amd7930_get_irqnum(0);
        if (cs->irq == 0)
		return (0);

	cs->cardmsg = &amd7930_card_msg;

	return (1);
}

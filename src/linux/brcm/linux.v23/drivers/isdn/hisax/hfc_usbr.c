/* $Id: hfc_usbr.c,v 2.5 2001/07/06 21:30:11 werner Exp $

 * hfc_usb.c  low level driver for CCD´s hfc-usb single chip controllers
 *            type approval valid for HFC-S USB based TAs
 *
 * Author     Werner Cornelius (werner@isdn-development.de)
 *
 * Copyright 2001  by Werner Cornelius (werner@isdn4linux.de)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#define __NO_VERSION__
#include <linux/init.h>
#include "hisax.h"
#include "isdnl1.h"
#include "hisax_loadable.h"

extern const char *CardType[];

static const char *hfcusb_revision = "$Revision: 2.5 $";

/*********************************/
/* schedule a new b_channel task */
/*********************************/
static void
hfcusb_sched_event(struct BCState *bcs, int event)
{
	bcs->event |= 1 << event;
	queue_task(&bcs->tqueue, &tq_immediate);
	mark_bh(IMMEDIATE_BH);
}

/************************************************/
/* select a b-channel entry matching and active */
/************************************************/
static
struct BCState *
Sel_BCS(struct IsdnCardState *cs, int channel)
{
	if (cs->bcs[0].mode && (cs->bcs[0].channel == channel))
		return (&cs->bcs[0]);
	else if (cs->bcs[1].mode && (cs->bcs[1].channel == channel))
		return (&cs->bcs[1]);
	else
		return (NULL);
}

/**********************************************/
/* transfer function (D-channel from l2 to l1 */
/**********************************************/
static void
hfcusb_d_l2l1(struct PStack *st, int pr, void *arg)
{
	struct IsdnCardState *cs = st->l1.hardware;
	struct hisax_drvreg *drv = cs->hw.hfcusb.drv;

	if (drv) {
		switch (pr) {
			case (PH_DATA | REQUEST):
			case (PH_PULL | INDICATION):
				cs->hw.hfcusb.dch_tx_busy = 1;
				break;
			case (PH_PULL | REQUEST):
				if (!cs->hw.hfcusb.dch_tx_busy) {
					test_and_clear_bit(FLG_L1_PULL_REQ,
							   &st->l1.Flags);
					st->l1.l1l2(st, PH_PULL | CONFIRM,
						    NULL);
				} else
					test_and_set_bit(FLG_L1_PULL_REQ,
							 &st->l1.Flags);
				return;
		}
		drv->dch_l2l1(drv, pr, arg);
	} else
		debugl1(cs, "hfcusb l2l1 called without existing driver");
}				/* hfcusb_d_l2l1 */

/*****************************/
/* E-channel receive routine */
/*****************************/
static void
EChannel_proc_rcv(struct IsdnCardState *cs)
{
	u_char *ptr;
	struct sk_buff *skb;
	struct hisax_drvreg *usbdrv =
	    (struct hisax_drvreg *) cs->hw.hfcusb.drv;


	while ((skb = skb_dequeue(&usbdrv->erq)) != NULL) {
		if (cs->debug & DEB_DLOG_HEX) {
			ptr = cs->dlog;
			if ((skb->len) < MAX_DLOG_SPACE / 3 - 10) {
				*ptr++ = 'E';
				*ptr++ = 'C';
				*ptr++ = 'H';
				*ptr++ = 'O';
				*ptr++ = ':';
				ptr += QuickHex(ptr, skb->data, skb->len);
				ptr--;
				*ptr++ = '\n';
				*ptr = 0;
				HiSax_putstatus(cs, NULL, cs->dlog);
			} else
				HiSax_putstatus(cs, "LogEcho: ",
						"warning Frame too big (%d)",
						skb->len);
		}
		dev_kfree_skb_any(skb);
	}
}

/******************************************/
/* transfer function called from L1 to L2 */
/******************************************/
static void
hfcusb_d_l1l2(void *cs1, int pr, void *arg)
{
	struct IsdnCardState *cs = (struct IsdnCardState *) cs1;

	switch (pr) {
		case (PH_DATA | INDICATION):
			if (!((int) arg))
				DChannel_proc_rcv(cs);
			else
				EChannel_proc_rcv(cs);
			break;

		case (PH_DATA | CONFIRM):
			cs->hw.hfcusb.dch_tx_busy = 0;
			DChannel_proc_xmt(cs);
			break;

		case (PH_ACTIVATE | INDICATION):
		case (PH_ACTIVATE | CONFIRM):
			cs->stlist->l1.l1l2(cs->stlist, pr, arg);
			if (cs->debug & L1_DEB_ISAC)
				debugl1(cs, "layer 1 activated");
			break;

		case (PH_DEACTIVATE | INDICATION):
		case (PH_DEACTIVATE | CONFIRM):
			cs->stlist->l1.l1l2(cs->stlist, pr, arg);
			if (cs->debug & L1_DEB_ISAC)
				debugl1(cs, "layer 1 deactivated");
			break;

		default:
			debugl1(cs, "unknown l1 msg 0x%x ", pr);
	}
}				/* hfcusb_d_l1l2 */


/******************************************/
/* transfer function called from L1 to L2 */
/******************************************/
static void
hfcusb_b_l1l2(void *cs1, int chan, int pr, void *arg)
{
	struct IsdnCardState *cs = (struct IsdnCardState *) cs1;
	struct BCState *bcs = Sel_BCS(cs, chan);

	switch (pr) {
		case (PH_DATA | INDICATION):
			if (!bcs)
				return;
			hfcusb_sched_event(bcs, B_RCVBUFREADY);
			break;

		case (PH_DATA | CONFIRM):
			if (!bcs)
				return;
			bcs->tx_cnt -= bcs->tx_skb->len;
			if (bcs->st->lli.l1writewakeup &&
			    (PACKET_NOACK != bcs->tx_skb->pkt_type))
				bcs->st->lli.l1writewakeup(bcs->st,
							   bcs->tx_skb->
							   len);
			dev_kfree_skb_any(bcs->tx_skb);
			bcs->tx_skb = skb_dequeue(&bcs->squeue);
			break;

		case (PH_ACTIVATE | INDICATION):
		case (PH_ACTIVATE | CONFIRM):
			cs->stlist->l1.l1l2(cs->stlist, pr, arg);
			if (cs->debug & L1_DEB_ISAC)
				debugl1(cs, "layer 1 activated");
			break;

		case (PH_DEACTIVATE | INDICATION):
		case (PH_DEACTIVATE | CONFIRM):
			cs->stlist->l1.l1l2(cs->stlist, pr, arg);
			if (cs->debug & L1_DEB_ISAC)
				debugl1(cs, "layer 1 deactivated");
			break;

		default:
			debugl1(cs, "unknown l1 b msg 0x%x ", pr);
	}
}				/* hfcusb_b_l1l2 */


/***********************************************/
/* called during init setting l1 stack pointer */
/***********************************************/
void
setstack_hfcusb(struct PStack *st, struct IsdnCardState *cs)
{
	st->l2.l2l1 = hfcusb_d_l2l1;
}

/**************************************/
/* send B-channel data if not blocked */
/**************************************/
static void
hfcusb_send_data(struct BCState *bcs)
{
	struct IsdnCardState *cs = bcs->cs;
	struct hisax_drvreg *drv =
	    (struct hisax_drvreg *) cs->hw.hfcusb.drv;

	if (!drv)
		return;
	drv->bch_l2l1(drv->argl1, bcs->channel, PH_DATA | REQUEST,
		      bcs->tx_skb);
}

/***************************************************************/
/* activate/deactivate hardware for selected channels and mode */
/***************************************************************/
void
mode_hfcusb(struct BCState *bcs, int mode, int bc)
{
	struct IsdnCardState *cs = bcs->cs;
	struct hisax_drvreg *drv = cs->hw.hfcusb.drv;

	if (!drv)
		return;
	if (cs->debug & L1_DEB_HSCX)
		debugl1(cs, "HFCUSB bchannel mode %d bchan %d/%d",
			mode, bc, bcs->channel);
	bcs->mode = mode;
	bcs->channel = bc;
	if (mode) {
		drv->bsk[bc] = &bcs->tx_skb;
		drv->brq[bc] = &bcs->rqueue;
	}
	drv->bch_l2l1(drv->argl1, bc, PH_ACTIVATE | REQUEST,
		      (void *) mode);
	if (!mode) {
		drv->bsk[bc] = NULL;
		drv->brq[bc] = NULL;
	}
}

/******************************/
/* Layer2 -> Layer 1 Transfer */
/******************************/
static void
hfcusb_b_l2l1(struct PStack *st, int pr, void *arg)
{
	struct sk_buff *skb = arg;
	struct hisax_drvreg *drv = st->l1.bcs->cs->hw.hfcusb.drv;
	long flags;

	switch (pr) {
		case (PH_DATA | REQUEST):
			save_flags(flags);
			cli();
			if (st->l1.bcs->tx_skb) {
				skb_queue_tail(&st->l1.bcs->squeue, skb);
				restore_flags(flags);
			} else {
				st->l1.bcs->tx_skb = skb;
				st->l1.bcs->cs->BC_Send_Data(st->l1.bcs);
				restore_flags(flags);
			}
			break;
		case (PH_PULL | INDICATION):
			if (st->l1.bcs->tx_skb) {
				printk(KERN_WARNING
				       "hfc_l2l1: this shouldn't happen\n");
				break;
			}
			save_flags(flags);
			cli();
			st->l1.bcs->tx_skb = skb;
			st->l1.bcs->cs->BC_Send_Data(st->l1.bcs);
			restore_flags(flags);
			break;
		case (PH_PULL | REQUEST):
			if (!st->l1.bcs->tx_skb) {
				test_and_clear_bit(FLG_L1_PULL_REQ,
						   &st->l1.Flags);
				st->l1.l1l2(st, PH_PULL | CONFIRM, NULL);
			} else
				test_and_set_bit(FLG_L1_PULL_REQ,
						 &st->l1.Flags);
			break;
		case (PH_ACTIVATE | REQUEST):
			if (drv) {
				test_and_set_bit(BC_FLG_ACTIV,
						 &st->l1.bcs->Flag);
				mode_hfcusb(st->l1.bcs, st->l1.mode,
					    st->l1.bc);
				l1_msg_b(st, pr, arg);
			}
			break;
		case (PH_DEACTIVATE | REQUEST):
			l1_msg_b(st, pr, arg);
			break;
		case (PH_DEACTIVATE | CONFIRM):
			test_and_clear_bit(BC_FLG_ACTIV,
					   &st->l1.bcs->Flag);
			test_and_clear_bit(BC_FLG_BUSY, &st->l1.bcs->Flag);
			mode_hfcusb(st->l1.bcs, 0, st->l1.bc);
			st->l1.l1l2(st, PH_DEACTIVATE | CONFIRM, NULL);
			break;
	}
}

/******************************************/
/* deactivate B-channel access and queues */
/******************************************/
static void
close_hfcusb(struct BCState *bcs)
{
	mode_hfcusb(bcs, 0, bcs->channel);
	if (test_and_clear_bit(BC_FLG_INIT, &bcs->Flag)) {
		skb_queue_purge(&bcs->rqueue);
		skb_queue_purge(&bcs->squeue);
		if (bcs->tx_skb) {
			dev_kfree_skb_any(bcs->tx_skb);
			bcs->tx_skb = NULL;
			test_and_clear_bit(BC_FLG_BUSY, &bcs->Flag);
		}
	}
}

/*************************************/
/* init B-channel queues and control */
/*************************************/
static int
open_hfcusbstate(struct IsdnCardState *cs, struct BCState *bcs)
{
	if (!test_and_set_bit(BC_FLG_INIT, &bcs->Flag)) {
		skb_queue_head_init(&bcs->rqueue);
		skb_queue_head_init(&bcs->squeue);
	}
	bcs->tx_skb = NULL;
	test_and_clear_bit(BC_FLG_BUSY, &bcs->Flag);
	bcs->event = 0;
	bcs->tx_cnt = 0;
	return (0);
}

/*********************************/
/* inits the stack for B-channel */
/*********************************/
static int
setstack_2b(struct PStack *st, struct BCState *bcs)
{
	bcs->channel = st->l1.bc;
	if (open_hfcusbstate(st->l1.hardware, bcs))
		return (-1);
	st->l1.bcs = bcs;
	st->l2.l2l1 = hfcusb_b_l2l1;
	setstack_manager(st);
	bcs->st = st;
	setstack_l1_B(st);
	return (0);
}

/********************************/
/* called for card init message */
/********************************/
void __devinit
inithfcusb(struct IsdnCardState *cs)
{
	cs->setstack_d = setstack_hfcusb;
	cs->BC_Send_Data = &hfcusb_send_data;
	cs->bcs[0].BC_SetStack = setstack_2b;
	cs->bcs[1].BC_SetStack = setstack_2b;
	cs->bcs[0].BC_Close = close_hfcusb;
	cs->bcs[1].BC_Close = close_hfcusb;
	mode_hfcusb(cs->bcs, 0, 0);
	mode_hfcusb(cs->bcs + 1, 0, 1);
}



/*******************************************/
/* handle card messages from control layer */
/*******************************************/
static int
hfcusb_card_msg(struct IsdnCardState *cs, int mt, void *arg)
{
	if (cs->debug & L1_DEB_ISAC)
		debugl1(cs, "HFCUSB: card_msg %x", mt);
	switch (mt) {
		case CARD_INIT:
			inithfcusb(cs);
			return (0);
		case CARD_RELEASE:
		case CARD_RESET:
		case CARD_TEST:
			return (0);
	}
	return (0);
}


extern void
 HiSax_closecard(int cardnr);
/*****************************/
/* release a driver instance */
/* called when hardware is   */
/* no longer present.        */
/*****************************/
static void
release_hfcdrv(void *arg)
{
	struct IsdnCardState *cs = (struct IsdnCardState *) arg;

	cs->hw.hfcusb.drv = NULL;	/* disable any further use of driver */
	HiSax_closecard(cs->cardnr);
}				/* release_hfcdrv */

/*********************************************/
/* called once when a new device is detected */
/* initialises local data                    */
/*********************************************/
int
setup_hfc_usb(struct IsdnCard *card)
{
	struct IsdnCardState *cs = card->cs;
	char tmp[64];
	struct hisax_drvreg *usbdrv =
	    (struct hisax_drvreg *) cs->hw.hfcusb.drv;

	if (!usbdrv)
		return (0);	/* no driver data present */

	strcpy(tmp, hfcusb_revision);
	printk(KERN_INFO "HiSax: HFC-USB driver Rev. %s\n",
	       HiSax_getrev(tmp));

	usbdrv->release_driver = &release_hfcdrv;	/* release routine */
	usbdrv->arg_hisax = (void *) cs;	/* parameter for calling */
	usbdrv->dch_l1l2 = &hfcusb_d_l1l2;	/* access from L1 to HiSax */
	usbdrv->bch_l1l2 = &hfcusb_b_l1l2;
	usbdrv->drq = &cs->rq;
	usbdrv->dsq = &cs->sq;
	cs->cardmsg = &hfcusb_card_msg;
	return (1);		/* success */
}

/*
 * Common [OS-independent] portion of
 * Broadcom Home Networking Division 10/100 Mbit/s Ethernet
 * Device Driver.
 *
 * Copyright (C) 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 * $Id: etc.c,v 1.105.2.2.12.1 2009/03/13 00:45:23 Exp $
 */

#include <typedefs.h>
#include <osl.h>
#include <bcmendian.h>
#include <proto/ethernet.h>
#include <proto/vlan.h>
#include <proto/bcmip.h>
#include <proto/802.1d.h>
#include <bcmenetmib.h>
#include <bcmenetrxh.h>
#include <bcmenetphy.h>
#include <et_dbg.h>
#include <etc.h>
#include <et_export.h>
#include <bcmutils.h>

#ifdef ETROBO
#ifndef	_siutils_h_
typedef const struct si_pub  si_t;
#endif
#include <bcmrobo.h>
#endif /* ETROBO */

uint32 et_msg_level =
	0;

/* local prototypes */
static void etc_loopback(etc_info_t *etc, int on);

/* 802.1d priority to traffic class mapping. queues correspond one-to-one
 * with traffic classes.
 */
uint32 up2tc[NUMPRIO] = {
	TC_BE,  	/* 0    BE    TC_BE    Best Effort */
	TC_BK,  	/* 1    BK    TC_BK    Background */
	TC_BK,  	/* 2    --    TC_BK    Background */
	TC_BE,  	/* 3    EE    TC_BE    Best Effort */
	TC_CL,  	/* 4    CL    TC_CL    Controlled Load */
	TC_CL,  	/* 5    VI    TC_CL    Controlled Load */
	TC_VO,  	/* 6    VO    TC_VO    Voice */
	TC_VO   	/* 7    NC    TC_VO    Voice */
};

uint32 priq_selector[] = {
	[0x0] = TC_NONE, [0x1] = TC_BK, [0x2] = TC_BE, [0x3] = TC_BE,
	[0x4] = TC_CL,   [0x5] = TC_CL, [0x6] = TC_CL, [0x7] = TC_CL,
	[0x8] = TC_VO,   [0x9] = TC_VO, [0xa] = TC_VO, [0xb] = TC_VO,
	[0xc] = TC_VO,   [0xd] = TC_VO, [0xe] = TC_VO, [0xf] = TC_VO
};

/* find the chip opsvec for this chip */
struct chops*
etc_chipmatch(uint vendor, uint device)
{
	{
		extern struct chops bcm47xx_et_chops;

		if (bcm47xx_et_chops.id(vendor, device))
			return (&bcm47xx_et_chops);
	}

#ifdef CFG_GMAC
	{
		extern struct chops bcmgmac_et_chops;

		if (bcmgmac_et_chops.id(vendor, device))
			return (&bcmgmac_et_chops);
	}
#endif /* CFG_GMAC */
	return (NULL);
}

void*
etc_attach(void *et, uint vendor, uint device, uint unit, void *osh, void *regsva)
{
	etc_info_t *etc;

	ET_TRACE(("et%d: etc_attach: vendor 0x%x device 0x%x\n", unit, vendor, device));

	/* some code depends on packed structures */
	ASSERT(sizeof(struct ether_addr) == ETHER_ADDR_LEN);
	ASSERT(sizeof(struct ether_header) == ETHER_HDR_LEN);

	/* allocate etc_info_t state structure */
	if ((etc = (etc_info_t*) MALLOC(osh, sizeof(etc_info_t))) == NULL) {
		ET_ERROR(("et%d: etc_attach: out of memory, malloced %d bytes\n", unit,
		          MALLOCED(osh)));
		return (NULL);
	}
	bzero((char*)etc, sizeof(etc_info_t));

	etc->et = et;
	etc->unit = unit;
	etc->osh = osh;
	etc->vendorid = (uint16) vendor;
	etc->deviceid = (uint16) device;
	etc->forcespeed = ET_AUTO;
	etc->linkstate = FALSE;

	/* set chip opsvec */
	etc->chops = etc_chipmatch(vendor, device);
	ASSERT(etc->chops);

	/* chip attach */
	if ((etc->ch = (*etc->chops->attach)(etc, osh, regsva)) == NULL) {
		ET_ERROR(("et%d: chipattach error\n", unit));
		goto fail;
	}

	return ((void*)etc);

fail:
	etc_detach(etc);
	return (NULL);
}

void
etc_detach(etc_info_t *etc)
{
	if (etc == NULL)
		return;

	/* free chip private state */
	if (etc->ch) {
		(*etc->chops->detach)(etc->ch);
		etc->chops = etc->ch = NULL;
	}

	MFREE(etc->osh, etc, sizeof(etc_info_t));
}

void
etc_reset(etc_info_t *etc)
{
	ET_TRACE(("et%d: etc_reset\n", etc->unit));

	etc->reset++;

	/* reset the chip */
	(*etc->chops->reset)(etc->ch);

	/* free any posted tx packets */
	(*etc->chops->txreclaim)(etc->ch, TRUE);

#ifdef DMA
	/* free any posted rx packets */
	(*etc->chops->rxreclaim)(etc->ch);
#endif /* DMA */
}

void
etc_init(etc_info_t *etc, uint options)
{
	ET_TRACE(("et%d: etc_init\n", etc->unit));

	ASSERT(etc->pioactive == NULL);
	ASSERT(!ETHER_ISNULLADDR(&etc->cur_etheraddr));
	ASSERT(!ETHER_ISMULTI(&etc->cur_etheraddr));

	/* init the chip */
	(*etc->chops->init)(etc->ch, options);
}

/* mark interface up */
void
etc_up(etc_info_t *etc)
{
	etc->up = TRUE;

	et_init(etc->et, ET_INIT_DEF_OPTIONS);
}

/* mark interface down */
uint
etc_down(etc_info_t *etc, int reset)
{
	uint callback;

	callback = 0;

	ET_FLAG_DOWN(etc);

	if (reset)
		et_reset(etc->et);

	/* suppress link state changes during power management mode changes */
	if (etc->linkstate) {
		etc->linkstate = FALSE;
		if (!etc->pm_modechange)
			et_link_down(etc->et);
	}

	return (callback);
}

/* common ioctl handler.  return: 0=ok, -1=error */
int
etc_ioctl(etc_info_t *etc, int cmd, void *arg)
{
	int error;
	int val;
	int *vec = (int*)arg;

	error = 0;

	val = arg ? *(int*)arg : 0;

	ET_TRACE(("et%d: etc_ioctl: cmd 0x%x\n", etc->unit, cmd));

	switch (cmd) {
	case ETCUP:
		et_up(etc->et);
		break;

	case ETCDOWN:
		et_down(etc->et, TRUE);
		break;

	case ETCLOOP:
		etc_loopback(etc, val);
		break;

	case ETCDUMP:
		if (et_msg_level & 0x10000)
			bcmdumplog((char *)arg, 4096);
		break;

	case ETCSETMSGLEVEL:
		et_msg_level = val;
		break;

	case ETCPROMISC:
		etc_promisc(etc, val);
		break;

	case ETCQOS:
		etc_qos(etc, val);
		break;

	case ETCSPEED:
		if (val == ET_1000FULL) {
			etc->speed = 1000;
			etc->duplex = 1;
		} else if (val == ET_1000HALF) {
			etc->speed = 1000;
			etc->duplex = 0;
		} else if (val == ET_100FULL) {
			etc->speed = 100;
			etc->duplex = 1;
		} else if (val == ET_100HALF) {
			etc->speed = 100;
			etc->duplex = 0;
		} else if (val == ET_10FULL) {
			etc->speed = 10;
			etc->duplex = 1;
		} else if (val == ET_10HALF) {
			etc->speed = 10;
			etc->duplex = 0;
		} else if (val == ET_AUTO)
			;
		else
			goto err;

		etc->forcespeed = val;

		/* explicitly reset the phy */
		(*etc->chops->phyreset)(etc->ch, etc->phyaddr);

		/* request restart autonegotiation if we're reverting to adv mode */
		if (etc->forcespeed == ET_AUTO) {
			etc->advertise = (ADV_100FULL | ADV_100HALF |
			                  ADV_10FULL | ADV_10HALF);
			etc->advertise2 = ADV_1000FULL;
			etc->needautoneg = TRUE;
		} else {
			etc->advertise = etc->advertise2 = 0;
			etc->needautoneg = FALSE;
		}

		et_init(etc->et, ET_INIT_DEF_OPTIONS);
		break;

	case ETCPHYRD:
		if (vec) {
			vec[1] = (*etc->chops->phyrd)(etc->ch, etc->phyaddr, vec[0]);
			ET_TRACE(("etc_ioctl: ETCPHYRD of reg 0x%x => 0x%x\n", vec[0], vec[1]));
		}
		break;

	case ETCPHYRD2:
		if (vec) {
			uint phyaddr, reg;
			phyaddr = vec[0] >> 16;
			if (phyaddr < MAXEPHY) {
				reg = vec[0] & 0xffff;
				vec[1] = (*etc->chops->phyrd)(etc->ch, phyaddr, reg);
				ET_TRACE(("etc_ioctl: ETCPHYRD2 of phy 0x%x, reg 0x%x => 0x%x\n",
				          phyaddr, reg, vec[1]));
			}
		}
		break;

	case ETCPHYWR:
		if (vec) {
			ET_TRACE(("etc_ioctl: ETCPHYWR to reg 0x%x <= 0x%x\n", vec[0], vec[1]));
			(*etc->chops->phywr)(etc->ch, etc->phyaddr, vec[0], (uint16)vec[1]);
		}
		break;

	case ETCPHYWR2:
		if (vec) {
			uint phyaddr, reg;
			phyaddr = vec[0] >> 16;
			if (phyaddr < MAXEPHY) {
				reg = vec[0] & 0xffff;
				(*etc->chops->phywr)(etc->ch, phyaddr, reg, (uint16)vec[1]);
				ET_TRACE(("etc_ioctl: ETCPHYWR2 to phy 0x%x, reg 0x%x <= 0x%x\n",
				          phyaddr, reg, vec[1]));
			}
		}
		break;

#ifdef ETROBO
	case ETCROBORD:
		if (etc->robo && vec) {
			uint page, reg;
			uint16 val;
			robo_info_t *robo = (robo_info_t *)etc->robo;

			page = vec[0] >> 16;
			reg = vec[0] & 0xffff;
			val = -1;
			robo->ops->read_reg(etc->robo, page, reg, &val, 2);
			vec[1] = val;
			ET_TRACE(("etc_ioctl: ETCROBORD of page 0x%x, reg 0x%x => 0x%x\n",
			          page, reg, val));
		}
		break;

	case ETCROBOWR:
		if (etc->robo && vec) {
			uint page, reg;
			uint16 val;
			robo_info_t *robo = (robo_info_t *)etc->robo;

			page = vec[0] >> 16;
			reg = vec[0] & 0xffff;
			val = vec[1];
			robo->ops->write_reg(etc->robo, page, vec[0], &val, 2);
			ET_TRACE(("etc_ioctl: ETCROBOWR to page 0x%x, reg 0x%x <= 0x%x\n",
			          page, reg, val));
		}
		break;
#endif /* ETROBO */


	default:
	err:
		error = -1;
	}

	return (error);
}

/* called once per second */
void
etc_watchdog(etc_info_t *etc)
{
	uint16 status;
	uint16 lpa;

	etc->now++;

	/* no local phy registers */
	if (etc->phyaddr == EPHY_NOREG) {
		etc->linkstate = TRUE;
		etc->duplex = 1;
		/* keep emac txcontrol duplex bit consistent with current phy duplex */
		(*etc->chops->duplexupd)(etc->ch);
		return;
	}

	status = (*etc->chops->phyrd)(etc->ch, etc->phyaddr, 1);
	/* check for bad mdio read */
	if (status == 0xffff) {
		ET_ERROR(("et%d: etc_watchdog: bad mdio read: phyaddr %d mdcport %d\n",
			etc->unit, etc->phyaddr, etc->mdcport));
		return;
	}

	if (etc->forcespeed == ET_AUTO) {
		uint16 adv, adv2 = 0, status2 = 0, estatus;

		adv = (*etc->chops->phyrd)(etc->ch, etc->phyaddr, 4);
		lpa = (*etc->chops->phyrd)(etc->ch, etc->phyaddr, 5);

		/* read extended status register. if we are 1000BASE-T
		 * capable then get our advertised capabilities and the
		 * link partner capabilities from 1000BASE-T control and
		 * status registers.
		 */
		estatus = (*etc->chops->phyrd)(etc->ch, etc->phyaddr, 15);
		if ((estatus != 0xffff) && (estatus & EST_1000TFULL)) {
			/* read 1000BASE-T control and status registers */
			adv2 = (*etc->chops->phyrd)(etc->ch, etc->phyaddr, 9);
			status2 = (*etc->chops->phyrd)(etc->ch, etc->phyaddr, 10);
		}

		/* update current speed and duplex */
		if ((adv2 & ADV_1000FULL) && (status2 & LPA_1000FULL)) {
			etc->speed = 1000;
			etc->duplex = 1;
		} else if ((adv2 & ADV_1000HALF) && (status2 & LPA_1000HALF)) {
			etc->speed = 1000;
			etc->duplex = 0;
		} else if ((adv & ADV_100FULL) && (lpa & LPA_100FULL)) {
			etc->speed = 100;
			etc->duplex = 1;
		} else if ((adv & ADV_100HALF) && (lpa & LPA_100HALF)) {
			etc->speed = 100;
			etc->duplex = 0;
		} else if ((adv & ADV_10FULL) && (lpa & LPA_10FULL)) {
			etc->speed = 10;
			etc->duplex = 1;
		} else {
			etc->speed = 10;
			etc->duplex = 0;
		}
	}

	/* monitor link state */
	if (!etc->linkstate && (status & STAT_LINK)) {
		etc->linkstate = TRUE;
		if (etc->pm_modechange)
			etc->pm_modechange = FALSE;
		else
			et_link_up(etc->et);
	} else if (etc->linkstate && !(status & STAT_LINK)) {
		etc->linkstate = FALSE;
		if (!etc->pm_modechange)
			et_link_down(etc->et);
	}

	/* keep emac txcontrol duplex bit consistent with current phy duplex */
	(*etc->chops->duplexupd)(etc->ch);

	/* check for remote fault error */
	if (status & STAT_REMFAULT) {
		ET_ERROR(("et%d: remote fault\n", etc->unit));
	}

	/* check for jabber error */
	if (status & STAT_JAB) {
		ET_ERROR(("et%d: jabber\n", etc->unit));
	}

	/*
	 * Read chip mib counters occationally before the 16bit ones can wrap.
	 * We don't use the high-rate mib counters.
	 */
	if ((etc->now % 30) == 0)
		(*etc->chops->statsupd)(etc->ch);
}

static void
etc_loopback(etc_info_t *etc, int on)
{
	ET_TRACE(("et%d: etc_loopback: %d\n", etc->unit, on));

	etc->loopbk = (bool) on;
	et_init(etc->et, ET_INIT_DEF_OPTIONS);
}

void
etc_promisc(etc_info_t *etc, uint on)
{
	ET_TRACE(("et%d: etc_promisc: %d\n", etc->unit, on));

	etc->promisc = (bool) on;
	et_init(etc->et, ET_INIT_DEF_OPTIONS);
}

void
etc_qos(etc_info_t *etc, uint on)
{
	ET_TRACE(("et%d: etc_qos: %d\n", etc->unit, on));

	etc->qos = (bool) on;
	et_init(etc->et, ET_INIT_DEF_OPTIONS);
}

/* WAR: BCM53115 switch is not retaining the tag while forwarding
 * the vlan/priority tagged frames even when tag status preserve
 * is enabled. This problem can be only worked around by doing
 * double tagging for priority tagged frames. This will trick the
 * switch in to just removing the outer tag on the egress. Inner
 * tag remains which contains the prio.
 */
#ifdef ETROBO
void *
etc_bcm53115_war(etc_info_t *etc, void *p)
{
	struct ethervlan_header *evh;
	uint16 vlan_tag;
	int vlan_prio;
	uint8 *data = PKTDATA(etc->osh, p);

	evh = (struct ethervlan_header *)data;
	if (evh->vlan_type != hton16(ETHER_TYPE_8021Q))
		return (p);

	vlan_tag = evh->vlan_tag;
	vlan_prio = vlan_tag & hton16(VLAN_PRI_MASK << VLAN_PRI_SHIFT);

	/* No need to do anything for priority 0 */
	if (vlan_prio == 0)
		return (p);

	/* If the packet is shared or there is not enough headroom
	 * then allocate new header buffer and link the original
	 * buffer to it.
	 */
	if ((PKTHEADROOM(etc->osh, p) < VLAN_TAG_LEN) || PKTSHARED(p)) {
		void *pkt;
		uint16 ether_type;

		if ((pkt = PKTGET(etc->osh, VLAN_TAG_LEN +
		                  ETHERVLAN_HDR_LEN, TRUE)) == NULL) {
			ET_ERROR(("et%d: PKTGET of size %d failed during expand head\n",
			          etc->unit, VLAN_TAG_LEN + ETHERVLAN_HDR_LEN));
			return (NULL);
		}

		/* Assign priority of original frame */
		PKTSETPRIO(pkt, ntoh16(vlan_prio) >> VLAN_PRI_SHIFT);

		ether_type = evh->ether_type;

		/* Copy the vlan header to the first buffer */
		memcpy(PKTDATA(etc->osh, pkt), data, ETHERVLAN_HDR_LEN);
		PKTPULL(etc->osh, p, ETHERVLAN_HDR_LEN);

		/* Align the pointer to initialize the inner vlan tag and type
		 * fields.
		 */
		evh = (struct ethervlan_header *)(PKTDATA(etc->osh, pkt) + VLAN_TAG_LEN);
		evh->vlan_tag = vlan_tag;
		evh->ether_type = ether_type;

		/* Chain the original buffer to new header buffer */
		PKTSETNEXT(etc->osh, pkt, p);

		p = pkt;
	} else {
		data = PKTPUSH(etc->osh, p, VLAN_TAG_LEN);
		ETHERVLAN_MOVE_HDR(data, data + VLAN_TAG_LEN);
		evh = (struct ethervlan_header *)(data + VLAN_TAG_LEN);
	}

	evh->vlan_type = hton16(ETHER_TYPE_8021Q);

	/* Clear the vlan id in the inner tag */
	evh->vlan_tag &= ~(hton16(VLAN_VID_MASK));

	return (p);
}
#endif /* ETROBO */


uint
etc_totlen(etc_info_t *etc, void *p)
{
	uint total;

	total = 0;
	for (; p; p = PKTNEXT(etc->osh, p))
		total += PKTLEN(etc->osh, p);
	return (total);
}

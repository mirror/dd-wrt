/*
 * Broadcom Gigabit Ethernet MAC (Unimac) core.
 *
 * This file implements the chip-specific routines for the GMAC core.
 *
 * Copyright (C) 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 * $Id: etcgmac.c,v 1.2.2.3.14.15 2009/04/09 08:03:38 Exp $
 */

#include <typedefs.h>
#include <osl.h>
#include <bcmdefs.h>
#include <bcmendian.h>
#include <bcmutils.h>
#include <bcmdevs.h>
#include <bcmenetphy.h>
#include <proto/ethernet.h>
#include <proto/802.1d.h>
#include <siutils.h>
#include <sbhnddma.h>
#include <sbchipc.h>
#include <hnddma.h>
#include <et_dbg.h>
#include <hndsoc.h>
#include <hndpmu.h>
#include <bcmgmacmib.h>
#include <gmac_core.h>
#include <et_export.h>		/* for et_phyxx() routines */
#include <etcgmac.h>
#include <bcmenetrxh.h>
#include <bcmgmacrxh.h>

#ifdef ETROBO
#include <bcmrobo.h>
#endif /* ETROBO */
#ifdef ETADM
#include <etc_adm.h>
#endif /* ETADM */

struct bcmgmac;	/* forward declaration */
#define ch_t	struct bcmgmac
#include <etc.h>

/* private chip state */
struct bcmgmac {
	void 		*et;		/* pointer to et private state */
	etc_info_t	*etc;		/* pointer to etc public state */

	gmacregs_t	*regs;		/* pointer to chip registers */
	osl_t 		*osh;		/* os handle */

	void 		*etphy;		/* pointer to et for shared mdc/mdio contortion */

	uint32		intstatus;	/* saved interrupt condition bits */
	uint32		intmask;	/* current software interrupt mask */

	hnddma_t	*di[NUMTXQ];	/* dma engine software state */

	bool		mibgood;	/* true once mib registers have been cleared */
	gmacmib_t	mib;		/* mib statistic counters */
	si_t 		*sih;		/* si utils handle */

	char		*vars;		/* sprom name=value */
	uint		vars_size;

	void		*adm;		/* optional admtek private data */
	mcfilter_t	mf;		/* multicast filter */
};

/* local prototypes */
static bool chipid(uint vendor, uint device);
static void *chipattach(etc_info_t *etc, void *osh, void *regsva);
static void chipdetach(ch_t *ch);
static void chipreset(ch_t *ch);
static void chipinit(ch_t *ch, uint options);
static bool chiptx(ch_t *ch, void *p);
static void *chiprx(ch_t *ch);
static void chiprxfill(ch_t *ch);
static int chipgetintrevents(ch_t *ch, bool in_isr);
static bool chiperrors(ch_t *ch);
static void chipintrson(ch_t *ch);
static void chipintrsoff(ch_t *ch);
static void chiptxreclaim(ch_t *ch, bool all);
static void chiprxreclaim(ch_t *ch);
static void chipstatsupd(ch_t *ch);
static void chipdumpmib(ch_t *ch, struct bcmstrbuf *b);
static void chipenablepme(ch_t *ch);
static void chipdisablepme(ch_t *ch);
static void chipphyreset(ch_t *ch, uint phyaddr);
static uint16 chipphyrd(ch_t *ch, uint phyaddr, uint reg);
static void chipphywr(ch_t *ch, uint phyaddr, uint reg, uint16 v);
static void chipdump(ch_t *ch, struct bcmstrbuf *b);
static void chiplongname(ch_t *ch, char *buf, uint bufsize);
static void chipduplexupd(ch_t *ch);

static void chipphyinit(ch_t *ch, uint phyaddr);
static void chipphyor(ch_t *ch, uint phyaddr, uint reg, uint16 v);
static void chipphyforce(ch_t *ch, uint phyaddr);
static void chipphyadvertise(ch_t *ch, uint phyaddr);
static void gmac_mf_cleanup(ch_t *ch);
static int gmac_speed(ch_t *ch, uint32 speed);
static void gmac_miiconfig(ch_t *ch);

struct chops bcmgmac_et_chops = {
	chipid,
	chipattach,
	chipdetach,
	chipreset,
	chipinit,
	chiptx,
	chiprx,
	chiprxfill,
	chipgetintrevents,
	chiperrors,
	chipintrson,
	chipintrsoff,
	chiptxreclaim,
	chiprxreclaim,
	chipstatsupd,
	chipdumpmib,
	chipenablepme,
	chipdisablepme,
	chipphyreset,
	chipphyrd,
	chipphywr,
	chipdump,
	chiplongname,
	chipduplexupd
};

static uint devices[] = {
	BCM47XX_GMAC_ID,
	BCM4716_CHIP_ID,
	0x0000
};

static bool
chipid(uint vendor, uint device)
{
	int i;

	if (vendor != VENDOR_BROADCOM)
		return (FALSE);

	for (i = 0; devices[i]; i++) {
		if (device == devices[i])
			return (TRUE);
	}

	return (FALSE);
}

static void *
chipattach(etc_info_t *etc, void *osh, void *regsva)
{
	ch_t *ch;
	gmacregs_t *regs;
	uint i;
	char name[16];
	char *var;
	uint boardflags, boardtype, reset;

	ET_TRACE(("et%d: chipattach: regsva 0x%lx\n", etc->unit, (ulong)regsva));

	if ((ch = (ch_t *)MALLOC(osh, sizeof(ch_t))) == NULL) {
		ET_ERROR(("et%d: chipattach: out of memory, malloced %d bytes\n", etc->unit,
		          MALLOCED(osh)));
		return (NULL);
	}
	bzero((char *)ch, sizeof(ch_t));

	ch->etc = etc;
	ch->et = etc->et;
	ch->osh = osh;

	/* store the pointer to the sw mib */
	etc->mib = (void *)&ch->mib;

	/* get si handle */
	if ((ch->sih = si_attach(etc->deviceid, ch->osh, regsva, PCI_BUS, NULL, &ch->vars,
	                         &ch->vars_size)) == NULL) {
		ET_ERROR(("et%d: chipattach: si_attach error\n", etc->unit));
		goto fail;
	}

	if ((regs = (gmacregs_t *)si_setcore(ch->sih, GMAC_CORE_ID, etc->unit)) == NULL) {
		ET_ERROR(("et%d: chipattach: Could not setcore to the GMAC core\n", etc->unit));
		goto fail;
	}

	ch->regs = regs;
	etc->chip = ch->sih->chip;
	etc->chiprev = ch->sih->chiprev;
	etc->coreid = si_coreid(ch->sih);
	etc->corerev = si_corerev(ch->sih);
	etc->nicmode = !(ch->sih->bustype == SI_BUS);
	etc->coreunit = si_coreunit(ch->sih);
	etc->boardflags = getintvar(ch->vars, "boardflags");

	boardflags = etc->boardflags;
	boardtype = ch->sih->boardtype;

	/* get our local ether addr */
	sprintf(name, "et%dmacaddr", etc->coreunit);
	var = getvar(ch->vars, name);
	if (var == NULL) {
		ET_ERROR(("et%d: chipattach: getvar(%s) not found\n", etc->unit, name));
		goto fail;
	}
	bcm_ether_atoe(var, &etc->perm_etheraddr);

	if (ETHER_ISNULLADDR(&etc->perm_etheraddr)) {
		ET_ERROR(("et%d: chipattach: invalid format: %s=%s\n", etc->unit, name, var));
		goto fail;
	}
	bcopy((char *)&etc->perm_etheraddr, (char *)&etc->cur_etheraddr, ETHER_ADDR_LEN);

	/*
	 * Too much can go wrong in scanning MDC/MDIO playing "whos my phy?" .
	 * Instead, explicitly require the environment var "et<coreunit>phyaddr=<val>".
	 */

	/* get our phyaddr value */
	sprintf(name, "et%dphyaddr", etc->coreunit);
	var = getvar(ch->vars, name);
	if (var == NULL) {
		ET_ERROR(("et%d: chipattach: getvar(%s) not found\n", etc->unit, name));
		goto fail;
	}
	etc->phyaddr = bcm_atoi(var) & EPHY_MASK;

	/* nvram says no phy is present */
	if (etc->phyaddr == EPHY_NONE) {
		ET_ERROR(("et%d: chipattach: phy not present\n", etc->unit));
		goto fail;
	}

	/* configure pci core */
	si_pci_setup(ch->sih, (1 << si_coreidx(ch->sih)));

	/* reset the gmac core */
	chipreset(ch);

	/* dma attach */
	sprintf(name, "et%d", etc->coreunit);

	/* allocate dma resources for txqs */
	/* TX: TC_BK, RX: RX_Q0 */
	ch->di[0] = dma_attach(osh, name, ch->sih,
	                       DMAREG(ch, DMA_TX, TX_Q0),
	                       DMAREG(ch, DMA_RX, RX_Q0),
	                       NTXD, NRXD, RXBUFSZ, NRXBUFPOST, HWRXOFF,
	                       &et_msg_level);

	/* TX: TC_BE, RX: UNUSED */
	ch->di[1] = dma_attach(osh, name, ch->sih,
	                       DMAREG(ch, DMA_TX, TX_Q1),
	                       NULL /* rxq unused */,
	                       NTXD, 0, 0, 0, 0, &et_msg_level);

	/* TX: TC_CL, RX: UNUSED */
	ch->di[2] = dma_attach(osh, name, ch->sih,
	                       DMAREG(ch, DMA_TX, TX_Q2),
	                       NULL /* rxq unused */,
	                       NTXD, 0, 0, 0, 0, &et_msg_level);

	/* TX: TC_VO, RX: UNUSED */
	ch->di[3] = dma_attach(osh, name, ch->sih,
	                       DMAREG(ch, DMA_TX, TX_Q3),
	                       NULL /* rxq unused */,
	                       NTXD, 0, 0, 0, 0, &et_msg_level);

	for (i = 0; i < NUMTXQ; i++)
		if (ch->di[i] == NULL) {
			ET_ERROR(("et%d: chipattach: dma_attach failed\n", etc->unit));
			goto fail;
		}

	for (i = 0; i < NUMTXQ; i++)
		if (ch->di[i] != NULL)
			etc->txavail[i] = (uint *)&ch->di[i]->txavail;

	/* set default sofware intmask */
	ch->intmask = DEF_INTMASK;

	/* reset the external phy */
	if ((reset = getgpiopin(ch->vars, "ephy_reset", GPIO_PIN_NOTDEFINED)) !=
	    GPIO_PIN_NOTDEFINED) {
		reset = 1 << reset;

		/* Keep RESET low for 2 us */
		si_gpioout(ch->sih, reset, 0, GPIO_DRV_PRIORITY);
		si_gpioouten(ch->sih, reset, reset, GPIO_DRV_PRIORITY);
		OSL_DELAY(2);

		/* Keep RESET high for at least 2 us */
		si_gpioout(ch->sih, reset, reset, GPIO_DRV_PRIORITY);
		OSL_DELAY(2);

		/* if external phy is present enable auto-negotation and
		 * advertise full capabilities as default config.
		 */
		ASSERT(etc->phyaddr != EPHY_NOREG);
		etc->needautoneg = TRUE;
		etc->advertise = (ADV_100FULL | ADV_100HALF | ADV_10FULL | ADV_10HALF);
		etc->advertise2 = ADV_1000FULL;
	}

	/* reset phy: reset it once now */
	chipphyreset(ch, etc->phyaddr);

#ifdef ETROBO
	/*
	 * Broadcom Robo ethernet switch.
	 */
	if ((boardflags & BFL_ENETROBO) && (etc->phyaddr == EPHY_NOREG)) {
		/* Attach to the switch */
		if (!(etc->robo = bcm_robo_attach(ch->sih, ch, ch->vars,
		                                  (miird_f)bcmgmac_et_chops.phyrd,
		                                  (miiwr_f)bcmgmac_et_chops.phywr))) {
			ET_ERROR(("et%d: chipattach: robo_attach failed\n", etc->unit));
			goto fail;
		}
		/* Enable the switch and set it to a known good state */
		if (bcm_robo_enable_device(etc->robo)) {
			ET_ERROR(("et%d: chipattach: robo_enable_device failed\n", etc->unit));
			goto fail;
		}
		/* Configure the switch to do VLAN */
		if ((boardflags & BFL_ENETVLAN) &&
		    bcm_robo_config_vlan(etc->robo, etc->perm_etheraddr.octet)) {
			ET_ERROR(("et%d: chipattach: robo_config_vlan failed\n", etc->unit));
			goto fail;
		}
		/* Enable switching/forwarding */
		if (bcm_robo_enable_switch(etc->robo)) {
			ET_ERROR(("et%d: chipattach: robo_enable_switch failed\n", etc->unit));
			goto fail;
		}
	}
#endif /* ETROBO */

#ifdef ETADM
	/*
	 * ADMtek ethernet switch.
	 */
	if (boardflags & BFL_ENETADM) {
		/* Attach to the device */
		if (!(ch->adm = adm_attach(ch->sih, ch->vars))) {
			ET_ERROR(("et%d: chipattach: adm_attach failed\n", etc->unit));
			goto fail;
		}
		/* Enable the external switch and set it to a known good state */
		if (adm_enable_device(ch->adm)) {
			ET_ERROR(("et%d: chipattach: adm_enable_device failed\n", etc->unit));
			goto fail;
		}
		/* Configure the switch */
		if ((boardflags & BFL_ENETVLAN) && adm_config_vlan(ch->adm)) {
			ET_ERROR(("et%d: chipattach: adm_config_vlan failed\n", etc->unit));
			goto fail;
		}
	}
#endif /* ETADM */

	return ((void *) ch);

fail:
	chipdetach(ch);
	return (NULL);
}

static void
chipdetach(ch_t *ch)
{
	int32 i;

	ET_TRACE(("et%d: chipdetach\n", ch->etc->unit));

	if (ch == NULL)
		return;

#ifdef ETROBO
	/* free robo state */
	if (ch->etc->robo)
		bcm_robo_detach(ch->etc->robo);
#endif /* ETROBO */

#ifdef ETADM
	/* free ADMtek state */
	if (ch->adm)
		adm_detach(ch->adm);
#endif /* ETADM */

	/* free dma state */
	for (i = 0; i < NUMTXQ; i++)
		if (ch->di[i] != NULL) {
			dma_detach(ch->di[i]);
			ch->di[i] = NULL;
		}

	/* put the core back into reset */
	if (ch->sih)
		si_core_disable(ch->sih, 0);

	ch->etc->mib = NULL;

	/* free si handle */
	si_detach(ch->sih);
	ch->sih = NULL;

	/* free vars */
	if (ch->vars)
		MFREE(ch->osh, ch->vars, ch->vars_size);

	/* free chip private state */
	MFREE(ch->osh, ch, sizeof(ch_t));
}

static void
chiplongname(ch_t *ch, char *buf, uint bufsize)
{
	char *s;

	switch (ch->etc->deviceid) {
		case BCM47XX_GMAC_ID:
		case BCM4716_CHIP_ID:
		default:
			s = "Broadcom BCM47XX 10/100/1000 Mbps Ethernet Controller";
			break;
	}

	strncpy(buf, s, bufsize);
	buf[bufsize - 1] = '\0';
}

static void
chipdump(ch_t *ch, struct bcmstrbuf *b)
{
}


static void
gmac_clearmib(ch_t *ch)
{
	volatile uint32 *ptr;

	/* enable clear on read */
	OR_REG(ch->osh, &ch->regs->devcontrol, DC_MROR);

	for (ptr = &ch->regs->mib.tx_good_octets; ptr <= &ch->regs->mib.rx_uni_pkts; ptr++) {
		(void)R_REG(ch->osh, ptr);
		if (ptr == &ch->regs->mib.tx_q3_octets_high)
			ptr++;
	}

	return;
}

static void
gmac_init_reset(ch_t *ch)
{
	OR_REG(ch->osh, &ch->regs->cmdcfg, CC_SR);
	OSL_DELAY(GMAC_RESET_DELAY);
}

static void
gmac_clear_reset(ch_t *ch)
{
	AND_REG(ch->osh, &ch->regs->cmdcfg, ~CC_SR);
	OSL_DELAY(GMAC_RESET_DELAY);
}

static void
gmac_reset(ch_t *ch)
{
	uint32 ocmdcfg, cmdcfg;

	/* put the mac in reset */
	gmac_init_reset(ch);

	/* initialize default config */
	ocmdcfg = cmdcfg = R_REG(ch->osh, &ch->regs->cmdcfg);

	cmdcfg &= ~(CC_TE | CC_RE | CC_RPI | CC_TAI | CC_HD | CC_ML |
	            CC_CFE | CC_RL | CC_RED | CC_PE | CC_TPI | CC_PAD_EN);
	cmdcfg |= (CC_PROM | CC_PF | CC_NLC | CC_CFE);

	if (cmdcfg != ocmdcfg)
		W_REG(ch->osh, &ch->regs->cmdcfg, cmdcfg);

	/* bring mac out of reset */
	gmac_clear_reset(ch);
}

static void
gmac_promisc(ch_t *ch, bool mode)
{
	uint32 cmdcfg;

	cmdcfg = R_REG(ch->osh, &ch->regs->cmdcfg);

	/* put the mac in reset */
	gmac_init_reset(ch);

	/* enable or disable promiscuous mode */
	if (mode)
		cmdcfg |= CC_PROM;
	else
		cmdcfg &= ~CC_PROM;

	W_REG(ch->osh, &ch->regs->cmdcfg, cmdcfg);

	/* bring mac out of reset */
	gmac_clear_reset(ch);
}

static int
gmac_speed(ch_t *ch, uint32 speed)
{
	uint32 cmdcfg;
	uint32 hd_ena = 0;

	switch (speed) {
		case ET_10HALF:
			hd_ena = CC_HD;
			/* FALLTHRU */

		case ET_10FULL:
			speed = 0;
			break;

		case ET_100HALF:
			hd_ena = CC_HD;
			/* FALLTHRU */

		case ET_100FULL:
			speed = 1;
			break;

		case ET_1000FULL:
			speed = 2;
			break;

		case ET_1000HALF:
			ET_ERROR(("et%d: gmac_speed: supports 1000 mbps full duplex only\n",
			          ch->etc->unit));
			return (FAILURE);

		default:
			ET_ERROR(("et%d: gmac_speed: speed %d not supported\n",
			          ch->etc->unit, speed));
			return (FAILURE);
	}

	cmdcfg = R_REG(ch->osh, &ch->regs->cmdcfg);

	/* put mac in reset */
	gmac_init_reset(ch);

	/* set the speed */
	cmdcfg &= ~(CC_ES_MASK | CC_HD);
	cmdcfg |= ((speed << CC_ES_SHIFT) | hd_ena);
	W_REG(ch->osh, &ch->regs->cmdcfg, cmdcfg);

	/* bring mac out of reset */
	gmac_clear_reset(ch);

	return (SUCCESS);
}

static void
gmac_macloopback(ch_t *ch, bool on)
{
	uint32 ocmdcfg, cmdcfg;

	ocmdcfg = cmdcfg = R_REG(ch->osh, &ch->regs->cmdcfg);

	/* put mac in reset */
	gmac_init_reset(ch);

	/* set/clear the mac loopback mode */
	if (on)
		cmdcfg |= CC_ML;
	else
		cmdcfg &= ~CC_ML;

	if (cmdcfg != ocmdcfg)
		W_REG(ch->osh, &ch->regs->cmdcfg, cmdcfg);

	/* bring mac out of reset */
	gmac_clear_reset(ch);
}

static int
gmac_loopback(ch_t *ch, uint32 mode)
{
	switch (mode) {
		case LOOPBACK_MODE_DMA:
			/* to enable loopback for any channel set the loopback
			 * enable bit in xmt0control register.
			 */
			dma_fifoloopbackenable(ch->di[TX_Q0]);
			break;

		case LOOPBACK_MODE_MAC:
			gmac_macloopback(ch, TRUE);
			break;

		case LOOPBACK_MODE_NONE:
			gmac_macloopback(ch, FALSE);
			break;

		default:
			ET_ERROR(("et%d: gmac_loopaback: Unknown loopback mode %d\n",
			          ch->etc->unit, mode));
			return (FAILURE);
	}

	return (SUCCESS);
}

static void
gmac_enable(ch_t *ch)
{
	uint32 cmdcfg, rxqctl, bp_clk, mdp, mode;
	gmacregs_t *regs;

	regs = ch->regs;

	cmdcfg = R_REG(ch->osh, &ch->regs->cmdcfg);

	/* put mac in reset */
	gmac_init_reset(ch);

	cmdcfg |= CC_SR;

	/* first deassert rx_ena and tx_ena while in reset */
	cmdcfg &= ~(CC_RE | CC_TE);
	W_REG(ch->osh, &regs->cmdcfg, cmdcfg);

	/* bring mac out of reset */
	gmac_clear_reset(ch);

	/* enable the mac transmit and receive paths now */
	OSL_DELAY(2);
	cmdcfg &= ~CC_SR;
	cmdcfg |= (CC_RE | CC_TE);

	/* assert rx_ena and tx_ena when out of reset to enable the mac */
	W_REG(ch->osh, &regs->cmdcfg, cmdcfg);

	/* WAR to not force ht for 47162 when gmac is in rev mii mode */
	mode = ((R_REG(ch->osh, &regs->devstatus) & DS_MM_MASK) >> DS_MM_SHIFT);
	if ((CHIPID(ch->sih->chip) != BCM47162_CHIP_ID) || (mode != 0))
		/* request ht clock */
		OR_REG(ch->osh, &regs->clk_ctl_st, CS_FH);

	if ((CHIPID(ch->sih->chip) == BCM47162_CHIP_ID) && (mode == 2))
		si_pmu_chipcontrol(ch->sih, PMU_CHIPCTL1, PMU_CC1_RXC_DLL_BYPASS,
		                   PMU_CC1_RXC_DLL_BYPASS);

	/* init the mac data period. the value is set according to expr
	 * ((128ns / bp_clk) - 3).
	 */
	rxqctl = R_REG(ch->osh, &regs->rxqctl);
	rxqctl &= ~RC_MDP_MASK;
	bp_clk = si_clock(ch->sih) / 1000000;
	mdp = ((bp_clk * 128) / 1000) - 3;
	W_REG(ch->osh, &regs->rxqctl, rxqctl | (mdp << RC_MDP_SHIFT));

	return;
}

static void
gmac_txflowcontrol(ch_t *ch, bool on)
{
	uint32 cmdcfg;

	cmdcfg = R_REG(ch->osh, &ch->regs->cmdcfg);

	/* put the mac in reset */
	gmac_init_reset(ch);

	/* to enable tx flow control clear the rx pause ignore bit */
	if (on)
		cmdcfg &= ~CC_RPI;
	else
		cmdcfg |= CC_RPI;

	W_REG(ch->osh, &ch->regs->cmdcfg, cmdcfg);

	/* bring mac out of reset */
	gmac_clear_reset(ch);
}

static void
gmac_miiconfig(ch_t *ch)
{
	uint32 devstatus, mode;
	gmacregs_t *regs;

	regs = ch->regs;

	/* Read the devstatus to figure out the configuration
	 * mode of the interface.
	 */
	devstatus = R_REG(ch->osh, &regs->devstatus);
	mode = ((devstatus & DS_MM_MASK) >> DS_MM_SHIFT);

	/* Set the speed to 100 if the switch interface is 
	 * using mii/rev mii.
	 */
	if ((mode == 0) || (mode == 1)) {
		if (ch->etc->forcespeed == ET_AUTO)
			gmac_speed(ch, ET_100FULL);
		else
			gmac_speed(ch, ch->etc->forcespeed);
	}
}

static void
chipreset(ch_t *ch)
{
	gmacregs_t *regs;
	uint32 i, sflags, flagbits = 0;

	ET_TRACE(("et%d: chipreset\n", ch->etc->unit));

	regs = ch->regs;

	if (!si_iscoreup(ch->sih)) {
		if (!ch->etc->nicmode)
			si_pci_setup(ch->sih, (1 << si_coreidx(ch->sih)));
		/* power on reset: reset the enet core */
		goto chipinreset;
	}

	/* update software counters before resetting the chip */
	if (ch->mibgood)
		chipstatsupd(ch);

	/* reset the tx dma engines */
	for (i = 0; i < NUMTXQ; i++) {
		if (ch->di[i]) {
			ET_TRACE(("et%d: resetting tx dma%d\n", ch->etc->unit, i));
			dma_txreset(ch->di[i]);
		}
	}

	/* set gmac into loopback mode to ensure no rx traffic */
	gmac_loopback(ch, LOOPBACK_MODE_MAC);
	OSL_DELAY(1);

	/* reset the rx dma engine */
	if (ch->di[RX_Q0]) {
		ET_TRACE(("et%d: resetting rx dma\n", ch->etc->unit));
		dma_rxreset(ch->di[RX_Q0]);
	}

	/* clear the multicast filter table */
	gmac_mf_cleanup(ch);

chipinreset:
	if ((sflags = si_core_sflags(ch->sih, 0, 0)) & SISF_SW_ATTACHED) {
		ET_TRACE(("et%d: internal switch attached\n", ch->etc->unit));
		flagbits = SICF_SWCLKE;
		if (!ch->etc->robo) {
			ET_TRACE(("et%d: reseting switch\n", ch->etc->unit));
			flagbits |= SICF_SWRST;
		}
	}

	/* reset core */
	si_core_reset(ch->sih, flagbits, 0);

	if ((sflags & SISF_SW_ATTACHED) && (!ch->etc->robo)) {
		ET_TRACE(("et%d: taking switch out of reset\n", ch->etc->unit));
		si_core_cflags(ch->sih, SICF_SWRST, 0);
	}

	/* reset gmac */
	gmac_reset(ch);

	/* clear mib */
	gmac_clearmib(ch);
	ch->mibgood = TRUE;

	OR_REG(ch->osh, &regs->phycontrol, PC_MTE);

	/* Read the devstatus to figure out the configuration mode of
	 * the interface. Set the speed to 100 if the switch interface
	 * is mii/rmii.
	 */
	gmac_miiconfig(ch);

	/* gmac doesn't have internal phy */
	chipphyinit(ch, ch->etc->phyaddr);

	/* clear persistent sw intstatus */
	ch->intstatus = 0;
}

/*
 * Lookup a multicast address in the filter hash table.
 */
static int
gmac_mf_lkup(ch_t *ch, struct ether_addr *mcaddr)
{
	mflist_t *ptr;

	/* find the multicast address */
	for (ptr = ch->mf.bucket[GMAC_MCADDR_HASH(mcaddr)]; ptr != NULL; ptr = ptr->next) {
		if (!ETHER_MCADDR_CMP(&ptr->mc_addr, mcaddr))
			return (SUCCESS);
	}

	return (FAILURE);
}

/*
 * Add a multicast address to the filter hash table.
 */
static int
gmac_mf_add(ch_t *ch, struct ether_addr *mcaddr)
{
	uint32 hash;
	mflist_t *entry;

	/* add multicast addresses only */
	if (!ETHER_ISMULTI(mcaddr)) {
		ET_ERROR(("et%d: adding invalid multicast address %s\n",
		          ch->etc->unit, bcm_ether_ntoa(mcaddr, mac)));
		return (FAILURE);
	}

	/* discard duplicate add requests */
	if (gmac_mf_lkup(ch, mcaddr) == SUCCESS) {
		ET_ERROR(("et%d: adding duplicate mcast filter entry\n", ch->etc->unit));
		return (FAILURE);
	}

	/* allocate memory for list entry */
	entry = MALLOC(ch->osh, sizeof(mflist_t));
	if (entry == NULL) {
		ET_ERROR(("et%d: out of memory allocating mcast filter entry\n", ch->etc->unit));
		return (FAILURE);
	}

	/* add the entry to the hash bucket */
	ether_copy(mcaddr, &entry->mc_addr);
	hash = GMAC_MCADDR_HASH(mcaddr);
	entry->next = ch->mf.bucket[hash];
	ch->mf.bucket[hash] = entry;

	return (SUCCESS);
}

/*
 * Cleanup the multicast filter hash table.
 */
static void
gmac_mf_cleanup(ch_t *ch)
{
	mflist_t *ptr;
	int32 i;

	for (i = 0; i < GMAC_HASHT_SIZE; i++) {
		for (ptr = ch->mf.bucket[i]; ptr != NULL; ptr = ptr->next)
			MFREE(ch->osh, ptr, sizeof(mflist_t));
		ch->mf.bucket[i] = NULL;
	}
}

/*
 * Initialize all the chip registers.  If dma mode, init tx and rx dma engines
 * but leave the devcontrol tx and rx (fifos) disabled.
 */
static void
chipinit(ch_t *ch, uint options)
{
	etc_info_t *etc;
	gmacregs_t *regs;
	uint idx;
	uint i;

	regs = ch->regs;
	etc = ch->etc;
	idx = 0;

	ET_TRACE(("et%d: chipinit\n", etc->unit));

	/* Do timeout fixup */
	si_core_tofixup(ch->sih);

	/* enable one rx interrupt per received frame */
	W_REG(ch->osh, &regs->intrecvlazy, (1 << IRL_FC_SHIFT));

	/* enable 802.3x tx flow control (honor received PAUSE frames) */
	gmac_txflowcontrol(ch, TRUE);

	/* enable/disable promiscuous mode */
	gmac_promisc(ch, etc->promisc);

	if (!etc->promisc) {
		/* set our local address */
		W_REG(ch->osh, &regs->macaddrhigh,
		      hton32(*(uint32 *)&etc->cur_etheraddr.octet[0]));
		W_REG(ch->osh, &regs->macaddrlow,
		      hton16(*(uint16 *)&etc->cur_etheraddr.octet[4]));

		/* gmac doesn't have a cam, hence do the multicast address filtering
		 * in the software
		 */
		/* allmulti or a list of discrete multicast addresses */
		if (!etc->allmulti && etc->nmulticast)
			for (i = 0; i < etc->nmulticast; i++)
				(void)gmac_mf_add(ch, &etc->multicast[i]);
	}

	/* optionally enable mac-level loopback */
	if (etc->loopbk)
		gmac_loopback(ch, LOOPBACK_MODE_MAC);
	else
		gmac_loopback(ch, LOOPBACK_MODE_NONE);

	/* set max frame lengths - account for possible vlan tag */
	W_REG(ch->osh, &regs->rxmaxlength, ETHER_MAX_LEN + 32);

	/*
	 * Optionally, disable phy autonegotiation and force our speed/duplex
	 * or constrain our advertised capabilities.
	 */
	if (etc->forcespeed != ET_AUTO) {
		gmac_speed(ch, etc->forcespeed);
		chipphyforce(ch, etc->phyaddr);
	} else if (etc->advertise && etc->needautoneg)
		chipphyadvertise(ch, etc->phyaddr);

	/* enable the overflow continue feature and disable parity */
	dma_ctrlflags(ch->di[0], DMA_CTRL_ROC | DMA_CTRL_PEN /* mask */,
	              DMA_CTRL_ROC /* value */);

	if (options & ET_INIT_FULL) {
		/* initialize the tx and rx dma channels */
		for (i = 0; i < NUMTXQ; i++)
			dma_txinit(ch->di[i]);
		dma_rxinit(ch->di[RX_Q0]);

		/* post dma receive buffers */
		dma_rxfill(ch->di[RX_Q0]);

		/* lastly, enable interrupts */
		if (options & ET_INIT_INTRON)
			et_intrson(etc->et);
	}
	else
		dma_rxenable(ch->di[RX_Q0]);

	/* turn on the emac */
	gmac_enable(ch);
}

/* dma transmit */
static bool BCMFASTPATH
chiptx(ch_t *ch, void *p0)
{
	int error, len;
	uint32 q = TX_Q0;

	ET_TRACE(("et%d: chiptx\n", ch->etc->unit));
	ET_LOG("et%d: chiptx", ch->etc->unit, 0);

#ifdef ETROBO
	if ((ch->etc->robo != NULL) &&
	    (((robo_info_t *)ch->etc->robo)->devid == DEVID53115)) {
		void *p = p0;

	    	if ((p0 = etc_bcm53115_war(ch->etc, p)) == NULL) {
			PKTFREE(ch->osh, p, TRUE);
			return FALSE;
		}
	}
#endif /* ETROBO */

	len = PKTLEN(ch->osh, p0);

	/* check tx max length */
	if (len > (ETHER_MAX_LEN + 32)) {
		ET_ERROR(("et%d: chiptx: max frame length exceeded\n",
		          ch->etc->unit));
		return FALSE;
	}

	if ((len < GMAC_MIN_FRAMESIZE) && (ch->etc->corerev == 0))
		PKTSETLEN(ch->osh, p0, GMAC_MIN_FRAMESIZE);

	/* queue the packet based on its priority */
	if (ch->etc->qos)
		q = etc_up2tc(PKTPRIO(p0));

	ASSERT(q < NUMTXQ);
	error = dma_txfast(ch->di[q], p0, TRUE);

	/* set back the orig length */
	PKTSETLEN(ch->osh, p0, len);

	if (error) {
		ET_ERROR(("et%d: chiptx: out of txds\n", ch->etc->unit));
		ch->etc->txnobuf++;
		return FALSE;
	}

	return TRUE;
}

/* reclaim completed transmit descriptors and packets */
static void BCMFASTPATH
chiptxreclaim(ch_t *ch, bool forceall)
{
	int32 i;

	ET_TRACE(("et%d: chiptxreclaim\n", ch->etc->unit));

	for (i = 0; i < NUMTXQ; i++) {
		dma_txreclaim(ch->di[i], forceall);
		ch->intstatus &= ~(I_XI0 << i);
	}
}

/* dma receive: returns a pointer to the next frame received, or NULL if there are no more */
static void * BCMFASTPATH
chiprx(ch_t *ch)
{
	void *p;
	struct ether_addr *da;

	ET_TRACE(("et%d: chiprx\n", ch->etc->unit));
	ET_LOG("et%d: chiprx", ch->etc->unit, 0);

	/* gmac doesn't have a cam to do address filtering. so we implement
	 * the multicast address filtering here.
	 */
	while ((p = dma_rx(ch->di[RX_Q0])) != NULL) {
		/* check for overflow error packet */
		if (RXH_FLAGS(ch->etc, PKTDATA(ch->osh, p)) & GRXF_OVF) {
			PKTFREE(ch->osh, p, FALSE);
			ch->etc->rxoflodiscards++;
			continue;
		}

		/* skip the rx header */
		PKTPULL(ch->osh, p, HWRXOFF);

		/* do filtering only for multicast packets when allmulti is false */
		da = (struct ether_addr *)PKTDATA(ch->osh, p);
		if (!ETHER_ISMULTI(da) || ch->etc->allmulti ||
		    (gmac_mf_lkup(ch, da) == SUCCESS) || ETHER_ISBCAST(da)) {
			PKTPUSH(ch->osh, p, HWRXOFF);
			return (p);
		}

		PKTFREE(ch->osh, p, FALSE);
	}

	ch->intstatus &= ~I_RI;

	/* post more rx buffers since we consumed a few */
	dma_rxfill(ch->di[RX_Q0]);

	return (NULL);
}

/* reclaim completed dma receive descriptors and packets */
static void
chiprxreclaim(ch_t *ch)
{
	ET_TRACE(("et%d: chiprxreclaim\n", ch->etc->unit));
	dma_rxreclaim(ch->di[RX_Q0]);
	ch->intstatus &= ~I_RI;
}

/* allocate and post dma receive buffers */
static void BCMFASTPATH
chiprxfill(ch_t *ch)
{
	ET_TRACE(("et%d: chiprxfill\n", ch->etc->unit));
	ET_LOG("et%d: chiprx", ch->etc->unit, 0);
	dma_rxfill(ch->di[RX_Q0]);
}

/* get current and pending interrupt events */
static int BCMFASTPATH
chipgetintrevents(ch_t *ch, bool in_isr)
{
	uint32 intstatus;
	int events;

	events = 0;

	/* read the interrupt status register */
	intstatus = R_REG(ch->osh, &ch->regs->intstatus);

	/* defer unsolicited interrupts */
	intstatus &= (in_isr ? ch->intmask : DEF_INTMASK);

	if (intstatus != 0)
		events = INTR_NEW;

	/* or new bits into persistent intstatus */
	intstatus = (ch->intstatus |= intstatus);

	/* return if no events */
	if (intstatus == 0)
		return (0);

	/* convert chip-specific intstatus bits into generic intr event bits */
	if (intstatus & I_RI)
		events |= INTR_RX;
	if (intstatus & (I_XI0 | I_XI1 | I_XI2 | I_XI3))
		events |= INTR_TX;
	if (intstatus & I_ERRORS)
		events |= INTR_ERROR;

	return (events);
}

/* enable chip interrupts */
static void BCMFASTPATH
chipintrson(ch_t *ch)
{
	ch->intmask = DEF_INTMASK;
	W_REG(ch->osh, &ch->regs->intmask, ch->intmask);
}

/* disable chip interrupts */
static void BCMFASTPATH
chipintrsoff(ch_t *ch)
{
	/* disable further interrupts from gmac */
	W_REG(ch->osh, &ch->regs->intmask, 0);
	(void) R_REG(ch->osh, &ch->regs->intmask);	/* sync readback */
	ch->intmask = 0;

	/* clear the interrupt conditions */
	W_REG(ch->osh, &ch->regs->intstatus, ch->intstatus);
}

/* return true of caller should re-initialize, otherwise false */
static bool BCMFASTPATH
chiperrors(ch_t *ch)
{
	uint32 intstatus;
	etc_info_t *etc;

	etc = ch->etc;

	intstatus = ch->intstatus;
	ch->intstatus &= ~(I_ERRORS);

	ET_TRACE(("et%d: chiperrors: intstatus 0x%x\n", etc->unit, intstatus));

	if (intstatus & I_PDEE) {
		ET_ERROR(("et%d: descriptor error\n", etc->unit));
		etc->dmade++;
	}

	if (intstatus & I_PDE) {
		ET_ERROR(("et%d: data error\n", etc->unit));
		etc->dmada++;
	}

	if (intstatus & I_DE) {
		ET_ERROR(("et%d: descriptor protocol error\n", etc->unit));
		etc->dmape++;
	}

	if (intstatus & I_RDU) {
		ET_ERROR(("et%d: receive descriptor underflow\n", etc->unit));
		etc->rxdmauflo++;
	}

	if (intstatus & I_RFO) {
		ET_ERROR(("et%d: receive fifo overflow\n", etc->unit));
		etc->rxoflo++;
	}

	if (intstatus & I_XFU) {
		ET_ERROR(("et%d: transmit fifo underflow\n", etc->unit));
		etc->txuflo++;
	}

	/* if overflows or decriptors underflow, don't report it
	 * as an error and provoque a reset
	 */
	if (intstatus & ~(I_RDU | I_RFO) & I_ERRORS)
		return (TRUE);

	return (FALSE);
}

static void
chipstatsupd(ch_t *ch)
{
	etc_info_t *etc;
	gmacregs_t *regs;
	volatile uint32 *s;
	uint32 *d;

	etc = ch->etc;
	regs = ch->regs;

	/* read the mib counters and update the driver maintained software
	 * counters.
	 */
	OR_REG(ch->osh, &regs->devcontrol, DC_MROR);
	for (s = &regs->mib.tx_good_octets, d = &ch->mib.tx_good_octets;
	     s <=  &regs->mib.rx_uni_pkts; s++, d++) {
		*d += R_REG(ch->osh, s);
		if (s == &ch->regs->mib.tx_q3_octets_high) {
			s++;
			d++;
		}
	}


	/*
	 * Aggregate transmit and receive errors that probably resulted
	 * in the loss of a frame are computed on the fly.
	 *
	 * We seem to get lots of tx_carrier_lost errors when flipping
	 * speed modes so don't count these as tx errors.
	 *
	 * Arbitrarily lump the non-specific dma errors as tx errors.
	 */
	etc->txerror = ch->mib.tx_jabber_pkts + ch->mib.tx_oversize_pkts
		+ ch->mib.tx_underruns + ch->mib.tx_excessive_cols
		+ ch->mib.tx_late_cols + etc->txnobuf + etc->dmade
		+ etc->dmada + etc->dmape + etc->txuflo;
	etc->rxerror = ch->mib.rx_jabber_pkts + ch->mib.rx_oversize_pkts
		+ ch->mib.rx_missed_pkts + ch->mib.rx_crc_align_errs
		+ ch->mib.rx_undersize + ch->mib.rx_crc_errs
		+ ch->mib.rx_align_errs + ch->mib.rx_symbol_errs
		+ etc->rxnobuf + etc->rxdmauflo + etc->rxoflo + etc->rxbadlen;
	etc->rxgiants = (ch->di[RX_Q0])->rxgiants;
}

static void
chipdumpmib(ch_t *ch, struct bcmstrbuf *b)
{
	gmacmib_t *m;

	m = &ch->mib;

	bcm_bprintf(b, "tx_broadcast_pkts %d tx_multicast_pkts %d tx_jabber_pkts %d "
	               "tx_oversize_pkts %d\n",
	               m->tx_broadcast_pkts, m->tx_multicast_pkts,
	               m->tx_jabber_pkts,
	               m->tx_oversize_pkts);
	bcm_bprintf(b, "tx_fragment_pkts %d tx_underruns %d\n",
	               m->tx_fragment_pkts, m->tx_underruns);
	bcm_bprintf(b, "tx_total_cols %d tx_single_cols %d tx_multiple_cols %d "
	               "tx_excessive_cols %d\n",
	               m->tx_total_cols, m->tx_single_cols, m->tx_multiple_cols,
	               m->tx_excessive_cols);
	bcm_bprintf(b, "tx_late_cols %d tx_defered %d tx_carrier_lost %d tx_pause_pkts %d\n",
	               m->tx_late_cols, m->tx_defered, m->tx_carrier_lost,
	               m->tx_pause_pkts);

	/* receive stat counters */
	/* hardware mib pkt and octet counters wrap too quickly to be useful */
	bcm_bprintf(b, "rx_broadcast_pkts %d rx_multicast_pkts %d rx_jabber_pkts %d "
	               "rx_oversize_pkts %d\n",
	               m->rx_broadcast_pkts, m->rx_multicast_pkts,
	               m->rx_jabber_pkts, m->rx_oversize_pkts);
	bcm_bprintf(b, "rx_fragment_pkts %d rx_missed_pkts %d rx_crc_align_errs %d "
	               "rx_undersize %d\n",
	               m->rx_fragment_pkts, m->rx_missed_pkts,
	               m->rx_crc_align_errs, m->rx_undersize);
	bcm_bprintf(b, "rx_crc_errs %d rx_align_errs %d rx_symbol_errs %d\n",
	               m->rx_crc_errs, m->rx_align_errs, m->rx_symbol_errs);
	bcm_bprintf(b, "rx_pause_pkts %d rx_nonpause_pkts %d\n",
	               m->rx_pause_pkts, m->rx_nonpause_pkts);
}

static void
chipenablepme(ch_t *ch)
{
	return;
}

static void
chipdisablepme(ch_t *ch)
{
	return;
}

static void
chipduplexupd(ch_t *ch)
{
	uint32 cmdcfg;
	int32 duplex, speed;

	cmdcfg = R_REG(ch->osh, &ch->regs->cmdcfg);

	/* check if duplex mode changed */
	if (ch->etc->duplex && (cmdcfg & CC_HD))
		duplex = 0;
	else if (!ch->etc->duplex && ((cmdcfg & CC_HD) == 0))
		duplex = CC_HD;
	else
		duplex = -1;

	/* check if the speed changed */
	speed = ((cmdcfg & CC_ES_MASK) >> CC_ES_SHIFT);
	if ((ch->etc->speed == 1000) && (speed != 2))
		speed = 2;
	else if ((ch->etc->speed == 100) && (speed != 1))
		speed = 1;
	else if ((ch->etc->speed == 10) && (speed != 0))
		speed = 0;
	else
		speed = -1;

	/* no duplex or speed change required */
	if ((speed == -1) && (duplex == -1))
		return;

	/* update the speed */
	if (speed != -1) {
		cmdcfg &= ~CC_ES_MASK;
		cmdcfg |= (speed << CC_ES_SHIFT);
	}

	/* update the duplex mode */
	if (duplex != -1) {
		cmdcfg &= ~CC_HD;
		cmdcfg |= duplex;
	}

	ET_TRACE(("chipduplexupd: updating speed & duplex %x\n", cmdcfg));

	/* put mac in reset */
	gmac_init_reset(ch);

	W_REG(ch->osh, &ch->regs->cmdcfg, cmdcfg);

	/* bring mac out of reset */
	gmac_clear_reset(ch);
}

static uint16
chipphyrd(ch_t *ch, uint phyaddr, uint reg)
{
	uint32 tmp;
	gmacregs_t *regs;

	ASSERT(phyaddr < MAXEPHY);
	ASSERT(reg < MAXPHYREG);

	regs = ch->regs;

	/* issue the read */
	tmp = R_REG(ch->osh, &regs->phycontrol);
	tmp &= ~0x1f;
	tmp |= phyaddr;
	W_REG(ch->osh, &regs->phycontrol, tmp);
	W_REG(ch->osh, &regs->phyaccess,
	      (PA_START | (phyaddr << PA_ADDR_SHIFT) | (reg << PA_REG_SHIFT)));

	/* wait for it to complete */
	SPINWAIT((R_REG(ch->osh, &regs->phyaccess) & PA_START), 1000);
	tmp = R_REG(ch->osh, &regs->phyaccess);
	if (tmp & PA_START) {
		ET_ERROR(("et%d: chipphyrd: did not complete\n", ch->etc->unit));
		tmp = 0xffff;
	}

	return (tmp & PA_DATA_MASK);
}

static void
chipphywr(ch_t *ch, uint phyaddr, uint reg, uint16 v)
{
	uint32 tmp;
	gmacregs_t *regs;

	ASSERT(phyaddr < MAXEPHY);
	ASSERT(reg < MAXPHYREG);

	regs = ch->regs;

	/* clear mdioint bit of intstatus first  */
	tmp = R_REG(ch->osh, &regs->phycontrol);
	tmp &= ~0x1f;
	tmp |= phyaddr;
	W_REG(ch->osh, &regs->phycontrol, tmp);
	W_REG(ch->osh, &regs->intstatus, I_MDIO);
	ASSERT((R_REG(ch->osh, &regs->intstatus) & I_MDIO) == 0);

	/* issue the write */
	W_REG(ch->osh, &regs->phyaccess,
	      (PA_START | PA_WRITE | (phyaddr << PA_ADDR_SHIFT) | (reg << PA_REG_SHIFT) | v));

	/* wait for it to complete */
	SPINWAIT((R_REG(ch->osh, &regs->phyaccess) & PA_START), 1000);
	if (R_REG(ch->osh, &regs->phyaccess) & PA_START) {
		ET_ERROR(("et%d: chipphywr: did not complete\n", ch->etc->unit));
	}
}

static void
chipphyor(ch_t *ch, uint phyaddr, uint reg, uint16 v)
{
	uint16 tmp;

	tmp = chipphyrd(ch, phyaddr, reg);
	tmp |= v;
	chipphywr(ch, phyaddr, reg, tmp);
}

static void
chipphyreset(ch_t *ch, uint phyaddr)
{
	ASSERT(phyaddr < MAXEPHY);

	if (phyaddr == EPHY_NOREG)
		return;

	ET_TRACE(("et%d: chipphyreset: phyaddr %d\n", ch->etc->unit, phyaddr));

	chipphywr(ch, phyaddr, 0, CTL_RESET);
	OSL_DELAY(100);
	if (chipphyrd(ch, phyaddr, 0) & CTL_RESET) {
		ET_ERROR(("et%d: chipphyreset: reset not complete\n", ch->etc->unit));
	}

	chipphyinit(ch, phyaddr);
}

static void
chipphyinit(ch_t *ch, uint phyaddr)
{
	int i;

	if (CHIPID(ch->sih->chip) == BCM5356_CHIP_ID && ch->sih->chiprev == 0) {
		for (i = 0; i < 5; i++) {
			if (i != 2) {
				chipphywr(ch, i, 0x04, 0x0461);
			}
			chipphywr(ch, i, 0x1f, 0x008b);
			chipphywr(ch, i, 0x1d, 0x0100);
			if (i == 2) {
				chipphywr(ch, 2, 0x1f, 0xf);
				chipphywr(ch, 2, 0x13, 0xa842);
			}
			chipphywr(ch, i, 0x1f, 0x000b);
			OSL_DELAY(300000);
		}
	} else if (CHIPID(ch->sih->chip) == BCM5356_CHIP_ID && ch->sih->chiprev > 0) {
		for (i = 0; i < 5; i++) {
			chipphywr(ch, i, 0x1f, 0x008b);
			chipphywr(ch, i, 0x15, 0x0100);
			chipphywr(ch, i, 0x1f, 0x000f);
			chipphywr(ch, i, 0x12, 0x2aaa);
			chipphywr(ch, i, 0x1f, 0x000b);
		}
	}

	if (phyaddr == EPHY_NOREG)
		return;

	ET_TRACE(("et%d: chipphyinit: phyaddr %d\n", ch->etc->unit, phyaddr));
}

static void
chipphyforce(ch_t *ch, uint phyaddr)
{
	etc_info_t *etc;
	uint16 ctl;

	ASSERT(phyaddr < MAXEPHY);

	if (phyaddr == EPHY_NOREG)
		return;

	etc = ch->etc;

	if (etc->forcespeed == ET_AUTO)
		return;

	ET_TRACE(("et%d: chipphyforce: phyaddr %d speed %d\n",
	          ch->etc->unit, phyaddr, etc->forcespeed));

	ctl = chipphyrd(ch, phyaddr, 0);
	ctl &= ~(CTL_SPEED | CTL_SPEED_MSB | CTL_ANENAB | CTL_DUPLEX);

	switch (etc->forcespeed) {
		case ET_10HALF:
			break;

		case ET_10FULL:
			ctl |= CTL_DUPLEX;
			break;

		case ET_100HALF:
			ctl |= CTL_SPEED_100;
			break;

		case ET_100FULL:
			ctl |= (CTL_SPEED_100 | CTL_DUPLEX);
			break;

		case ET_1000FULL:
			ctl |= (CTL_SPEED_1000 | CTL_DUPLEX);
			break;
	}

	chipphywr(ch, phyaddr, 0, ctl);
}

/* set selected capability bits in autonegotiation advertisement */
static void
chipphyadvertise(ch_t *ch, uint phyaddr)
{
	etc_info_t *etc;
	uint16 adv, adv2;

	ASSERT(phyaddr < MAXEPHY);

	if (phyaddr == EPHY_NOREG)
		return;

	etc = ch->etc;

	if ((etc->forcespeed != ET_AUTO) || !etc->needautoneg)
		return;

	ASSERT(etc->advertise);

	ET_TRACE(("et%d: chipphyadvertise: phyaddr %d advertise %x\n",
	          ch->etc->unit, phyaddr, etc->advertise));

	/* reset our advertised capabilitity bits */
	adv = chipphyrd(ch, phyaddr, 4);
	adv &= ~(ADV_100FULL | ADV_100HALF | ADV_10FULL | ADV_10HALF);
	adv |= etc->advertise;
	chipphywr(ch, phyaddr, 4, adv);

	adv2 = chipphyrd(ch, phyaddr, 9);
	adv2 &= ~(ADV_1000FULL | ADV_1000HALF);
	adv2 |= etc->advertise2;
	chipphywr(ch, phyaddr, 9, adv2);

	ET_TRACE(("et%d: chipphyadvertise: phyaddr %d adv %x adv2 %x phyad0 %x\n",
	          ch->etc->unit, phyaddr, adv, adv2, chipphyrd(ch, phyaddr, 0)));

	/* restart autonegotiation */
	chipphyor(ch, phyaddr, 0, CTL_RESTART);

	etc->needautoneg = FALSE;
}

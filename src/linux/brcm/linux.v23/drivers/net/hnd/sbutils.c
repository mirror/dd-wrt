/*
 * Misc utility routines for accessing chip-specific features
 * of the SiliconBackplane-based Broadcom chips.
 *
 * Copyright 2005, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 * $Id: sbutils.c,v 1.6 2005/03/07 08:35:32 kanki Exp $
 */

#include <typedefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <bcmdevs.h>
#include <sbconfig.h>
#include <sbchipc.h>
#include <sbpci.h>
#include <pcicfg.h>
#include <sbpcmcia.h>
#include <sbextif.h>
#include <sbutils.h>
#include <bcmsrom.h>

/* debug/trace */
#define	SB_ERROR(args)


typedef uint32 (*sb_intrsoff_t)(void *intr_arg);
typedef void (*sb_intrsrestore_t)(void *intr_arg, uint32 arg);
typedef bool (*sb_intrsenabled_t)(void *intr_arg);

/* misc sb info needed by some of the routines */
typedef struct sb_info {
	uint	chip;			/* chip number */
	uint	chiprev;		/* chip revision */
	uint	chippkg;		/* chip package option */
	uint	boardtype;		/* board type */
	uint	boardvendor;		/* board vendor id */
	uint	bustype;		/* what bus type we are going through */

	void	*osh;			/* osl os handle */
	void	*sdh;			/* bcmsdh handle */

	void	*curmap;		/* current regs va */
	void	*regs[SB_MAXCORES];	/* other regs va */

	uint	curidx;			/* current core index */
	uint	dev_coreid;		/* the core provides driver functions */
	uint	pciidx;			/* pci core index */
	uint	pcirev;			/* pci core rev */

	uint	pcmciaidx;		/* pcmcia core index */
	uint	pcmciarev;		/* pcmcia core rev */
	bool	memseg;			/* flag to toggle MEM_SEG register */

	uint	ccrev;			/* chipc core rev */

	uint	gpioidx;		/* gpio control core index */
	uint	gpioid;			/* gpio control coretype */

	uint	numcores;		/* # discovered cores */
	uint	coreid[SB_MAXCORES];	/* id of each core */

	void	*intr_arg;		/* interrupt callback function arg */
	sb_intrsoff_t		intrsoff_fn;		/* function turns chip interrupts off */
	sb_intrsrestore_t	intrsrestore_fn;	/* function restore chip interrupts */
	sb_intrsenabled_t	intrsenabled_fn;	/* function to check if chip interrupts are enabled */
} sb_info_t;

/* local prototypes */
static void* BCMINIT(sb_doattach)(sb_info_t *si, uint devid, void *osh, void *regs, uint bustype, void *sdh, char **vars, int *varsz);
static void BCMINIT(sb_scan)(sb_info_t *si);
static uint sb_corereg(void *sbh, uint coreidx, uint regoff, uint mask, uint val);
static uint _sb_coreidx(void *sbh);
static uint sb_findcoreidx(void *sbh, uint coreid, uint coreunit);
static uint BCMINIT(sb_pcidev2chip)(uint pcidev);
static uint BCMINIT(sb_chip2numcores)(uint chip);

#define	SB_INFO(sbh)	(sb_info_t*)sbh
#define	SET_SBREG(sbh, r, mask, val)	W_SBREG((sbh), (r), ((R_SBREG((sbh), (r)) & ~(mask)) | (val)))
#define	GOODCOREADDR(x)	(((x) >= SB_ENUM_BASE) && ((x) <= SB_ENUM_LIM) && ISALIGNED((x), SB_CORE_SIZE))
#define	GOODREGS(regs)	((regs) && ISALIGNED((uintptr)(regs), SB_CORE_SIZE))
#define	REGS2SB(va)	(sbconfig_t*) ((int8*)(va) + SBCONFIGOFF)
#define	GOODIDX(idx)	(((uint)idx) < SB_MAXCORES)
#define	BADIDX		(SB_MAXCORES+1)
#define	NOREV		(SBIDH_RC_MASK + 1)

#define	R_SBREG(sbh, sbr)	sb_read_sbreg((sbh), (sbr))
#define	W_SBREG(sbh, sbr, v)	sb_write_sbreg((sbh), (sbr), (v))
#define	AND_SBREG(sbh, sbr, v)	W_SBREG((sbh), (sbr), (R_SBREG((sbh), (sbr)) & (v)))
#define	OR_SBREG(sbh, sbr, v)	W_SBREG((sbh), (sbr), (R_SBREG((sbh), (sbr)) | (v)))

/*
 * Macros to disable/restore function core(D11, ENET, ILINE20, etc) interrupts before/
 * after core switching to avoid invalid register accesss inside ISR.
 */
#define INTR_OFF(si, intr_val) \
	if ((si)->intrsoff_fn && (si)->coreid[(si)->curidx] == (si)->dev_coreid) {	\
		intr_val = (*(si)->intrsoff_fn)((si)->intr_arg); }
#define INTR_RESTORE(si, intr_val) \
	if ((si)->intrsrestore_fn && (si)->coreid[(si)->curidx] == (si)->dev_coreid) {	\
		(*(si)->intrsrestore_fn)((si)->intr_arg, intr_val); }

/* power control defines */
#define	LPOMINFREQ	25000			/* low power oscillator min */
#define	LPOMAXFREQ	43000			/* low power oscillator max */
#define	XTALMINFREQ	19800000		/* 20mhz - 1% */
#define	XTALMAXFREQ	20200000		/* 20mhz + 1% */
#define	PCIMINFREQ	25000000		/* 25mhz */
#define	PCIMAXFREQ	34000000		/* 33mhz + fudge */
#define SCC_DEF_DIV	0			/* default slow clock divider */

#define XTAL_ON_DELAY		1000	/* Xtal power on delay in us */

#define SCC_LOW2FAST_LIMIT	5000	/* turn on fast clock time, in unit of ms */

static uint32
sb_read_sbreg(void *sbh, volatile uint32 *sbr)
{
	sb_info_t *si;
	uint8 tmp;
	uint32 val, intr_val = 0;

	si = SB_INFO(sbh);

	/*
	 * compact flash only has 11 bits address, while we needs 12 bits address.
	 * MEM_SEG will be OR'd with other 11 bits address in hardware,
	 * so we program MEM_SEG with 12th bit when necessary(access sb regsiters).
	 * For normal PCMCIA bus(CFTable_regwinsz > 2k), do nothing special
	 */
	if(si->memseg) {
		INTR_OFF(si, intr_val);
		tmp = 1;
		OSL_PCMCIA_WRITE_ATTR(si->osh, MEM_SEG, &tmp, 1);
		(uintptr)sbr &= ~(1 << 11);	/* mask out bit 11*/
	}

	val = R_REG(sbr);

	if(si->memseg) {
		tmp = 0;
		OSL_PCMCIA_WRITE_ATTR(si->osh, MEM_SEG, &tmp, 1);
		INTR_RESTORE(si, intr_val);
	}

	return (val);
}

static void
sb_write_sbreg(void *sbh, volatile uint32 *sbr, uint32 v)
{
	sb_info_t *si;
	uint8 tmp;
	volatile uint32 dummy;
	uint32 intr_val = 0;

	si = SB_INFO(sbh);

	/*
	 * compact flash only has 11 bits address, while we needs 12 bits address.
	 * MEM_SEG will be OR'd with other 11 bits address in hardware,
	 * so we program MEM_SEG with 12th bit when necessary(access sb regsiters).
	 * For normal PCMCIA bus(CFTable_regwinsz > 2k), do nothing special
	 */
	if(si->memseg) {
		INTR_OFF(si, intr_val);
		tmp = 1;
		OSL_PCMCIA_WRITE_ATTR(si->osh, MEM_SEG, &tmp, 1);
		(uintptr)sbr &= ~(1 << 11);	/* mask out bit 11 */
	}

	if (BUSTYPE(si->bustype) == PCMCIA_BUS) {
#ifdef IL_BIGENDIAN
		dummy = R_REG(sbr);
		W_REG(((volatile uint16 *)sbr + 1), (uint16)((v >> 16) & 0xffff));
		dummy = R_REG(sbr);
		W_REG((volatile uint16 *)sbr, (uint16)(v & 0xffff));
#else
		dummy = R_REG(sbr);
		W_REG((volatile uint16 *)sbr, (uint16)(v & 0xffff));
		dummy = R_REG(sbr);
		W_REG(((volatile uint16 *)sbr + 1), (uint16)((v >> 16) & 0xffff));
#endif
	} else
		W_REG(sbr, v);

	if(si->memseg) {
		tmp = 0;
		OSL_PCMCIA_WRITE_ATTR(si->osh, MEM_SEG, &tmp, 1);
		INTR_RESTORE(si, intr_val);
	}
}

/*
 * Allocate a sb handle.
 * devid - pci device id (used to determine chip#)
 * osh - opaque OS handle
 * regs - virtual address of initial core registers
 * bustype - pci/pcmcia/sb/sdio/etc
 * vars - pointer to a pointer area for "environment" variables
 * varsz - pointer to int to return the size of the vars
 */
void* 
BCMINITFN(sb_attach)(uint devid, void *osh, void *regs, uint bustype, void *sdh, char **vars, int *varsz)
{
	sb_info_t *si;

	/* alloc sb_info_t */
	if ((si = MALLOC(osh, sizeof (sb_info_t))) == NULL) {
		SB_ERROR(("sb_attach: malloc failed! malloced %d bytes\n", MALLOCED(osh)));
		return (NULL);
	}

	if (BCMINIT(sb_doattach)(si, devid, osh, regs, bustype, sdh, vars, varsz) == NULL) {
		MFREE(osh, si, sizeof (sb_info_t));
		return (NULL);
	}
	return si;
}

/* Using sb_kattach depends on SB_BUS support, either implicit  */
/* no limiting BCMBUSTYPE value) or explicit (value is SB_BUS). */
#if !defined(BCMBUSTYPE) || (BCMBUSTYPE == SB_BUS)

/* global kernel resource */
static sb_info_t ksi;

/* generic kernel variant of sb_attach() */
void* 
BCMINITFN(sb_kattach)()
{
	uint32 *regs;
	char *unused;
	int varsz;

	if (ksi.curmap == NULL) {
		uint32 cid;

		regs = (uint32 *)REG_MAP(SB_ENUM_BASE, SB_CORE_SIZE);
		cid = R_REG((uint32 *)regs);
		if (((cid & CID_ID_MASK) == BCM4712_DEVICE_ID) &&
		    ((cid & CID_PKG_MASK) != BCM4712LARGE_PKG_ID) &&
		    ((cid & CID_REV_MASK) <= (3 << CID_REV_SHIFT))) {
			uint32 *scc, val;

			scc = (uint32 *)((uchar*)regs + OFFSETOF(chipcregs_t, slow_clk_ctl));
			val = R_REG(scc);
			SB_ERROR(("    initial scc = 0x%x\n", val));
			val |= SCC_SS_XTAL;
			W_REG(scc, val);
		}

		if (BCMINIT(sb_doattach)(&ksi, BCM4710_DEVICE_ID, NULL, (void*)regs,
			SB_BUS, NULL, &unused, &varsz) == NULL) {
			return NULL;
		}
	}

	return &ksi;
}
#endif

static void* 
BCMINITFN(sb_doattach)(sb_info_t *si, uint devid, void *osh, void *regs, uint bustype, void *sdh, char **vars, int *varsz)
{
	uint origidx;
	chipcregs_t *cc;
	uint32 w;
	int res;

	ASSERT(GOODREGS(regs));

	bzero((uchar*)si, sizeof (sb_info_t));

	si->pciidx = si->gpioidx = BADIDX;

	si->osh = osh;
	si->curmap = regs;
	si->sdh = sdh;

	/* check to see if we are a sb core mimic'ing a pci core */
	if (bustype == PCI_BUS) {
		if (OSL_PCI_READ_CONFIG(osh, PCI_SPROM_CONTROL, sizeof (uint32)) == 0xffffffff)
			bustype = SB_BUS;
		else
			bustype = PCI_BUS;
	}

	si->bustype = bustype;
	if (si->bustype != BUSTYPE(si->bustype)) {
		SB_ERROR(("sb_doattach: bus type %d does not match configured bus type %d\n",
			  si->bustype, BUSTYPE(si->bustype)));
		return NULL;
	}

	/* need to set memseg flag for CF card first before any sb registers access */
	if (BUSTYPE(si->bustype) == PCMCIA_BUS)
		si->memseg = TRUE;

	/* kludge to enable the clock on the 4306 which lacks a slowclock */
	if (BUSTYPE(si->bustype) == PCI_BUS)
		sb_pwrctl_xtal((void*)si, XTAL|PLL, ON);

	/* initialize current core index value */
	si->curidx = _sb_coreidx((void*)si);
	if (si->curidx == BADIDX) {
		return NULL;
	}

	/* keep and reuse the initial register mapping */
	origidx = si->curidx;
	if (BUSTYPE(si->bustype) == SB_BUS)
		si->regs[origidx] = regs;

	/* is core-0 a chipcommon core? */
	si->numcores = 1;
	cc = (chipcregs_t*) sb_setcoreidx((void*)si, 0);
	if (sb_coreid((void*)si) != SB_CC)
		cc = NULL;

	/* determine chip id and rev */
	if (cc) {
		/* chip common core found! */
		si->chip = R_REG(&cc->chipid) & CID_ID_MASK;
		si->chiprev = (R_REG(&cc->chipid) & CID_REV_MASK) >> CID_REV_SHIFT;
		si->chippkg = (R_REG(&cc->chipid) & CID_PKG_MASK) >> CID_PKG_SHIFT;
	} else {
		/* The only pcmcia chip without a chipcommon core is a 4301 */
		if (BUSTYPE(si->bustype) == PCMCIA_BUS)
			devid = BCM4301_DEVICE_ID;

		/* no chip common core -- must convert device id to chip id */
		if ((si->chip = BCMINIT(sb_pcidev2chip)(devid)) == 0) {
			SB_ERROR(("sb_attach: unrecognized device id 0x%04x\n", devid));
			return NULL;
		}
	}

	/* get chipcommon rev */
	si->ccrev = cc ? sb_corerev((void*)si) : NOREV;

	/* determine numcores */
	if (cc && ((si->ccrev == 4) || (si->ccrev >= 6)))
		si->numcores = (R_REG(&cc->chipid) & CID_CC_MASK) >> CID_CC_SHIFT;
	else
		si->numcores = BCMINIT(sb_chip2numcores)(si->chip);

	/* return to original core */
	sb_setcoreidx((void*)si, origidx);

	/* sanity checks */
	ASSERT(si->chip);

	/* scan for cores */
	BCMINIT(sb_scan)(si);

	/* srom_var_init() depends on sb_scan() info */
	if ((res = srom_var_init(si, si->bustype, si->curmap, osh, vars, varsz))) {
		SB_ERROR(("sb_attach: srom_var_init failed: bad srom\n"));
		return (NULL);
	}
	
	if (cc == NULL) {
		/*
		 * The chip revision number is hardwired into all
		 * of the pci function config rev fields and is
		 * independent from the individual core revision numbers.
		 * For example, the "A0" silicon of each chip is chip rev 0.
		 * For PCMCIA we get it from the CIS instead.
		 */
		if (BUSTYPE(si->bustype) == PCMCIA_BUS) {
			ASSERT(vars);
			si->chiprev = getintvar(*vars, "chiprev");
		} else if (BUSTYPE(si->bustype) == PCI_BUS) {
			w = OSL_PCI_READ_CONFIG(osh, PCI_CFG_REV, sizeof (uint32));
			si->chiprev = w & 0xff;
		} else
			si->chiprev = 0;
	}

	if (BUSTYPE(si->bustype) == PCMCIA_BUS) {
		w = getintvar(*vars, "regwindowsz");
		si->memseg = (w <= CFTABLE_REGWIN_2K) ? TRUE : FALSE;
	}

	/* gpio control core is required */
	if (!GOODIDX(si->gpioidx)) {
		SB_ERROR(("sb_attach: gpio control core not found\n"));
		return NULL;
	}

	/* get boardtype and boardrev */
	switch (BUSTYPE(si->bustype)) {
	case PCI_BUS:
		/* do a pci config read to get subsystem id and subvendor id */
		w = OSL_PCI_READ_CONFIG(osh, PCI_CFG_SVID, sizeof (uint32));
		si->boardvendor = w & 0xffff;
		si->boardtype = (w >> 16) & 0xffff;
		break;

	case PCMCIA_BUS:
	case SDIO_BUS:
		si->boardvendor = getintvar(*vars, "manfid");
		si->boardtype = getintvar(*vars, "prodid");
		break;

	case SB_BUS:
		si->boardvendor = VENDOR_BROADCOM;
		si->boardtype = 0xffff;
		break;
	}

	if (si->boardtype == 0) {
		SB_ERROR(("sb_attach: unknown board type\n"));
		ASSERT(si->boardtype);
	}


	return ((void*)si);
}

uint
sb_coreid(void *sbh)
{
	sb_info_t *si;
	sbconfig_t *sb;

	si = SB_INFO(sbh);
	sb = REGS2SB(si->curmap);

	return ((R_SBREG(sbh, &(sb)->sbidhigh) & SBIDH_CC_MASK) >> SBIDH_CC_SHIFT);
}

uint
sb_coreidx(void *sbh)
{
	sb_info_t *si;

	si = SB_INFO(sbh);
	return (si->curidx);
}

/* return current index of core */
static uint
_sb_coreidx(void *sbh)
{
	sb_info_t *si;
	sbconfig_t *sb;
	uint32 sbaddr = 0;

	si = SB_INFO(sbh);
	ASSERT(si);

	switch (BUSTYPE(si->bustype)) {
	case SB_BUS:
		sb = REGS2SB(si->curmap);
		sbaddr = sb_base(R_SBREG(sbh, &sb->sbadmatch0));
		break;

	case PCI_BUS:
		sbaddr = OSL_PCI_READ_CONFIG(si->osh, PCI_BAR0_WIN, sizeof (uint32));
		break;

	case PCMCIA_BUS: {
		uint8 tmp = 0;

		OSL_PCMCIA_READ_ATTR(si->osh, PCMCIA_ADDR0, &tmp, 1);
		sbaddr  = (uint)tmp << 12;
		OSL_PCMCIA_READ_ATTR(si->osh, PCMCIA_ADDR1, &tmp, 1);
		sbaddr |= (uint)tmp << 16;
		OSL_PCMCIA_READ_ATTR(si->osh, PCMCIA_ADDR2, &tmp, 1);
		sbaddr |= (uint)tmp << 24;
		break;
	}
	default:
		ASSERT(0);
	}

	if (!GOODCOREADDR(sbaddr))
		return BADIDX;

	return ((sbaddr - SB_ENUM_BASE) / SB_CORE_SIZE);
}

uint
sb_corevendor(void *sbh)
{
	sb_info_t *si;
	sbconfig_t *sb;

	si = SB_INFO(sbh);
	sb = REGS2SB(si->curmap);

	return ((R_SBREG(sbh, &(sb)->sbidhigh) & SBIDH_VC_MASK) >> SBIDH_VC_SHIFT);
}

uint
sb_corerev(void *sbh)
{
	sb_info_t *si;
	sbconfig_t *sb;

	si = SB_INFO(sbh);
	sb = REGS2SB(si->curmap);

	return (R_SBREG(sbh, &(sb)->sbidhigh) & SBIDH_RC_MASK);
}

void *
sb_osh(void *sbh)
{
	sb_info_t *si;

	si = SB_INFO(sbh);
	return si->osh;
}

#define	SBTML_ALLOW	(SBTML_PE | SBTML_FGC | SBTML_FL_MASK)

/* set/clear sbtmstatelow core-specific flags */
uint32
sb_coreflags(void *sbh, uint32 mask, uint32 val)
{
	sb_info_t *si;
	sbconfig_t *sb;
	uint32 w;

	si = SB_INFO(sbh);
	sb = REGS2SB(si->curmap);

	ASSERT((val & ~mask) == 0);
	ASSERT((mask & ~SBTML_ALLOW) == 0);

	/* mask and set */
	if (mask || val) {
		w = (R_SBREG(sbh, &sb->sbtmstatelow) & ~mask) | val;
		W_SBREG(sbh, &sb->sbtmstatelow, w);
	}

	/* return the new value */
	return (R_SBREG(sbh, &sb->sbtmstatelow) & SBTML_ALLOW);
}

/* set/clear sbtmstatehigh core-specific flags */
uint32
sb_coreflagshi(void *sbh, uint32 mask, uint32 val)
{
	sb_info_t *si;
	sbconfig_t *sb;
	uint32 w;

	si = SB_INFO(sbh);
	sb = REGS2SB(si->curmap);

	ASSERT((val & ~mask) == 0);
	ASSERT((mask & ~SBTMH_FL_MASK) == 0);

	/* mask and set */
	if (mask || val) {
		w = (R_SBREG(sbh, &sb->sbtmstatehigh) & ~mask) | val;
		W_SBREG(sbh, &sb->sbtmstatehigh, w);
	}

	/* return the new value */
	return (R_SBREG(sbh, &sb->sbtmstatehigh) & SBTMH_FL_MASK);
}

bool
sb_iscoreup(void *sbh)
{
	sb_info_t *si;
	sbconfig_t *sb;

	si = SB_INFO(sbh);
	sb = REGS2SB(si->curmap);

	return ((R_SBREG(sbh, &(sb)->sbtmstatelow) & (SBTML_RESET | SBTML_REJ | SBTML_CLK)) == SBTML_CLK);
}

/*
 * Switch to 'coreidx', issue a single arbitrary 32bit register mask&set operation,
 * switch back to the original core, and return the new value.
 */
static uint
sb_corereg(void *sbh, uint coreidx, uint regoff, uint mask, uint val)
{
	sb_info_t *si;
	uint origidx;
	uint32 *r;
	uint w;
	uint intr_val = 0;

	ASSERT(GOODIDX(coreidx));
	ASSERT(regoff < SB_CORE_SIZE);
	ASSERT((val & ~mask) == 0);

	si = SB_INFO(sbh);

	INTR_OFF(si, intr_val);

	/* save current core index */
	origidx = sb_coreidx(sbh);

	/* switch core */
	r = (uint32*) ((uchar*) sb_setcoreidx(sbh, coreidx) + regoff);

	/* mask and set */
	if (mask || val) {
		if (regoff >= SBCONFIGOFF) {
			w = (R_SBREG(sbh, r) & ~mask) | val;
			W_SBREG(sbh, r, w);
		} else {
			w = (R_REG(r) & ~mask) | val;
			W_REG(r, w);
		}
	}

	/* readback */
	if (regoff >= SBCONFIGOFF)
		w = R_SBREG(sbh,r);
	else {
		if ((si->chip == BCM5354_CHIP_ID) &&
		    (coreidx == SB_CC_IDX) &&
		    (regoff == OFFSETOF(chipcregs_t, watchdog))) {
			w = val;
		} else
			w = R_REG(r);
	}

	/* restore core index */
	if (origidx != coreidx)
		sb_setcoreidx(sbh, origidx);

	INTR_RESTORE(si, intr_val);
	return (w);
}

/* scan the sb enumerated space to identify all cores */
static void
BCMINITFN(sb_scan)(sb_info_t *si)
{
	void *sbh;
	uint origidx;
	uint i;

	sbh = (void*) si;

	/* numcores should already be set */
	ASSERT((si->numcores > 0) && (si->numcores <= SB_MAXCORES));

	/* save current core index */
	origidx = sb_coreidx(sbh);

	si->pciidx = si->pcmciaidx = si->gpioidx = BADIDX;
	si->pcirev = si->pcmciarev = NOREV;

	for (i = 0; i < si->numcores; i++) {
		sb_setcoreidx(sbh, i);
		si->coreid[i] = sb_coreid(sbh);

		if (si->coreid[i] == SB_PCI) {
			si->pciidx = i;
			si->pcirev = sb_corerev(sbh);

		} else if (si->coreid[i] == SB_PCMCIA) {
			si->pcmciaidx = i;
			si->pcmciarev = sb_corerev(sbh);
		}
	}

	/*
	 * Find the gpio "controlling core" type and index.
	 * Precedence:
	 * - if there's a chip common core - use that
	 * - else if there's a pci core (rev >= 2) - use that
	 * - else there had better be an extif core (4710 only)
	 */
	if (GOODIDX(sb_findcoreidx(sbh, SB_CC, 0))) {
		si->gpioidx = sb_findcoreidx(sbh, SB_CC, 0);
		si->gpioid = SB_CC;
	} else if (GOODIDX(si->pciidx) && (si->pcirev >= 2)) {
		si->gpioidx = si->pciidx;
		si->gpioid = SB_PCI;
	} else if (sb_findcoreidx(sbh, SB_EXTIF, 0)) {
		si->gpioidx = sb_findcoreidx(sbh, SB_EXTIF, 0);
		si->gpioid = SB_EXTIF;
	} else
		ASSERT(si->gpioidx != BADIDX);

	/* return to original core index */
	sb_setcoreidx(sbh, origidx);
}

/* may be called with core in reset */
void
sb_detach(void *sbh)
{
	sb_info_t *si;
	uint idx;

	si = SB_INFO(sbh);

	if (si == NULL)
		return;

	if (BUSTYPE(si->bustype) == SB_BUS)
		for (idx = 0; idx < SB_MAXCORES; idx++)
			if (si->regs[idx]) {
				REG_UNMAP(si->regs[idx]);
				si->regs[idx] = NULL;
			}

	MFREE(si->osh, si, sizeof (sb_info_t));
}

/* use pci dev id to determine chip id for chips not having a chipcommon core */
static uint
BCMINITFN(sb_pcidev2chip)(uint pcidev)
{
	if ((pcidev >= BCM4710_DEVICE_ID) && (pcidev <= BCM47XX_USB_ID))
		return (BCM4710_DEVICE_ID);
	if ((pcidev >= BCM4610_DEVICE_ID) && (pcidev <= BCM4610_USB_ID))
		return (BCM4610_DEVICE_ID);
	if ((pcidev >= BCM4402_DEVICE_ID) && (pcidev <= BCM4402_V90_ID))
		return (BCM4402_DEVICE_ID);
	if (pcidev == BCM4401_ENET_ID)
		return (BCM4402_DEVICE_ID);
	if ((pcidev >= BCM4307_V90_ID) && (pcidev <= BCM4307_D11B_ID))
		return (BCM4307_DEVICE_ID);
	if (pcidev == BCM4301_DEVICE_ID)
		return (BCM4301_DEVICE_ID);

	return (0);
}

/* convert chip number to number of i/o cores */
static uint
BCMINITFN(sb_chip2numcores)(uint chip)
{
	if (chip == 0x4710)
		return (9);
	if (chip == 0x4610)
		return (9);
	if (chip == 0x4402)
		return (3);
	if ((chip == 0x4307) || (chip == 0x4301))
		return (5);
	if (chip == 0x4310)
		return (8);
	if (chip == 0x4306)	/* < 4306c0 */
		return (6);
	if (chip == 0x4704)
		return (9);
	if (chip == 0x5365)
		return (7);

	SB_ERROR(("sb_chip2numcores: unsupported chip 0x%x\n", chip));
	ASSERT(0);
	return (1);
}

/* return index of coreid or BADIDX if not found */
static uint
sb_findcoreidx(void *sbh, uint coreid, uint coreunit)
{
	sb_info_t *si;
	uint found;
	uint i;

	si = SB_INFO(sbh);
	found = 0;

	for (i = 0; i < si->numcores; i++)
		if (si->coreid[i] == coreid) {
			if (found == coreunit)
				return (i);
			found++;
		}

	return (BADIDX);
}

/* 
 * this function changes logical "focus" to the indiciated core, 
 * must be called with interrupt off.
 * Moreover, callers should keep interrupts off during switching out of and back to d11 core
 */
void*
sb_setcoreidx(void *sbh, uint coreidx)
{
	sb_info_t *si;
	uint32 sbaddr;
	uint8 tmp;

	si = SB_INFO(sbh);

	if (coreidx >= si->numcores)
		return (NULL);
	
	/*
	 * If the user has provided an interrupt mask enabled function,
	 * then assert interrupts are disabled before switching the core.
	 */
	ASSERT((si->intrsenabled_fn == NULL) || !(*(si)->intrsenabled_fn)((si)->intr_arg));

	sbaddr = SB_ENUM_BASE + (coreidx * SB_CORE_SIZE);

	switch (BUSTYPE(si->bustype)) {
	case SB_BUS:
		/* map new one */
		if (!si->regs[coreidx]) {
			si->regs[coreidx] = (void*)REG_MAP(sbaddr, SB_CORE_SIZE);
			ASSERT(GOODREGS(si->regs[coreidx]));
		}
		si->curmap = si->regs[coreidx];
		break;

	case PCI_BUS:
		/* point bar0 window */
		OSL_PCI_WRITE_CONFIG(si->osh, PCI_BAR0_WIN, 4, sbaddr);
		break;

	case PCMCIA_BUS:
		tmp = (sbaddr >> 12) & 0x0f;
		OSL_PCMCIA_WRITE_ATTR(si->osh, PCMCIA_ADDR0, &tmp, 1);
		tmp = (sbaddr >> 16) & 0xff;
		OSL_PCMCIA_WRITE_ATTR(si->osh, PCMCIA_ADDR1, &tmp, 1);
		tmp = (sbaddr >> 24) & 0xff;
		OSL_PCMCIA_WRITE_ATTR(si->osh, PCMCIA_ADDR2, &tmp, 1);
		break;
	}

	si->curidx = coreidx;

	return (si->curmap);
}

/* 
 * this function changes logical "focus" to the indiciated core, 
 * must be called with interrupt off.
 * Moreover, callers should keep interrupts off during switching out of and back to d11 core
 */
void*
sb_setcore(void *sbh, uint coreid, uint coreunit)
{
	sb_info_t *si;
	uint idx;

	si = SB_INFO(sbh);

	idx = sb_findcoreidx(sbh, coreid, coreunit);
	if (!GOODIDX(idx))
		return (NULL);

	return (sb_setcoreidx(sbh, idx));
}

/* return chip number */
uint
BCMINITFN(sb_chip)(void *sbh)
{
	sb_info_t *si;

	si = SB_INFO(sbh);
	return (si->chip);
}

/* return chip revision number */
uint
BCMINITFN(sb_chiprev)(void *sbh)
{
	sb_info_t *si;

	si = SB_INFO(sbh);
	return (si->chiprev);
}

/* return chip common revision number */
uint
BCMINITFN(sb_chipcrev)(void *sbh)
{
	sb_info_t *si;

	si = SB_INFO(sbh);
	return (si->ccrev);
}

/* return chip package option */
uint
BCMINITFN(sb_chippkg)(void *sbh)
{
	sb_info_t *si;

	si = SB_INFO(sbh);
	return (si->chippkg);
}

/* return PCI core rev. */
uint
BCMINITFN(sb_pcirev)(void *sbh)
{
	sb_info_t *si;

	si = SB_INFO(sbh);
	return (si->pcirev);
}

/* return PCMCIA core rev. */
uint
BCMINITFN(sb_pcmciarev)(void *sbh)
{
	sb_info_t *si;

	si = SB_INFO(sbh);
	return (si->pcmciarev);
}

/* return board vendor id */
uint
BCMINITFN(sb_boardvendor)(void *sbh)
{
	sb_info_t *si;

	si = SB_INFO(sbh);
	return (si->boardvendor);
}

/* return boardtype */
uint
BCMINITFN(sb_boardtype)(void *sbh)
{
	sb_info_t *si;
	char *var;

	si = SB_INFO(sbh);

	if (BUSTYPE(si->bustype) == SB_BUS && si->boardtype == 0xffff) {
		/* boardtype format is a hex string */
		si->boardtype = getintvar(NULL, "boardtype");

		/* backward compatibility for older boardtype string format */
		if ((si->boardtype == 0) && (var = getvar(NULL, "boardtype"))) {
			if (!strcmp(var, "bcm94710dev"))
				si->boardtype = BCM94710D_BOARD;
			else if (!strcmp(var, "bcm94710ap"))
				si->boardtype = BCM94710AP_BOARD;
			else if (!strcmp(var, "bcm94310u"))
				si->boardtype = BCM94310U_BOARD;
			else if (!strcmp(var, "bu4711"))
				si->boardtype = BU4711_BOARD;
			else if (!strcmp(var, "bu4710"))
				si->boardtype = BU4710_BOARD;
			else if (!strcmp(var, "bcm94702mn"))
				si->boardtype = BCM94702MN_BOARD;
			else if (!strcmp(var, "bcm94710r1"))
				si->boardtype = BCM94710R1_BOARD;
			else if (!strcmp(var, "bcm94710r4"))
				si->boardtype = BCM94710R4_BOARD;
			else if (!strcmp(var, "bcm94702cpci"))
				si->boardtype = BCM94702CPCI_BOARD;
			else if (!strcmp(var, "bcm95380_rr"))
				si->boardtype = BCM95380RR_BOARD;
		}
	}

	return (si->boardtype);
}

/* return bus type of sdh device */
uint
sb_bus(void *sbh)
{
	sb_info_t *si;

	si = SB_INFO(sbh);
	return (si->bustype);
}

/* return list of found cores */
uint
sb_corelist(void *sbh, uint coreid[])
{
	sb_info_t *si;

	si = SB_INFO(sbh);

	bcopy((uchar*)si->coreid, (uchar*)coreid, (si->numcores * sizeof (uint)));
	return (si->numcores);
}

/* return current register mapping */
void *
sb_coreregs(void *sbh)
{
	sb_info_t *si;

	si = SB_INFO(sbh);
	ASSERT(GOODREGS(si->curmap));

	return (si->curmap);
}


/* do buffered registers update */
void
sb_commit(void *sbh)
{
	sb_info_t *si;
	uint origidx;
	uint intr_val = 0;

	si = SB_INFO(sbh);

	origidx = si->curidx;
	ASSERT(GOODIDX(origidx));

	INTR_OFF(si, intr_val);

	/* switch over to chipcommon core if there is one, else use pci */
	if (si->ccrev != NOREV) {
		chipcregs_t *ccregs = (chipcregs_t *)sb_setcore(sbh, SB_CC, 0);

		/* do the buffer registers update */
		W_REG(&ccregs->broadcastaddress, SB_COMMIT);
		W_REG(&ccregs->broadcastdata, 0x0);
	} else if (si->pciidx != BADIDX) {
		sbpciregs_t *pciregs = (sbpciregs_t *)sb_setcore(sbh, SB_PCI, 0);

		/* do the buffer registers update */
		W_REG(&pciregs->bcastaddr, SB_COMMIT);
		W_REG(&pciregs->bcastdata, 0x0);
	} else {
		ASSERT((si->ccrev != NOREV) && (si->pciidx != BADIDX));
	}

	/* restore core index */
	sb_setcoreidx(sbh, origidx);
	INTR_RESTORE(si, intr_val);
}

/* reset and re-enable a core */
void
sb_core_reset(void *sbh, uint32 bits)
{
	sb_info_t *si;
	sbconfig_t *sb;
	volatile uint32 dummy;

	si = SB_INFO(sbh);
	ASSERT(GOODREGS(si->curmap));
	sb = REGS2SB(si->curmap);

	/*
	 * Must do the disable sequence first to work for arbitrary current core state.
	 */
	sb_core_disable(sbh, bits);

	/*
	 * Now do the initialization sequence.
	 */

	/* set reset while enabling the clock and forcing them on throughout the core */
	W_SBREG(sbh, &sb->sbtmstatelow, (SBTML_FGC | SBTML_CLK | SBTML_RESET | bits));
	dummy = R_SBREG(sbh, &sb->sbtmstatelow);

	if (sb_coreid(sbh) == SB_ILINE100) {
		bcm_mdelay(50);
	} else {
		OSL_DELAY(1);
	}

	if (R_SBREG(sbh, &sb->sbtmstatehigh) & SBTMH_SERR) {
		W_SBREG(sbh, &sb->sbtmstatehigh, 0);
	}
	if ((dummy = R_SBREG(sbh, &sb->sbimstate)) & (SBIM_IBE | SBIM_TO)) {
		AND_SBREG(sbh, &sb->sbimstate, ~(SBIM_IBE | SBIM_TO));
	}

	/* clear reset and allow it to propagate throughout the core */
	W_SBREG(sbh, &sb->sbtmstatelow, (SBTML_FGC | SBTML_CLK | bits));
	dummy = R_SBREG(sbh, &sb->sbtmstatelow);
	OSL_DELAY(1);

	/* leave clock enabled */
	W_SBREG(sbh, &sb->sbtmstatelow, (SBTML_CLK | bits));
	dummy = R_SBREG(sbh, &sb->sbtmstatelow);
	OSL_DELAY(1);
}

void
sb_core_tofixup(void *sbh)
{
	sb_info_t *si;
	sbconfig_t *sb;

	si = SB_INFO(sbh);

	if ((si->pciidx == BADIDX) || (si->pcirev >= 5))
		return;

	ASSERT(GOODREGS(si->curmap));
	sb = REGS2SB(si->curmap);

	if (BUSTYPE(si->bustype) == SB_BUS) {
		SET_SBREG(sbh, &sb->sbimconfiglow,
			  SBIMCL_RTO_MASK | SBIMCL_STO_MASK,
			  (0x5 << SBIMCL_RTO_SHIFT) | 0x3);
	} else {
		if (sb_coreid(sbh) == SB_PCI) {
			SET_SBREG(sbh, &sb->sbimconfiglow,
				  SBIMCL_RTO_MASK | SBIMCL_STO_MASK,
				  (0x3 << SBIMCL_RTO_SHIFT) | 0x2);
		} else {
			SET_SBREG(sbh, &sb->sbimconfiglow, (SBIMCL_RTO_MASK | SBIMCL_STO_MASK), 0);
		}
	}

	sb_commit(sbh);
}

void
sb_core_disable(void *sbh, uint32 bits)
{
	sb_info_t *si;
	volatile uint32 dummy;
	sbconfig_t *sb;

	si = SB_INFO(sbh);

	ASSERT(GOODREGS(si->curmap));
	sb = REGS2SB(si->curmap);

	/* if core is already in reset, just return */
	if (R_SBREG(sbh, &sb->sbtmstatelow) & SBTML_RESET)
		return;

	/* if clocks are not enabled, put into reset and return */
	if ((R_SBREG(sbh, &sb->sbtmstatelow) & SBTML_CLK) == 0)
		goto disable;

	/* set the target reject bit and spin until busy is clear */
	W_SBREG(sbh, &sb->sbtmstatelow, (SBTML_CLK | SBTML_REJ));
	dummy = R_SBREG(sbh, &sb->sbtmstatelow);
	OSL_DELAY(1);
	SPINWAIT((R_SBREG(sbh, &sb->sbtmstatehigh) & SBTMH_BUSY), 100000);

 	if (R_SBREG(sbh, &sb->sbidlow) & SBIDL_INIT) {
		OR_SBREG(sbh, &sb->sbimstate, SBIM_RJ);
		dummy = R_SBREG(sbh, &sb->sbimstate);
		OSL_DELAY(1);
		SPINWAIT((R_SBREG(sbh, &sb->sbimstate) & SBIM_BY), 100000);
	}

	/* set reset and reject while enabling the clocks */
	W_SBREG(sbh, &sb->sbtmstatelow, (bits | SBTML_FGC | SBTML_CLK | SBTML_REJ | SBTML_RESET));
	dummy = R_SBREG(sbh, &sb->sbtmstatelow);
	OSL_DELAY(10);

	/* don't forget to clear the initiator reject bit */
	if (R_SBREG(sbh, &sb->sbidlow) & SBIDL_INIT)
		AND_SBREG(sbh, &sb->sbimstate, ~SBIM_RJ);

disable:
	/* leave reset and reject asserted */
	W_SBREG(sbh, &sb->sbtmstatelow, (bits | SBTML_REJ | SBTML_RESET));
	OSL_DELAY(1);
}

void
sb_watchdog(void *sbh, uint ticks)
{
	sb_info_t *si = SB_INFO(sbh);

	/* instant NMI */
	switch (si->gpioid) {
	case SB_CC:
		sb_corereg(sbh, si->gpioidx, OFFSETOF(chipcregs_t, watchdog), ~0, ticks);
		break;
	case SB_EXTIF:
		sb_corereg(sbh, si->gpioidx, OFFSETOF(extifregs_t, watchdog), ~0, ticks);
		break;
	}
}

/* initialize the pcmcia core */
void
sb_pcmcia_init(void *sbh)
{
	sb_info_t *si;
	uint8 cor;

	si = SB_INFO(sbh);

	/* enable d11 mac interrupts */
	if (si->chip == BCM4301_DEVICE_ID) {
		/* Have to use FCR2 in 4301 */
		OSL_PCMCIA_READ_ATTR(si->osh, PCMCIA_FCR2 + PCMCIA_COR, &cor, 1);
		cor |= COR_IRQEN | COR_FUNEN;
		OSL_PCMCIA_WRITE_ATTR(si->osh, PCMCIA_FCR2 + PCMCIA_COR, &cor, 1);
	} else {
		OSL_PCMCIA_READ_ATTR(si->osh, PCMCIA_FCR0 + PCMCIA_COR, &cor, 1);
		cor |= COR_IRQEN | COR_FUNEN;
		OSL_PCMCIA_WRITE_ATTR(si->osh, PCMCIA_FCR0 + PCMCIA_COR, &cor, 1);
	}

}


/*
 * Configure the pci core for pci client (NIC) action
 * and get appropriate dma offset value.
 * coremask is the bitvec of cores by index to be enabled.
 */
void
sb_pci_setup(void *sbh, uint32 *dmaoffset, uint coremask)
{
	sb_info_t *si;
	sbconfig_t *sb;
	sbpciregs_t *pciregs;
	uint32 sbflag;
	uint32 w;
	uint idx;

	si = SB_INFO(sbh);

	if (dmaoffset)
		*dmaoffset = 0;

	/* if not pci bus, we're done */
	if (BUSTYPE(si->bustype) != PCI_BUS)
		return;

	ASSERT(si->pciidx != BADIDX);

	/* get current core index */
	idx = si->curidx;

	/* we interrupt on this backplane flag number */
	ASSERT(GOODREGS(si->curmap));
	sb = REGS2SB(si->curmap);
	sbflag = R_SBREG(sbh, &sb->sbtpsflag) & SBTPS_NUM0_MASK;

	/* switch over to pci core */
	pciregs = (sbpciregs_t*) sb_setcoreidx(sbh, si->pciidx);
	sb = REGS2SB(pciregs);

	/*
	 * Enable sb->pci interrupts.  Assume
	 * PCI rev 2.3 support was added in pci core rev 6 and things changed..
	 */
	if (si->pcirev < 6) {
		/* set sbintvec bit for our flag number */
		OR_SBREG(sbh, &sb->sbintvec, (1 << sbflag));
	} else {
		/* pci config write to set this core bit in PCIIntMask */
		w = OSL_PCI_READ_CONFIG(si->osh, PCI_INT_MASK, sizeof(uint32));
		w |= (coremask << PCI_SBIM_SHIFT);
		OSL_PCI_WRITE_CONFIG(si->osh, PCI_INT_MASK, sizeof(uint32), w);
	}

	/* enable prefetch and bursts for dma big window */
	OR_REG(&pciregs->sbtopci2, (SBTOPCI_PREF|SBTOPCI_BURST));

	/* enable read multiple for dma big window */
	if (si->pcirev >= 11)
		OR_REG(&pciregs->sbtopci2, SBTOPCI_RC_READMULTI);

	if (si->pcirev < 5) {
		SET_SBREG(sbh, &sb->sbimconfiglow, SBIMCL_RTO_MASK | SBIMCL_STO_MASK,
			(0x3 << SBIMCL_RTO_SHIFT) | 0x2);
		sb_commit(sbh);
	}

	/* switch back to previous core */
	sb_setcoreidx(sbh, idx);

	/* use large sb pci dma window */
	if (dmaoffset)
		*dmaoffset = SB_PCI_DMA;
}

uint32
sb_base(uint32 admatch)
{
	uint32 base;
	uint type;

	type = admatch & SBAM_TYPE_MASK;
	ASSERT(type < 3);

	base = 0;

	if (type == 0) {
		base = admatch & SBAM_BASE0_MASK;
	} else if (type == 1) {
		ASSERT(!(admatch & SBAM_ADNEG));	/* neg not supported */
		base = admatch & SBAM_BASE1_MASK;
	} else if (type == 2) {
		ASSERT(!(admatch & SBAM_ADNEG));	/* neg not supported */
		base = admatch & SBAM_BASE2_MASK;
	}

	return (base);
}

uint32
sb_size(uint32 admatch)
{
	uint32 size;
	uint type;

	type = admatch & SBAM_TYPE_MASK;
	ASSERT(type < 3);

	size = 0;

	if (type == 0) {
		size = 1 << (((admatch & SBAM_ADINT0_MASK) >> SBAM_ADINT0_SHIFT) + 1);
	} else if (type == 1) {
		ASSERT(!(admatch & SBAM_ADNEG));	/* neg not supported */
		size = 1 << (((admatch & SBAM_ADINT1_MASK) >> SBAM_ADINT1_SHIFT) + 1);
	} else if (type == 2) {
		ASSERT(!(admatch & SBAM_ADNEG));	/* neg not supported */
		size = 1 << (((admatch & SBAM_ADINT2_MASK) >> SBAM_ADINT2_SHIFT) + 1);
	}

	return (size);
}

/* return the core-type instantiation # of the current core */
uint
sb_coreunit(void *sbh)
{
	sb_info_t *si;
	uint idx;
	uint coreid;
	uint coreunit;
	uint i;

	si = SB_INFO(sbh);
	coreunit = 0;

	idx = si->curidx;

	ASSERT(GOODREGS(si->curmap));
	coreid = sb_coreid(sbh);

	/* count the cores of our type */
	for (i = 0; i < idx; i++)
		if (si->coreid[i] == coreid)
			coreunit++;

	return (coreunit);
}

static INLINE uint32
factor6(uint32 x)
{
	switch (x) {
	case CC_F6_2:	return 2;
	case CC_F6_3:	return 3;
	case CC_F6_4:	return 4;
	case CC_F6_5:	return 5;
	case CC_F6_6:	return 6;
	case CC_F6_7:	return 7;
	default:	return 0;
	}
}

/* calculate the speed the SB would run at given a set of clockcontrol values */
uint32
sb_clock_rate(uint32 pll_type, uint32 n, uint32 m)
{
	uint32 n1, n2, clock, m1, m2, m3, mc;

	n1 = n & CN_N1_MASK;
	n2 = (n & CN_N2_MASK) >> CN_N2_SHIFT;

	if (pll_type == PLL_TYPE6) {
		if (m & CC_T6_MMASK)
			return CC_T6_M1;
		else
			return CC_T6_M0;
	} else if ((pll_type == PLL_TYPE1) ||
		   (pll_type == PLL_TYPE3) ||
		   (pll_type == PLL_TYPE4) ||
		   (pll_type == PLL_TYPE7)) {
		n1 = factor6(n1);
		n2 += CC_F5_BIAS;
	} else if (pll_type == PLL_TYPE2) {
		n1 += CC_T2_BIAS;
		n2 += CC_T2_BIAS;
		ASSERT((n1 >= 2) && (n1 <= 7));
		ASSERT((n2 >= 5) && (n2 <= 23));
	} else if (pll_type == PLL_TYPE5) {
		return (100000000);
	} else
		ASSERT((pll_type >= PLL_TYPE1) && (pll_type <= PLL_TYPE4));
	/* PLL types 3 and 7 use BASE2 (25Mhz) */
	if ((pll_type == PLL_TYPE3) ||
	    (pll_type == PLL_TYPE7)) { 
		clock =  CC_CLOCK_BASE2 * n1 * n2;
	}
	else 
		clock = CC_CLOCK_BASE1 * n1 * n2;

	if (clock == 0)
		return 0;

	m1 = m & CC_M1_MASK;
	m2 = (m & CC_M2_MASK) >> CC_M2_SHIFT;
	m3 = (m & CC_M3_MASK) >> CC_M3_SHIFT;
	mc = (m & CC_MC_MASK) >> CC_MC_SHIFT;

	if ((pll_type == PLL_TYPE1) ||
	    (pll_type == PLL_TYPE3) ||
	    (pll_type == PLL_TYPE4) ||
	    (pll_type == PLL_TYPE7)) {
		m1 = factor6(m1);
		if ((pll_type == PLL_TYPE1) || (pll_type == PLL_TYPE3))
			m2 += CC_F5_BIAS;
		else
			m2 = factor6(m2);
		m3 = factor6(m3);

		switch (mc) {
		case CC_MC_BYPASS:	return (clock);
		case CC_MC_M1:		return (clock / m1);
		case CC_MC_M1M2:	return (clock / (m1 * m2));
		case CC_MC_M1M2M3:	return (clock / (m1 * m2 * m3));
		case CC_MC_M1M3:	return (clock / (m1 * m3));
		default:		return (0);
		}
	} else {
		ASSERT(pll_type == PLL_TYPE2);

		m1 += CC_T2_BIAS;
		m2 += CC_T2M2_BIAS;
		m3 += CC_T2_BIAS;
		ASSERT((m1 >= 2) && (m1 <= 7));
		ASSERT((m2 >= 3) && (m2 <= 10));
		ASSERT((m3 >= 2) && (m3 <= 7));

		if ((mc & CC_T2MC_M1BYP) == 0)
			clock /= m1;
		if ((mc & CC_T2MC_M2BYP) == 0)
			clock /= m2;
		if ((mc & CC_T2MC_M3BYP) == 0)
			clock /= m3;

		return(clock);
	}
}

/* returns the current speed the SB is running at */
uint32
sb_clock(void *sbh)
{
	sb_info_t *si;
	extifregs_t *eir;
	chipcregs_t *cc;
	uint32 n, m;
	uint idx;
	uint32 pll_type, rate;
	uint intr_val = 0;
	uint32 cap;
	si = SB_INFO(sbh);
	idx = si->curidx;
	pll_type = PLL_TYPE1;

	INTR_OFF(si, intr_val);

	/* switch to extif or chipc core */
	if ((eir = (extifregs_t *) sb_setcore(sbh, SB_EXTIF, 0))) {
		n = R_REG(&eir->clockcontrol_n);
		m = R_REG(&eir->clockcontrol_sb);
	} else if ((cc = (chipcregs_t *) sb_setcore(sbh, SB_CC, 0))) {
		cap = R_REG(&cc->capabilities);

		if (cap & CAP_PMU) {

			if (sb_chip(sbh) == BCM5354_CHIP_ID) {
				/* 5354 has a constant sb clock of 120MHz */
				rate = 120000000;
				goto end;
			} else
#ifdef BCM4328
			if (sb_chip(sbh) == BCM4328_CHIP_ID) {
				rate = 80000000;
				goto end;
			}
			else
#endif	/* BCM4328 */
				ASSERT(0);
		}
		pll_type = R_REG(&cc->capabilities) & CAP_PLL_MASK;
		n = R_REG(&cc->clockcontrol_n);
		if (pll_type == PLL_TYPE6)
			m = R_REG(&cc->clockcontrol_mips);
		else if ((pll_type == PLL_TYPE3) && (BCMINIT(sb_chip)(sbh) != BCM5365_DEVICE_ID))
			m = R_REG(&cc->clockcontrol_m2);
		else
			m = R_REG(&cc->clockcontrol_sb);
	} else {
		INTR_RESTORE(si, intr_val);
		return 0;
	}

	if (BCMINIT(sb_chip)(sbh) == BCM5365_DEVICE_ID) {
		rate = 100000000;
	} else {
		/* calculate rate */
		rate = sb_clock_rate(pll_type, n, m);
		if (pll_type == PLL_TYPE3)
			rate = rate / 2;
	}

end:
	/* switch back to previous core */

	sb_setcoreidx(sbh, idx);

	INTR_RESTORE(si, intr_val);

	return rate;
}

/* change logical "focus" to the gpio core for optimized access */
void*
sb_gpiosetcore(void *sbh)
{
	sb_info_t *si;

	si = SB_INFO(sbh);

	return (sb_setcoreidx(sbh, si->gpioidx));
}

/* mask&set gpiocontrol bits */
uint32
sb_gpiocontrol(void *sbh, uint32 mask, uint32 val)
{
	sb_info_t *si;
	uint regoff;

	si = SB_INFO(sbh);
	regoff = 0;

	switch (si->gpioid) {
	case SB_CC:
		regoff = OFFSETOF(chipcregs_t, gpiocontrol);
		break;

	case SB_PCI:
		regoff = OFFSETOF(sbpciregs_t, gpiocontrol);
		break;

	case SB_EXTIF:
		return (0);
	}

	return (sb_corereg(sbh, si->gpioidx, regoff, mask, val));
}

/* mask&set gpio output enable bits */
uint32
sb_gpioouten(void *sbh, uint32 mask, uint32 val)
{
	sb_info_t *si;
	uint regoff;

	si = SB_INFO(sbh);
	regoff = 0;

	switch (si->gpioid) {
	case SB_CC:
		regoff = OFFSETOF(chipcregs_t, gpioouten);
		break;

	case SB_PCI:
		regoff = OFFSETOF(sbpciregs_t, gpioouten);
		break;

	case SB_EXTIF:
		regoff = OFFSETOF(extifregs_t, gpio[0].outen);
		break;
	}

	return (sb_corereg(sbh, si->gpioidx, regoff, mask, val));
}

/* mask&set gpio output bits */
uint32
sb_gpioout(void *sbh, uint32 mask, uint32 val)
{
	sb_info_t *si;
	uint regoff;

	si = SB_INFO(sbh);
	regoff = 0;

	switch (si->gpioid) {
	case SB_CC:
		regoff = OFFSETOF(chipcregs_t, gpioout);
		break;

	case SB_PCI:
		regoff = OFFSETOF(sbpciregs_t, gpioout);
		break;

	case SB_EXTIF:
		regoff = OFFSETOF(extifregs_t, gpio[0].out);
		break;
	}

	return (sb_corereg(sbh, si->gpioidx, regoff, mask, val));
}

/* return the current gpioin register value */
uint32
sb_gpioin(void *sbh)
{
	sb_info_t *si;
	uint regoff;

	si = SB_INFO(sbh);
	regoff = 0;

	switch (si->gpioid) {
	case SB_CC:
		regoff = OFFSETOF(chipcregs_t, gpioin);
		break;

	case SB_PCI:
		regoff = OFFSETOF(sbpciregs_t, gpioin);
		break;

	case SB_EXTIF:
		regoff = OFFSETOF(extifregs_t, gpioin);
		break;
	}

	return (sb_corereg(sbh, si->gpioidx, regoff, 0, 0));
}

/* mask&set gpio interrupt polarity bits */
uint32
sb_gpiointpolarity(void *sbh, uint32 mask, uint32 val)
{
	sb_info_t *si;
	uint regoff;

	si = SB_INFO(sbh);
	regoff = 0;

	switch (si->gpioid) {
	case SB_CC:
		regoff = OFFSETOF(chipcregs_t, gpiointpolarity);
		break;

	case SB_PCI:
		/* pci gpio implementation does not support interrupt polarity */
		ASSERT(0);
		break;

	case SB_EXTIF:
		regoff = OFFSETOF(extifregs_t, gpiointpolarity);
		break;
	}

	return (sb_corereg(sbh, si->gpioidx, regoff, mask, val));
}

/* mask&set gpio interrupt mask bits */
uint32
sb_gpiointmask(void *sbh, uint32 mask, uint32 val)
{
	sb_info_t *si;
	uint regoff;

	si = SB_INFO(sbh);
	regoff = 0;

	switch (si->gpioid) {
	case SB_CC:
		regoff = OFFSETOF(chipcregs_t, gpiointmask);
		break;

	case SB_PCI:
		/* pci gpio implementation does not support interrupt mask */
		ASSERT(0);
		break;

	case SB_EXTIF:
		regoff = OFFSETOF(extifregs_t, gpiointmask);
		break;
	}

	return (sb_corereg(sbh, si->gpioidx, regoff, mask, val));
}


/*
 * Return the slow clock source.
 * Three sources of SLOW CLOCK: LPO, Xtal, PCI
 */
static uint
sb_slowclk_src(void *sbh)
{
	sb_info_t *si;
	chipcregs_t *cc;
	uint32 v;

	si = SB_INFO(sbh);

	ASSERT(sb_coreid(sbh) == SB_CC);

	if (si->ccrev < 6) {
		switch (BUSTYPE(si->bustype)) {
			case PCMCIA_BUS: return (SCC_SS_XTAL);
			case PCI_BUS:
				v = OSL_PCI_READ_CONFIG(si->osh, PCI_GPIO_OUT, sizeof (uint32));
				if (v & PCI_CFG_GPIO_SCS)
					return (SCC_SS_PCI);
				else
					return (SCC_SS_XTAL);
			default: return (SCC_SS_XTAL);
		}
	} else if (si->ccrev < 10) {
		cc = (chipcregs_t*) sb_setcoreidx(sbh, si->curidx);
		v = R_REG(&cc->slow_clk_ctl) & SCC_SS_MASK;
		return (v);
	} else {
		return (SCC_SS_XTAL);
	}
}

/*
 * Return the slowclock min or max frequency.
 * Three sources of SLOW CLOCK:
 *	1. On Chip LPO		-	32khz or 160khz
 *	2. On Chip Xtal OSC	-	20mhz/4*(divider+1)
 *	3. External PCI clock	-	66mhz/4*(divider+1)
 */
static uint
sb_slowclk_freq(void *sbh, bool max)
{
	sb_info_t *si;
	chipcregs_t *cc;
	uint32 slowclk;
	uint div;

	si = SB_INFO(sbh);

	ASSERT(sb_coreid(sbh) == SB_CC);

	cc = (chipcregs_t*) sb_setcoreidx(sbh, si->curidx);

	/* shouldn't be here unless we've established the chip has dynamic power control */
	ASSERT(R_REG(&cc->capabilities) & CAP_PWR_CTL);

	slowclk = sb_slowclk_src(sbh);
	if (si->ccrev < 6) {
		if (slowclk == SCC_SS_PCI)
			return (max? (PCIMAXFREQ/64) : (PCIMINFREQ/64));
		else
			return (max? (XTALMAXFREQ/32) : (XTALMINFREQ/32));
	} else if (si->ccrev < 10) {
		div = 4 * (((R_REG(&cc->slow_clk_ctl) & SCC_CD_MASK) >> SCC_CD_SHF) + 1);
		if (slowclk == SCC_SS_LPO)
			return (max? LPOMAXFREQ : LPOMINFREQ);
		else if (slowclk == SCC_SS_XTAL)
			return (max? (XTALMAXFREQ/div) : (XTALMINFREQ/div));
		else if (slowclk == SCC_SS_PCI)
			return (max? (PCIMAXFREQ/div) : (PCIMINFREQ/div));
		else
			ASSERT(0);
	} else {
		/* Chipc rev 10 is InstaClock */
		div = R_REG(&cc->system_clk_ctl) >> SYCC_CD_SHF;
		div = 4 * (div + 1);
		return (max ? XTALMAXFREQ : (XTALMINFREQ/div));
	}
	return (0);
}

static void
sb_pwrctl_setdelay(void *sbh, void *chipcregs)
{
	sb_info_t *si;
	chipcregs_t * cc;
	uint slowmaxfreq, pll_delay, slowclk;
	uint pll_on_delay, fref_sel_delay;

	si = SB_INFO(sbh);
	pll_delay = PLL_DELAY;

	/* If the slow clock is not sourced by the xtal then add the xtal_on_delay
	 * since the xtal will also be powered down by dynamic power control logic.
	 */
	slowclk = sb_slowclk_src(sbh);
	if (slowclk != SCC_SS_XTAL)
		pll_delay += XTAL_ON_DELAY;

	/* Starting with 4318 it is ILP that is used for the delays */
	slowmaxfreq = sb_slowclk_freq(sbh, (si->ccrev >= 10) ? FALSE : TRUE);

	pll_on_delay = ((slowmaxfreq * pll_delay) + 999999) / 1000000;
	fref_sel_delay = ((slowmaxfreq * FREF_DELAY) + 999999) / 1000000;

	cc = (chipcregs_t *)chipcregs;
	W_REG(&cc->pll_on_delay, pll_on_delay);
	W_REG(&cc->fref_sel_delay, fref_sel_delay);
}

/* set or get slow clock divider */
int
sb_pwrctl_slowclk(void *sbh, bool set, uint *div)
{
	sb_info_t *si;
	uint origidx;
	chipcregs_t *cc;
	uint intr_val = 0;
	uint err = 0;
	
	si = SB_INFO(sbh);

	/* chipcommon cores prior to rev6 don't support slowclkcontrol */
	if (si->ccrev < 6)
		return 1;

	/* chipcommon cores rev10 are a whole new ball game */
	if (si->ccrev >= 10)
		return 1;

	if (set && ((*div % 4) || (*div < 4)))
		return 2;
	
	INTR_OFF(si, intr_val);
	origidx = si->curidx;
	cc = (chipcregs_t*) sb_setcore(sbh, SB_CC, 0);
	ASSERT(cc != NULL);
	
	if (!(R_REG(&cc->capabilities) & CAP_PWR_CTL)) {
		err = 3;
		goto done;
	}

	if (set) {
		SET_REG(&cc->slow_clk_ctl, SCC_CD_MASK, ((*div / 4 - 1) << SCC_CD_SHF));
		sb_pwrctl_setdelay(sbh, (void *)cc);
	} else
		*div = 4 * (((R_REG(&cc->slow_clk_ctl) & SCC_CD_MASK) >> SCC_CD_SHF) + 1);

done:
	sb_setcoreidx(sbh, origidx);
	INTR_RESTORE(si, intr_val);
	return err;
}

/* initialize power control delay registers */
void
sb_pwrctl_init(void *sbh)
{
	sb_info_t *si;
	uint origidx;
	chipcregs_t *cc;

	si = SB_INFO(sbh);

	origidx = si->curidx;

	if ((cc = (chipcregs_t*) sb_setcore(sbh, SB_CC, 0)) == NULL)
		return;

	if (!(R_REG(&cc->capabilities) & CAP_PWR_CTL))
		goto done;

	/* 4317pc does not work with SlowClock less than 5Mhz */
	if (BUSTYPE(si->bustype) == PCMCIA_BUS) {
		if ((si->ccrev >= 6) && (si->ccrev < 10))
			SET_REG(&cc->slow_clk_ctl, SCC_CD_MASK, (SCC_DEF_DIV << SCC_CD_SHF));
	}
	
	sb_pwrctl_setdelay(sbh, (void *)cc);

done:
	sb_setcoreidx(sbh, origidx);
}

/* return the value suitable for writing to the dot11 core FAST_PWRUP_DELAY register */
uint16
sb_pwrctl_fast_pwrup_delay(void *sbh)
{
	sb_info_t *si;
	uint origidx;
	chipcregs_t *cc;
	uint slowminfreq;
	uint16 fpdelay;
	uint intr_val = 0;

	si = SB_INFO(sbh);
	fpdelay = 0;
	origidx = si->curidx;

	if (BUSTYPE(si->bustype) == SB_BUS)
		goto done;

	INTR_OFF(si, intr_val);

	if ((cc = (chipcregs_t*) sb_setcore(sbh, SB_CC, 0)) == NULL)
		goto done;

	if (!(R_REG(&cc->capabilities) & CAP_PWR_CTL))
		goto done;

	slowminfreq = sb_slowclk_freq(sbh, FALSE);
	fpdelay = (((R_REG(&cc->pll_on_delay) + 2) * 1000000) + (slowminfreq - 1)) / slowminfreq;

done:
	sb_setcoreidx(sbh, origidx);
	INTR_RESTORE(si, intr_val);
	return (fpdelay);
}

/* turn primary xtal and/or pll off/on */
int
sb_pwrctl_xtal(void *sbh, uint what, bool on)
{
	sb_info_t *si;
	uint32 in, out, outen;

	si = SB_INFO(sbh);

	switch (BUSTYPE(si->bustype)) {


		case PCMCIA_BUS:
			return (0);


		case PCI_BUS:

			in = OSL_PCI_READ_CONFIG(si->osh, PCI_GPIO_IN, sizeof (uint32));
			out = OSL_PCI_READ_CONFIG(si->osh, PCI_GPIO_OUT, sizeof (uint32));
			outen = OSL_PCI_READ_CONFIG(si->osh, PCI_GPIO_OUTEN, sizeof (uint32));

			/*
			 * Avoid glitching the clock if GPRS is already using it.
			 * We can't actually read the state of the PLLPD so we infer it
			 * by the value of XTAL_PU which *is* readable via gpioin.
			 */
			if (on && (in & PCI_CFG_GPIO_XTAL))
				return (0);

			if (what & XTAL)
				outen |= PCI_CFG_GPIO_XTAL;
			if (what & PLL)
				outen |= PCI_CFG_GPIO_PLL;

			if (on) {
				/* turn primary xtal on */
				if (what & XTAL) {
					out |= PCI_CFG_GPIO_XTAL;
					if (what & PLL)
						out |= PCI_CFG_GPIO_PLL;
					OSL_PCI_WRITE_CONFIG(si->osh, PCI_GPIO_OUT, sizeof (uint32), out);
					OSL_PCI_WRITE_CONFIG(si->osh, PCI_GPIO_OUTEN, sizeof (uint32), outen);
					OSL_DELAY(XTAL_ON_DELAY);
				}

				/* turn pll on */
				if (what & PLL) {
					out &= ~PCI_CFG_GPIO_PLL;
					OSL_PCI_WRITE_CONFIG(si->osh, PCI_GPIO_OUT, sizeof (uint32), out);
					OSL_DELAY(2000);
				}
			} else {
				if (what & XTAL)
					out &= ~PCI_CFG_GPIO_XTAL;
				if (what & PLL)
					out |= PCI_CFG_GPIO_PLL;
				OSL_PCI_WRITE_CONFIG(si->osh, PCI_GPIO_OUT, sizeof (uint32), out);
				OSL_PCI_WRITE_CONFIG(si->osh, PCI_GPIO_OUTEN, sizeof (uint32), outen);
			}

		default:
			return (-1);
	}

	return (0);
}

/* set dynamic power control mode (forceslow, forcefast, dynamic) */
/*   returns true if ignore pll off is set and false if it is not */
bool
sb_pwrctl_clk(void *sbh, uint mode)
{
	sb_info_t *si;
	uint origidx;
	chipcregs_t *cc;
	uint32 scc;
	bool forcefastclk=FALSE;
	uint intr_val = 0;

	si = SB_INFO(sbh);

	/* chipcommon cores prior to rev6 don't support slowclkcontrol */
	if (si->ccrev < 6)
		return (FALSE);

	/* chipcommon cores rev10 are a whole new ball game */
	if (si->ccrev >= 10)
		return (FALSE);

	INTR_OFF(si, intr_val);

	origidx = si->curidx;

	cc = (chipcregs_t*) sb_setcore(sbh, SB_CC, 0);
	ASSERT(cc != NULL);

	if (!(R_REG(&cc->capabilities) & CAP_PWR_CTL))
		goto done;

	switch (mode) {
	case CLK_FAST:	/* force fast (pll) clock */
		/* don't forget to force xtal back on before we clear SCC_DYN_XTAL.. */
		sb_pwrctl_xtal(sbh, XTAL, ON);

		SET_REG(&cc->slow_clk_ctl, (SCC_XC | SCC_FS | SCC_IP), SCC_IP);
		break;

	case CLK_SLOW:	/* force slow clock */
		if ((BUSTYPE(si->bustype) == SDIO_BUS) || (BUSTYPE(si->bustype) == PCMCIA_BUS))
			return (-1);

		if (si->ccrev >= 6)
			OR_REG(&cc->slow_clk_ctl, SCC_FS);
		break;

	case CLK_DYNAMIC:	/* enable dynamic power control */
		scc = R_REG(&cc->slow_clk_ctl);
		scc &= ~(SCC_FS | SCC_IP | SCC_XC);
		if ((scc & SCC_SS_MASK) != SCC_SS_XTAL)
			scc |= SCC_XC;
		W_REG(&cc->slow_clk_ctl, scc);

		/* for dynamic control, we have to release our xtal_pu "force on" */
		if (scc & SCC_XC)
			sb_pwrctl_xtal(sbh, XTAL, OFF);
		break;
	}

	/* Is the h/w forcing the use of the fast clk */
	forcefastclk = (bool)((R_REG(&cc->slow_clk_ctl) & SCC_IP) == SCC_IP);

done:
	sb_setcoreidx(sbh, origidx);
	INTR_RESTORE(si, intr_val);
	return (forcefastclk);
}

/* register driver interrupt disabling and restoring callback functions */
void
sb_register_intr_callback(void *sbh, void *intrsoff_fn, void *intrsrestore_fn, void *intrsenabled_fn, void *intr_arg)
{
	sb_info_t *si;

	si = SB_INFO(sbh);
	si->intr_arg = intr_arg;
	si->intrsoff_fn = (sb_intrsoff_t)intrsoff_fn;
	si->intrsrestore_fn = (sb_intrsrestore_t)intrsrestore_fn;
	si->intrsenabled_fn = (sb_intrsenabled_t)intrsenabled_fn;
	/* save current core id.  when this function called, the current core
	 * must be the core which provides driver functions(il, et, wl, etc.)
	 */
	si->dev_coreid = si->coreid[si->curidx];
}


#ifdef __KERNEL__
void *poidGetbcm947xx_sbh(void)
{  extern void *bcm947xx_sbh;
 
   return(bcm947xx_sbh);
}
#endif


/*
 * Code to operate on PCI/E core, in NIC mode
 * Implements pci_api.h
 * Copyright (C) 2009, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: nicpci.c,v 1.29.8.2 2010/03/16 18:34:52 Exp $
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <hndsoc.h>
#include <bcmdevs.h>
#include <sbchipc.h>
#include <pci_core.h>
#include <pcie_core.h>
#include <nicpci.h>
#include <pcicfg.h>

typedef struct {
	union {
		sbpcieregs_t *pcieregs;
		sbpciregs_t *pciregs;
	} regs;				/* Memory mapped register to the core */

	si_t *sih;			/* System interconnect handle */
	osl_t *osh;			/* OSL handle */
	uint8	pciecap_lcreg_offset;	/* PCIE capability LCreg offset in the config space */
	uint8	pciecap_devctrl_offset;	/* PCIE DevControl reg offset in the config space */
	bool	pcie_pr42767;
	uint8	pcie_polarity;
	uint8	pcie_war_aspm_ovr;		/* Override ASPM/Clkreq settings */

	uint8 pmecap_offset;	/* PM Capability offset in the config space */
	bool pmecap;		/* Capable of generating PME */
} pcicore_info_t;

/* debug/trace */
#define	PCI_ERROR(args)

/* routines to access mdio slave device registers */
static bool pcie_mdiosetblock(pcicore_info_t *pi,  uint blk);
static int pcie_mdioop(pcicore_info_t *pi, uint physmedia, uint regaddr, bool write, uint *val);
static int pcie_mdiowrite(pcicore_info_t *pi, uint physmedia, uint readdr, uint val);
static int pcie_mdioread(pcicore_info_t *pi, uint physmedia, uint readdr, uint *ret_val);

static void pcie_extendL1timer(pcicore_info_t *pi, bool extend);
static void pcie_clkreq_upd(pcicore_info_t *pi, uint state);

static void pcie_war_aspm_clkreq(pcicore_info_t *pi);
static void pcie_war_serdes(pcicore_info_t *pi);
static void pcie_war_noplldown(pcicore_info_t *pi);
static void pcie_war_polarity(pcicore_info_t *pi);
static void pcie_war_pci_setup(pcicore_info_t *pi);

static bool pcicore_pmecap(pcicore_info_t *pi);
static void pcicore_fixlatencytimer(pcicore_info_t* pch, uint8 timer_val);

#define PCIE(sih) ((BUSTYPE((sih)->bustype) == PCI_BUS) && ((sih)->buscoretype == PCIE_CORE_ID))
#define PCIE_ASPM(sih)	((PCIE(sih)) && (((sih)->buscorerev >= 3) && ((sih)->buscorerev <= 5)))

#define DWORD_ALIGN(x)  (x & ~(0x03))
#define BYTE_POS(x) (x & 0x3)
#define WORD_POS(x) (x & 0x1)

#define BYTE_SHIFT(x)  (8 * BYTE_POS(x))
#define WORD_SHIFT(x)  (16 * WORD_POS(x))

#define BYTE_VAL(a, x) ((a >> BYTE_SHIFT(x)) & 0xFF)
#define WORD_VAL(a, x) ((a >> WORD_SHIFT(x)) & 0xFFFF)

#define read_pci_cfg_byte(a) \
	(BYTE_VAL(OSL_PCI_READ_CONFIG(osh, DWORD_ALIGN(a), 4), a) & 0xff)

#define read_pci_cfg_word(a) \
	(WORD_VAL(OSL_PCI_READ_CONFIG(osh, DWORD_ALIGN(a), 4), a) & 0xffff)

#define write_pci_cfg_byte(a, val) do { \
	uint32 tmpval  = (OSL_PCI_READ_CONFIG(osh, DWORD_ALIGN(a), 4) & ~0xFF << BYTE_POS(a)) | \
	        val << BYTE_POS(a); \
	OSL_PCI_WRITE_CONFIG(osh, DWORD_ALIGN(a), 4, tmpval); \
	} while (0)

#define write_pci_cfg_word(a, val) do { \
	uint32 tmpval = (OSL_PCI_READ_CONFIG(osh, DWORD_ALIGN(a), 4) & ~0xFFFF << WORD_POS(a)) | \
	        val << WORD_POS(a); \
	OSL_PCI_WRITE_CONFIG(osh, DWORD_ALIGN(a), 4, tmpval); \
	} while (0)

/* delay needed between the mdio control/ mdiodata register data access */
#define PR28829_DELAY() OSL_DELAY(10)

/* Initialize the PCI core. It's caller's responsibility to make sure that this is done
 * only once
 */
void *
pcicore_init(si_t *sih, osl_t *osh, void *regs)
{
	pcicore_info_t *pi;

	ASSERT(sih->bustype == PCI_BUS);

	/* alloc pcicore_info_t */
	if ((pi = MALLOC(osh, sizeof(pcicore_info_t))) == NULL) {
		PCI_ERROR(("pci_attach: malloc failed! malloced %d bytes\n", MALLOCED(osh)));
		return (NULL);
	}

	bzero(pi, sizeof(pcicore_info_t));

	pi->sih = sih;
	pi->osh = osh;

	if (sih->buscoretype == PCIE_CORE_ID) {
		uint8 cap_ptr;
		pi->regs.pcieregs = (sbpcieregs_t*)regs;
		cap_ptr = pcicore_find_pci_capability(pi->osh, PCI_CAP_PCIECAP_ID, NULL, NULL);
		ASSERT(cap_ptr);
		pi->pciecap_lcreg_offset = cap_ptr + PCIE_CAP_LINKCTRL_OFFSET;
		pi->pciecap_devctrl_offset = cap_ptr + PCIE_CAP_DEVCTRL_OFFSET;
	} else
		pi->regs.pciregs = (sbpciregs_t*)regs;

	return pi;
}

void
pcicore_deinit(void *pch)
{
	pcicore_info_t *pi = (pcicore_info_t *)pch;

	if (pi == NULL)
		return;
	MFREE(pi->osh, pi, sizeof(pcicore_info_t));
}

/* return cap_offset if requested capability exists in the PCI config space */
/* Note that it's caller's responsibility to make sure it's a pci bus */
uint8
pcicore_find_pci_capability(osl_t *osh, uint8 req_cap_id, uchar *buf, uint32 *buflen)
{
	uint8 cap_id;
	uint8 cap_ptr = 0;
	uint32 bufsize;
	uint8 byte_val;

	/* check for Header type 0 */
	byte_val = read_pci_cfg_byte(PCI_CFG_HDR);
	if ((byte_val & 0x7f) != PCI_HEADER_NORMAL)
		goto end;

	/* check if the capability pointer field exists */
	byte_val = read_pci_cfg_byte(PCI_CFG_STAT);
	if (!(byte_val & PCI_CAPPTR_PRESENT))
		goto end;

	cap_ptr = read_pci_cfg_byte(PCI_CFG_CAPPTR);
	/* check if the capability pointer is 0x00 */
	if (cap_ptr == 0x00)
		goto end;

	/* loop thr'u the capability list and see if the pcie capabilty exists */

	cap_id = read_pci_cfg_byte(cap_ptr);

	while (cap_id != req_cap_id) {
		cap_ptr = read_pci_cfg_byte((cap_ptr+1));
		if (cap_ptr == 0x00) break;
		cap_id = read_pci_cfg_byte(cap_ptr);
	}
	if (cap_id != req_cap_id) {
		goto end;
	}
	/* found the caller requested capability */
	if ((buf != NULL) && (buflen != NULL)) {
		uint8 cap_data;

		bufsize = *buflen;
		if (!bufsize) goto end;
		*buflen = 0;
		/* copy the cpability data excluding cap ID and next ptr */
		cap_data = cap_ptr + 2;
		if ((bufsize + cap_data)  > SZPCR)
			bufsize = SZPCR - cap_data;
		*buflen = bufsize;
		while (bufsize--) {
			*buf = read_pci_cfg_byte(cap_data);
			cap_data++;
			buf++;
		}
	}
end:
	return cap_ptr;
}

/* ***** Register Access API */
uint
pcie_readreg(osl_t *osh, sbpcieregs_t *pcieregs, uint addrtype, uint offset)
{
	uint retval = 0xFFFFFFFF;

	ASSERT(pcieregs != NULL);

	switch (addrtype) {
		case PCIE_CONFIGREGS:
			W_REG(osh, (&pcieregs->configaddr), offset);
			(void)R_REG(osh, (&pcieregs->configaddr));
			retval = R_REG(osh, &(pcieregs->configdata));
			break;
		case PCIE_PCIEREGS:
			W_REG(osh, &(pcieregs->pcieindaddr), offset);
			(void)R_REG(osh, (&pcieregs->pcieindaddr));
			retval = R_REG(osh, &(pcieregs->pcieinddata));
			break;
		default:
			ASSERT(0);
			break;
	}

	return retval;
}

uint
pcie_writereg(osl_t *osh, sbpcieregs_t *pcieregs, uint addrtype, uint offset, uint val)
{
	ASSERT(pcieregs != NULL);

	switch (addrtype) {
		case PCIE_CONFIGREGS:
			W_REG(osh, (&pcieregs->configaddr), offset);
			W_REG(osh, (&pcieregs->configdata), val);
			break;
		case PCIE_PCIEREGS:
			W_REG(osh, (&pcieregs->pcieindaddr), offset);
			W_REG(osh, (&pcieregs->pcieinddata), val);
			break;
		default:
			ASSERT(0);
			break;
	}
	return 0;
}

static bool
pcie_mdiosetblock(pcicore_info_t *pi, uint blk)
{
	sbpcieregs_t *pcieregs = pi->regs.pcieregs;
	uint mdiodata, i = 0;
	uint pcie_serdes_spinwait = 200;

	mdiodata = MDIODATA_START | MDIODATA_WRITE | (MDIODATA_DEV_ADDR << MDIODATA_DEVADDR_SHF) |
	        (MDIODATA_BLK_ADDR << MDIODATA_REGADDR_SHF) | MDIODATA_TA | (blk << 4);
	W_REG(pi->osh, &pcieregs->mdiodata, mdiodata);

	PR28829_DELAY();
	/* retry till the transaction is complete */
	while (i < pcie_serdes_spinwait) {
		if (R_REG(pi->osh, &(pcieregs->mdiocontrol)) & MDIOCTL_ACCESS_DONE) {
			break;
		}
		OSL_DELAY(1000);
		i++;
	}

	if (i >= pcie_serdes_spinwait) {
		PCI_ERROR(("pcie_mdiosetblock: timed out\n"));
		return FALSE;
	}

	return TRUE;
}

static int
pcie_mdioop(pcicore_info_t *pi, uint physmedia, uint regaddr, bool write, uint *val)
{
	sbpcieregs_t *pcieregs = pi->regs.pcieregs;
	uint mdiodata;
	uint i = 0;
	uint pcie_serdes_spinwait = 10;

	/* enable mdio access to SERDES */
	W_REG(pi->osh, (&pcieregs->mdiocontrol), MDIOCTL_PREAM_EN | MDIOCTL_DIVISOR_VAL);

	if (pi->sih->buscorerev >= 10) {
		/* new serdes is slower in rw, using two layers of reg address mapping */
		if (!pcie_mdiosetblock(pi, physmedia))
			return 1;
		mdiodata = (MDIODATA_DEV_ADDR << MDIODATA_DEVADDR_SHF) |
			(regaddr << MDIODATA_REGADDR_SHF);
		pcie_serdes_spinwait *= 20;
	} else {
		mdiodata = (physmedia << MDIODATA_DEVADDR_SHF_OLD) |
			(regaddr << MDIODATA_REGADDR_SHF_OLD);
	}

	if (!write)
		mdiodata |= (MDIODATA_START | MDIODATA_READ | MDIODATA_TA);
	else
		mdiodata |= (MDIODATA_START | MDIODATA_WRITE | MDIODATA_TA | *val);

	W_REG(pi->osh, &pcieregs->mdiodata, mdiodata);

	PR28829_DELAY();

	/* retry till the transaction is complete */
	while (i < pcie_serdes_spinwait) {
		if (R_REG(pi->osh, &(pcieregs->mdiocontrol)) & MDIOCTL_ACCESS_DONE) {
			if (!write) {
				PR28829_DELAY();
				*val = (R_REG(pi->osh, &(pcieregs->mdiodata)) & MDIODATA_MASK);
			}
			/* Disable mdio access to SERDES */
			W_REG(pi->osh, (&pcieregs->mdiocontrol), 0);
			return 0;
		}
		OSL_DELAY(1000);
		i++;
	}

	PCI_ERROR(("pcie_mdioop: timed out op: %d\n", write));
	/* Disable mdio access to SERDES */
	W_REG(pi->osh, (&pcieregs->mdiocontrol), 0);
	return 1;
}

/* use the mdio interface to read from mdio slaves */
static int
pcie_mdioread(pcicore_info_t *pi, uint physmedia, uint regaddr, uint *regval)
{
	return pcie_mdioop(pi, physmedia, regaddr, FALSE, regval);
}

/* use the mdio interface to write to mdio slaves */
static int
pcie_mdiowrite(pcicore_info_t *pi, uint physmedia, uint regaddr, uint val)
{
	return pcie_mdioop(pi, physmedia, regaddr, TRUE, &val);
}

/* ***** Support functions ***** */
static uint32
pcie_devcontrol_mrrs(void *pch, uint32 mask, uint32 val)
{
	pcicore_info_t *pi = (pcicore_info_t *)pch;
	uint32 reg_val;
	uint8 offset;

	offset = pi->pciecap_devctrl_offset;
	if (!offset)
		return 0;

	reg_val = OSL_PCI_READ_CONFIG(pi->osh, offset, sizeof(uint32));
	/* set operation */
	if (mask) {
		if (val > PCIE_CAP_DEVCTRL_MRRS_128B) {
			if (pi->sih->buscorerev < 18) {
				PCI_ERROR(("%s pcie corerev %d doesn't support >128B MRRS",
					__FUNCTION__, pi->sih->buscorerev));
				val = PCIE_CAP_DEVCTRL_MRRS_128B;
			}
		}

		reg_val &= ~PCIE_CAP_DEVCTRL_MRRS_MASK;
		reg_val |= (val << PCIE_CAP_DEVCTRL_MRRS_SHIFT) & PCIE_CAP_DEVCTRL_MRRS_MASK;

		OSL_PCI_WRITE_CONFIG(pi->osh, offset, sizeof(uint32), reg_val);
		reg_val = OSL_PCI_READ_CONFIG(pi->osh, offset, sizeof(uint32));
	}
	return reg_val;
}

uint8
pcie_clkreq(void *pch, uint32 mask, uint32 val)
{
	pcicore_info_t *pi = (pcicore_info_t *)pch;
	uint32 reg_val;
	uint8 offset;

	offset = pi->pciecap_lcreg_offset;
	if (!offset)
		return 0;

	reg_val = OSL_PCI_READ_CONFIG(pi->osh, offset, sizeof(uint32));
	/* set operation */
	if (mask) {
		if (val)
			reg_val |= PCIE_CLKREQ_ENAB;
		else
			reg_val &= ~PCIE_CLKREQ_ENAB;
		OSL_PCI_WRITE_CONFIG(pi->osh, offset, sizeof(uint32), reg_val);
		reg_val = OSL_PCI_READ_CONFIG(pi->osh, offset, sizeof(uint32));
	}
	if (reg_val & PCIE_CLKREQ_ENAB)
		return 1;
	else
		return 0;
}

static void
pcie_extendL1timer(pcicore_info_t *pi, bool extend)
{
	uint32 w;
	si_t *sih = pi->sih;
	osl_t *osh = pi->osh;
	sbpcieregs_t *pcieregs = pi->regs.pcieregs;

	if (!PCIE(sih) ||
	    !(sih->buscorerev == 7 || sih->buscorerev == 8))
		return;

	w = pcie_readreg(osh, pcieregs, PCIE_PCIEREGS, PCIE_DLLP_PMTHRESHREG);
	if (extend)
		w |= PCIE_ASPMTIMER_EXTEND;
	else
		w &= ~PCIE_ASPMTIMER_EXTEND;
	pcie_writereg(osh, pcieregs, PCIE_PCIEREGS, PCIE_DLLP_PMTHRESHREG, w);
	w = pcie_readreg(osh, pcieregs, PCIE_PCIEREGS, PCIE_DLLP_PMTHRESHREG);
}

/* centralized clkreq control policy */
static void
pcie_clkreq_upd(pcicore_info_t *pi, uint state)
{
	si_t *sih = pi->sih;
	ASSERT(PCIE(sih));

	switch (state) {
	case SI_DOATTACH:
		if (PCIE_ASPM(sih))
			pcie_clkreq((void *)pi, 1, 0);
		break;
	case SI_PCIDOWN:
		if (sih->buscorerev == 6) {	/* turn on serdes PLL down */
			si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, chipcontrol_addr),
			           ~0, 0);
			si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, chipcontrol_data),
			           ~0x40, 0);
		} else if (pi->pcie_pr42767) {
			pcie_clkreq((void *)pi, 1, 1);
		}
		break;
	case SI_PCIUP:
		if (sih->buscorerev == 6) {	/* turn off serdes PLL down */
			si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, chipcontrol_addr),
			           ~0, 0);
			si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, chipcontrol_data),
			           ~0x40, 0x40);
		} else if (PCIE_ASPM(sih)) {		/* disable clkreq */
			pcie_clkreq((void *)pi, 1, 0);
		}
		break;
	default:
		ASSERT(0);
		break;
	}
}

/* ***** PCI core WARs ***** */
/* Done only once at attach time */
static void
pcie_war_polarity(pcicore_info_t *pi)
{
	uint32 w;

	if (pi->pcie_polarity != 0)
		return;

	w = pcie_readreg(pi->osh, pi->regs.pcieregs, PCIE_PCIEREGS, PCIE_PLP_STATUSREG);

	/* Detect the current polarity at attach and force that polarity and
	 * disable changing the polarity
	 */
	if ((w & PCIE_PLP_POLARITYINV_STAT) == 0)
		pi->pcie_polarity = (SERDES_RX_CTRL_FORCE);
	else
		pi->pcie_polarity = (SERDES_RX_CTRL_FORCE | SERDES_RX_CTRL_POLARITY);
}

/* enable ASPM and CLKREQ if srom doesn't have it */
/* Needs to happen when update to shadow SROM is needed
 *   : Coming out of 'standby'/'hibernate'
 *   : If pcie_war_aspm_ovr state changed
 */
static void
pcie_war_aspm_clkreq(pcicore_info_t *pi)
{
	sbpcieregs_t *pcieregs = pi->regs.pcieregs;
	si_t *sih = pi->sih;
	uint16 val16, *reg16;
	uint32 w;

	if (!PCIE_ASPM(sih))
		return;

	/* bypass this on QT or VSIM */
	if (!ISSIM_ENAB(sih)) {

		reg16 = &pcieregs->sprom[SRSH_ASPM_OFFSET];
		val16 = R_REG(pi->osh, reg16);

		val16 &= ~SRSH_ASPM_ENB;
		if (pi->pcie_war_aspm_ovr == PCIE_ASPM_ENAB)
			val16 |= SRSH_ASPM_ENB;
		else if (pi->pcie_war_aspm_ovr == PCIE_ASPM_L1_ENAB)
			val16 |= SRSH_ASPM_L1_ENB;
		else if (pi->pcie_war_aspm_ovr == PCIE_ASPM_L0s_ENAB)
			val16 |= SRSH_ASPM_L0s_ENB;

		W_REG(pi->osh, reg16, val16);

		w = OSL_PCI_READ_CONFIG(pi->osh, pi->pciecap_lcreg_offset, sizeof(uint32));
		w &= ~PCIE_ASPM_ENAB;
		w |= pi->pcie_war_aspm_ovr;
		OSL_PCI_WRITE_CONFIG(pi->osh, pi->pciecap_lcreg_offset, sizeof(uint32), w);
	}

	reg16 = &pcieregs->sprom[SRSH_CLKREQ_OFFSET_REV5];
	val16 = R_REG(pi->osh, reg16);

	if (pi->pcie_war_aspm_ovr != PCIE_ASPM_DISAB) {
		val16 |= SRSH_CLKREQ_ENB;
		pi->pcie_pr42767 = TRUE;
	} else
		val16 &= ~SRSH_CLKREQ_ENB;

	W_REG(pi->osh, reg16, val16);
}

void
pcie_war_ovr_aspm_disable(void *pch)
{
	pcicore_info_t *pi = (pcicore_info_t *)pch;

	pi->pcie_war_aspm_ovr = FALSE;

	/* Update the current state */
	pcie_war_aspm_clkreq(pi);
}

/* Apply the polarity determined at the start */
/* Needs to happen when coming out of 'standby'/'hibernate' */
static void
pcie_war_serdes(pcicore_info_t *pi)
{
	uint32 w = 0;

	if (pi->pcie_polarity != 0)
		pcie_mdiowrite(pi, MDIODATA_DEV_RX, SERDES_RX_CTRL, pi->pcie_polarity);

	pcie_mdioread(pi, MDIODATA_DEV_PLL, SERDES_PLL_CTRL, &w);
	if (w & PLL_CTRL_FREQDET_EN) {
		w &= ~PLL_CTRL_FREQDET_EN;
		pcie_mdiowrite(pi, MDIODATA_DEV_PLL, SERDES_PLL_CTRL, w);
	}
}

/* Fix MISC config to allow coming out of L2/L3-Ready state w/o PRST */
/* Needs to happen when coming out of 'standby'/'hibernate' */
static void
BCMINITFN(pcie_misc_config_fixup)(pcicore_info_t *pi)
{
	sbpcieregs_t *pcieregs = pi->regs.pcieregs;
	uint16 val16, *reg16;

	reg16 = &pcieregs->sprom[SRSH_PCIE_MISC_CONFIG];
	val16 = R_REG(pi->osh, reg16);

	if ((val16 & SRSH_L23READY_EXIT_NOPERST) == 0) {
		val16 |= SRSH_L23READY_EXIT_NOPERST;
		W_REG(pi->osh, reg16, val16);
	}
}

/* quick hack for testing */
/* Needs to happen when coming out of 'standby'/'hibernate' */
static void
pcie_war_noplldown(pcicore_info_t *pi)
{
	sbpcieregs_t *pcieregs = pi->regs.pcieregs;
	uint16 *reg16;

	ASSERT(pi->sih->buscorerev == 7);

	/* turn off serdes PLL down */
	si_corereg(pi->sih, SI_CC_IDX, OFFSETOF(chipcregs_t, chipcontrol),
	           CHIPCTRL_4321_PLL_DOWN, CHIPCTRL_4321_PLL_DOWN);

	/*  clear srom shadow backdoor */
	reg16 = &pcieregs->sprom[SRSH_BD_OFFSET];
	W_REG(pi->osh, reg16, 0);
}

/* Needs to happen when coming out of 'standby'/'hibernate' */
static void
pcie_war_pci_setup(pcicore_info_t *pi)
{
	si_t *sih = pi->sih;
	osl_t *osh = pi->osh;
	sbpcieregs_t *pcieregs = pi->regs.pcieregs;
	uint32 w;

	if ((sih->buscorerev == 0) || (sih->buscorerev == 1)) {
		w = pcie_readreg(osh, pcieregs, PCIE_PCIEREGS, PCIE_TLP_WORKAROUNDSREG);
		w |= 0x8;
		pcie_writereg(osh, pcieregs, PCIE_PCIEREGS, PCIE_TLP_WORKAROUNDSREG, w);
	}

	if (sih->buscorerev == 1) {
		w = pcie_readreg(osh, pcieregs, PCIE_PCIEREGS, PCIE_DLLP_LCREG);
		w |= (0x40);
		pcie_writereg(osh, pcieregs, PCIE_PCIEREGS, PCIE_DLLP_LCREG, w);
	}

	if (sih->buscorerev == 0) {
		pcie_mdiowrite(pi, MDIODATA_DEV_RX, SERDES_RX_TIMER1, 0x8128);
		pcie_mdiowrite(pi, MDIODATA_DEV_RX, SERDES_RX_CDR, 0x0100);
		pcie_mdiowrite(pi, MDIODATA_DEV_RX, SERDES_RX_CDRBW, 0x1466);
	} else if (PCIE_ASPM(sih)) {
		/* Change the L1 threshold for better performance */
		w = pcie_readreg(osh, pcieregs, PCIE_PCIEREGS, PCIE_DLLP_PMTHRESHREG);
		w &= ~(PCIE_L1THRESHOLDTIME_MASK);
		w |= (PCIE_L1THRESHOLD_WARVAL << PCIE_L1THRESHOLDTIME_SHIFT);
		pcie_writereg(osh, pcieregs, PCIE_PCIEREGS, PCIE_DLLP_PMTHRESHREG, w);

		pcie_war_serdes(pi);

		pcie_war_aspm_clkreq(pi);
	} else if (pi->sih->buscorerev == 7)
		pcie_war_noplldown(pi);

	/* Note that the fix is actually in the SROM, that's why this is open-ended */
	if (pi->sih->buscorerev >= 6)
		pcie_misc_config_fixup(pi);
}

void
pcie_war_ovr_aspm_update(void *pch, uint8 aspm)
{
	pcicore_info_t *pi = (pcicore_info_t *)pch;

	if (!PCIE_ASPM(pi->sih))
		return;

	/* Validate */
	if (aspm > PCIE_ASPM_ENAB)
		return;

	pi->pcie_war_aspm_ovr = aspm;

	/* Update the current state */
	pcie_war_aspm_clkreq(pi);
}

/* ***** Functions called during driver state changes ***** */
void
BCMATTACHFN(pcicore_attach)(void *pch, char *pvars, int state)
{
	pcicore_info_t *pi = (pcicore_info_t *)pch;
	si_t *sih = pi->sih;

	if (PCIE_ASPM(sih)) {
		if (((sih->boardvendor == VENDOR_APPLE) &&
		     ((uint8)getintvar(pvars, "sromrev") == 4) &&
		     ((uint8)getintvar(pvars, "boardrev") <= 0x71)) ||
		    ((uint32)getintvar(pvars, "boardflags2") & BFL2_PCIEWAR_OVR)) {
			pi->pcie_war_aspm_ovr = PCIE_ASPM_DISAB;
		} else {
			pi->pcie_war_aspm_ovr = PCIE_ASPM_ENAB;
		}
	}

	/* These need to happen in this order only */
	pcie_war_polarity(pi);

	pcie_war_serdes(pi);

	pcie_war_aspm_clkreq(pi);

	pcie_clkreq_upd(pi, state);

	/* Default setting for increasing the TX drive strength */
	if ((sih->boardvendor == VENDOR_APPLE) &&
	    (sih->boardtype == 0x8d))
		pcicore_pcieserdesreg(pch, MDIO_DEV_TXCTRL0, 0x18, 0xff, 0x7f);

}

void
pcicore_hwup(void *pch)
{
	pcicore_info_t *pi = (pcicore_info_t *)pch;

	if (!pi || !PCIE(pi->sih))
		return;

	if (pi->sih->boardtype == CB2_4321_BOARD || pi->sih->boardtype == CB2_4321_AG_BOARD)
		pcicore_fixlatencytimer(pch, 0x20);

	pcie_war_pci_setup(pi);

	/* Default setting for increasing the TX drive strength */
	if ((pi->sih->boardvendor == VENDOR_APPLE) &&
	    (pi->sih->boardtype == 0x8d))
		pcicore_pcieserdesreg(pch, MDIO_DEV_TXCTRL0, 0x18, 0xff, 0x7f);
}

void
pcicore_up(void *pch, int state)
{
	pcicore_info_t *pi = (pcicore_info_t *)pch;

	if (!pi || !PCIE(pi->sih))
		return;

	/* Restore L1 timer for better performance */
	pcie_extendL1timer(pi, TRUE);

	pcie_clkreq_upd(pi, state);

	if (pi->sih->buscorerev == 18)
		pcie_devcontrol_mrrs(pi, PCIE_CAP_DEVCTRL_MRRS_MASK, PCIE_CAP_DEVCTRL_MRRS_128B);
}

/* When the device is going to enter D3 state (or the system is going to enter S3/S4 states */
void
pcicore_sleep(void *pch)
{
	pcicore_info_t *pi = (pcicore_info_t *)pch;
	uint32 w;

	if (!pi || !PCIE_ASPM(pi->sih))
		return;

	w = OSL_PCI_READ_CONFIG(pi->osh, pi->pciecap_lcreg_offset, sizeof(uint32));
	w &= ~PCIE_CAP_LCREG_ASPML1;
	OSL_PCI_WRITE_CONFIG(pi->osh, pi->pciecap_lcreg_offset, sizeof(uint32), w);

	pi->pcie_pr42767 = FALSE;
}

void
pcicore_down(void *pch, int state)
{
	pcicore_info_t *pi = (pcicore_info_t *)pch;

	if (!pi || !PCIE(pi->sih))
		return;

	pcie_clkreq_upd(pi, state);

	/* Reduce L1 timer for better power savings */
	pcie_extendL1timer(pi, FALSE);
}

/* ***** Wake-on-wireless-LAN (WOWL) support functions ***** */
/* Just uses PCI config accesses to find out, when needed before sb_attach is done */
bool
pcicore_pmecap_fast(osl_t *osh)
{
	uint8 cap_ptr;
	uint32 pmecap;

	cap_ptr = pcicore_find_pci_capability(osh, PCI_CAP_POWERMGMTCAP_ID, NULL, NULL);

	if (!cap_ptr)
		return FALSE;

	pmecap = OSL_PCI_READ_CONFIG(osh, cap_ptr, sizeof(uint32));

	return ((pmecap & PME_CAP_PM_STATES) != 0);
}

/* return TRUE if PM capability exists in the pci config space
 * Uses and caches the information using core handle
 */
static bool
pcicore_pmecap(pcicore_info_t *pi)
{
	uint8 cap_ptr;
	uint32 pmecap;

	if (!pi->pmecap_offset) {
		cap_ptr = pcicore_find_pci_capability(pi->osh, PCI_CAP_POWERMGMTCAP_ID, NULL, NULL);
		if (!cap_ptr)
			return FALSE;

		pi->pmecap_offset = cap_ptr;

		pmecap = OSL_PCI_READ_CONFIG(pi->osh, pi->pmecap_offset, sizeof(uint32));

		/* At least one state can generate PME */
		pi->pmecap = (pmecap & PME_CAP_PM_STATES) != 0;
	}

	return (pi->pmecap);
}

/* Enable PME generation */
void
pcicore_pmeen(void *pch)
{
	pcicore_info_t *pi = (pcicore_info_t *)pch;
	uint32 w;

	/* if not pmecapable return */
	if (!pcicore_pmecap(pi))
		return;

	w = OSL_PCI_READ_CONFIG(pi->osh, pi->pmecap_offset + PME_CSR_OFFSET, sizeof(uint32));
	w |= (PME_CSR_PME_EN);
	OSL_PCI_WRITE_CONFIG(pi->osh, pi->pmecap_offset + PME_CSR_OFFSET, sizeof(uint32), w);
}

/*
 * Return TRUE if PME status set
 */
bool
pcicore_pmestat(void *pch)
{
	pcicore_info_t *pi = (pcicore_info_t *)pch;
	uint32 w;

	if (!pcicore_pmecap(pi))
		return FALSE;

	w = OSL_PCI_READ_CONFIG(pi->osh, pi->pmecap_offset + PME_CSR_OFFSET, sizeof(uint32));

	return (w & PME_CSR_PME_STAT) == PME_CSR_PME_STAT;
}

/* Disable PME generation, clear the PME status bit if set
 */
void
pcicore_pmeclr(void *pch)
{
	pcicore_info_t *pi = (pcicore_info_t *)pch;
	uint32 w;

	if (!pcicore_pmecap(pi))
		return;

	w = OSL_PCI_READ_CONFIG(pi->osh, pi->pmecap_offset + PME_CSR_OFFSET, sizeof(uint32));

	PCI_ERROR(("pcicore_pci_pmeclr PMECSR : 0x%x\n", w));

	/* PMESTAT is cleared by writing 1 to it */
	w &= ~(PME_CSR_PME_EN);

	OSL_PCI_WRITE_CONFIG(pi->osh, pi->pmecap_offset + PME_CSR_OFFSET, sizeof(uint32), w);
}

static void
pcicore_fixlatencytimer(pcicore_info_t* pch, uint8 timer_val)
{
	pcicore_info_t *pi = (pcicore_info_t *)pch;
	osl_t *osh = pi->osh;
	uint8 lattim = read_pci_cfg_byte(PCI_CFG_LATTIM);

	if (!lattim) {
		PCI_ERROR(("%s: Modifying PCI_CFG_LATTIM from 0x%x to 0x%x\n",
		           __FUNCTION__, lattim, timer_val));
		write_pci_cfg_byte(PCI_CFG_LATTIM, timer_val);
	}
}

uint32
pcie_lcreg(void *pch, uint32 mask, uint32 val)
{
	pcicore_info_t *pi = (pcicore_info_t *)pch;
	uint8 offset;

	offset = pi->pciecap_lcreg_offset;
	if (!offset)
		return 0;

	/* set operation */
	if (mask)
		OSL_PCI_WRITE_CONFIG(pi->osh, offset, sizeof(uint32), val);

	return OSL_PCI_READ_CONFIG(pi->osh, offset, sizeof(uint32));
}


uint32
pcicore_pciereg(void *pch, uint32 offset, uint32 mask, uint32 val, uint type)
{
	uint32 reg_val = 0;
	pcicore_info_t *pi = (pcicore_info_t *)pch;
	sbpcieregs_t *pcieregs = pi->regs.pcieregs;
	osl_t *osh = pi->osh;

	if (mask) {
		PCI_ERROR(("PCIEREG: 0x%x writeval  0x%x\n", offset, val));
		pcie_writereg(osh, pcieregs, type, offset, val);
	}

	/* Should not read register 0x154 */
	if (pi->sih->buscorerev <= 5 && offset == PCIE_DLLP_PCIE11 && type == PCIE_PCIEREGS)
		return reg_val;

	reg_val = pcie_readreg(osh, pcieregs, type, offset);
	PCI_ERROR(("PCIEREG: 0x%x readval is 0x%x\n", offset, reg_val));

	return reg_val;
}

uint32
pcicore_pcieserdesreg(void *pch, uint32 mdioslave, uint32 offset, uint32 mask, uint32 val)
{
	uint32 reg_val = 0;
	pcicore_info_t *pi = (pcicore_info_t *)pch;

	if (mask) {
		PCI_ERROR(("PCIEMDIOREG: 0x%x writeval  0x%x\n", offset, val));
		pcie_mdiowrite(pi, mdioslave, offset, val);
	}

	if (pcie_mdioread(pi, mdioslave, offset, &reg_val))
		reg_val = 0xFFFFFFFF;
	PCI_ERROR(("PCIEMDIOREG: dev 0x%x offset 0x%x read 0x%x\n", mdioslave, offset, reg_val));

	return reg_val;
}


#if defined(BCMDBG_DUMP)

/* size that can take bitfielddump */
#define BITFIELD_DUMP_SIZE  2048

/* Dump PCIE PLP/DLLP/TLP  diagnostic registers */
int
pcicore_dump_pcieregs(void *pch, struct bcmstrbuf *b)
{
	pcicore_info_t *pi = (pcicore_info_t *)pch;
	sbpcieregs_t *pcieregs = pi->regs.pcieregs;
	si_t *sih = pi->sih;
	uint reg_val = 0;
	char *bitfield_dump_buf;

	if (!(bitfield_dump_buf = MALLOC(pi->osh, BITFIELD_DUMP_SIZE))) {
		printf("bitfield dump allocation failed\n");
		return BCME_NOMEM;
	}

	bcm_bprintf(b, "PLPRegs \t");
	bcmdumpfields(si_pcie_readreg, (void *)(uintptr)pi->sih, PCIE_PCIEREGS,
		(struct fielddesc *)(uintptr)pcie_plp_regdesc,
		bitfield_dump_buf, BITFIELD_DUMP_SIZE);
	bcm_bprintf(b, "%s", bitfield_dump_buf);
	bzero(bitfield_dump_buf, BITFIELD_DUMP_SIZE);
	bcm_bprintf(b, "\n");
	bcm_bprintf(b, "DLLPRegs \t");
	bcmdumpfields(si_pcie_readreg, (void *)(uintptr)pi->sih, PCIE_PCIEREGS,
		(struct fielddesc *)(uintptr)pcie_dllp_regdesc,
		bitfield_dump_buf, BITFIELD_DUMP_SIZE);
	bcm_bprintf(b, "%s", bitfield_dump_buf);
	bzero(bitfield_dump_buf, BITFIELD_DUMP_SIZE);
	bcm_bprintf(b, "\n");
	bcm_bprintf(b, "TLPRegs \t");
	bcmdumpfields(si_pcie_readreg, (void *)(uintptr)pi->sih, PCIE_PCIEREGS,
		(struct fielddesc *)(uintptr)pcie_tlp_regdesc,
		bitfield_dump_buf, BITFIELD_DUMP_SIZE);
	bcm_bprintf(b, "%s", bitfield_dump_buf);
	bzero(bitfield_dump_buf, BITFIELD_DUMP_SIZE);
	bcm_bprintf(b, "\n");

	/* enable mdio access to SERDES */
	W_REG(pi->osh, (&pcieregs->mdiocontrol), MDIOCTL_PREAM_EN | MDIOCTL_DIVISOR_VAL);

	bcm_bprintf(b, "SERDES regs \n");
	if (sih->buscorerev >= 10) {
		pcie_mdioread(pi, MDIO_DEV_IEEE0, 0x2, &reg_val);
		bcm_bprintf(b, "block IEEE0, offset 2: 0x%x\n", reg_val);
		pcie_mdioread(pi, MDIO_DEV_IEEE0, 0x3, &reg_val);
		bcm_bprintf(b, "block IEEE0, offset 2: 0x%x\n", reg_val);
		pcie_mdioread(pi, MDIO_DEV_IEEE1, 0x08, &reg_val);
		bcm_bprintf(b, "block IEEE1, lanestatus: 0x%x\n", reg_val);
		pcie_mdioread(pi, MDIO_DEV_IEEE1, 0x0a, &reg_val);
		bcm_bprintf(b, "block IEEE1, lanestatus2: 0x%x\n", reg_val);
		pcie_mdioread(pi, MDIO_DEV_BLK4, 0x16, &reg_val);
		bcm_bprintf(b, "MDIO_DEV_BLK4, lanetest0: 0x%x\n", reg_val);
		pcie_mdioread(pi, MDIO_DEV_TXPLL, 0x11, &reg_val);
		bcm_bprintf(b, "MDIO_DEV_TXPLL, pllcontrol: 0x%x\n", reg_val);
		pcie_mdioread(pi, MDIO_DEV_TXPLL, 0x12, &reg_val);
		bcm_bprintf(b, "MDIO_DEV_TXPLL, plltimer1: 0x%x\n", reg_val);
		pcie_mdioread(pi, MDIO_DEV_TXPLL, 0x13, &reg_val);
		bcm_bprintf(b, "MDIO_DEV_TXPLL, plltimer2: 0x%x\n", reg_val);
		pcie_mdioread(pi, MDIO_DEV_TXPLL, 0x14, &reg_val);
		bcm_bprintf(b, "MDIO_DEV_TXPLL, plltimer3: 0x%x\n", reg_val);
		pcie_mdioread(pi, MDIO_DEV_TXPLL, 0x17, &reg_val);
		bcm_bprintf(b, "MDIO_DEV_TXPLL, freqdetcounter: 0x%x\n", reg_val);
	} else {
		pcie_mdioread(pi, MDIODATA_DEV_RX, SERDES_RX_TIMER1, &reg_val);
		bcm_bprintf(b, "rxtimer1 0x%x ", reg_val);
		pcie_mdioread(pi, MDIODATA_DEV_RX, SERDES_RX_CDR, &reg_val);
		bcm_bprintf(b, "rxCDR 0x%x ", reg_val);
		pcie_mdioread(pi, MDIODATA_DEV_RX, SERDES_RX_CDRBW, &reg_val);
		bcm_bprintf(b, "rxCDRBW 0x%x\n", reg_val);
	}

	/* disable mdio access to SERDES */
	W_REG(pi->osh, (&pcieregs->mdiocontrol), 0);

	MFREE(pi->osh, bitfield_dump_buf, BITFIELD_DUMP_SIZE);

	return 0;
}
#endif	

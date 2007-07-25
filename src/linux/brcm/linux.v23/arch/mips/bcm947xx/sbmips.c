/*
 * BCM47XX Sonics SiliconBackplane MIPS core routines
 *
 * Copyright 2005, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: sbmips.c,v 1.3 2005/03/07 08:35:32 kanki Exp $
 */

#include <typedefs.h>
#include <osl.h>
#include <sbutils.h>
#include <bcmdevs.h>
#include <bcmnvram.h>
#include <bcmutils.h>
#include <hndmips.h>
#include <sbconfig.h>
#include <sbextif.h>
#include <sbchipc.h>
#include <sbmemc.h>
#include <mipsinc.h>

/* 
 * Returns TRUE if an external UART exists at the given base
 * register.
 */
static bool	
BCMINITFN(serial_exists)(uint8 *regs)
{
	uint8 save_mcr, status1;

	save_mcr = R_REG(&regs[UART_MCR]);
	W_REG(&regs[UART_MCR], UART_MCR_LOOP | 0x0a);
	status1 = R_REG(&regs[UART_MSR]) & 0xf0;
	W_REG(&regs[UART_MCR], save_mcr);

	return (status1 == 0x90);
}

/* 
 * Initializes UART access. The callback function will be called once
 * per found UART.
 */
void 
BCMINITFN(sb_serial_init)(void *sbh, void (*add)(void *regs, uint irq, uint baud_base, uint reg_shift))
{
	void *regs;
	ulong base;
	uint irq;
	int i, n;

	if ((regs = sb_setcore(sbh, SB_EXTIF, 0))) {
		extifregs_t *eir = (extifregs_t *) regs;
		sbconfig_t *sb;

		/* Determine external UART register base */
		sb = (sbconfig_t *)((ulong) eir + SBCONFIGOFF);
		base = EXTIF_CFGIF_BASE(sb_base(R_REG(&sb->sbadmatch1)));

		/* Determine IRQ */
		irq = sb_irq(sbh);

		/* Disable GPIO interrupt initially */
		W_REG(&eir->gpiointpolarity, 0);
		W_REG(&eir->gpiointmask, 0);

		/* Search for external UARTs */
		n = 2;
		for (i = 0; i < 2; i++) {
			regs = (void *) REG_MAP(base + (i * 8), 8);
			if (BCMINIT(serial_exists)(regs)) {
				/* Set GPIO 1 to be the external UART IRQ */
				W_REG(&eir->gpiointmask, 2);
				if (add)
					add(regs, irq, 13500000, 0);
			}
		}

		/* Add internal UART if enabled */
		if (R_REG(&eir->corecontrol) & CC_UE)
			if (add)
				add((void *) &eir->uartdata, irq, sb_clock(sbh), 2);
	} else if ((regs = sb_setcore(sbh, SB_CC, 0))) {
		chipcregs_t *cc = (chipcregs_t *) regs;
		uint32 rev, cap, pll, baud_base, div;

		/* Determine core revision and capabilities */
		rev = sb_corerev(sbh);
		cap = R_REG(&cc->capabilities);
		pll = cap & CAP_PLL_MASK;

		/* Determine IRQ */
		irq = sb_irq(sbh);

		if (pll == PLL_TYPE1) {
			/* PLL clock */
			baud_base = sb_clock_rate(pll,
						  R_REG(&cc->clockcontrol_n),
						  R_REG(&cc->clockcontrol_m2));
			div = 1;
		} else {
			if (rev >= 11) {
				/* Fixed ALP clock */
				baud_base = 20000000;
				div = 1;
				/* Set the override bit so we don't divide it */
				W_REG(&cc->corecontrol, CC_UARTCLKO);
			} else if (rev >= 3) {
				/* Internal backplane clock */
				baud_base = sb_clock(sbh);
				div = 2;	/* Minimum divisor */
				W_REG(&cc->clkdiv, ((R_REG(&cc->clkdiv) & ~CLKD_UART) | div));
			} else {
				/* Fixed internal backplane clock */
				baud_base = 88000000;
				div = 48;
			}

			/* Clock source depends on strapping if UartClkOverride is unset */
			if ((rev > 0) &&
			    ((R_REG(&cc->corecontrol) & CC_UARTCLKO) == 0)) {
				if ((cap & CAP_UCLKSEL) == CAP_UINTCLK) {
					/* Internal divided backplane clock */
					baud_base /= div;
				} else {
					/* Assume external clock of 1.8432 MHz */
					baud_base = 1843200;
				}
			}
		}

		/* Add internal UARTs */
		n = cap & CAP_UARTS_MASK;
		for (i = 0; i < n; i++) {
			/* Register offset changed after revision 0 */
			if (rev)
				regs = (void *)((ulong) &cc->uart0data + (i * 256));
			else
				regs = (void *)((ulong) &cc->uart0data + (i * 8));

			if (add)
				add(regs, irq, baud_base, 0);
		}
	}
}

/*
 * Initialize jtag master and return handle for
 * jtag_rwreg. Returns NULL on failure.
 */
void *
sb_jtagm_init(void *sbh, uint clkd, bool exttap)
{
	void *regs;

	if ((regs = sb_setcore(sbh, SB_CC, 0)) != NULL) {
		chipcregs_t *cc = (chipcregs_t *) regs;
		uint32 tmp;

		/*
		 * Determine jtagm availability from
		 * core revision and capabilities.
		 */
		tmp = sb_corerev(sbh);
		/*
		 * Corerev 10 has jtagm, but the only chip
		 * with it does not have a mips, and
		 * the layout of the jtagcmd register is
		 * different. We'll only accept >= 11.
		 */
		if (tmp < 11)
			return (NULL);

		tmp = R_REG(&cc->capabilities);
		if ((tmp & CAP_JTAGP) == 0)
			return (NULL);

		/* Set clock divider if requested */
		if (clkd != 0) {
			tmp = R_REG(&cc->clkdiv);
			tmp = (tmp & ~CLKD_JTAG) |
				((clkd << CLKD_JTAG_SHIFT) & CLKD_JTAG);
			W_REG(&cc->clkdiv, tmp);
		}

		/* Enable jtagm */
		tmp = JCTRL_EN | (exttap ? JCTRL_EXT_EN : 0);
		W_REG(&cc->jtagctrl, tmp);
	}

	return (regs);
}

void
sb_jtagm_disable(void *h)
{
	chipcregs_t *cc = (chipcregs_t *)h;

	W_REG(&cc->jtagctrl, R_REG(&cc->jtagctrl) & ~JCTRL_EN);
}

/*
 * Read/write a jtag register. Assumes a target with
 * 8 bit IR and 32 bit DR.
 */
#define	IRWIDTH		8
#define	DRWIDTH		32
uint32
jtag_rwreg(void *h, uint32 ir, uint32 dr)
{
	chipcregs_t *cc = (chipcregs_t *) h;
	uint32 tmp;

	W_REG(&cc->jtagir, ir);
	W_REG(&cc->jtagdr, dr);
	tmp = JCMD_START | JCMD_ACC_IRDR |
		((IRWIDTH - 1) << JCMD_IRW_SHIFT) |
		(DRWIDTH - 1);
	W_REG(&cc->jtagcmd, tmp);
	while (((tmp = R_REG(&cc->jtagcmd)) & JCMD_BUSY) == JCMD_BUSY) {
		/* OSL_DELAY(1); */
	}

	tmp = R_REG(&cc->jtagdr);
	return (tmp);
}

/* Returns the SB interrupt flag of the current core. */
uint32
sb_flag(void *sbh)
{
	void *regs;
	sbconfig_t *sb;

	regs = sb_coreregs(sbh);
	sb = (sbconfig_t *)((ulong) regs + SBCONFIGOFF);

	return (R_REG(&sb->sbtpsflag) & SBTPS_NUM0_MASK);
}

static const uint32 sbips_int_mask[] = {
	0,
	SBIPS_INT1_MASK,
	SBIPS_INT2_MASK,
	SBIPS_INT3_MASK,
	SBIPS_INT4_MASK
};

static const uint32 sbips_int_shift[] = {
	0,
	0,
	SBIPS_INT2_SHIFT,
	SBIPS_INT3_SHIFT,
	SBIPS_INT4_SHIFT
};

/* 
 * Returns the MIPS IRQ assignment of the current core. If unassigned,
 * 0 is returned.
 */
uint
sb_irq(void *sbh)
{
	uint idx;
	void *regs;
	sbconfig_t *sb;
	uint32 flag, sbipsflag;
	uint irq = 0;

	flag = sb_flag(sbh);

	idx = sb_coreidx(sbh);

	if ((regs = sb_setcore(sbh, SB_MIPS, 0)) ||
	    (regs = sb_setcore(sbh, SB_MIPS33, 0))) {
		sb = (sbconfig_t *)((ulong) regs + SBCONFIGOFF);

		/* sbipsflag specifies which core is routed to interrupts 1 to 4 */
		sbipsflag = R_REG(&sb->sbipsflag);
		for (irq = 1; irq <= 4; irq++) {
			if (((sbipsflag & sbips_int_mask[irq]) >> sbips_int_shift[irq]) == flag)
				break;
		}
		if (irq == 5)
			irq = 0;
	}

	sb_setcoreidx(sbh, idx);

	return irq;
}

/* Clears the specified MIPS IRQ. */
static void 
BCMINITFN(sb_clearirq)(void *sbh, uint irq)
{
	void *regs;
	sbconfig_t *sb;

	if (!(regs = sb_setcore(sbh, SB_MIPS, 0)) &&
	    !(regs = sb_setcore(sbh, SB_MIPS33, 0)))
		ASSERT(regs);
	sb = (sbconfig_t *)((ulong) regs + SBCONFIGOFF);

	if (irq == 0)
		W_REG(&sb->sbintvec, 0);
	else
		OR_REG(&sb->sbipsflag, sbips_int_mask[irq]);
}

/* 
 * Assigns the specified MIPS IRQ to the specified core. Shared MIPS
 * IRQ 0 may be assigned more than once.
 */
static void 
BCMINITFN(sb_setirq)(void *sbh, uint irq, uint coreid, uint coreunit)
{
	void *regs;
	sbconfig_t *sb;
	uint32 flag;

	regs = sb_setcore(sbh, coreid, coreunit);
	ASSERT(regs);
	flag = sb_flag(sbh);

	if (!(regs = sb_setcore(sbh, SB_MIPS, 0)) &&
	    !(regs = sb_setcore(sbh, SB_MIPS33, 0)))
		ASSERT(regs);
	sb = (sbconfig_t *)((ulong) regs + SBCONFIGOFF);

	if (irq == 0)
		OR_REG(&sb->sbintvec, 1 << flag);
	else {
		flag <<= sbips_int_shift[irq];
		ASSERT(!(flag & ~sbips_int_mask[irq]));
		flag |= R_REG(&sb->sbipsflag) & ~sbips_int_mask[irq];
		W_REG(&sb->sbipsflag, flag);
	}
}	

/* 
 * Initializes clocks and interrupts. SB and NVRAM access must be
 * initialized prior to calling.
 */
void 
BCMINITFN(sb_mips_init)(void *sbh)
{
	ulong hz, ns, tmp;
	extifregs_t *eir;
	chipcregs_t *cc;
	char *value;
	uint irq;

	/* Figure out current SB clock speed */
	if ((hz = sb_clock(sbh)) == 0)
		hz = 100000000;
	ns = 1000000000 / hz;

	/* Setup external interface timing */
	if ((eir = sb_setcore(sbh, SB_EXTIF, 0))) {
		/* Initialize extif so we can get to the LEDs and external UART */
		W_REG(&eir->prog_config, CF_EN);

		/* Set timing for the flash */
		tmp = CEIL(10, ns) << FW_W3_SHIFT;	/* W3 = 10nS */
		tmp = tmp | (CEIL(40, ns) << FW_W1_SHIFT); /* W1 = 40nS */
		tmp = tmp | CEIL(120, ns);		/* W0 = 120nS */
		W_REG(&eir->prog_waitcount, tmp);	/* 0x01020a0c for a 100Mhz clock */

		/* Set programmable interface timing for external uart */
		tmp = CEIL(10, ns) << FW_W3_SHIFT;	/* W3 = 10nS */
		tmp = tmp | (CEIL(20, ns) << FW_W2_SHIFT); /* W2 = 20nS */
		tmp = tmp | (CEIL(100, ns) << FW_W1_SHIFT); /* W1 = 100nS */
		tmp = tmp | CEIL(120, ns);		/* W0 = 120nS */
		W_REG(&eir->prog_waitcount, tmp);	/* 0x01020a0c for a 100Mhz clock */
	} else if ((cc = sb_setcore(sbh, SB_CC, 0))) {
//==================================tallest===============================================
		/* set register for external IO to control LED. */
                W_REG(&cc->prog_config, 0x11);
                tmp = CEIL(10, ns) << FW_W3_SHIFT;      /* W3 = 10nS */
                tmp = tmp | (CEIL(40, ns) << FW_W1_SHIFT); /* W1 = 40nS */
                tmp = tmp | CEIL(240, ns);              /* W0 = 120nS */
                W_REG(&cc->prog_waitcount, tmp);        /* 0x01020a0c for a 100Mhz clock */
//========================================================================================
		/* Set timing for the flash */
		tmp = CEIL(10, ns) << FW_W3_SHIFT;	/* W3 = 10nS */
		tmp |= CEIL(10, ns) << FW_W1_SHIFT;	/* W1 = 10nS */
		tmp |= CEIL(120, ns);			/* W0 = 120nS */

		// Added by Chen-I for 5365
		if (BCMINIT(sb_chip)(sbh) == BCM5365_DEVICE_ID)
		{
			W_REG(&cc->flash_waitcount, tmp);
			W_REG(&cc->pcmcia_memwait, tmp);
		}
		else
		{
			if (sb_corerev(sbh) < 9)  
				W_REG(&cc->flash_waitcount, tmp);
	
			if ( (sb_corerev(sbh) < 9) || 
			     ((BCMINIT(sb_chip)(sbh) == BCM5350_DEVICE_ID) && BCMINIT(sb_chiprev)(sbh) == 0) ) {
				W_REG(&cc->pcmcia_memwait, tmp);
			}
		}

		// Added by Chen-I & Yen for enabling 5350 EXTIF
		if (BCMINIT(sb_chip)(sbh) == BCM5350_DEVICE_ID) 
		{
			/* Set programmable interface timing for external uart */
			tmp = CEIL(10, ns) << FW_W3_SHIFT;      /* W3 = 10nS */
			tmp = tmp | (CEIL(20, ns) << FW_W2_SHIFT); /* W2 = 20nS */
			tmp = tmp | (CEIL(100, ns) << FW_W1_SHIFT); /* W1 = 100nS */
			tmp = tmp | CEIL(120, ns);              /* W0 = 120nS */
			W_REG(&cc->prog_waitcount, tmp);       /* 0x01020a0c for a 100Mhz clock */
			//printf("===========config_REG=%d\n", R_REG(&cc->prog_config));
			//printf("-----------config_REG_addr=%x\n", &cc->prog_config);
			//printf("===========waitcount_REG=%d\n", R_REG(&cc->prog_waitcount));
			//printf("-----------waitcount_REG=%x\n", &cc->prog_waitcount);
		}
	}

	/* Chip specific initialization */
	switch (BCMINIT(sb_chip)(sbh)) {
	case BCM4710_DEVICE_ID:
		/* Clear interrupt map */
		for (irq = 0; irq <= 4; irq++)
			BCMINIT(sb_clearirq)(sbh, irq);
		BCMINIT(sb_setirq)(sbh, 0, SB_CODEC, 0);
		BCMINIT(sb_setirq)(sbh, 0, SB_EXTIF, 0);
		BCMINIT(sb_setirq)(sbh, 2, SB_ENET, 1);
		BCMINIT(sb_setirq)(sbh, 3, SB_ILINE20, 0);
		BCMINIT(sb_setirq)(sbh, 4, SB_PCI, 0);
		ASSERT(eir);
		value = BCMINIT(nvram_get)("et0phyaddr");
		if (value && !strcmp(value, "31")) {
			/* Enable internal UART */
			W_REG(&eir->corecontrol, CC_UE);
			/* Give USB its own interrupt */
			BCMINIT(sb_setirq)(sbh, 1, SB_USB, 0);
		} else {
			/* Disable internal UART */
			W_REG(&eir->corecontrol, 0);
			/* Give Ethernet its own interrupt */
			BCMINIT(sb_setirq)(sbh, 1, SB_ENET, 0);
			BCMINIT(sb_setirq)(sbh, 0, SB_USB, 0);
		}
		break;
	case BCM4310_DEVICE_ID:
		MTC0(C0_BROADCOM, 0, MFC0(C0_BROADCOM, 0) & ~(1 << 22));
		break;
        case BCM5350_DEVICE_ID:
                /* Clear interrupt map */
                for (irq = 0; irq <= 4; irq++)
                        BCMINIT(sb_clearirq)(sbh, irq);
                BCMINIT(sb_setirq)(sbh, 0, SB_CC, 0);
                BCMINIT(sb_setirq)(sbh, 1, SB_D11, 0);
                BCMINIT(sb_setirq)(sbh, 2, SB_ENET, 0);
	        BCMINIT(sb_setirq)(sbh, 3, SB_IPSEC, 0);
                BCMINIT(sb_setirq)(sbh, 4, SB_USB, 0);
		break;
	}
}

uint32
BCMINITFN(sb_mips_clock)(void *sbh)
{
	extifregs_t *eir;
	chipcregs_t *cc;
	uint32 n, m;
	uint idx;
	uint32 pll_type, rate = 0;

	/* get index of the current core */
	idx = sb_coreidx(sbh);
	pll_type = PLL_TYPE1;

	/* switch to extif or chipc core */
	if ((eir = (extifregs_t *) sb_setcore(sbh, SB_EXTIF, 0))) {
		n = R_REG(&eir->clockcontrol_n);
		m = R_REG(&eir->clockcontrol_sb);
	} else if ((cc = (chipcregs_t *) sb_setcore(sbh, SB_CC, 0))) {

		/* 5354 chip uses a non programmable PLL of frequency 240MHz */
		if (sb_chip(sbh) == BCM5354_CHIP_ID) {
			rate = 240000000;
			goto out;
		}

		pll_type = R_REG(&cc->capabilities) & CAP_PLL_MASK;
		n = R_REG(&cc->clockcontrol_n);
		if ((pll_type == PLL_TYPE2) ||
		    (pll_type == PLL_TYPE4) ||
		    (pll_type == PLL_TYPE6) ||
		    (pll_type == PLL_TYPE7))
			m = R_REG(&cc->clockcontrol_mips);
		else if (pll_type == PLL_TYPE5) {
			rate = 200000000;
			goto out;
		}
		else if (pll_type == PLL_TYPE3) {
			if (BCMINIT(sb_chip)(sbh) == BCM5365_DEVICE_ID) { /* 5365 is also type3 */
				rate = 200000000;
				goto out;
			} else
				m = R_REG(&cc->clockcontrol_m2); /* 5350 uses m2 to control mips */
		} else
			m = R_REG(&cc->clockcontrol_sb);
	} else
		goto out;

	// Added by Chen-I for 5365 
	if (BCMINIT(sb_chip)(sbh) == BCM5365_DEVICE_ID)
		rate = 100000000;
	else
		/* calculate rate */
		rate = sb_clock_rate(pll_type, n, m);

	if (pll_type == PLL_TYPE6)
		rate = SB2MIPS_T6(rate);

out:
	/* switch back to previous core */
	sb_setcoreidx(sbh, idx);

	return rate;
}

#define ALLINTS (IE_IRQ0 | IE_IRQ1 | IE_IRQ2 | IE_IRQ3 | IE_IRQ4)

static void 
BCMINITFN(handler)(void)
{
	/* Step 11 */
	__asm__ (
		".set\tmips32\n\t"
		"ssnop\n\t"
		"ssnop\n\t"
	/* Disable interrupts */
	/*	MTC0(C0_STATUS, 0, MFC0(C0_STATUS, 0) & ~(ALLINTS | STO_IE)); */
		"mfc0 $15, $12\n\t"
	/* Just a Hack to not to use reg 'at' which was causing problems on 4704 A2 */
		"li $14, -31746\n\t"
		"and $15, $15, $14\n\t"
		"mtc0 $15, $12\n\t"
		"eret\n\t"
		"nop\n\t"
		"nop\n\t"
		".set\tmips0"
	);
}

/* The following MUST come right after handler() */
static void 
BCMINITFN(afterhandler)(void)
{
}

/*
 * Set the MIPS, backplane and PCI clocks as closely as possible.
 */
bool 
BCMINITFN(sb_mips_setclock)(void *sbh, uint32 mipsclock, uint32 sbclock, uint32 pciclock)
{
	extifregs_t *eir = NULL;
	chipcregs_t *cc = NULL;
	mipsregs_t *mipsr = NULL;
	volatile uint32 *clockcontrol_n, *clockcontrol_sb, *clockcontrol_pci, *clockcontrol_m2;
	uint32 orig_n, orig_sb, orig_pci, orig_m2, orig_mips, orig_ratio_parm, orig_ratio_cfg;
	uint32 pll_type, sync_mode;
	uint ic_size, ic_lsize;
	uint idx, i;
	typedef struct {
		uint32 mipsclock;
		uint16 n;
		uint32 sb;
		uint32 pci33;
		uint32 pci25;
	} n3m_table_t;
	static n3m_table_t BCMINITDATA(type1_table)[] = {
		{  96000000, 0x0303, 0x04020011, 0x11030011, 0x11050011 }, /*  96.000 32.000 24.000 */
		{ 100000000, 0x0009, 0x04020011, 0x11030011, 0x11050011 }, /* 100.000 33.333 25.000 */
		{ 104000000, 0x0802, 0x04020011, 0x11050009, 0x11090009 }, /* 104.000 31.200 24.960 */
		{ 108000000, 0x0403, 0x04020011, 0x11050009, 0x02000802 }, /* 108.000 32.400 24.923 */
		{ 112000000, 0x0205, 0x04020011, 0x11030021, 0x02000403 }, /* 112.000 32.000 24.889 */
		{ 115200000, 0x0303, 0x04020009, 0x11030011, 0x11050011 }, /* 115.200 32.000 24.000 */
		{ 120000000, 0x0011, 0x04020011, 0x11050011, 0x11090011 }, /* 120.000 30.000 24.000 */
		{ 124800000, 0x0802, 0x04020009, 0x11050009, 0x11090009 }, /* 124.800 31.200 24.960 */
		{ 128000000, 0x0305, 0x04020011, 0x11050011, 0x02000305 }, /* 128.000 32.000 24.000 */
		{ 132000000, 0x0603, 0x04020011, 0x11050011, 0x02000305 }, /* 132.000 33.000 24.750 */
		{ 136000000, 0x0c02, 0x04020011, 0x11090009, 0x02000603 }, /* 136.000 32.640 24.727 */
		{ 140000000, 0x0021, 0x04020011, 0x11050021, 0x02000c02 }, /* 140.000 30.000 24.706 */
		{ 144000000, 0x0405, 0x04020011, 0x01020202, 0x11090021 }, /* 144.000 30.857 24.686 */
		{ 150857142, 0x0605, 0x04020021, 0x02000305, 0x02000605 }, /* 150.857 33.000 24.000 */
		{ 152000000, 0x0e02, 0x04020011, 0x11050021, 0x02000e02 }, /* 152.000 32.571 24.000 */
		{ 156000000, 0x0802, 0x04020005, 0x11050009, 0x11090009 }, /* 156.000 31.200 24.960 */
		{ 160000000, 0x0309, 0x04020011, 0x11090011, 0x02000309 }, /* 160.000 32.000 24.000 */
		{ 163200000, 0x0c02, 0x04020009, 0x11090009, 0x02000603 }, /* 163.200 32.640 24.727 */
		{ 168000000, 0x0205, 0x04020005, 0x11030021, 0x02000403 }, /* 168.000 32.000 24.889 */
		{ 176000000, 0x0602, 0x04020003, 0x11050005, 0x02000602 }, /* 176.000 33.000 24.000 */
	};
	typedef struct {
		uint32 mipsclock;
		uint16 n;
		uint32 m2; /* that is the clockcontrol_m2 */
	} type3_table_t;
	static type3_table_t type3_table[] = { /* for 5350, mips clock is always double sb clock */
		{ 150000000, 0x311, 0x4020005 }, 
		{ 200000000, 0x311, 0x4020003 }, 
	};
	typedef struct {
		uint32 mipsclock;
		uint32 sbclock;
		uint16 n;
		uint32 sb;
		uint32 pci33;
		uint32 m2;
		uint32 m3;
		uint32 ratio_cfg;
		uint32 ratio_parm;
	} n4m_table_t;

	static n4m_table_t BCMINITDATA(type2_table)[] = {
		{ 180000000,  80000000, 0x0403, 0x01010000, 0x01020300, 0x01020600, 0x05000100,	 8, 0x012a00a9 },
		{ 180000000,  90000000, 0x0403, 0x01000100, 0x01020300, 0x01000100, 0x05000100, 11, 0x0aaa0555 },
		{ 200000000, 100000000, 0x0303, 0x02010000, 0x02040001, 0x02010000, 0x06000001, 11, 0x0aaa0555 },
		{ 211200000, 105600000, 0x0902, 0x01000200, 0x01030400, 0x01000200, 0x05000200, 11, 0x0aaa0555 },
		{ 220800000, 110400000, 0x1500, 0x01000200, 0x01030400, 0x01000200, 0x05000200, 11, 0x0aaa0555 },
		{ 230400000, 115200000, 0x0604, 0x01000200, 0x01020600, 0x01000200, 0x05000200, 11, 0x0aaa0555 },
		{ 234000000, 104000000, 0x0b01, 0x01010000, 0x01010700, 0x01020600, 0x05000100,  8, 0x012a00a9 },
		{ 240000000, 120000000,	0x0803,	0x01000200, 0x01020600,	0x01000200, 0x05000200, 11, 0x0aaa0555 },
		{ 252000000, 126000000,	0x0504,	0x01000100, 0x01020500,	0x01000100, 0x05000100, 11, 0x0aaa0555 },
		{ 264000000, 132000000, 0x0903, 0x01000200, 0x01020700, 0x01000200, 0x05000200, 11, 0x0aaa0555 },
		{ 270000000, 120000000, 0x0703, 0x01010000, 0x01030400, 0x01020600, 0x05000100,  8, 0x012a00a9 },
		{ 276000000, 122666666, 0x1500, 0x01010000, 0x01030400, 0x01020600, 0x05000100,  8, 0x012a00a9 },
		{ 280000000, 140000000, 0x0503, 0x01000000, 0x01010600, 0x01000000, 0x05000000, 11, 0x0aaa0555 },
		{ 288000000, 128000000, 0x0604, 0x01010000, 0x01030400, 0x01020600, 0x05000100,  8, 0x012a00a9 },
		{ 288000000, 144000000, 0x0404, 0x01000000, 0x01010600, 0x01000000, 0x05000000, 11, 0x0aaa0555 },
		{ 300000000, 133333333, 0x0803, 0x01010000, 0x01020600, 0x01020600, 0x05000100,  8, 0x012a00a9 },
		{ 300000000, 150000000, 0x0803, 0x01000100, 0x01020600, 0x01000100, 0x05000100, 11, 0x0aaa0555 }
	};

	static n4m_table_t BCMINITDATA(type4_table)[] = {
		{ 192000000,  96000000, 0x0702,	0x04000011, 0x11030011, 0x04000011, 0x04000003, 11, 0x0aaa0555 },
		{ 198000000,  99000000, 0x0603,	0x11020005, 0x11030011, 0x11020005, 0x04000005, 11, 0x0aaa0555 },
		{ 200000000, 100000000, 0x0009,	0x04020011, 0x11030011, 0x04020011, 0x04020003, 11, 0x0aaa0555 },
		{ 204000000, 102000000, 0x0c02, 0x11020005, 0x01030303, 0x11020005, 0x04000005, 11, 0x0aaa0555 },
		{ 208000000, 104000000, 0x0802, 0x11030002, 0x11090005, 0x11030002, 0x04000003, 11, 0x0aaa0555 },
		{ 210000000, 105000000, 0x0209, 0x11020005, 0x01030303, 0x11020005, 0x04000005, 11, 0x0aaa0555 },
		{ 216000000, 108000000, 0x0111, 0x11020005, 0x01030303, 0x11020005, 0x04000005, 11, 0x0aaa0555 },
		{ 224000000, 112000000, 0x0205, 0x11030002, 0x02002103, 0x11030002, 0x04000003, 11, 0x0aaa0555 },
		{ 228000000, 101333333, 0x0e02, 0x11030003, 0x11210005, 0x01030305, 0x04000005,  8, 0x012a00a9 },
		{ 228000000, 114000000, 0x0e02, 0x11020005, 0x11210005, 0x11020005, 0x04000005, 11, 0x0aaa0555 },
		{ 240000000, 102857143,	0x0109,	0x04000021, 0x01050203,	0x11030021, 0x04000003, 13, 0x254a14a9 },
		{ 240000000, 120000000,	0x0109,	0x11030002, 0x01050203,	0x11030002, 0x04000003, 11, 0x0aaa0555 },
		{ 252000000, 100800000,	0x0203,	0x04000009, 0x11050005,	0x02000209, 0x04000002,  9, 0x02520129 },
		{ 252000000, 126000000,	0x0203,	0x04000005, 0x11050005,	0x04000005, 0x04000002, 11, 0x0aaa0555 },
		{ 264000000, 132000000, 0x0602, 0x04000005, 0x11050005, 0x04000005, 0x04000002, 11, 0x0aaa0555 },
		{ 272000000, 116571428, 0x0c02, 0x04000021, 0x02000909, 0x02000221, 0x04000003, 13, 0x254a14a9 },
		{ 280000000, 120000000, 0x0209, 0x04000021, 0x01030303, 0x02000221, 0x04000003, 13, 0x254a14a9 },
		{ 288000000, 123428571, 0x0111, 0x04000021, 0x01030303, 0x02000221, 0x04000003, 13, 0x254a14a9 },
		{ 300000000, 120000000, 0x0009, 0x04000009, 0x01030203, 0x02000902, 0x04000002,  9, 0x02520129 },
		{ 300000000, 150000000, 0x0009, 0x04000005, 0x01030203, 0x04000005, 0x04000002, 11, 0x0aaa0555 }
	};

	static n4m_table_t BCMINITDATA(type7_table)[] = {
		{ 183333333,  91666666, 0x0605,	0x04000011, 0x11030011, 0x04000011, 0x04000003, 11, 0x0aaa0555 },
		{ 187500000,  93750000, 0x0a03,	0x04000011, 0x11030011, 0x04000011, 0x04000003, 11, 0x0aaa0555 },
		{ 196875000,  98437500, 0x1003, 0x11020005, 0x11050011, 0x11020005, 0x04000005, 11, 0x0aaa0555 },
		{ 200000000, 100000000, 0x0311, 0x04000011, 0x11030011, 0x04000009, 0x04000003, 11, 0x0aaa0555 },
		{ 200000000, 100000000, 0x0311, 0x04020011, 0x11030011, 0x04020011, 0x04020003, 11, 0x0aaa0555 },
		{ 206250000, 103125000, 0x1103, 0x11020005, 0x11050011, 0x11020005, 0x04000005, 11, 0x0aaa0555 },
		{ 212500000, 106250000,	0x0c05,	0x11020005, 0x01030303,	0x11020005, 0x04000005, 11, 0x0aaa0555 },
		{ 215625000, 107812500,	0x1203,	0x11090009, 0x11050005,	0x11020005, 0x04000005, 11, 0x0aaa0555 },
		{ 216666666, 108333333, 0x0805, 0x11020003, 0x11030011, 0x11020003, 0x04000003, 11, 0x0aaa0555 },
		{ 225000000, 112500000, 0x0d03, 0x11020003, 0x11030011, 0x11020003, 0x04000003, 11, 0x0aaa0555 },
		{ 233333333, 116666666, 0x0905, 0x11020003, 0x11030011, 0x11020003, 0x04000003, 11, 0x0aaa0555 },
		{ 237500000, 118750000, 0x0e05, 0x11020005, 0x11210005, 0x11020005, 0x04000005, 11, 0x0aaa0555 },
		{ 240000000, 120000000, 0x0b11, 0x11020009, 0x11210009, 0x11020009, 0x04000009, 11, 0x0aaa0555 },
		{ 250000000, 125000000, 0x0f03, 0x11020003, 0x11210003, 0x11020003, 0x04000003, 11, 0x0aaa0555 }
	};

	ulong start, end, dst;
	bool ret = FALSE;
	
	/* get index of the current core */
	idx = sb_coreidx(sbh);
	clockcontrol_m2 = NULL;

	/* switch to extif or chipc core */
	if ((eir = (extifregs_t *) sb_setcore(sbh, SB_EXTIF, 0))) {
		pll_type = PLL_TYPE1;
		clockcontrol_n = &eir->clockcontrol_n;
		clockcontrol_sb = &eir->clockcontrol_sb;
		clockcontrol_pci = &eir->clockcontrol_pci;
		clockcontrol_m2 = &cc->clockcontrol_m2;
	} else if ((cc = (chipcregs_t *) sb_setcore(sbh, SB_CC, 0))) {

		/* 5354 chipcommon pll setting can't be changed. 
		 * The PMU on power up comes up with the default clk frequency
		 * of 240MHz
		 */
		if (sb_chip(sbh) == BCM5354_CHIP_ID) {
			ret = TRUE;
			goto done;
		}

		pll_type = R_REG(&cc->capabilities) & CAP_PLL_MASK;
		if (pll_type == PLL_TYPE6) {
			clockcontrol_n = NULL;
			clockcontrol_sb = NULL;
			clockcontrol_pci = NULL;
		} else {
			clockcontrol_n = &cc->clockcontrol_n;
			clockcontrol_sb = &cc->clockcontrol_sb;
			clockcontrol_pci = &cc->clockcontrol_pci;
		clockcontrol_m2 = &cc->clockcontrol_m2;
		}
	} else
		goto done;

	if (pll_type == PLL_TYPE6) {
		/* Silence compilers */
		orig_n = orig_sb = orig_pci = 0;
	} else {
		/* Store the current clock register values */
		orig_n = R_REG(clockcontrol_n);
		orig_sb = R_REG(clockcontrol_sb);
		orig_pci = R_REG(clockcontrol_pci);
	}

	if (pll_type == PLL_TYPE1) {
		/* Keep the current PCI clock if not specified */
		if (pciclock == 0) {
			pciclock = sb_clock_rate(pll_type, R_REG(clockcontrol_n), R_REG(clockcontrol_pci));
			pciclock = (pciclock <= 25000000) ? 25000000 : 33000000;
		}

		/* Search for the closest MIPS clock less than or equal to a preferred value */
		for (i = 0; i < ARRAYSIZE(BCMINIT(type1_table)); i++) {
			ASSERT(BCMINIT(type1_table)[i].mipsclock ==
			       sb_clock_rate(pll_type, BCMINIT(type1_table)[i].n, BCMINIT(type1_table)[i].sb));
			if (BCMINIT(type1_table)[i].mipsclock > mipsclock)
				break;
		}
		if (i == 0) {
			ret = FALSE;
			goto done;
		} else {
			ret = TRUE;
			i--;
		}
		ASSERT(BCMINIT(type1_table)[i].mipsclock <= mipsclock);

		/* No PLL change */
		if ((orig_n == BCMINIT(type1_table)[i].n) &&
		    (orig_sb == BCMINIT(type1_table)[i].sb) &&
		    (orig_pci == BCMINIT(type1_table)[i].pci33))
			goto done;

		/* Set the PLL controls */
		W_REG(clockcontrol_n, BCMINIT(type1_table)[i].n);
		W_REG(clockcontrol_sb, BCMINIT(type1_table)[i].sb);
		if (pciclock == 25000000)
			W_REG(clockcontrol_pci, BCMINIT(type1_table)[i].pci25);
		else
			W_REG(clockcontrol_pci, BCMINIT(type1_table)[i].pci33);

		/* Reset */
		sb_watchdog(sbh, 1);

		while (1);
	} else if ((pll_type == PLL_TYPE3) &&
		   (BCMINIT(sb_chip)(sbh) != BCM5365_DEVICE_ID)) {
		/* 5350 */
		/* Search for the closest MIPS clock less than or equal to a preferred value */

		for (i = 0; i < ARRAYSIZE(type3_table); i++) {
			if (type3_table[i].mipsclock > mipsclock)
				break;
		}
		if (i == 0) {
			ret = FALSE;
			goto done;
		} else {
			ret = TRUE;
			i--;
		}
		ASSERT(type3_table[i].mipsclock <= mipsclock);

		/* No PLL change */
		orig_m2 = R_REG(&cc->clockcontrol_m2);
		if ((orig_n == type3_table[i].n) &&
		    (orig_m2 == type3_table[i].m2))  {
			goto done;
		}
		
		/* Set the PLL controls */
		W_REG(clockcontrol_n, type3_table[i].n);
		W_REG(clockcontrol_m2, type3_table[i].m2);

		/* Reset */
		sb_watchdog(sbh, 1);
		while (1);
	} else if ((pll_type == PLL_TYPE2) ||
		   (pll_type == PLL_TYPE4) ||
		   (pll_type == PLL_TYPE6) ||
		   (pll_type == PLL_TYPE7)) {
		n4m_table_t *table = NULL, *te;
		uint tabsz = 0;

		ASSERT(cc);

		orig_mips = R_REG(&cc->clockcontrol_mips);

		if (pll_type == PLL_TYPE6) {
			uint32 new_mips = 0;

			ret = TRUE;
			if (mipsclock <= SB2MIPS_T6(CC_T6_M1))
				new_mips = CC_T6_MMASK;

			if (orig_mips == new_mips)
				goto done;

			W_REG(&cc->clockcontrol_mips, new_mips);
			goto end_fill;
		}

		if (pll_type == PLL_TYPE2) {
			table = BCMINIT(type2_table);
			tabsz = ARRAYSIZE(BCMINIT(type2_table));
		} else if (pll_type == PLL_TYPE4) {
			table = BCMINIT(type4_table);
			tabsz = ARRAYSIZE(BCMINIT(type4_table));
		} else if (pll_type == PLL_TYPE7) {
			table = BCMINIT(type7_table);
			tabsz = ARRAYSIZE(BCMINIT(type7_table));
		} else
			ASSERT((char *)"No table for plltype" == NULL);

		/* Store the current clock register values */
		orig_m2 = R_REG(&cc->clockcontrol_m2);
		orig_ratio_parm = 0;
		orig_ratio_cfg = 0;

		/* Look up current ratio */
		for (i = 0; i < tabsz; i++) {
			if ((orig_n == table[i].n) &&
			    (orig_sb == table[i].sb) &&
			    (orig_pci == table[i].pci33) &&
			    (orig_m2 == table[i].m2) &&
			    (orig_mips == table[i].m3)) {
				orig_ratio_parm = table[i].ratio_parm;
				orig_ratio_cfg = table[i].ratio_cfg;
				break;
			}
		}

		/* Search for the closest MIPS clock greater or equal to a preferred value */
		for (i = 0; i < tabsz; i++) {
			ASSERT(table[i].mipsclock ==
			       sb_clock_rate(pll_type, table[i].n, table[i].m3));
			if ((mipsclock <= table[i].mipsclock) &&
			    ((sbclock == 0) || (sbclock <= table[i].sbclock)))
				break;
		}
		if (i == tabsz) {
			ret = FALSE;
			goto done;
		} else {
			te = &table[i];
			ret = TRUE;
		}

		/* No PLL change */
		if ((orig_n == te->n) &&
		    (orig_sb == te->sb) &&
		    (orig_pci == te->pci33) &&
		    (orig_m2 == te->m2) &&
		    (orig_mips == te->m3))
			goto done;

		/* Set the PLL controls */
		W_REG(clockcontrol_n, te->n);
		W_REG(clockcontrol_sb, te->sb);
		W_REG(clockcontrol_pci, te->pci33);
		W_REG(&cc->clockcontrol_m2, te->m2);
		W_REG(&cc->clockcontrol_mips, te->m3);

		/* Set the chipcontrol bit to change mipsref to the backplane divider if needed */
		if ((pll_type == PLL_TYPE7) &&
		    (te->sb != te->m2) &&
		    (sb_clock_rate(pll_type, te->n, te->m2) == 120000000))
			W_REG(&cc->chipcontrol, R_REG(&cc->chipcontrol) | 0x100);

		/* No ratio change */
		if (orig_ratio_parm == te->ratio_parm)
			goto end_fill;

		icache_probe(MFC0(C0_CONFIG, 1), &ic_size, &ic_lsize);

		/* Preload the code into the cache */
		start = ((ulong) &&start_fill) & ~(ic_lsize - 1);
		end = ((ulong) &&end_fill + (ic_lsize - 1)) & ~(ic_lsize - 1);
		while (start < end) {
			cache_unroll(start, Fill_I);
			start += ic_lsize;
		}

		/* Copy the handler */
		start = (ulong) &BCMINIT(handler);
		end = (ulong) &BCMINIT(afterhandler);
		dst = KSEG1ADDR(0x180);
		for (i = 0; i < (end - start); i += 4)
			*((ulong *)(dst + i)) = *((ulong *)(start + i));
		
		/* Preload handler into the cache one line at a time */
		for (i = 0; i < (end - start); i += 4)
			cache_unroll(dst + i, Fill_I);

		/* Clear BEV bit */
		MTC0(C0_STATUS, 0, MFC0(C0_STATUS, 0) & ~ST0_BEV);

		/* Enable interrupts */
		MTC0(C0_STATUS, 0, MFC0(C0_STATUS, 0) | (ALLINTS | ST0_IE));

		/* Enable MIPS timer interrupt */
		if (!(mipsr = sb_setcore(sbh, SB_MIPS, 0)) &&
		    !(mipsr = sb_setcore(sbh, SB_MIPS33, 0)))
			ASSERT(mipsr);
		W_REG(&mipsr->intmask, 1);

	start_fill:
		/* step 1, set clock ratios */
		MTC0(C0_BROADCOM, 3, te->ratio_parm);
		MTC0(C0_BROADCOM, 1, te->ratio_cfg);

		/* step 2: program timer intr */
		W_REG(&mipsr->timer, 100);
		(void) R_REG(&mipsr->timer);

		/* step 3, switch to async */
		sync_mode = MFC0(C0_BROADCOM, 4);
		MTC0(C0_BROADCOM, 4, 1 << 22);

		/* step 4, set cfg active */
		MTC0(C0_BROADCOM, 2, 0x9);


		/* steps 5 & 6 */ 
		__asm__ __volatile__ (
			".set\tmips3\n\t"
			"wait\n\t"
			".set\tmips0"
		);

		/* step 7, clear cfg_active */
		MTC0(C0_BROADCOM, 2, 0);
		
		/* Additional Step: set back to orig sync mode */
		MTC0(C0_BROADCOM, 4, sync_mode);

		/* step 8, fake soft reset */
		MTC0(C0_BROADCOM, 5, MFC0(C0_BROADCOM, 5) | 4);

	end_fill:
		/* step 9 set watchdog timer */
		sb_watchdog(sbh, 20);
		(void) R_REG(&cc->chipid);

		/* step 11 */
		__asm__ __volatile__ (
			".set\tmips3\n\t"
			"sync\n\t"
			"wait\n\t"
			".set\tmips0"
		);
		while (1);
	}

done:
	/* switch back to previous core */
	sb_setcoreidx(sbh, idx);

	return ret;
}


/* returns the ncdl value to be programmed into sdram_ncdl for calibration */
uint32
BCMINITFN(sb_memc_get_ncdl)(void *sbh)
{
	sbmemcregs_t *memc;
	uint32 ret = 0;
	uint32 config, rd, wr, misc, dqsg, cd, sm, sd;
	uint idx, rev;

	idx = sb_coreidx(sbh);

	memc = (sbmemcregs_t *)sb_setcore(sbh, SB_MEMC, 0);
	if (memc == 0)
		goto out;

	rev = sb_corerev(sbh);

	config = R_REG(&memc->config);
	wr = R_REG(&memc->wrncdlcor);
	rd = R_REG(&memc->rdncdlcor);
	misc = R_REG(&memc->miscdlyctl);
	dqsg = R_REG(&memc->dqsgatencdl);

	rd &= MEMC_RDNCDLCOR_RD_MASK;
	wr &= MEMC_WRNCDLCOR_WR_MASK; 
	dqsg &= MEMC_DQSGATENCDL_G_MASK;

	if (config & MEMC_CONFIG_DDR) {
		ret = (wr << 16) | (rd << 8) | dqsg;
	} else {
		if ((rev > 0) || (sb_chip(sbh) == BCM5365_DEVICE_ID))
			cd = rd;
		else
			cd = (rd == MEMC_CD_THRESHOLD) ? rd : (wr + MEMC_CD_THRESHOLD);
		sm = (misc & MEMC_MISC_SM_MASK) >> MEMC_MISC_SM_SHIFT;
		sd = (misc & MEMC_MISC_SD_MASK) >> MEMC_MISC_SD_SHIFT;
		ret = (sm << 16) | (sd << 8) | cd;
	}

out:
	/* switch back to previous core */
	sb_setcoreidx(sbh, idx);

	return ret;
}

/* returns the PFC values to be used based on the chip ID*/

uint32
BCMINITFN(sb_mips_get_pfc)(void *sbh)
{
	if (BCMINIT(sb_chip)(sbh) == BCM5350_DEVICE_ID) 
		return 0x11;
	else 
		return 0x15;
}

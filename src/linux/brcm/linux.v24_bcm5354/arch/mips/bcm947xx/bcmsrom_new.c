/*
 *  Misc useful routines to access NIC SROM/OTP .
 *
 * Copyright 2006, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 * $Id$
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <proto/ethernet.h>	/* for sprom content groking */
#include <osl.h>
#include <bcmutils.h>
#include <sbchipc.h>
#include <bcmdevs.h>
#include <bcmendian.h>
#include <sbpcmcia.h>
#include <pcicfg.h>
#include <sbconfig.h>
#include <sbutils.h>
#include <bcmsrom_new.h>
#include <bcmnvram.h>
#ifdef BCMSDIODEV
#include <sbsdio.h>
#endif

#ifdef WLTEST
#include <sbsprom.h>
#endif /* WLTEST */

/* debug/trace */
#if defined(WLTEST)
#define	BS_ERROR(args)	printf args
#else
#define	BS_ERROR(args)
#endif	

#define	VARS_MAX	4096	/* should be reduced */

#define WRITE_ENABLE_DELAY	500	/* 500 ms after write enable/disable toggle */
#define WRITE_WORD_DELAY	20	/* 20 ms between each word write */

static int initvars_srom_pci(sb_t *sbh, void *curmap, char **vars, uint *count);
static int initvars_cis_pcmcia(sb_t *sbh, osl_t *osh, char **vars, uint *count);
static int initvars_flash_sb(sb_t *sbh, char **vars, uint *count);
#ifdef BCMSDIODEV
static int initvars_cis_sddev(osl_t *osh, sb_t *sbh, void *curmap, char **vars, uint *count);
#endif /* BCMSDIODEV */
static int sprom_cmd_pcmcia(osl_t *osh, uint8 cmd);
static int sprom_read_pcmcia(osl_t *osh, uint16 addr, uint16 *data);
static int sprom_write_pcmcia(osl_t *osh, uint16 addr, uint16 data);
static int sprom_read_pci(osl_t *osh, uint16 *sprom, uint wordoff, uint16 *buf, uint nwords,
                          bool check_crc);

static int initvars_table(osl_t *osh, char *start, char *end, char **vars, uint *count);
static int initvars_flash(osl_t *osh, char **vp, uint len, char *devpath);

#ifdef	SB_PCMCIA_SROM
static int get_sb_pcmcia_srom(osl_t *osh, uint8 *pcmregs, uint boff, uint16 *srom, uint bsz);
static int set_sb_pcmcia_srom(osl_t *osh, uint8 *pcmregs, uint boff, uint16 *srom, uint bsz);
#endif	/* SB_PCMCIA_SROM */

/*
 * Initialize local vars from the right source for this platform.
 * Return 0 on success, nonzero on error.
 */
int
BCMINITFN(srom_var_init)(sb_t *sbh, uint bustype, void *curmap, osl_t *osh,
	char **vars, uint *count)
{
//printk(KERN_EMERG "srom_var_init\n");
	ASSERT(bustype == BUSTYPE(bustype));
	if (vars == NULL || count == NULL)
		return (0);

	switch (BUSTYPE(bustype)) {
	case SB_BUS:
#ifdef BCMSDIODEV
		if ((sbh->cccaps & CC_CAP_FLASH_MASK) == FLASH_NONE) {
			return (initvars_cis_sddev(osh, sbh, curmap, vars, count));
		} /* else fall through to JTAG_BUS case, because it must have FLASH... */
#endif /* BCMSDIODEV */

	case JTAG_BUS:
		return initvars_flash_sb(sbh, vars, count);

	case PCI_BUS:
		ASSERT(curmap);	/* can not be NULL */
		return initvars_srom_pci(sbh, curmap, vars, count);

	case PCMCIA_BUS:
		return initvars_cis_pcmcia(sbh, osh, vars, count);


	default:
		ASSERT(0);
	}
	return (-1);
}

/* support only 16-bit word read from srom */
int
srom_read(sb_t *sbh, uint bustype, void *curmap, osl_t *osh,
          uint byteoff, uint nbytes, uint16 *buf)
{
	void *srom;
	uint i, off, nw;
//printk(KERN_EMERG "srom_read\n");
	ASSERT(bustype == BUSTYPE(bustype));

	/* check input - 16-bit access only */
	if (byteoff & 1 || nbytes & 1 || (byteoff + nbytes) > (SPROM_SIZE * 2))
		return 1;

	off = byteoff / 2;
	nw = nbytes / 2;

	if (BUSTYPE(bustype) == PCI_BUS) {
		if (!curmap)
			return 1;
		srom = (uchar*)curmap + PCI_BAR0_SPROM_OFFSET;
		if (sprom_read_pci(osh, srom, off, buf, nw, FALSE))
			return 1;
	} else if (BUSTYPE(bustype) == PCMCIA_BUS) {
		for (i = 0; i < nw; i++) {
			if (sprom_read_pcmcia(osh, (uint16)(off + i), (uint16 *)(buf + i)))
				return 1;
		}
	} else if (BUSTYPE(bustype) == SB_BUS) {
#ifdef SB_PCMCIA_SROM
		if (SPROMBUS == PCMCIA_BUS) {
			uint origidx = sb_coreidx(sbh);
			void *regs = sb_setcore(sbh, SB_PCMCIA, 0);
			int rc;
			ASSERT(regs != NULL);
			rc = get_sb_pcmcia_srom(osh, regs, byteoff, buf, nbytes);
			sb_setcoreidx(sbh, origidx);
			return rc;
		}
#endif	/* SB_PCMCIA_SROM */
		return 1;
	} else {
		return 1;
	}
//printk(KERN_EMERG "done\n");

	return 0;
}

/* support only 16-bit word write into srom */
int
srom_write(sb_t *sbh, uint bustype, void *curmap, osl_t *osh,
           uint byteoff, uint nbytes, uint16 *buf)
{
	uint16 *srom;
	uint i, nw, crc_range;
	uint16 image[SPROM_SIZE];
	uint8 crc;
	volatile uint32 val32;
//printk(KERN_EMERG "srom_write\n");

	ASSERT(bustype == BUSTYPE(bustype));

	/* check input - 16-bit access only */
	if ((byteoff & 1) || (nbytes & 1))
		return 1;

	if (byteoff == 0x55aa) {
		/* Erase request */
		crc_range = 0;
		memset((void *)image, 0xff, nbytes);
		nw = nbytes / 2;
	} else if ((byteoff == 0) &&
	           ((nbytes == SPROM_SIZE) ||
	            (nbytes == (SPROM_CRC_RANGE * 2)) ||
	            (nbytes == (SROM4_WORDS * 2)))) {
		/* Are we writing the whole thing at once? */
		crc_range = nbytes;
		bcopy((void *)buf, (void *)image, nbytes);
		nw = nbytes / 2;
	} else {
		if ((byteoff + nbytes) > (SPROM_SIZE * 2))
			return 1;

		if ((BUSTYPE(bustype) == PCMCIA_BUS) || (BUSTYPE(bustype) == SDIO_BUS))
			crc_range = SPROM_SIZE;
		else
			crc_range = SPROM_CRC_RANGE * 2;	/* Tentative */

		nw = crc_range / 2;
		/* read first 64 words from srom */
		if (srom_read(sbh, bustype, curmap, osh, 0, crc_range, image))
			return 1;
		if (image[SROM4_SIGN] == SROM4_SIGNATURE) {
			nw = SROM4_WORDS;
			crc_range = nw * 2;
			if (srom_read(sbh, bustype, curmap, osh, 0, crc_range, image))
				return 1;
		}
		/* make changes */
		bcopy((void *)buf, (void *)&image[byteoff / 2], nbytes);
	}

	if (crc_range) {
		/* calculate crc */
		htol16_buf(image, crc_range);
		crc = ~hndcrc8((uint8 *)image, crc_range - 1, CRC8_INIT_VALUE);
		ltoh16_buf(image, crc_range);
		image[nw - 1] = (crc << 8) | (image[nw - 1] & 0xff);
	}

	if (BUSTYPE(bustype) == PCI_BUS) {
		srom = (uint16 *)((uchar*)curmap + PCI_BAR0_SPROM_OFFSET);
		/* enable writes to the SPROM */
		val32 = OSL_PCI_READ_CONFIG(osh, PCI_SPROM_CONTROL, sizeof(uint32));
		val32 |= SPROM_WRITEEN;
		OSL_PCI_WRITE_CONFIG(osh, PCI_SPROM_CONTROL, sizeof(uint32), val32);
		bcm_mdelay(WRITE_ENABLE_DELAY);
		/* write srom */
		for (i = 0; i < nw; i++) {
			W_REG(osh, &srom[i], image[i]);
			bcm_mdelay(WRITE_WORD_DELAY);
		}
		/* disable writes to the SPROM */
		OSL_PCI_WRITE_CONFIG(osh, PCI_SPROM_CONTROL, sizeof(uint32), val32 &
		                     ~SPROM_WRITEEN);
	} else if (BUSTYPE(bustype) == PCMCIA_BUS) {
		/* enable writes to the SPROM */
		if (sprom_cmd_pcmcia(osh, SROM_WEN))
			return 1;
		bcm_mdelay(WRITE_ENABLE_DELAY);
		/* write srom */
		for (i = 0; i < nw; i++) {
			sprom_write_pcmcia(osh, (uint16)(i), image[i]);
			bcm_mdelay(WRITE_WORD_DELAY);
		}
		/* disable writes to the SPROM */
		if (sprom_cmd_pcmcia(osh, SROM_WDS))
			return 1;
	} else if (BUSTYPE(bustype) == SB_BUS) {
#ifdef SB_PCMCIA_SROM
		if (SPROMBUS == PCMCIA_BUS) {
			uint origidx = sb_coreidx(sbh);
			void *regs = sb_setcore(sbh, SB_PCMCIA, 0);
			int rc;
			ASSERT(regs != NULL);
			rc = set_sb_pcmcia_srom(osh, regs, byteoff, buf, nbytes);
			sb_setcoreidx(sbh, origidx);
			return rc;
		}
#endif	/* SB_PCMCIA_SROM */
		return 1;
	} else {
		return 1;
	}

	bcm_mdelay(WRITE_ENABLE_DELAY);
	return 0;
}

#ifdef	SB_PCMCIA_SROM
#define SB_PCMCIA_READ(regs, fcr) (*((volatile uint8 *)((regs) + 0x600 + (fcr) - 0x700 / 2)))
#define SB_PCMCIA_WRITE(regs, fcr, v) \
		(*((volatile uint8 *)((regs) + 0x600 + (fcr) - 0x700 / 2)) = (v))

/* set PCMCIA srom command register */
static int
srom_cmd_sb_pcmcia(uint8 *pcmregs, uint8 cmd)
{
	uint8 status = 0;
	uint wait_cnt = 0;

	/* write srom command register */
	SB_PCMCIA_WRITE(pcmregs, SROM_CS, cmd);

	/* wait status */
	while (++wait_cnt < 1000000) {
		status = SB_PCMCIA_READ(pcmregs, SROM_CS);
		if (status & SROM_DONE)
			return 0;
		OSL_DELAY(1);
	}

	BS_ERROR(("sr_cmd: Give up after %d tries, stat = 0x%x\n", wait_cnt, status));
	return 1;
}

/* read a word from the PCMCIA srom over SB */
static int
srom_read_sb_pcmcia(uint8 *pcmregs, uint16 addr, uint16 *data)
{
	uint8 addr_l, addr_h,  data_l, data_h;

	addr_l = (uint8)((addr * 2) & 0xff);
	addr_h = (uint8)(((addr * 2) >> 8) & 0xff);

	/* set address */
	SB_PCMCIA_WRITE(pcmregs, SROM_ADDRH, addr_h);
	SB_PCMCIA_WRITE(pcmregs, SROM_ADDRL, addr_l);

	/* do read */
	if (srom_cmd_sb_pcmcia(pcmregs, SROM_READ))
		return 1;

	/* read data */
	data_h = SB_PCMCIA_READ(pcmregs, SROM_DATAH);
	data_l = SB_PCMCIA_READ(pcmregs, SROM_DATAL);
	*data = ((uint16)data_h << 8) | data_l;

	return 0;
}

/* write a word to the PCMCIA srom */
static int
srom_write_sb_pcmcia(uint8 *pcmregs, uint16 addr, uint16 data)
{
	uint8 addr_l, addr_h, data_l, data_h;
	int rc;

	addr_l = (uint8)((addr * 2) & 0xff);
	addr_h = (uint8)(((addr * 2) >> 8) & 0xff);

	/* set address */
	SB_PCMCIA_WRITE(pcmregs, SROM_ADDRH, addr_h);
	SB_PCMCIA_WRITE(pcmregs, SROM_ADDRL, addr_l);

	data_l = (uint8)(data & 0xff);
	data_h = (uint8)((data >> 8) & 0xff);

	/* write data */
	SB_PCMCIA_WRITE(pcmregs, SROM_DATAH, data_h);
	SB_PCMCIA_WRITE(pcmregs, SROM_DATAL, data_l);

	/* do write */
	rc = srom_cmd_sb_pcmcia(pcmregs, SROM_WRITE);
	OSL_DELAY(20000);
	return rc;
}

/*
 * Read the srom for the pcmcia-srom over sb case.
 * Return 0 on success, nonzero on error.
 */
static int
get_sb_pcmcia_srom(osl_t *osh, uint8 *pcmregs,
                   uint boff, uint16 *srom, uint bsz)
{
	uint i, nw, woff, wsz;
	int err = 0;

	/* read must be at word boundary */
	ASSERT((boff & 1) == 0 && (bsz & 1) == 0);

	/* read sprom size and validate the parms */
	switch (SB_PCMCIA_READ(pcmregs, SROM_INFO) & SRI_SZ_MASK) {
	case 1:
		nw = 256;	/* SROM_INFO == 1 means 4kbit */
		break;
	case 2:
		nw = 1024;	/* SROM_INFO == 2 means 16kbit */
		break;
	default:
		BS_ERROR(("get_sb_pcmcia_srom: sprom size unknown\n"));
		err = -1;
		goto out;
	}
	if (boff + bsz > 2 * nw) {
		BS_ERROR(("get_sb_pcmcia_srom: sprom size exceeded\n"));
		err = -2;
		goto out;
	}

	/* read in sprom contents */
	for (woff = boff / 2, wsz = bsz / 2, i = 0;
	     woff < nw && i < wsz; woff ++, i ++) {
		if (srom_read_sb_pcmcia(pcmregs, woff, &srom[i])) {
			BS_ERROR(("get_sb_pcmcia_srom: sprom read failed\n"));
			err = -3;
			goto out;
		}
	}

out:
	return err;
}

/*
 * Write the srom for the pcmcia-srom over sb case.
 * Return 0 on success, nonzero on error.
 */
static int
set_sb_pcmcia_srom(osl_t *osh, uint8 *pcmregs, uint boff, uint16 *srom, uint bsz)
{
	uint i, nw, woff, wsz;
	uint16 word;
	uint8 crc;
	int err = 0;

	/* write must be at word boundary */
	ASSERT((boff & 1) == 0 && (bsz & 1) == 0);
	/* read sprom size and validate the parms */
	switch (SB_PCMCIA_READ(pcmregs, SROM_INFO) & SRI_SZ_MASK) {
	case 1:
		nw = 256;	/* SROM_INFO == 1 means 4kbit */
		break;
	case 2:
		nw = 1024;	/* SROM_INFO == 2 means 16kbit */
		break;
	default:
		BS_ERROR(("set_sb_pcmcia_srom: sprom size unknown\n"));
		err = -1;
		goto out;
	}
	if (boff + bsz > 2 * nw) {
		BS_ERROR(("set_sb_pcmcia_srom: sprom size exceeded\n"));
		err = -2;
		goto out;
	}
	/* enable write */
	if (srom_cmd_sb_pcmcia(pcmregs, SROM_WEN)) {
		BS_ERROR(("set_sb_pcmcia_srom: sprom wen failed\n"));
		err = -3;
		goto out;
	}
	/* write buffer to sprom */
	for (woff = boff / 2, wsz = bsz / 2, i = 0;
	     woff < nw && i < wsz; woff ++, i ++) {
		if (srom_write_sb_pcmcia(pcmregs, woff, srom[i])) {
			BS_ERROR(("set_sb_pcmcia_srom: sprom write failed\n"));
			err = -4;
			goto out;
		}
	}
	/* fix crc */
	crc = CRC8_INIT_VALUE;
	for (woff = 0; woff < nw; woff ++) {
		if (srom_read_sb_pcmcia(pcmregs, woff, &word)) {
			BS_ERROR(("set_sb_pcmcia_srom: sprom fix crc read failed\n"));
			err = -5;
			goto out;
		}
		word = htol16(word);
		crc = hndcrc8((uint8 *)&word, woff != nw - 1 ? 2 : 1, crc);
	}
	word = (~crc << 8) + (ltoh16(word) & 0xff);
	if (srom_write_sb_pcmcia(pcmregs, woff - 1, word)) {
		BS_ERROR(("set_sb_pcmcia_srom: sprom fix crc write failed\n"));
		err = -6;
		goto out;
	}
	/* disable write */
	if (srom_cmd_sb_pcmcia(pcmregs, SROM_WDS)) {
		BS_ERROR(("set_sb_pcmcia_srom: sprom wds failed\n"));
		err = -7;
		goto out;
	}

out:
	return err;
}
#endif	/* SB_PCMCIA_SROM */

int
srom_parsecis(osl_t *osh, uint8 **pcis, uint ciscnt, char **vars, uint *count)
{
	char eabuf[32];
	char *vp, *base;
	uint8 *cis, tup, tlen, sromrev = 1;
	int i, j;
	uint varsize;
	bool ag_init = FALSE;
	uint32 w32;
	uint funcid;
	uint cisnum;

	ASSERT(vars);
	ASSERT(count);

	base = vp = MALLOC(osh, VARS_MAX);
	ASSERT(vp);
	if (!vp)
		return -2;

	for (cisnum = 0; cisnum < ciscnt; cisnum++) {
		cis = *pcis++;
		i = 0;
		funcid = 0;
		do {
			tup = cis[i++];
			tlen = cis[i++];
			if ((i + tlen) >= CIS_SIZE)
				break;

			switch (tup) {
			case CISTPL_VERS_1:
				/* assume the strings are good if the version field checks out */
				if (((cis[i + 1] << 8) + cis[i]) >= 0x0008) {
					vp += sprintf(vp, "manf=%s", &cis[i + 2]);
					vp++;
					vp += sprintf(vp, "productname=%s",
					              &cis[i + 3 + strlen((char *)&cis[i + 2])]);
					vp++;
					break;
				}

			case CISTPL_MANFID:
				vp += sprintf(vp, "manfid=0x%x", (cis[i + 1] << 8) + cis[i]);
				vp++;
				vp += sprintf(vp, "prodid=0x%x", (cis[i + 3] << 8) + cis[i + 2]);
				vp++;
				break;

			case CISTPL_FUNCID:
				funcid = cis[i];
				break;

			case CISTPL_FUNCE:
				switch (funcid) {
				default:
					if (cis[i] == LAN_NID) {
						ASSERT(cis[i + 1] == ETHER_ADDR_LEN);
						vp += sprintf(vp, "boardnum=%d",
						              (cis[i + 6] << 8) + cis[i + 7]);
						vp++;
						bcm_ether_ntoa((struct ether_addr *)&cis[i + 2],
						               eabuf);
						vp += sprintf(vp, "macaddr=%s", eabuf);
						vp++;
						break;
					}
					break;
				}
				break;

			case CISTPL_CFTABLE:
				vp += sprintf(vp, "regwindowsz=%d", (cis[i + 7] << 8) | cis[i + 6]);
				vp++;
				break;

			case CISTPL_BRCM_HNBU:
				switch (cis[i]) {
				case HNBU_SROMREV:
					sromrev = cis[i + 1];
					break;

				case HNBU_CHIPID:
					vp += sprintf(vp, "vendid=0x%x", (cis[i + 2] << 8) +
					              cis[i + 1]);
					vp++;
					vp += sprintf(vp, "devid=0x%x", (cis[i + 4] << 8) +
					              cis[i + 3]);
					vp++;
					if (tlen == 7) {
						vp += sprintf(vp, "chiprev=%d",
						              (cis[i + 6] << 8) + cis[i + 5]);
						vp++;
					}
					break;

				case HNBU_BOARDREV:
					vp += sprintf(vp, "boardrev=%d", cis[i + 1]);
					vp++;
					break;

				case HNBU_AA:
					vp += sprintf(vp, "aa2g=%d", cis[i + 1]);
					vp++;
					break;

				case HNBU_AG:
					vp += sprintf(vp, "ag0=%d", cis[i + 1]);
					vp++;
					ag_init = TRUE;
					break;

				case HNBU_ANT5G:
					vp += sprintf(vp, "aa5g=%d", cis[i + 1]);
					vp++;
					vp += sprintf(vp, "ag1=%d", cis[i + 2]);
					vp++;
					break;

				case HNBU_CC:
					ASSERT(sromrev == 1);
					vp += sprintf(vp, "cc=%d", cis[i + 1]);
					vp++;
					break;

				case HNBU_PAPARMS:
					if (tlen == 2) {
						ASSERT(sromrev == 1);
						vp += sprintf(vp, "pa0maxpwr=%d", cis[i + 1]);
						vp++;
					} else if (tlen >= 9) {
						if (tlen == 10) {
							ASSERT(sromrev == 2 || sromrev == 3);
							vp += sprintf(vp, "opo=%d", cis[i + 9]);
							vp++;
						} else
							ASSERT(tlen == 9);

						for (j = 0; j < 3; j++) {
							vp += sprintf(vp, "pa0b%d=%d", j,
							              (cis[i + (j * 2) + 2] << 8) +
							              cis[i + (j * 2) + 1]);
							vp++;
						}
						vp += sprintf(vp, "pa0itssit=%d", cis[i + 7]);
						vp++;
						vp += sprintf(vp, "pa0maxpwr=%d", cis[i + 8]);
						vp++;
					} else
						ASSERT(tlen >= 9);
					break;

				case HNBU_PAPARMS5G:
					ASSERT(sromrev >= 2);
					for (j = 0; j < 3; j++) {
						vp += sprintf(vp, "pa1b%d=%d", j,
							(cis[i + (j * 2) + 2] << 8) +
							cis[i + (j * 2) + 1]);
						vp++;
					}
					for (j = 3; j < 6; j++) {
						vp += sprintf(vp, "pa1lob%d=%d", j - 3,
							(cis[i + (j * 2) + 2] << 8) +
							cis[i + (j * 2) + 1]);
						vp++;
					}
					for (j = 6; j < 9; j++) {
						vp += sprintf(vp, "pa1hib%d=%d", j - 6,
							(cis[i + (j * 2) + 2] << 8) +
							cis[i + (j * 2) + 1]);
						vp++;
					}
					vp += sprintf(vp, "pa1itssit=%d", cis[i + 19]);
					vp++;
					vp += sprintf(vp, "pa1maxpwr=%d", cis[i + 20]);
					vp++;
					vp += sprintf(vp, "pa1lomaxpwr=%d", cis[i + 21]);
					vp++;
					vp += sprintf(vp, "pa1himaxpwr=%d", cis[i + 22]);
					vp++;
					break;

				case HNBU_OEM:
					ASSERT(sromrev == 1);
					vp += sprintf(vp, "oem=%02x%02x%02x%02x%02x%02x%02x%02x",
					              cis[i + 1], cis[i + 2],
					              cis[i + 3], cis[i + 4],
					              cis[i + 5], cis[i + 6],
					              cis[i + 7], cis[i + 8]);
					vp++;
					break;

				case HNBU_BOARDFLAGS:
					w32 = (cis[i + 2] << 8) + cis[i + 1];
					if (tlen == 5)
						w32 |= (cis[i + 4] << 24) + (cis[i + 3] << 16);
					vp += sprintf(vp, "boardflags=0x%x", w32);
					vp++;
					break;

				case HNBU_LEDS:
					if (cis[i + 1] != 0xff) {
						vp += sprintf(vp, "ledbh0=%d", cis[i + 1]);
						vp++;
					}
					if (cis[i + 2] != 0xff) {
						vp += sprintf(vp, "ledbh1=%d", cis[i + 2]);
						vp++;
					}
					if (cis[i + 3] != 0xff) {
						vp += sprintf(vp, "ledbh2=%d", cis[i + 3]);
						vp++;
					}
					if (cis[i + 4] != 0xff) {
						vp += sprintf(vp, "ledbh3=%d", cis[i + 4]);
						vp++;
					}
					break;

				case HNBU_CCODE:
					ASSERT(sromrev > 1);
					if ((cis[i + 1] == 0) || (cis[i + 2] == 0))
						vp += sprintf(vp, "ccode=");
					else
						vp += sprintf(vp, "ccode=%c%c",
						              cis[i + 1], cis[i + 2]);
					vp++;
					vp += sprintf(vp, "cctl=0x%x", cis[i + 3]);
					vp++;
					break;

				case HNBU_CCKPO:
					ASSERT(sromrev > 2);
					vp += sprintf(vp, "cckpo=0x%x",
					              (cis[i + 2] << 8) | cis[i + 1]);
					vp++;
					break;

				case HNBU_OFDMPO:
					ASSERT(sromrev > 2);
					vp += sprintf(vp, "ofdmpo=0x%x",
					              (cis[i + 4] << 24) |
					              (cis[i + 3] << 16) |
					              (cis[i + 2] << 8) |
					              cis[i + 1]);
					vp++;
					break;

				case HNBU_RDLID:
					vp += sprintf(vp, "rdlid=0x%x",
					              (cis[i + 2] << 8) | cis[i + 1]);
					vp++;
					break;

				case HNBU_RDLRNDIS:
					vp += sprintf(vp, "rdlrndis=%d", cis[i + 1]);
					vp++;
					break;

				case HNBU_RDLRWU:
					vp += sprintf(vp, "rdlrwu=%d", cis[i + 1]);
					vp++;
					break;

				case HNBU_XTALFREQ:
					vp += sprintf(vp, "xtalfreq=%d",
					              (cis[i + 4] << 24) |
					              (cis[i + 3] << 16) |
					              (cis[i + 2] << 8) |
					              cis[i + 1]);
					vp++;
					break;

				case HNBU_RSSISMBXA2G:
					ASSERT(sromrev == 3);
					vp += sprintf(vp, "rssismf2g=%d", cis[i + 1] & 0xf);
					vp++;
					vp += sprintf(vp, "rssismc2g=%d", (cis[i + 1] >> 4) & 0xf);
					vp++;
					vp += sprintf(vp, "rssisav2g=%d", cis[i + 2] & 0x7);
					vp++;
					vp += sprintf(vp, "bxa2g=%d", (cis[i + 2] >> 3) & 0x3);
					vp++;
					break;

				case HNBU_RSSISMBXA5G:
					ASSERT(sromrev == 3);
					vp += sprintf(vp, "rssismf5g=%d", cis[i + 1] & 0xf);
					vp++;
					vp += sprintf(vp, "rssismc5g=%d", (cis[i + 1] >> 4) & 0xf);
					vp++;
					vp += sprintf(vp, "rssisav5g=%d", cis[i + 2] & 0x7);
					vp++;
					vp += sprintf(vp, "bxa5g=%d", (cis[i + 2] >> 3) & 0x3);
					vp++;
					break;

				case HNBU_TRI2G:
					ASSERT(sromrev == 3);
					vp += sprintf(vp, "tri2g=%d", cis[i + 1]);
					vp++;
					break;

				case HNBU_TRI5G:
					ASSERT(sromrev == 3);
					vp += sprintf(vp, "tri5gl=%d", cis[i + 1]);
					vp++;
					vp += sprintf(vp, "tri5g=%d", cis[i + 2]);
					vp++;
					vp += sprintf(vp, "tri5gh=%d", cis[i + 3]);
					vp++;
					break;

				case HNBU_RXPO2G:
					ASSERT(sromrev == 3);
					vp += sprintf(vp, "rxpo2g=%d", cis[i + 1]);
					vp++;
					break;

				case HNBU_RXPO5G:
					ASSERT(sromrev == 3);
					vp += sprintf(vp, "rxpo5g=%d", cis[i + 1]);
					vp++;
					break;
				}
				break;
			}
			i += tlen;
		} while (tup != 0xff);
	}

	/* Set the srom version */
	vp += sprintf(vp, "sromrev=%d", sromrev);
	vp++;

	/* if there is no antenna gain field, set default */
	if (ag_init == FALSE) {
		vp += sprintf(vp, "ag0=%d", 0xff);
		vp++;
	}

	/* final nullbyte terminator */
	*vp++ = '\0';
	varsize = (uint)(vp - base);

	ASSERT((vp - base) < VARS_MAX);

	if (varsize == VARS_MAX) {
		*vars = base;
	} else {
		vp = MALLOC(osh, varsize);
		ASSERT(vp);
		if (vp)
			bcopy(base, vp, varsize);
		MFREE(osh, base, VARS_MAX);
		*vars = vp;
		if (!vp) {
			*count = 0;
			return -2;
		}
	}
	*count = varsize;

	return (0);
}


/* set PCMCIA sprom command register */
static int
sprom_cmd_pcmcia(osl_t *osh, uint8 cmd)
{
	uint8 status = 0;
	uint wait_cnt = 1000;

	/* write sprom command register */
	OSL_PCMCIA_WRITE_ATTR(osh, SROM_CS, &cmd, 1);

	/* wait status */
	while (wait_cnt--) {
		OSL_PCMCIA_READ_ATTR(osh, SROM_CS, &status, 1);
		if (status & SROM_DONE)
			return 0;
	}

	return 1;
}

/* read a word from the PCMCIA srom */
static int
sprom_read_pcmcia(osl_t *osh, uint16 addr, uint16 *data)
{
	uint8 addr_l, addr_h, data_l, data_h;

	addr_l = (uint8)((addr * 2) & 0xff);
	addr_h = (uint8)(((addr * 2) >> 8) & 0xff);

	/* set address */
	OSL_PCMCIA_WRITE_ATTR(osh, SROM_ADDRH, &addr_h, 1);
	OSL_PCMCIA_WRITE_ATTR(osh, SROM_ADDRL, &addr_l, 1);

	/* do read */
	if (sprom_cmd_pcmcia(osh, SROM_READ))
		return 1;

	/* read data */
	data_h = data_l = 0;
	OSL_PCMCIA_READ_ATTR(osh, SROM_DATAH, &data_h, 1);
	OSL_PCMCIA_READ_ATTR(osh, SROM_DATAL, &data_l, 1);

	*data = (data_h << 8) | data_l;
	return 0;
}

/* write a word to the PCMCIA srom */
static int
sprom_write_pcmcia(osl_t *osh, uint16 addr, uint16 data)
{
	uint8 addr_l, addr_h, data_l, data_h;

	addr_l = (uint8)((addr * 2) & 0xff);
	addr_h = (uint8)(((addr * 2) >> 8) & 0xff);
	data_l = (uint8)(data & 0xff);
	data_h = (uint8)((data >> 8) & 0xff);

	/* set address */
	OSL_PCMCIA_WRITE_ATTR(osh, SROM_ADDRH, &addr_h, 1);
	OSL_PCMCIA_WRITE_ATTR(osh, SROM_ADDRL, &addr_l, 1);

	/* write data */
	OSL_PCMCIA_WRITE_ATTR(osh, SROM_DATAH, &data_h, 1);
	OSL_PCMCIA_WRITE_ATTR(osh, SROM_DATAL, &data_l, 1);

	/* do write */
	return sprom_cmd_pcmcia(osh, SROM_WRITE);
}

/*
 * Read in and validate sprom.
 * Return 0 on success, nonzero on error.
 */
static int
sprom_read_pci(osl_t *osh, uint16 *sprom, uint wordoff, uint16 *buf, uint nwords, bool check_crc)
{
	int err = 0;
	uint i;

	/* read the sprom */
	for (i = 0; i < nwords; i++) {
#ifdef BCMQT
		buf[i] = R_REG(osh, &sprom[wordoff + i]);
#endif
		buf[i] = R_REG(osh, &sprom[wordoff + i]);
	}

	if (check_crc) {
		if (buf[0] == 0xffff) {
			/* The hardware thinks that an srom that starts with 0xffff
			 * is blank, regardless of the rest of the content, so declare
			 * it bad.
			 */
			BS_ERROR(("%s: buf[0] = 0x%x, returning bad-crc\n", __FUNCTION__, buf[0]));
			return 1;
		}

		/* fixup the endianness so crc8 will pass */
		htol16_buf(buf, nwords * 2);
		if (hndcrc8((uint8 *)buf, nwords * 2, CRC8_INIT_VALUE) != CRC8_GOOD_VALUE)
			err = 1;
		/* now correct the endianness of the byte array */
		ltoh16_buf(buf, nwords * 2);
	}

	return err;
}

/*
* Create variable table from memory.
* Return 0 on success, nonzero on error.
*/
static int
BCMINITFN(initvars_table)(osl_t *osh, char *start, char *end, char **vars, uint *count)
{
	int c = (int)(end - start);

	/* do it only when there is more than just the null string */
	if (c > 1) {
		char *vp = MALLOC(osh, c);
		ASSERT(vp);
		if (!vp)
			return BCME_NOMEM;
		bcopy(start, vp, c);
		*vars = vp;
		*count = c;
	}
	else {
		*vars = NULL;
		*count = 0;
	}

	return 0;
}

/*
 * Find variables with <devpath> from flash. 'base' points to the beginning
 * of the table upon enter and to the end of the table upon exit when success.
 * Return 0 on success, nonzero on error.
 */
static int
BCMINITFN(initvars_flash)(osl_t *osh, char **base, uint len, char *devpath)
{
	char *vp = *base;
	char *flash;
	int err;
	char *s;
	uint l, dl, copy_len;

	/* allocate memory and read in flash */
	if (!(flash = MALLOC(osh, NVRAM_SPACE)))
		return BCME_NOMEM;
	if ((err = nvram_getall(flash, NVRAM_SPACE)))
		goto exit;

	/* grab vars with the <devpath> prefix in name */
	dl = strlen(devpath);
	for (s = flash; s && *s; s += l + 1) {
		l = strlen(s);

		/* skip non-matching variable */
		if (strncmp(s, devpath, dl))
			continue;

		/* is there enough room to copy? */
		copy_len = l - dl + 1;
		if (len < copy_len) {
			err = BCME_BUFTOOSHORT;
			goto exit;
		}

		/* no prefix, just the name=value */
		strcpy(vp, &s[dl]);
		vp += copy_len;
		len -= copy_len;
	}

	/* add null string as terminator */
	if (len < 1) {
		err = BCME_BUFTOOSHORT;
		goto exit;
	}
	*vp++ = '\0';

	*base = vp;

exit:	MFREE(osh, flash, NVRAM_SPACE);
	return err;
}

/*
 * Initialize nonvolatile variable table from flash.
 * Return 0 on success, nonzero on error.
 */
static int
BCMINITFN(initvars_flash_sb)(sb_t *sbh, char **vars, uint *count)
{
	osl_t *osh = sb_osh(sbh);
	char devpath[SB_DEVPATH_BUFSZ];
	char *vp, *base;
	int err;

	ASSERT(vars);
	ASSERT(count);

	if ((err = sb_devpath(sbh, devpath, sizeof(devpath))))
		return err;

	base = vp = MALLOC(osh, VARS_MAX);
	ASSERT(vp);
	if (!vp)
		return BCME_NOMEM;

	if ((err = initvars_flash(osh, &vp, VARS_MAX, devpath)))
		goto err;

	err = initvars_table(osh, base, vp, vars, count);

err:	MFREE(osh, base, VARS_MAX);
	return err;
}

#ifdef WLTEST
char mfgsromvars[256];
char *defaultsromvars = "il0macaddr=00:11:22:33:44:51\0"
		"et0macaddr=00:11:22:33:44:52\0"
		"et1macaddr=00:11:22:33:44:53\0"
		"boardtype=0xffff\0"
		"boardrev=0x10\0"
		"boardflags=8\0"
		"sromrev=2\0"
		"aa2g=3";
#define	MFGSROM_DEFVARSLEN	147 /* default srom len */
#endif /* WL_TEST */

/*
 * Initialize nonvolatile variable table from sprom.
 * Return 0 on success, nonzero on error.
 */
static int
BCMINITFN(initvars_srom_pci)(sb_t *sbh, void *curmap, char **vars, uint *count)
{
	uint16 w, *b;
	uint8 sromrev = 0;
	struct ether_addr ea;
	char eabuf[32];
	uint32 w32;
	int woff, i;
	char *vp, *base;
	osl_t *osh = sb_osh(sbh);
	bool flash = FALSE;
	char name[SB_DEVPATH_BUFSZ+16], *value;
	char devpath[SB_DEVPATH_BUFSZ];
	int err;

	/*
	 * Apply CRC over SROM content regardless SROM is present or not,
	 * and use variable <devpath>sromrev's existance in flash to decide
	 * if we should return an error when CRC fails or read SROM variables
	 * from flash.
	 */
	b = MALLOC(osh, SROM_MAX);
	ASSERT(b);
	if (!b)
		return -2;

	err = sprom_read_pci(osh, (void *)((int8 *)curmap + PCI_BAR0_SPROM_OFFSET), 0, b,
	                     64, TRUE);

	if (b[SROM4_SIGN] == SROM4_SIGNATURE) {
		/* sromrev >= 4, read more */
		err = sprom_read_pci(osh, (void *)((int8 *)curmap + PCI_BAR0_SPROM_OFFSET), 0, b,
		                     SROM4_WORDS, TRUE);
		sromrev = b[SROM4_WORDS - 1] & 0xff;
	} else if (err == 0) {
		/* srom is good and is rev < 4 */
		/* top word of sprom contains version and crc8 */
		sromrev = b[63] & 0xff;
		/* bcm4401 sroms misprogrammed */
		if (sromrev == 0x10)
			sromrev = 1;
	}

	if (err) {
#ifdef WLTEST
		BS_ERROR(("SROM Crc Error, so see if we could use a default\n"));
		w32 = OSL_PCI_READ_CONFIG(osh, PCI_SPROM_CONTROL, sizeof(uint32));
		if (w32 & SPROM_OTPIN_USE) {
			BS_ERROR(("srom crc failed with OTP, use default vars....\n"));
			vp = base =  mfgsromvars;
			if (sb_chip(sbh) == BCM4311_CHIP_ID) {
				BS_ERROR(("setting the devid to be 4311\n"));
				vp += sprintf(vp, "devid=0x4311");
				vp++;
			}
			bcopy(defaultsromvars, 	vp, MFGSROM_DEFVARSLEN);
			vp += MFGSROM_DEFVARSLEN;
			goto varsdone;
		} else {
#endif /* WLTEST */
			BS_ERROR(("srom crc failed with SPROM....\n"));
			sb_devpath(sbh, devpath, sizeof(devpath));
			sprintf(name, "%ssromrev", devpath);
			if (!(value = getvar(NULL, name)))
				return (-1);
			sromrev = (uint8)bcm_strtoul(value, NULL, 0);
			flash = TRUE;
#ifdef WLTEST
		}
#endif /* WLTEST */
	}

	/* srom version check */
	if (sromrev > 4)
		return (-2);

	ASSERT(vars);
	ASSERT(count);

	base = vp = MALLOC(osh, VARS_MAX);
	ASSERT(vp);
	if (!vp)
		return -2;

	/* read variables from flash */
	if (flash) {
		if ((err = initvars_flash(osh, &vp, VARS_MAX, devpath)))
			goto err;
		goto varsdone;
	}

	vp += sprintf(vp, "sromrev=%d", sromrev);
	vp++;

	if (sromrev >= 4) {
		uint path, pathbase;
		const uint pathbases[MAX_PATH] = {SROM4_PATH0, SROM4_PATH1,
		                                  SROM4_PATH2, SROM4_PATH3};

		vp += sprintf(vp, "boardrev=%d", b[SROM4_BREV]);
		vp++;

		vp += sprintf(vp, "boardflags=%d", (b[SROM4_BFL1] << 16) | b[SROM4_BFL0]);
		vp++;

		vp += sprintf(vp, "boardflags2=%d", (b[SROM4_BFL3] << 16) | b[SROM4_BFL2]);
		vp++;

		/* The macaddr */
		ea.octet[0] = (b[SROM4_MACHI] >> 8) & 0xff;
		ea.octet[1] = b[SROM4_MACHI] & 0xff;
		ea.octet[2] = (b[SROM4_MACMID] >> 8) & 0xff;
		ea.octet[3] = b[SROM4_MACMID] & 0xff;
		ea.octet[4] = (b[SROM4_MACLO] >> 8) & 0xff;
		ea.octet[5] = b[SROM4_MACLO] & 0xff;
		bcm_ether_ntoa(&ea, eabuf);
		vp += sprintf(vp, "macaddr=%s", eabuf);
		vp++;

		w = b[SROM4_CCODE];
		if (w == 0)
			vp += sprintf(vp, "ccode=");
		else
			vp += sprintf(vp, "ccode=%c%c", (w >> 8), (w & 0xff));
		vp++;
		vp += sprintf(vp, "regrev=%d", b[SROM4_REGREV]);
		vp++;

		w = b[SROM4_LEDBH10];
		if ((w != 0) && (w != 0xffff)) {
			/* ledbh0 */
			vp += sprintf(vp, "ledbh0=%d", (w & 0xff));
			vp++;

			/* ledbh1 */
			vp += sprintf(vp, "ledbh1=%d", (w >> 8) & 0xff);
			vp++;
		}
		w = b[SROM4_LEDBH32];
		if ((w != 0) && (w != 0xffff)) {
			/* ledbh2 */
			vp += sprintf(vp, "ledbh2=%d", w & 0xff);
			vp++;

			/* ledbh3 */
			vp += sprintf(vp, "ledbh3=%d", (w >> 8) & 0xff);
			vp++;
		}
		/* LED Powersave duty cycle (oncount >> 24) (offcount >> 8) */
		w = b[SROM4_LEDDC];
		if (w != 0xffff) {
			w32 = ((uint32)((unsigned char)(w >> 8) & 0xff) << 24) |  /* oncount */
			        ((uint32)((unsigned char)(w & 0xff)) << 8); /* offcount */
			vp += sprintf(vp, "leddc=%d", w32);
			vp++;
		}

		w = b[SROM4_AA];
		vp += sprintf(vp, "aa2g=%d", w & SROM4_AA2G_MASK);
		vp++;
		vp += sprintf(vp, "aa5g=%d", w >> SROM4_AA5G_SHIFT);
		vp++;

		w = b[SROM4_AG10];
		vp += sprintf(vp, "ag0=%d", w & 0xff);
		vp++;
		vp += sprintf(vp, "ag1=%d", (w >> 8) & 0xff);
		vp++;
		w = b[SROM4_AG32];
		vp += sprintf(vp, "ag2=%d", w & 0xff);
		vp++;
		vp += sprintf(vp, "ag3=%d", (w >> 8) & 0xff);
		vp++;

		/* Fixed power indices when power control is disabled */
		for (i = 0; i < 2; i++) {
			w = b[SROM4_TXPID2G + i];
			vp += sprintf(vp, "txpid2ga%d=%d", 2 * i, w & 0xff);
			vp++;
			vp += sprintf(vp, "txpid2ga%d=%d", (2 * i) + 1, (w >> 8) & 0xff);
			vp++;
			w = b[SROM4_TXPID5G + i];
			vp += sprintf(vp, "txpid5ga%d=%d", 2 * i, w & 0xff);
			vp++;
			vp += sprintf(vp, "txpid5ga%d=%d", (2 * i) + 1, (w >> 8) & 0xff);
			vp++;
			w = b[SROM4_TXPID5GL + i];
			vp += sprintf(vp, "txpid5gla%d=%d", 2 * i, w & 0xff);
			vp++;
			vp += sprintf(vp, "txpid5gla%d=%d", (2 * i) + 1, (w >> 8) & 0xff);
			vp++;
			w = b[SROM4_TXPID5GH + i];
			vp += sprintf(vp, "txpid5gha%d=%d", 2 * i, w & 0xff);
			vp++;
			vp += sprintf(vp, "txpid5gha%d=%d", (2 * i) + 1, (w >> 8) & 0xff);
			vp++;
		}

		/* Per path variables */
		for (path = 0; path < MAX_PATH; path++) {
			pathbase = pathbases[path];
			w = b[pathbase + SROM4_2G_ITT_MAXP];
			vp += sprintf(vp, "itt2ga%d=%d", path, w >> B2G_ITT_SHIFT);
			vp++;
			vp += sprintf(vp, "maxp2ga%d=%d", path, w & B2G_MAXP_MASK);
			vp++;

			for (i = 0; i < 4; i++) {
				vp += sprintf(vp, "pa2gw%da%d=%d", i, path,
				              b[pathbase + SROM4_2G_PA + i]);
				vp++;
			}

			w = b[pathbase + SROM4_5G_ITT_MAXP];
			vp += sprintf(vp, "itt5ga%d=%d", path, w >> B5G_ITT_SHIFT);
			vp++;
			vp += sprintf(vp, "maxp5ga%d=%d", path, w & B5G_MAXP_MASK);
			vp++;

			w = b[pathbase + SROM4_5GLH_MAXP];
			vp += sprintf(vp, "maxp5gla%d=%d", path, w >> B5GL_MAXP_SHIFT);
			vp++;
			vp += sprintf(vp, "maxp5gha%d=%d", path, w & B5GH_MAXP_MASK);
			vp++;

			for (i = 0; i < 4; i++) {
				vp += sprintf(vp, "pa5gw%da%d=%d", i, path,
				              b[pathbase + SROM4_5G_PA + i]);
				vp++;
				vp += sprintf(vp, "pa5glw%da%d=%d", i, path,
				              b[pathbase + SROM4_5GL_PA + i]);
				vp++;
				vp += sprintf(vp, "pa5ghw%da%d=%d", i, path,
				              b[pathbase + SROM4_5GH_PA + i]);
				vp++;
			}
		}

		vp += sprintf(vp, "cck2gpo=%d", b[SROM4_2G_CCKPO]);
		vp++;

		w32 = ((uint32)b[SROM4_2G_OFDMPO + 1] << 16) | b[SROM4_2G_OFDMPO];
		vp += sprintf(vp, "ofdm2gpo=%d", w32);
		vp++;

		w32 = ((uint32)b[SROM4_5G_OFDMPO + 1] << 16) | b[SROM4_5G_OFDMPO];
		vp += sprintf(vp, "ofdm5gpo=%d", w32);
		vp++;

		w32 = ((uint32)b[SROM4_5GL_OFDMPO + 1] << 16) | b[SROM4_5GL_OFDMPO];
		vp += sprintf(vp, "ofdm5glpo=%d", w32);
		vp++;

		w32 = ((uint32)b[SROM4_5GH_OFDMPO + 1] << 16) | b[SROM4_5GH_OFDMPO];
		vp += sprintf(vp, "ofdm5ghpo=%d", w32);
		vp++;

		for (i = 0; i < 8; i++) {
			vp += sprintf(vp, "mcs2gpo%d=%d", i, b[SROM4_2G_MCSPO + i]);
			vp++;
			vp += sprintf(vp, "mcs5gpo%d=%d", i, b[SROM4_5G_MCSPO + i]);
			vp++;
			vp += sprintf(vp, "mcs5glpo%d=%d", i, b[SROM4_5GL_MCSPO + i]);
			vp++;
			vp += sprintf(vp, "mcs5ghpo%d=%d", i, b[SROM4_5GH_MCSPO + i]);
			vp++;
		}

		vp += sprintf(vp, "cddpo=%d", b[SROM4_CDDPO]);
		vp++;
		vp += sprintf(vp, "stbcpo=%d", b[SROM4_STBCPO]);
		vp++;
		vp += sprintf(vp, "bw40po=%d", b[SROM4_BW40PO]);
		vp++;
		vp += sprintf(vp, "bwduppo=%d", b[SROM4_BWDUPPO]);
		vp++;

		goto done;
	}
	if (sromrev >= 3) {
		/* New section takes over the 3th hardware function space */

		/* Words 22+23 are 11a (mid) ofdm power offsets */
		w32 = ((uint32)b[23] << 16) | b[22];
		vp += sprintf(vp, "ofdmapo=%d", w32);
		vp++;

		/* Words 24+25 are 11a (low) ofdm power offsets */
		w32 = ((uint32)b[25] << 16) | b[24];
		vp += sprintf(vp, "ofdmalpo=%d", w32);
		vp++;

		/* Words 26+27 are 11a (high) ofdm power offsets */
		w32 = ((uint32)b[27] << 16) | b[26];
		vp += sprintf(vp, "ofdmahpo=%d", w32);
		vp++;

		/* LED Powersave duty cycle (oncount >> 24) (offcount >> 8) */
		w32 = ((uint32)((unsigned char)(b[21] >> 8) & 0xff) << 24) |  /* oncount */
			((uint32)((unsigned char)(b[21] & 0xff)) << 8); /* offcount */
		vp += sprintf(vp, "leddc=%d", w32);

		vp++;
	}

	if (sromrev >= 2) {
		/* New section takes over the 4th hardware function space */

		/* Word 29 is max power 11a high/low */
		w = b[29];
		vp += sprintf(vp, "pa1himaxpwr=%d", w & 0xff);
		vp++;
		vp += sprintf(vp, "pa1lomaxpwr=%d", (w >> 8) & 0xff);
		vp++;

		/* Words 30-32 set the 11alow pa settings,
		 * 33-35 are the 11ahigh ones.
		 */
		for (i = 0; i < 3; i++) {
			vp += sprintf(vp, "pa1lob%d=%d", i, b[30 + i]);
			vp++;
			vp += sprintf(vp, "pa1hib%d=%d", i, b[33 + i]);
			vp++;
		}
		w = b[59];
		if (w == 0)
			vp += sprintf(vp, "ccode=");
		else
			vp += sprintf(vp, "ccode=%c%c", (w >> 8), (w & 0xff));
		vp++;

	}

	/* parameter section of sprom starts at byte offset 72 */
	woff = 72/2;

	/* first 6 bytes are il0macaddr */
	ea.octet[0] = (b[woff] >> 8) & 0xff;
	ea.octet[1] = b[woff] & 0xff;
	ea.octet[2] = (b[woff+1] >> 8) & 0xff;
	ea.octet[3] = b[woff+1] & 0xff;
	ea.octet[4] = (b[woff+2] >> 8) & 0xff;
	ea.octet[5] = b[woff+2] & 0xff;
	woff += ETHER_ADDR_LEN/2;
	bcm_ether_ntoa(&ea, eabuf);
	vp += sprintf(vp, "il0macaddr=%s", eabuf);
	vp++;

	/* next 6 bytes are et0macaddr */
	ea.octet[0] = (b[woff] >> 8) & 0xff;
	ea.octet[1] = b[woff] & 0xff;
	ea.octet[2] = (b[woff+1] >> 8) & 0xff;
	ea.octet[3] = b[woff+1] & 0xff;
	ea.octet[4] = (b[woff+2] >> 8) & 0xff;
	ea.octet[5] = b[woff+2] & 0xff;
	woff += ETHER_ADDR_LEN/2;
	bcm_ether_ntoa(&ea, eabuf);
	vp += sprintf(vp, "et0macaddr=%s", eabuf);
	vp++;

	/* next 6 bytes are et1macaddr */
	ea.octet[0] = (b[woff] >> 8) & 0xff;
	ea.octet[1] = b[woff] & 0xff;
	ea.octet[2] = (b[woff+1] >> 8) & 0xff;
	ea.octet[3] = b[woff+1] & 0xff;
	ea.octet[4] = (b[woff+2] >> 8) & 0xff;
	ea.octet[5] = b[woff+2] & 0xff;
	woff += ETHER_ADDR_LEN/2;
	bcm_ether_ntoa(&ea, eabuf);
	vp += sprintf(vp, "et1macaddr=%s", eabuf);
	vp++;

	/*
	 * Enet phy settings one or two singles or a dual
	 * Bits 4-0 : MII address for enet0 (0x1f for not there)
	 * Bits 9-5 : MII address for enet1 (0x1f for not there)
	 * Bit 14   : Mdio for enet0
	 * Bit 15   : Mdio for enet1
	 */
	w = b[woff];
	vp += sprintf(vp, "et0phyaddr=%d", (w & 0x1f));
	vp++;
	vp += sprintf(vp, "et1phyaddr=%d", ((w >> 5) & 0x1f));
	vp++;
	vp += sprintf(vp, "et0mdcport=%d", ((w >> 14) & 0x1));
	vp++;
	vp += sprintf(vp, "et1mdcport=%d", ((w >> 15) & 0x1));
	vp++;

	/* Word 46 has board rev, antennas 0/1 & Country code/control */
	w = b[46];
	vp += sprintf(vp, "boardrev=%d", w & 0xff);
	vp++;

	if (sromrev > 1)
		vp += sprintf(vp, "cctl=%d", (w >> 8) & 0xf);
	else
		vp += sprintf(vp, "cc=%d", (w >> 8) & 0xf);
	vp++;

	vp += sprintf(vp, "aa2g=%d", (w >> 12) & 0x3);
	vp++;

	vp += sprintf(vp, "aa5g=%d", (w >> 14) & 0x3);
	vp++;

	/* Words 47-49 set the (wl) pa settings */
	woff = 47;

	for (i = 0; i < 3; i++) {
		vp += sprintf(vp, "pa0b%d=%d", i, b[woff+i]);
		vp++;
		vp += sprintf(vp, "pa1b%d=%d", i, b[woff+i+6]);
		vp++;
	}

	/*
	 * Words 50-51 set the customer-configured wl led behavior.
	 * 8 bits/gpio pin.  High bit:  activehi=0, activelo=1;
	 * LED behavior values defined in wlioctl.h .
	 */
	w = b[50];
	if ((w != 0) && (w != 0xffff)) {
		/* ledbh0 */
		vp += sprintf(vp, "ledbh0=%d", (w & 0xff));
		vp++;

		/* ledbh1 */
		vp += sprintf(vp, "ledbh1=%d", (w >> 8) & 0xff);
		vp++;
	}
	w = b[51];
	if ((w != 0) && (w != 0xffff)) {
		/* ledbh2 */
		vp += sprintf(vp, "ledbh2=%d", w & 0xff);
		vp++;

		/* ledbh */
		vp += sprintf(vp, "ledbh3=%d", (w >> 8) & 0xff);
		vp++;
	}

	/* Word 52 is max power 0/1 */
	w = b[52];
	vp += sprintf(vp, "pa0maxpwr=%d", w & 0xff);
	vp++;
	vp += sprintf(vp, "pa1maxpwr=%d", (w >> 8) & 0xff);
	vp++;

	/* Word 56 is idle tssi target 0/1 */
	w = b[56];
	vp += sprintf(vp, "pa0itssit=%d", w & 0xff);
	vp++;
	vp += sprintf(vp, "pa1itssit=%d", (w >> 8) & 0xff);
	vp++;

	/* Word 57 is boardflags, if not programmed make it zero */
	w32 = (uint32)b[57];
	if (w32 == 0xffff) w32 = 0;
	if (sromrev > 1) {
		/* Word 28 is the high bits of boardflags */
		w32 |= (uint32)b[28] << 16;
	}
	vp += sprintf(vp, "boardflags=%d", w32);
	vp++;

	/* Word 58 is antenna gain 0/1 */
	w = b[58];
	vp += sprintf(vp, "ag0=%d", w & 0xff);
	vp++;

	vp += sprintf(vp, "ag1=%d", (w >> 8) & 0xff);
	vp++;

	if (sromrev == 1) {
		/* set the oem string */
		vp += sprintf(vp, "oem=%02x%02x%02x%02x%02x%02x%02x%02x",
		              ((b[59] >> 8) & 0xff), (b[59] & 0xff),
		              ((b[60] >> 8) & 0xff), (b[60] & 0xff),
		              ((b[61] >> 8) & 0xff), (b[61] & 0xff),
		              ((b[62] >> 8) & 0xff), (b[62] & 0xff));
		vp++;
	} else if (sromrev == 2 || sromrev == 3) {
		/* Word 60 OFDM tx power offset from CCK level */
		/* OFDM Power Offset - opo */
		vp += sprintf(vp, "opo=%d", b[60] & 0xff);
		vp++;
	} else {
		/* Word 60: cck power offsets */
		vp += sprintf(vp, "cckpo=%d", b[60]);
		vp++;

		/* Words 61+62: 11g ofdm power offsets */
		w32 = ((uint32)b[62] << 16) | b[61];
		vp += sprintf(vp, "ofdmgpo=%d", w32);
		vp++;
	}

	/* final nullbyte terminator */
done:	*vp++ = '\0';

	ASSERT((vp - base) <= VARS_MAX);

varsdone:
	err = initvars_table(osh, base, vp, vars, count);

err:
#ifdef WLTEST
	if (base != mfgsromvars)
#endif
		MFREE(osh, base, VARS_MAX);
	MFREE(osh, b, SROM_MAX);
	return err;
}

/*
 * Read the cis and call parsecis to initialize the vars.
 * Return 0 on success, nonzero on error.
 */
static int
initvars_cis_pcmcia(sb_t *sbh, osl_t *osh, char **vars, uint *count)
{
	uint8 *cis = NULL;
	int rc;
	uint data_sz;

	data_sz = (sb_pcmciarev(sbh) == 1) ? (SPROM_SIZE * 2) : CIS_SIZE;

	if ((cis = MALLOC(osh, data_sz)) == NULL)
		return (-2);

	if (sb_pcmciarev(sbh) == 1) {
		if (srom_read(sbh, PCMCIA_BUS, (void *)NULL, osh, 0, data_sz, (uint16 *)cis)) {
			MFREE(osh, cis, data_sz);
			return (-1);
		}
		/* fix up endianess for 16-bit data vs 8-bit parsing */
		ltoh16_buf((uint16 *)cis, data_sz);
	} else
		OSL_PCMCIA_READ_ATTR(osh, 0, cis, data_sz);

	rc = srom_parsecis(osh, &cis, 1, vars, count);

	MFREE(osh, cis, data_sz);

	return (rc);
}


#ifdef BCMSDIODEV
static int
initvars_cis_sddev(osl_t *osh, sb_t *sbh, void *curmap, char **vars, uint *count)
{
	char *vp;
	uint varsz;
	extern char *_vars;

	ASSERT(_vars);

	for (vp = _vars; vp && *vp; vp++)
		vp += strlen(vp);

	varsz = vp - _vars;

	if ((*vars = MALLOC(osh, varsz)) == NULL) {
		*count = 0;
		return -1;
	}

	bcopy(_vars, *vars, varsz);
	*count = varsz;

	return 0;
}
#endif /* BCMSDIODEV */

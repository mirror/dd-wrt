/*
 *  Routines to access SPROM and to parse SROM/CIS variables.
 *
 * Copyright (C) 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: bcmsrom.c,v 1.166.2.81.4.1 2008/10/31 06:00:49 Exp $
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <stdarg.h>
#include <bcmutils.h>
#include <hndsoc.h>
#include <sbchipc.h>
#include <bcmdevs.h>
#include <bcmendian.h>
#include <sbpcmcia.h>
#include <pcicfg.h>
#include <siutils.h>
#include <bcmsrom.h>
#include <bcmsrom_tbl.h>
#ifdef BCMSPI
#include <spid.h>
#endif /* BCMSPI */

#include <bcmnvram.h>
#include <bcmotp.h>

#if defined(BCMUSBDEV)
#include <sbsdio.h>
#include <sbhnddma.h>
#include <sbsdpcmdev.h>
#endif 

#ifdef WLTEST
#include <sbsprom.h>
#endif /* WLTEST */
#include <proto/ethernet.h>	/* for sprom content groking */

/* debug/trace */
#if defined(WLTEST)
#define	BS_ERROR(args)	printf args
#else
#define	BS_ERROR(args)
#endif	

#define WRITE_ENABLE_DELAY	500	/* 500 ms after write enable/disable toggle */
#define WRITE_WORD_DELAY	20	/* 20 ms between each word write */

typedef struct varbuf {
	char *base;		/* pointer to buffer base */
	char *buf;		/* pointer to current position */
	unsigned int size;	/* current (residual) size in bytes */
} varbuf_t;

static int initvars_srom_si(si_t *sih, osl_t *osh, void *curmap, char **vars, uint *count);
static void _initvars_srom_pci(uint8 sromrev, uint16 *srom, uint off, varbuf_t *b);
static int initvars_srom_pci(si_t *sih, void *curmap, char **vars, uint *count);
static int initvars_cis_pcmcia(si_t *sih, osl_t *osh, char **vars, uint *count);
#if !defined(BCMUSBDEV)
static int initvars_flash_si(si_t *sih, char **vars, uint *count);
#endif	
#ifdef BCMSPI
static int initvars_cis_spi(osl_t *osh, char **vars, uint *count);
#endif /* BCMSPI */
static int sprom_cmd_pcmcia(osl_t *osh, uint8 cmd);
static int sprom_read_pcmcia(osl_t *osh, uint16 addr, uint16 *data);
static int sprom_write_pcmcia(osl_t *osh, uint16 addr, uint16 data);
static int sprom_read_pci(osl_t *osh, si_t *sih, uint16 *sprom, uint wordoff, uint16 *buf,
                          uint nwords, bool check_crc);
#if defined(BCMNVRAMW) || defined(BCMNVRAMR)
static int otp_read_pci(osl_t *osh, si_t *sih, uint16 *buf, uint bufsz);
#endif /* defined(BCMNVRAMW) || defined(BCMNVRAMR) */
static uint16 srom_cc_cmd(si_t *sih, osl_t *osh, void *ccregs, uint32 cmd, uint wordoff,
                          uint16 data);

static int initvars_table(osl_t *osh, char *start, char *end, char **vars, uint *count);
static int initvars_flash(si_t *sih, osl_t *osh, char **vp, uint len);

#if defined(BCMUSBDEV)
static int get_si_pcmcia_srom(si_t *sih, osl_t *osh, uint8 *pcmregs,
                              uint boff, uint16 *srom, uint bsz, bool check_crc);
static int set_si_pcmcia_srom(si_t *sih, osl_t *osh, uint8 *pcmregs,
                              uint boff, uint16 *srom, uint bsz);
#endif 

#ifdef	USB4322
static int initvars_cis_usb(si_t *sih, osl_t *osh, char **vars, uint *count);

/* default to bcm943231 P100), other boards should have OTP programmed */
static char BCMATTACHDATA(defaultsromvars_4322usb)[] =
	"vendid=0x14e4\0"
	"subvendid=0x0a5c\0"
	"subdevid=0xbdc\0"
	"macaddr=00:90:4c:d3:04:73\0"
	"sromrev=8\0"
	"devid=0x4340\0"
	"boardrev=0x1100\0"
	"boardflags=0xa00\0"
	"boardflags2=0x3\0"
	"prodid=0x4a8\0"
	"boardtype=0x04d5\0"
	"tssipos2g=0x1\0"
	"extpagain2g=0x2\0"
	"pdetrange2g=0x2\0"
	"triso2g=0x3\0"
	"antswctl2g=0x2\0"
	"antswitch=0\0"
	"maxp2ga0=0x48\0"
	"itt2ga0=0x20\0"
	"pa2gw0a0=0xFE7B\0"
	"pa2gw1a0=0x153B\0"
	"pa2gw2a0=0xFAEF\0"
	"maxp2ga1=0x48\0"
	"itt2ga1=0x20\0"
	"pa2gw0a1=0xFE7B\0"
	"pa2gw1a1=0x153B\0"
	"pa2gw2a1=0xFAEF\0"
	"cctl=0\0"
	"ccode=US\0"
	"regrev=0x0\0"
	"ledbh0=0xff\0"
	"ledbh1=0x2\0"
	"ledbh2=0x3\0"
	"ledbh3=0xff\0"
	"leddc=0x0a0a\0"
	"opo=0x0\0"
	"aa2g=0x3\0"
	"aa5g=0x3\0"
	"ag0=0x2\0"
	"ag1=0x2\0"
	"ag2=0xff\0"
	"ag3=0xff\0"
	"txchain=0x3\0"
	"rxchain=0x3\0"
	"END\0";

#endif	/* USB4322 */

#if defined(WLTEST)
/* Also used by wl_readconfigdata for vars download */
char BCMATTACHDATA(mfgsromvars)[VARS_MAX];
int BCMATTACHDATA(defvarslen) = 0;

static char BCMATTACHDATA(defaultsromvars_wltest)[] =
	"macaddr=00:90:4c:f8:00:01\0"
	"et0macaddr=00:11:22:33:44:52\0"
	"et0phyaddr=30\0"
	"et0mdcport=0\0"
	"gpio2=robo_reset\0"
	"boardvendor=0x14e4\0"
	"boardflags=0x210\0"
	"boardflags2=0\0"
	"boardtype=0x04c3\0"
	"boardrev=0x1100\0"
	"sromrev=8\0"
	"devid=0x432c\0"
	"ccode=0\0"
	"regrev=0\0"
	"ledbh0=255\0"
	"ledbh1=255\0"
	"ledbh2=255\0"
	"ledbh3=255\0"
	"leddc=0xffff\0"
	"aa2g=3\0"
	"ag0=2\0"
	"ag1=2\0"
	"aa5g=3\0"
	"aa0=2\0"
	"aa1=2\0"
	"txchain=3\0"
	"rxchain=3\0"
	"antswitch=0\0"
	"itt2ga0=0x20\0"
	"maxp2ga0=0x48\0"
	"pa2gw0a0=0xfe9e\0"
	"pa2gw1a0=0x15d5\0"
	"pa2gw2a0=0xfae9\0"
	"itt2ga1=0x20\0"
	"maxp2ga1=0x48\0"
	"pa2gw0a1=0xfeb3\0"
	"pa2gw1a1=0x15c9\0"
	"pa2gw2a1=0xfaf7\0"
	"tssipos2g=1\0"
	"extpagain2g=0\0"
	"pdetrange2g=0\0"
	"triso2g=3\0"
	"antswctl2g=0\0"
	"tssipos5g=1\0"
	"extpagain5g=0\0"
	"pdetrange5g=0\0"
	"triso5g=3\0"
	"antswctl5g=0\0"
	"cck2gpo=0\0"
	"ofdm2gpo=0\0"
	"mcs2gpo0=0\0"
	"mcs2gpo1=0\0"
	"mcs2gpo2=0\0"
	"mcs2gpo3=0\0"
	"mcs2gpo4=0\0"
	"mcs2gpo5=0\0"
	"mcs2gpo6=0\0"
	"mcs2gpo7=0\0"
	"cddpo=0\0"
	"stbcpo=0\0"
	"bw40po=4\0"
	"bwduppo=0\0"
	"END\0";
#endif 

#if defined(WLTEST) || defined(USB4322)
/* It must end with pattern of "END" */
static uint
BCMATTACHFN(srom_vars_len)(char *vars)
{
	uint pos = 0;
	uint len;
	char *s;

	for (s = vars; s && *s;) {

		if (strcmp(s, "END") == 0)
			break;

		len = strlen(s);
		s += strlen(s) + 1;
		pos += len + 1;
		/* BS_ERROR(("len %d vars[pos] %s\n", pos, s)); */
		if (pos > 4000) {
			return 0;
		}
	}

	return pos + 4;	/* include the "END\0" */
}
#endif 

/* Initialization of varbuf structure */
static void
BCMNMIATTACHFN(varbuf_init)(varbuf_t *b, char *buf, uint size)
{
	b->size = size;
	b->base = b->buf = buf;
}

/* append a null terminated var=value string */
static int
BCMNMIATTACHFN(varbuf_append)(varbuf_t *b, const char *fmt, ...)
{
	va_list ap;
	int r;
	size_t len;
	char *s;

	if (b->size < 2)
	  return 0;

	va_start(ap, fmt);
	r = vsnprintf(b->buf, b->size, fmt, ap);
	va_end(ap);

	/* C99 snprintf behavior returns r >= size on overflow,
	 * others return -1 on overflow.
	 * All return -1 on format error.
	 * We need to leave room for 2 null terminations, one for the current var
	 * string, and one for final null of the var table. So check that the
	 * strlen written, r, leaves room for 2 chars.
	 */
	if ((r == -1) || (r > (int)(b->size - 2))) {
		b->size = 0;
		return 0;
	}

	/* Remove any earlier occurrence of the same variable */
	if ((s = strchr(b->buf, '=')) != NULL) {
		len = (size_t)(s - b->buf);
		for (s = b->base; s < b->buf;) {
			if ((bcmp(s, b->buf, len) == 0) && s[len] == '=') {
				len = strlen(s) + 1;
				memmove(s, (s + len), ((b->buf + r + 1) - (s + len)));
				b->buf -= len;
				b->size += (unsigned int)len;
				break;
			}

			while (*s++)
				;
		}
	}

	/* skip over this string's null termination */
	r++;
	b->size -= r;
	b->buf += r;

	return r;
}

/*
 * Initialize local vars from the right source for this platform.
 * Return 0 on success, nonzero on error.
 */
int
BCMATTACHFN(srom_var_init)(si_t *sih, uint bustype, void *curmap, osl_t *osh,
	char **vars, uint *count)
{
	uint len;

	len = 0;

	ASSERT(bustype == BUSTYPE(bustype));
	if (vars == NULL || count == NULL)
		return (0);

	*vars = NULL;
	*count = 0;

	switch (BUSTYPE(bustype)) {
	case SI_BUS:
	case JTAG_BUS:
		return initvars_srom_si(sih, osh, curmap, vars, count);

	case PCI_BUS:
		ASSERT(curmap != NULL);
		return initvars_srom_pci(sih, curmap, vars, count);

	case PCMCIA_BUS:
		return initvars_cis_pcmcia(sih, osh, vars, count);


#ifdef BCMSPI
	case SPI_BUS:
		return initvars_cis_spi(osh, vars, count);
#endif /* BCMSPI */

	default:
		ASSERT(0);
	}
	return (-1);
}

/* support only 16-bit word read from srom */
int
srom_read(si_t *sih, uint bustype, void *curmap, osl_t *osh,
          uint byteoff, uint nbytes, uint16 *buf, bool check_crc)
{
	uint16 *srom;
	uint i, off, nw;

	ASSERT(bustype == BUSTYPE(bustype));

	/* check input - 16-bit access only */
	if (byteoff & 1 || nbytes & 1 || (byteoff + nbytes) > SROM_MAX)
		return 1;

	off = byteoff / 2;
	nw = nbytes / 2;

	if (BUSTYPE(bustype) == PCI_BUS) {
		if (!curmap)
			return 1;
		if (sih->ccrev >= 31) {
			if ((sih->cccaps & CC_CAP_SROM) == 0) {
				/* No srom support in this chip */
				return 1;
			} else {
				srom = (uint16 *)
				        ((uint8 *)curmap + PCI_16KB0_CCREGS_OFFSET + CC_SROM_OTP);
			}
		} else {
			srom = (uint16 *)((uint8 *)curmap + PCI_BAR0_SPROM_OFFSET);
		}
		if (sprom_read_pci(osh, sih, srom, off, buf, nw, check_crc))
			return 1;
	} else if (BUSTYPE(bustype) == PCMCIA_BUS) {
		for (i = 0; i < nw; i++) {
			if (sprom_read_pcmcia(osh, (uint16)(off + i), (uint16 *)(buf + i)))
				return 1;
		}
#ifdef BCMSPI
	} else if (BUSTYPE(bustype) == SPI_BUS) {
	                if (bcmsdh_cis_read(NULL, SDIO_FUNC_1, (uint8 *)buf, byteoff + nbytes) != 0)
				return 1;
#endif /* BCMSPI */
	} else if (BUSTYPE(bustype) == SI_BUS) {
#if defined(BCMUSBDEV)
		if (SPROMBUS == PCMCIA_BUS) {
			uint origidx;
			void *regs;
			int rc;
			bool wasup;

			/* Don't bother if we can't talk to SPROM */
			if (!si_is_sprom_available(sih))
				return 1;

			origidx = si_coreidx(sih);
			regs = si_setcore(sih, PCMCIA_CORE_ID, 0);
			ASSERT(regs != NULL);

			if (!(wasup = si_iscoreup(sih)))
				si_core_reset(sih, 0, 0);

			rc = get_si_pcmcia_srom(sih, osh, regs, byteoff, buf, nbytes, check_crc);

			if (!wasup)
				si_core_disable(sih, 0);

			si_setcoreidx(sih, origidx);
			return rc;
		}
#endif 

		return 1;
	} else {
		return 1;
	}

	return 0;
}

/* support only 16-bit word write into srom */
int
srom_write(si_t *sih, uint bustype, void *curmap, osl_t *osh,
           uint byteoff, uint nbytes, uint16 *buf)
{
	uint i, nw, crc_range;
	uint16 old[SROM_MAXW], new[SROM_MAXW];
	uint8 crc;
	volatile uint32 val32;

	ASSERT(bustype == BUSTYPE(bustype));

	/* check input - 16-bit access only. use byteoff 0x55aa to indicate
	 * srclear
	 */
	if ((byteoff != 0x55aa) && ((byteoff & 1) || (nbytes & 1)))
		return 1;

	if ((byteoff != 0x55aa) && ((byteoff + nbytes) > SROM_MAX))
		return 1;

	if (BUSTYPE(bustype) == PCMCIA_BUS) {
		crc_range = SROM_MAX;
	}
#if defined(BCMUSBDEV)
	else {
		crc_range = srom_size(sih, osh);
	}
#else /* def BCMUSBDEV || def BCMSDIODEV */
	else {
		crc_range = (SROM8_SIGN + 1) * 2;	/* must big enough for SROM8 */
	}
#endif 

	nw = crc_range / 2;
	/* read first small number words from srom, then adjust the length, read all */
	if (srom_read(sih, bustype, curmap, osh, 0, crc_range, old, FALSE))
		return 1;

	BS_ERROR(("%s: old[SROM4_SIGN] 0x%x, old[SROM8_SIGN] 0x%x\n",
	          __FUNCTION__, old[SROM4_SIGN], old[SROM8_SIGN]));
	/* Deal with blank srom */
	if (old[0] == 0xffff) {
		/* see if the input buffer is valid SROM image or not */
		if ((buf[SROM4_SIGN] == SROM4_SIGNATURE) ||
			(buf[SROM8_SIGN] == SROM4_SIGNATURE)) {
			BS_ERROR(("%s: buf[SROM4_SIGN] 0x%x, buf[SROM8_SIGN] 0x%x\n",
				__FUNCTION__, buf[SROM4_SIGN], buf[SROM8_SIGN]));

			/* block invalid buffer size */
			if (nbytes < SROM4_WORDS * 2)
				return BCME_BUFTOOSHORT;
			else if (nbytes > SROM4_WORDS * 2)
				return BCME_BUFTOOLONG;

			nw = SROM4_WORDS;
		} else if (nbytes == SROM_WORDS * 2){ /* the other possible SROM format */
			BS_ERROR(("%s: Not SROM4 or SROM8.\n", __FUNCTION__));

			nw = SROM_WORDS;
		} else {
			BS_ERROR(("%s: Invalid input file signature\n", __FUNCTION__));
			return BCME_BADARG;
		}
		crc_range = nw * 2;
		if (srom_read(sih, bustype, curmap, osh, 0, crc_range, old, FALSE))
			return 1;
	} else if ((old[SROM4_SIGN] == SROM4_SIGNATURE) ||
	    (old[SROM8_SIGN] == SROM4_SIGNATURE)) {
		nw = SROM4_WORDS;
		crc_range = nw * 2;
		if (srom_read(sih, bustype, curmap, osh, 0, crc_range, old, FALSE))
			return 1;
	} else {
		/* Assert that we have already read enough for sromrev 2 */
		ASSERT(crc_range >= SROM_WORDS * 2);
		nw = SROM_WORDS;
		crc_range = nw * 2;
	}

	if (byteoff == 0x55aa) {
		/* Erase request */
		crc_range = 0;
		memset((void *)new, 0xff, nw * 2);
	} else {
		/* Copy old contents */
		bcopy((void *)old, (void *)new, nw * 2);
		/* make changes */
		bcopy((void *)buf, (void *)&new[byteoff / 2], nbytes);
	}

	if (crc_range) {
		/* calculate crc */
		htol16_buf(new, crc_range);
		crc = ~hndcrc8((uint8 *)new, crc_range - 1, CRC8_INIT_VALUE);
		ltoh16_buf(new, crc_range);
		new[nw - 1] = (crc << 8) | (new[nw - 1] & 0xff);
	}

	if (BUSTYPE(bustype) == PCI_BUS) {
		uint16 *srom;
		void *ccregs = NULL;

		/* enable writes to the SPROM */
		if (sih->ccrev >= 31) {
			ccregs = (void *)((uint8 *)curmap + PCI_16KB0_CCREGS_OFFSET);
			srom = (uint16 *)((uint8 *)ccregs + CC_SROM_OTP);
			(void)srom_cc_cmd(sih, osh, ccregs, SRC_OP_WREN, 0, 0);
		} else {
			srom = (uint16 *)((uint8 *)curmap + PCI_BAR0_SPROM_OFFSET);
			val32 = OSL_PCI_READ_CONFIG(osh, PCI_SPROM_CONTROL, sizeof(uint32));
			val32 |= SPROM_WRITEEN;
			OSL_PCI_WRITE_CONFIG(osh, PCI_SPROM_CONTROL, sizeof(uint32), val32);
		}
		bcm_mdelay(WRITE_ENABLE_DELAY);
		/* write srom */
		for (i = 0; i < nw; i++) {
			if (old[i] != new[i]) {
				W_REG(osh, &srom[i], new[i]);
				bcm_mdelay(WRITE_WORD_DELAY);
			}
		}
		/* disable writes to the SPROM */
		if (sih->ccrev >= 31) {
			(void)srom_cc_cmd(sih, osh, ccregs, SRC_OP_WRDIS, 0, 0);
		} else {
			OSL_PCI_WRITE_CONFIG(osh, PCI_SPROM_CONTROL, sizeof(uint32), val32 &
			                     ~SPROM_WRITEEN);
		}
	} else if (BUSTYPE(bustype) == PCMCIA_BUS) {
		/* enable writes to the SPROM */
		if (sprom_cmd_pcmcia(osh, SROM_WEN))
			return 1;
		bcm_mdelay(WRITE_ENABLE_DELAY);
		/* write srom */
		for (i = 0; i < nw; i++) {
			if (old[i] != new[i]) {
				sprom_write_pcmcia(osh, (uint16)(i), new[i]);
				bcm_mdelay(WRITE_WORD_DELAY);
			}
		}
		/* disable writes to the SPROM */
		if (sprom_cmd_pcmcia(osh, SROM_WDS))
			return 1;
	} else if (BUSTYPE(bustype) == SI_BUS) {
#if defined(BCMUSBDEV)
		if (SPROMBUS == PCMCIA_BUS) {
			uint origidx;
			void *regs;
			int rc;
			bool wasup;

			origidx = si_coreidx(sih);
			regs = si_setcore(sih, PCMCIA_CORE_ID, 0);
			ASSERT(regs != NULL);

			if (!(wasup = si_iscoreup(sih)))
				si_core_reset(sih, 0, 0);

			rc = set_si_pcmcia_srom(sih, osh, regs, byteoff, buf, nbytes);

			if (!wasup)
				si_core_disable(sih, 0);

			si_setcoreidx(sih, origidx);
			return rc;
		}
#endif 
		return 1;
	} else {
		return 1;
	}

	bcm_mdelay(WRITE_ENABLE_DELAY);
	return 0;
}

#if defined(BCMUSBDEV)
#define SI_PCMCIA_READ(osh, regs, fcr) \
		R_REG(osh, (volatile uint8 *)(regs) + 0x600 + (fcr) - 0x700 / 2)
#define SI_PCMCIA_WRITE(osh, regs, fcr, v) \
		W_REG(osh, (volatile uint8 *)(regs) + 0x600 + (fcr) - 0x700 / 2, v)

/* set PCMCIA srom command register */
static int
srom_cmd_si_pcmcia(osl_t *osh, uint8 *pcmregs, uint8 cmd)
{
	uint8 status = 0;
	uint wait_cnt = 0;

	/* write srom command register */
	SI_PCMCIA_WRITE(osh, pcmregs, SROM_CS, cmd);

	/* wait status */
	while (++wait_cnt < 1000000) {
		status = SI_PCMCIA_READ(osh, pcmregs, SROM_CS);
		if (status & SROM_DONE)
			return 0;
		OSL_DELAY(1);
	}

	BS_ERROR(("sr_cmd: Give up after %d tries, stat = 0x%x\n", wait_cnt, status));
	return 1;
}

/* read a word from the PCMCIA srom over SI */
static int
srom_read_si_pcmcia(osl_t *osh, uint8 *pcmregs, uint16 addr, uint16 *data)
{
	uint8 addr_l, addr_h,  data_l, data_h;

	addr_l = (uint8)((addr * 2) & 0xff);
	addr_h = (uint8)(((addr * 2) >> 8) & 0xff);

	/* set address */
	SI_PCMCIA_WRITE(osh, pcmregs, SROM_ADDRH, addr_h);
	SI_PCMCIA_WRITE(osh, pcmregs, SROM_ADDRL, addr_l);

	/* do read */
	if (srom_cmd_si_pcmcia(osh, pcmregs, SROM_READ))
		return 1;

	/* read data */
	data_h = SI_PCMCIA_READ(osh, pcmregs, SROM_DATAH);
	data_l = SI_PCMCIA_READ(osh, pcmregs, SROM_DATAL);
	*data = ((uint16)data_h << 8) | data_l;

	return 0;
}

/* write a word to the PCMCIA srom over SI */
static int
srom_write_si_pcmcia(osl_t *osh, uint8 *pcmregs, uint16 addr, uint16 data)
{
	uint8 addr_l, addr_h, data_l, data_h;
	int rc;

	addr_l = (uint8)((addr * 2) & 0xff);
	addr_h = (uint8)(((addr * 2) >> 8) & 0xff);

	/* set address */
	SI_PCMCIA_WRITE(osh, pcmregs, SROM_ADDRH, addr_h);
	SI_PCMCIA_WRITE(osh, pcmregs, SROM_ADDRL, addr_l);

	data_l = (uint8)(data & 0xff);
	data_h = (uint8)((data >> 8) & 0xff);

	/* write data */
	SI_PCMCIA_WRITE(osh, pcmregs, SROM_DATAH, data_h);
	SI_PCMCIA_WRITE(osh, pcmregs, SROM_DATAL, data_l);

	/* do write */
	rc = srom_cmd_si_pcmcia(osh, pcmregs, SROM_WRITE);
	OSL_DELAY(20000);
	return rc;
}

/*
 * Read the srom for the pcmcia-srom over si case.
 * Return 0 on success, nonzero on error.
 */
static int
get_si_pcmcia_srom(si_t *sih, osl_t *osh, uint8 *pcmregs,
                   uint boff, uint16 *srom, uint bsz, bool check_crc)
{
	uint i, nw, woff, wsz;
	int err = 0;

	/* read must be at word boundary */
	ASSERT((boff & 1) == 0 && (bsz & 1) == 0);

	/* read sprom size and validate the parms */
	if ((nw = srom_size(sih, osh)) == 0) {
		BS_ERROR(("get_si_pcmcia_srom: sprom size unknown\n"));
		err = -1;
		goto out;
	}
	if (boff + bsz > 2 * nw) {
		BS_ERROR(("get_si_pcmcia_srom: sprom size exceeded\n"));
		err = -2;
		goto out;
	}

	/* read in sprom contents */
	for (woff = boff / 2, wsz = bsz / 2, i = 0;
	     woff < nw && i < wsz; woff ++, i ++) {
		if (srom_read_si_pcmcia(osh, pcmregs, (uint16)woff, &srom[i])) {
			BS_ERROR(("get_si_pcmcia_srom: sprom read failed\n"));
			err = -3;
			goto out;
		}
	}

	if (check_crc) {
		if (srom[0] == 0xffff) {
			/* The hardware thinks that an srom that starts with 0xffff
			 * is blank, regardless of the rest of the content, so declare
			 * it bad.
			 */
			BS_ERROR(("%s: srom[0] == 0xffff, assuming unprogrammed srom\n",
			          __FUNCTION__));
			err = -4;
			goto out;
		}

		/* fixup the endianness so crc8 will pass */
		htol16_buf(srom, nw * 2);
		if (hndcrc8((uint8 *)srom, nw * 2, CRC8_INIT_VALUE) != CRC8_GOOD_VALUE) {
			BS_ERROR(("%s: bad crc\n", __FUNCTION__));
			err = -5;
		}
		/* now correct the endianness of the byte array */
		ltoh16_buf(srom, nw * 2);
	}

out:
	return err;
}

/*
 * Write the srom for the pcmcia-srom over si case.
 * Return 0 on success, nonzero on error.
 */
static int
set_si_pcmcia_srom(si_t *sih, osl_t *osh, uint8 *pcmregs,
                   uint boff, uint16 *srom, uint bsz)
{
	uint i, nw, woff, wsz;
	uint16 word;
	uint8 crc;
	int err = 0;

	/* write must be at word boundary */
	ASSERT((boff & 1) == 0 && (bsz & 1) == 0);

	/* read sprom size and validate the parms */
	if ((nw = srom_size(sih, osh)) == 0) {
		BS_ERROR(("set_si_pcmcia_srom: sprom size unknown\n"));
		err = -1;
		goto out;
	}
	if (boff + bsz > 2 * nw) {
		BS_ERROR(("set_si_pcmcia_srom: sprom size exceeded\n"));
		err = -2;
		goto out;
	}

	/* enable write */
	if (srom_cmd_si_pcmcia(osh, pcmregs, SROM_WEN)) {
		BS_ERROR(("set_si_pcmcia_srom: sprom wen failed\n"));
		err = -3;
		goto out;
	}

	/* write buffer to sprom */
	for (woff = boff / 2, wsz = bsz / 2, i = 0;
	     woff < nw && i < wsz; woff ++, i ++) {
		if (srom_write_si_pcmcia(osh, pcmregs, (uint16)woff, srom[i])) {
			BS_ERROR(("set_si_pcmcia_srom: sprom write failed\n"));
			err = -4;
			goto out;
		}
	}

	/* fix crc */
	crc = CRC8_INIT_VALUE;
	for (woff = 0; woff < nw; woff ++) {
		if (srom_read_si_pcmcia(osh, pcmregs, (uint16)woff, &word)) {
			BS_ERROR(("set_si_pcmcia_srom: sprom fix crc read failed\n"));
			err = -5;
			goto out;
		}
		word = htol16(word);
		crc = hndcrc8((uint8 *)&word, woff != nw - 1 ? 2 : 1, crc);
	}
	word = (~crc << 8) + (ltoh16(word) & 0xff);
	if (srom_write_si_pcmcia(osh, pcmregs, (uint16)(woff - 1), word)) {
		BS_ERROR(("set_si_pcmcia_srom: sprom fix crc write failed\n"));
		err = -6;
		goto out;
	}

	/* disable write */
	if (srom_cmd_si_pcmcia(osh, pcmregs, SROM_WDS)) {
		BS_ERROR(("set_si_pcmcia_srom: sprom wds failed\n"));
		err = -7;
		goto out;
	}

out:
	return err;
}
#endif 

static char BCMNMIATTACHDATA(vstr_manf)[] = "manf=%s";
static char BCMNMIATTACHDATA(vstr_productname)[] = "productname=%s";
static char BCMNMIATTACHDATA(vstr_manfid)[] = "manfid=0x%x";
static char BCMNMIATTACHDATA(vstr_prodid)[] = "prodid=0x%x";
static char BCMNMIATTACHDATA(vstr_regwindowsz)[] = "regwindowsz=%d";
static char BCMNMIATTACHDATA(vstr_sromrev)[] = "sromrev=%d";
static char BCMNMIATTACHDATA(vstr_chiprev)[] = "chiprev=%d";
static char BCMNMIATTACHDATA(vstr_subvendid)[] = "subvendid=0x%x";
static char BCMNMIATTACHDATA(vstr_subdevid)[] = "subdevid=0x%x";
static char BCMNMIATTACHDATA(vstr_boardrev)[] = "boardrev=0x%x";
static char BCMNMIATTACHDATA(vstr_aa2g)[] = "aa2g=0x%x";
static char BCMNMIATTACHDATA(vstr_aa5g)[] = "aa5g=0x%x";
static char BCMNMIATTACHDATA(vstr_ag)[] = "ag%d=0x%x";
static char BCMNMIATTACHDATA(vstr_cc)[] = "cc=%d";
static char BCMNMIATTACHDATA(vstr_opo)[] = "opo=%d";
static char BCMNMIATTACHDATA(vstr_pa0b)[][9] = { "pa0b0=%d", "pa0b1=%d", "pa0b2=%d" };
static char BCMNMIATTACHDATA(vstr_pa0itssit)[] = "pa0itssit=%d";
static char BCMNMIATTACHDATA(vstr_pa0maxpwr)[] = "pa0maxpwr=%d";
static char BCMNMIATTACHDATA(vstr_pa1b)[][9] = { "pa1b0=%d", "pa1b1=%d", "pa1b2=%d" };
static char BCMNMIATTACHDATA(vstr_pa1lob)[][11] = { "pa1lob0=%d", "pa1lob1=%d", "pa1lob2=%d" };
static char BCMNMIATTACHDATA(vstr_pa1hib)[][11] = { "pa1hib0=%d", "pa1hib1=%d", "pa1hib2=%d" };
static char BCMNMIATTACHDATA(vstr_pa1itssit)[] = "pa1itssit=%d";
static char BCMNMIATTACHDATA(vstr_pa1maxpwr)[] = "pa1maxpwr=%d";
static char BCMNMIATTACHDATA(vstr_pa1lomaxpwr)[] = "pa1lomaxpwr=%d";
static char BCMNMIATTACHDATA(vstr_pa1himaxpwr)[] = "pa1himaxpwr=%d";
static char BCMNMIATTACHDATA(vstr_oem)[] = "oem=%02x%02x%02x%02x%02x%02x%02x%02x";
static char BCMNMIATTACHDATA(vstr_boardflags)[] = "boardflags=0x%x";
static char BCMNMIATTACHDATA(vstr_boardflags2)[] = "boardflags2=0x%x";
static char BCMNMIATTACHDATA(vstr_ledbh)[] = "ledbh%d=0x%x";
static char BCMNMIATTACHDATA(vstr_noccode)[] = "ccode=0x0";
static char BCMNMIATTACHDATA(vstr_ccode)[] = "ccode=%c%c";
static char BCMNMIATTACHDATA(vstr_cctl)[] = "cctl=0x%x";
static char BCMNMIATTACHDATA(vstr_cckpo)[] = "cckpo=0x%x";
static char BCMNMIATTACHDATA(vstr_ofdmpo)[] = "ofdmpo=0x%x";
static char BCMNMIATTACHDATA(vstr_rdlid)[] = "rdlid=0x%x";
static char BCMNMIATTACHDATA(vstr_rdlrndis)[] = "rdlrndis=%d";
static char BCMNMIATTACHDATA(vstr_rdlrwu)[] = "rdlrwu=%d";
static char BCMNMIATTACHDATA(vstr_usbfs)[] = "usbfs=%d";
static char BCMNMIATTACHDATA(vstr_wpsgpio)[] = "wpsgpio=%d";
static char BCMNMIATTACHDATA(vstr_rdlsn)[] = "rdlsn=%d";
static char BCMNMIATTACHDATA(vstr_rssismf2g)[] = "rssismf2g=%d";
static char BCMNMIATTACHDATA(vstr_rssismc2g)[] = "rssismc2g=%d";
static char BCMNMIATTACHDATA(vstr_rssisav2g)[] = "rssisav2g=%d";
static char BCMNMIATTACHDATA(vstr_bxa2g)[] = "bxa2g=%d";
static char BCMNMIATTACHDATA(vstr_rssismf5g)[] = "rssismf5g=%d";
static char BCMNMIATTACHDATA(vstr_rssismc5g)[] = "rssismc5g=%d";
static char BCMNMIATTACHDATA(vstr_rssisav5g)[] = "rssisav5g=%d";
static char BCMNMIATTACHDATA(vstr_bxa5g)[] = "bxa5g=%d";
static char BCMNMIATTACHDATA(vstr_tri2g)[] = "tri2g=%d";
static char BCMNMIATTACHDATA(vstr_tri5gl)[] = "tri5gl=%d";
static char BCMNMIATTACHDATA(vstr_tri5g)[] = "tri5g=%d";
static char BCMNMIATTACHDATA(vstr_tri5gh)[] = "tri5gh=%d";
static char BCMNMIATTACHDATA(vstr_rxpo2g)[] = "rxpo2g=%d";
static char BCMNMIATTACHDATA(vstr_rxpo5g)[] = "rxpo5g=%d";
static char BCMNMIATTACHDATA(vstr_boardtype)[] = "boardtype=0x%x";
static char BCMNMIATTACHDATA(vstr_leddc)[] = "leddc=0x%04x";
static char BCMNMIATTACHDATA(vstr_vendid)[] = "vendid=0x%x";
static char BCMNMIATTACHDATA(vstr_devid)[] = "devid=0x%x";
static char BCMNMIATTACHDATA(vstr_xtalfreq)[] = "xtalfreq=%d";
static char BCMNMIATTACHDATA(vstr_txchain)[] = "txchain=0x%x";
static char BCMNMIATTACHDATA(vstr_rxchain)[] = "rxchain=0x%x";
static char BCMNMIATTACHDATA(vstr_antswitch)[] = "antswitch=0x%x";
static char BCMNMIATTACHDATA(vstr_regrev)[] = "regrev=0x%x";
static char BCMNMIATTACHDATA(vstr_antswctl2g)[] = "antswctl2g=0x%x";
static char BCMNMIATTACHDATA(vstr_triso2g)[] = "triso2g=0x%x";
static char BCMNMIATTACHDATA(vstr_pdetrange2g)[] = "pdetrange2g=0x%x";
static char BCMNMIATTACHDATA(vstr_extpagain2g)[] = "extpagain2g=0x%x";
static char BCMNMIATTACHDATA(vstr_tssipos2g)[] = "tssipos2g=0x%x";
static char BCMNMIATTACHDATA(vstr_antswctl5g)[] = "antswctl5g=0x%x";
static char BCMNMIATTACHDATA(vstr_triso5g)[] = "triso5g=0x%x";
static char BCMNMIATTACHDATA(vstr_pdetrange5g)[] = "pdetrange5g=0x%x";
static char BCMNMIATTACHDATA(vstr_extpagain5g)[] = "extpagain5g=0x%x";
static char BCMNMIATTACHDATA(vstr_tssipos5g)[] = "tssipos5g=0x%x";
static char BCMNMIATTACHDATA(vstr_maxp2ga0)[] = "maxp2ga0=0x%x";
static char BCMNMIATTACHDATA(vstr_itt2ga0)[] = "itt2ga0=0x%x";
static char BCMNMIATTACHDATA(vstr_pa)[] = "pa%dgw%da%d=0x%x";
static char BCMNMIATTACHDATA(vstr_pahl)[] = "pa%dg%cw%da%d=0x%x";
static char BCMNMIATTACHDATA(vstr_maxp5ga0)[] = "maxp5ga0=0x%x";
static char BCMNMIATTACHDATA(vstr_itt5ga0)[] = "itt5ga0=0x%x";
static char BCMNMIATTACHDATA(vstr_maxp5gha0)[] = "maxp5gha0=0x%x";
static char BCMNMIATTACHDATA(vstr_maxp5gla0)[] = "maxp5gla0=0x%x";
static char BCMNMIATTACHDATA(vstr_maxp2ga1)[] = "maxp2ga1=0x%x";
static char BCMNMIATTACHDATA(vstr_itt2ga1)[] = "itt2ga1=0x%x";
static char BCMNMIATTACHDATA(vstr_maxp5ga1)[] = "maxp5ga1=0x%x";
static char BCMNMIATTACHDATA(vstr_itt5ga1)[] = "itt5ga1=0x%x";
static char BCMNMIATTACHDATA(vstr_maxp5gha1)[] = "maxp5gha1=0x%x";
static char BCMNMIATTACHDATA(vstr_maxp5gla1)[] = "maxp5gla1=0x%x";
static char BCMNMIATTACHDATA(vstr_cck2gpo)[] = "cck2gpo=0x%x";
static char BCMNMIATTACHDATA(vstr_ofdm2gpo)[] = "ofdm2gpo=0x%x";
static char BCMNMIATTACHDATA(vstr_ofdm5gpo)[] = "ofdm5gpo=0x%x";
static char BCMNMIATTACHDATA(vstr_ofdm5glpo)[] = "ofdm5glpo=0x%x";
static char BCMNMIATTACHDATA(vstr_ofdm5ghpo)[] = "ofdm5ghpo=0x%x";
static char BCMNMIATTACHDATA(vstr_mcspo)[] = "mcs%dgpo%d=0x%x";
static char BCMNMIATTACHDATA(vstr_mcspohl)[] = "mcs%dg%cpo%d=0x%x";
static char BCMNMIATTACHDATA(vstr_custom)[] = "customvar%d=0x%x";
#ifdef USB4322
static char BCMNMIATTACHDATA(vstr_end)[] = "END\0";
#endif /* USB4322 */

#define FROMHOST()
static char BCMINITDATA(vstr_boardnum)[] = "boardnum=%d";
static char BCMINITDATA(vstr_macaddr)[] = "macaddr=%s";

int
BCMNMIATTACHFN(srom_parsecis)(osl_t *osh, uint8 *pcis[], uint ciscnt, char **vars, uint *count)
{
	char eabuf[32];
	char *base;
	varbuf_t b;
	uint8 *cis, tup, tlen, sromrev = 1;
	int i, j;
	bool ag_init = FALSE;
	uint32 w32;
	uint funcid;
	uint cisnum;
	int32 boardnum;
	int err;
	bool standard_cis;

	ASSERT(vars != NULL);
	ASSERT(count != NULL);

	boardnum = -1;

	base = MALLOC(osh, MAXSZ_NVRAM_VARS);
	ASSERT(base != NULL);
	if (!base)
		return -2;

	varbuf_init(&b, base, MAXSZ_NVRAM_VARS);

	eabuf[0] = '\0';
	for (cisnum = 0; cisnum < ciscnt; cisnum++) {
		cis = *pcis++;
		i = 0;
		funcid = 0;
		standard_cis = TRUE;
		do {
			if (standard_cis) {
				tup = cis[i++];
				if (tup == CISTPL_NULL || tup == CISTPL_END)
					tlen = 0;
				else {
					tlen = cis[i++];
					/*
					 * what follows is a nonstandard HNBU CIS
					 * that lacks CISTPL_BRCM_HNBU tags
					 */
					if (cis[i] == HNBU_HNBUCIS) {
						/*
						 * skip 0xff (end of standard CIS)
						 * after this tuple
						 */
						tlen++;
						standard_cis = FALSE;
					}
				}
			} else {
				if (cis[i] == CISTPL_NULL || cis[i] == CISTPL_END) {
					tlen = 0;
					tup = cis[i];
				} else {
					tlen = cis[i];
					tup = CISTPL_BRCM_HNBU;
				}
				++i;
			}
			if ((i + tlen) >= CIS_SIZE)
				break;

			switch (tup) {
			case CISTPL_VERS_1: FROMHOST();
				/* assume the strings are good if the version field checks out */
				if (((cis[i + 1] << 8) + cis[i]) >= 0x0008) {
					varbuf_append(&b, vstr_manf, &cis[i + 2]);
					varbuf_append(&b, vstr_productname,
					              &cis[i + 3 + strlen((char *)&cis[i + 2])]);
					break;
				}

			case CISTPL_MANFID: FROMHOST();
				varbuf_append(&b, vstr_manfid, (cis[i + 1] << 8) + cis[i]);
				varbuf_append(&b, vstr_prodid, (cis[i + 3] << 8) + cis[i + 2]);
				break;

			case CISTPL_FUNCID: FROMHOST();
				funcid = cis[i];
				break;

			case CISTPL_FUNCE: FROMHOST();
				switch (funcid) {
				default: FROMHOST();
					/* set macaddr if HNBU_MACADDR not seen yet */
					if (eabuf[0] == '\0' && cis[i] == LAN_NID) {
						ASSERT(cis[i + 1] == ETHER_ADDR_LEN);
						bcm_ether_ntoa((struct ether_addr *)&cis[i + 2],
						               eabuf);
					}
					/* set boardnum if HNBU_BOARDNUM not seen yet */
					if (boardnum == -1)
						boardnum = (cis[i + 6] << 8) + cis[i + 7];
					break;
				}
				break;

			case CISTPL_CFTABLE: FROMHOST();
				varbuf_append(&b, vstr_regwindowsz, (cis[i + 7] << 8) | cis[i + 6]);
				break;

			case CISTPL_BRCM_HNBU:
				switch (cis[i]) {
				case HNBU_SROMREV:
					sromrev = cis[i + 1];
					varbuf_append(&b, vstr_sromrev, sromrev);
					break;

				case HNBU_CHIPID: FROMHOST();
					varbuf_append(&b, vstr_vendid, (cis[i + 2] << 8) +
					              cis[i + 1]);
					varbuf_append(&b, vstr_devid, (cis[i + 4] << 8) +
					              cis[i + 3]);
					if (tlen >= 7) {
						varbuf_append(&b, vstr_chiprev,
						              (cis[i + 6] << 8) + cis[i + 5]);
					}
					if (tlen >= 9) {
						varbuf_append(&b, vstr_subvendid,
						              (cis[i + 8] << 8) + cis[i + 7]);
					}
					if (tlen >= 11) {
						varbuf_append(&b, vstr_subdevid,
						              (cis[i + 10] << 8) + cis[i + 9]);
						/* subdevid doubles for boardtype */
						varbuf_append(&b, vstr_boardtype,
						              (cis[i + 10] << 8) + cis[i + 9]);
					}
					break;

				case HNBU_BOARDREV: FROMHOST();
					if (tlen == 2)
						varbuf_append(&b, vstr_boardrev, cis[i + 1]);
					else
						varbuf_append(&b, vstr_boardrev,
							(cis[i + 2] << 8) + cis[i + 1]);
					break;

				case HNBU_AA: FROMHOST();
					varbuf_append(&b, vstr_aa2g, cis[i + 1]);
					if (tlen >= 3)
						varbuf_append(&b, vstr_aa5g, cis[i + 2]);
					break;

				case HNBU_AG:
					varbuf_append(&b, vstr_ag, 0, cis[i + 1]);
					if (tlen >= 3)
						varbuf_append(&b, vstr_ag, 1, cis[i + 2]);
					if (tlen >= 4)
						varbuf_append(&b, vstr_ag, 2, cis[i + 3]);
					if (tlen >= 5)
						varbuf_append(&b, vstr_ag, 3, cis[i + 4]);
					ag_init = TRUE;
					break;

				case HNBU_ANT5G:
					varbuf_append(&b, vstr_aa5g, cis[i + 1]);
					varbuf_append(&b, vstr_ag, 1, cis[i + 2]);
					break;

				case HNBU_CC:
					ASSERT(sromrev == 1);
					varbuf_append(&b, vstr_cc, cis[i + 1]);
					break;

				case HNBU_PAPARMS:
					if (tlen == 2) {
						ASSERT(sromrev == 1);
						varbuf_append(&b, vstr_pa0maxpwr, cis[i + 1]);
					} else if (tlen >= 9) {
						if (tlen == 10) {
							ASSERT(sromrev >= 2);
							varbuf_append(&b, vstr_opo, cis[i + 9]);
						} else
							ASSERT(tlen == 9);

						for (j = 0; j < 3; j++) {
							varbuf_append(&b, vstr_pa0b[j],
							              (cis[i + (j * 2) + 2] << 8) +
							              cis[i + (j * 2) + 1]);
						}
						varbuf_append(&b, vstr_pa0itssit, cis[i + 7]);
						varbuf_append(&b, vstr_pa0maxpwr, cis[i + 8]);
					} else
						ASSERT(tlen >= 9);
					break;

				case HNBU_PAPARMS5G:
					ASSERT((sromrev == 2) || (sromrev == 3));
					for (j = 0; j < 3; j++) {
						varbuf_append(&b, vstr_pa1b[j],
							(cis[i + (j * 2) + 2] << 8) +
							cis[i + (j * 2) + 1]);
					}
					for (j = 3; j < 6; j++) {
						varbuf_append(&b, vstr_pa1lob[j - 3],
							(cis[i + (j * 2) + 2] << 8) +
							cis[i + (j * 2) + 1]);
					}
					for (j = 6; j < 9; j++) {
						varbuf_append(&b, vstr_pa1hib[j - 6],
							(cis[i + (j * 2) + 2] << 8) +
							cis[i + (j * 2) + 1]);
					}
					varbuf_append(&b, vstr_pa1itssit, cis[i + 19]);
					varbuf_append(&b, vstr_pa1maxpwr, cis[i + 20]);
					varbuf_append(&b, vstr_pa1lomaxpwr, cis[i + 21]);
					varbuf_append(&b, vstr_pa1himaxpwr, cis[i + 22]);
					break;

				case HNBU_OEM: FROMHOST();
					ASSERT(sromrev == 1);
					varbuf_append(&b, vstr_oem,
					              cis[i + 1], cis[i + 2],
					              cis[i + 3], cis[i + 4],
					              cis[i + 5], cis[i + 6],
					              cis[i + 7], cis[i + 8]);
					break;

				case HNBU_BOARDFLAGS: FROMHOST();
					w32 = (cis[i + 2] << 8) + cis[i + 1];
					if (tlen >= 5)
						w32 |= ((cis[i + 4] << 24) + (cis[i + 3] << 16));
					varbuf_append(&b, vstr_boardflags, w32);

					if (tlen >= 7) {
						w32 = (cis[i + 6] << 8) + cis[i + 5];
						if (tlen >= 9)
							w32 |= ((cis[i + 8] << 24) +
								(cis[i + 7] << 16));
						varbuf_append(&b, vstr_boardflags2, w32);
					}
					break;

				case HNBU_LEDS: FROMHOST();
					for (j = 1; j <= 4; j++) {
						if (cis[i + j] != 0xff) {
							varbuf_append(&b, vstr_ledbh, j-1,
							cis[i + j]);
						}
					}
					break;

				case HNBU_CCODE:
					ASSERT(sromrev > 1);
					if ((cis[i + 1] == 0) || (cis[i + 2] == 0))
						varbuf_append(&b, vstr_noccode);
					else
						varbuf_append(&b, vstr_ccode,
						              cis[i + 1], cis[i + 2]);
					varbuf_append(&b, vstr_cctl, cis[i + 3]);
					break;

				case HNBU_CCKPO:
					ASSERT(sromrev > 2);
					varbuf_append(&b, vstr_cckpo,
					              (cis[i + 2] << 8) | cis[i + 1]);
					break;

				case HNBU_OFDMPO:
					ASSERT(sromrev > 2);
					varbuf_append(&b, vstr_ofdmpo,
					              (cis[i + 4] << 24) |
					              (cis[i + 3] << 16) |
					              (cis[i + 2] << 8) |
					              cis[i + 1]);
					break;

				case HNBU_RDLID: FROMHOST();
					varbuf_append(&b, vstr_rdlid,
					              (cis[i + 2] << 8) | cis[i + 1]);
					break;

				case HNBU_RDLRNDIS: FROMHOST();
					varbuf_append(&b, vstr_rdlrndis, cis[i + 1]);
					break;

				case HNBU_RDLRWU: FROMHOST();
					varbuf_append(&b, vstr_rdlrwu, cis[i + 1]);
					break;

				case HNBU_WPS: FROMHOST();
					varbuf_append(&b, vstr_wpsgpio, cis[i + 1]);
					break;

				case HNBU_USBFS: FROMHOST();
					varbuf_append(&b, vstr_usbfs, cis[i + 1]);
					break;

				case HNBU_RDLSN: FROMHOST();
					varbuf_append(&b, vstr_rdlsn,
					              (cis[i + 2] << 8) | cis[i + 1]);
					break;

				case HNBU_XTALFREQ: FROMHOST();
					varbuf_append(&b, vstr_xtalfreq,
					              (cis[i + 4] << 24) |
					              (cis[i + 3] << 16) |
					              (cis[i + 2] << 8) |
					              cis[i + 1]);
					break;

				case HNBU_RSSISMBXA2G:
					ASSERT(sromrev == 3);
					varbuf_append(&b, vstr_rssismf2g, cis[i + 1] & 0xf);
					varbuf_append(&b, vstr_rssismc2g, (cis[i + 1] >> 4) & 0xf);
					varbuf_append(&b, vstr_rssisav2g, cis[i + 2] & 0x7);
					varbuf_append(&b, vstr_bxa2g, (cis[i + 2] >> 3) & 0x3);
					break;

				case HNBU_RSSISMBXA5G:
					ASSERT(sromrev == 3);
					varbuf_append(&b, vstr_rssismf5g, cis[i + 1] & 0xf);
					varbuf_append(&b, vstr_rssismc5g, (cis[i + 1] >> 4) & 0xf);
					varbuf_append(&b, vstr_rssisav5g, cis[i + 2] & 0x7);
					varbuf_append(&b, vstr_bxa5g, (cis[i + 2] >> 3) & 0x3);
					break;

				case HNBU_TRI2G: FROMHOST();
					ASSERT(sromrev == 3);
					varbuf_append(&b, vstr_tri2g, cis[i + 1]);
					break;

				case HNBU_TRI5G: FROMHOST();
					ASSERT(sromrev == 3);
					varbuf_append(&b, vstr_tri5gl, cis[i + 1]);
					varbuf_append(&b, vstr_tri5g, cis[i + 2]);
					varbuf_append(&b, vstr_tri5gh, cis[i + 3]);
					break;

				case HNBU_RXPO2G:
					ASSERT(sromrev == 3);
					varbuf_append(&b, vstr_rxpo2g, cis[i + 1]);
					break;

				case HNBU_RXPO5G:
					ASSERT(sromrev == 3);
					varbuf_append(&b, vstr_rxpo5g, cis[i + 1]);
					break;

				case HNBU_BOARDNUM: FROMHOST();
					boardnum = (cis[i + 2] << 8) + cis[i + 1];
					break;

				case HNBU_MACADDR: FROMHOST();
					bcm_ether_ntoa((struct ether_addr *)&cis[i + 1],
					               eabuf);
					if (boardnum == -1)
						boardnum = (cis[i + 5] << 8) + cis[i + 6];
					break;

				case HNBU_BOARDTYPE: FROMHOST();
					varbuf_append(&b, vstr_boardtype,
					              (cis[i + 2] << 8) + cis[i + 1]);
					break;

				case HNBU_LEDDC: FROMHOST();
					w32 = ((cis[i + 2] << 24) | /* oncount */
					       (cis[i + 1] << 8)); /* offcount */
					varbuf_append(&b, vstr_leddc, w32);
					break;

				case HNBU_CHAINSWITCH:
					varbuf_append(&b, vstr_txchain, cis[i + 1]);
					varbuf_append(&b, vstr_rxchain, cis[i + 2]);
					varbuf_append(&b, vstr_antswitch,
					      (cis[i + 4] << 8) + cis[i + 3]);
					break;

				case HNBU_REGREV:
					varbuf_append(&b, vstr_regrev, cis[i + 1]);
					break;

				case HNBU_FEM: {
					uint16 fem = (cis[i + 2] << 8) + cis[i + 1];
					varbuf_append(&b, vstr_antswctl2g, (fem &
						SROM8_FEM_ANTSWLUT_MASK) >>
						SROM8_FEM_ANTSWLUT_SHIFT);
					varbuf_append(&b, vstr_triso2g, (fem &
						SROM8_FEM_TR_ISO_MASK) >>
						SROM8_FEM_TR_ISO_SHIFT);
					varbuf_append(&b, vstr_pdetrange2g, (fem &
						SROM8_FEM_PDET_RANGE_MASK) >>
						SROM8_FEM_PDET_RANGE_SHIFT);
					varbuf_append(&b, vstr_extpagain2g, (fem &
						SROM8_FEM_EXTPA_GAIN_MASK) >>
						SROM8_FEM_EXTPA_GAIN_SHIFT);
					varbuf_append(&b, vstr_tssipos2g, (fem &
						SROM8_FEM_TSSIPOS_MASK) >>
						SROM8_FEM_TSSIPOS_SHIFT);
					if (tlen < 5) break;

					fem = (cis[i + 4] << 8) + cis[i + 3];
					varbuf_append(&b, vstr_antswctl5g, (fem &
						SROM8_FEM_ANTSWLUT_MASK) >>
						SROM8_FEM_ANTSWLUT_SHIFT);
					varbuf_append(&b, vstr_triso5g, (fem &
						SROM8_FEM_TR_ISO_MASK) >>
						SROM8_FEM_TR_ISO_SHIFT);
					varbuf_append(&b, vstr_pdetrange5g, (fem &
						SROM8_FEM_PDET_RANGE_MASK) >>
						SROM8_FEM_PDET_RANGE_SHIFT);
					varbuf_append(&b, vstr_extpagain5g, (fem &
						SROM8_FEM_EXTPA_GAIN_MASK) >>
						SROM8_FEM_EXTPA_GAIN_SHIFT);
					varbuf_append(&b, vstr_tssipos5g, (fem &
						SROM8_FEM_TSSIPOS_MASK) >>
						SROM8_FEM_TSSIPOS_SHIFT);
					break;
					}

				case HNBU_PAPARMS_C0:
					varbuf_append(&b, vstr_maxp2ga0, cis[i + 1]);
					varbuf_append(&b, vstr_itt2ga0, cis[i + 2]);
					varbuf_append(&b, vstr_pa, 2, 0, 0,
						(cis[i + 4] << 8) + cis[i + 3]);
					varbuf_append(&b, vstr_pa, 2, 1, 0,
						(cis[i + 6] << 8) + cis[i + 5]);
					varbuf_append(&b, vstr_pa, 2, 2, 0,
						(cis[i + 8] << 8) + cis[i + 7]);
					if (tlen < 31) break;

					varbuf_append(&b, vstr_maxp5ga0, cis[i + 9]);
					varbuf_append(&b, vstr_itt5ga0, cis[i + 10]);
					varbuf_append(&b, vstr_maxp5gha0, cis[i + 11]);
					varbuf_append(&b, vstr_maxp5gla0, cis[i + 12]);
					varbuf_append(&b, vstr_pa, 5, 0, 0,
						(cis[i + 14] << 8) + cis[i + 13]);
					varbuf_append(&b, vstr_pa, 5, 1, 0,
						(cis[i + 16] << 8) + cis[i + 15]);
					varbuf_append(&b, vstr_pa, 5, 2, 0,
						(cis[i + 18] << 8) + cis[i + 17]);
					varbuf_append(&b, vstr_pahl, 5, 'l', 0, 0,
						(cis[i + 20] << 8) + cis[i + 19]);
					varbuf_append(&b, vstr_pahl, 5, 'l', 1, 0,
						(cis[i + 22] << 8) + cis[i + 21]);
					varbuf_append(&b, vstr_pahl, 5, 'l', 2, 0,
						(cis[i + 24] << 8) + cis[i + 23]);
					varbuf_append(&b, vstr_pahl, 5, 'h', 0, 0,
						(cis[i + 26] << 8) + cis[i + 25]);
					varbuf_append(&b, vstr_pahl, 5, 'h', 1, 0,
						(cis[i + 28] << 8) + cis[i + 27]);
					varbuf_append(&b, vstr_pahl, 5, 'h', 2, 0,
						(cis[i + 30] << 8) + cis[i + 29]);
					break;

				case HNBU_PAPARMS_C1:
					varbuf_append(&b, vstr_maxp2ga1, cis[i + 1]);
					varbuf_append(&b, vstr_itt2ga1, cis[i + 2]);
					varbuf_append(&b, vstr_pa, 2, 0, 1,
						(cis[i + 4] << 8) + cis[i + 3]);
					varbuf_append(&b, vstr_pa, 2, 1, 1,
						(cis[i + 6] << 8) + cis[i + 5]);
					varbuf_append(&b, vstr_pa, 2, 2, 1,
						(cis[i + 8] << 8) + cis[i + 7]);
					if (tlen < 31) break;

					varbuf_append(&b, vstr_maxp5ga1, cis[i + 9]);
					varbuf_append(&b, vstr_itt5ga1, cis[i + 10]);
					varbuf_append(&b, vstr_maxp5gha1, cis[i + 11]);
					varbuf_append(&b, vstr_maxp5gla1, cis[i + 12]);
					varbuf_append(&b, vstr_pa, 5, 0, 1,
						(cis[i + 14] << 8) + cis[i + 13]);
					varbuf_append(&b, vstr_pa, 5, 1, 1,
						(cis[i + 16] << 8) + cis[i + 15]);
					varbuf_append(&b, vstr_pa, 5, 2, 1,
						(cis[i + 18] << 8) + cis[i + 17]);
					varbuf_append(&b, vstr_pahl, 5, 'l', 0, 1,
						(cis[i + 20] << 8) + cis[i + 19]);
					varbuf_append(&b, vstr_pahl, 5, 'l', 1, 1,
						(cis[i + 22] << 8) + cis[i + 21]);
					varbuf_append(&b, vstr_pahl, 5, 'l', 2, 1,
						(cis[i + 24] << 8) + cis[i + 23]);
					varbuf_append(&b, vstr_pahl, 5, 'h', 0, 1,
						(cis[i + 26] << 8) + cis[i + 25]);
					varbuf_append(&b, vstr_pahl, 5, 'h', 1, 1,
						(cis[i + 28] << 8) + cis[i + 27]);
					varbuf_append(&b, vstr_pahl, 5, 'h', 2, 1,
						(cis[i + 30] << 8) + cis[i + 29]);
					break;

				case HNBU_PO_CCKOFDM:
					varbuf_append(&b, vstr_cck2gpo,
						(cis[i + 2] << 8) + cis[i + 1]);
					varbuf_append(&b, vstr_ofdm2gpo,
						(cis[i + 6] << 24) + (cis[i + 5] << 16) +
						(cis[i + 4] << 8) + cis[i + 3]);
					if (tlen < 19) break;

					varbuf_append(&b, vstr_ofdm5gpo,
						(cis[i + 10] << 24) + (cis[i + 9] << 16) +
						(cis[i + 8] << 8) + cis[i + 7]);
					varbuf_append(&b, vstr_ofdm5glpo,
						(cis[i + 14] << 24) + (cis[i + 13] << 16) +
						(cis[i + 12] << 8) + cis[i + 11]);
					varbuf_append(&b, vstr_ofdm5ghpo,
						(cis[i + 18] << 24) + (cis[i + 17] << 16) +
						(cis[i + 16] << 8) + cis[i + 15]);
					break;

				case HNBU_PO_MCS2G:
					for (j = 0; j <= 7; j++) {
						varbuf_append(&b, vstr_mcspo, 2, j,
							(cis[i + 2 + 2*j] << 8) + cis[i + 1 + 2*j]);
					}
					break;

				case HNBU_PO_MCS5GM:
					for (j = 0; j <= 7; j++) {
						varbuf_append(&b, vstr_mcspo, 5, j,
							(cis[i + 2 + 2*j] << 8) + cis[i + 1 + 2*j]);
					}
					break;

				case HNBU_PO_MCS5GLH:
					for (j = 0; j <= 7; j++) {
						varbuf_append(&b, vstr_mcspohl, 5, 'l', j,
							(cis[i + 2 + 2*j] << 8) + cis[i + 1 + 2*j]);
					}

					for (j = 0; j <= 7; j++) {
						varbuf_append(&b, vstr_mcspohl, 5, 'h', j,
							(cis[i + 18 + 2*j] << 8) +
							cis[i + 17 + 2*j]);
					}

					break;

				case HNBU_CUSTOM1:
					varbuf_append(&b, vstr_custom, 1, ((cis[i + 4] << 24) +
						(cis[i + 3] << 16) + (cis[i + 2] << 8) +
						cis[i + 1]));
					break;
#if defined(BCMCCISSR3)
				case HNBU_SROM3SWRGN: FROMHOST();
					if (tlen >= 73) {
						uint16 srom[35];
						uint8 srev = cis[i + 1 + 70];
						ASSERT(srev == 3);
						/* make tuple value 16-bit aligned and parse it */
						bcopy(&cis[i + 1], srom, sizeof(srom));
						_initvars_srom_pci(srev, srom, SROM3_SWRGN_OFF, &b);
						/* 2.4G antenna gain is included in SROM */
						ag_init = TRUE;
						/* Ethernet MAC address is included in SROM */
						eabuf[0] = 0;
						boardnum = -1;
					}
					/* create extra variables */
					if (tlen >= 75)
						varbuf_append(&b, vstr_vendid,
						              (cis[i + 1 + 73] << 8) +
						              cis[i + 1 + 72]);
					if (tlen >= 77)
						varbuf_append(&b, vstr_devid,
						              (cis[i + 1 + 75] << 8) +
						              cis[i + 1 + 74]);
					if (tlen >= 79)
						varbuf_append(&b, vstr_xtalfreq,
						              (cis[i + 1 + 77] << 8) +
						              cis[i + 1 + 76]);
					break;
#endif	
				}
				break;
			}
			i += tlen;
		} while (tup != CISTPL_END);
	}

	if (boardnum != -1) {
		varbuf_append(&b, vstr_boardnum, boardnum);
	}

	if (eabuf[0]) {
		varbuf_append(&b, vstr_macaddr, eabuf);
	}

	/* if there is no antenna gain field, set default */
	if (getvar(NULL, "ag0") == NULL && ag_init == FALSE) {
		varbuf_append(&b, vstr_ag, 0, 0xff);
	}

#ifdef USB4322
	varbuf_append(&b, vstr_end, NULL);
#endif /* USB4322 */

	/* final nullbyte terminator */
	ASSERT(b.size >= 1);
	*b.buf++ = '\0';

	ASSERT(b.buf - base <= MAXSZ_NVRAM_VARS);

	err = initvars_table(osh, base, b.buf, vars, count);

	MFREE(osh, base, MAXSZ_NVRAM_VARS);
	return err;
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

/* In chips with chipcommon rev 32 and later, the srom is in chipcommon,
 * not in the bus cores.
 */
static uint16
srom_cc_cmd(si_t *sih, osl_t *osh, void *ccregs, uint32 cmd, uint wordoff, uint16 data)
{
	chipcregs_t *cc = (chipcregs_t *)ccregs;
	uint wait_cnt = 1000;

	if ((cmd == SRC_OP_READ) || (cmd == SRC_OP_WRITE)) {
		W_REG(osh, &cc->sromaddress, wordoff * 2);
		if (cmd == SRC_OP_WRITE)
			W_REG(osh, &cc->sromdata, data);
	}

	W_REG(osh, &cc->sromcontrol, SRC_START | cmd);

	while (wait_cnt--) {
		if ((R_REG(osh, &cc->sromcontrol) & SRC_BUSY) == 0)
			break;
	}

	if (wait_cnt) {
		BS_ERROR(("%s: Command 0x%x timed out\n", __FUNCTION__, cmd));
		return 0xffff;
	}
	if (cmd == SRC_OP_READ)
		return (uint16)R_REG(osh, &cc->sromdata);
	else
		return 0xffff;
}

/*
 * Read in and validate sprom.
 * Return 0 on success, nonzero on error.
 */
static int
sprom_read_pci(osl_t *osh, si_t *sih, uint16 *sprom, uint wordoff, uint16 *buf, uint nwords,
	bool check_crc)
{
	int err = 0;
	uint i;

	/* read the sprom */
	for (i = 0; i < nwords; i++) {
		if (ISSIM_ENAB(sih)) {
			buf[i] = R_REG(osh, &sprom[wordoff + i]);
		}
		buf[i] = R_REG(osh, &sprom[wordoff + i]);
	}

	/* bypass crc checking for simulation to allow srom hack */
	if (ISSIM_ENAB(sih))
		return err;

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

#if defined(BCMNVRAMW) || defined(BCMNVRAMR)
static int
BCMATTACHFN(otp_read_pci)(osl_t *osh, si_t *sih, uint16 *buf, uint bufsz)
{
	uint8 *otp;
	uint sz = OTP_SZ_MAX/2; /* size in words */
	int err = 0;

	if ((otp = MALLOC(osh, OTP_SZ_MAX)) == NULL) {
		return BCME_ERROR;
	}

	bzero(otp, OTP_SZ_MAX);

	err = otp_read_region(sih, OTP_HW_RGN, (uint16 *)otp, &sz);

	bcopy(otp, buf, bufsz);

	if (otp)
		MFREE(osh, otp, OTP_SZ_MAX);

	/* Check CRC */
	if (buf[0] == 0xffff) {
		/* The hardware thinks that an srom that starts with 0xffff
		 * is blank, regardless of the rest of the content, so declare
		 * it bad.
		 */
		BS_ERROR(("%s: buf[0] = 0x%x, returning bad-crc\n", __FUNCTION__, buf[0]));
		return 1;
	}

	/* fixup the endianness so crc8 will pass */
	htol16_buf(buf, bufsz);
	if (hndcrc8((uint8 *)buf, SROM4_WORDS * 2, CRC8_INIT_VALUE) != CRC8_GOOD_VALUE)
		err = 1;
	/* now correct the endianness of the byte array */
	ltoh16_buf(buf, bufsz);

	return err;
}
#endif /* defined(BCMNVRAMW) || defined(BCMNVRAMR) */

#if defined(WLTEST)
int
srom_otp_write_region_crc(si_t *sih, uint nbytes, uint16* buf16, bool write)
{
	int err = 0, crc = 0;
	uint8 *buf8;

	/* Check nbytes is not odd or too big */
	if ((nbytes & 1) || (nbytes > SROM_MAX))
		return 1;

	/* block invalid buffer size */
	if (nbytes < SROM4_WORDS * 2)
		return BCME_BUFTOOSHORT;
	else if (nbytes > SROM4_WORDS * 2)
		return BCME_BUFTOOLONG;

	/* Verify signatures */
	if (!((buf16[SROM4_SIGN] == SROM4_SIGNATURE) ||
		(buf16[SROM8_SIGN] == SROM4_SIGNATURE))) {
		BS_ERROR(("%s: wrong signature SROM4_SIGN %x SROM8_SIGN %x\n",
			__FUNCTION__, buf16[SROM4_SIGN], buf16[SROM8_SIGN]));
		return BCME_ERROR;
	}

	/* Check CRC */
	if (buf16[0] == 0xffff) {
		/* The hardware thinks that an srom that starts with 0xffff
		 * is blank, regardless of the rest of the content, so declare
		 * it bad.
		 */
		BS_ERROR(("%s: invalid buf16[0] = 0x%x\n", __FUNCTION__, buf16[0]));
		goto out;
	}

	buf8 = (uint8*)buf16;
	/* fixup the endianness and then calculate crc */
	htol16_buf(buf8, nbytes);
	crc = ~hndcrc8(buf8, nbytes - 1, CRC8_INIT_VALUE);
	/* now correct the endianness of the byte array */
	ltoh16_buf(buf8, nbytes);
	buf16[SROM4_CRCREV] = (crc << 8) | (buf16[SROM4_CRCREV] & 0xff);

#ifdef BCMNVRAMW
	/* Write the CRC back */
	if (write)
		err = otp_cis_append_region(sih, OTP_HW_RGN, (char*)buf8, (int)nbytes);
#endif /* BCMNVRAMW */

out:
	return write ? err : crc;
}
#endif 

/*
* Create variable table from memory.
* Return 0 on success, nonzero on error.
*/
static int
BCMNMIATTACHFN(initvars_table)(osl_t *osh, char *start, char *end, char **vars, uint *count)
{
	int c = (int)(end - start);

	/* do it only when there is more than just the null string */
	if (c > 1) {
		char *vp = MALLOC(osh, c);
		ASSERT(vp != NULL);
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
initvars_flash(si_t *sih, osl_t *osh, char **base, uint len)
{
	char *vp = *base;
	char *flash;
	int err;
	char *s;
	uint l, dl, copy_len;
	char devpath[SI_DEVPATH_BUFSZ];

	/* allocate memory and read in flash */
	if (!(flash = MALLOC(osh, NVRAM_SPACE)))
		return BCME_NOMEM;
	if ((err = nvram_getall(flash, NVRAM_SPACE)))
		goto exit;

	si_devpath(sih, devpath, sizeof(devpath));

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
		strncpy(vp, &s[dl], copy_len);
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

#if !defined(BCMUSBDEV)
/*
 * Initialize nonvolatile variable table from flash.
 * Return 0 on success, nonzero on error.
 */
static int
BCMATTACHFN(initvars_flash_si)(si_t *sih, char **vars, uint *count)
{
	osl_t *osh = si_osh(sih);
	char *vp, *base;
	int err;

	ASSERT(vars != NULL);
	ASSERT(count != NULL);

	base = vp = MALLOC(osh, MAXSZ_NVRAM_VARS);
	ASSERT(vp != NULL);
	if (!vp)
		return BCME_NOMEM;

	if ((err = initvars_flash(sih, osh, &vp, MAXSZ_NVRAM_VARS)) == 0)
		err = initvars_table(osh, base, vp, vars, count);

	MFREE(osh, base, MAXSZ_NVRAM_VARS);

	return err;
}
#endif	

/* Parse SROM and create name=value pairs. 'srom' points to
 * the SROM word array. 'off' specifies the offset of the
 * first word 'srom' points to, which should be either 0 or
 * SROM3_SWRG_OFF (full SROM or software region).
 */

static uint
mask_shift(uint16 mask)
{
	uint i;
	for (i = 0; i < (sizeof(mask) << 3); i ++) {
		if (mask & (1 << i))
			return i;
	}
	ASSERT(mask);
	return 0;
}

static uint
mask_width(uint16 mask)
{
	int i;
	for (i = (sizeof(mask) << 3) - 1; i >= 0; i --) {
		if (mask & (1 << i))
			return (uint)(i - mask_shift(mask) + 1);
	}
	ASSERT(mask);
	return 0;
}


static void
BCMATTACHFN(_initvars_srom_pci)(uint8 sromrev, uint16 *srom, uint off, varbuf_t *b)
{
	uint16 w;
	uint32 val;
	const sromvar_t *srv;
	uint width;
	uint flags;
	uint32 sr = (1 << sromrev);

	varbuf_append(b, "sromrev=%d", sromrev);

	for (srv = pci_sromvars; srv->name != NULL; srv ++) {
		const char *name;

		if ((srv->revmask & sr) == 0)
			continue;

		if (srv->off < off)
			continue;

		flags = srv->flags;
		name = srv->name;

		if (flags & SRFL_ETHADDR) {
			char eabuf[ETHER_ADDR_STR_LEN];
			struct ether_addr ea;

			ea.octet[0] = (srom[srv->off - off] >> 8) & 0xff;
			ea.octet[1] = srom[srv->off - off] & 0xff;
			ea.octet[2] = (srom[srv->off + 1 - off] >> 8) & 0xff;
			ea.octet[3] = srom[srv->off + 1 - off] & 0xff;
			ea.octet[4] = (srom[srv->off + 2 - off] >> 8) & 0xff;
			ea.octet[5] = srom[srv->off + 2 - off] & 0xff;
			bcm_ether_ntoa(&ea, eabuf);

			varbuf_append(b, "%s=%s", name, eabuf);
		}
		else {
			ASSERT(mask_valid(srv->mask));
			ASSERT(mask_width(srv->mask));

			w = srom[srv->off - off];
			val = (w & srv->mask) >> mask_shift(srv->mask);
			width = mask_width(srv->mask);

			while (srv->flags & SRFL_MORE) {
				srv ++;
				ASSERT(srv->name != NULL);

				if (srv->off == 0 || srv->off < off)
					continue;

				ASSERT(mask_valid(srv->mask));
				ASSERT(mask_width(srv->mask));

				w = srom[srv->off - off];
				val += ((w & srv->mask) >> mask_shift(srv->mask)) << width;
				width += mask_width(srv->mask);
			}

			if ((flags & SRFL_NOFFS) && ((int)val == (1 << width) - 1))
				continue;

			if (flags & SRFL_CCODE) {
				if (val == 0)
					varbuf_append(b, "ccode=");
				else
					varbuf_append(b, "ccode=%c%c", (val >> 8), (val & 0xff));
			}
			/* LED Powersave duty cycle has to be scaled:
			 *(oncount >> 24) (offcount >> 8)
			 */
			else if (flags & SRFL_LEDDC) {
				uint32 w32 = (((val >> 8) & 0xff) << 24) | /* oncount */
					     (((val & 0xff)) << 8); /* offcount */
				varbuf_append(b, "leddc=%d", w32);
			}
			else if (flags & SRFL_PRHEX)
				varbuf_append(b, "%s=0x%x", name, val);
			else if ((flags & SRFL_PRSIGN) && (val & (1 << (width - 1))))
				varbuf_append(b, "%s=%d", name, (int)(val | (~0 << width)));
			else
				varbuf_append(b, "%s=%u", name, val);
		}
	}

	if (sromrev >= 4) {
		/* Do per-path variables */
		uint p, pb, psz;

		if (sromrev >= 8) {
			pb = SROM8_PATH0;
			psz = SROM8_PATH1 - SROM8_PATH0;
		} else {
			pb = SROM4_PATH0;
			psz = SROM4_PATH1 - SROM4_PATH0;
		}

		for (p = 0; p < MAX_PATH_SROM; p++) {
			for (srv = perpath_pci_sromvars; srv->name != NULL; srv ++) {
				if ((srv->revmask & sr) == 0)
					continue;

				if (pb + srv->off < off)
					continue;

				w = srom[pb + srv->off - off];
				ASSERT(mask_valid(srv->mask));
				val = (w & srv->mask) >> mask_shift(srv->mask);
				width = mask_width(srv->mask);

				/* Cheating: no per-path var is more than 1 word */

				if ((srv->flags & SRFL_NOFFS) && ((int)val == (1 << width) - 1))
					continue;

				if (srv->flags & SRFL_PRHEX)
					varbuf_append(b, "%s%d=0x%x", srv->name, p, val);
				else
					varbuf_append(b, "%s%d=%d", srv->name, p, val);
			}
			pb += psz;
		}
	}
}

/*
 * Initialize nonvolatile variable table from sprom.
 * Return 0 on success, nonzero on error.
 */
static int
BCMATTACHFN(initvars_srom_pci)(si_t *sih, void *curmap, char **vars, uint *count)
{
	uint16 *srom, *sromwindow;
	uint8 sromrev = 0;
	uint32 sr;
	varbuf_t b;
	char *vp, *base = NULL;
	osl_t *osh = si_osh(sih);
	bool flash = FALSE;
	int err = 0;

	/*
	 * Apply CRC over SROM content regardless SROM is present or not,
	 * and use variable <devpath>sromrev's existance in flash to decide
	 * if we should return an error when CRC fails or read SROM variables
	 * from flash.
	 */
	srom = MALLOC(osh, SROM_MAX);
	ASSERT(srom != NULL);
	if (!srom)
		return -2;

	if (sih->ccrev >= 31) {
		sromwindow = (uint16 *)((uint8 *)curmap + PCI_16KB0_CCREGS_OFFSET + CC_SROM_OTP);
	} else {
		sromwindow = (uint16 *)((uint8 *)curmap + PCI_BAR0_SPROM_OFFSET);
	}

	if (si_is_sprom_available(sih)) {
		err = sprom_read_pci(osh, sih, (void *)((int8 *)curmap + PCI_BAR0_SPROM_OFFSET),
			0, srom, SROM_WORDS, TRUE);

		if ((srom[SROM4_SIGN] == SROM4_SIGNATURE) ||
		    (((sih->buscoretype == PCIE_CORE_ID) && (sih->buscorerev >= 6)) ||
		     ((sih->buscoretype == PCI_CORE_ID) && (sih->buscorerev >= 0xe)))) {
			/* sromrev >= 4, read more */
			err = sprom_read_pci(osh, sih, sromwindow, 0, srom, SROM4_WORDS, TRUE);
			sromrev = srom[SROM4_CRCREV] & 0xff;

		} else if (err == 0) {
			/* srom is good and is rev < 4 */
			/* top word of sprom contains version and crc8 */
			sromrev = srom[SROM_CRCREV] & 0xff;
			/* bcm4401 sroms misprogrammed */
			if (sromrev == 0x10)
				sromrev = 1;
		}
	}
#if defined(BCMNVRAMW) || defined(BCMNVRAMR)
	/* Use OTP if SPROM not available */
	else if ((err = otp_read_pci(osh, sih, srom, SROM_MAX)) == 0) {
		sromrev = srom[SROM4_CRCREV] & 0xff;
	}
#endif /* defined(BCMNVRAMW) || defined(BCMNVRAMR) */
	else {
		BS_ERROR(("Neither SPROM nor OTP has valid image\n"));
	}

	/* We want internal/wltest driver to come up with default sromvars so we can
	 * program a blank SPROM/OTP.
	 */
	if (err) {
		char *value;
#if defined(WLTEST)
		uint32 val;
#endif 

		if ((value = si_getdevpathvar(sih, "sromrev"))) {
			sromrev = (uint8)bcm_strtoul(value, NULL, 0);
			flash = TRUE;
			goto varscont;
		}

		BS_ERROR(("SROM CRC Error\n"));

#if defined(WLTEST)
		val = OSL_PCI_READ_CONFIG(osh, PCI_SPROM_CONTROL, sizeof(uint32));
		if ((si_is_sprom_available(sih) && srom[0] == 0xffff) ||
			(val & SPROM_OTPIN_USE)) {
			vp = base = mfgsromvars;

			if (defvarslen == 0) {
				BS_ERROR(("No nvm file, use generic default (for programming"
					" SPROM/OTP only)\n"));
				/* For 4311 A1 there is no signature to indicate that OTP is
				 * programmed, so can't really verify the OTP is unprogrammed
				 * or a bad OTP.
				 */
				if (sih->chip == BCM4311_CHIP_ID) {
					const char *devid = "devid=0x4311";
					const size_t devid_strlen = strlen(devid);
					BS_ERROR(("setting the devid to be 4311\n"));
					bcopy(devid, vp, devid_strlen + 1);
					vp += devid_strlen + 1;
				}
				defvarslen = srom_vars_len(defaultsromvars_wltest);
				bcopy(defaultsromvars_wltest, vp, defvarslen);
			} else {
				BS_ERROR(("Use nvm file as default\n"));
			}

			vp += defvarslen;
			BS_ERROR(("Used %d bytes of defaultsromvars\n", defvarslen));
			goto varsdone;
		} else
#endif 
		{
			err = -1;
			goto errout;
		}
	}

varscont:
	/* Bitmask for the sromrev */
	sr = 1 << sromrev;

	/* srom version check
	 * Current valid versions: 1, 2, 3, 4, 5, 8
	 */
	if ((sr & 0x13e) == 0) {
		err = -2;
		goto errout;
	}

	ASSERT(vars != NULL);
	ASSERT(count != NULL);

	base = vp = MALLOC(osh, MAXSZ_NVRAM_VARS);
	ASSERT(vp != NULL);
	if (!vp) {
		err = -2;
		goto errout;
	}

	/* read variables from flash */
	if (flash) {
		if ((err = initvars_flash(sih, osh, &vp, MAXSZ_NVRAM_VARS)))
			goto errout;
		goto varsdone;
	}

	varbuf_init(&b, base, MAXSZ_NVRAM_VARS);

	/* parse SROM into name=value pairs. */
	_initvars_srom_pci(sromrev, srom, 0, &b);

	/* final nullbyte terminator */
	ASSERT(b.size >= 1);
	vp = b.buf;
	*vp++ = '\0';

	ASSERT((vp - base) <= MAXSZ_NVRAM_VARS);

varsdone:
	err = initvars_table(osh, base, vp, vars, count);

errout:
#if defined(WLTEST)
	if (base && (base != mfgsromvars))
#else
	if (base)
#endif
		MFREE(osh, base, MAXSZ_NVRAM_VARS);

	MFREE(osh, srom, SROM_MAX);
	return err;
}

/*
 * Read the cis and call parsecis to initialize the vars.
 * Return 0 on success, nonzero on error.
 */
static int
BCMATTACHFN(initvars_cis_pcmcia)(si_t *sih, osl_t *osh, char **vars, uint *count)
{
	uint8 *cis = NULL;
	int rc;
	uint data_sz;

	data_sz = (sih->buscorerev == 1) ? SROM_MAX : CIS_SIZE;

	if ((cis = MALLOC(osh, data_sz)) == NULL)
		return (-2);

	if (sih->buscorerev == 1) {
		if (srom_read(sih, PCMCIA_BUS, (void *)NULL, osh, 0, data_sz, (uint16 *)cis,
		              TRUE)) {
			MFREE(osh, cis, data_sz);
			return (-1);
		}
		/* fix up endianess for 16-bit data vs 8-bit parsing */
		htol16_buf((uint16 *)cis, data_sz);
	} else
		OSL_PCMCIA_READ_ATTR(osh, 0, cis, data_sz);

	rc = srom_parsecis(osh, &cis, 1, vars, count);

	MFREE(osh, cis, data_sz);

	return (rc);
}


#ifdef USB4322
/*
 * Read the USB cis and call parsecis to initialize the vars.
 * Return 0 on success, nonzero on error.
 */
static int
BCMATTACHFN(initvars_cis_usb)(si_t *sih, osl_t *osh, char **vars, uint *count)
{
	uint8 *cis;
	uint sz = OTP_SZ_MAX/2; /* size in words */
	int rc = 0;

	if ((cis = MALLOC(osh, OTP_SZ_MAX)) == NULL) {
		return -1;
	}

	bzero(cis, OTP_SZ_MAX);

	if (otp_read_region(sih, OTP_SW_RGN, (uint16 *)cis, &sz)) {
		BS_ERROR(("%s: OTP read SW region failure.\n*", __FUNCTION__));
		rc = -2;
	} else {
		BS_ERROR(("%s: OTP programmed. use OTP for srom vars\n*", __FUNCTION__));
		rc = srom_parsecis(osh, &cis, 1, vars, count);
	}

	MFREE(osh, cis, OTP_SZ_MAX);

	return (rc);
}
#endif /* USB4322 */

#ifdef BCMSPI
/*
 * Read the SPI cis and call parsecis to initialize the vars.
 * Return 0 on success, nonzero on error.
 */
static int
BCMATTACHFN(initvars_cis_spi)(osl_t *osh, char **vars, uint *count)
{
	uint8 *cis;
	int rc;

#if defined(NDIS) && (!defined(UNDER_CE))
	uint8 cisd[SBSDIO_CIS_SIZE_LIMIT];
#endif /* defined(NDIS) && (!defined(UNDER_CE)) */

#if defined(NDIS) && (!defined(UNDER_CE))
	cis = (uint8*)cisd;
#else
	if ((cis = MALLOC(osh, SBSDIO_CIS_SIZE_LIMIT)) == NULL) {
		return -1;
	}
#endif /* defined(NDIS) && (!defined(UNDER_CE)) */

	bzero(cis, SBSDIO_CIS_SIZE_LIMIT);

	if (bcmsdh_cis_read(NULL, SDIO_FUNC_1, cis, SBSDIO_CIS_SIZE_LIMIT) != 0) {
#if defined(NDIS) && (!defined(UNDER_CE))
		/* nothing to do */
#else
		MFREE(osh, cis, SBSDIO_CIS_SIZE_LIMIT);
#endif /* defined(NDIS) && (!defined(UNDER_CE)) */
		return -2;
	}

	rc = srom_parsecis(osh, &cis, SDIO_FUNC_1, vars, count);

#if defined(NDIS) && (!defined(UNDER_CE))
	/* nothing to do here */
#else
	MFREE(osh, cis, SBSDIO_CIS_SIZE_LIMIT);
#endif /* defined(NDIS) && (!defined(UNDER_CE)) */

	return (rc);
}
#endif /* BCMSPI */

#if defined(BCMUSBDEV)
/* Return sprom size in 16-bit words */
uint
srom_size(si_t *sih, osl_t *osh)
{
	uint size = 0;
	if (SPROMBUS == PCMCIA_BUS) {
		uint32 origidx;
		sdpcmd_regs_t *pcmregs;
		bool wasup;

		origidx = si_coreidx(sih);
		pcmregs = si_setcore(sih, PCMCIA_CORE_ID, 0);
		ASSERT(pcmregs);

		if (!(wasup = si_iscoreup(sih)))
			si_core_reset(sih, 0, 0);

		/* not worry about earlier core revs */
		if (si_corerev(sih) < 8)
			goto done;


		switch (SI_PCMCIA_READ(osh, pcmregs, SROM_INFO) & SRI_SZ_MASK) {
		case 1:
			size = 256;	/* SROM_INFO == 1 means 4kbit */
			break;
		case 2:
			size = 1024;	/* SROM_INFO == 2 means 16kbit */
			break;
		default:
			break;
		}

	done:
		if (!wasup)
			si_core_disable(sih, 0);

		si_setcoreidx(sih, origidx);
	}
	return size;
}
#endif 

#if defined(BCMUSBDEV)
static int
BCMATTACHFN(initvars_srom_si)(si_t *sih, osl_t *osh, void *curmap, char **vars, uint *varsz)
{
	static bool srvars = FALSE;	/* Use OTP/SPROM as global variables */

#ifdef BCM_DONGLEVARS
	int sel = 0;	/* where to read srom/cis: 0 - none, 1 - otp, 2 - sprom */
	uint sz = 0;	/* srom size in bytes */
	void *oh = NULL;
	int rc = BCME_OK;
#endif

	/* Bail out if we've dealt with OTP/SPROM before! */
	if (srvars)
		goto exit;

#ifdef USB4322
	if (CHIPID(sih->chip) == BCM4322_CHIP_ID) {
		uint len = 0;
		char *base = NULL;

		/* Use built-in default is OTP not programmed */
		if (initvars_cis_usb(sih, osh, vars, varsz)) {
			/* Fall back to fake srom vars if OTP not programmed */
			len = srom_vars_len(defaultsromvars_4322usb);
			base = MALLOC(osh, (len + 1));
			if (base == NULL) {
				BS_ERROR(("initvars_srom_si: MALLOC failed.\n"));
				goto exit;
			}
			bzero(base, (len + 1));
			bcopy(defaultsromvars_4322usb, base, len);

			/* add final nullbyte terminator */
			*(base + len) = '\0';

			*vars = base;
			*varsz = len + 1;

			BS_ERROR(("initvars_srom_si: faked nvram %d bytes\n", len));
		}

		/* Make SROM variables global */
		nvram_append((void *)sih, *vars, *varsz);
	}

	return BCME_OK;
#endif	/* USB4322 */

#ifdef BCM_DONGLEVARS
	/* Access OTP if it is present, powered on, and programmed */
	if ((oh = otp_init(sih)) != NULL && (otp_status(oh) & OTPS_GUP_SW)) {
		sz = otp_size(oh);
		sel = 1;
	}
	/* Access the SPROM if it is present */
	else if ((sz = srom_size(sih, osh)) != 0) {
		sz <<= 1;
		sel = 2;
	}

	/* Read CIS in OTP/SPROM */
	if (sel != 0) {
		uint16 *srom;
		uint8 *body = NULL;
		uint otpsz = sz;

		ASSERT(sz);

		/* Allocate memory */
		if ((srom = (uint16 *)MALLOC(osh, sz)) == NULL)
			return BCME_NOMEM;

		/* Read CIS */
		switch (sel) {
		case 1:
			rc = otp_read_region(sih, OTP_SW_RGN, srom, &otpsz);
			sz = otpsz;
			body = (uint8 *)srom;
			break;
		case 2:
			rc = srom_read(sih, SI_BUS, curmap, osh, 0, sz, srom, TRUE);
			/* sprom has 8 byte h/w header */
			body = (uint8 *)srom + SBSDIO_SPROM_CIS_OFFSET;
			break;
		default:
			/* impossible to come here */
			ASSERT(0);
			break;
		}

		/* Parse CIS */
		if (rc == BCME_OK) {
			uint i, tpls = 0xffffffff;
			/* # sdiod fns + common + extra */
			uint8 *cis[SBSDIO_NUM_FUNCTION + 2];
			uint ciss = 0;

			/* each word is in host endian */
			htol16_buf((uint8 *)srom, sz);

			ASSERT(body);

			/* count cis tuple chains */
			for (i = 0; i < sz && ciss < ARRAYSIZE(cis) && tpls != 0; i ++) {
				cis[ciss++] = &body[i];
				for (tpls = 0; i < sz - 1; tpls ++) {
					if (body[i++] == CISTPL_END)
						break;
					i += body[i] + 1;
				}
			}

			/* call parser routine only when there are tuple chains */
			if (ciss > 0)
				rc = srom_parsecis(osh, cis, ciss, vars, varsz);
		}

		/* Clean up */
		MFREE(osh, srom, sz);

		/* Make SROM variables global */
		if (rc == BCME_OK)
			nvram_append((void *)sih, *vars, *varsz);
	}
#endif	/* BCM_DONGLEVARS */

	srvars = TRUE;

exit:
	/* Tell the caller there is no individual SROM variables */
	*vars = NULL;
	*varsz = 0;

	/* return OK so the driver will load & use defaults if bad srom/otp */
	return BCME_OK;
}
#else /* !BCMUSBDEV && !BCMSDIODEV */
static int
BCMATTACHFN(initvars_srom_si)(si_t *sih, osl_t *osh, void *curmap, char **vars, uint *varsz)
{
	/* Search flash nvram section for srom variables */
	return initvars_flash_si(sih, vars, varsz);
}
#endif	

/*
 * Read-only support for NVRAM on flash and otp.
 *
 * Copyright 2005, Broadcom Corporation
 * All Rights Reserved.                
 *                                     
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;   
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior      
 * written permission of Broadcom Corporation.                            
 *
 * $Id$
 */

#include <typedefs.h>
#include <osl.h>
#include <sbutils.h>
#include <bcmendian.h>
#include <bcmutils.h>
#include <bcmnvram.h>
#include <bcmotp.h>
#ifdef BCMJTAG
#include <bcmdevs.h>
#endif	/* BCMJTAG */
#include <sflash.h>
#include <sbconfig.h>
#include <sbchipc.h>

#define NVR_MSG(x)

typedef struct _vars {
	struct _vars	*next;
	int		size;
	char		vars[1];
} vars_t;
#define	VARS_T_OH	8	/* Overhead in _vars, can this be sizeof() if we do vars[0]? */

static vars_t *vars = NULL;

/* copy flash to ram */
static void
get_flash_nvram(sb_t *sbh, struct nvram_header *nvh)
{
	uint nvs = R_REG(&nvh->len) - sizeof(struct nvram_header);
	vars_t *new;


	new = (vars_t *)MALLOC(sb_osh(sbh), nvs + VARS_T_OH);
	if (new == NULL) {
		NVR_MSG(("Out of memory for flash vars\n"));
		return;
	}

	new->size = nvs;
	new->next = vars;
	vars = new;

#ifdef BCMJTAG
	if (BUSTYPE(sb_bus(sbh)) == JTAG_BUS) {
		uint32 *s, *d;
		uint sz = nvs;

		s = (uint32 *)(&nvh[1]);
		d = (uint32 *)new->vars;

		ASSERT(ISALIGNED((uint)s, sizeof(uint32)));
		ASSERT(ISALIGNED((uint)d, sizeof(uint32)));

		while (sz >= sizeof(uint32)) {
			*d++ = ltoh32(R_REG(s++));
			sz -= sizeof(uint32);
		}
		if (sz) {
			union {
				uint32	w;
				char	b[sizeof(uint32)];
			} data;
			uint i;
			char *dst = (char *)d;

			data.w =  ltoh32(R_REG(s));
			for (i = 0; i < sz; i++)
				*dst++ = data.b[i];
		}
	} else
#endif	/* BCMJTAG */
		bcopy((char *)(&nvh[1]), new->vars, nvs);

	NVR_MSG(("%s: flash nvram @ 0x%x, copied %d bytes to 0x%x\n", __FUNCTION__,
		 (uint)nvh, (uint)nvs, (uint)new->vars));
}


int
BCMINITFN(nvram_init)(void *sb)
{
	uint idx;
	chipcregs_t *cc;
	struct sflash *info;
	void *oh;
	vars_t *new;
	struct nvram_header *nvh = NULL;
	uint16 *rawotp = NULL;
	uint32 cap = 0, off, flsz, flbase, otpsz = 0;
	sb_t *sbh;


	/* Make sure */
	vars = NULL;

	sbh = (sb_t *) sb;

	/* Check for flash */
	idx = sb_coreidx(sbh);
	if ((cc = sb_setcore(sbh, SB_CC, 0)) != NULL) {
		flbase = (uint32)OSL_UNCACHED(SB_FLASH2);
		flsz = 0;
		cap = R_REG(&cc->capabilities);
		switch (cap & CAP_FLASH_MASK) {
		case PFLASH:
			flsz = SB_FLASH2_SZ;
			break;

		case SFLASH_ST:
		case SFLASH_AT:
			if ((info = sflash_init(cc)) == NULL)
				break;
			flsz = info->size;
			break;

		case FLASH_NONE:
		default:
			break;
		}
	} else {
		flbase = (uint32)OSL_UNCACHED(SB_FLASH1);
		flsz = SB_FLASH1_SZ;
	}

	/* If we found flash, see if there is nvram there */
	if (flsz != 0) {
		off = FLASH_MIN;
		nvh = NULL;
		while (off <= flsz) {
			nvh = (struct nvram_header *) (flbase + off - NVRAM_SPACE);
			if (R_REG(&nvh->magic) == NVRAM_MAGIC)
				break;
			off <<= 1;
			nvh = NULL;
		};

		if (nvh != NULL)
			get_flash_nvram(sbh, nvh);
	}

	/* Check for otp */
	if ((cc != NULL) && ((cap & CAP_OTPSIZE) != 0)) {
		uint32 base, bound, lim, st;
		uint16 *d16;
		int i, chunk, gchunks, tsz = 0;

		if ((oh = otp_init(sbh)) == NULL) {
			NVR_MSG(("otp_init failed\n"));
			goto otpout;
		}

		/* Read the whole otp so we can easily manipulate it */
		otpsz = lim = otp_size(oh);
		if ((rawotp = MALLOC(sb_osh(sbh), lim)) == NULL) {
			NVR_MSG(("Out of memory for rawotp\n"));
			goto otpout;
		}
		for (i = 0, d16 = rawotp; i < (lim / 2); i++)
			*d16++ = otpr(oh, i);

		st = otp_status(oh);
		if ((st & OTP_HW_REGION) == 0) {
			NVR_MSG(("otp: hw region not written (0x%x)\n", st));

			/* This could be a programming failure in the first
			 * chunk followed by one or more good chunks
			 */
			for (i = 0; i < (lim / 2); i++)
				if (rawotp[i] == OTP_MAGIC)
					break;

			if (i < (lim / 2)) {
				base = i;
				bound = (i * 2) + rawotp[i + 1];
				NVR_MSG(("otp: trying chunk at 0x%x-0x%x\n", i * 2, bound));
			} else {
				NVR_MSG(("otp: unprogrammed\n"));
				goto otpout;
			}
		} else {
			bound = rawotp[(lim / 2) + OTP_BOUNDARY_OFF];
			/* There are two cases: 1) The whole otp is used as nvram
			 * and 2) There is a hardware header followed by nvram.
			 */
			if (rawotp[0] == OTP_MAGIC) {
				base = 0;
				if (bound != rawotp[1])
					NVR_MSG(("otp: Bound 0x%x != chunk0 len 0x%x\n", bound,
						 rawotp[1]));
			} else
				base = bound;
		}

		/* Find and copy the data */

		chunk = 0;
		gchunks = 0;
		i = base / 2;
		while ((i < (lim / 2)) && (rawotp[i] == OTP_MAGIC)) {
			int dsz, rsz = rawotp[i + 1];

			if (((i * 2) + rsz) >= lim) {
				NVR_MSG(("  bad chunk size, chunk %d, base 0x%x, size 0x%x\n",
					 chunk, i * 2, rsz));
				/* Bad length, try to find another chunk anyway */
				rsz = 6;
			}
			if (hndcrc16((uint8 *)&rawotp[i], rsz,
				     CRC16_INIT_VALUE) == CRC16_GOOD_VALUE) {
				/* Good crc, copy the vars */
				NVR_MSG(("  good chunk %d, base 0x%x, size 0x%x\n",
					 chunk, i * 2, rsz));
				gchunks++;
				dsz = rsz - 6;
				tsz += dsz;
				new = (vars_t *)MALLOC(sb_osh(sbh), dsz + VARS_T_OH);
				if (new == NULL) {
					NVR_MSG(("Out of memory for otp\n"));
					goto otpout;
				}
				bcopy((char *)&rawotp[i + 2], new->vars, dsz);
				new->size = dsz;
				new->next = vars;
				vars = new;
				i += rsz / 2;
			} else {
				/* bad length or crc didn't check, try to find the next set */
				NVR_MSG(("  chunk %d @ 0x%x size 0x%x: bad crc, ",
					 chunk, i * 2, rsz));
				if (rawotp[i + (rsz / 2)] == OTP_MAGIC) {
					/* Assume length is good */
					i += rsz / 2;
				} else {
					while (++i < (lim / 2))
						if (rawotp[i] == OTP_MAGIC)
							break;
				}
				if (i < (lim / 2))
					NVR_MSG(("trying next base 0x%x\n", i * 2));
				else
					NVR_MSG(("no more chunks\n"));
			}
			chunk++;
		}

		NVR_MSG(("  otp size = %d, boundary = 0x%x, nv base = 0x%x\n",
			 lim, bound, base));
		if (tsz != 0)
			NVR_MSG(("  Found %d bytes in %d good chunks out of %d\n",
				 tsz, gchunks, chunk));
		else
			NVR_MSG(("  No good chunks found out of %d\n", chunk));
	}
otpout:

	/* Last, if we do have flash but no regular nvram was found in it,
	 * try for embedded nvram.
	 * Note that since we are doing this last, embedded nvram will override
	 * otp, a change from the normal precedence in the designs that use
	 * the full read/write nvram support.
	 */
	if ((flsz != 0) && (nvh == NULL)) {

		nvh = (struct nvram_header *)(flbase + 1024);
		if (R_REG(&nvh->magic) == NVRAM_MAGIC)
			get_flash_nvram(sbh, nvh);
		else {
			nvh = (struct nvram_header *)(flbase + 4096);
			if (R_REG(&nvh->magic) == NVRAM_MAGIC)
				get_flash_nvram(sbh, nvh);
		}
	}

	if (rawotp != NULL)
		MFREE(sb_osh(sbh), rawotp, otpsz);

	/* All done */
	sb_setcoreidx(sbh, idx);

	return 0;
}

void
BCMINITFN(nvram_exit)(void *sb)
{
	vars_t *this, *next;
	sb_t *sbh;

	sbh = (sb_t *)sb;
	this = vars;
	while (this) {
		next = this->next;
		MFREE(sb_osh(sbh), this, this->size + VARS_T_OH);
		this = next;
	}
	vars = NULL;
}

static char *
findvar(char *vars, char *lim, const char *name)
{
	char *s;
	int len;

	len = strlen(name);

	for (s = vars; (s < lim) && *s; ) {
		if ((bcmp(s, name, len) == 0) && (s[len] == '='))
			return (&s[len+1]);

		while (*s++)
			;
	}

	return NULL;
}


char *defvars = "il0macaddr=00:11:22:33:44:55\0"
		"boardtype=0xffff\0"
		"boardrev=0x10\0"
		"boardflags=8\0"
		"aa0=3";
#define	DEFVARSLEN	79

char *
BCMINITFN(nvram_get)(const char *name)
{
	char *v = NULL;
	vars_t *this;

	this = vars;
	while (this) {
		v = findvar(this->vars,
			    (char *)((uint)this->vars + this->size),
			    name);
		if (v) break;
		this = this->next;
	}

	if (v == NULL) {
		v = findvar(defvars, defvars + DEFVARSLEN, name);
		if (v)
			NVR_MSG(("%s: variable %s defaulted to %s\n",
				 __FUNCTION__, name, v));
	}

	return v;
}

int
BCMINITFN(nvram_set)(const char *name, const char *value)
{
	return 0;
}

int
BCMINITFN(nvram_unset)(const char *name)
{
	return 0;
}

int
BCMINITFN(nvram_commit)(void)
{
	return 0;
}

int
BCMINITFN(nvram_getall)(char *buf, int count)
{
	int len, resid = count;
	vars_t *this;

	this = vars;
	while (this) {
		char *from, *lim, *to;
		int acc;

		from = this->vars;
		lim = (char *)((uint)this->vars + this->size);
		to = buf;
		acc = 0;
		while ((from < lim) && (*from)) {
			len = strlen(from) + 1;
			if (resid < (acc + len))
				return BCME_BUFTOOSHORT;
			bcopy(from, to, len);
			acc += len;
			from += len;
			to += len;
		}

		resid -= acc;
		buf += acc;
		this = this->next;
	}
	if (resid < 1)
		return BCME_BUFTOOSHORT;
	*buf = '\0';
	return 0;
}

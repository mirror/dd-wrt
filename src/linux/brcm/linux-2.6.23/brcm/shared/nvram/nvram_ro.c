/*
 * Read-only support for NVRAM on flash and otp.
 *
 * Copyright (C) 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: nvram_ro.c,v 1.57.2.2 2008/07/16 00:41:46 Exp $
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmendian.h>
#include <bcmnvram.h>
#include <sbchipc.h>
#include <bcmsrom.h>
#include <bcmotp.h>
#ifdef BCMJTAG
#include <bcmdevs.h>
#endif	/* BCMJTAG */
#include <sflash.h>
#include <hndsoc.h>

#ifdef BCMDBG_ERR
#define NVR_MSG(x) printf x
#else
#define NVR_MSG(x)
#endif	/* BCMDBG_ERR */

typedef struct _vars {
	struct _vars *next;
	int bufsz;	/* allocated size */
	int size;	/* actual vars size */
	char *vars;
} vars_t;

#define	VARS_T_OH	sizeof(vars_t)

static vars_t *vars = NULL;

/* support flash memory by default */
#ifndef FLASH
#define FLASH	1
#endif

#if FLASH
/* copy flash to ram */
static void
BCMINITFN(get_flash_nvram)(si_t *sih, struct nvram_header *nvh)
{
	osl_t *osh;
	uint nvs, bufsz;
	vars_t *new;

	osh = si_osh(sih);

	nvs = R_REG(osh, &nvh->len) - sizeof(struct nvram_header);
	bufsz = nvs + VARS_T_OH;

	if ((new = (vars_t *)MALLOC(osh, bufsz)) == NULL) {
		NVR_MSG(("Out of memory for flash vars\n"));
		return;
	}
	new->vars = (char *)new + VARS_T_OH;

	new->bufsz = bufsz;
	new->size = nvs;
	new->next = vars;
	vars = new;

#ifdef BCMJTAG
	if (BUSTYPE(sih->bustype) == JTAG_BUS) {
		uint32 *s, *d;
		uint sz = nvs;

		s = (uint32 *)(&nvh[1]);
		d = (uint32 *)new->vars;

		ASSERT(ISALIGNED((uintptr)s, sizeof(uint32)));
		ASSERT(ISALIGNED((uintptr)d, sizeof(uint32)));

		while (sz >= sizeof(uint32)) {
			*d++ = ltoh32(R_REG(osh, s++));
			sz -= sizeof(uint32);
		}
		if (sz) {
			union {
				uint32	w;
				char	b[sizeof(uint32)];
			} data;
			uint i;
			char *dst = (char *)d;

			data.w =  ltoh32(R_REG(osh, s));
			for (i = 0; i < sz; i++)
				*dst++ = data.b[i];
		}
	} else
#endif	/* BCMJTAG */
		bcopy((char *)(&nvh[1]), new->vars, nvs);

	NVR_MSG(("%s: flash nvram @ %p, copied %d bytes to %p\n", __FUNCTION__,
	         nvh, nvs, new->vars));
}
#endif /* FLASH */

#if defined(BCMUSBDEV) || defined(BCMHOSTVARS)
#if defined(BCMHOSTVARS)
extern char *_vars;
extern uint _varsz;
#endif
#ifndef CONFIG_XIP
extern uint8 embedded_nvram[];
#endif
#endif	

int
BCMATTACHFN(nvram_init)(void *si)
{
#if FLASH
	uint idx;
	chipcregs_t *cc;
	si_t *sih;
	osl_t *osh;
	void *oh;
	struct nvram_header *nvh = NULL;
	uintptr flbase;
	struct sflash *info;
	uint32 cap = 0, off, flsz;
#endif /* FLASH */

	/* Make sure we read nvram in flash just once before freeing the memory */
	if (vars != NULL) {
		NVR_MSG(("nvram_init: called again without calling nvram_exit()\n"));
		return 0;
	}

#if FLASH
	sih = (si_t *)si;
	osh = si_osh(sih);

	/* Check for flash */
	idx = si_coreidx(sih);
	cc = si_setcore(sih, CC_CORE_ID, 0);
	ASSERT(cc);

	flbase = (uintptr)OSL_UNCACHED((void*)SI_FLASH2);
	flsz = 0;
	cap = R_REG(osh, &cc->capabilities);
	switch (cap & CC_CAP_FLASH_MASK) {
	case PFLASH:
		flsz = SI_FLASH2_SZ;
		break;

	case SFLASH_ST:
	case SFLASH_AT:
		if ((info = sflash_init(sih, cc)) == NULL)
			break;
		flsz = info->size;
		break;

	case FLASH_NONE:
	default:
		break;
	}

	/* If we found flash, see if there is nvram there */
	if (flsz != 0) {
		off = FLASH_MIN;
		nvh = NULL;
		while (off <= flsz) {
			nvh = (struct nvram_header *) (flbase + off - NVRAM_SPACE);
			if (R_REG(osh, &nvh->magic) == NVRAM_MAGIC)
				break;
			off <<= 1;
			nvh = NULL;
		};

		if (nvh != NULL)
			get_flash_nvram(sih, nvh);
	}

	/* Check for otp */
	if ((oh = otp_init(sih)) != NULL) {
		uint sz = otp_size(oh);
		uint bufsz = sz + VARS_T_OH;
		vars_t *new = (vars_t *)MALLOC(osh, bufsz);
		if (new != NULL)
			new->vars = (char *)new + VARS_T_OH;
		if (new == NULL) {
			NVR_MSG(("Out of memory for otp\n"));
		} else if (otp_nvread(oh, new->vars, &sz)) {
			NVR_MSG(("otp_nvread error\n"));
			MFREE(osh, new, bufsz);
		} else {
			new->bufsz = bufsz;
			new->size = sz;
			new->next = vars;
			vars = new;
		}
	}

	/* Last, if we do have flash but no regular nvram was found in it,
	 * try for embedded nvram.
	 * Note that since we are doing this last, embedded nvram will override
	 * otp, a change from the normal precedence in the designs that use
	 * the full read/write nvram support.
	 */
	if ((flsz != 0) && (nvh == NULL)) {
		nvh = (struct nvram_header *)(flbase + 1024);
		if (R_REG(osh, &nvh->magic) == NVRAM_MAGIC)
			get_flash_nvram(sih, nvh);
		else {
			nvh = (struct nvram_header *)(flbase + 4096);
			if (R_REG(osh, &nvh->magic) == NVRAM_MAGIC)
				get_flash_nvram(sih, nvh);
		}
	}

	/* All done */
	si_setcoreidx(sih, idx);
#endif /* FLASH */

#if defined(BCMUSBDEV) || defined(BCMHOSTVARS)
#if defined(BCMHOSTVARS)
	/* Honor host supplied variables and make them global */
	if (_vars != NULL && _varsz != 0)
		nvram_append(si, _vars, _varsz);
#endif	
#ifndef CONFIG_XIP
#endif	/* CONFIG_XIP */
#endif	
	return 0;
}

int
BCMATTACHFN(nvram_append)(void *si, char *varlst, uint varsz)
{
	si_t *sih = (si_t *) si;
	osl_t *osh = si_osh(sih);
	uint bufsz = VARS_T_OH;
	vars_t *new;

	if ((new = MALLOC(osh, bufsz)) == NULL)
		return BCME_NOMEM;

	new->vars = varlst;
	new->bufsz = bufsz;
	new->size = varsz;
	new->next = vars;
	vars = new;

	return BCME_OK;
}

void
BCMATTACHFN(nvram_exit)(void *si)
{
	vars_t *this, *next;
	si_t *sih;

	sih = (si_t *)si;
	this = vars;
	while (this) {
		next = this->next;
		MFREE(si_osh(sih), this, this->bufsz);
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

	for (s = vars; (s < lim) && *s;) {
		if ((bcmp(s, name, len) == 0) && (s[len] == '='))
			return (&s[len+1]);

		while (*s++)
			;
	}

	return NULL;
}

#ifdef BCMSPACE
char *defvars = "il0macaddr=00:11:22:33:44:55\0"
		"boardtype=0xffff\0"
		"boardrev=0x10\0"
		"boardflags=8\0"
		"aa0=3\0"
		"sromrev=2";
#define	DEFVARSLEN	89	/* Length of *defvars */
#endif	/* BCMSPACE */

char *
nvram_get(const char *name)
{
	char *v = NULL;
	vars_t *cur;

	for (cur = vars; cur; cur = cur->next)
		if ((v = findvar(cur->vars, cur->vars + cur->size, name)))
			break;

#ifdef BCMSPACE
	if (v == NULL) {
		v = findvar(defvars, defvars + DEFVARSLEN, name);
		if (v)
			NVR_MSG(("%s: variable %s defaulted to %s\n",
			         __FUNCTION__, name, v));
	}
#endif	/* BCMSPACE */

	return v;
}

int
BCMATTACHFN(nvram_set)(const char *name, const char *value)
{
	return 0;
}

int
BCMATTACHFN(nvram_unset)(const char *name)
{
	return 0;
}

int
BCMATTACHFN(nvram_reset)(void *si)
{
	return 0;
}

int
BCMATTACHFN(nvram_commit)(void)
{
	return 0;
}

int
nvram_getall(char *buf, int count)
{
	int len, resid = count;
	vars_t *this;

	this = vars;
	while (this) {
		char *from, *lim, *to;
		int acc;

		from = this->vars;
		lim = (char *)((uintptr)this->vars + this->size);
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

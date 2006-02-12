/*
 * Write-once support for otp.
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
#include <sbconfig.h>
#include <sbchipc.h>
#include <bcmotp.h>

#define OTP_MSG(x)

typedef struct _otpinfo {
	sb_t	*sbh;		/* Saved sb handle */
	chipcregs_t *cc;	/* Saved pointer to chipc regs */
	uint	ccrev;		/* chipc revision */
	uint	size;		/* Size of otp in bytes */
	uint16 *otpw;		/* Word pointer to otp */
	uint	hwprot;		/* Hardware protection bits */
	uint	signvalid;	/* Signature valid bits */
	int	boundary;	/* hw/sw boundary */
} otpinfo_t;

static otpinfo_t otpinfo;

#define OTPP_TRIES	10000000
#define OTP_TRIES	300

uint16
otpr(void *oh, uint wn)
{
	otpinfo_t *oi = (otpinfo_t *)oh;

	ASSERT(wn < ((oi->size / 2) + OTP_LIM_OFF));

	return (R_REG(&oi->otpw[wn]));
}

uint16
otproff(void *oh, int woff)
{
	otpinfo_t *oi = (otpinfo_t *)oh;

	ASSERT(woff >= (-((int)oi->size / 2)));
	ASSERT(woff < OTP_LIM_OFF);

	return (R_REG(&oi->otpw[(oi->size / 2) + woff]));
}

void *
otp_init(sb_t *sbh)
{
	uint idx;
	chipcregs_t *cc;
	otpinfo_t *oi;
	uint32 cap = 0;
	void *ret = NULL;

	oi = &otpinfo;
	bzero(oi, sizeof(otpinfo_t));

	idx = sb_coreidx(sbh);

	/* Check for otp */
	if ((cc = sb_setcore(sbh, SB_CC, 0)) != NULL) {
		cap = R_REG(&cc->capabilities);
		if ((cap & CAP_OTPSIZE) == 0) {
			/* Nothing there */
			goto out;
		}

		oi->sbh = sbh;
		oi->cc = cc;
		oi->ccrev = BCMINIT(sb_chipcrev)(sbh);
		oi->size = 1 << (((cap & CAP_OTPSIZE) >> CAP_OTPSIZE_SHIFT) + CAP_OTPSIZE_BASE);
		oi->hwprot = (int)(R_REG(&cc->otpstatus) & OTPS_PROTECT);
		oi->otpw = (uint16 *)((uint32)cc + CC_OTP);
		oi->boundary = -1;

		if (otproff(oi, OTP_HWSIGN_OFF) == OTP_SIGNATURE) {
			oi->signvalid |= OTP_HW_REGION;
			oi->boundary = otproff(oi, OTP_BOUNDARY_OFF);
		}

		if (otproff(oi, OTP_SWSIGN_OFF) == OTP_SIGNATURE)
			oi->signvalid |= OTP_SW_REGION;

		if (otproff(oi, OTP_CIDSIGN_OFF) == OTP_SIGNATURE)
			oi->signvalid |= OTP_CID_REGION;

		ret = (void *)oi;
	}

out:	/* All done */
	sb_setcoreidx(sbh, idx);

	return ret;
}

int
otp_status(void *oh)
{
	otpinfo_t *oi = (otpinfo_t *)oh;

	if (oi->cc == NULL)
		return -1;
	else
		return ((int)(oi->hwprot | oi->signvalid));
}


int
otp_size(void *oh)
{
	otpinfo_t *oi = (otpinfo_t *)oh;

	if (oi->cc == NULL)
		return -1;
	else
		return ((int)(oi->size));
}

#ifdef BCMNVRAMW

static int
otp_write_word(void *oh, int wn, uint16 data)
{
	otpinfo_t *oi = (otpinfo_t *)oh;
	chipcregs_t *cc = oi->cc;
	uint base, row, col, bit, i, j, k, max_tries;
	uint32 pwait, otpc, otpp, pst, st;

#ifdef	OTP_FORCEFAIL
	OTP_MSG(("%s: [0x%x] = 0x%x\n", __FUNCTION__, wn * 2, data));
#endif

	/* This is bit-at-a-time writing, future cores may do word-at-a-time */
	base = (wn * 16) + (wn / 4);
	otpc = 0x20000000;
	if (oi->ccrev > 12) {
		otpc |= 0x38;
		max_tries = 1;
	} else {
		max_tries = OTP_TRIES;
	}
	for (i = 0; i < 16; i++) {
		pwait = 0x00004000;
		bit = data & 1;
		row = (base + i) / 65;
		col = (base + i) % 65;
		otpp = OTPP_START |
			((bit << OTPP_VALUE_SHIFT) & OTPP_VALUE) |
			((row << OTPP_ROW_SHIFT) & OTPP_ROW_MASK) |
			(col & OTPP_COL_MASK);
		j = 0;
		while (1) {
			j++;
			W_REG(&cc->otpcontrol, otpc | pwait);
			W_REG(&cc->otpprog, otpp);
			pst = R_REG(&cc->otpprog);
			for (k = 0; ((pst & OTPP_BUSY) == OTPP_BUSY) && (k < OTPP_TRIES); k++)
				pst = R_REG(&cc->otpprog);
			if (k >= OTPP_TRIES) {
				OTP_MSG(("BUSY stuck: pst=0x%x, count=%d\n", pst, k));
				st = OTPS_PROGFAIL;
				break;
			}
			st = R_REG(&cc->otpstatus);
			if ((st & OTPS_PROGFAIL) == 0)
				break;
			if (j >= max_tries) {
				break;
			} else if (pwait != OTPC_PROGWAIT) {
				pwait = (pwait << 3) & OTPC_PROGWAIT;
				if (pwait == 0)
					pwait = OTPC_PROGWAIT;
			}
		}
		if (st & OTPS_PROGFAIL) {
			OTP_MSG(("After %d tries: otpc = 0x%x, otpp = 0x%x/0x%x, otps = 0x%x\n",
			       j, otpc | pwait, otpp, pst, st));
			OTP_MSG(("otp prog failed. wn=%d, bit=%d, ppret=%d, ret=%d\n",
			       wn, i, k, j));
			return 1;
		}
		data >>= 1;
	}
	return 0;
}

int
otp_write_region(void *oh, int region, uint16 *data, uint wlen)
{
	otpinfo_t *oi = (otpinfo_t *)oh;
	uint32 st;
	uint wn, base = 0, lim;
	int ret;

	if ((oi->cc == NULL) || (oi->ccrev < 12)) {
		/* No support for buggy otp */
		return -1;
	}

	/* Check valid region */
	if ((region != OTP_HW_REGION) &&
	    (region != OTP_SW_REGION) &&
	    (region != OTP_CID_REGION))
		return -2;

	/* Region already written? */
	st = oi->hwprot | oi-> signvalid;
	if ((st & region) != 0)
		return -3;

	/* HW and CID have to be written before SW */
	if ((st & OTP_SW_REGION) != 0)
		return -4;

	/* Bounds for the region */
	lim = (oi->size / 2) + OTP_SWLIM_OFF;
	if (region == OTP_HW_REGION) {
		base = 0;
	} else if (region == OTP_SW_REGION) {
		base = oi->boundary / 2;
	} else if (region == OTP_CID_REGION) {
		base = (oi-> size / 2) + OTP_CID_OFF;
		lim = (oi-> size / 2) + OTP_LIM_OFF;
	}
	if (wlen > (lim - base))
		return -5;
	lim = base + wlen;


	/* Write the data */
	ret = -7;
	for (wn = base; wn < lim; wn++)
		if (otp_write_word(oh, wn, *data++) != 0)
			goto out;

	/* Done with the data, write the signature & boundary if needed */
	if (region == OTP_HW_REGION) {
		ret = -8;
		if (otp_write_word(oh, (oi->size / 2) + OTP_BOUNDARY_OFF, lim * 2) != 0)
			goto out;
		ret = -9;
		if (otp_write_word(oh, (oi->size / 2) + OTP_HWSIGN_OFF, OTP_SIGNATURE) != 0)
			goto out;
		oi->boundary = lim * 2;
		oi->signvalid |= OTP_HW_REGION;
	} else if (region == OTP_SW_REGION) {
		ret = -10;
		if (otp_write_word(oh, (oi->size / 2) + OTP_SWSIGN_OFF, OTP_SIGNATURE) != 0)
			goto out;
		oi->signvalid |= OTP_SW_REGION;
	} else if (region == OTP_CID_REGION) {
		ret = -11;
		if (otp_write_word(oh, (oi->size / 2) + OTP_CIDSIGN_OFF, OTP_SIGNATURE) != 0)
			goto out;
		oi->signvalid |= OTP_CID_REGION;
	}
	ret = 0;
out:

	OTP_MSG(("bits written: %d, average (%d/%d): %d, max retry: %d, pp max: %d\n",
	       st_n, st_s, st_n, st_s / st_n, st_hwm, pp_hwm));

	return ret;
}

int
otp_nvwrite(void *oh, uint16 *data, uint wlen)
{
	otpinfo_t *oi = (otpinfo_t *)oh;
	uint32 st;
	uint16 crc, clen, *p, hdr[2];
	uint wn, base = 0, lim;
	int err, gerr = 0;

	if ((oi->cc == NULL) || (oi->ccrev < 12)) {
		/* No support for buggy otp */
		return BCME_UNSUPPORTED;
	}

	/* otp already written? */
	st = oi->hwprot | oi-> signvalid;
	if ((st & (OTP_HW_REGION | OTP_SW_REGION)) == (OTP_HW_REGION | OTP_SW_REGION))
		return BCME_EPERM;

	/* Bounds for the region */
	lim = (oi->size / 2) + OTP_SWLIM_OFF;
	base = 0;

	/* Look for possible chunks from the end down */
	wn = lim;
	while (wn > 0) {
		wn--;
		if (otpr(oh, wn) == OTP_MAGIC) {
			base = wn + (otpr(oh, wn + 1) / 2);
			break;
		}
	}
	if (base == 0) {
		OTP_MSG(("Unprogrammed otp\n"));
	} else {
		OTP_MSG(("Found some chunks, skipping to 0x%x\n", base * 2));
	}
	if ((wlen + 3) > (lim - base))
		return BCME_NORESOURCE;


	/* Prepare the header and crc */
	hdr[0] = OTP_MAGIC;
	hdr[1] = (wlen + 3) * 2;
	crc = hndcrc16((uint8 *)hdr, sizeof(hdr), CRC16_INIT_VALUE);
	crc = hndcrc16((uint8 *)data, wlen * 2, crc);
	crc = ~crc;

	do {
		p = data;
		wn = base + 2;
		lim = base + wlen + 2;

		OTP_MSG(("writing chunk, 0x%x bytes @ 0x%x-0x%x\n", wlen * 2,
			 base * 2, (lim + 1) * 2));

		/* Write the header */
		err = otp_write_word(oh, base, hdr[0]);

		/* Write the data */
		while (wn < lim) {
			err += otp_write_word(oh, wn++, *p++);

			/* If there has been an error, close this chunk */
			if (err != 0) {
				OTP_MSG(("closing early @ 0x%x\n", wn * 2));
				break;
			}
		}

		/* If we wrote the whole chunk, write the crc */
		if (wn == lim) {
			OTP_MSG(("  whole chunk written, crc = 0x%x\n", crc));
			err += otp_write_word(oh, wn++, crc);
			clen = hdr[1];
		} else {
			/* If there was an error adjust the count to point to
			 * the word after the error so we can start the next
			 * chunk there.
			 */
			clen = (wn - base) * 2;
			OTP_MSG(("  partial chunk written, chunk len = 0x%x\n", clen));
		}
		/* And now write the chunk length */
		err += otp_write_word(oh, base + 1, clen);

		if (base == 0) {
			/* Write the signature and boundary if this is the HW region,
			 * but don't report failure if either of these 2 writes fail.
			 */
			if (otp_write_word(oh, (oi->size / 2) + OTP_BOUNDARY_OFF, wn * 2) == 0)
				gerr += otp_write_word(oh, (oi->size / 2) + OTP_HWSIGN_OFF,
						       OTP_SIGNATURE);
			else
				gerr++;
			oi->boundary = wn * 2;
			oi->signvalid |= OTP_HW_REGION;
		}

		if (err != 0) {
			gerr += err;
			/* Errors, do it all over again if there is space left */
			if ((wlen + 3) <= ((oi->size / 2) + OTP_SWLIM_OFF - wn)) {
				base = wn;
				lim = base + wlen + 2;
				OTP_MSG(("Programming errors, retry @ 0x%x\n", wn * 2));
			} else {
				OTP_MSG(("Programming errors, no space left ( 0x%x)\n", wn * 2));
				break;
			}
		}
	} while (err != 0);

	OTP_MSG(("bits written: %d, average (%d/%d): %d, max retry: %d, pp max: %d\n",
	       st_n, st_s, st_n, st_s / st_n, st_hwm, pp_hwm));

	if (gerr != 0)
		OTP_MSG(("programming %s after %d errors\n", (err == 0) ? "succedded" : "failed",
			 gerr));

	if (err)
		return BCME_ERROR;
	else
		return 0;
}

#endif /* BCMNVRAMW */

/*
 * OTP support.
 *
 * Copyright (C) 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: bcmotp.h,v 13.15.2.10 2008/10/06 00:10:49 Exp $
 */

#ifndef	_bcmotp_h_
#define	_bcmotp_h_

/* OTP regions */
#define OTP_HW_RGN	1
#define OTP_SW_RGN	2
#define OTP_CI_RGN	4
#define OTP_FUSE_RGN	8

/* OTP Size */
#define OTP_SZ_MAX		(6144/8)	/* maximum bytes in one CIS */

#if defined(BCMAUTOOTP) || !defined(BCMHNDOTP)	    /* Newer IPX OTP wrapper */
/* Fixed size subregions sizes in words */
#define OTPGU_CI_SZ		2

/* Maximum OTP redundancy entries.  */
#define MAXNUMRDES	9

/* OTP usage */
#define OTP4325_FM_DISABLED_OFFSET	188
#endif	/* BCMAUTOOTP || !BCMHNDOTP */

/* OTP common function type */
typedef int	(*otp_status_t)(void *oh);
typedef int	(*otp_size_t)(void *oh);
typedef void*	(*otp_init_t)(si_t *sih);
typedef uint16	(*otp_read_bit_t)(void *oh, chipcregs_t *cc, uint off);
typedef int	(*otp_read_region_t)(void *oh, int region, uint16 *data, uint *wlen);
typedef int	(*otp_nvread_t)(void *oh, char *data, uint *len);
typedef int	(*otp_write_region_t)(void *oh, int region, uint16 *data, uint wlen);
typedef int	(*otp_cis_append_region_t)(si_t *sih, int region, char *vars, int count);
typedef int	(*otp_nvwrite_t)(void *oh, uint16 *data, uint wlen);
typedef int	(*otp_dump_t)(void *oh, int arg, char *buf, uint size);

/* OTP function struct */
typedef struct otp_fn_s {
	otp_status_t		status;
	otp_size_t		size;
	otp_read_bit_t		read_bit;
	otp_init_t		init;
	otp_read_region_t	read_region;
	otp_nvread_t		nvread;
#ifdef BCMNVRAMW
	otp_write_region_t	write_region;
	otp_cis_append_region_t	cis_append_region;
	otp_nvwrite_t		nvwrite;
#endif /* BCMNVRAMW */
#if defined(WLTEST)
	otp_dump_t		dump;
#endif	
	uint32			magic;
} otp_fn_t;
#define	OTP_FN_MAGIC		0x87654321

typedef struct {
	uint		ccrev;		/* chipc revision */
	otp_fn_t	*fn;		/* OTP functions */
	si_t		*sih;		/* Saved sb handle */
	osl_t		*osh;
#if defined(BCMAUTOOTP) || !defined(BCMHNDOTP)	    /* Newer IPX OTP wrapper */
	/* Geometry */
	uint16		wsize;		/* Size of otp in words */
	uint16		rows;
	uint16		cols;
	/* Flag bits (lock/prog/rv). Reflected only when OTP is power cycled */
	uint32		status;
	/* Subregion boundaries */
	uint16	hwbase;		/* hardware subregion offset */
	uint16	hwlim;		/* hardware subregion boundary */
	uint16	swbase;		/* software subregion offset */
	uint16	swlim;		/* software subregion boundary */
	uint16	fbase;		/* fuse subregion offset */
	uint16	flim;		/* fuse subregion boundary */
	/* Use these to hide differences between different revs */
	int	otpgu_base;	/* offset to General Use Region */
#ifdef BCMNVRAMW
	struct {
		uint8 width;		/* entry width in bits */
		uint8 val_shift;	/* value bit offset in the entry */
		uint8 offsets;		/* # entries */
		uint8 stat_shift;	/* valid bit in otpstatus */
		uint16 offset[MAXNUMRDES];	/* entry offset in OTP */
	} rde_cb;		/* OTP redundancy control blocks */
#endif	/* BCMNVRAMW */
#endif	/* BCMAUTOOTP || !BCMHNDOTP */
#if defined(BCMAUTOOTP) || defined(BCMHNDOTP)	    /* Older HND OTP wrapper */
	uint	size;		/* Size of otp in bytes */
	uint	hwprot;		/* Hardware protection bits */
	uint	signvalid;	/* Signature valid bits */
	int	boundary;	/* hw/sw boundary */
#endif	/* BCMAUTOOTP || BCMHNDOTP */
} otpinfo_t;


#define otp_status(oh)		(((otpinfo_t*)oh)->fn->status(oh))
#define otp_size(oh)		(((otpinfo_t*)oh)->fn->size(oh))
extern uint16 otp_read_bit(void *oh, uint offset);
extern void* otp_init(si_t *sih);
extern int otp_read_region(si_t *sih, int region, uint16 *data, uint *wlen);
#define otp_nvread(oh, data, len)	(((otpinfo_t*)oh)->fn->nvread(oh, data, len))
#ifdef BCMNVRAMW
extern int otp_write_region(si_t *sih, int region, uint16 *data, uint wlen);
extern int otp_cis_append_region(si_t *sih, int region, char *vars, int count);
#define otp_nvwrite(oh, data, len)	(((otpinfo_t*)oh)->fn->nvwrite(oh, data, len))
#define otp_write_rde(oh, rde, bit, val)	ipxotp_write_rde(oh, rde, bit, val)
extern int ipxotp_write_rde(void *oh, int rde, uint bit, uint val);
#endif /* BCMNVRAMW */
#if defined(WLTEST)
#define otp_dump(oh, arg, buf, size)	(((otpinfo_t*)oh)->fn->dump(oh, arg, buf, size))
#endif

#endif /* _bcmotp_h_ */

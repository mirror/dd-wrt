/*
 * Copyright (c) 2002-2006 Sam Leffler, Errno Consulting
 * Copyright (c) 2005-2006 Atheros Communications, Inc.
 * All rights reserved.
 *
 * This module contains the regulatory domain table and accessor functions
 * for the information in the table.
 * The channel list creation is also contained in this module.
 *
 * "The country table and respective Regulatory Domain channel and power
 * settings are based on available knowledge as of software release. The
 * underlying global regulatory and spectrum rules change on a regular basis,
 * therefore, no warranty is given that the channel and power information
 * herein is complete, accurate or up to date.  Developers are responsible
 * for regulatory compliance of end-products developed using the enclosed
 * data per all applicable national requirements.  Furthermore, data in this
 * table does not guarantee that spectrum is available and that regulatory
 * approval is possible in every case. Knowldegable regulatory compliance
 * or government contacts should be consulted by the manufacturer to ensure
 * that the most current and accurate settings are used in each end-product.
 * This table was designed so that developers are able to update the country
 * table mappings as well as the Regulatory Domain definitions in order to
 * incorporate the most current channel and power settings in the end-product."
 *
 * $Id: //depot/sw/branches/sam_hal/ah_regdomain.c#11 $
 * $Atheros: //depot/sw/src/common/wlanchannel.c#258 $
 */
#include "opt_ah.h"

#include "ah.h"
#include "ah_internal.h"
#include "ah_eeprom.h"
#include "ah_devid.h"

/*
 * XXX this code needs a audit+review
 */

/* used throughout this file... */
#define	N(a)	(sizeof (a) / sizeof (a[0]))

#define HAL_MODE_11A_TURBO	HAL_MODE_108A
#define HAL_MODE_11G_TURBO	HAL_MODE_108G

/* 10MHz is half the 11A bandwidth used to determine upper edge freq
   of the outdoor channel */
#define HALF_MAXCHANBW		10
					   
/* 
 * Used to set the RegDomain bitmask which chooses which frequency
 * band specs are used.
 */

#define BMLEN 2		/* Use 2 64 bit uint for channel bitmask
			   NB: Must agree with macro below (BM) */
#define BMZERO {(u_int64_t) 0, (u_int64_t) 0}	/* BMLEN zeros */
#define CHAN_TURBO_G_BMZERO BMZERO,
			
#define BM(_fa, _fb, _fc, _fd, _fe, _ff, _fg, _fh, _fi, _fj, _fk, _fl) \
      {((((_fa >= 0) && (_fa < 64)) ? (((u_int64_t) 1) << _fa) : (u_int64_t) 0) | \
	(((_fb >= 0) && (_fb < 64)) ? (((u_int64_t) 1) << _fb) : (u_int64_t) 0) | \
	(((_fc >= 0) && (_fc < 64)) ? (((u_int64_t) 1) << _fc) : (u_int64_t) 0) | \
	(((_fd >= 0) && (_fd < 64)) ? (((u_int64_t) 1) << _fd) : (u_int64_t) 0) | \
	(((_fe >= 0) && (_fe < 64)) ? (((u_int64_t) 1) << _fe) : (u_int64_t) 0) | \
	(((_ff >= 0) && (_ff < 64)) ? (((u_int64_t) 1) << _ff) : (u_int64_t) 0) | \
	(((_fg >= 0) && (_fg < 64)) ? (((u_int64_t) 1) << _fg) : (u_int64_t) 0) | \
	(((_fh >= 0) && (_fh < 64)) ? (((u_int64_t) 1) << _fh) : (u_int64_t) 0) | \
	(((_fi >= 0) && (_fi < 64)) ? (((u_int64_t) 1) << _fi) : (u_int64_t) 0) | \
	(((_fj >= 0) && (_fj < 64)) ? (((u_int64_t) 1) << _fj) : (u_int64_t) 0) | \
	(((_fk >= 0) && (_fk < 64)) ? (((u_int64_t) 1) << _fk) : (u_int64_t) 0) | \
	(((_fl >= 0) && (_fl < 64)) ? (((u_int64_t) 1) << _fl) : (u_int64_t) 0) | \
	       ((((_fa > 63) && (_fa < 128)) ? (((u_int64_t) 1) << (_fa - 64)) : (u_int64_t) 0) | \
		(((_fb > 63) && (_fb < 128)) ? (((u_int64_t) 1) << (_fb - 64)) : (u_int64_t) 0) | \
		(((_fc > 63) && (_fc < 128)) ? (((u_int64_t) 1) << (_fc - 64)) : (u_int64_t) 0) | \
		(((_fd > 63) && (_fd < 128)) ? (((u_int64_t) 1) << (_fd - 64)) : (u_int64_t) 0) | \
		(((_fe > 63) && (_fe < 128)) ? (((u_int64_t) 1) << (_fe - 64)) : (u_int64_t) 0) | \
		(((_ff > 63) && (_ff < 128)) ? (((u_int64_t) 1) << (_ff - 64)) : (u_int64_t) 0) | \
		(((_fg > 63) && (_fg < 128)) ? (((u_int64_t) 1) << (_fg - 64)) : (u_int64_t) 0) | \
		(((_fh > 63) && (_fh < 128)) ? (((u_int64_t) 1) << (_fh - 64)) : (u_int64_t) 0) | \
		(((_fi > 63) && (_fi < 128)) ? (((u_int64_t) 1) << (_fi - 64)) : (u_int64_t) 0) | \
		(((_fj > 63) && (_fj < 128)) ? (((u_int64_t) 1) << (_fj - 64)) : (u_int64_t) 0) | \
		(((_fk > 63) && (_fk < 128)) ? (((u_int64_t) 1) << (_fk - 64)) : (u_int64_t) 0) | \
		(((_fl > 63) && (_fl < 128)) ? (((u_int64_t) 1) << (_fl - 64)) : (u_int64_t) 0)))}



/* Mask to check whether a domain is a multidomain or a single
   domain */

#define MULTI_DOMAIN_MASK 0xFF00

/* Enumerated Regulatory Domain Information 8 bit values indicate that
 * the regdomain is really a pair of unitary regdomains.  12 bit values
 * are the real unitary regdomains and are the only ones which have the
 * frequency bitmasks and flags set.
 */

enum EnumRd {
	/*
	 * The following regulatory domain definitions are
	 * found in the EEPROM. Each regulatory domain
	 * can operate in either a 5GHz or 2.4GHz wireless mode or
	 * both 5GHz and 2.4GHz wireless modes.
	 * In general, the value holds no special
	 * meaning and is used to decode into either specific
	 * 2.4GHz or 5GHz wireless mode for that particular
	 * regulatory domain.
	 */
	NO_ENUMRD	= 0x00,
	NULL1_WORLD	= 0x03,		/* For 11b-only countries (no 11a allowed) */
	NULL1_ETSIB	= 0x07,		/* Israel */
	NULL1_ETSIC	= 0x08,
	NULL1_GSM	= 0x09,		/* 900MHz-only operation */
	FCC1_FCCA	= 0x10,		/* USA */
	FCC1_WORLD	= 0x11,		/* Hong Kong */
	FCC4_FCCA	= 0x12,		/* USA - Public Safety */
	FCC5_FCCA	= 0x13,		/* US with no DFS (UNII-1 + UNII-3 Only)*/
	FCC6_FCCA	= 0x14,		/* Canada for AP only*/

	FCC2_FCCA	= 0x20,		/* Canada */
	FCC2_WORLD	= 0x21,		/* Australia & HK */
	FCC2_ETSIC	= 0x22,
	FCC6_WORLD	= 0x23,		/* Australia for AP only*/
	FRANCE_RES	= 0x31,		/* Legacy France for OEM */
	FCC3_FCCA	= 0x3A,		/* USA & Canada w/5470 band, 11h, DFS enabled */
	FCC3_WORLD	= 0x3B,		/* USA & Canada w/5470 band, 11h, DFS enabled */

	ETSI1_WORLD	= 0x37,
	BFWA		= 0x33, 	/* Broadband Fixed Wireless Access */
	EGAL		= 0x92, 	/* Broadband Fixed Wireless Access */
#ifdef AH_CARS
	CARSREG		= 0x93, 	/* Broadband Fixed Wireless Access */
#endif
	ETSI3_ETSIA	= 0x32,		/* France (optional) */
	ETSI2_WORLD	= 0x35,		/* Hungary & others */
	ETSI3_WORLD	= 0x36,		/* France & others */
	ETSI4_WORLD	= 0x30,
	ETSI4_ETSIC	= 0x38,
	ETSI5_WORLD	= 0x39,
	ETSI6_WORLD	= 0x34,		/* Bulgaria */
	ETSI7_WORLD	= 0x3c,		/* Bulgaria */
	ETSI8_WORLD	= 0x3d,		/* Bulgaria */
//	ETSI_RESERVED	= 0x33,		/* Reserved (Do not used) */

	MKK1_MKKA	= 0x40,		/* Japan (JP1) */
	MKK1_MKKB	= 0x41,		/* Japan (JP0) */
	APL4_WORLD	= 0x42,		/* Singapore */
	MKK2_MKKA	= 0x43,		/* Japan with 4.9G channels */
	APL_RESERVED	= 0x44,		/* Reserved (Do not used)  */
	APL2_WORLD	= 0x45,		/* Korea */
	APL2_APLC	= 0x46,
	APL3_WORLD	= 0x47,
	MKK1_FCCA	= 0x48,		/* Japan (JP1-1) */
	APL2_APLD	= 0x49,		/* Korea with 2.3G channels */
	MKK1_MKKA1	= 0x4A,		/* Japan (JE1) */
	MKK1_MKKA2	= 0x4B,		/* Japan (JE2) */
	MKK1_MKKC	= 0x4C,		/* Japan (MKK1_MKKA,except Ch14) */

	APL3_FCCA       = 0x50,
	APL1_WORLD	= 0x52,		/* Latin America */
	APL1_FCCA	= 0x53,
	APL1_APLA	= 0x54,
	APL1_ETSIC	= 0x55,
	APL2_ETSIC	= 0x56,		/* Venezuela */
	APL5_WORLD	= 0x58,		/* Chile */
	APL6_WORLD	= 0x5B,		/* Singapore */
	APL7_FCCA   = 0x5C,     /* Taiwan 5.47 Band */
	APL8_WORLD  = 0x5D,     /* Malaysia 5GHz */
	APL9_WORLD  = 0x5E,     /* Korea 5GHz */
	APL10_WORLD = 0x5F,     /* Korea 5GHz, After 11/2007. For STAs only */
	APL11_WORLD	= 0x5A,		/* India */

	RAI_WORLD	= 0xa0,
	RAIIT_WORLD	= 0xa1,
	IT_WORLD	= 0xa2,
	RAI 		= 0x01a0,
	RAIIT		= 0x01a1,
	IT		= 0x01a2,

	/*
	 * World mode SKUs
	 */
	WOR0_WORLD	= 0x60,		/* World0 (WO0 SKU) */
	WOR1_WORLD	= 0x61,		/* World1 (WO1 SKU) */
	WOR2_WORLD	= 0x62,		/* World2 (WO2 SKU) */
	WOR3_WORLD	= 0x63,		/* World3 (WO3 SKU) */
	WOR4_WORLD	= 0x64,		/* World4 (WO4 SKU) */	
	WOR5_ETSIC	= 0x65,		/* World5 (WO5 SKU) */    

	WOR01_WORLD	= 0x66,		/* World0-1 (WW0-1 SKU) */
	WOR02_WORLD	= 0x67,		/* World0-2 (WW0-2 SKU) */
	EU1_WORLD	= 0x68,		/* Same as World0-2 (WW0-2 SKU), except active scan ch1-13. No ch14 */

	WOR9_WORLD	= 0x69,		/* World9 (WO9 SKU) */	
	WORA_WORLD	= 0x6A,		/* WorldA (WOA SKU) */	

	MKK3_MKKB	= 0x80,		/* Japan UNI-1 even + MKKB */
	MKK3_MKKA2	= 0x81,		/* Japan UNI-1 even + MKKA2 */
	MKK3_MKKC	= 0x82,		/* Japan UNI-1 even + MKKC */

	MKK4_MKKB	= 0x83,		/* Japan UNI-1 even + UNI-2 + MKKB */
	MKK4_MKKA2	= 0x84,		/* Japan UNI-1 even + UNI-2 + MKKA2 */
	MKK4_MKKC	= 0x85,		/* Japan UNI-1 even + UNI-2 + MKKC */

	MKK5_MKKB	= 0x86,		/* Japan UNI-1 even + UNI-2 + mid-band + MKKB */
	MKK5_MKKA2	= 0x87,		/* Japan UNI-1 even + UNI-2 + mid-band + MKKA2 */
	MKK5_MKKC	= 0x88,		/* Japan UNI-1 even + UNI-2 + mid-band + MKKC */

	MKK6_MKKB	= 0x89,		/* Japan UNI-1 even + UNI-1 odd MKKB */
	MKK6_MKKA2	= 0x8A,		/* Japan UNI-1 even + UNI-1 odd + MKKA2 */
	MKK6_MKKC	= 0x8B,		/* Japan UNI-1 even + UNI-1 odd + MKKC */

	MKK7_MKKB	= 0x8C,		/* Japan UNI-1 even + UNI-1 odd + UNI-2 + MKKB */
	MKK7_MKKA2	= 0x8D,		/* Japan UNI-1 even + UNI-1 odd + UNI-2 + MKKA2 */
	MKK7_MKKC	= 0x8E,		/* Japan UNI-1 even + UNI-1 odd + UNI-2 + MKKC */

	MKK8_MKKB	= 0x8F,		/* Japan UNI-1 even + UNI-1 odd + UNI-2 + mid-band + MKKB */
	MKK8_MKKA2	= 0x90,		/* Japan UNI-1 even + UNI-1 odd + UNI-2 + mid-band + MKKA2 */
	MKK8_MKKC	= 0x91,		/* Japan UNI-1 even + UNI-1 odd + UNI-2 + mid-band + MKKC */

	/* Following definitions are used only by s/w to map old
 	 * Japan SKUs.
	 */
	MKK3_MKKA       = 0xF0,         /* Japan UNI-1 even + MKKA */
	MKK3_MKKA1      = 0xF1,         /* Japan UNI-1 even + MKKA1 */
	MKK3_FCCA       = 0xF2,         /* Japan UNI-1 even + FCCA */
	MKK4_MKKA       = 0xF3,         /* Japan UNI-1 even + UNI-2 + MKKA */
	MKK4_MKKA1      = 0xF4,         /* Japan UNI-1 even + UNI-2 + MKKA1 */
	MKK4_FCCA       = 0xF5,         /* Japan UNI-1 even + UNI-2 + FCCA */
	MKK9_MKKA       = 0xF6,         /* Japan UNI-1 even + 4.9GHz */
	MKK10_MKKA      = 0xF7,         /* Japan UNI-1 even + UNI-2 + 4.9GHz */

	/*
	 * Regulator domains ending in a number (e.g. APL1,
	 * MK1, ETSI4, etc) apply to 5GHz channel and power
	 * information.  Regulator domains ending in a letter
	 * (e.g. APLA, FCCA, etc) apply to 2.4GHz channel and
	 * power information.
	 */
	APL1		= 0x0150,	/* LAT & Asia */
	APL2		= 0x0250,	/* LAT & Asia */
	APL3		= 0x0350,	/* Taiwan */
	APL4		= 0x0450,	/* Jordan */
	APL5		= 0x0550,	/* Chile */
	APL6		= 0x0650,	/* Singapore */
	APL7		= 0x0750,	/* India */
	APL8		= 0x0850,	/* Malaysia */
	APL9		= 0x0950,	/* Korea (South) ROC 3 */
	APL10		= 0x1050,	/* Korea. After 11/2007. For STAs only */
	APL11		= 0x1150,	/* India */
	
	ETSI1		= 0x0130,	/* Europe & others */
	BFWA1		= 0x1f30,
	EGAL1		= 0x1f31,
#ifdef AH_CARS
	CARSREG1	= 0x1f32,
#endif
	ETSI2		= 0x0230,	/* Europe & others */
	ETSI3		= 0x0330,	/* Europe & others */
	ETSI4		= 0x0430,	/* Europe & others */
	ETSI5		= 0x0530,	/* Europe & others */
	ETSI6		= 0x0630,	/* Europe & others */
	ETSI7		= 0x0730,	/* Europe & others */
	ETSI8		= 0x0830,	/* Europe & others */
	ETSIA		= 0x0A30,	/* France */
	ETSIB		= 0x0B30,	/* Israel */
	ETSIC		= 0x0C30,	/* Latin America */

	FCC1		= 0x0110,	/* US & others */
	FCC2		= 0x0120,	/* Canada, Australia & New Zealand */
	FCC3		= 0x0160,	/* US w/new middle band & DFS */    
	FCC4          	= 0x0165,     	/* US Public Safety */
	FCC5		= 0x0510,     	   
	FCC6		= 0x0610,	/* Canada & Australia */
	FCCA		= 0x0A10,	 

	APLD		= 0x0D50,	/* South Korea */

	MKK1		= 0x0140,	/* Japan (UNI-1 odd)*/
	MKK2		= 0x0240,	/* Japan (4.9 GHz + UNI-1 odd) */
	MKK3		= 0x0340,	/* Japan (UNI-1 even) */
	MKK4		= 0x0440,	/* Japan (UNI-1 even + UNI-2) */
	MKK5		= 0x0540,	/* Japan (UNI-1 even + UNI-2 + mid-band) */
	MKK6		= 0x0640,	/* Japan (UNI-1 odd + UNI-1 even) */
	MKK7		= 0x0740,	/* Japan (UNI-1 odd + UNI-1 even + UNI-2 */
	MKK8		= 0x0840,	/* Japan (UNI-1 odd + UNI-1 even + UNI-2 + mid-band) */
	MKK9            = 0x0940,       /* Japan (UNI-1 even + 4.9 GHZ) */
	MKK10           = 0x0B40,       /* Japan (UNI-1 even + UNI-2 + 4.9 GHZ) */
	MKKA		= 0x0A40,	/* Japan */
	MKKC		= 0x0A50,

	NULL1		= 0x0198,
	WORLD		= 0x0199,
	WORLD11		= 0x019b,
	GSM		= 0x019a,
	DEBUG_REG_DMN	= 0x01ff,
};

#define	WORLD_SKU_MASK		0x00F0
#define	WORLD_SKU_PREFIX	0x0060

enum {					/* conformance test limits */
	FCC	= 0x10,
	MKK	= 0x40,
	ETSI	= 0x30,
};

/*
 * The following are flags for different requirements per reg domain.
 * These requirements are either inhereted from the reg domain pair or
 * from the unitary reg domain if the reg domain pair flags value is
 * 0
 */

enum {
	NO_REQ			= 0x00000000,
#if 1
	DISALLOW_ADHOC_11A	= 0x00000000,
	DISALLOW_ADHOC_11A_TURB	= 0x00000000,
#else
	DISALLOW_ADHOC_11A	= 0x00000001,
	DISALLOW_ADHOC_11A_TURB	= 0x00000002,
#endif
	NEED_NFC		= 0x00000004,

	ADHOC_PER_11D		= 0x00000008,  /* Start Ad-Hoc mode */
	ADHOC_NO_11A		= 0x00000010,

	PUBLIC_SAFETY_DOMAIN	= 0x00000020, 	/* public safety domain */
	LIMIT_FRAME_4MS 	= 0x00000040, 	/* 4msec limit on the frame length */

	NO_HOSTAP		= 0x00000080,	/* No HOSTAP mode opereation */
};

/*
 * The following describe the bit masks for different passive scan
 * capability/requirements per regdomain.
 */
#define	NO_PSCAN	0x0ULL
#define	PSCAN_FCC	0x0000000000000001ULL
#define	PSCAN_FCC_T	0x0000000000000002ULL
#define	PSCAN_ETSI	0x0000000000000004ULL
#define	PSCAN_MKK1	0x0000000000000008ULL
#define	PSCAN_MKK2	0x0000000000000010ULL
#define	PSCAN_MKKA	0x0000000000000020ULL
#define	PSCAN_MKKA_G	0x0000000000000040ULL
#define	PSCAN_ETSIA	0x0000000000000080ULL
#define	PSCAN_ETSIB	0x0000000000000100ULL
#define	PSCAN_ETSIC	0x0000000000000200ULL
#define	PSCAN_WWR	0x0000000000000400ULL
#define	PSCAN_MKKA1	0x0000000000000800ULL
#define	PSCAN_MKKA1_G	0x0000000000001000ULL
#define	PSCAN_MKKA2	0x0000000000002000ULL
#define	PSCAN_MKKA2_G	0x0000000000004000ULL
#define	PSCAN_MKK3	0x0000000000008000ULL
#define	PSCAN_DEFER	0x7FFFFFFFFFFFFFFFULL
#define	IS_ECM_CHAN	0x8000000000000000ULL

/*
 * THE following table is the mapping of regdomain pairs specified by
 * an 8 bit regdomain value to the individual unitary reg domains
 */

typedef struct reg_dmn_pair_mapping {
	HAL_REG_DOMAIN regDmnEnum;	/* 16 bit reg domain pair */
	HAL_REG_DOMAIN regDmn5GHz;	/* 5GHz reg domain */
	HAL_REG_DOMAIN regDmn2GHz;	/* 2GHz reg domain */
	u_int32_t flags5GHz;		/* Requirements flags (AdHoc
					   disallow, noise floor cal needed,
					   etc) */
	u_int32_t flags2GHz;		/* Requirements flags (AdHoc
					   disallow, noise floor cal needed,
					   etc) */
	u_int64_t pscanMask;		/* Passive Scan flags which
					   can override unitary domain
					   passive scan flags.  This
					   value is used as a mask on
					   the unitary flags*/
	u_int16_t singleCC;		/* Country code of single country if
					   a one-on-one mapping exists */
}  REG_DMN_PAIR_MAPPING;

static REG_DMN_PAIR_MAPPING regDomainPairs[] = {
	{NO_ENUMRD,	DEBUG_REG_DMN,	DEBUG_REG_DMN, NO_REQ, NO_REQ, PSCAN_DEFER, 0 },
	{NULL1_WORLD,	NULL1,		WORLD,		NO_REQ, NO_REQ, PSCAN_DEFER, 0 },
	{NULL1_ETSIB,	NULL1,		ETSIB,		NO_REQ, NO_REQ, PSCAN_DEFER, 0 },
	{NULL1_ETSIC,	NULL1,		ETSIC,		NO_REQ, NO_REQ, PSCAN_DEFER, 0 },
	{NULL1_GSM,	NULL1,		GSM,		NO_REQ, NO_REQ, PSCAN_DEFER, 0 },

	{FCC2_FCCA,	    FCC2,		FCCA,		NO_REQ, NO_REQ, PSCAN_DEFER, 0 },
	{FCC2_WORLD,	FCC2,		WORLD,		NO_REQ, NO_REQ, PSCAN_DEFER, 0 },
	{FCC2_ETSIC,	FCC2,		ETSIC,		NO_REQ, NO_REQ, PSCAN_DEFER, 0 },
	{FCC3_FCCA,	    FCC3,		FCCA,		NO_REQ, NO_REQ, PSCAN_DEFER, 0 },
	{FCC3_WORLD,	    FCC3,		WORLD,		NO_REQ, NO_REQ, PSCAN_DEFER, 0 },
	{FCC4_FCCA,	    FCC4,		FCCA,		DISALLOW_ADHOC_11A | DISALLOW_ADHOC_11A_TURB, NO_REQ, PSCAN_DEFER, 0 },
	{FCC5_FCCA,     FCC5,		FCCA,		NO_REQ, NO_REQ, PSCAN_DEFER, 0 },
	{FCC6_FCCA,     FCC6,		FCCA,		NO_REQ, NO_REQ, PSCAN_DEFER, 0 },
	{FCC6_WORLD,    FCC6,		WORLD,		NO_REQ, NO_REQ, PSCAN_DEFER, 0 },

	{ETSI1_WORLD,	ETSI1,		WORLD,		DISALLOW_ADHOC_11A | DISALLOW_ADHOC_11A_TURB, NO_REQ, PSCAN_DEFER, 0 },
	{ETSI7_WORLD,	ETSI7,		WORLD,		NO_REQ, NO_REQ, PSCAN_DEFER, 0 },
	{ETSI8_WORLD,	ETSI8,		WORLD,		NO_REQ, NO_REQ, PSCAN_DEFER, 0 },
	{EGAL,		EGAL1,		WORLD,		NO_REQ, NO_REQ, NO_PSCAN, 0 },
	{BFWA,      	BFWA1,		WORLD,		NO_REQ, NO_REQ, PSCAN_DEFER, 0 },
#ifdef AH_CARS
	{CARSREG,	CARSREG1,	WORLD,		NO_REQ, NO_REQ, NO_PSCAN, 0 },
#endif


	{RAI_WORLD,	RAI,		WORLD,		NO_REQ, NO_REQ, NO_PSCAN, 0 },
	{RAIIT_WORLD,	RAIIT,		WORLD,		NO_REQ, NO_REQ, NO_PSCAN, 0 },
	{IT_WORLD,	IT,		WORLD,		NO_REQ, NO_REQ, NO_PSCAN, 0 },


	{ETSI2_WORLD,	ETSI2,		WORLD,		DISALLOW_ADHOC_11A | DISALLOW_ADHOC_11A_TURB, NO_REQ, PSCAN_DEFER, 0 },
	{ETSI3_WORLD,	ETSI3,		WORLD,		DISALLOW_ADHOC_11A | DISALLOW_ADHOC_11A_TURB, NO_REQ, PSCAN_DEFER, 0 },
	{ETSI4_WORLD,	ETSI4,		WORLD,		DISALLOW_ADHOC_11A | DISALLOW_ADHOC_11A_TURB, NO_REQ, PSCAN_DEFER, 0 },
	{ETSI5_WORLD,	ETSI5,		WORLD,		DISALLOW_ADHOC_11A | DISALLOW_ADHOC_11A_TURB, NO_REQ, PSCAN_DEFER, 0 },
	{ETSI6_WORLD,	ETSI6,		WORLD,		DISALLOW_ADHOC_11A | DISALLOW_ADHOC_11A_TURB, NO_REQ, PSCAN_DEFER, 0 },

	{ETSI3_ETSIA,	ETSI3,		WORLD,		DISALLOW_ADHOC_11A | DISALLOW_ADHOC_11A_TURB, NO_REQ, PSCAN_DEFER, 0 },
	{FRANCE_RES,	ETSI3,		WORLD,		NO_REQ, NO_REQ, PSCAN_DEFER, 0 },

	{FCC1_WORLD,	FCC1,		WORLD,		NO_REQ, NO_REQ, PSCAN_DEFER, 0 },
	{FCC1_FCCA,	    FCC1,		FCCA,		NO_REQ, NO_REQ, PSCAN_DEFER, 0 },
	{APL1_FCCA,     APL1,		FCCA,		NO_REQ, NO_REQ, PSCAN_DEFER, 0 },
	{APL1_WORLD,	APL1,		WORLD,		NO_REQ, NO_REQ, PSCAN_DEFER, 0 },
	{APL2_WORLD,	APL2,		WORLD,		NO_REQ, NO_REQ, PSCAN_DEFER, 0 },
	{APL3_WORLD,	APL3,		WORLD,		NO_REQ, NO_REQ, PSCAN_DEFER, 0 },
	{APL4_WORLD,	APL4,		WORLD,		NO_REQ, NO_REQ, PSCAN_DEFER, 0 },
	{APL5_WORLD,	APL5,		WORLD,		NO_REQ, NO_REQ, PSCAN_DEFER, 0 },
	{APL6_WORLD,	APL6,		WORLD,		NO_REQ, NO_REQ, PSCAN_DEFER, 0 },
	{APL7_FCCA,	APL7,		FCCA,		NO_REQ, NO_REQ, PSCAN_DEFER, 0 },
	{APL8_WORLD,	APL8,		WORLD,		NO_REQ, NO_REQ, PSCAN_DEFER, 0 },
	{APL9_WORLD,	APL9,		WORLD,		NO_REQ, NO_REQ, PSCAN_DEFER, 0 },
	{APL10_WORLD,	APL10,		WORLD,		NO_REQ, NO_REQ, PSCAN_DEFER, 0 },
	{APL11_WORLD,	APL11,		WORLD11,		NO_REQ, NO_REQ, PSCAN_DEFER, 0 },

	{APL3_FCCA,		APL3,		FCCA,		NO_REQ, NO_REQ, PSCAN_DEFER, 0 },
	{APL1_ETSIC,	APL1,		ETSIC,		NO_REQ, NO_REQ, PSCAN_DEFER, 0 },
	{APL2_ETSIC,	APL2,		ETSIC,		NO_REQ, NO_REQ, PSCAN_DEFER, 0 },
	{APL2_APLD,	    APL2,		APLD,		NO_REQ, NO_REQ, PSCAN_DEFER,  },

	{MKK1_MKKA,	MKK1,		MKKA,		DISALLOW_ADHOC_11A_TURB | NEED_NFC | LIMIT_FRAME_4MS, NEED_NFC, PSCAN_MKK1 | PSCAN_MKKA, CTRY_JAPAN },
	{MKK1_MKKB,	MKK1,		MKKA,		DISALLOW_ADHOC_11A | DISALLOW_ADHOC_11A_TURB | NEED_NFC| LIMIT_FRAME_4MS, NEED_NFC, PSCAN_MKK1 | PSCAN_MKKA | PSCAN_MKKA_G, CTRY_JAPAN1 },
	{MKK1_FCCA,	MKK1,		FCCA,		DISALLOW_ADHOC_11A_TURB | NEED_NFC| LIMIT_FRAME_4MS, NEED_NFC, PSCAN_MKK1, CTRY_JAPAN2 },
	{MKK1_MKKA1,	MKK1,		MKKA,		DISALLOW_ADHOC_11A_TURB | NEED_NFC| LIMIT_FRAME_4MS, NEED_NFC, PSCAN_MKK1 | PSCAN_MKKA1 | PSCAN_MKKA1_G, CTRY_JAPAN4 },
	{MKK1_MKKA2,	MKK1,		MKKA,		DISALLOW_ADHOC_11A_TURB | NEED_NFC| LIMIT_FRAME_4MS, NEED_NFC, PSCAN_MKK1 | PSCAN_MKKA2 | PSCAN_MKKA2_G, CTRY_JAPAN5 },
	{MKK1_MKKC,	MKK1,		MKKC,		DISALLOW_ADHOC_11A_TURB | NEED_NFC| LIMIT_FRAME_4MS, NEED_NFC, PSCAN_MKK1, CTRY_JAPAN6 },

	/* MKK2 */
	{MKK2_MKKA,	MKK2,		MKKA,		DISALLOW_ADHOC_11A | DISALLOW_ADHOC_11A_TURB | NEED_NFC| LIMIT_FRAME_4MS, NEED_NFC, PSCAN_MKK2 | PSCAN_MKKA | PSCAN_MKKA_G, CTRY_JAPAN3 },

	/* MKK3 */
	{MKK3_MKKA,     MKK3,           MKKA,           DISALLOW_ADHOC_11A_TURB | NEED_NFC | LIMIT_FRAME_4MS, NEED_NFC , PSCAN_MKKA, 0 },
	{MKK3_MKKB,	MKK3,		MKKA,		DISALLOW_ADHOC_11A | DISALLOW_ADHOC_11A_TURB | NEED_NFC | LIMIT_FRAME_4MS, NEED_NFC, PSCAN_MKKA | PSCAN_MKKA_G, CTRY_JAPAN7 },
	{MKK3_MKKA1,    MKK3,           MKKA,           DISALLOW_ADHOC_11A_TURB | NEED_NFC | LIMIT_FRAME_4MS, NEED_NFC, PSCAN_MKKA1 | PSCAN_MKKA1_G, 0 },
	{MKK3_MKKA2,MKK3,		MKKA,		DISALLOW_ADHOC_11A_TURB | NEED_NFC | LIMIT_FRAME_4MS, NEED_NFC, PSCAN_MKKA2 | PSCAN_MKKA2_G, CTRY_JAPAN8 },
	{MKK3_MKKC,	MKK3,		MKKC,		DISALLOW_ADHOC_11A_TURB | NEED_NFC | LIMIT_FRAME_4MS, NEED_NFC, NO_PSCAN, CTRY_JAPAN9 },
	{MKK3_FCCA,     MKK3,           FCCA,           DISALLOW_ADHOC_11A_TURB | NEED_NFC | LIMIT_FRAME_4MS, NEED_NFC, NO_PSCAN, 0 },

	/* MKK4 */
	{MKK4_MKKB,	MKK4,		MKKA,		DISALLOW_ADHOC_11A | DISALLOW_ADHOC_11A_TURB | NEED_NFC | LIMIT_FRAME_4MS, NEED_NFC, PSCAN_MKK3 | PSCAN_MKKA | PSCAN_MKKA_G, CTRY_JAPAN10 },
	{MKK4_MKKA1,    MKK4,           MKKA,           DISALLOW_ADHOC_11A_TURB | NEED_NFC | LIMIT_FRAME_4MS, NEED_NFC, PSCAN_MKK3 | PSCAN_MKKA1 | PSCAN_MKKA1_G, 0 },
	{MKK4_MKKA2,	MKK4,		MKKA,		DISALLOW_ADHOC_11A_TURB | NEED_NFC | LIMIT_FRAME_4MS, NEED_NFC, PSCAN_MKK3 |PSCAN_MKKA2 | PSCAN_MKKA2_G, CTRY_JAPAN11 },
	{MKK4_MKKC,	MKK4,		MKKC,		DISALLOW_ADHOC_11A_TURB | NEED_NFC | LIMIT_FRAME_4MS, NEED_NFC, PSCAN_MKK3, CTRY_JAPAN12 },
	{MKK4_FCCA,     MKK4,           FCCA,           DISALLOW_ADHOC_11A_TURB | NEED_NFC | LIMIT_FRAME_4MS, NEED_NFC, PSCAN_MKK3, 0 },

	/* MKK5 */
	{MKK5_MKKB,	MKK5,		MKKA,		DISALLOW_ADHOC_11A | DISALLOW_ADHOC_11A_TURB | NEED_NFC | LIMIT_FRAME_4MS, NEED_NFC, PSCAN_MKK3 | PSCAN_MKKA | PSCAN_MKKA_G, CTRY_JAPAN13 },
	{MKK5_MKKA2,MKK5,		MKKA,		DISALLOW_ADHOC_11A_TURB | NEED_NFC | LIMIT_FRAME_4MS, NEED_NFC, PSCAN_MKK3 | PSCAN_MKKA2 | PSCAN_MKKA2_G, CTRY_JAPAN14 },
	{MKK5_MKKC,	MKK5,		MKKC,		DISALLOW_ADHOC_11A_TURB | NEED_NFC | LIMIT_FRAME_4MS, NEED_NFC, PSCAN_MKK3, CTRY_JAPAN15 },

	/* MKK6 */
	{MKK6_MKKB,	MKK6,		MKKA,		DISALLOW_ADHOC_11A | DISALLOW_ADHOC_11A_TURB | NEED_NFC | LIMIT_FRAME_4MS, NEED_NFC, PSCAN_MKK1 | PSCAN_MKKA | PSCAN_MKKA_G, CTRY_JAPAN16 },
	{MKK6_MKKA2,	MKK6,		MKKA,		DISALLOW_ADHOC_11A_TURB | NEED_NFC | LIMIT_FRAME_4MS, NEED_NFC, PSCAN_MKK1 | PSCAN_MKKA2 | PSCAN_MKKA2_G, CTRY_JAPAN17 },
	{MKK6_MKKC,	MKK6,		MKKC,		DISALLOW_ADHOC_11A_TURB | NEED_NFC | LIMIT_FRAME_4MS, NEED_NFC, PSCAN_MKK1, CTRY_JAPAN18 },

	/* MKK7 */
	{MKK7_MKKB,	MKK7,		MKKA,		DISALLOW_ADHOC_11A | DISALLOW_ADHOC_11A_TURB | NEED_NFC | LIMIT_FRAME_4MS, NEED_NFC, PSCAN_MKK1 | PSCAN_MKK3 | PSCAN_MKKA | PSCAN_MKKA_G, CTRY_JAPAN19 },
	{MKK7_MKKA2, MKK7,		MKKA,		DISALLOW_ADHOC_11A_TURB | NEED_NFC | LIMIT_FRAME_4MS, NEED_NFC, PSCAN_MKK1 | PSCAN_MKK3 | PSCAN_MKKA2 | PSCAN_MKKA2_G, CTRY_JAPAN20 },
	{MKK7_MKKC,	MKK7,		MKKC,		DISALLOW_ADHOC_11A_TURB | NEED_NFC | LIMIT_FRAME_4MS, NEED_NFC, PSCAN_MKK1 | PSCAN_MKK3, CTRY_JAPAN21 },

	/* MKK8 */
	{MKK8_MKKB,	MKK8,		MKKA,		DISALLOW_ADHOC_11A | DISALLOW_ADHOC_11A_TURB | NEED_NFC | LIMIT_FRAME_4MS, NEED_NFC, PSCAN_MKK1 | PSCAN_MKK3 | PSCAN_MKKA | PSCAN_MKKA_G, CTRY_JAPAN22 },
	{MKK8_MKKA2,MKK8,		MKKA,		DISALLOW_ADHOC_11A_TURB | NEED_NFC | LIMIT_FRAME_4MS, NEED_NFC, PSCAN_MKK1 | PSCAN_MKK3 | PSCAN_MKKA2 | PSCAN_MKKA2_G, CTRY_JAPAN23 },
	{MKK8_MKKC,	MKK8,		MKKC,		DISALLOW_ADHOC_11A_TURB | NEED_NFC | LIMIT_FRAME_4MS, NEED_NFC, PSCAN_MKK1 | PSCAN_MKK3 , CTRY_JAPAN24 },

	{MKK9_MKKA,     MKK9,           MKKA,           DISALLOW_ADHOC_11A | DISALLOW_ADHOC_11A_TURB | NEED_NFC | LIMIT_FRAME_4MS, NEED_NFC, PSCAN_MKK3 | PSCAN_MKKA | PSCAN_MKKA_G, 0 },
        {MKK10_MKKA,    MKK10,          MKKA,           DISALLOW_ADHOC_11A | DISALLOW_ADHOC_11A_TURB | NEED_NFC | LIMIT_FRAME_4MS, NEED_NFC, PSCAN_MKK3 | PSCAN_MKKA | PSCAN_MKKA_G, 0 },

		/* These are super domains */
	{WOR0_WORLD,	WOR0_WORLD,	WOR0_WORLD,	NO_REQ, NO_REQ, PSCAN_DEFER, 0 },
	{WOR1_WORLD,	WOR1_WORLD,	WOR1_WORLD,	DISALLOW_ADHOC_11A | DISALLOW_ADHOC_11A_TURB, NO_REQ, PSCAN_DEFER, 0 },
	{WOR2_WORLD,	WOR2_WORLD,	WOR2_WORLD,	DISALLOW_ADHOC_11A_TURB, NO_REQ, PSCAN_DEFER, 0 },
	{WOR3_WORLD,	WOR3_WORLD,	WOR3_WORLD,	NO_REQ, NO_REQ, PSCAN_DEFER, 0 },
	{WOR4_WORLD,	WOR4_WORLD,	WOR4_WORLD,	DISALLOW_ADHOC_11A | DISALLOW_ADHOC_11A_TURB, NO_REQ, PSCAN_DEFER, 0 },
	{WOR5_ETSIC,	WOR5_ETSIC,	WOR5_ETSIC,	DISALLOW_ADHOC_11A | DISALLOW_ADHOC_11A_TURB, NO_REQ, PSCAN_DEFER, 0 },
	{WOR01_WORLD,	WOR01_WORLD,	WOR01_WORLD,	NO_REQ, NO_REQ, PSCAN_DEFER, 0 },
	{WOR02_WORLD,	WOR02_WORLD,	WOR02_WORLD,	NO_REQ, NO_REQ, PSCAN_DEFER, 0 },
	{EU1_WORLD,	EU1_WORLD,	EU1_WORLD,	NO_REQ, NO_REQ, PSCAN_DEFER, 0 },
	{WOR9_WORLD,	WOR9_WORLD,	WOR9_WORLD,	DISALLOW_ADHOC_11A | DISALLOW_ADHOC_11A_TURB, NO_REQ, PSCAN_DEFER, 0 },
	{WORA_WORLD,	WORA_WORLD,	WORA_WORLD,	DISALLOW_ADHOC_11A | DISALLOW_ADHOC_11A_TURB, NO_REQ, PSCAN_DEFER, 0 },
};

/*
 * The following table of vendor specific regdomain pairs and
 * additional flags used to modify the flags5GHz and flags2GHz
 * of the original regdomain
 */

#define	NO_INTERSECT_REQ	0xFFFFFFFF
#define	NO_UNION_REQ		0
#define	MAX_MAPS		2

struct ccmap {
	char isoName[3];
	HAL_CTRY_CODE countryCode;
};

typedef struct vendor_pair_mapping {
	HAL_REG_DOMAIN regDmnEnum;	/* 16 bit reg domain pair */
	HAL_VENDORS vendor;		/* Vendor code */
	u_int32_t flags5GHzIntersect;	/* AND mask for requirements flags (AdHoc
					   disallow, noise floor cal needed,
					   etc) */
	u_int32_t flags5GHzUnion;	/* OR mask for requirements flags (AdHoc
					   disallow, noise floor cal needed,
					   etc) */
	u_int32_t flags2GHzIntersect;	/* AND mask for requirements flags (AdHoc
					   disallow, noise floor cal needed,
					   etc) */
	u_int32_t flags2GHzUnion;	/* AND mask for requirements flags (AdHoc
					   disallow, noise floor cal needed,
					   etc) */
	struct ccmap ccmappings[MAX_MAPS];	/* Vendor mapping of country strings to 
					   country codes */     		
}  VENDOR_PAIR_MAPPING;


static VENDOR_PAIR_MAPPING regDomainVendorPairs[] = {
	{WOR5_ETSIC, HAL_VENDOR_APPLE,	NO_INTERSECT_REQ,	NO_HOSTAP,	NO_INTERSECT_REQ,	NO_UNION_REQ, {{"JP",CTRY_JAPAN20}}},
	{WORA_WORLD, HAL_VENDOR_APPLE,	NO_INTERSECT_REQ,	NO_HOSTAP,	NO_INTERSECT_REQ,	NO_UNION_REQ, {{"JP", CTRY_JAPAN20}}},
};

		
/* 
 * The following table is the master list for all different freqeuncy
 * bands with the complete matrix of all possible flags and settings
 * for each band if it is used in ANY reg domain.
 */

#define DEF_REGDMN		FCC1_FCCA
#define	DEF_DMN_5		FCC1
#define	DEF_DMN_2		FCCA
#define	COUNTRY_ERD_FLAG        0x8000
#define WORLDWIDE_ROAMING_FLAG  0x4000
#define	SUPER_DOMAIN_MASK	0x0fff
#define	COUNTRY_CODE_MASK	0x3fff
#define CHANNEL_14		(2484)	/* 802.11g operation is not permitted on channel 14 */
#define IS_11G_CH14(_ch,_cf) \
	(((_ch) == CHANNEL_14) && ((_cf) == CHANNEL_G))

#define	YES	AH_TRUE
#define	NO	AH_FALSE

typedef struct {
	HAL_CTRY_CODE		countryCode;	   
	HAL_REG_DOMAIN		regDmnEnum;
	HAL_BOOL		allow11g;
	HAL_BOOL		allow11aTurbo;
	HAL_BOOL		allow11gTurbo;
	HAL_BOOL		allow11ng20;
	HAL_BOOL		allow11ng40;
	HAL_BOOL		allow11na20;
	HAL_BOOL		allow11na40;
	u_int16_t		outdoorChanStart;
} COUNTRY_CODE_TO_ENUM_RD;

static COUNTRY_CODE_TO_ENUM_RD allCountries[] = {
    {CTRY_DEBUG,       NO_ENUMRD,	YES, YES, YES, YES,YES, YES,YES, 7000 },
    {CTRY_DEFAULT,     DEF_REGDMN,	YES, YES, YES, YES,YES, YES,YES, 7000 },
    {CTRY_ALBANIA,     NULL1_WORLD,	YES,  NO, YES, YES, NO,  NO, NO, 7000 },
    {CTRY_ALGERIA,     NULL1_WORLD,	YES,  NO, YES, YES, NO,  NO, NO, 7000 },
    {CTRY_ARGENTINA,   FCC3_WORLD,      YES,  NO,  NO, YES, YES, YES, YES, 7000 },
    {CTRY_ARMENIA,     ETSI4_WORLD,	YES,  NO, YES, YES,YES,  NO, NO, 7000 },
    {CTRY_ARUBA,       ETSI1_WORLD,     YES,  NO, YES, YES, YES, YES, YES, 5470 },
    {CTRY_AUSTRALIA,   FCC2_WORLD,      YES, YES, YES, YES,YES, YES,YES, 7000 },
    {CTRY_AUSTRIA,     ETSI1_WORLD,	YES,  YES, YES, YES,YES, YES,YES, 5470 },
    {CTRY_AZERBAIJAN,  ETSI4_WORLD,     YES, YES, YES, YES,YES, YES,YES, 7000 },
    {CTRY_BANGLADESH,  NULL1_WORLD,     YES, NO , YES, YES, YES , NO, NO ,7000},
    {CTRY_BARBADOS,    FCC2_WORLD,      YES,  NO, YES, YES, YES, YES, YES, 7000 },
    {CTRY_BAHRAIN,     APL6_WORLD,	YES,  NO, YES, YES,YES, YES, NO, 7000 },
    {CTRY_BELARUS,     NULL1_WORLD,	YES,  NO, YES, YES,YES, YES, NO, 7000 },
    {CTRY_BELGIUM,     ETSI1_WORLD,	YES,  YES, YES, YES,YES, YES,YES, 5470 },
    {CTRY_BELIZE,      APL1_ETSIC,	YES, YES, YES, YES,YES, YES,YES, 7000 },
    {CTRY_BOLIVIA,     APL1_ETSIC,	YES, YES, YES, YES,YES, YES,YES, 7000 },
    {CTRY_BOSNIA_HERZ, ETSI1_WORLD,     YES,  NO, YES, YES, YES, YES,  YES, 5470 },
    {CTRY_BRAZIL,      FCC3_WORLD,	YES,  NO,  NO, YES, NO, YES, NO, 7000 },
    {CTRY_BRUNEI_DARUSSALAM,APL1_WORLD, YES, YES, YES, YES,YES, YES,YES, 7000 },
    {CTRY_BULGARIA,    ETSI6_WORLD,	YES,  YES, YES, YES,YES, YES,YES, 5470 },
    {CTRY_CANADA,      FCC2_FCCA,	YES, YES, YES, YES,YES, YES,YES, 7000 },
    {CTRY_CAMBODIA,    ETSI1_WORLD,     YES, NO, YES, YES, YES, YES, YES, 5470 },
    {CTRY_CANADA,      FCC3_FCCA,       YES, YES, YES, YES, YES, YES, YES, 7000 },
    {CTRY_CHILE,       APL6_WORLD,	YES, YES, YES, YES,YES, YES,YES, 7000 },
    {CTRY_CHINA,       APL1_WORLD,	YES, YES, YES, YES,YES, YES,YES, 7000 },
    {CTRY_COLOMBIA,    FCC1_FCCA,       YES,  NO, YES, YES,YES, YES, NO, 7000 },
    {CTRY_COSTA_RICA,  NULL1_WORLD,     YES,  NO, YES, YES,YES, YES, NO, 7000 },
    {CTRY_CROATIA,     ETSI1_WORLD,	YES,  NO, YES, YES,YES, YES, YES, 5470 },
    {CTRY_CYPRUS,      ETSI1_WORLD,	YES, YES, YES, YES,YES, YES,YES, 5470 },
    {CTRY_CZECH,       ETSI1_WORLD,	YES,  NO, YES, YES,YES, YES,YES, 5470 },
    {CTRY_DENMARK,     ETSI1_WORLD,	YES,  YES, YES, YES,YES, YES,YES, 5470 },
    {CTRY_DOMINICAN_REPUBLIC,FCC1_FCCA,	YES, YES, YES, YES,YES, YES,YES, 7000 },
    {CTRY_ECUADOR,     NULL1_WORLD,	NO,   NO,  NO,  NO, NO,  NO, NO, 7000 },
    {CTRY_EGYPT,       ETSI3_WORLD,	YES,  NO, YES, YES,YES, YES, NO, 7000 },
    {CTRY_EL_SALVADOR, NULL1_WORLD,	YES,  NO, YES, YES,YES,  NO, NO, 7000 },
    {CTRY_ESTONIA,     ETSI1_WORLD,	YES,  YES, YES, YES,YES, YES,YES, 5470 },
    {CTRY_FINLAND,     ETSI1_WORLD,	YES,  YES, YES, YES,YES, YES,YES, 5470 },
    {CTRY_FRANCE,      ETSI1_WORLD,	YES,  YES, YES, YES,YES, YES,YES, 5470 },
    {CTRY_FRANCE2,     ETSI3_WORLD,	YES,  NO, YES, YES,YES, YES,YES, 7000 },
    {CTRY_GEORGIA,     ETSI4_WORLD,	YES, YES, YES, YES,YES, YES,YES, 7000 },
    {CTRY_GERMANY,     ETSI1_WORLD,	YES,  YES, YES, YES,YES, YES,YES, 5470 },
    {CTRY_GERMANY_BFWA, BFWA,		NO,  YES, NO, NO, NO, YES,YES ,0xffff},
    {CTRY_EGALISTAN,     EGAL,	        YES,  YES, YES, YES,YES, YES,YES, 5470 },
#ifdef AH_CARS
    {CTRY_CARS,         CARSREG,	YES,  YES, YES, YES,YES, YES,YES, 7000 },
#endif
    {CTRY_GREECE,      ETSI1_WORLD,	YES,  YES, YES, YES,YES, YES,YES, 5470 },
    {CTRY_GREENLAND,   ETSI1_WORLD,     YES,  NO, YES, YES, YES, YES,  NO, 5470 },
    {CTRY_GRENADA,     FCC3_FCCA,       YES,  NO, YES, YES, YES, YES, YES, 7000 },
    {CTRY_GUAM,        FCC1_FCCA,       YES,  NO, YES, YES, YES, YES,  NO, 7000 },
    {CTRY_GUATEMALA,   FCC1_FCCA,	YES, YES, YES, YES,YES, YES,YES, 7000 },
    {CTRY_HAITI,       ETSI1_WORLD,     YES,  NO, YES, YES, YES, YES, YES, 5470 },
    {CTRY_HONDURAS,    NULL1_WORLD,	YES, NO,  YES, YES,YES, YES, NO, 7000 },
    {CTRY_HONG_KONG,   FCC3_WORLD,	YES, YES, YES, YES,YES, YES,YES, 7000 },
    {CTRY_HUNGARY,     ETSI1_WORLD,	YES,  YES, YES, YES,YES, YES,YES, 5470 },
    {CTRY_ICELAND,     ETSI1_WORLD,	YES,  YES, YES, YES,YES, YES,YES, 5470 },
    {CTRY_INDIA,       APL11_WORLD,	YES,  YES, YES, YES,YES, YES, NO, 5825 },
    {CTRY_INDONESIA,   NULL1_WORLD,	YES,  NO, YES, YES,YES, YES, NO, 7000 },
    {CTRY_IRAN,        APL1_WORLD,	YES, YES, YES, YES,YES, YES,YES, 7000 },
    {CTRY_IRELAND,     ETSI1_WORLD,	YES,  YES, YES, YES,YES, YES,YES, 5470 },
    {CTRY_ISRAEL,      NULL1_WORLD,	YES,  YES, YES, YES,YES, YES, NO, 7000 },
#ifdef RAIEXTRA
    {CTRY_ITALY,       IT_WORLD,	YES,  YES, YES, YES,YES, YES,YES, 7000 },
    {CTRY_ITALYRAI,    RAIIT_WORLD,	YES,  YES, YES, YES,YES, YES,YES, 7000 },
    {CTRY_RAI,         RAI_WORLD,	YES,  YES, YES, YES,YES, YES,YES, 7000 },
#else
    {CTRY_ITALY,       ETSI1_WORLD,	YES,  YES, YES, YES,YES, YES,YES, 5470 },
#endif
    {CTRY_JAMAICA,     FCC3_WORLD,      YES,  NO, YES, YES, YES, YES, YES, 7000 },
    {CTRY_JAPAN,       MKK1_MKKA,	YES,  NO,  NO, YES, NO, YES, NO, 7000 },
    {CTRY_JAPAN1,      MKK1_MKKB,	YES,  NO,  NO,  NO, NO,  NO, NO, 7000 },
    {CTRY_JAPAN2,      MKK1_FCCA,	YES,  NO,  NO,  NO, NO,  NO, NO, 7000 },
    {CTRY_JAPAN3,      MKK2_MKKA,	YES,  NO,  NO,  NO, NO,  NO, NO, 7000 },
    {CTRY_JAPAN4,      MKK1_MKKA1,	YES,  NO,  NO,  NO, NO,  NO, NO, 7000 },
    {CTRY_JAPAN5,      MKK1_MKKA2,	YES,  NO,  NO,  NO, NO,  NO, NO, 7000 },
    {CTRY_JAPAN6,      MKK1_MKKC,	YES,  NO,  NO,  NO, NO,  NO, NO, 7000 },

    {CTRY_JAPAN7,      MKK3_MKKB,	YES,  NO,  NO,  NO, NO,  NO, NO, 7000 },
    {CTRY_JAPAN8,      MKK3_MKKA2,	YES,  NO,  NO,  NO, NO,  NO, NO, 7000 },
    {CTRY_JAPAN9,      MKK3_MKKC,	YES,  NO,  NO,  NO, NO,  NO, NO, 7000 },

    {CTRY_JAPAN10,     MKK4_MKKB,	YES,  NO,  NO,  NO, NO,  NO, NO, 7000 },
    {CTRY_JAPAN11,     MKK4_MKKA2,	YES,  NO,  NO,  NO, NO,  NO, NO, 7000 },
    {CTRY_JAPAN12,     MKK4_MKKC,	YES,  NO,  NO,  NO, NO,  NO, NO, 7000 },

    {CTRY_JAPAN13,     MKK5_MKKB,	YES,  NO,  NO,  NO, NO,  NO, NO, 7000 },
    {CTRY_JAPAN14,     MKK5_MKKA2,	YES,  NO,  NO,  NO, NO,  NO, NO, 7000 },
    {CTRY_JAPAN15,     MKK5_MKKC,	YES,  NO,  NO,  NO, NO,  NO, NO, 7000 },

    {CTRY_JAPAN16,     MKK6_MKKB,	YES,  NO,  NO,  NO, NO,  NO, NO, 7000 },
    {CTRY_JAPAN17,     MKK6_MKKA2,	YES,  NO,  NO,  NO, NO,  NO, NO, 7000 },
    {CTRY_JAPAN18,     MKK6_MKKC,	YES,  NO,  NO,  NO, NO,  NO, NO, 7000 },

    {CTRY_JAPAN19,     MKK7_MKKB,	YES,  NO,  NO,  NO, NO,  NO, NO, 7000 },
    {CTRY_JAPAN20,     MKK7_MKKA2,	YES,  NO,  NO, YES, NO, YES, NO, 7000 },
    {CTRY_JAPAN21,     MKK7_MKKC,	YES,  NO,  NO,  NO, NO,  NO, NO, 7000 },

    {CTRY_JAPAN22,     MKK8_MKKB,	YES,  NO,  NO,  NO, NO,  NO, NO, 7000 },
    {CTRY_JAPAN23,     MKK8_MKKA2,	YES,  NO,  NO,  NO, NO,  NO, NO, 7000 },
    {CTRY_JAPAN24,     MKK8_MKKC,	YES,  NO,  NO,  NO, NO,  NO, NO, 7000 },

    {CTRY_JORDAN,      ETSI2_WORLD,	YES,  NO, YES, YES,YES, YES, YES, 7000 },
    {CTRY_KAZAKHSTAN,  NULL1_WORLD,	YES,  NO, YES, YES,YES,  NO, NO, 7000 },
    {CTRY_KENYA,       APL1_WORLD,      YES, NO,  YES, YES, YES, YES,YES,7000},
    {CTRY_KOREA_NORTH, APL2_WORLD,	YES, YES, YES, YES,YES, YES,YES, 7000 },
    {CTRY_KOREA_ROC,   APL2_WORLD,	YES,  NO,  NO, YES, NO, YES, NO, 7000 },
    {CTRY_KOREA_ROC2,  APL2_WORLD,	YES,  NO,  NO, YES, NO, YES, NO, 7000 },
    {CTRY_KOREA_ROC3,  APL9_WORLD,	YES,  NO,  NO, YES, NO, YES, NO, 7000 },
    {CTRY_KUWAIT,      ETSI3_WORLD,	YES,  NO, YES, YES,YES, YES, YES, 7000 },
    {CTRY_LATVIA,      ETSI1_WORLD,	YES,  YES, YES, YES,YES, YES,YES, 5470 },
    {CTRY_LEBANON,     NULL1_WORLD,	YES,  NO, YES, YES,YES, YES, NO, 7000 },
    {CTRY_LIECHTENSTEIN,ETSI1_WORLD,	YES,  YES, YES, YES,YES, YES,YES, 5470 },
    {CTRY_LITHUANIA,   ETSI1_WORLD,	YES,  YES, YES, YES,YES, YES,YES, 5470 },
    {CTRY_LUXEMBOURG,  ETSI1_WORLD,	YES,  YES, YES, YES,YES, YES,YES, 5470 },
    {CTRY_MACAU,       FCC2_WORLD,	YES, YES, YES, YES,YES, YES,YES, 7000 },
    {CTRY_MACEDONIA,   NULL1_WORLD,	YES,  NO, YES, YES,YES,  NO, NO, 7000 },
    {CTRY_MALAYSIA,    APL8_WORLD,	YES,  NO,  NO, YES, NO, YES, NO, 7000 },
    {CTRY_MALTA,       ETSI1_WORLD,	YES,  YES, YES, YES,YES, YES,YES, 5470 },
    {CTRY_MEXICO,      FCC1_FCCA,	YES, YES, YES, YES,YES, YES,YES, 7000 },
    {CTRY_MONACO,      ETSI4_WORLD,	YES, YES, YES, YES,YES, YES,YES, 7000 },
    {CTRY_MOROCCO,     APL4_WORLD,	YES,  NO, YES, YES,NO,  YES, YES, 7000 },
    {CTRY_NEPAL,       APL1_WORLD,      YES, NO, YES, YES , YES, YES ,YES ,7000},
    {CTRY_NETHERLANDS, ETSI1_WORLD,	YES,  YES, YES, YES,YES, YES,YES, 5470 },
    {CTRY_NEW_ZEALAND, FCC2_ETSIC,	YES,  NO, YES, YES,YES, YES,YES, 7000 },
    {CTRY_NORWAY,      ETSI1_WORLD,	YES,  YES, YES, YES,YES, YES,YES, 5470 },
    {CTRY_NORWAY_BFWA,      ETSI8_WORLD,	NO,  YES, NO, NO,NO, YES,YES, 0xffff },
    {CTRY_OMAN,        FCC3_WORLD,	YES,  NO, YES, YES,YES, YES, NO, 7000 },
    {CTRY_PAKISTAN,    NULL1_WORLD,	YES,  NO, YES, YES,YES,  NO, NO, 7000 },
    {CTRY_PAPUA_NEW_GUINEA, FCC1_WORLD, YES ,YES ,YES ,YES,YES, YES,YES,7000}, 
    {CTRY_PANAMA,      FCC1_FCCA,	YES, YES, YES, YES,YES, YES,YES, 7000 },
    {CTRY_PERU,        APL1_WORLD,	YES,  NO, YES, YES,YES, YES, NO, 7000 },
    {CTRY_PHILIPPINES, FCC3_WORLD,	YES, YES, YES, YES,YES, YES,YES, 7000 },
    {CTRY_POLAND,      ETSI1_WORLD,	YES,  YES, YES, YES,YES, YES,YES, 5470 },
    {CTRY_PORTUGAL,    ETSI1_WORLD,	YES,  YES, YES, YES,YES, YES,YES, 5470 },
    {CTRY_PUERTO_RICO, FCC1_FCCA,	YES, YES, YES, YES,YES, YES,YES, 7000 },
    {CTRY_QATAR,       APL1_WORLD,	YES,  NO, YES, YES,YES,  YES, YES, 7000 },
    {CTRY_ROMANIA,     NULL1_WORLD,	YES,  NO, YES, YES,YES,  NO, NO, 7000 },
    {CTRY_RUSSIA,      NULL1_WORLD,	YES,  NO, YES, YES,YES,  NO, NO, 7000 },
    {CTRY_SAUDI_ARABIA,FCC2_WORLD,	YES,  NO, YES, YES,YES, YES, NO, 7000 },
    {CTRY_SERBIA,      ETSI1_WORLD,     YES,  NO, YES, YES,YES, YES, YES, 7000 },
    {CTRY_MONTENEGRO,  ETSI1_WORLD,     YES,  NO, YES, YES,YES, YES, YES, 7000 },
    {CTRY_SINGAPORE,   APL6_WORLD,	YES, YES, YES, YES,YES, YES,YES, 7000 },
    {CTRY_SLOVAKIA,    ETSI1_WORLD,	YES,  YES, YES, YES,YES, YES,YES, 5470 },
    {CTRY_SLOVENIA,    ETSI1_WORLD,	YES,  YES, YES, YES,YES, YES,YES, 5470 },
    {CTRY_SOUTH_AFRICA,ETSI1_WORLD,	YES,  YES, YES, YES,YES, YES,YES, 5470 },
    {CTRY_SPAIN,       ETSI1_WORLD,	YES,  YES, YES, YES,YES, YES,YES, 5470 },
    {CTRY_SWEDEN,      ETSI1_WORLD,	YES,  YES, YES, YES,YES, YES,YES, 5470 },
    {CTRY_SWITZERLAND, ETSI1_WORLD,	YES,  YES, YES, YES,YES, YES,YES, 5470 },
    {CTRY_SYRIA,       NULL1_WORLD,	YES,  NO, YES, YES,YES, YES, NO, 7000 },
    {CTRY_TAIWAN,      APL3_FCCA,	YES, YES, YES, YES,YES, YES,YES, 7000 },
    {CTRY_THAILAND,    NULL1_WORLD,	YES,  NO, YES, YES,YES,  NO, NO, 7000 },
    {CTRY_TRINIDAD_Y_TOBAGO,FCC3_WORLD,YES,  NO, YES, YES,YES, YES, YES, 7000 },
    {CTRY_TUNISIA,     ETSI3_WORLD,	YES,  NO, YES, YES,YES, YES, NO, 7000 },
    {CTRY_TURKEY,      ETSI3_WORLD,	YES,  NO, YES, YES,YES, YES, NO, 7000 },
    {CTRY_UKRAINE,     NULL1_WORLD,	YES,  NO, YES, YES,YES,  NO, NO, 7000 },
    {CTRY_UAE,         NULL1_WORLD,	YES,  NO, YES, YES,YES,  NO, NO, 7000 },
    {CTRY_UNITED_KINGDOM, ETSI1_WORLD,	YES,  YES, YES, YES,YES, YES, YES, 5470 },
    {CTRY_UNITED_KINGDOM_BFWA, ETSI7_WORLD,	YES,  YES, YES, YES,YES, YES, YES, 0xffff },
    {CTRY_UNITED_STATES, FCC1_FCCA,	YES, YES, YES, YES,YES, YES,YES, 5470 },
    {CTRY_UNITED_STATES_FCC49,FCC4_FCCA,YES, YES, YES, YES,YES, YES,YES, 7000 },
    {CTRY_UNITED_STATES2, FCC6_FCCA,    YES, YES, YES, YES, YES, YES, YES, 7000 },
    {CTRY_URUGUAY,     FCC3_WORLD,	YES,  NO, YES, YES,YES, YES, YES, 7000 },
    {CTRY_UZBEKISTAN,  FCC3_FCCA,	YES, YES, YES, YES,YES, YES,YES, 7000 },
    {CTRY_VENEZUELA,   APL2_ETSIC,	YES,  NO, YES, YES,YES, YES, NO, 7000 },
    {CTRY_VIET_NAM,    NULL1_WORLD,	YES,  NO, YES, YES,YES,  NO, NO, 7000 },
    {CTRY_YEMEN,       NULL1_WORLD,	YES,  NO, YES, YES,YES,  NO, NO, 7000 },
    {CTRY_ZIMBABWE,    NULL1_WORLD,	YES,  NO, YES, YES,YES,  NO, NO, 7000 }
};


typedef struct RegDmnFreqBand {
	u_int16_t	lowChannel;	/* Low channel center in MHz */
	u_int16_t	highChannel;	/* High Channel center in MHz */
	u_int8_t	powerDfs;	/* Max power (dBm) for channel
					   range when using DFS */
	u_int8_t	antennaMax;	/* Max allowed antenna gain */
	u_int8_t	channelBW;	/* Bandwidth of the channel */
	u_int8_t	channelSep;	/* Channel separation within
					   the band */
	u_int64_t	useDfs;		/* Use DFS in the RegDomain
					   if corresponding bit is set */
	u_int64_t	usePassScan;	/* Use Passive Scan in the RegDomain
					   if corresponding bit is set */
	u_int8_t	regClassId;	/* Regulatory class id */
} REG_DMN_FREQ_BAND;

/* Bit masks for DFS per regdomain */

enum {
	NO_DFS   = 0x0000000000000000ULL,
	DFS_FCC3 = 0x0000000000000001ULL,
	DFS_ETSI = 0x0000000000000002ULL,
	DFS_MKK4 = 0x0000000000000004ULL,
};

/* The table of frequency bands is indexed by a bitmask.  The ordering
 * must be consistent with the enum below.  When adding a new
 * frequency band, be sure to match the location in the enum with the
 * comments 
 */

/*
 * 5GHz 11A channel tags
 */

enum {
#ifdef AH_SUPPORT_SC
	F1_SUPERCHANNEL,
#endif
	F1_5500_5700_RAI, // IT Ex
	F1_5845_5925_RAI,

	F1_5800_5925,
	F1_5755_5875,
	F1_4915_4925,
	F1_4935_4945,
	F1_4920_4980,
	F1_4942_4987,
	F1_4945_4985,
	F1_4950_4980,
	F1_5035_5040,
	F1_5040_5080,
	F1_5055_5055,

	F1_5120_5240,

	F1_5170_5230,
	F2_5170_5230,

	F1_5180_5240,
	F2_5180_5240,
	F3_5180_5240,
	F4_5180_5240,
	F5_5180_5240,
	F6_5180_5240,
	F7_5180_5240,
	F8_5180_5240,

	F1_5180_5320,

	F1_5240_5280,

	F1_5260_5280,

	F1_5260_5320,
	F2_5260_5320,
	F3_5260_5320,
	F4_5260_5320,
	F5_5260_5320,
	F6_5260_5320,

	F1_5260_5700,

	F1_5280_5320,

	F1_5500_5580,

	F1_5500_5620,

	F1_5500_5700,
	F2_5500_5700,
	F3_5500_5700,
	F4_5500_5700,
	F5_5500_5700,
	F6_5500_5700,

	F1_5660_5700,

	F1_5745_5805,
	F2_5745_5805,
	F3_5745_5805,

	F1_5745_5825,
	F2_5745_5825,
	F3_5745_5825,
	F4_5745_5825,
	F5_5745_5825,
	F6_5745_5825,
	F1_5825_5875,
	
	W1_4920_4980,
	W1_5040_5080,
	W1_5170_5230,
	W1_5180_5240,
	W1_5260_5320,
	W1_5745_5825,
	W1_5500_5700,
	W2_5260_5320,
	W2_5180_5240,
	W2_5825_5825,
	U1_5725_5795,
	U1_5815_5850,
	U2_5725_5795,
	U2_5815_5850
};

static REG_DMN_FREQ_BAND regDmn5GhzFreq[] = {
#ifdef AH_SUPPORT_SC
	{ 4915, 6100, 40, 40, 20, 5, NO_DFS, NO_PSCAN, 0},	/* T1_5130_5210 */
#endif

	{ 5500, 5700, 40, 40, 20, 20, NO_DFS, NO_PSCAN, 0},
	{ 5835, 5925, 40, 40, 20, 20, NO_DFS, NO_PSCAN, 0},

	{ 5800, 5925, 36, 0, 10, 5, NO_DFS, NO_PSCAN, 0}, 
	{ 5755, 5875, 36, 0, 20, 5, NO_DFS, NO_PSCAN, 0}, 
	{ 4915, 4925, 23, 0, 10, 5, NO_DFS, PSCAN_MKK2, 16 },				/* F1_4915_4925 */
	{ 4935, 4945, 23, 0, 10, 5, NO_DFS, PSCAN_MKK2, 16 },				/* F1_4935_4945 */
	{ 4920, 4980, 23, 0, 20, 20, NO_DFS, PSCAN_MKK2, 7 },				/* F1_4920_4980 */
	{ 4942, 4987, 27, 6, 5,  5, NO_DFS, PSCAN_FCC, 0 },				/* F1_4942_4987 */
	{ 4945, 4985, 30, 6, 10, 5, NO_DFS, PSCAN_FCC, 0 },				/* F1_4945_4985 */
	{ 4950, 4980, 33, 6, 20, 5, NO_DFS, PSCAN_FCC, 0 },				/* F1_4950_4980 */
	{ 5035, 5040, 23, 0, 10, 5, NO_DFS, PSCAN_MKK2, 12 },				/* F1_5035_5040 */
	{ 5040, 5080, 23, 0, 20, 20, NO_DFS, PSCAN_MKK2, 2 },				/* F1_5040_5080 */
	{ 5055, 5055, 23, 0, 10, 5, NO_DFS, PSCAN_MKK2, 12 },				/* F1_5055_5055 */

	{ 5120, 5240, 5,  6, 20, 20, NO_DFS, NO_PSCAN, 0 },				/* F1_5120_5240 */

	{ 5170, 5230, 23, 0, 20, 20, NO_DFS, PSCAN_MKK1 | PSCAN_MKK2, 1 },		/* F1_5170_5230 */
	{ 5170, 5230, 20, 0, 20, 20, NO_DFS, PSCAN_MKK1 | PSCAN_MKK2, 1 },		/* F2_5170_5230 */

	{ 5180, 5240, 15, 0, 20, 20, NO_DFS, PSCAN_FCC | PSCAN_ETSI, 0 },		/* F1_5180_5240 */
	{ 5180, 5240, 17, 6, 20, 20, NO_DFS, NO_PSCAN, 1 },				/* F2_5180_5240 */
	{ 5180, 5240, 18, 0, 20, 20, NO_DFS, PSCAN_FCC | PSCAN_ETSI, 0 },		/* F3_5180_5240 */
	{ 5180, 5240, 20, 0, 20, 20, NO_DFS, PSCAN_FCC | PSCAN_ETSI, 0 },		/* F4_5180_5240 */
	{ 5180, 5240, 23, 0, 20, 20, NO_DFS, PSCAN_FCC | PSCAN_ETSI, 0 },		/* F5_5180_5240 */
	{ 5180, 5240, 23, 6, 20, 20, NO_DFS, PSCAN_FCC, 0 },				/* F6_5180_5240 */
	{ 5180, 5240, 20, 0, 20, 20, NO_DFS, PSCAN_MKK1 | PSCAN_MKK3, 0 },		/* F7_5180_5240 */
	{ 5180, 5240, 23, 6, 20, 20, NO_DFS, NO_PSCAN, 0 },                             /* F8_5180_5240 */

	{ 5180, 5320, 20, 6, 20, 20, DFS_ETSI, PSCAN_ETSI, 0 },				/* F1_5180_5320 */

	{ 5240, 5280, 23, 0, 20, 20, DFS_FCC3, PSCAN_FCC | PSCAN_ETSI, 0 },		/* F1_5240_5280 */

	{ 5260, 5280, 23, 0, 20, 20, DFS_FCC3 | DFS_ETSI, PSCAN_FCC | PSCAN_ETSI, 0 },	/* F1_5260_5280 */

	{ 5260, 5320, 18, 0, 20, 20, DFS_FCC3 | DFS_ETSI, PSCAN_FCC | PSCAN_ETSI, 0 },	/* F1_5260_5320 */

	{ 5260, 5320, 20, 0, 20, 20, DFS_FCC3 | DFS_ETSI | DFS_MKK4, PSCAN_FCC | PSCAN_ETSI | PSCAN_MKK3 , 0 },
	/* F2_5260_5320 */

	{ 5260, 5320, 20, 6, 20, 20, DFS_FCC3 | DFS_ETSI, PSCAN_FCC, 2 },		/* F3_5260_5320 */
	{ 5260, 5320, 23, 6, 20, 20, DFS_FCC3 | DFS_ETSI, PSCAN_FCC, 2 },		/* F4_5260_5320 */
	{ 5260, 5320, 23, 6, 20, 20, DFS_FCC3 | DFS_ETSI, PSCAN_FCC, 0 },		/* F5_5260_5320 */
	{ 5260, 5320, 30, 0, 20, 20, NO_DFS, NO_PSCAN, 0 },				/* F6_5260_5320 */

	{ 5260, 5700, 5,  6, 20, 20, DFS_FCC3 | DFS_ETSI, NO_PSCAN, 0 },		/* F1_5260_5700 */

	{ 5280, 5320, 17, 6, 20, 20, DFS_FCC3 | DFS_ETSI, PSCAN_FCC, 0 },		/* F1_5280_5320 */

	{ 5500, 5580, 23, 6, 20, 20, DFS_FCC3, PSCAN_FCC, 0},                           /* F1_5500_5580 */

	{ 5500, 5620, 30, 6, 20, 20, DFS_ETSI, PSCAN_ETSI, 0 },				/* F1_5500_5620 */

	{ 5500, 5700, 20, 6, 20, 20, DFS_FCC3 | DFS_ETSI, PSCAN_FCC, 4 },		/* F1_5500_5700 */
	{ 5500, 5700, 27, 0, 20, 20, DFS_FCC3 | DFS_ETSI, PSCAN_FCC | PSCAN_ETSI, 0 },	/* F2_5500_5700 */
	{ 5500, 5700, 30, 0, 20, 20, DFS_FCC3 | DFS_ETSI, PSCAN_FCC | PSCAN_ETSI, 0 },	/* F3_5500_5700 */
	{ 5500, 5700, 23, 0, 20, 20, DFS_FCC3 | DFS_ETSI | DFS_MKK4, PSCAN_MKK3 | PSCAN_FCC, 0 },/* F4_5500_5700 */
	{ 5500, 5700, 30, 6, 20, 20, DFS_ETSI, PSCAN_ETSI, 0 },					/* F5_5500_5700 */
	{ 5500, 5700, 20, 0, 20, 20, DFS_FCC3 | DFS_ETSI | DFS_MKK4, PSCAN_MKK3 | PSCAN_FCC, 0 },/* F6_5500_5700 */

	{ 5660, 5700, 23, 6, 20, 20, DFS_FCC3, PSCAN_FCC, 0},                           /* F1_5660_5700 */

	{ 5745, 5805, 23, 0, 20, 20, NO_DFS, NO_PSCAN, 0 },				/* F1_5745_5805 */
	{ 5745, 5805, 30, 6, 20, 20, NO_DFS, NO_PSCAN, 0 },				/* F2_5745_5805 */
	{ 5745, 5805, 30, 6, 20, 20, DFS_ETSI, PSCAN_ETSI, 0 },				/* F3_5745_5805 */
	{ 5745, 5825, 5,  6, 20, 20, NO_DFS, NO_PSCAN, 0 },				/* F1_5745_5825 */
	{ 5745, 5825, 17, 0, 20, 20, NO_DFS, NO_PSCAN, 0 },				/* F2_5745_5825 */
	{ 5745, 5825, 20, 0, 20, 20, NO_DFS, NO_PSCAN, 0 },				/* F3_5745_5825 */	
	{ 5745, 5825, 30, 0, 20, 20, NO_DFS, NO_PSCAN, 0 },				/* F4_5745_5825 */
	{ 5745, 5825, 30, 6, 20, 20, NO_DFS, NO_PSCAN, 3 },				/* F5_5745_5825 */
	{ 5745, 5825, 30, 6, 20, 20, NO_DFS, NO_PSCAN, 0 },				/* F6_5745_5825 */
	{ 5825, 5875, 36, 0, 20, 5, NO_DFS, NO_PSCAN, 0 },				/* W2_5825_5825 */

	/*
	 * Below are the world roaming channels
	 * All WWR domains have no power limit, instead use the card's CTL
	 * or max power settings.
	 */
	{ 4920, 4980, 30, 0, 20, 20, NO_DFS, PSCAN_WWR, 0 },				/* W1_4920_4980 */
	{ 5040, 5080, 30, 0, 20, 20, NO_DFS, PSCAN_WWR, 0 },				/* W1_5040_5080 */
	{ 5170, 5230, 30, 0, 20, 20, NO_DFS, PSCAN_WWR, 0 },		/* W1_5170_5230 */
	{ 5180, 5240, 30, 0, 20, 20, NO_DFS, PSCAN_WWR, 0 },		/* W1_5180_5240 */
	{ 5260, 5320, 30, 0, 20, 20, DFS_FCC3 | DFS_ETSI, PSCAN_WWR, 0 },		/* W1_5260_5320 */
	{ 5745, 5825, 30, 0, 20, 20, NO_DFS, PSCAN_WWR, 0 },				/* W1_5745_5825 */
	{ 5500, 5700, 30, 0, 20, 20, DFS_FCC3 | DFS_ETSI, PSCAN_WWR, 0 },		/* W1_5500_5700 */
	{ 5260, 5320, 30, 0, 20, 20, NO_DFS, NO_PSCAN,  0 },				/* W2_5260_5320 */
	{ 5180, 5240, 30, 0, 20, 20, NO_DFS, NO_PSCAN,  0 },				/* W2_5180_5240 */
	{ 5825, 5825, 30, 0, 20, 20, NO_DFS, PSCAN_WWR, 0 },				/* W2_5825_5825 */
	{ 5735, 5785, 36, 0, 20, 5, DFS_ETSI, NO_PSCAN, 0}, 
	{ 5825, 5840, 36, 0, 20, 5, DFS_ETSI, NO_PSCAN, 0}, 
	{ 5735, 5785, 36, 0, 20, 20, DFS_ETSI, NO_PSCAN, 0}, 
	{ 5825, 5840, 36, 0, 20, 20, DFS_ETSI, NO_PSCAN, 0}, 
};

/*
 * 5GHz Turbo (dynamic & static) tags
 */

enum {
#ifdef AH_SUPPORT_SC
	T1_SUPERCHANNEL,
	T2_SUPERCHANNEL,
#endif

	T1_5500_5700_RAI, // IT Ex
	T1_5845_5925_RAI,

	T1_5755_5875,
	T1_5130_5210,
	T1_5250_5330,
	T1_5370_5490,
	T1_5530_5650,

	T1_5200_5200,
	T2_5200_5200,
	T3_5200_5200,
	T4_5200_5200,
	T5_5200_5200,
	T6_5200_5200,
	T7_5200_5200,                        
	T8_5200_5200,                        

	T1_5150_5190,
	T1_5230_5310,
	T1_5350_5470,
	T1_5510_5670,

	T1_5200_5240,
	T2_5200_5240,
	T1_5210_5210,
	T2_5210_5210,
	T3_5210_5210,
	T4_5210_5210,
	T5_5210_5210,
	T6_5210_5210,
	T7_5210_5210,
	T8_5210_5210,
	T9_5210_5210,
	T10_5210_5210,
	T1_5240_5240,

	T1_5280_5280,
	T2_5280_5280,
	T1_5250_5250,
	T1_5290_5290,
	T2_5290_5290,
	T3_5290_5290,                
	T1_5250_5290,
	T2_5250_5290,
	T3_5250_5290,
	T4_5250_5290,

	T1_5540_5660,
	T1_5530_5670,
	T1_5760_5800,
	T2_5760_5800,
	T3_5760_5800,
	T4_5760_5800,
	T5_5760_5800,
	T6_5760_5800,
	T7_5760_5800,                                        

	T1_5765_5805,
	T2_5765_5805,
	T3_5765_5805,
	T4_5765_5805,
	T5_5765_5805,
	T6_5765_5805,
	T7_5765_5805,
	T8_5765_5805,
	T9_5765_5805,                                                                


	T1_5825_5875,

	WT1_5210_5250,
	WT1_5290_5290,
	WT1_5540_5660,
	WT1_5760_5800,
	UT1_5725_5795,
	UT1_5815_5850,
	UT2_5725_5795,
	UT2_5815_5850
};
#define CHAN_TURBO_G_BMZERO BMZERO,

static REG_DMN_FREQ_BAND regDmn5GhzTurboFreq[] = {
#ifdef AH_SUPPORT_SC
	{ 4915, 6100, 40, 40, 40, 5, NO_DFS, NO_PSCAN, 0},	/* T1_5130_5210 */
	{ 5175, 5855, 33, 0, 40, 40, NO_DFS, NO_PSCAN, 0},	/* T1_5130_5210 */
#endif
	{ 5540, 5660, 40, 40, 40, 20, NO_DFS, NO_PSCAN, 0},
	{ 5855, 5905, 40, 40, 40, 20, NO_DFS, NO_PSCAN, 0},

	{ 5755, 5875, 36, 0, 40, 5, NO_DFS, NO_PSCAN, 0},
	{ 5130, 5210, 5,  6, 40, 40, NO_DFS, NO_PSCAN, 0},	/* T1_5130_5210 */
	{ 5250, 5330, 5,  6, 40, 40, DFS_FCC3, NO_PSCAN, 0},	/* T1_5250_5330 */
	{ 5370, 5490, 5,  6, 40, 40, NO_DFS, NO_PSCAN, 0},	/* T1_5370_5490 */
	{ 5530, 5650, 5,  6, 40, 40, DFS_FCC3, NO_PSCAN, 0},	/* T1_5530_5650 */

	{ 5200, 5200, 20, 0, 40, 40, NO_DFS, NO_PSCAN, 0},	/* T1_5200_5200 */
	{ 5200, 5200, 18, 0, 40, 40, DFS_ETSI, NO_PSCAN, 0},	/* T2_5200_5200 */
	{ 5200, 5200, 15, 0, 40, 40, DFS_ETSI, NO_PSCAN, 0},	/* T3_5200_5200 */
	{ 5200, 5200, 17, 6, 40, 40, DFS_FCC3, NO_PSCAN, 0},	/* T4_5200_5200 */
	{ 5200, 5200, 23, 0, 40, 40, DFS_MKK4, NO_PSCAN, 0},	/* T5_5200_5200 */
	{ 5200, 5200, 20, 0, 40, 40, DFS_MKK4, NO_PSCAN, 0},	/* T6_5200_5200 */
	{ 5200, 5200, 23, 6, 40, 40, NO_DFS, NO_PSCAN, 0},	/* T7_5200_5200 */
	{ 5200, 5200, 17, 6, 40, 40, NO_DFS, NO_PSCAN, 0},	/* T8_5200_5200 */


	{ 5150, 5190, 5,  6, 40, 40, NO_DFS, NO_PSCAN, 0},	/* T1_5150_5190 */
	{ 5230, 5310, 5,  6, 40, 40, DFS_FCC3, NO_PSCAN, 0},	/* T1_5230_5310 */
	{ 5350, 5470, 5,  6, 40, 40, NO_DFS, NO_PSCAN, 0},	/* T1_5350_5470 */
	{ 5510, 5670, 5,  6, 40, 40, DFS_FCC3, NO_PSCAN, 0},	/* T1_5510_5670 */

	{ 5200, 5240, 17, 6, 40, 40, NO_DFS, NO_PSCAN, 0},	/* T1_5200_5240 */
	{ 5200, 5240, 23, 6, 40, 40, NO_DFS, NO_PSCAN, 0},	/* T2_5200_5240 */
	{ 5210, 5210, 17, 6, 40, 40, NO_DFS, NO_PSCAN, 0},	/* T1_5210_5210 */
	{ 5210, 5210, 23, 0, 40, 40, NO_DFS, NO_PSCAN, 0},	/* T2_5210_5210 */
	{ 5210, 5210, 18, 0, 40, 40, DFS_ETSI, NO_PSCAN, 0},	/* T3_5210_5210 */
	{ 5210, 5210, 17, 0, 40, 40, DFS_FCC3, NO_PSCAN, 0},	/* T4_5210_5210 */
	{ 5210, 5210, 15, 0, 40, 40, DFS_ETSI, NO_PSCAN, 0},	/* T5_5210_5210 */
	{ 5210, 5210, 17, 6, 40, 40, DFS_FCC3, NO_PSCAN, 0},	/* T6_5210_5210 */
	{ 5210, 5210, 23, 6, 40, 40, NO_DFS, NO_PSCAN, 0},	/* T7_5210_5210 */
	{ 5210, 5210, 27, 6, 40, 40, DFS_FCC3, NO_PSCAN, 0},	/* T8_5210_5210 */
	{ 5210, 5210, 20, 0, 40, 40, NO_DFS, NO_PSCAN, 0},	/* T9_5210_5210 */
	{ 5210, 5210, 20, 0, 40, 40, DFS_MKK4, NO_PSCAN, 0},	/* T10_5210_5210 */
	{ 5240, 5240, 23, 6, 40, 40, NO_DFS, PSCAN_FCC_T, 0},	/* T1_5240_5240 */

	{ 5280, 5280, 23, 6, 40, 40, DFS_FCC3, PSCAN_FCC_T, 0},	/* T1_5280_5280 */
	{ 5280, 5280, 20, 6, 40, 40, DFS_FCC3, PSCAN_FCC_T, 0},	/* T2_5280_5280 */
	{ 5250, 5250, 17, 0, 40, 40, DFS_FCC3, PSCAN_FCC_T, 0},	/* T1_5250_5250 */
	{ 5290, 5290, 20, 0, 40, 40, DFS_FCC3, PSCAN_FCC_T, 0},	/* T1_5290_5290 */
	{ 5290, 5290, 30, 0, 40, 40, NO_DFS,  PSCAN_FCC_T, 0},	/* T2_5290_5290 */ 
	{ 5290, 5290, 20, 0, 40, 40, DFS_ETSI,  PSCAN_FCC_T, 0},/* T3_5290_5290 */ 

	{ 5250, 5290, 20, 0, 40, 40, DFS_FCC3, PSCAN_FCC_T, 0},	/* T1_5250_5290 */
	{ 5250, 5290, 23, 6, 40, 40, DFS_FCC3, PSCAN_FCC_T, 0},	/* T2_5250_5290 */
	{ 5250, 5290, 23, 6, 40, 40, NO_DFS, PSCAN_FCC_T, 0},	/* T3_5250_5290 */ 
	{ 5250, 5290, 27, 6, 40, 40, DFS_FCC3, PSCAN_FCC_T, 0},	/* T4_5250_5290 */ 

	{ 5540, 5660, 20, 6, 40, 40, DFS_FCC3, PSCAN_FCC_T, 0},	/* T1_5540_5660 */
	{ 5530, 5670, 30, 0, 40, 40, DFS_FCC3, PSCAN_FCC_T, 0},	/* T1_5530_5670 */
	{ 5760, 5800, 20, 0, 40, 40, NO_DFS, NO_PSCAN, 0},	/* T1_5760_5800 */
	{ 5760, 5800, 30, 6, 40, 40, NO_DFS, NO_PSCAN, 0},	/* T2_5760_5800 */
	{ 5760, 5800, 20, 0, 40, 40, NO_DFS, NO_PSCAN, 0},	/* T3_5760_5800 */
	{ 5760, 5800, 17, 0, 40, 40, NO_DFS, NO_PSCAN, 0},	/* T4_5760_5800 */
	{ 5760, 5800, 30, 0, 40, 40, DFS_ETSI, NO_PSCAN, 0},	/* T5_5760_5800 */
	{ 5760, 5800, 30, 6, 40, 40, DFS_ETSI, NO_PSCAN, 0},	/* T6_5760_5800 */
	{ 5760, 5800, 27, 6, 40, 40, DFS_FCC3, NO_PSCAN, 0},	/* T7_5760_5800 */


	{ 5765, 5805, 30, 6, 40, 40, NO_DFS, NO_PSCAN, 0},	/* T1_5765_5805 */
	{ 5765, 5805, 23, 0, 40, 40, NO_DFS, NO_PSCAN, 0},	/* T2_5765_5805 */
	{ 5765, 5805, 20, 0, 40, 40, NO_DFS, NO_PSCAN, 0},	/* T3_5765_5805 */
	{ 5765, 5805, 17, 0, 40, 40, NO_DFS, NO_PSCAN, 0},	/* T4_5765_5805 */
	{ 5765, 5805, 20, 0, 40, 40, DFS_ETSI, NO_PSCAN, 0},	/* T5_5765_5805 */
	{ 5765, 5805, 30, 0, 40, 40, DFS_ETSI, NO_PSCAN, 0},	/* T6_5765_5805 */
	{ 5765, 5805, 30, 6, 40, 40, NO_DFS, NO_PSCAN, 0},	/* T7_5765_5805 */
	{ 5765, 5805, 30, 6, 40, 40, DFS_FCC3, NO_PSCAN, 0},	/* T8_5765_5805 */
	{ 5765, 5805, 27, 6, 40, 40, DFS_FCC3, NO_PSCAN, 0},	/* T9_5765_5805 */


	{ 5825, 5875, 36, 0, 40, 5, NO_DFS, NO_PSCAN, 0 },				/* W2_5825_5825 */

	/*
	 * Below are the WWR frequencies
	 */

	{ 5210, 5250, 15, 0, 40, 40, DFS_FCC3 | DFS_ETSI, PSCAN_WWR, 0}, /* WT1_5210_5250 */
	{ 5290, 5290, 18, 0, 40, 40, DFS_FCC3 | DFS_ETSI, PSCAN_WWR, 0}, /* WT1_5290_5290 */
	{ 5540, 5660, 20, 0, 40, 40, DFS_FCC3 | DFS_ETSI, PSCAN_WWR, 0}, /* WT1_5540_5660 */
	{ 5760, 5800, 20, 0, 40, 40, NO_DFS, PSCAN_WWR, 0},	/* WT1_5760_5800 */
	{ 5745, 5775, 36, 0, 40, 5, DFS_ETSI, NO_PSCAN, 0}, 
	{ 5835, 5830, 36, 0, 40, 5, DFS_ETSI, NO_PSCAN, 0}, 
	{ 5745, 5775, 36, 0, 40, 40, DFS_ETSI, NO_PSCAN, 0}, 
	{ 5835, 5830, 36, 0, 40, 40, DFS_ETSI, NO_PSCAN, 0}, 
};
/*
 * 2GHz 11b channel tags
 */
enum {
#ifdef AH_SUPPORT_SC
	B1_SUPERCHANNEL,
	B2_SUPERCHANNEL,
#endif
	F1_2312_2372,
	F2_2312_2372,

	F1_2412_2472,
	F2_2412_2472,
	F3_2412_2472,

	F1_2412_2462,
	F2_2412_2462,

	F1_2432_2442,

	F1_2457_2472,

	F1_2467_2472,

	F1_2484_2484,
	F2_2484_2484,

	F1_2512_2732,
	F1_2407_2472,
	
	W1_2312_2372,
	W1_2412_2412,
	W1_2417_2432,
	W1_2437_2442,
	W1_2447_2457,
	W1_2462_2462,
	W1_2467_2467,
	W2_2467_2467,
	W1_2472_2472,
	W2_2472_2472,
	W1_2484_2484,
	W2_2484_2484,
};

static REG_DMN_FREQ_BAND regDmn2GhzFreq[] = {
#ifdef AH_SUPPORT_SC
	{ 2192, 2800, 40, 40, 20, 5, NO_DFS, NO_PSCAN, 0},
	{ 2224, 2800, 40, 40, 20, 5, NO_DFS, NO_PSCAN, 0},
#endif
	{ 2312, 2372, 5,  6, 20, 5, NO_DFS, NO_PSCAN, 0},	/* F1_2312_2372 */
	{ 2312, 2372, 20, 0, 20, 5, NO_DFS, NO_PSCAN, 0},	/* F2_2312_2372 */

	{ 2412, 2472, 5,  6, 20, 5, NO_DFS, NO_PSCAN, 0},	/* F1_2412_2472 */
	{ 2412, 2472, 20, 0, 20, 5, NO_DFS, PSCAN_MKKA, 0},	/* F2_2412_2472 */
	{ 2412, 2472, 30, 0, 20, 5, NO_DFS, NO_PSCAN, 0},	/* F3_2412_2472 */

	{ 2412, 2462, 27, 6, 20, 5, NO_DFS, NO_PSCAN, 0},	/* F1_2412_2462 */
	{ 2412, 2462, 20, 0, 20, 5, NO_DFS, PSCAN_MKKA, 0},	/* F2_2412_2462 */
	
	{ 2432, 2442, 20, 0, 20, 5, NO_DFS, NO_PSCAN, 0},	/* F1_2432_2442 */

	{ 2457, 2472, 20, 0, 20, 5, NO_DFS, NO_PSCAN, 0},	/* F1_2457_2472 */

	{ 2467, 2472, 20, 0, 20, 5, NO_DFS, PSCAN_MKKA2 | PSCAN_MKKA, 0}, /* F1_2467_2472 */

	{ 2484, 2484, 5,  6, 20, 5, NO_DFS, NO_PSCAN, 0},	/* F1_2484_2484 */
	{ 2484, 2484, 20, 0, 20, 5, NO_DFS, PSCAN_MKKA | PSCAN_MKKA1 | PSCAN_MKKA2, 0},	/* F2_2484_2484 */

	{ 2512, 2732, 5,  6, 20, 5, NO_DFS, NO_PSCAN, 0},	/* F1_2512_2732 */
	{ 2402, 2483, 30, 0, 20, 5, NO_DFS, PSCAN_MKKA, 0},	/* F2_2412_2472 */

	/*
	 * WWR have powers opened up to 20dBm.  Limits should often come from CTL/Max powers
	 */

	{ 2312, 2372, 20, 0, 20, 5, NO_DFS, NO_PSCAN, 0},	/* W1_2312_2372 */
	{ 2412, 2412, 20, 0, 20, 5, NO_DFS, NO_PSCAN, 0},	/* W1_2412_2412 */
	{ 2417, 2432, 20, 0, 20, 5, NO_DFS, NO_PSCAN, 0},	/* W1_2417_2432 */
	{ 2437, 2442, 20, 0, 20, 5, NO_DFS, NO_PSCAN, 0},	/* W1_2437_2442 */
	{ 2447, 2457, 20, 0, 20, 5, NO_DFS, NO_PSCAN, 0},	/* W1_2447_2457 */
	{ 2462, 2462, 20, 0, 20, 5, NO_DFS, NO_PSCAN, 0},	/* W1_2462_2462 */
	{ 2467, 2467, 20, 0, 20, 5, NO_DFS, PSCAN_WWR | IS_ECM_CHAN, 0}, /* W1_2467_2467 */
	{ 2467, 2467, 20, 0, 20, 5, NO_DFS, NO_PSCAN | IS_ECM_CHAN, 0},	/* W2_2467_2467 */
	{ 2472, 2472, 20, 0, 20, 5, NO_DFS, PSCAN_WWR | IS_ECM_CHAN, 0}, /* W1_2472_2472 */
	{ 2472, 2472, 20, 0, 20, 5, NO_DFS, NO_PSCAN | IS_ECM_CHAN, 0},	/* W2_2472_2472 */
	{ 2484, 2484, 20, 0, 20, 5, NO_DFS, PSCAN_WWR | IS_ECM_CHAN, 0}, /* W1_2484_2484 */
	{ 2484, 2484, 20, 0, 20, 5, NO_DFS, NO_PSCAN | IS_ECM_CHAN, 0},	/* W2_2484_2484 */
};

/*
 * 2GHz 11g channel tags
 */

enum {
#ifdef AH_SUPPORT_SC
	G1_SUPERCHANNEL,
	G2_SUPERCHANNEL,
#endif
	G1_2312_2372,
	G2_2312_2372,

	G1_2412_2472,
	G2_2412_2472,
	G3_2412_2472,

	G1_2412_2462,
	G2_2412_2462,

	G1_2432_2442,

	G1_2457_2472,

	G1_2512_2732,

	G1_2467_2472 ,
	G1_2407_2472,

	WG1_2312_2372,
	WG1_2412_2412,
	WG1_2417_2432,
	WG1_2437_2442,
	WG1_2447_2457,
	WG1_2462_2462,
	WG1_2467_2467,
	WG2_2467_2467,
	WG1_2472_2472,
	WG2_2472_2472,

	S1_907_922_5,
	S1_907_922_10,
	S1_912_917,
};
	
static REG_DMN_FREQ_BAND regDmn2Ghz11gFreq[] = {
#ifdef AH_SUPPORT_SC
	{ 2192, 2800, 30, 30, 20, 5, NO_DFS, NO_PSCAN, 0},
	{ 2224, 2800, 30, 30, 20, 5, NO_DFS, NO_PSCAN, 0},
#endif
	{ 2312, 2372, 5,  6, 20, 5, NO_DFS, NO_PSCAN, 0},	/* G1_2312_2372 */
	{ 2312, 2372, 20, 0, 20, 5, NO_DFS, NO_PSCAN, 0},	/* G2_2312_2372 */

	{ 2412, 2472, 5,  6, 20, 5, NO_DFS, NO_PSCAN, 0},	/* G1_2412_2472 */
	{ 2412, 2472, 20, 0, 20, 5,  NO_DFS, PSCAN_MKKA_G, 0},	/* G2_2412_2472 */
	{ 2412, 2472, 30, 0, 20, 5, NO_DFS, NO_PSCAN, 0},	/* G3_2412_2472 */

	{ 2412, 2462, 27, 6, 20, 5, NO_DFS, NO_PSCAN, 0},	/* G1_2412_2462 */
	{ 2412, 2462, 20, 0, 20, 5, NO_DFS, PSCAN_MKKA_G, 0},	/* G2_2412_2462 */
	
	{ 2432, 2442, 20, 0, 20, 5, NO_DFS, NO_PSCAN, 0},	/* G1_2432_2442 */

	{ 2457, 2472, 20, 0, 20, 5, NO_DFS, NO_PSCAN, 0},	/* G1_2457_2472 */

	{ 2512, 2732, 5,  6, 20, 5, NO_DFS, NO_PSCAN, 0},	/* G1_2512_2732 */

	{ 2467, 2472, 20, 0, 20, 5, NO_DFS, PSCAN_MKKA2 | PSCAN_MKKA, 0 }, /* G1_2467_2472 */
	{ 2402, 2483, 30, 0, 20, 5, NO_DFS, PSCAN_MKKA_G, 0},	/* F2_2412_2472 */

	/*
	 * WWR open up the power to 20dBm
	 */

	{ 2312, 2372, 20, 0, 20, 5, NO_DFS, NO_PSCAN, 0},	/* WG1_2312_2372 */
	{ 2412, 2412, 20, 0, 20, 5, NO_DFS, NO_PSCAN, 0},	/* WG1_2412_2412 */
	{ 2417, 2432, 20, 0, 20, 5, NO_DFS, NO_PSCAN, 0},	/* WG1_2417_2432 */
	{ 2437, 2442, 20, 0, 20, 5, NO_DFS, NO_PSCAN, 0},	/* WG1_2437_2442 */
	{ 2447, 2457, 20, 0, 20, 5, NO_DFS, NO_PSCAN, 0},	/* WG1_2447_2457 */
	{ 2462, 2462, 20, 0, 20, 5, NO_DFS, NO_PSCAN, 0},	/* WG1_2462_2462 */
	{ 2467, 2467, 20, 0, 20, 5, NO_DFS, PSCAN_WWR | IS_ECM_CHAN, 0}, /* WG1_2467_2467 */
	{ 2467, 2467, 20, 0, 20, 5, NO_DFS, NO_PSCAN | IS_ECM_CHAN, 0},	/* WG2_2467_2467 */
	{ 2472, 2472, 20, 0, 20, 5, NO_DFS, PSCAN_WWR | IS_ECM_CHAN, 0}, /* WG1_2472_2472 */
	{ 2472, 2472, 20, 0, 20, 5, NO_DFS, NO_PSCAN | IS_ECM_CHAN, 0},	/* WG2_2472_2472 */

	/*
	 * Mapping for 900MHz cards like Ubiquiti SR9.
	 */
	{ 2422, 2437, 30, 0,  5, 5, NO_DFS, PSCAN_FCC, 0 },				/* S1_907_922_5 */
	{ 2422, 2437, 30, 0, 10, 5, NO_DFS, PSCAN_FCC, 0 },				/* S1_907_922_10 */
	{ 2427, 2432, 30, 0, 20, 5, NO_DFS, PSCAN_FCC, 0 },				/* S1_912_917 */
};

/*
 * 2GHz Dynamic turbo tags
 */

enum {
#ifdef AH_SUPPORT_SC
	BT1_SUPERCHANNEL,
	BT2_SUPERCHANNEL,
#endif
	T1_2312_2372,
	T1_2437_2437,
	T2_2437_2437,
	T3_2437_2437,
	T1_2512_2732,
	T1_2407_2472
};

static REG_DMN_FREQ_BAND regDmn2Ghz11gTurboFreq[] = {
#ifdef AH_SUPPORT_SC
	{ 2192, 2800, 30, 30, 40, 5, NO_DFS, NO_PSCAN, 0},
	{ 2224, 2800, 30, 30, 40, 5, NO_DFS, NO_PSCAN, 0},
#endif
	{ 2312, 2372, 5,  6, 40, 40, NO_DFS, NO_PSCAN, 0},  /* T1_2312_2372 */
	{ 2437, 2437, 5,  6, 40, 40, NO_DFS, NO_PSCAN, 0},  /* T1_2437_2437 */
	{ 2437, 2437, 20, 6, 40, 40, NO_DFS, NO_PSCAN, 0},  /* T2_2437_2437 */
	{ 2437, 2437, 18, 6, 40, 40, NO_DFS, PSCAN_WWR, 0}, /* T3_2437_2437 */
	{ 2512, 2732, 5,  6, 40, 40, NO_DFS, NO_PSCAN, 0},  /* T1_2512_2732 */
	{ 2402, 2483, 30, 0, 40, 40, NO_DFS, NO_PSCAN, 0},	/* F2_2412_2472 */
};

typedef struct regDomain {
	u_int16_t regDmnEnum;	/* value from EnumRd table */
	u_int8_t conformanceTestLimit;
	u_int64_t dfsMask;	/* DFS bitmask for 5Ghz tables */
	u_int64_t pscan;	/* Bitmask for passive scan */
	u_int32_t flags;	/* Requirement flags (AdHoc disallow, noise
				   floor cal needed, etc) */
	u_int64_t chan11a[BMLEN];/* 128 bit bitmask for channel/band
				   selection */
	u_int64_t chan11a_turbo[BMLEN];/* 128 bit bitmask for channel/band
				   selection */
	u_int64_t chan11a_dyn_turbo[BMLEN]; /* 128 bit bitmask for channel/band
					       selection */
	u_int64_t chan11b[BMLEN];/* 128 bit bitmask for channel/band
				   selection */
	u_int64_t chan11g[BMLEN];/* 128 bit bitmask for channel/band
				   selection */
	u_int64_t chan11g_turbo[BMLEN];/* 128 bit bitmask for channel/band
					  selection */
} REG_DOMAIN;

static REG_DOMAIN regDomains[] = {

	{DEBUG_REG_DMN, FCC, DFS_FCC3, NO_PSCAN, NO_REQ,
	 BM(F1_5120_5240, F1_5260_5700, F1_5745_5825, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BM(T1_5130_5210, T1_5250_5330, T1_5370_5490, T1_5530_5650, T1_5150_5190, T1_5230_5310, T1_5350_5470, T1_5510_5670, -1, -1, -1, -1),
	 BM(T1_5200_5240, T1_5280_5280, T1_5540_5660, T1_5765_5805, -1, -1, -1, -1, -1, -1, -1, -1),
	 BM(F1_2312_2372, F1_2412_2472, F1_2484_2484, F1_2512_2732, -1, -1, -1, -1, -1, -1, -1, -1),
	 BM(G1_2312_2372, G1_2412_2472, G1_2512_2732, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BM(T1_2312_2372, T1_2437_2437, T1_2512_2732, -1, -1, -1, -1, -1, -1, -1, -1, -1)},

	{APL1, FCC, NO_DFS, NO_PSCAN, NO_REQ,
	 BM(F4_5745_5825, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BMZERO,
	 BMZERO,
	 BMZERO,
	 BMZERO,
	 BMZERO},

	{APL2, FCC, NO_DFS, NO_PSCAN, NO_REQ,
	 BM(F1_5745_5805, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BMZERO,
	 BMZERO,
	 BMZERO,
	 BMZERO,
	 BMZERO},

	{APL3, FCC, NO_DFS, NO_PSCAN, NO_REQ,
	 BM(F1_5280_5320, F2_5745_5805, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BMZERO,
	 BMZERO,
	 BMZERO,
	 BMZERO,
	 BMZERO},

	{APL4, FCC, NO_DFS, NO_PSCAN, NO_REQ,
	 BM(F4_5180_5240,  F3_5745_5825, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BMZERO,
	 BMZERO,
	 BMZERO,
	 BMZERO,
	 BMZERO},

	{APL5, FCC, NO_DFS, NO_PSCAN, NO_REQ,
	 BM(F2_5745_5825, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BMZERO,
	 BMZERO,
	 BMZERO,
	 BMZERO,
	 BMZERO},

	{APL6, ETSI, DFS_ETSI, PSCAN_FCC_T | PSCAN_FCC , NO_REQ,
	 BM(F4_5180_5240, F2_5260_5320, F3_5745_5825, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BM(T2_5210_5210, T1_5250_5290, T1_5760_5800, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BMZERO,
	 BMZERO,
	 BMZERO,
	 BMZERO},
	{APL7, FCC, DFS_FCC3 | DFS_ETSI, PSCAN_FCC | PSCAN_ETSI , NO_REQ,
	 BM(F1_5280_5320, F5_5500_5700, F3_5745_5805, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BM(T3_5290_5290, T5_5760_5800, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BM(T1_5540_5660, T6_5765_5805, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BMZERO,
	 BMZERO,
	 CHAN_TURBO_G_BMZERO
     },

	{APL8, ETSI, NO_DFS, NO_PSCAN, DISALLOW_ADHOC_11A|DISALLOW_ADHOC_11A_TURB,
	 BM(F6_5260_5320, F4_5745_5825, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BMZERO,
	 BMZERO,
	 BMZERO,
	 BMZERO,
	 BMZERO},

	{APL9, ETSI, DFS_ETSI, PSCAN_ETSI, DISALLOW_ADHOC_11A|DISALLOW_ADHOC_11A_TURB,
	 BM(F1_5180_5320, F1_5500_5620, F3_5745_5805, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BMZERO,
	 BMZERO,
	 BMZERO,
	 BMZERO,
	 BMZERO},

 	{APL10, ETSI, DFS_ETSI, PSCAN_ETSI , DISALLOW_ADHOC_11A|DISALLOW_ADHOC_11A_TURB,
	 BM(F1_5180_5320, F5_5500_5700, F3_5745_5805, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BM(T3_5290_5290, T5_5760_5800, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BM(T1_5540_5660, T6_5765_5805, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BMZERO,
	 BMZERO,
	 CHAN_TURBO_G_BMZERO
     },
	{APL11, ETSI, DFS_ETSI, PSCAN_FCC_T | PSCAN_FCC , NO_REQ,
	 BM(F4_5180_5240, F2_5260_5320, F3_5745_5825, F1_5825_5875, -1, -1, -1, -1, -1, -1, -1, -1),
	 BM(T2_5210_5210, T1_5250_5290, T1_5760_5800, T1_5825_5875, -1, -1, -1, -1, -1, -1, -1, -1),
	 BMZERO,
	 BMZERO,
	 BMZERO,
	 BMZERO},

	{ETSI1, ETSI, DFS_ETSI, PSCAN_ETSI, DISALLOW_ADHOC_11A | DISALLOW_ADHOC_11A_TURB,
	 BM(W2_5180_5240, F2_5260_5320, F2_5500_5700, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BM(T1_5210_5210, T1_5250_5290, T1_5530_5670, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BMZERO,
	 BMZERO,
	 BMZERO,
	 BMZERO},

	{ETSI7, ETSI, DFS_ETSI, PSCAN_ETSI, NO_REQ,
	 BM(U1_5725_5795, U1_5815_5850, -1, -1, -1,  -1, -1, -1, -1, -1, -1, -1),
	 BM(UT1_5725_5795, UT1_5815_5850, -1, -1, -1,  -1, -1, -1, -1, -1, -1, -1),
	 BM(UT1_5725_5795, UT1_5815_5850, -1, -1, -1,  -1, -1, -1, -1, -1, -1, -1),
	 BMZERO,
	 BMZERO,
	 BMZERO},

	{ETSI8, ETSI, DFS_ETSI, PSCAN_ETSI, NO_REQ,
	 BM(U2_5725_5795, U2_5815_5850, -1, -1, -1,  -1, -1, -1, -1, -1, -1, -1),
	 BM(UT2_5725_5795, UT2_5815_5850, -1, -1, -1,  -1, -1, -1, -1, -1, -1, -1),
	 BM(UT2_5725_5795, UT2_5815_5850, -1, -1, -1,  -1, -1, -1, -1, -1, -1, -1),
	 BMZERO,
	 BMZERO,
	 BMZERO},

	{EGAL1, ETSI, NO_DFS, NO_PSCAN, NO_REQ,
	 BM(W2_5180_5240, F2_5260_5320, F2_5500_5700, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BM(T1_5210_5210, T1_5250_5290, T1_5530_5670, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BMZERO,
	 BMZERO,
	 BMZERO,
	 BMZERO},

	{BFWA1 , ETSI, DFS_ETSI, PSCAN_ETSI, NO_REQ,
	 BM(F1_5755_5875, -1,-1, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BM(T1_5755_5875, -1,-1, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BM(T1_5755_5875, -1,-1, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BMZERO,
	 BMZERO,
	 BMZERO},

#ifdef AH_CARS

	{CARSREG1 , ETSI, DFS_ETSI, PSCAN_ETSI, NO_REQ,
	 BM(F1_5800_5925, -1,-1, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BMZERO,
	 BMZERO,
	 BMZERO,
	 BMZERO,
	 BMZERO},
#endif
	{ETSI2, ETSI, DFS_ETSI, PSCAN_ETSI, DISALLOW_ADHOC_11A | DISALLOW_ADHOC_11A_TURB,
	 BM(F3_5180_5240, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BMZERO,
	 BMZERO,
	 BMZERO,
	 BMZERO,
	 BMZERO},

	{ETSI3, ETSI, DFS_ETSI, PSCAN_ETSI, DISALLOW_ADHOC_11A | DISALLOW_ADHOC_11A_TURB,
	 BM(W2_5180_5240, F2_5260_5320, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BMZERO,
	 BMZERO,
	 BMZERO,
	 BMZERO,
	 BMZERO},

	{ETSI4, ETSI, DFS_ETSI, PSCAN_ETSI, DISALLOW_ADHOC_11A | DISALLOW_ADHOC_11A_TURB,
	 BM(F3_5180_5240, F1_5260_5320, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BMZERO,
	 BMZERO,
	 BMZERO,
	 BMZERO,
	 BMZERO},

	{ETSI5, ETSI, DFS_ETSI, PSCAN_ETSI, DISALLOW_ADHOC_11A | DISALLOW_ADHOC_11A_TURB,
	 BM(F1_5180_5240, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BMZERO,
	 BMZERO,
	 BMZERO,
	 BMZERO,
	 BMZERO},

	{ETSI6, ETSI, DFS_ETSI, PSCAN_ETSI, DISALLOW_ADHOC_11A | DISALLOW_ADHOC_11A_TURB,
	 BM(F5_5180_5240, F1_5260_5280, F3_5500_5700, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BMZERO,
	 BMZERO,
	 BMZERO,
	 BMZERO,
	 BMZERO},

	{FCC1, FCC, NO_DFS, NO_PSCAN, NO_REQ,
	 BM(F2_5180_5240, F4_5260_5320, F5_5745_5825, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BM(T1_5210_5210, T2_5250_5290, T2_5760_5800, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BM(T1_5200_5240, T1_5280_5280, T1_5765_5805, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BMZERO,
	 BMZERO,
	 BMZERO},

	{FCC2, FCC, NO_DFS, NO_PSCAN, NO_REQ,
	 BM(F6_5180_5240, F5_5260_5320, F6_5745_5825, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BM(-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BM(T2_5200_5240, T1_5280_5280, T1_5765_5805, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BMZERO,
	 BMZERO,
	 BMZERO},

	{FCC3, FCC, DFS_FCC3, PSCAN_FCC | PSCAN_FCC_T, NO_REQ,
	 BM(F2_5180_5240, F3_5260_5320, F1_5500_5700, F5_5745_5825, -1, -1, -1, -1, -1, -1, -1, -1),
	 BM(T1_5210_5210, T1_5250_5250, T1_5290_5290, T2_5760_5800, -1, -1, -1, -1, -1, -1, -1, -1),
	 BM(T1_5200_5240, T2_5280_5280, T1_5540_5660, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BMZERO,
	 BMZERO,
	 BMZERO},

	{FCC4, FCC, DFS_FCC3, PSCAN_FCC | PSCAN_FCC_T, NO_REQ,
	 BM(F1_4942_4987, F1_4945_4985, F1_4950_4980, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BMZERO,
	 BMZERO,
	 BMZERO,
	 BMZERO,
	 BMZERO},

	{FCC5, FCC, NO_DFS, NO_PSCAN, NO_REQ,
	 BM(F2_5180_5240, F6_5745_5825, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BM(T6_5210_5210, T2_5760_5800, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BM(T8_5200_5200, T7_5765_5805, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BMZERO,
	 BMZERO,
	 CHAN_TURBO_G_BMZERO
     },
	
	{FCC6, FCC, DFS_FCC3, PSCAN_FCC, NO_REQ,
	 BM(F8_5180_5240, F5_5260_5320, F1_5500_5580, F1_5660_5700, F6_5745_5825, -1, -1, -1, -1, -1, -1, -1),
	 BM(T7_5210_5210, T3_5250_5290, T2_5760_5800, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BM(T7_5200_5200, T1_5240_5240, T2_5280_5280, T1_5765_5805, -1, -1, -1, -1, -1, -1, -1, -1),
	 BMZERO,
	 BMZERO,
	 CHAN_TURBO_G_BMZERO
     },

	{MKK1, MKK, NO_DFS, PSCAN_MKK1, DISALLOW_ADHOC_11A_TURB,
	 BM(F1_5170_5230, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BMZERO,
	 BMZERO,
	 BMZERO,
	 BMZERO,
	 BMZERO},

	{MKK2, MKK, NO_DFS, PSCAN_MKK2, DISALLOW_ADHOC_11A_TURB,
	 BM(F1_4915_4925, F1_4935_4945, F1_4920_4980, F1_5035_5040, F1_5055_5055, F1_5040_5080, F1_5170_5230, -1, -1, -1, -1, -1),
	 BMZERO,
	 BMZERO,
	 BMZERO,
	 BMZERO,
	 BMZERO},

	/* UNI-1 even */
	{MKK3, MKK, NO_DFS, PSCAN_MKK3, DISALLOW_ADHOC_11A_TURB,
	 BM(F4_5180_5240, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BMZERO,
	 BMZERO,
	 BMZERO,
	 BMZERO,
	 BMZERO},

	/* UNI-1 even + UNI-2 */
	{MKK4, MKK, DFS_MKK4, PSCAN_MKK3, DISALLOW_ADHOC_11A_TURB,
	 BM(F4_5180_5240, F2_5260_5320, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BMZERO,
	 BMZERO,
	 BMZERO,
	 BMZERO,
	 BMZERO},

	/* UNI-1 even + UNI-2 + mid-band */
	{MKK5, MKK, DFS_MKK4, PSCAN_MKK3, DISALLOW_ADHOC_11A_TURB,
	 BM(F4_5180_5240, F2_5260_5320, F4_5500_5700, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BMZERO,
	 BMZERO,
	 BMZERO,
	 BMZERO,
	 BMZERO},

	/* UNI-1 odd + even */
	{MKK6, MKK, NO_DFS, PSCAN_MKK1, DISALLOW_ADHOC_11A_TURB,
	 BM(F2_5170_5230, F4_5180_5240, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BMZERO,
	 BMZERO,
	 BMZERO,
	 BMZERO,
	 BMZERO},

	/* UNI-1 odd + UNI-1 even + UNI-2 */
	{MKK7, MKK, DFS_MKK4, PSCAN_MKK1 | PSCAN_MKK3 , DISALLOW_ADHOC_11A_TURB,
	 BM(F1_5170_5230, F4_5180_5240, F2_5260_5320, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BMZERO,
	 BMZERO,
	 BMZERO,
	 BMZERO,
	 BMZERO},

	/* UNI-1 odd + UNI-1 even + UNI-2 + mid-band */
	{MKK8, MKK, DFS_MKK4, PSCAN_MKK1 | PSCAN_MKK3 , DISALLOW_ADHOC_11A_TURB,
	 BM(F1_5170_5230, F4_5180_5240, F2_5260_5320, F4_5500_5700, -1, -1, -1, -1, -1, -1, -1, -1),
	 BMZERO,
	 BMZERO,
	 BMZERO,
	 BMZERO,
	 BMZERO},

        /* UNI-1 even + 4.9 GHZ */
        {MKK9, MKK, NO_DFS, PSCAN_MKK3, DISALLOW_ADHOC_11A_TURB,
         BM(F1_4915_4925, F1_4935_4945, F1_4920_4980, F1_5035_5040, F1_5055_5055, F1_5040_5080, F4_5180_5240, -1, -1, -1, -1, -1),
         BMZERO,
         BMZERO,
         BMZERO,
         BMZERO,
         BMZERO},

        /* UNI-1 even + UNI-2 + 4.9 GHZ */
        {MKK10, MKK, DFS_MKK4, PSCAN_MKK3, DISALLOW_ADHOC_11A_TURB,
         BM(F1_4915_4925, F1_4935_4945, F1_4920_4980, F1_5035_5040, F1_5055_5055, F1_5040_5080, F4_5180_5240, F2_5260_5320, -1, -1, -1, -1),
         BMZERO,
         BMZERO,
         BMZERO,
         BMZERO,
         BMZERO},

	/* Defined here to use when 2G channels are authorised for country K2 */
	{APLD, NO_CTL, NO_DFS, NO_PSCAN, NO_REQ,
	 BMZERO,
	 BMZERO,
	 BMZERO,
	 BM(F2_2312_2372,F2_2412_2472, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BM(G2_2312_2372,G2_2412_2472, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1),
     BMZERO},

	{ETSIA, NO_CTL, NO_DFS, PSCAN_ETSIA, DISALLOW_ADHOC_11A | DISALLOW_ADHOC_11A_TURB,
	 BMZERO,
	 BMZERO,
	 BMZERO,
	 BM(F1_2457_2472,-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BM(G1_2457_2472,-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BM(T2_2437_2437,-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1)},

	{ETSIB, ETSI, NO_DFS, PSCAN_ETSIB, DISALLOW_ADHOC_11A | DISALLOW_ADHOC_11A_TURB,
	 BMZERO,
	 BMZERO,
	 BMZERO,
	 BM(F1_2432_2442,-1,-1,-1,-1,-1,-1,-1, -1, -1, -1, -1),
	 BM(G1_2432_2442,-1,-1,-1,-1,-1,-1,-1, -1, -1, -1, -1),
	 BM(T2_2437_2437,-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1)},

	{ETSIC, ETSI, NO_DFS, PSCAN_ETSIC, DISALLOW_ADHOC_11A | DISALLOW_ADHOC_11A_TURB,
	 BMZERO,
	 BMZERO,
	 BMZERO,
	 BM(F3_2412_2472,-1,-1,-1,-1,-1,-1,-1, -1, -1, -1, -1),
	 BM(G3_2412_2472,-1,-1,-1,-1,-1,-1,-1, -1, -1, -1, -1),
	 BM(T2_2437_2437,-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1)},

	{FCCA, FCC, NO_DFS, NO_PSCAN, NO_REQ,
	 BMZERO,
	 BMZERO,
	 BMZERO,
	 BM(F1_2412_2462,-1,-1,-1,-1,-1,-1,-1, -1, -1, -1, -1),
	 BM(G1_2412_2462,-1,-1,-1,-1,-1,-1,-1, -1, -1, -1, -1),
	 BM(T2_2437_2437,-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1)},

	{MKKA, MKK, NO_DFS, PSCAN_MKKA | PSCAN_MKKA_G | PSCAN_MKKA1 | PSCAN_MKKA1_G | PSCAN_MKKA2 | PSCAN_MKKA2_G, DISALLOW_ADHOC_11A_TURB,
	 BMZERO,
	 BMZERO,
	 BMZERO,
	 BM(F2_2412_2462, F1_2467_2472, F2_2484_2484, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BM(G2_2412_2462, G1_2467_2472, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BM(T2_2437_2437,-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1)},

	{MKKC, MKK, NO_DFS, NO_PSCAN, NO_REQ,
	 BMZERO,
	 BMZERO,
	 BMZERO,
	 BM(F2_2412_2472,-1,-1,-1,-1,-1,-1,-1, -1, -1, -1, -1),
	 BM(G2_2412_2472,-1,-1,-1,-1,-1,-1,-1, -1, -1, -1, -1),
	 BM(T2_2437_2437,-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1)},

	{WORLD, ETSI, NO_DFS, NO_PSCAN, NO_REQ,
	 BMZERO,
	 BMZERO,
	 BMZERO,
	 BM(F2_2412_2472,-1,-1,-1,-1,-1,-1,-1, -1, -1, -1, -1),
	 BM(G2_2412_2472,-1,-1,-1,-1,-1,-1,-1, -1, -1, -1, -1),
	 BM(T2_2437_2437,-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1)},

	{WORLD11, ETSI, NO_DFS, NO_PSCAN, NO_REQ,
	 BMZERO,
	 BMZERO,
	 BMZERO,
	 BM(F1_2407_2472,-1,-1,-1,-1,-1,-1,-1, -1, -1, -1, -1),
	 BM(G1_2407_2472,-1,-1,-1,-1,-1,-1,-1, -1, -1, -1, -1),
	 BM(T1_2407_2472,-1,-1,-1,-1,-1,-1,-1, -1, -1, -1, -1)},
#ifdef RAIEXTRA
	{IT, NO_CTL, NO_DFS, NO_PSCAN, NO_REQ,
	 BM(F1_5500_5700_RAI, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BM(T1_5500_5700_RAI, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BM(T1_5500_5700_RAI, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BMZERO,
	 BMZERO,
	 BMZERO},

	{RAI, NO_CTL, NO_DFS, NO_PSCAN, NO_REQ,
	 BM(F1_5845_5925_RAI, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BM(T1_5845_5925_RAI, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BM(T1_5845_5925_RAI, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BMZERO,
	 BMZERO,
	 BMZERO},
	 
	{RAIIT, NO_CTL, NO_DFS, NO_PSCAN, NO_REQ,
	 BM(F1_5500_5700_RAI, F1_5845_5925_RAI, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BM(T1_5500_5700_RAI, T1_5845_5925_RAI, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BM(T1_5500_5700_RAI, T1_5845_5925_RAI, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BMZERO,
	 BMZERO,
	 BMZERO},
#endif
#ifdef AH_SUPPORT_SC
	{WOR0_WORLD, NO_CTL, NO_DFS, NO_PSCAN, NO_REQ,
	 BM(F1_SUPERCHANNEL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BM(T1_SUPERCHANNEL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BM(T2_SUPERCHANNEL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BM(B1_SUPERCHANNEL, B2_SUPERCHANNEL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BM(G1_SUPERCHANNEL, G2_SUPERCHANNEL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BM(BT1_SUPERCHANNEL, BT2_SUPERCHANNEL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1)},
#else
	{WOR0_WORLD, NO_CTL, DFS_FCC3 | DFS_ETSI, PSCAN_WWR, ADHOC_PER_11D,
	 BM(W1_5260_5320, W1_5180_5240, W1_5170_5230, W1_5745_5825, W1_5500_5700, -1, -1, -1, -1, -1, -1, -1),
	 BM(WT1_5210_5250, WT1_5290_5290, WT1_5760_5800, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BMZERO,
	 BM(W1_2412_2412,W1_2437_2442,W1_2462_2462,W1_2472_2472,W1_2417_2432, W1_2447_2457, W1_2467_2467, W1_2484_2484, -1, -1, -1, -1),
	 BM(WG1_2412_2412,WG1_2437_2442,WG1_2462_2462,WG1_2472_2472,WG1_2417_2432,WG1_2447_2457,WG1_2467_2467, -1, -1, -1, -1, -1),
	 BM(T3_2437_2437, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1)},
#endif

	{WOR01_WORLD, NO_CTL, DFS_FCC3 | DFS_ETSI, PSCAN_WWR, ADHOC_PER_11D,
	 BM(W1_5260_5320, W1_5180_5240, W1_5170_5230, W1_5745_5825, W1_5500_5700, -1, -1, -1, -1, -1, -1, -1),
	 BM(WT1_5210_5250, WT1_5290_5290, WT1_5760_5800, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BMZERO,
	 BM(W1_2412_2412, W1_2437_2442, W1_2462_2462, W1_2417_2432, W1_2447_2457, -1, -1, -1, -1, -1, -1, -1),
	 BM(WG1_2412_2412, WG1_2437_2442, WG1_2462_2462, WG1_2417_2432, WG1_2447_2457, -1, -1, -1, -1, -1, -1, -1),
	 BM(T3_2437_2437, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1)},

	{WOR02_WORLD, NO_CTL, DFS_FCC3 | DFS_ETSI, PSCAN_WWR, ADHOC_PER_11D,
	 BM(W1_5260_5320, W1_5180_5240,W1_5170_5230,W1_5745_5825,W1_5500_5700, -1, -1, -1, -1, -1, -1, -1),
	 BM(WT1_5210_5250, WT1_5290_5290, WT1_5760_5800, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BMZERO,
	 BM(W1_2412_2412,W1_2437_2442,W1_2462_2462, W1_2472_2472,W1_2417_2432, W1_2447_2457, W1_2467_2467, -1, -1, -1, -1, -1),
	 BM(WG1_2412_2412,WG1_2437_2442,WG1_2462_2462, WG1_2472_2472,WG1_2417_2432, WG1_2447_2457, WG1_2467_2467, -1, -1, -1, -1, -1),
	 BM(T3_2437_2437, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1)},

	{EU1_WORLD, NO_CTL, DFS_FCC3 | DFS_ETSI, PSCAN_WWR, ADHOC_PER_11D,
	 BM(W1_5260_5320, W1_5180_5240,W1_5170_5230,W1_5745_5825,W1_5500_5700, -1, -1, -1, -1, -1, -1, -1),
	 BM(WT1_5210_5250, WT1_5290_5290, WT1_5760_5800, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BMZERO,
	 BM(W1_2412_2412,W1_2437_2442,W1_2462_2462, W2_2472_2472,W1_2417_2432, W1_2447_2457, W2_2467_2467, -1, -1, -1, -1, -1),
	 BM(WG1_2412_2412,WG1_2437_2442,WG1_2462_2462, WG2_2472_2472,WG1_2417_2432, WG1_2447_2457, WG2_2467_2467, -1, -1, -1, -1, -1),
	 BM(T3_2437_2437, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1)},

	{WOR1_WORLD, NO_CTL, DFS_FCC3 | DFS_ETSI, PSCAN_WWR, ADHOC_NO_11A,
	 BM(W1_5260_5320, W1_5180_5240, W1_5170_5230, W1_5745_5825, W1_5500_5700, -1, -1, -1, -1, -1, -1, -1),
	 BMZERO,
	 BMZERO,
	 BM(W1_2412_2412,W1_2437_2442,W1_2462_2462,W1_2472_2472,W1_2417_2432, W1_2447_2457, W1_2467_2467, W1_2484_2484, -1, -1, -1, -1),
	 BM(WG1_2412_2412,WG1_2437_2442,WG1_2462_2462,WG1_2472_2472,WG1_2417_2432,WG1_2447_2457,WG1_2467_2467, -1, -1, -1, -1, -1),
	 BM(T3_2437_2437, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1)},

	{WOR2_WORLD, NO_CTL, DFS_FCC3 | DFS_ETSI, PSCAN_WWR, ADHOC_NO_11A,
	 BM(W1_5260_5320, W1_5180_5240, W1_5170_5230, W1_5745_5825, W1_5500_5700, -1, -1, -1, -1, -1, -1, -1),
	 BM(WT1_5210_5250, WT1_5290_5290, WT1_5760_5800, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BMZERO,
	 BM(W1_2412_2412,W1_2437_2442,W1_2462_2462,W1_2472_2472,W1_2417_2432, W1_2447_2457, W1_2467_2467, W1_2484_2484, -1, -1, -1, -1),
	 BM(WG1_2412_2412,WG1_2437_2442,WG1_2462_2462,WG1_2472_2472,WG1_2417_2432,WG1_2447_2457,WG1_2467_2467, -1, -1, -1, -1, -1),
	 BM(T3_2437_2437, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1)},

	{WOR3_WORLD, NO_CTL, DFS_FCC3 | DFS_ETSI, PSCAN_WWR, ADHOC_PER_11D,
	 BM(W1_5260_5320, W1_5180_5240, W1_5170_5230, W1_5745_5825, -1, -1, -1, -1, -1, -1, -1, -1),
	 BM(WT1_5210_5250, WT1_5290_5290, WT1_5760_5800, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BMZERO,
	 BM(W1_2412_2412,W1_2437_2442,W1_2462_2462,W1_2472_2472,W1_2417_2432, W1_2447_2457, W1_2467_2467, -1, -1, -1, -1, -1),
	 BM(WG1_2412_2412,WG1_2437_2442,WG1_2462_2462,WG1_2472_2472,WG1_2417_2432,WG1_2447_2457,WG1_2467_2467,-1, -1, -1, -1, -1),
	 BM(T3_2437_2437, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1)},

	{WOR4_WORLD, NO_CTL, DFS_FCC3 | DFS_ETSI, PSCAN_WWR, ADHOC_NO_11A,
	 BM(W2_5260_5320, W2_5180_5240, F2_5745_5805, W2_5825_5825, -1, -1, -1, -1, -1, -1, -1, -1),
	 BM(WT1_5210_5250, WT1_5290_5290, WT1_5760_5800, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BMZERO,
	 BM(W1_2412_2412,W1_2437_2442,W1_2462_2462, W1_2417_2432,W1_2447_2457,-1, -1, -1, -1, -1, -1, -1),
	 BM(WG1_2412_2412,WG1_2437_2442,WG1_2462_2462, WG1_2417_2432,WG1_2447_2457,-1, -1, -1, -1, -1, -1, -1),
	 BM(T3_2437_2437, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1)},

	{WOR5_ETSIC, NO_CTL, DFS_FCC3 | DFS_ETSI, PSCAN_WWR, ADHOC_NO_11A,
	 BM(W1_5260_5320, W2_5180_5240, F6_5745_5825, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BMZERO,
	 BMZERO,
	 BM(W1_2412_2412, W1_2437_2442, W1_2462_2462, W2_2472_2472, W1_2417_2432, W1_2447_2457, W2_2467_2467, -1, -1, -1, -1, -1),
	 BM(WG1_2412_2412, WG1_2437_2442, WG1_2462_2462, WG2_2472_2472, WG1_2417_2432, WG1_2447_2457, WG2_2467_2467, -1, -1, -1, -1, -1),
	 BM(T3_2437_2437, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1)},

	{WOR9_WORLD, NO_CTL, DFS_FCC3 | DFS_ETSI, PSCAN_WWR, ADHOC_NO_11A,
	 BM(W1_5260_5320, W1_5180_5240, W1_5745_5825, W1_5500_5700, -1, -1, -1, -1, -1, -1, -1, -1),
	 BM(WT1_5210_5250, WT1_5290_5290, WT1_5760_5800, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BMZERO,
	 BM(W1_2412_2412, W1_2437_2442, W1_2462_2462, W1_2417_2432, W1_2447_2457, -1, -1, -1, -1, -1, -1, -1),
	 BM(WG1_2412_2412, WG1_2437_2442, WG1_2462_2462, WG1_2417_2432, WG1_2447_2457, -1, -1, -1, -1, -1, -1, -1),
	 BM(T3_2437_2437, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1)},

	{WORA_WORLD, NO_CTL, DFS_FCC3 | DFS_ETSI, PSCAN_WWR, ADHOC_NO_11A,
	 BM(W1_5260_5320, W1_5180_5240, W1_5745_5825, W1_5500_5700, -1, -1, -1, -1, -1, -1, -1, -1),
	 BMZERO,
	 BMZERO,
	 BM(W1_2412_2412, W1_2437_2442, W1_2462_2462, W1_2472_2472, W1_2417_2432, W1_2447_2457, W1_2467_2467, -1, -1, -1, -1, -1),
	 BM(WG1_2412_2412, WG1_2437_2442, WG1_2462_2462, WG1_2472_2472, WG1_2417_2432, WG1_2447_2457, WG1_2467_2467, -1, -1, -1, -1, -1),
	 BM(T3_2437_2437, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1)},

	{GSM, NO_CTL, NO_DFS, PSCAN_FCC | PSCAN_FCC_T, NO_REQ,
	 BMZERO,
	 BMZERO,
	 BMZERO,
	 BMZERO,
	 BM(S1_907_922_5, S1_907_922_10, S1_912_917, -1, -1, -1, -1, -1, -1, -1, -1, -1),
	 BMZERO},

	{NULL1, NO_CTL, NO_DFS, NO_PSCAN, NO_REQ,
	 BMZERO,
	 BMZERO,
	 BMZERO,
	 BMZERO,
	 BMZERO,
	 BMZERO}
};

struct cmode {
	u_int	mode;
	u_int	flags;
};

static const struct cmode modes[] = {
	{ HAL_MODE_TURBO,	CHANNEL_ST},	/* TURBO means 11a Static Turbo */
	{ HAL_MODE_11A,		CHANNEL_A},
	{ HAL_MODE_11B,		CHANNEL_B},
	{ HAL_MODE_11G,		CHANNEL_G},
	{ HAL_MODE_11G_TURBO,	CHANNEL_108G},
	{ HAL_MODE_11A_TURBO,	CHANNEL_108A},
	{ HAL_MODE_11NG_HT20,	CHANNEL_G_HT20},
	{ HAL_MODE_11NG_HT40PLUS,	CHANNEL_G_HT40PLUS},
	{ HAL_MODE_11NG_HT40MINUS,	CHANNEL_G_HT40MINUS},
	{ HAL_MODE_11NA_HT20,	CHANNEL_A_HT20},
	{ HAL_MODE_11NA_HT40PLUS,	CHANNEL_A_HT40PLUS},
	{ HAL_MODE_11NA_HT40MINUS,	CHANNEL_A_HT40MINUS},
};

static int
chansort(const void *a, const void *b)
{
#define CHAN_FLAGS	(CHANNEL_ALL|CHANNEL_HALF|CHANNEL_QUARTER|CHANNEL_SUBQUARTER)
	const HAL_CHANNEL_INTERNAL *ca = a;
	const HAL_CHANNEL_INTERNAL *cb = b;

	return (ca->channel == cb->channel) ?
		(ca->channelFlags & CHAN_FLAGS) -
			(cb->channelFlags & CHAN_FLAGS) :
		ca->channel - cb->channel;
#undef CHAN_FLAGS
}
typedef int ath_hal_cmp_t(const void *, const void *);
static	void ath_hal_sort(void *a, size_t n, size_t es, ath_hal_cmp_t *cmp);
static COUNTRY_CODE_TO_ENUM_RD* findCountry(HAL_CTRY_CODE countryCode);
static HAL_BOOL getWmRD(struct ath_hal *ah, COUNTRY_CODE_TO_ENUM_RD *country, u_int16_t channelFlag, REG_DOMAIN *rd, HAL_BOOL sc);
static void ath_hal_update_regdomain(struct ath_hal *ah);


static u_int16_t
getEepromRD(struct ath_hal *ah, HAL_BOOL sc)
{
#ifdef AH_CARS
		return CARSREG;
#else
#ifdef AH_SUPPORT_SC
	if (AH_PRIVATE(ah)->ah_useSC && sc)
		return WOR0_WORLD;
#endif
	return AH_PRIVATE(ah)->ah_currentRD &~ WORLDWIDE_ROAMING_FLAG;
#endif
}

/*
 * Test to see if the bitmask array is all zeros
 */
static HAL_BOOL
isChanBitMaskZero(u_int64_t *bitmask)
{
	int i;

	for (i=0; i<BMLEN; i++) {
		if (bitmask[i] != 0)
			return AH_FALSE;
	}
	return AH_TRUE;
}

/*
 * Return whether or not the regulatory domain/country in EEPROM
 * is acceptable.
 */
static HAL_BOOL
isEepromValid(struct ath_hal *ah)
{
	u_int16_t rd = getEepromRD(ah, AH_TRUE);
	int i;

#ifdef AH_SUPPORT_SC
	if (AH_PRIVATE(ah)->ah_useSC && (rd==WOR0_WORLD))
	    return AH_TRUE;
#endif

	if (rd & COUNTRY_ERD_FLAG) {
		u_int16_t cc = rd &~ COUNTRY_ERD_FLAG;
		for (i = 0; i < N(allCountries); i++)
			if (allCountries[i].countryCode == cc)
				return AH_TRUE;
	} else {
		for (i = 0; i < N(regDomainPairs); i++)
			if (regDomainPairs[i].regDmnEnum == rd)
				return AH_TRUE;
	}
	HALDEBUG(ah, "%s: invalid regulatory domain/country code 0x%x\n",
		__func__, rd);
	return AH_FALSE;
}

/*
 * Returns whether or not the specified country code
 * is allowed by the EEPROM setting
 */
static HAL_BOOL
isCountryCodeValid(struct ath_hal *ah, HAL_CTRY_CODE cc)
{
	u_int16_t rd;

	/* Default setting requires no checks */
	if (cc == CTRY_DEFAULT)
		return AH_TRUE;
#ifdef AH_DEBUG_COUNTRY
	if (cc == CTRY_DEBUG)
		return AH_TRUE;
#endif
	rd = getEepromRD(ah, AH_TRUE);
#ifdef AH_SUPPORT_SC
	if (rd == WOR0_WORLD)
	    return AH_TRUE;
#endif
	HALDEBUG(ah, "%s: EEPROM regdomain 0x%x\n", __func__, rd);

	if (rd & COUNTRY_ERD_FLAG) {
		/* EEP setting is a country - config shall match */
		HALDEBUG(ah, "%s: EEPROM setting is country code %u\n",
			__func__, rd &~ COUNTRY_ERD_FLAG);
		return (cc == (rd & ~COUNTRY_ERD_FLAG));
	} else if (rd == DEBUG_REG_DMN || rd == NO_ENUMRD) {
		/* Set to Debug or AllowAnyCountry mode - allow any setting */
		HALDEBUG(ah, "%s: DEBUG or NO\n", __func__);
		return AH_TRUE;
#ifdef AH_SUPPORT_11D
	} else	if ((rd & WORLD_SKU_MASK) == WORLD_SKU_PREFIX) {
		int i;
		for (i=0; i < N(allCountries); i++) {
			if (cc == allCountries[i].countryCode)
				return AH_TRUE;
		}
#endif
	} else {
		int i;
		for (i = 0; i < N(allCountries); i++) {
			if (cc == allCountries[i].countryCode &&
			    allCountries[i].regDmnEnum == rd)
				return AH_TRUE;
		}
	}
	return AH_FALSE;
}

/*
 * Return the mask of available modes based on the hardware
 * capabilities and the specified country code and reg domain.
 */
static u_int
ath_hal_getwmodesnreg(struct ath_hal *ah, COUNTRY_CODE_TO_ENUM_RD *country,
			 REG_DOMAIN *rd5GHz)
{
	u_int modesAvail;

	/* Get modes that HW is capable of */
	modesAvail = ath_hal_getWirelessModes(ah);

	/* Check country regulations for allowed modes */
	if ((modesAvail & (HAL_MODE_11A_TURBO|HAL_MODE_TURBO)) &&
	    !country->allow11aTurbo)
		modesAvail &= ~(HAL_MODE_11A_TURBO | HAL_MODE_TURBO);
	if ((modesAvail & HAL_MODE_11G_TURBO) && !country->allow11gTurbo)
		modesAvail &= ~HAL_MODE_11G_TURBO;
	if ((modesAvail & HAL_MODE_11G) && !country->allow11g)
		modesAvail &= ~HAL_MODE_11G;
	if ((modesAvail & HAL_MODE_11A) && isChanBitMaskZero(rd5GHz->chan11a))
		modesAvail &= ~HAL_MODE_11A;

	if ((modesAvail & HAL_MODE_11NG_HT20) && !country->allow11ng20)
		modesAvail &= ~HAL_MODE_11NG_HT20;
	if ((modesAvail & HAL_MODE_11NA_HT20) && !country->allow11na20)
		modesAvail &= ~HAL_MODE_11NA_HT20;
	if ((modesAvail & HAL_MODE_11NG_HT40PLUS) && !country->allow11ng40)
		modesAvail &= ~HAL_MODE_11NG_HT40PLUS;
	if ((modesAvail & HAL_MODE_11NG_HT40MINUS) && !country->allow11ng40)
		modesAvail &= ~HAL_MODE_11NG_HT40MINUS;
	if ((modesAvail & HAL_MODE_11NA_HT40PLUS) && !country->allow11na40)
		modesAvail &= ~HAL_MODE_11NA_HT40PLUS;
	if ((modesAvail & HAL_MODE_11NA_HT40MINUS) && !country->allow11na40)
		modesAvail &= ~HAL_MODE_11NA_HT40MINUS;

	return modesAvail;
}

/*
 * Return the mask of available modes based on the hardware
 * capabilities and the specified country code.
 */

u_int
ath_hal_getwirelessmodes(struct ath_hal *ah, HAL_CTRY_CODE cc)
{
	COUNTRY_CODE_TO_ENUM_RD *country=AH_NULL;
	u_int mode=0;
	REG_DOMAIN rd;
	
	country = findCountry(cc);
	if (country != AH_NULL) {
		if (getWmRD(ah, country, ~CHANNEL_2GHZ, &rd, AH_TRUE))
			mode = ath_hal_getwmodesnreg(ah, country, &rd);
	}
	return(mode);
}

/*
 * Return if device is public safety.
 */
HAL_BOOL
ath_hal_ispublicsafetysku(struct ath_hal *ah)
{
	u_int16_t rd;

	rd = getEepromRD(ah, AH_TRUE);

	switch (rd) {
		case FCC4_FCCA:
		case (CTRY_UNITED_STATES_FCC49 | COUNTRY_ERD_FLAG):
			return AH_TRUE;

		case DEBUG_REG_DMN:
		case NO_ENUMRD:
			if (AH_PRIVATE(ah)->ah_countryCode == 
						CTRY_UNITED_STATES_FCC49) {
				return AH_TRUE;
			}
			break;
	}

	return AH_FALSE;
}

/*
 * Return if device is actually operating in 900 MHz band.
 */
HAL_BOOL
ath_hal_isgsmsku(struct ath_hal *ah)
{
	u_int16_t rd = getEepromRD(ah, AH_TRUE);

	switch (rd) {
	case NULL1_GSM:
	case (CTRY_GSM | COUNTRY_ERD_FLAG):
		return AH_TRUE;
	case DEBUG_REG_DMN:
	case NO_ENUMRD:
		return AH_PRIVATE(ah)->ah_countryCode == CTRY_GSM;
	}
	return AH_FALSE;
}

/*
 * Find the pointer to the country element in the country table
 * corresponding to the country code
 */
static COUNTRY_CODE_TO_ENUM_RD*
findCountry(HAL_CTRY_CODE countryCode)
{
	int i;

	for (i=0; i<N(allCountries); i++) {
		if (allCountries[i].countryCode == countryCode)
			return (&allCountries[i]);
	}
	return (AH_NULL);		/* Not found */
}

/*
 * Calculate a default country based on the EEPROM setting.
 */
static HAL_CTRY_CODE
getDefaultCountry(struct ath_hal *ah)
{
	u_int16_t rd;
	int i;

	rd = getEepromRD(ah, AH_FALSE);
	if (rd & COUNTRY_ERD_FLAG) {
		COUNTRY_CODE_TO_ENUM_RD *country=AH_NULL;
		u_int16_t cc= rd & ~COUNTRY_ERD_FLAG;
		
		country = findCountry(cc);
		if (country != AH_NULL)
			return cc;
	}
	/*
	 * Check reg domains that have only one country
	 */
	for (i = 0; i < N(regDomainPairs); i++)
		if (regDomainPairs[i].regDmnEnum == rd) {
			if (regDomainPairs[i].singleCC != 0)
				return regDomainPairs[i].singleCC;
			else
				i = N(regDomainPairs);
		}
	return CTRY_DEFAULT;
}

static HAL_BOOL
isValidRegDmn(int regDmn, REG_DOMAIN *rd)
{
	int i;

	for (i=0;i<N(regDomains); i++) {
		if (regDomains[i].regDmnEnum == regDmn) {
			if (rd != AH_NULL) {
				OS_MEMCPY(rd, &regDomains[i],
					  sizeof(REG_DOMAIN));
			}
			return AH_TRUE;
		}
	}
	return AH_FALSE;
}

static HAL_BOOL
isValidRegDmnPair(int regDmnPair)
{
	int i;

	if (regDmnPair == NO_ENUMRD)
		return AH_FALSE;
	for (i=0; i<N(regDomainPairs); i++) {
		if (regDomainPairs[i].regDmnEnum == regDmnPair)
			return AH_TRUE;
	}
	return AH_FALSE;
}

/*
 * Return the Wireless Mode Regulatory Domain based
 * on the country code and the wireless mode.
 */
static HAL_BOOL
getWmRD(struct ath_hal *ah, COUNTRY_CODE_TO_ENUM_RD *country, u_int16_t channelFlag, REG_DOMAIN *rd, HAL_BOOL sc)
{
	int i, found, regDmn;
	u_int64_t flags=NO_REQ;
	REG_DMN_PAIR_MAPPING *regPair=AH_NULL;
	VENDOR_PAIR_MAPPING *vendorPair=AH_NULL;
	struct ath_hal_private *ahp;

	ahp = AH_PRIVATE(ah);
	if (country->countryCode == CTRY_DEFAULT) {
		u_int16_t rdnum;
		rdnum = getEepromRD(ah, sc);

		if (!(rdnum & COUNTRY_ERD_FLAG)) {
			if (isValidRegDmn(rdnum, AH_NULL) ||
			    isValidRegDmnPair(rdnum)) {
				regDmn = rdnum;
			} else
				regDmn = country->regDmnEnum;
		} else
			regDmn = country->regDmnEnum;
	} else
		regDmn = country->regDmnEnum;
	if ((regDmn & MULTI_DOMAIN_MASK) == 0) {

		for (i=0, found=0; (i<N(regDomainPairs))&&(!found); i++) {
			if (regDomainPairs[i].regDmnEnum == regDmn) {
				regPair = &regDomainPairs[i];
				found = 1;
			}
		}
		if (!found) {
			HALDEBUG(ah, "%s: Failed to find reg domain pair %u\n",
				 __func__, regDmn);
			return AH_FALSE;
		}
		if (!(channelFlag & CHANNEL_2GHZ)) {
			regDmn = regPair->regDmn5GHz;
			flags = regPair->flags5GHz;
		}
		if (channelFlag & CHANNEL_2GHZ) {
			regDmn = regPair->regDmn2GHz;
			flags = regPair->flags2GHz;
		}
		for (i=0, found=0; (i<N(regDomainVendorPairs))&&(!found); i++) {
			if ((regDomainVendorPairs[i].regDmnEnum == regDmn) &&
			    (AH_PRIVATE(ah)->ah_vendor == regDomainVendorPairs[i].vendor)) {
				vendorPair = &regDomainVendorPairs[i];
				found = 1;
			}
		}
		if (found) {
			if (!(channelFlag & CHANNEL_2GHZ)) {
				flags &= vendorPair->flags5GHzIntersect;
				flags |= vendorPair->flags5GHzUnion;
			}
			if (channelFlag & CHANNEL_2GHZ) {
				flags &= vendorPair->flags2GHzIntersect;
				flags |= vendorPair->flags2GHzUnion;
			}
		}
	}

	/*
	 * We either started with a unitary reg domain or we've found the 
	 * unitary reg domain of the pair
	 */

	found = isValidRegDmn(regDmn, rd);
	if (!found) {
		HALDEBUG(ah, "%s: Failed to find unitary reg domain %u\n",
			 __func__, country->regDmnEnum);
		return AH_FALSE;
	} else {
		rd->pscan &= regPair->pscanMask;
		if (((country->regDmnEnum & MULTI_DOMAIN_MASK) == 0) &&
		    (flags != NO_REQ)) {
			rd->flags = flags;
		}
		return AH_TRUE;
	}
}

static HAL_BOOL
IS_BIT_SET(int bit, u_int64_t *bitmask)
{
	int byteOffset, bitnum;
	u_int64_t val;

	byteOffset = bit/64;
	bitnum = bit - byteOffset*64;
	val = ((u_int64_t) 1) << bitnum;
	if (bitmask[byteOffset] & val)
		return AH_TRUE;
	else
		return AH_FALSE;
}
	
/* Add given regclassid into regclassids array upto max of maxregids */
static void
ath_add_regclassid(u_int8_t *regclassids, u_int maxregids, u_int *nregids, u_int8_t regclassid)
{
	int i;

	/* Is regclassid valid? */
	if (regclassid == 0)
		return;

	for (i=0; i < maxregids; i++) {
		if (regclassids[i] == regclassid)
			return;
		if (regclassids[i] == 0)
			break;
	}

	if (i == maxregids)
		return;
	else {
		regclassids[i] = regclassid;
		*nregids += 1;
	}

	return;
}

/*
 * Setup the channel list based on the information in the EEPROM and
 * any supplied country code.  Note that we also do a bunch of EEPROM
 * verification here and setup certain regulatory-related access
 * control data used later on.
 */

HAL_BOOL
ath_hal_init_channels(struct ath_hal *ah,
		      HAL_CHANNEL *chans, u_int maxchans, u_int *nchans,
		      u_int8_t *regclassids, u_int maxregids, u_int *nregids,
		      HAL_CTRY_CODE cc, u_int modeSelect,
		      HAL_BOOL enableOutdoor, HAL_BOOL enableExtendedChannels)
{
#define CHANNEL_HALF_BW		10
#define CHANNEL_QUARTER_BW	5
#define CHANNEL_SUBQUARTER_BW	2
	u_int modesAvail;
	u_int16_t maxChan=7000;
	u_int16_t minChan=0;
	COUNTRY_CODE_TO_ENUM_RD *country=AH_NULL;
	COUNTRY_CODE_TO_ENUM_RD *orig_country=AH_NULL;
	REG_DOMAIN rd5GHz, rd2GHz;
	REG_DOMAIN orig_rd5GHz, orig_rd2GHz;
	const struct cmode *cm;
	HAL_CHANNEL_INTERNAL *ichans=AH_PRIVATE(ah)->ah_memchannels;
	int next=0,b;
	u_int8_t ctl;
	int is_quarterchan_cap, is_halfchan_cap; 
	int cbw;
#ifdef AH_CARS
	cc = CTRY_CARS;
#endif
	HALDEBUG(ah, "%s: cc %u mode 0x%x%s%s\n", __func__,
		 cc, modeSelect, enableOutdoor? " Enable outdoor" : " ",
		 enableExtendedChannels ? " Enable ecm" : "");

	/*
	 * Validate the EEPROM setting and setup defaults
	 */
	if (!isEepromValid(ah)) {
		/*
		 * Don't return any channels if the EEPROM has an
		 * invalid regulatory domain/country code setting.
		 */
		HALDEBUG(ah, "%s: invalid EEPROM contents\n",__func__);
		return AH_FALSE;
	}
	AH_PRIVATE(ah)->ah_countryCode = getDefaultCountry(ah);

#ifndef AH_SUPPORT_11D
	if (AH_PRIVATE(ah)->ah_countryCode == CTRY_DEFAULT) {
#endif
		/* 
		 * We now have enough state to validate any country code
		 * passed in by the caller.  XXX - TODO: Switch based on Japan
		 * country code to new reg domain if valid japan card
		 */
		if (!isCountryCodeValid(ah, cc)) {
			/* NB: Atheros silently ignores invalid country codes */
			HALDEBUG(ah, "%s: invalid country code %d\n",
				 __func__, cc);
			return AH_FALSE;
		}
		AH_PRIVATE(ah)->ah_countryCode = cc & COUNTRY_CODE_MASK;
#ifndef AH_SUPPORT_11D
	}
#endif

	/* 
  	 * Update old SKU numbers based on EEPROM settings to
 	 * newer SKU if required. The update is done _once_ only. 
	 * Multiple calls to this routine does not change SKU settings in 
	 * any way.
	 *
	 * NOTE: Currently only JAPAN SKUs are checked and updated if required.
 	 */
	ath_hal_update_regdomain(ah);

	/* Get pointers to the country element and the reg domain elements */
	country = findCountry(AH_PRIVATE(ah)->ah_countryCode);
#ifdef AH_SUPPORT_SC
	if (AH_PRIVATE(ah)->ah_useSC) {
		orig_country = country;
		country = findCountry(0);
		enableOutdoor = 0;
	}
#endif

	if (country == AH_NULL) {
		HALDEBUG(ah, "Country is NULL!!!!, cc= %d\n",
			AH_PRIVATE(ah)->ah_countryCode);
		return AH_FALSE;
	}

	if (!getWmRD(ah, country, ~CHANNEL_2GHZ, &rd5GHz, AH_TRUE)) {
		HALDEBUG(ah,"%s: couldn't find unitary 5GHz reg domain for country %u\n",
			 __func__, AH_PRIVATE(ah)->ah_countryCode);
		return AH_FALSE;
	}
	if (!getWmRD(ah, country, CHANNEL_2GHZ, &rd2GHz, AH_TRUE)) {
		HALDEBUG(ah,"%s: couldn't find unitary 2GHz reg domain for country %u\n",
			 __func__, AH_PRIVATE(ah)->ah_countryCode);
		return AH_FALSE;
	}
	if (orig_country) {
		if (!getWmRD(ah, orig_country, ~CHANNEL_2GHZ, &orig_rd5GHz, AH_FALSE)) {
			HALDEBUG(ah,"%s: couldn't find unitary 5GHz reg domain for country %u\n",
				 __func__, AH_PRIVATE(ah)->ah_countryCode);
			return AH_FALSE;
		}
		if (!getWmRD(ah, orig_country, CHANNEL_2GHZ, &orig_rd2GHz, AH_FALSE)) {
			HALDEBUG(ah,"%s: couldn't find unitary 2GHz reg domain for country %u\n",
				 __func__, AH_PRIVATE(ah)->ah_countryCode);
			return AH_FALSE;
		}
	}

	modesAvail = ath_hal_getwmodesnreg(ah, country, &rd5GHz);

	if (cc == NULL1_WORLD) {
            modeSelect &= ~(HAL_MODE_11A|HAL_MODE_TURBO|HAL_MODE_108A|HAL_MODE_11A_HALF_RATE|HAL_MODE_11A_QUARTER_RATE
							|HAL_MODE_11NA_HT20|HAL_MODE_11NA_HT40PLUS|HAL_MODE_11NA_HT40MINUS);
	}

	if (country->outdoorChanStart==0xffff) {
	    maxChan = 7000;
	    minChan = 0;
	} 
	else if (!enableOutdoor)
		maxChan = country->outdoorChanStart;
	else if (country->outdoorChanStart < 7000)
		minChan = country->outdoorChanStart;
	next = 0;

	if (maxchans > HAL_CHANNELS)
		maxchans = HAL_CHANNELS;

	is_halfchan_cap = AH_PRIVATE(ah)->ah_caps.halChanHalfRate;
	is_quarterchan_cap = AH_PRIVATE(ah)->ah_caps.halChanQuarterRate;
	for (cm = modes; cm < &modes[N(modes)]; cm++) {
		u_int16_t c, c_hi, c_lo;
		u_int64_t *channelBM=AH_NULL;
		u_int64_t *orig_channelBM=AH_NULL;
		REG_DOMAIN *rd=AH_NULL;
		REG_DOMAIN *orig_rd=AH_NULL;
		REG_DMN_FREQ_BAND *fband=AH_NULL,*freqs;//, *orig_fband = AH_NULL;
		int low_adj, hi_adj, channelSep, lastc;
		cbw = 0;

		if ((cm->mode & modeSelect) == 0) {
			HALDEBUG(ah, "%s: skip mode 0x%x flags 0x%x\n",
				 __func__, cm->mode, cm->flags);
			continue;
		}
		if ((cm->mode & modesAvail) == 0) {
			HALDEBUG(ah, "%s: !avail mode 0x%x (0x%x) flags 0x%x\n",
				 __func__, modesAvail, cm->mode, cm->flags);
			continue;
		}
		if (!ath_hal_getChannelEdges(ah, cm->flags, &c_lo, &c_hi)) {
			/* channel not supported by hardware, skip it */
			HALDEBUG(ah, "%s: channels 0x%x not supported by hardware\n",
				 __func__,cm->flags);
			continue;
		}
		switch (cm->mode) {
		case HAL_MODE_TURBO:
			rd = &rd5GHz;
			channelBM = rd->chan11a_turbo;
			if (orig_country) {
				orig_rd = &orig_rd5GHz;
				orig_channelBM = orig_rd->chan11a_turbo;
			}
			freqs = &regDmn5GhzTurboFreq[0];
			ctl = rd->conformanceTestLimit | CTL_TURBO;
			if (AH_PRIVATE(ah)->ah_chanbw && (AH_PRIVATE(ah)->ah_chanbw != 40)) 
			    continue;
			break;
		case HAL_MODE_11A:
			cbw = AH_PRIVATE(ah)->ah_chanbw;
			/* fall through */
		case HAL_MODE_11NA_HT20:
		case HAL_MODE_11NA_HT40PLUS:
		case HAL_MODE_11NA_HT40MINUS:
			rd = &rd5GHz;
			channelBM = rd->chan11a;
			if (orig_country) {
				orig_rd = &orig_rd5GHz;
				orig_channelBM = orig_rd->chan11a;
			}
			freqs = &regDmn5GhzFreq[0];
			ctl = rd->conformanceTestLimit;
			if ((AH_PRIVATE(ah)->ah_chanbw == 40)) 
			    continue;
			break;
		case HAL_MODE_11B:
			rd = &rd2GHz;
			channelBM = rd->chan11b;
			if (orig_country) {
				orig_rd = &orig_rd2GHz;
				orig_channelBM = orig_rd->chan11b;
			}
			freqs = &regDmn2GhzFreq[0];
			ctl = rd->conformanceTestLimit | CTL_11B;
			if ((AH_PRIVATE(ah)->ah_chanbw == 40)) 
			    continue;
			cbw = AH_PRIVATE(ah)->ah_chanbw;
			break;
		case HAL_MODE_11G:
#ifdef AH_SUPPORT_TURBOG
			if ((AH_PRIVATE(ah)->ah_chanbw == 40)) 
			    continue;
#endif
			cbw = AH_PRIVATE(ah)->ah_chanbw;
			/* fall through */
		case HAL_MODE_11NG_HT20:
		case HAL_MODE_11NG_HT40PLUS:
		case HAL_MODE_11NG_HT40MINUS:
			rd = &rd2GHz;
			channelBM = rd->chan11g;
			if (orig_country) {
				orig_rd = &orig_rd2GHz;
				orig_channelBM = orig_rd->chan11g;
			}
			freqs = &regDmn2Ghz11gFreq[0];
			ctl = rd->conformanceTestLimit | CTL_11G;
			if ((AH_PRIVATE(ah)->ah_chanbw == 40)) 
			    continue;
			break;
		case HAL_MODE_11G_TURBO:
			rd = &rd2GHz;
			channelBM = rd->chan11g_turbo;
			if (orig_country) {
				orig_rd = &orig_rd2GHz;
				orig_channelBM = orig_rd->chan11g_turbo;
			}
			freqs = &regDmn2Ghz11gTurboFreq[0];
			ctl = rd->conformanceTestLimit | CTL_108G;
#ifdef AH_SUPPORT_TURBOG
			if (AH_PRIVATE(ah)->ah_chanbw && (AH_PRIVATE(ah)->ah_chanbw != 40))
#endif
			    continue;
			break;
		case HAL_MODE_11A_TURBO:
			rd = &rd5GHz;
			channelBM = rd->chan11a_dyn_turbo;
			if (orig_country) {
				orig_rd = &orig_rd5GHz;
				orig_channelBM = orig_rd->chan11a_dyn_turbo;
			}
			freqs = &regDmn5GhzTurboFreq[0];
			ctl = rd->conformanceTestLimit | CTL_108G;
			if (AH_PRIVATE(ah)->ah_chanbw && (AH_PRIVATE(ah)->ah_chanbw != 40)) 
			    continue;
			break;
		default:
			HALDEBUG(ah, "%s: Unkonwn HAL mode 0x%x\n",
				__func__, cm->mode);
			continue;
		}
		if (isChanBitMaskZero(channelBM))
			continue;

		/*
		 * Setup special handling for HT40 channels; e.g.
		 * 5G HT40 channels require 40Mhz channel separation.
		 */
		hi_adj = (cm->mode == HAL_MODE_11NA_HT40PLUS ||
		    cm->mode == HAL_MODE_11NG_HT40PLUS) ? -20 : 0;
		low_adj = (cm->mode == HAL_MODE_11NA_HT40MINUS || 
		    cm->mode == HAL_MODE_11NG_HT40MINUS) ? 20 : 0;
		channelSep = (cm->mode == HAL_MODE_11NA_HT40PLUS ||
		    cm->mode == HAL_MODE_11NA_HT40MINUS) ? 40 : 0;

		for (b=0;b<64*BMLEN; b++) {
			if (IS_BIT_SET(b,channelBM)) {
				fband = &freqs[b];
				lastc = 0;

				ath_add_regclassid(regclassids, maxregids, 
						nregids, fband->regClassId);
				int channelsep = fband->channelSep;
				if (channelsep == 20 && cbw)
				{
					switch (cbw) {
						case CHANNEL_HALF_BW:
							channelsep = 10;
							break;
						case CHANNEL_QUARTER_BW:
							channelsep = 5;
							break;
						case CHANNEL_SUBQUARTER_BW:
							channelsep = 5;
							break;
					}
				}
				for (c = fband->lowChannel + low_adj;
				     c <= fband->highChannel + hi_adj;
				     c += channelsep) {
					HAL_CHANNEL_INTERNAL icv;
					HAL_BOOL scan = AH_TRUE;

					if (orig_country) {
						int b2;

						/* scan the orig. regdomain for matching channel range */
						scan = AH_FALSE;
						for (b2=0; b2 < 64*BMLEN; b2++) {
							if (!IS_BIT_SET(b2, orig_channelBM))
								continue;
							if ((c >= freqs[b2].lowChannel + low_adj) &&
								(c <= freqs[b2].highChannel + hi_adj)) {
								scan = AH_TRUE;
								break;
							}
						}
						if (c >= (enableOutdoor ? 7000 : orig_country->outdoorChanStart))
							scan = AH_FALSE;
					}
					if (!(c_lo <= c && c <= c_hi)) {
						HALDEBUG(ah, "%s: c %u out of range [%u..%u]\n",
							 __func__, c, c_lo, c_hi);
						continue;
					}
					if ((fband->channelBW == 
							CHANNEL_HALF_BW) && 
						!is_halfchan_cap) {
						HALDEBUG(ah, "%s: Skipping %u half rate channel\n", 
								__func__, c); 
						continue;
					}

					if ((fband->channelBW == 
						CHANNEL_QUARTER_BW) && 
						!is_quarterchan_cap) {
						HALDEBUG(ah, "%s: Skipping %u quarter rate channel\n", 
								__func__, c); 
						continue;
					}

#if 0
					if (((c+fband->channelSep)/2) > (maxChan+HALF_MAXCHANBW)) {
						HALDEBUG(ah, "%s: c %u > maxChan %u\n",
							 __func__, c, maxChan);
						continue;
					}
#endif
					if (c >= maxChan) {
						HALDEBUG(ah, "%s: c %u > maxChan %u\n",
							 __func__, c, maxChan);
						continue;
					}

					if (cm->mode==HAL_MODE_11A || cm->mode==HAL_MODE_11A_TURBO || cm->mode==HAL_MODE_TURBO)
					{
						if (c < minChan) {
							HALDEBUG(ah, "%s: c %u > minChan %u\n",
								 __func__, c, maxChan);
							continue;
						}
						if (orig_country && (c < (enableOutdoor ? orig_country->outdoorChanStart : 0)))
							scan = AH_FALSE;
					}
					if (next >= maxchans){
						HALDEBUG(ah, "%s: too many channels for channel table\n",
							 __func__);
						goto done;
					}
					if ((fband->usePassScan & IS_ECM_CHAN) &&
					    !enableExtendedChannels) {
						HALDEBUG(ah, "Skipping ecm channel\n");
						continue;
					}
					if ((rd->flags & NO_HOSTAP) && 
					    (AH_PRIVATE(ah)->ah_opmode == HAL_M_HOSTAP)) {
						HALDEBUG(ah, "Skipping HOSTAP channel\n");
						continue;
					}
					/*
					 * Make sure that channel separation
					 * meets the requirement.
					 */
					if (lastc && channelSep &&
					    (c-lastc) < channelSep)
						continue;

					OS_MEMZERO(&icv, sizeof(icv));
					icv.channel = c;
#ifdef AH_SUPPORT_SC
					icv.channel += AH_PRIVATE(ah)->ah_chanshift;
#endif
					icv.channelFlags = cm->flags;

					switch ((cbw ? cbw : fband->channelBW)) {
						case CHANNEL_HALF_BW:
							icv.channelFlags |= CHANNEL_HALF;
							break;
						case CHANNEL_QUARTER_BW:
							icv.channelFlags |= CHANNEL_QUARTER;
							break;
						case CHANNEL_SUBQUARTER_BW:
							icv.channelFlags |= CHANNEL_SUBQUARTER;
							break;
					}
#ifdef AH_NOPOWERLIMIT
					icv.maxRegTxPower = 40;
#else
					icv.maxRegTxPower = ((int)fband->powerDfs)-((int)AH_PRIVATE(ah)->ah_antGain);
					icv.maxRegTxPower += ((int)AH_PRIVATE(ah)->ah_antGainsub);
#endif
					icv.antennaMax = 0;
					icv.regDmnFlags = rd->flags;
					icv.conformanceTestLimit = ctl;
					if (fband->usePassScan & rd->pscan)
						icv.channelFlags |= CHANNEL_PASSIVE;
					else
						icv.channelFlags &= ~CHANNEL_PASSIVE;
					lastc = c;
					if (fband->useDfs & rd->dfsMask) {
						/* DFS and HT40 don't mix */
						if (cm->mode == HAL_MODE_11NA_HT40PLUS ||
						    cm->mode == HAL_MODE_11NA_HT40MINUS)
							continue;
						icv.privFlags = CHANNEL_DFS;
					} else
						icv.privFlags = 0;
					if (rd->flags & LIMIT_FRAME_4MS)
						icv.privFlags |= CHANNEL_4MS_LIMIT;
					if (!scan)
						icv.privFlags |= CHANNEL_NOSCAN;
					else
						icv.privFlags &= ~CHANNEL_NOSCAN;
					OS_MEMCPY(&ichans[next++],&icv,sizeof(HAL_CHANNEL_INTERNAL));
				}
			}
		}
	}
done:	if (next != 0) {
		int i;

		/* XXX maxchans set above so this cannot happen? */
		if (next > HAL_CHANNELS) {
			HALDEBUG(ah, "%s: too many channels %u; truncating to %u\n",
				 __func__, next,
				 (int) HAL_CHANNELS);
			next = HAL_CHANNELS;
		}

		/*
		 * Keep a private copy of the channel list so we can
		 * constrain future requests to only these channels
		 */
		ath_hal_sort(ichans, next, sizeof(HAL_CHANNEL_INTERNAL), chansort);
		AH_PRIVATE(ah)->ah_nchan = next;

		/*
		 * Copy the channel list to the public channel list
		 */
		for (i=0; i<next; i++) {
			chans[i].channel = ichans[i].channel;
			chans[i].channelFlags = ichans[i].channelFlags;
			chans[i].privFlags = ichans[i].privFlags;
			chans[i].maxRegTxPower = ichans[i].maxRegTxPower;
		}
		/*
		 * Retrieve power limits.
		 */
		ath_hal_getpowerlimits(ah, chans, next);
		for (i=0; i<next; i++) {
			ichans[i].maxTxPower = chans[i].maxTxPower;
			ichans[i].minTxPower = chans[i].minTxPower;
		}
	}
	*nchans = next;
	/* XXX copy private setting to public area */
	ah->ah_countryCode = AH_PRIVATE(ah)->ah_countryCode;
	return (next != 0);
#undef CHANNEL_HALF_BW
#undef CHANNEL_QUARTER_BW
#undef CHANNEL_SUBQUARTER_BW
}

/*
 * Return whether or not the specified channel is ok to use
 * based on the current regulatory domain constraints and 
 * DFS interference.
 */
HAL_CHANNEL_INTERNAL *
ath_hal_checkchannel(struct ath_hal *ah, const HAL_CHANNEL *c)
{
#define CHAN_FLAGS	(CHANNEL_ALL|CHANNEL_HALF|CHANNEL_QUARTER|CHANNEL_SUBQUARTER)
	HAL_CHANNEL_INTERNAL *base, *cc;
	/* NB: be wary of user-specified channel flags */
	int flags = c->channelFlags & CHAN_FLAGS;
	int n, lim, d;

	HALDEBUGn(ah, 9, "%s: channel %u/0x%x (0x%x) requested\n",
		__func__, c->channel, c->channelFlags, flags);

	/*
	 * Check current channel to avoid the lookup.
	 */
	cc = AH_PRIVATE(ah)->ah_curchan;
	if (cc != AH_NULL && cc->channel == c->channel &&
	    (cc->channelFlags & CHAN_FLAGS) == flags) {
		if ((cc->privFlags & CHANNEL_INTERFERENCE) &&
		    (cc->channelFlags & CHANNEL_DFS))
			return AH_NULL;
		else
			return cc;
	}

	/* binary search based on known sorting order */
	base = AH_PRIVATE(ah)->ah_memchannels;
	n = AH_PRIVATE(ah)->ah_nchan;
	/* binary search based on known sorting order */
	for (lim = n; lim != 0; lim >>= 1) {
		cc = &base[lim>>1];
		d = c->channel - cc->channel;
		if (d == 0) {
			if ((cc->channelFlags & CHAN_FLAGS) == flags) {
				if ((cc->privFlags & CHANNEL_INTERFERENCE) &&
				    (cc->channelFlags & CHANNEL_DFS))
					return AH_NULL;
				else
					return cc;
			}
			d = flags - (cc->channelFlags & CHAN_FLAGS);
		}
		HALDEBUGn(ah, 9, "%s: channel %u/0x%x d %d\n", __func__,
			cc->channel, cc->channelFlags, d);
		if (d > 0) {
			base = cc + 1;
			lim--;
		}
	}
	HALDEBUG(ah, "%s: no match for %u/0x%x\n",
		__func__, c->channel, c->channelFlags);
	return AH_NULL;
#undef CHAN_FLAGS
}

/*
 * Return the max allowed antenna gain and apply any regulatory
 * domain specific changes.
 *
 * NOTE: a negative reduction is possible in RD's that only
 * measure radiated power (e.g., ETSI) which would increase
 * that actual conducted output power (though never beyond
 * the calibrated target power).
 */
u_int
ath_hal_getantennareduction(struct ath_hal *ah, HAL_CHANNEL *chan, u_int twiceGain)
{
return 0;
/*	HAL_CHANNEL_INTERNAL *ichan=AH_NULL;
	int8_t antennaMax;

	if ((ichan = ath_hal_checkchannel(ah, chan)) != AH_NULL) {
		antennaMax = twiceGain - ichan->antennaMax*2;
		return (antennaMax < 0) ? 0 : antennaMax;
	} else {
		return 0;
	}*/
}


/* XXX - KCYU - fix this code ... maybe move ctl decision into channel set area or
 into the tables so no decision is needed in the code */

#define isWwrSKU(_ah) (((getEepromRD((_ah), AH_TRUE) & WORLD_SKU_MASK) == WORLD_SKU_PREFIX) || \
		       (getEepromRD(_ah, AH_TRUE) == WORLD))


/*
 * Return the test group from the specified channel from
 * the regulatory table.
 *
 * TODO: CTL for 11B CommonMode when regulatory domain is unknown
 */
u_int
ath_hal_getctl(struct ath_hal *ah, HAL_CHANNEL *chan)
{
	u_int ctl=NO_CTL;
	HAL_CHANNEL_INTERNAL *ichan;

	/* Special CTL to signify WWR SKU without a known country */
	if (AH_PRIVATE(ah)->ah_countryCode == CTRY_DEFAULT && isWwrSKU(ah)) {
		if (IS_CHAN_B(chan)) {
			ctl = SD_NO_CTL | CTL_11B;
		} else if (IS_CHAN_G(chan)) {
			ctl = SD_NO_CTL | CTL_11G;
		} else if (IS_CHAN_108G(chan)) {
			ctl = SD_NO_CTL | CTL_108G;
		} else if (IS_CHAN_T(chan)) {
			ctl = SD_NO_CTL | CTL_TURBO;
		} else {
			ctl = SD_NO_CTL | CTL_11A;
		}
	} else {
		if ((ichan = ath_hal_checkchannel(ah, chan)) != AH_NULL) {
			ctl = ichan->conformanceTestLimit;
#if 0
			/* Atheros change# 73449: limit 11G OFDM power */
			if (IS_CHAN_PUREG(chan) && (ctl & 0xf) == CTL_11B)
				ctl = (ctl &~ 0xf) | CTL_11G;
#endif
		}
	}
	return ctl;
}

/*
 * Return whether or not a noise floor check is required in
 * the current regulatory domain for the specified channel.
 */

HAL_BOOL
ath_hal_getnfcheckrequired(struct ath_hal *ah, HAL_CHANNEL *chan)
{
	HAL_CHANNEL_INTERNAL *ichan;

	if ((ichan = ath_hal_checkchannel(ah, chan)) != AH_NULL) {
		return ((ichan->regDmnFlags & NEED_NFC) ? AH_TRUE : AH_FALSE);
	}
	return AH_FALSE;
}


/*
 * Insertion sort.
 */
#define swap(_a, _b, _size) {			\
	u_int8_t *s = _b;			\
	int i = _size;				\
	do {					\
		u_int8_t tmp = *_a;		\
		*_a++ = *s;			\
		*s++ = tmp;			\
	} while (--i);				\
	_a -= _size;				\
}

static void
ath_hal_sort(void *a, size_t n, size_t size, ath_hal_cmp_t *cmp)
{
	u_int8_t *aa = a;
	u_int8_t *ai, *t;

	for (ai = aa+size; --n >= 1; ai += size)
		for (t = ai; t > aa; t -= size) {
			u_int8_t *u = t - size;
			if (cmp(u, t) <= 0)
				break;
			swap(u, t, size);
		}
}





/*
 ******************************************************************************
 * NOTICE ! NOTICE ! NOTICE !
 * Following code base converts old SKUs to newer SKUs based 
 * on EEPROM settings. Currently this is applicable for JAPAN
 * SKUs only.
 ******************************************************************************
 */

static int
ath_hal_japan_checkeeprom(struct ath_hal *ah)
{
	u_int16_t regcap, eever;
	struct ath_hal_private *ahpriv = AH_PRIVATE(ah);

	if (!ath_hal_eepromRead(ah, AR_EEPROM_VERSION, &eever)) {
		HALDEBUG(ah, "%s: unable to read EEPROM version\n", __func__);
		return 0;
	}

	if (!ath_hal_eepromRead(ah, (eever >= AR_EEPROM_VER4_0)? 
				AR_EEPROM_REG_CAPABILITIES_OFFSET:
				AR_EEPROM_REG_CAPABILITIES_OFFSET_PRE4_0, 
				&regcap)) {
		HALDEBUG(ah, "%s: unable to read EEPROM regcaps\n", __func__);
		return 0;
	}

	if (eever >= AR_EEPROM_VER5_3) {
		ahpriv->ah_regdomainUpdate = (regcap &
			(AR_EEPROM_EEREGCAP_EN_KK_U1_EVEN |
			 AR_EEPROM_EEREGCAP_EN_KK_U2)) ?
				AH_TRUE : AH_FALSE;
	} else if (eever >= AR_EEPROM_VER4_0) {
		ahpriv->ah_regdomainUpdate =
			(regcap & AR_EEPROM_EEREGCAP_EN_KK_NEW_11A) ?
							AH_TRUE:AH_FALSE;
	} else {
		ahpriv->ah_regdomainUpdate =
		 (regcap & AR_EEPROM_EEREGCAP_EN_KK_NEW_11A_PRE4_0) ?
							AH_TRUE:AH_FALSE;
	}

	return ((int)ahpriv->ah_regdomainUpdate);
}

/*
 * Check if 5GHZ operation is valid for any Japan regdomains.
 * As a result of UNI1 even and UNI2 band addition, check
 * Amode validity bit at new place in eeprom for all Japan regdomains.
 */
static int
ath_hal_japanAmode_valid(struct ath_hal *ah)
{
	u_int16_t rd = getEepromRD(ah, AH_TRUE);

	switch (rd) {
	case MKK1_MKKA:
	case MKK1_MKKB:
	case MKK2_MKKA:
	case MKK1_FCCA:
	case MKK1_MKKA1:
	case MKK1_MKKA2:
	case MKK1_MKKC:
	case (CTRY_JAPAN | COUNTRY_ERD_FLAG):
	case (CTRY_JAPAN1 | COUNTRY_ERD_FLAG):
	case (CTRY_JAPAN2 | COUNTRY_ERD_FLAG):
	case (CTRY_JAPAN3 | COUNTRY_ERD_FLAG):
	case (CTRY_JAPAN4 | COUNTRY_ERD_FLAG):
	case (CTRY_JAPAN5 | COUNTRY_ERD_FLAG):
	case (CTRY_JAPAN6 | COUNTRY_ERD_FLAG):
			return ath_hal_japan_checkeeprom(ah);

	case NO_ENUMRD:
		switch (AH_PRIVATE(ah)->ah_countryCode) {
			case (CTRY_JAPAN):
			case (CTRY_JAPAN1):
			case (CTRY_JAPAN2):
			case (CTRY_JAPAN3):
			case (CTRY_JAPAN4):
			case (CTRY_JAPAN5):
			case (CTRY_JAPAN6):
				return ath_hal_japan_checkeeprom(ah);
			default:
				break;
		}
	default:
		break;
	}

	return 0;
}

/* mapping of old skus to new skus for Japan */
typedef struct {
	HAL_REG_DOMAIN	domain;
	HAL_REG_DOMAIN	newdomain_pre53;	/* pre eeprom version 5.3 */
	HAL_REG_DOMAIN	newdomain_post53;	/* post eeprom version 5.3 */
} JAPAN_SKUMAP;

/* mapping of countrycode to new skus for Japan */
typedef struct {
	HAL_CTRY_CODE	ccode;
	HAL_REG_DOMAIN	newdomain_pre53;	/* pre eeprom version 5.3 */
	HAL_REG_DOMAIN	newdomain_post53;	/* post eeprom version 5.3 */
} JAPAN_COUNTRYMAP;

static JAPAN_SKUMAP j_skumap[] = {
	{ MKK1_MKKA, MKK3_MKKA, MKK4_MKKA },
	{ CTRY_JAPAN | COUNTRY_ERD_FLAG, MKK3_MKKA, MKK4_MKKA },

	{ MKK1_MKKB, MKK3_MKKB, MKK4_MKKB },
	{ CTRY_JAPAN1 | COUNTRY_ERD_FLAG, MKK3_MKKB, MKK4_MKKB },

	{ MKK1_FCCA, MKK3_FCCA, MKK4_FCCA },
	{ CTRY_JAPAN2 | COUNTRY_ERD_FLAG, MKK3_FCCA, MKK4_FCCA },

	{ MKK2_MKKA, MKK9_MKKA, MKK10_MKKA },
	{ CTRY_JAPAN3 | COUNTRY_ERD_FLAG, MKK9_MKKA, MKK10_MKKA },

	{ MKK1_MKKA1, MKK3_MKKA1, MKK4_MKKA1 },
	{ CTRY_JAPAN4 | COUNTRY_ERD_FLAG, MKK3_MKKA1, MKK4_MKKA1 },

	{ MKK1_MKKA2, MKK3_MKKA2, MKK4_MKKA2 },
	{ CTRY_JAPAN5 | COUNTRY_ERD_FLAG, MKK3_MKKA2, MKK4_MKKA2 },

	{ MKK1_MKKC, MKK3_MKKC, MKK4_MKKC },
	{ CTRY_JAPAN6 | COUNTRY_ERD_FLAG, MKK3_MKKC, MKK4_MKKC }
};

static JAPAN_COUNTRYMAP j_countrymap[] = {
	{CTRY_JAPAN,    MKK3_MKKA,      MKK4_MKKA },
	{CTRY_JAPAN1,	MKK3_MKKB,	MKK4_MKKB },
	{CTRY_JAPAN2,   MKK3_FCCA,      MKK4_FCCA },
	{CTRY_JAPAN3,   MKK9_MKKA,      MKK10_MKKA },
	{CTRY_JAPAN4,   MKK3_MKKA1,     MKK4_MKKA1 },
	{CTRY_JAPAN5,	MKK3_MKKA2,	MKK4_MKKA2 },
	{CTRY_JAPAN6,	MKK3_MKKC,	MKK4_MKKC }
};

static void
ath_hal_mapjapansku(struct ath_hal *ah, HAL_REG_DOMAIN oldregdomain)
{
	int i;
	u_int16_t eever;
	struct ath_hal_private *ahpriv = AH_PRIVATE(ah);

	if (!ath_hal_eepromRead(ah, AR_EEPROM_VERSION, &eever)) {
		HALDEBUG(ah, "%s: unable to read EEPROM version\n", __func__);
		return;
	}

	for (i = 0; i < N(j_skumap); i++) {
		if (j_skumap[i].domain == oldregdomain) {
			ahpriv->ah_currentRD = (eever < AR_EEPROM_VER5_3) ?
					j_skumap[i].newdomain_pre53:
					j_skumap[i].newdomain_post53;
			ahpriv->ah_countryCode = getDefaultCountry(ah);
			HALDEBUG(ah, "%s: map rd %u -> %u cc -> %u\n",
				__func__, oldregdomain,
				ahpriv->ah_currentRD, ahpriv->ah_countryCode);
			break;
		}
	}

	return;
}

/* Map japan country code to new regdomain */
static void
ath_hal_mapjapanccode(struct ath_hal *ah, HAL_CTRY_CODE ccode)
{
	int i;
	u_int16_t eever;
	struct ath_hal_private *ahpriv = AH_PRIVATE(ah);

	if (!ath_hal_eepromRead(ah, AR_EEPROM_VERSION, &eever)) {
		HALDEBUG(ah, "%s: unable to read EEPROM version\n", __func__);
		return;
	}

	for (i = 0; i < N(j_countrymap); i++) {
		if (j_countrymap[i].ccode == ccode) {
			ahpriv->ah_currentRD = (eever < AR_EEPROM_VER5_3) ?
					j_countrymap[i].newdomain_pre53:
					j_countrymap[i].newdomain_post53;
			ahpriv->ah_countryCode = getDefaultCountry(ah);
			HALDEBUG(ah, "%s: map rd %u cc %u -> %u\n",
				__func__, ahpriv->ah_currentRD,
				ccode, ahpriv->ah_countryCode);
			break;
		}
	}

	return;
}

/* Update regdomain values for legacy JAPAN SKUs
 * Depending on Eeprom version and older JAPAN SKUs, determine new SKU
 * to be used. 
 * Update superchannel settings as well
 */
static void
ath_hal_update_regdomain(struct ath_hal *ah)
{
	struct ath_hal_private *ahpriv = AH_PRIVATE(ah);

#ifdef AH_SUPPORT_SC
	if (ahpriv->ah_useSC)
		return;
#endif

	/* Do we need to update regdomain ? */
	if (ath_hal_japanAmode_valid(ah)) {
		/* Is 11A mode supported? */
		if (ahpriv->ah_caps.halWirelessModes & HAL_MODE_11A) {
			if (ahpriv->ah_currentRD != NO_ENUMRD) {
				ath_hal_mapjapansku(ah, ahpriv->ah_currentRD);
			} else {
				ath_hal_mapjapanccode(ah,
						ahpriv->ah_countryCode);
			}
		}
	}

	return;
}
#undef N

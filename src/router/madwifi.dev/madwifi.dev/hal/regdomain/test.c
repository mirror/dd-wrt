/*
 * Copyright (c) 2002-2006 Sam Leffler, Errno Consulting
 * Copyright (c) 2002-2006 Atheros Communications, Inc.
 * All rights reserved.
 *
 * $Id: //depot/sw/branches/sam_hal/regdomain/test.c#1 $
 */
#include "opt_ah.h"

#include "ah.h"
#include "ah_internal.h"
#include "ah_eeprom.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>

#define	IEEE80211_CHAN_MAX	255

int		ath_hal_debug = 0;
HAL_CTRY_CODE	cc = CTRY_DEFAULT;
HAL_BOOL	outdoor = AH_TRUE;
HAL_BOOL	Amode = 1;
HAL_BOOL	Bmode = 1;
HAL_BOOL	Gmode = 1;
HAL_BOOL	turbo5Disable = AH_FALSE;

u_int16_t	_numCtls = 8;
u_int16_t	_ctl[32] =
	{ 0x10, 0x13, 0x40, 0x30, 0x11, 0x31, 0x12, 0x32 };
RD_EDGES_POWER	_rdEdgesPower[NUM_EDGES*NUM_CTLS] = {
	{ 5180, 28, 0 },	/* 0x10 */
	{ 5240, 60, 0 },
	{ 5260, 36, 0 },
	{ 5320, 27, 0 },
	{ 5745, 36, 0 },
	{ 5765, 36, 0 },
	{ 5805, 36, 0 },
	{ 5825, 36, 0 },

	{ 5210, 28, 0 },	/* 0x13 */
	{ 5250, 28, 0 },
	{ 5290, 30, 0 },
	{ 5760, 36, 0 },
	{ 5800, 36, 0 },
	{ 0, 0, 0 },
	{ 0, 0, 0 },
	{ 0, 0, 0 },

	{ 5170, 60, 0 },	/* 0x40 */
	{ 5230, 60, 0 },
	{ 0, 0, 0 },
	{ 0, 0, 0 },
	{ 0, 0, 0 },
	{ 0, 0, 0 },
	{ 0, 0, 0 },
	{ 0, 0, 0 },

	{ 5180, 33, 0 },	/* 0x30 */
	{ 5320, 33, 0 },
	{ 5500, 34, 0 },
	{ 5700, 34, 0 },
	{ 5745, 35, 0 },
	{ 5765, 35, 0 },
	{ 5785, 35, 0 },
	{ 5825, 35, 0 },

	{ 2412, 36, 0 },	/* 0x11 */
	{ 2417, 36, 0 },
	{ 2422, 36, 0 },
	{ 2432, 36, 0 },
	{ 2442, 36, 0 },
	{ 2457, 36, 0 },
	{ 2467, 36, 0 },
	{ 2472, 36, 0 },

	{ 2412, 36, 0 },	/* 0x31 */
	{ 2417, 36, 0 },
	{ 2422, 36, 0 },
	{ 2432, 36, 0 },
	{ 2442, 36, 0 },
	{ 2457, 36, 0 },
	{ 2467, 36, 0 },
	{ 2472, 36, 0 },

	{ 2412, 36, 0 },	/* 0x12 */
	{ 2417, 36, 0 },
	{ 2422, 36, 0 },
	{ 2432, 36, 0 },
	{ 2442, 36, 0 },
	{ 2457, 36, 0 },
	{ 2467, 36, 0 },
	{ 2472, 36, 0 },

	{ 2412, 28, 0 },	/* 0x32 */
	{ 2417, 28, 0 },
	{ 2422, 28, 0 },
	{ 2432, 28, 0 },
	{ 2442, 28, 0 },
	{ 2457, 28, 0 },
	{ 2467, 28, 0 },
	{ 2472, 28, 0 },
};

u_int16_t	turbo2WMaxPower5 = 32;
u_int16_t	turbo2WMaxPower2;
int8_t		antennaGainMax[2] = { 0, 0 };	/* XXX */
int		eeversion = AR_EEPROM_VER3_1;
TRGT_POWER_ALL_MODES tpow = {
	8, {
	    { 22, 24, 28, 32, 5180 },
	    { 22, 24, 28, 32, 5200 },
	    { 22, 24, 28, 32, 5320 },
	    { 26, 30, 34, 34, 5500 },
	    { 26, 30, 34, 34, 5700 },
	    { 20, 30, 34, 36, 5745 },
	    { 20, 30, 34, 36, 5825 },
	    { 20, 30, 34, 36, 5850 },
	},
	2, {
	    { 23, 27, 31, 34, 2412 },
	    { 23, 27, 31, 34, 2447 },
	},
	2, {
	    { 36, 36, 36, 36, 2412 },
	    { 36, 36, 36, 36, 2484 },
	}
};
#define	numTargetPwr_11a	tpow.numTargetPwr_11a
#define	trgtPwr_11a		tpow.trgtPwr_11a
#define	numTargetPwr_11g	tpow.numTargetPwr_11g
#define	trgtPwr_11g		tpow.trgtPwr_11g
#define	numTargetPwr_11b	tpow.numTargetPwr_11b
#define	trgtPwr_11b		tpow.trgtPwr_11b

static HAL_BOOL
getChannelEdges(struct ath_hal *ah, u_int16_t flags, u_int16_t *low, u_int16_t *high)
{
	if (flags & CHANNEL_5GHZ) {
		*low = 4920;
		*high = 6100;
		return AH_TRUE;
	}
	if (flags & CHANNEL_2GHZ) {
		*low = 2312;
		*high = 2732;
		return AH_TRUE;
	}
	return AH_FALSE;
}

static u_int
getWirelessModes(struct ath_hal *ah)
{
	u_int mode = 0;

	if (Amode) {
		mode = HAL_MODE_11A;
		if (!turbo5Disable)
			mode |= HAL_MODE_TURBO;
	}
	if (Bmode)
		mode |= HAL_MODE_11B;
	if (Gmode)
		mode |= HAL_MODE_11G;
	return mode;
}

/*
 * Convert IEEE channel number to GHz frequency.
 */
u_int
ath_hal_ieee2mhz(u_int chan, u_int flags)
{
	if (flags & CHANNEL_2GHZ) {	/* 2GHz band */
		if (chan == 14)
			return 2484;
		if (chan < 14)
			return 2407 + chan*5;
		else
			return 2512 + ((chan-15)*20);
	} else if (flags & CHANNEL_5GHZ) {/* 5Ghz band */
		return 5000 + (chan*5);
	} else {			/* either, guess */
		if (chan == 14)
			return 2484;
		if (chan < 14)		/* 0-13 */
			return 2407 + chan*5;
		if (chan < 27)		/* 15-26 */
			return 2512 + ((chan-15)*20);
		return 5000 + (chan*5);
	}
}

/*
 * Country/Region Codes from MS WINNLS.H
 * Numbering from ISO 3166
 */
enum CountryCode {
    CTRY_ALBANIA              = 8,       /* Albania */
    CTRY_ALGERIA              = 12,      /* Algeria */
    CTRY_ARGENTINA            = 32,      /* Argentina */
    CTRY_ARMENIA              = 51,      /* Armenia */
    CTRY_AUSTRALIA            = 36,      /* Australia */
    CTRY_AUSTRIA              = 40,      /* Austria */
    CTRY_AZERBAIJAN           = 31,      /* Azerbaijan */
    CTRY_BAHRAIN              = 48,      /* Bahrain */
    CTRY_BELARUS              = 112,     /* Belarus */
    CTRY_BELGIUM              = 56,      /* Belgium */
    CTRY_BELIZE               = 84,      /* Belize */
    CTRY_BOLIVIA              = 68,      /* Bolivia */
    CTRY_BRAZIL               = 76,      /* Brazil */
    CTRY_BRUNEI_DARUSSALAM    = 96,      /* Brunei Darussalam */
    CTRY_BULGARIA             = 100,     /* Bulgaria */
    CTRY_CANADA               = 124,     /* Canada */
    CTRY_CHILE                = 152,     /* Chile */
    CTRY_CHINA                = 156,     /* People's Republic of China */
    CTRY_COLOMBIA             = 170,     /* Colombia */
    CTRY_COSTA_RICA           = 188,     /* Costa Rica */
    CTRY_CROATIA              = 191,     /* Croatia */
    CTRY_CYPRUS               = 196,
    CTRY_CZECH                = 203,     /* Czech Republic */
    CTRY_DENMARK              = 208,     /* Denmark */
    CTRY_DOMINICAN_REPUBLIC   = 214,     /* Dominican Republic */
    CTRY_ECUADOR              = 218,     /* Ecuador */
    CTRY_EGYPT                = 818,     /* Egypt */
    CTRY_EL_SALVADOR          = 222,     /* El Salvador */
    CTRY_ESTONIA              = 233,     /* Estonia */
    CTRY_FAEROE_ISLANDS       = 234,     /* Faeroe Islands */
    CTRY_FINLAND              = 246,     /* Finland */
    CTRY_FRANCE               = 250,     /* France */
    CTRY_FRANCE2              = 255,     /* France2 */
    CTRY_GEORGIA              = 268,     /* Georgia */
    CTRY_GERMANY              = 276,     /* Germany */
    CTRY_GREECE               = 300,     /* Greece */
    CTRY_GUATEMALA            = 320,     /* Guatemala */
    CTRY_HONDURAS             = 340,     /* Honduras */
    CTRY_HONG_KONG            = 344,     /* Hong Kong S.A.R., P.R.C. */
    CTRY_HUNGARY              = 348,     /* Hungary */
    CTRY_ICELAND              = 352,     /* Iceland */
    CTRY_INDIA                = 356,     /* India */
    CTRY_INDONESIA            = 360,     /* Indonesia */
    CTRY_IRAN                 = 364,     /* Iran */
    CTRY_IRAQ                 = 368,     /* Iraq */
    CTRY_IRELAND              = 372,     /* Ireland */
    CTRY_ISRAEL               = 376,     /* Israel */
    CTRY_ITALY                = 380,     /* Italy */
    CTRY_JAMAICA              = 388,     /* Jamaica */
    CTRY_JAPAN                = 392,     /* Japan */
    CTRY_JAPAN1               = 393,     /* Japan (JP1) */
    CTRY_JAPAN2               = 394,     /* Japan (JP0) */
    CTRY_JAPAN3               = 395,     /* Japan (JP1-1) */
    CTRY_JAPAN4               = 396,     /* Japan (JE1) */
    CTRY_JAPAN5               = 397,     /* Japan (JE2) */
    CTRY_JORDAN               = 400,     /* Jordan */
    CTRY_KAZAKHSTAN           = 398,     /* Kazakhstan */
    CTRY_KENYA                = 404,     /* Kenya */
    CTRY_KOREA_NORTH          = 408,     /* North Korea */
    CTRY_KOREA_ROC            = 410,     /* South Korea */
    CTRY_KOREA_ROC2           = 411,     /* South Korea */
    CTRY_KUWAIT               = 414,     /* Kuwait */
    CTRY_LATVIA               = 428,     /* Latvia */
    CTRY_LEBANON              = 422,     /* Lebanon */
    CTRY_LIBYA                = 434,     /* Libya */
    CTRY_LIECHTENSTEIN        = 438,     /* Liechtenstein */
    CTRY_LITHUANIA            = 440,     /* Lithuania */
    CTRY_LUXEMBOURG           = 442,     /* Luxembourg */
    CTRY_MACAU                = 446,     /* Macau */
    CTRY_MACEDONIA            = 807,     /* the Former Yugoslav Republic of Macedonia */
    CTRY_MALAYSIA             = 458,     /* Malaysia */
    CTRY_MEXICO               = 484,     /* Mexico */
    CTRY_MONACO               = 492,     /* Principality of Monaco */
    CTRY_MOROCCO              = 504,     /* Morocco */
    CTRY_NETHERLANDS          = 528,     /* Netherlands */
    CTRY_NEW_ZEALAND          = 554,     /* New Zealand */
    CTRY_NICARAGUA            = 558,     /* Nicaragua */
    CTRY_NORWAY               = 578,     /* Norway */
    CTRY_OMAN                 = 512,     /* Oman */
    CTRY_PAKISTAN             = 586,     /* Islamic Republic of Pakistan */
    CTRY_PANAMA               = 591,     /* Panama */
    CTRY_PARAGUAY             = 600,     /* Paraguay */
    CTRY_PERU                 = 604,     /* Peru */
    CTRY_PHILIPPINES          = 608,     /* Republic of the Philippines */
    CTRY_POLAND               = 616,     /* Poland */
    CTRY_PORTUGAL             = 620,     /* Portugal */
    CTRY_PUERTO_RICO          = 630,     /* Puerto Rico */
    CTRY_QATAR                = 634,     /* Qatar */
    CTRY_ROMANIA              = 642,     /* Romania */
    CTRY_RUSSIA               = 643,     /* Russia */
    CTRY_SAUDI_ARABIA         = 682,     /* Saudi Arabia */
    CTRY_SINGAPORE            = 702,     /* Singapore */
    CTRY_SLOVAKIA             = 703,     /* Slovak Republic */
    CTRY_SLOVENIA             = 705,     /* Slovenia */
    CTRY_SOUTH_AFRICA         = 710,     /* South Africa */
    CTRY_SPAIN                = 724,     /* Spain */
    CTRY_SWEDEN               = 752,     /* Sweden */
    CTRY_SWITZERLAND          = 756,     /* Switzerland */
    CTRY_SYRIA                = 760,     /* Syria */
    CTRY_TAIWAN               = 158,     /* Taiwan */
    CTRY_THAILAND             = 764,     /* Thailand */
    CTRY_TRINIDAD_Y_TOBAGO    = 780,     /* Trinidad y Tobago */
    CTRY_TUNISIA              = 788,     /* Tunisia */
    CTRY_TURKEY               = 792,     /* Turkey */
    CTRY_UAE                  = 784,     /* U.A.E. */
    CTRY_UKRAINE              = 804,     /* Ukraine */
    CTRY_UNITED_KINGDOM       = 826,     /* United Kingdom */
    CTRY_UNITED_STATES        = 840,     /* United States */
    CTRY_URUGUAY              = 858,     /* Uruguay */
    CTRY_UZBEKISTAN           = 860,     /* Uzbekistan */
    CTRY_VENEZUELA            = 862,     /* Venezuela */
    CTRY_VIET_NAM             = 704,     /* Viet Nam */
    CTRY_YEMEN                = 887,     /* Yemen */
    CTRY_ZIMBABWE             = 716      /* Zimbabwe */
};

/* Enumerated Regulatory Domain Information */
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
	FCC1_FCCA	= 0x10,		/* USA */
	FCC1_WORLD	= 0x11,		/* Hong Kong */

	FCC2_FCCA	= 0x20,		/* Canada */
	FCC2_WORLD	= 0x21,		/* Australia & HK */
	FCC2_ETSIC	= 0x22,
	FRANCE_RES	= 0x31,		/* Legacy France for OEM */
	FCC3_FCCA	= 0x3A,		/* USA & Canada w/5470 band, 11h, DFS enabled */

	ETSI1_WORLD	= 0x37,
	ETSI3_ETSIA	= 0x32,		/* France (optional) */
	ETSI2_WORLD	= 0x35,		/* Hungary & others */
	ETSI3_WORLD	= 0x36,		/* France & others */
	ETSI4_WORLD	= 0x30,
	ETSI4_ETSIC	= 0x38,
	ETSI5_WORLD	= 0x39,
	ETSI6_WORLD	= 0x34,		/* Bulgaria */
	ETSI_RESERVED	= 0x33,		/* Reserved (Do not used) */

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

	APL1_WORLD	= 0x52,		/* Latin America */
	APL1_FCCA	= 0x53,
	APL1_APLA	= 0x54,
	APL1_ETSIC	= 0x55,
	APL2_ETSIC	= 0x56,		/* Venezuela */
	APL5_WORLD	= 0x58,		/* Chile */

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
	APL4		= 0x0450,	/* Singapore */
	APL5		= 0x0550,	/* Chile */

	ETSI1		= 0x0130,	/* Europe & others */
	ETSI2		= 0x0230,	/* Europe & others */
	ETSI3		= 0x0330,	/* Europe & others */
	ETSI4		= 0x0430,	/* Europe & others */
	ETSI5		= 0x0530,	/* Europe & others */
	ETSI6		= 0x0630,	/* Europe & others */
	ETSIA		= 0x0A30,	/* France */
	ETSIB		= 0x0B30,	/* Israel */
	ETSIC		= 0x0C30,	/* Latin America */

	FCC1		= 0x0110,	/* US & others */
	FCC2		= 0x0120,	/* Canada, Australia & New Zealand */
	FCC3		= 0x0160,	/* US w/new middle band & DFS */    
	FCCA		= 0x0A10,	 

	APLD		= 0x0D50,	/* South Korea */

	MKK1		= 0x0140,	/* Japan */
	MKK2		= 0x0240,	/* Japan Extended */
	MKKA		= 0x0A40,	/* Japan */

	NULL1		= 0x0198,
	WORLD		= 0x0199,
	DEBUG_REG_DMN	= 0x01ff
};
#define DEF_REGDMN		FCC1_FCCA

static struct {
	const char *name;
	HAL_REG_DOMAIN rd;
} domains[] = {
	{ "NO_ENUMRD",	NO_ENUMRD },
	{ "NULL1_WORLD",	NULL1_WORLD },
	{ "NULL1_ETSIB",	NULL1_ETSIB },
	{ "NULL1_ETSIC",	NULL1_ETSIC },
	{ "FCC1_FCCA",	FCC1_FCCA },
	{ "FCC1_WORLD",	FCC1_WORLD },

	{ "FCC2_FCCA",	FCC2_FCCA },
	{ "FCC2_WORLD",	FCC2_WORLD },
	{ "FCC2_ETSIC",	FCC2_ETSIC },
	{ "FRANCE_RES",	FRANCE_RES },
	{ "FCC3_FCCA",	FCC3_FCCA },

	{ "ETSI1_WORLD",	ETSI1_WORLD },
	{ "ETSI3_ETSIA",	ETSI3_ETSIA },
	{ "ETSI2_WORLD",	ETSI2_WORLD },
	{ "ETSI3_WORLD",	ETSI3_WORLD },
	{ "ETSI4_WORLD",	ETSI4_WORLD },
	{ "ETSI4_ETSIC",	ETSI4_ETSIC },
	{ "ETSI5_WORLD",	ETSI5_WORLD },
	{ "ETSI6_WORLD",	ETSI6_WORLD },
	{ "ETSI_RESERVED",	ETSI_RESERVED },

	{ "MKK1_MKKA",	MKK1_MKKA },
	{ "MKK1_MKKB",	MKK1_MKKB },
	{ "APL4_WORLD",	APL4_WORLD },
	{ "MKK2_MKKA",	MKK2_MKKA },
	{ "APL_RESERVED",	APL_RESERVED },
	{ "APL2_WORLD",	APL2_WORLD },
	{ "APL2_APLC",	APL2_APLC },
	{ "APL3_WORLD",	APL3_WORLD },

	{ "MKK1_FCCA",	MKK1_FCCA },
	{ "APL2_APLD",	APL2_APLD },
	{ "MKK1_MKKA1",	MKK1_MKKA1 },
	{ "MKK1_MKKA2",	MKK1_MKKA2 },

	{ "APL1_WORLD",	APL1_WORLD },
	{ "APL1_FCCA",	APL1_FCCA },
	{ "APL1_APLA",	APL1_APLA },
	{ "APL1_ETSIC",	APL1_ETSIC },
	{ "APL2_ETSIC",	APL2_ETSIC },
	{ "APL5_WORLD",	APL5_WORLD },

	{ "WOR0_WORLD",	WOR0_WORLD },
	{ "WOR1_WORLD",	WOR1_WORLD },
	{ "WOR2_WORLD",	WOR2_WORLD },
	{ "WOR3_WORLD",	WOR3_WORLD },
	{ "WOR4_WORLD",	WOR4_WORLD },
	{ "WOR5_ETSIC",	WOR5_ETSIC },

	{ "WOR01_WORLD",WOR01_WORLD },
	{ "WOR02_WORLD",WOR02_WORLD },
	{ "EU1_WORLD",	EU1_WORLD },

	{ "APL1",	APL1 },
	{ "APL2",	APL2 },
	{ "APL3",	APL3 },
	{ "APL4",	APL4 },
	{ "APL5",	APL5 },

	{ "ETSI1",	ETSI1 },
	{ "ETSI2",	ETSI2 },
	{ "ETSI3",	ETSI3 },
	{ "ETSI4",	ETSI4 },
	{ "ETSI5",	ETSI5 },
	{ "ETSI6",	ETSI6 },
	{ "ETSIA",	ETSIA },
	{ "ETSIB",	ETSIB },
	{ "ETSIC",	ETSIC },

	{ "FCC1",	FCC1 },
	{ "FCC2",	FCC2 },
	{ "FCC3",	FCC3 },
	{ "FCCA",	FCCA },	 

	{ "APLD",	APLD },

	{ "MKK1",	MKK1 },
	{ "MKK2",	MKK2 },
	{ "MKKA",	MKKA },

	{ "NULL1",	NULL1 },
	{ "WORLD",	WORLD },
	{ "DEBUG_REG_DMN", DEBUG_REG_DMN },
};

static HAL_BOOL
rdlookup(const char *name, u_int8_t *rd)
{
#define	N(a)	(sizeof(a)/sizeof(a[0]))
	int i;

	for (i = 0; i < N(domains); i++)
		if (strcasecmp(domains[i].name, name) == 0) {
			*rd = domains[i].rd;
			return AH_TRUE;
		}
	return AH_FALSE;
#undef N
}

static const char *
getrdname(HAL_REG_DOMAIN rd)
{
#define	N(a)	(sizeof(a)/sizeof(a[0]))
	int i;

	for (i = 0; i < N(domains); i++)
		if (domains[i].rd == rd)
			return domains[i].name;
	return NULL;
#undef N
}

static void
rdlist()
{
#define	N(a)	(sizeof(a)/sizeof(a[0]))
	int i;

	printf("\nRegulatory domains:\n\n");
	for (i = 0; i < N(domains); i++)
		printf("%-15s%s", domains[i].name,
			((i+1)%5) == 0 ? "\n" : "");
	printf("\n");
#undef N
}

typedef struct {
	HAL_CTRY_CODE	countryCode;	   
	HAL_REG_DOMAIN	regDmnEnum;
	const char*	isoName;
	const char*	name;
	HAL_BOOL	allow11g;
	HAL_BOOL	allow11aTurbo;
	HAL_BOOL	allow11gTurbo;
} COUNTRY_CODE_TO_ENUM_RD;
 
#define	YES	AH_TRUE
#define	NO	AH_FALSE
/* Index into table to avoid DEBUG and NO COUNTRY SET entries */
#define CTRY_ONLY_INDEX 2
/*
 * Country Code Table to Enumerated RD
 */
static const COUNTRY_CODE_TO_ENUM_RD allCountries[] = {
    {CTRY_DEBUG,       NO_ENUMRD,     "DB", "DEBUG",      YES, YES, YES    },
    {CTRY_DEFAULT,     DEF_REGDMN,    "NA", "NO_COUNTRY_SET", YES, YES, YES },
    {CTRY_ALBANIA,     NULL1_WORLD,   "AL", "ALBANIA",    YES, NO, YES     },
    {CTRY_ALGERIA,     NULL1_WORLD,   "DZ", "ALGERIA",    YES, NO, YES     },
    {CTRY_ARGENTINA,   APL3_WORLD,    "AR", "ARGENTINA",   NO, NO, NO      },
    {CTRY_ARMENIA,     ETSI4_WORLD,   "AM", "ARMENIA",    YES, NO, YES     },
    {CTRY_AUSTRALIA,   FCC2_WORLD,    "AU", "AUSTRALIA",  YES, YES, YES    },
    {CTRY_AUSTRIA,     ETSI5_WORLD,   "AT", "AUSTRIA",    YES, NO, YES     },
    {CTRY_AZERBAIJAN,  ETSI4_WORLD,   "AZ", "AZERBAIJAN", YES, YES, YES    },
    {CTRY_BAHRAIN,     NULL1_WORLD,   "BH", "BAHRAIN",    YES, NO, YES     },
    {CTRY_BELARUS,     NULL1_WORLD,   "BY", "BELARUS",    YES, NO, YES     },
    {CTRY_BELGIUM,     ETSI4_WORLD,   "BE", "BELGIUM",    YES, NO, YES     },
    {CTRY_BELIZE,      APL1_ETSIC,    "BZ", "BELIZE",     YES, YES, YES    },
    {CTRY_BOLIVIA,     APL1_ETSIC,    "BO", "BOLVIA",     YES, YES, YES    },
    {CTRY_BRAZIL,      NULL1_ETSIC,   "BR", "BRAZIL",      NO, NO, NO      },
    {CTRY_BRUNEI_DARUSSALAM,APL1_WORLD,"BN", "BRUNEI DARUSSALAM", YES, YES, YES },
    {CTRY_BULGARIA,    ETSI6_WORLD,   "BG", "BULGARIA",   YES, NO, YES     },
    {CTRY_CANADA,      FCC2_FCCA,     "CA", "CANADA",     YES, YES, YES    },
    {CTRY_CHILE,       APL5_WORLD,    "CL", "CHILE",      NO,  YES, NO     },
    {CTRY_CHINA,       APL1_WORLD,    "CN", "CHINA",      YES, YES, YES    },
    {CTRY_COLOMBIA,    FCC1_FCCA,     "CO", "COLOMBIA",   YES, NO, YES     },
    {CTRY_COSTA_RICA,  NULL1_WORLD,   "CR", "COSTA RICA", YES, NO, YES     },
    {CTRY_CROATIA,     ETSI3_WORLD,   "HR", "CROATIA",    YES, NO, YES     },
    {CTRY_CYPRUS,      ETSI1_WORLD,   "CY", "CYPRUS",     YES, YES, YES    },
    {CTRY_CZECH,       ETSI3_WORLD,   "CZ", "CZECH REPUBLIC", NO, NO, NO   },
    {CTRY_DENMARK,     ETSI1_WORLD,   "DK", "DENMARK",    YES, NO, YES     },
    {CTRY_DOMINICAN_REPUBLIC,FCC1_FCCA,"DO", "DOMINICAN REPUBLIC", YES, YES, YES },
    {CTRY_ECUADOR,     NULL1_WORLD,   "EC", "ECUADOR",     NO, NO,NO       },
    {CTRY_EGYPT,       NULL1_WORLD,   "EG", "EGYPT",      YES, NO, YES     },
    {CTRY_EL_SALVADOR, NULL1_WORLD,   "SV", "EL SALVADOR",YES, NO, YES     },    
    {CTRY_ESTONIA,     ETSI1_WORLD,   "EE", "ESTONIA",    YES, NO, YES     },
    {CTRY_FINLAND,     ETSI1_WORLD,   "FI", "FINLAND",    YES, NO, YES     },
    {CTRY_FRANCE,      ETSI3_WORLD,   "FR", "FRANCE",     YES, NO, YES     },
    {CTRY_FRANCE2,     ETSI3_WORLD,   "F2", "FRANCE_RES", YES, NO, YES     },
    {CTRY_GEORGIA,     ETSI4_WORLD,   "GE", "GEORGIA",    YES, YES, YES    },
    {CTRY_GERMANY,     ETSI1_WORLD,   "DE", "GERMANY",    YES, NO, YES     },
    {CTRY_GREECE,      NULL1_WORLD,   "GR", "GREECE",     YES, NO, YES     },
    {CTRY_GUATEMALA,   FCC1_FCCA,     "GT", "GUATEMALA",  YES, YES, YES    },
    {CTRY_HONDURAS,    NULL1_WORLD,   "HN", "HONDURAS",   YES, NO, YES     },
    {CTRY_HONG_KONG,   FCC2_WORLD,    "HK", "HONG KONG",  YES, YES, YES    },
    {CTRY_HUNGARY,     ETSI2_WORLD,   "HU", "HUNGARY",    YES, NO, YES     },
    {CTRY_ICELAND,     ETSI1_WORLD,   "IS", "ICELAND",    YES, NO, YES     },
    {CTRY_INDIA,       NULL1_WORLD,   "IN", "INDIA",      YES, NO, YES     },
    {CTRY_INDONESIA,   NULL1_WORLD,   "ID", "INDONESIA",  YES, NO, YES     },
    {CTRY_IRAN,        APL1_WORLD,    "IR", "IRAN",       YES, YES, YES    },
    {CTRY_IRELAND,     ETSI1_WORLD,   "IE", "IRELAND",    YES, NO, YES     },
    {CTRY_ISRAEL,      NULL1_WORLD,   "IL", "ISRAEL",     YES, NO, YES     },
    {CTRY_ITALY,       ETSI1_WORLD,   "IT", "ITALY",      YES, NO, YES     },
    {CTRY_JAPAN,       MKK1_MKKA,     "JP", "JAPAN",      YES, NO, NO      },
    {CTRY_JAPAN1,      MKK1_MKKB,     "J1", "JAPAN1",     YES, NO, NO      },
    {CTRY_JAPAN2,      MKK1_FCCA,     "J2", "JAPAN2",     YES, NO, NO      },    
    {CTRY_JAPAN3,      MKK2_MKKA,     "J3", "JAPAN3",     YES, NO, NO      },
    {CTRY_JAPAN4,      MKK1_MKKA1,    "J4", "JAPAN4",     YES, NO, NO      },
    {CTRY_JAPAN5,      MKK1_MKKA2,    "J5", "JAPAN5",     YES, NO, NO      },    
    {CTRY_JORDAN,      NULL1_WORLD,   "JO", "JORDAN",     YES, NO, YES     },
    {CTRY_KAZAKHSTAN,  NULL1_WORLD,   "KZ", "KAZAKHSTAN", YES, NO, YES     },
    {CTRY_KOREA_NORTH, APL2_WORLD,    "KP", "NORTH KOREA",YES,YES, YES     },
    {CTRY_KOREA_ROC,   APL2_WORLD,    "KR", "KOREA REPUBLIC", YES, NO, NO  },
    {CTRY_KOREA_ROC2,  APL2_APLD,     "K2", "KOREA REPUBLIC2", YES, NO, NO },
    {CTRY_KUWAIT,      NULL1_WORLD,   "KW", "KUWAIT",     YES, NO, YES     },
    {CTRY_LATVIA,      NULL1_WORLD,   "LV", "LATVIA",     YES, NO, YES     },
    {CTRY_LEBANON,     NULL1_WORLD,   "LB", "LEBANON",    YES, NO, YES     },
    {CTRY_LIECHTENSTEIN,ETSI2_WORLD,  "LI", "LIECHTENSTEIN", YES, NO, YES  },
    {CTRY_LITHUANIA,   ETSI1_WORLD,   "LT", "LITHUANIA",  YES, NO, YES     },
    {CTRY_LUXEMBOURG,  ETSI1_WORLD,   "LU", "LUXEMBOURG", YES, NO, YES     },
    {CTRY_MACAU,       FCC2_WORLD,    "MO", "MACAU",      YES, YES, YES    },
    {CTRY_MACEDONIA,   NULL1_WORLD,   "MK", "MACEDONIA",  YES, NO, YES     },
    {CTRY_MALAYSIA,    NULL1_WORLD,   "MY", "MALAYSIA",    NO, NO, NO      },
    {CTRY_MEXICO,      FCC1_FCCA,     "MX", "MEXICO",     YES, YES, YES    },
    {CTRY_MONACO,      ETSI4_WORLD,   "MC", "MONACO",     YES, YES, YES    },
    {CTRY_MOROCCO,     NULL1_WORLD,   "MA", "MOROCCO",    YES, NO, YES     },
    {CTRY_NETHERLANDS, ETSI1_WORLD,   "NL", "NETHERLANDS",YES, NO, YES     },
    {CTRY_NEW_ZEALAND, FCC2_ETSIC,    "NZ", "NEW ZEALAND",YES, NO, YES     },
    {CTRY_NORWAY,      ETSI1_WORLD,   "NO", "NORWAY",     YES, NO, YES     },
    {CTRY_OMAN,        NULL1_WORLD,   "OM", "OMAN",       YES, NO, YES     },
    {CTRY_PAKISTAN,    NULL1_WORLD,   "PK", "PAKISTAN",   YES, NO, YES     },
    {CTRY_PANAMA,      FCC1_FCCA,     "PA", "PANAMA",     YES, YES, YES    },
    {CTRY_PERU,        NULL1_WORLD,   "PE", "PERU",       YES, NO, YES     },
    {CTRY_PHILIPPINES, FCC1_WORLD,    "PH", "PHILIPPINES",YES, YES, YES    },
    {CTRY_POLAND,      ETSI1_WORLD,   "PL", "POLAND",     YES, NO, YES     },
    {CTRY_PORTUGAL,    ETSI1_WORLD,   "PT", "PORTUGAL",   YES, NO, YES     },
    {CTRY_PUERTO_RICO, FCC1_FCCA,     "PR", "PUERTO RICO",YES, YES, YES    },
    {CTRY_QATAR,       NULL1_WORLD,   "QA", "QATAR",      YES, NO, YES     },
    {CTRY_ROMANIA,     NULL1_WORLD,   "RO", "ROMANIA",    YES, NO, YES     },
    {CTRY_RUSSIA,      NULL1_WORLD,   "RU", "RUSSIA",     YES, NO, YES     },
    {CTRY_SAUDI_ARABIA,NULL1_WORLD,   "SA", "SAUDI ARABIA", YES, NO, YES   },
    {CTRY_SINGAPORE,   APL4_WORLD,    "SG", "SINGAPORE",  YES, YES, YES    },
    {CTRY_SLOVAKIA,    ETSI3_WORLD,   "SK", "SLOVAK REPUBLIC", YES, NO, YES },
    {CTRY_SLOVENIA,    ETSI1_WORLD,   "SI", "SLOVENIA",   YES, NO, YES     },
    {CTRY_SOUTH_AFRICA,ETSI1_WORLD,   "ZA", "SOUTH AFRICA", YES, YES, YES  },
    {CTRY_SPAIN,       ETSI1_WORLD,   "ES", "SPAIN",      YES, NO, YES     },
    {CTRY_SWEDEN,      ETSI1_WORLD,   "SE", "SWEDEN",     YES, NO, YES     },
    {CTRY_SWITZERLAND, ETSI2_WORLD,   "CH", "SWITZERLAND",YES, NO, YES     },
    {CTRY_SYRIA,       NULL1_WORLD,   "SY", "SYRIA",      YES, NO, YES     },
    {CTRY_TAIWAN,      APL3_WORLD,    "TW", "TAIWAN",     YES, YES, YES    },
    {CTRY_THAILAND,    APL2_WORLD,    "TH", "THAILAND",   YES, YES, YES    },
    {CTRY_TRINIDAD_Y_TOBAGO,ETSI4_WORLD,"TT", "TRINIDAD & TOBAGO", YES, NO, YES },
    {CTRY_TUNISIA,     ETSI3_WORLD,   "TN", "TUNISIA",    YES, NO, YES     },
    {CTRY_TURKEY,      ETSI3_WORLD,   "TR", "TURKEY",     YES, NO, YES     },
    {CTRY_UKRAINE,     NULL1_WORLD,   "UA", "UKRAINE",    YES, NO, YES     },
    {CTRY_UAE,         NULL1_WORLD,   "AE", "UNITED ARAB EMIRATES", YES, NO, YES },
    {CTRY_UNITED_KINGDOM, ETSI1_WORLD, "GB", "UNITED KINGDOM", YES, NO, YES },
    {CTRY_UNITED_STATES, FCC1_FCCA,   "US", "UNITED STATES", YES, YES, YES },
    {CTRY_URUGUAY,     APL2_WORLD,    "UY", "URUGUAY",    YES, NO, YES     },
    {CTRY_UZBEKISTAN,  FCC3_FCCA,     "UZ", "UZBEKISTAN", YES, YES, YES    },    
    {CTRY_VENEZUELA,   APL2_ETSIC,    "VE", "VENEZUELA",  YES, NO, YES     },
    {CTRY_VIET_NAM,    NULL1_WORLD,   "VN", "VIET NAM",   YES, NO, YES     },
    {CTRY_YEMEN,       NULL1_WORLD,   "YE", "YEMEN",      YES, NO, YES     },
    {CTRY_ZIMBABWE,    NULL1_WORLD,   "ZW", "ZIMBABWE",   YES, NO, YES     }    
};
#undef	YES
#undef	NO

static HAL_BOOL
cclookup(const char *name, u_int8_t *rd, HAL_CTRY_CODE *cc)
{
#define	N(a)	(sizeof(a)/sizeof(a[0]))
	int i;

	for (i = 0; i < N(allCountries); i++)
		if (strcasecmp(allCountries[i].isoName, name) == 0 ||
		    strcasecmp(allCountries[i].name, name) == 0) {
			*rd = allCountries[i].regDmnEnum;
			*cc = allCountries[i].countryCode;
			return AH_TRUE;
		}
	return AH_FALSE;
#undef N
}

static const char *
getccname(HAL_CTRY_CODE cc)
{
#define	N(a)	(sizeof(a)/sizeof(a[0]))
	int i;

	for (i = 0; i < N(allCountries); i++)
		if (allCountries[i].countryCode == cc)
			return allCountries[i].name;
	return NULL;
#undef N
}

static const char *
getccisoname(HAL_CTRY_CODE cc)
{
#define	N(a)	(sizeof(a)/sizeof(a[0]))
	int i;

	for (i = 0; i < N(allCountries); i++)
		if (allCountries[i].countryCode == cc)
			return allCountries[i].isoName;
	return NULL;
#undef N
}

static void
cclist()
{
#define	N(a)	(sizeof(a)/sizeof(a[0]))
	int i;

	printf("\nCountry codes:\n");
	for (i = 0; i < N(allCountries); i++)
		printf("%2s %-15.15s%s",
			allCountries[i].isoName,
			allCountries[i].name,
			((i+1)%4) == 0 ? "\n" : " ");
	printf("\n");
#undef N
}

static HAL_BOOL
setRateTable(struct ath_hal *ah, HAL_CHANNEL *chan, 
		   int16_t tpcScaleReduction, int16_t powerLimit,
                   int16_t *pMinPower, int16_t *pMaxPower);

static void
calctxpower(struct ath_hal *ah, int nchan, HAL_CHANNEL *chans,
	int16_t tpcScaleReduction, int16_t powerLimit, int16_t *txpow)
{
	int16_t minpow;
	int i;

	for (i = 0; i < nchan; i++)
		if (!setRateTable(ah, &chans[i],
		    tpcScaleReduction, powerLimit, &minpow, &txpow[i])) {
			printf("unable to set rate table\n");
			exit(-1);
		}
}

int	n = 1;
const char *sep = "";
int	dopassive = 0;

static void
dumpchannels(struct ath_hal *ah, int nc, HAL_CHANNEL *chans, int16_t *txpow)
{
	int i;

	for (i = 0; i < nc; i++) {
		HAL_CHANNEL *c = &chans[i];
		int type;

		printf("%s%u", sep, c->channel);
		if (IS_CHAN_TURBO(c))
			type = 'T';
		else if (IS_CHAN_A(c))
			type = 'A';
		else if (IS_CHAN_G(c))
			type = 'G';
		else
			type = 'B';
		if (dopassive && IS_CHAN_PASSIVE(c))
			type = tolower(type);
		printf("%c %d.%d", type, txpow[i]/2, (txpow[i]%2)*5);
		if ((n++ % 6) == 0)
			sep = "\n";
		else
			sep = " ";
	}
}

static void
checkchannels(struct ath_hal *ah, HAL_CHANNEL *chans, int nchan)
{
	int i;

	for (i = 0; i < nchan; i++) {
		HAL_CHANNEL *c = &chans[i];
		if (!ath_hal_checkchannel(ah, c))
			printf("Channel %u (0x%x) disallowed\n",
				c->channel, c->channelFlags);
	}
}

static void
intersect(HAL_CHANNEL *dst, int16_t *dtxpow, int *nd,
    const HAL_CHANNEL *src, int16_t *stxpow, int ns)
{
	int i = 0, j, k, l;
	while (i < *nd) {
		for (j = 0; j < ns && dst[i].channel != src[j].channel; j++)
			;
		if (j < ns && dtxpow[i] == stxpow[j]) {
			for (k = i+1, l = i; k < *nd; k++, l++)
				dst[l] = dst[k];
			(*nd)--;
		} else
			i++;
	}
}

static void
usage(const char *progname)
{
	printf("usage: %s [-o] [-i] [cc | rd]\n", progname);
	exit(-1);
}

static	int verbose = 1;

static int
runtest(struct ath_hal_private *ahp, int cc, int modes, int outdoor, int xchanmode)
{
	HAL_CHANNEL chans[IEEE80211_CHAN_MAX];
	int i, n;

	if (verbose) {
		if (cc != CTRY_DEFAULT)
			printf("%s (%s, 0x%x, %u) %s (0x%x, %u)\n",
				getccname(cc), getccisoname(cc), cc, cc,
				getrdname(ahp->ah_currentRD), ahp->ah_currentRD,
				ahp->ah_currentRD);
		else
			printf("%s (0x%x, %u)\n",
				getrdname(ahp->ah_currentRD), ahp->ah_currentRD,
				ahp->ah_currentRD);
	}
	if (modes == 0)
		modes = HAL_MODE_11A | HAL_MODE_11B |
			HAL_MODE_11G | HAL_MODE_TURBO;
	if (!ath_hal_init_channels(&ahp->h, chans, IEEE80211_CHAN_MAX, &n,
	    cc, modes, outdoor, xchanmode)) {
		printf("ath_hal_init_channels failed!\n");
		return -1;
	}
	for (i = 0; i < n; i++) {
		if (!ath_hal_checkchannel(&ahp->h, &chans[i]))
			printf("%u/0x%x: FAIL\n",
				chans[i].channel, chans[i].channelFlags);
		else if (verbose > 1)
			printf("%u/0x%x: ok\n",
				chans[i].channel, chans[i].channelFlags);
	}
	return 0;
}

int
main(int argc, char *argv[])
{
#define	N(a)	(sizeof(a)/sizeof(a[0]))
	struct ath_hal_private ahp;
	int i, n;
	HAL_BOOL extendedChanMode = AH_TRUE;
	int modes = 0;

	memset(&ahp, 0, sizeof(ahp));
	ahp.ah_getChannelEdges = getChannelEdges;
	ahp.ah_getWirelessModes = getWirelessModes;
	ahp.ah_opmode = HAL_M_STA;
	ahp.ah_currentRD = 16;		/* FCC */

	while ((i = getopt(argc, argv, "deoilm:pABGTqv")) != -1)
		switch (i) {
		case 'd':
			ath_hal_debug = 1;
			break;
		case 'e':
			extendedChanMode = AH_FALSE;
			break;
		case 'o':
			outdoor = AH_TRUE;
			break;
		case 'i':
			outdoor = AH_FALSE;
			break;
		case 'l':
			cclist();
			rdlist();
			exit(0);
		case 'm':
			if (strncasecmp(optarg, "sta", 2) == 0)
				ahp.ah_opmode = HAL_M_STA;
			else if (strncasecmp(optarg, "ibss", 2) == 0)
				ahp.ah_opmode = HAL_M_IBSS;
			else if (strncasecmp(optarg, "adhoc", 2) == 0)
				ahp.ah_opmode = HAL_M_IBSS;
			else if (strncasecmp(optarg, "ap", 2) == 0)
				ahp.ah_opmode = HAL_M_HOSTAP;
			else if (strncasecmp(optarg, "hostap", 2) == 0)
				ahp.ah_opmode = HAL_M_HOSTAP;
			else if (strncasecmp(optarg, "monitor", 2) == 0)
				ahp.ah_opmode = HAL_M_MONITOR;
			else
				usage(argv[0]);
			break;
		case 'p':
			dopassive = 1;
			break;
		case 'A':
			modes |= HAL_MODE_11A;
			break;
		case 'B':
			modes |= HAL_MODE_11B;
			break;
		case 'G':
			modes |= HAL_MODE_11G;
			break;
		case 'T':
			modes |= HAL_MODE_TURBO;
			break;
		case 'q':
			verbose = 0;
			break;
		case 'v':
			verbose++;
			break;
		default:
			usage(argv[0]);
		}
	if (modes == 0)
		modes = HAL_MODE_11A | HAL_MODE_11B |
			HAL_MODE_11G | HAL_MODE_TURBO;

	for (i = 0; i < N(allCountries); i++) {
		ahp.ah_currentRD = allCountries[i].regDmnEnum;
		runtest(&ahp, allCountries[i].countryCode,
			modes, outdoor, extendedChanMode);
	}
	for (i = 0; i < N(domains); i++) {
		ahp.ah_currentRD = domains[i].rd;
		runtest(&ahp, CTRY_DEFAULT, modes, outdoor, extendedChanMode);
	}
	return (0);
}

/*
 * Search a list for a specified value v that is within
 * EEP_DELTA of the search values.  Return the closest
 * values in the list above and below the desired value.
 * EEP_DELTA is a factional value; everything is scaled
 * so only integer arithmetic is used.
 *
 * NB: the input list is assumed to be sorted in ascending order
 */
static void
ar5212GetLowerUpperValues(u_int16_t v, u_int16_t *lp, u_int16_t listSize,
                          u_int16_t *vlo, u_int16_t *vhi)
{
	u_int32_t target = v * EEP_SCALE;
	u_int16_t *ep = lp+listSize;

	/*
	 * Check first and last elements for out-of-bounds conditions.
	 */
	if (target < (u_int32_t)(lp[0] * EEP_SCALE - EEP_DELTA)) {
		*vlo = *vhi = lp[0];
		return;
	}
	if (target > (u_int32_t)(ep[-1] * EEP_SCALE + EEP_DELTA)) {
		*vlo = *vhi = ep[-1];
		return;
	}

	/* look for value being near or between 2 values in list */
	for (; lp < ep; lp++) {
		/*
		 * If value is close to the current value of the list
		 * then target is not between values, it is one of the values
		 */
		if (abs(lp[0] * EEP_SCALE - target) < EEP_DELTA) {
			*vlo = *vhi = lp[0];
			return;
		}
		/*
		 * Look for value being between current value and next value
		 * if so return these 2 values
		 */
		if (target < (u_int32_t)(lp[1] * EEP_SCALE - EEP_DELTA)) {
			*vlo = lp[0];
			*vhi = lp[1];
			return;
		}
	}
}

/*
 * Find the maximum conformance test limit for the given channel and CTL info
 */
static u_int16_t
ar5212GetMaxEdgePower(u_int16_t channel, RD_EDGES_POWER *pRdEdgesPower)
{
	/* temp array for holding edge channels */
	u_int16_t tempChannelList[NUM_EDGES];
	u_int16_t clo, chi, twiceMaxEdgePower;
	int i, numEdges;

	/* Get the edge power */
	for (i = 0; i < NUM_EDGES; i++) {
		if (pRdEdgesPower[i].rdEdge == 0)
			break;
		tempChannelList[i] = pRdEdgesPower[i].rdEdge;
	}
	numEdges = i;

	ar5212GetLowerUpperValues(channel, tempChannelList,
		numEdges, &clo, &chi);
	/* Get the index for the lower channel */
	for (i = 0; i < numEdges && clo != tempChannelList[i]; i++)
		;
	/* Is lower channel ever outside the rdEdge? */
	HALASSERT(i != numEdges);

	if ((clo == chi && clo == channel) || (pRdEdgesPower[i].flag)) {
		/* 
		 * If there's an exact channel match or an inband flag set
		 * on the lower channel use the given rdEdgePower 
		 */
		twiceMaxEdgePower = pRdEdgesPower[i].twice_rdEdgePower;
		HALASSERT(twiceMaxEdgePower > 0);
	} else
		twiceMaxEdgePower = MAX_RATE_POWER;
	return twiceMaxEdgePower;
}

/*
 * Returns interpolated or the scaled up interpolated value
 */
static u_int16_t
interpolate(u_int16_t target, u_int16_t srcLeft, u_int16_t srcRight,
	u_int16_t targetLeft, u_int16_t targetRight)
{
	u_int16_t rv;
	int16_t lRatio;

	/* to get an accurate ratio, always scale, if want to scale, then don't scale back down */
	if ((targetLeft * targetRight) == 0)
		return 0;

	if (srcRight != srcLeft) {
		/*
		 * Note the ratio always need to be scaled,
		 * since it will be a fraction.
		 */
		lRatio = (target - srcLeft) * EEP_SCALE / (srcRight - srcLeft);
		if (lRatio < 0) {
		    /* Return as Left target if value would be negative */
		    rv = targetLeft;
		} else if (lRatio > EEP_SCALE) {
		    /* Return as Right target if Ratio is greater than 100% (SCALE) */
		    rv = targetRight;
		} else {
			rv = (lRatio * targetRight + (EEP_SCALE - lRatio) *
					targetLeft) / EEP_SCALE;
		}
	} else {
		rv = targetLeft;
	}
	return rv;
}

/*
 * Return the four rates of target power for the given target power table 
 * channel, and number of channels
 */
static void
ar5212GetTargetPowers(struct ath_hal *ah, HAL_CHANNEL *chan,
	TRGT_POWER_INFO *powInfo,
	u_int16_t numChannels, TRGT_POWER_INFO *pNewPower)
{
	/* temp array for holding target power channels */
	u_int16_t tempChannelList[NUM_TEST_FREQUENCIES];
	u_int16_t clo, chi, ixlo, ixhi;
	int i;

	/* Copy the target powers into the temp channel list */
	for (i = 0; i < numChannels; i++)
		tempChannelList[i] = powInfo[i].testChannel;

	ar5212GetLowerUpperValues(chan->channel, tempChannelList,
		numChannels, &clo, &chi);

	/* Get the indices for the channel */
	ixlo = ixhi = 0;
	for (i = 0; i < numChannels; i++) {
		if (clo == tempChannelList[i]) {
			ixlo = i;
		}
		if (chi == tempChannelList[i]) {
			ixhi = i;
			break;
		}
	}

	/*
	 * Get the lower and upper channels, target powers,
	 * and interpolate between them.
	 */
	pNewPower->twicePwr6_24 = interpolate(chan->channel, clo, chi,
		powInfo[ixlo].twicePwr6_24, powInfo[ixhi].twicePwr6_24);
	pNewPower->twicePwr36 = interpolate(chan->channel, clo, chi,
		powInfo[ixlo].twicePwr36, powInfo[ixhi].twicePwr36);
	pNewPower->twicePwr48 = interpolate(chan->channel, clo, chi,
		powInfo[ixlo].twicePwr48, powInfo[ixhi].twicePwr48);
	pNewPower->twicePwr54 = interpolate(chan->channel, clo, chi,
		powInfo[ixlo].twicePwr54, powInfo[ixhi].twicePwr54);
}

static RD_EDGES_POWER*
findEdgePower(struct ath_hal *ah, u_int ctl)
{
	int i;

	for (i = 0; i < _numCtls; i++)
		if (_ctl[i] == ctl)
			return &_rdEdgesPower[i * NUM_EDGES];
	return AH_NULL;
}

/*
 * Sets the transmit power in the baseband for the given
 * operating channel and mode.
 */
static HAL_BOOL
setRateTable(struct ath_hal *ah, HAL_CHANNEL *chan, 
		   int16_t tpcScaleReduction, int16_t powerLimit,
                   int16_t *pMinPower, int16_t *pMaxPower)
{
	u_int16_t ratesArray[16];
	u_int16_t *rpow = ratesArray;
	u_int16_t twiceMaxRDPower, twiceMaxEdgePower, twiceMaxEdgePowerCck;
	int8_t twiceAntennaGain, twiceAntennaReduction;
	TRGT_POWER_INFO targetPowerOfdm, targetPowerCck;
	RD_EDGES_POWER *rep;
	int16_t scaledPower;
	u_int8_t cfgCtl;

	twiceMaxRDPower = ath_hal_getchannelpower(ah, chan) * 2;
	*pMaxPower = -MAX_RATE_POWER;
	*pMinPower = MAX_RATE_POWER;

	/* Get conformance test limit maximum for this channel */
	cfgCtl = ath_hal_getctl(ah, chan);
	rep = findEdgePower(ah, cfgCtl);
	if (rep != AH_NULL)
		twiceMaxEdgePower = ar5212GetMaxEdgePower(chan->channel, rep);
	else
		twiceMaxEdgePower = MAX_RATE_POWER;

	if (IS_CHAN_G(chan)) {
		/* Check for a CCK CTL for 11G CCK powers */
		cfgCtl = (cfgCtl & 0xFC) | 0x01;
		rep = findEdgePower(ah, cfgCtl);
		if (rep != AH_NULL)
			twiceMaxEdgePowerCck = ar5212GetMaxEdgePower(chan->channel, rep);
		else
			twiceMaxEdgePowerCck = MAX_RATE_POWER;
	} else {
		/* Set the 11B cck edge power to the one found before */
		twiceMaxEdgePowerCck = twiceMaxEdgePower;
	}

	/* Get Antenna Gain reduction */
	if (IS_CHAN_5GHZ(chan)) {
		twiceAntennaGain = antennaGainMax[0];
	} else {
		twiceAntennaGain = antennaGainMax[1];
	}
	twiceAntennaReduction =
		ath_hal_getantennareduction(ah, chan, twiceAntennaGain);

	if (IS_CHAN_OFDM(chan)) {
		/* Get final OFDM target powers */
		if (IS_CHAN_G(chan)) { 
			/* TODO - add Turbo 2.4 to this mode check */
			ar5212GetTargetPowers(ah, chan, trgtPwr_11g,
				numTargetPwr_11g, &targetPowerOfdm);
		} else {
			ar5212GetTargetPowers(ah, chan, trgtPwr_11a,
				numTargetPwr_11a, &targetPowerOfdm);
		}

		/* Get Maximum OFDM power */
		/* Minimum of target and edge powers */
		scaledPower = AH_MIN(twiceMaxEdgePower,
				twiceMaxRDPower - twiceAntennaReduction);

		/*
		 * If turbo is set, reduce power to keep power
		 * consumption under 2 Watts.  Note that we always do
		 * this unless specially configured.  Then we limit
		 * power only for non-AP operation.
		 */
		if (IS_CHAN_TURBO(chan)
#ifdef AH_ENABLE_AP_SUPPORT
		    && AH_PRIVATE(ah)->ah_opmode != HAL_M_HOSTAP
#endif
		) {
			/*
			 * If turbo is set, reduce power to keep power
			 * consumption under 2 Watts
			 */
			if (eeversion >= AR_EEPROM_VER3_1)
				scaledPower = AH_MIN(scaledPower,
					turbo2WMaxPower5);
			/*
			 * EEPROM version 4.0 added an additional
			 * constraint on 2.4GHz channels.
			 */
			if (eeversion >= AR_EEPROM_VER4_0 &&
			    IS_CHAN_2GHZ(chan))
				scaledPower = AH_MIN(scaledPower,
					turbo2WMaxPower2);
		}
		/* Reduce power by max regulatory domain allowed restrictions */
		scaledPower -= (tpcScaleReduction * 2);
		scaledPower = (scaledPower < 0) ? 0 : scaledPower;
		scaledPower = AH_MIN(scaledPower, powerLimit);

		scaledPower = AH_MIN(scaledPower, targetPowerOfdm.twicePwr6_24);

		/* Set OFDM rates 9, 12, 18, 24, 36, 48, 54, XR */
		rpow[0] = rpow[1] = rpow[2] = rpow[3] = rpow[4] = scaledPower;
		rpow[5] = AH_MIN(rpow[0], targetPowerOfdm.twicePwr36);
		rpow[6] = AH_MIN(rpow[0], targetPowerOfdm.twicePwr48);
		rpow[7] = AH_MIN(rpow[0], targetPowerOfdm.twicePwr54);

#ifdef notyet
		if (eeversion >= AR_EEPROM_VER4_0) {
			/* Setup XR target power from EEPROM */
			rpow[15] = AH_MIN(scaledPower, IS_CHAN_2GHZ(chan) ?
				xrTargetPower2 : xrTargetPower5);
		} else {
			/* XR uses 6mb power */
			rpow[15] = rpow[0];
		}
#else
		rpow[15] = rpow[0];
#endif

		*pMinPower = rpow[7];
		*pMaxPower = rpow[0];

#if 0
		ahp->ah_ofdmTxPower = rpow[0];
#endif

		HALDEBUG(ah, "%s: MaxRD: %d TurboMax: %d MaxCTL: %d "
			"TPC_Reduction %d\n",
			__func__,
			twiceMaxRDPower, turbo2WMaxPower5,
			twiceMaxEdgePower, tpcScaleReduction * 2);
	}

	if (IS_CHAN_CCK(chan) || IS_CHAN_G(chan)) {
		/* Get final CCK target powers */
		ar5212GetTargetPowers(ah, chan, trgtPwr_11b,
			numTargetPwr_11b, &targetPowerCck);

		/* Reduce power by max regulatory domain allowed restrictions */
		scaledPower = AH_MIN(twiceMaxEdgePowerCck,
			twiceMaxRDPower - twiceAntennaReduction);

		scaledPower -= (tpcScaleReduction * 2);
		scaledPower = (scaledPower < 0) ? 0 : scaledPower;
		scaledPower = AH_MIN(scaledPower, powerLimit);

		rpow[8] = (scaledPower < 1) ? 1 : scaledPower;

		/* Set CCK rates 2L, 2S, 5.5L, 5.5S, 11L, 11S */
		rpow[8]  = AH_MIN(scaledPower, targetPowerCck.twicePwr6_24);
		rpow[9]  = AH_MIN(scaledPower, targetPowerCck.twicePwr36);
		rpow[10] = rpow[9];
		rpow[11] = AH_MIN(scaledPower, targetPowerCck.twicePwr48);
		rpow[12] = rpow[11];
		rpow[13] = AH_MIN(scaledPower, targetPowerCck.twicePwr54);
		rpow[14] = rpow[13];

		/* Set min/max power based off OFDM values or initialization */
		if (rpow[13] < *pMinPower)
		    *pMinPower = rpow[13];
		if (rpow[9] > *pMaxPower)
		    *pMaxPower = rpow[9];

	}
#if 0
	ahp->ah_tx6PowerInHalfDbm = *pMaxPower;
#endif
	return AH_TRUE;
}

void*
ath_hal_malloc(size_t size)
{
	return calloc(1, size);
}

void
ath_hal_free(void* p)
{
	return free(p);
}

void
ath_hal_vprintf(struct ath_hal *ah, const char* fmt, va_list ap)
{
	vprintf(fmt, ap);
}

void
ath_hal_printf(struct ath_hal *ah, const char* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	ath_hal_vprintf(ah, fmt, ap);
	va_end(ap);
}

void
HALDEBUG(struct ath_hal *ah, const char* fmt, ...)
{
	if (ath_hal_debug) {
		__va_list ap;
		va_start(ap, fmt);
		ath_hal_vprintf(ah, fmt, ap);
		va_end(ap);
	}
}

void
HALDEBUGn(struct ath_hal *ah, u_int level, const char* fmt, ...)
{
	if (ath_hal_debug >= level) {
		__va_list ap;
		va_start(ap, fmt);
		ath_hal_vprintf(ah, fmt, ap);
		va_end(ap);
	}
}

/*
 * Copyright (c) 2002-2006 Sam Leffler, Errno Consulting
 * Copyright (c) 2002-2006 Atheros Communications, Inc.
 * All rights reserved.
 *
 * $Id: //depot/sw/branches/sam_hal/ah.c#15 $
 */
#include "opt_ah.h"

#include "ah.h"
#include "ah_internal.h"
#include "ah_devid.h"

#ifdef AH_SUPPORT_AR5210
extern	struct ath_hal *ar5210Attach(u_int16_t, HAL_SOFTC,
	HAL_BUS_TAG, HAL_BUS_HANDLE, HAL_STATUS*);
#endif
#ifdef AH_SUPPORT_AR5211
extern	struct ath_hal *ar5211Attach(u_int16_t, HAL_SOFTC,
	HAL_BUS_TAG, HAL_BUS_HANDLE, HAL_STATUS*);
#endif
#ifdef AH_SUPPORT_AR5212
extern	struct ath_hal *ar5212Attach(u_int16_t, HAL_SOFTC,
	HAL_BUS_TAG, HAL_BUS_HANDLE, HAL_STATUS*);
#endif
#ifdef AH_SUPPORT_AR5312
extern	struct ath_hal *ar5312Attach(u_int16_t, HAL_SOFTC,
	HAL_BUS_TAG, HAL_BUS_HANDLE, HAL_STATUS*);
#endif
#ifdef AH_SUPPORT_AR5416
extern	struct ath_hal *ar5416Attach(u_int16_t, HAL_SOFTC,
	HAL_BUS_TAG, HAL_BUS_HANDLE, HAL_STATUS*);
#endif

#include "version.h"
char ath_hal_version[] = ATH_HAL_VERSION;
const char* ath_hal_buildopts[] = {
#ifdef AH_SUPPORT_AR5210
	"AR5210",
#endif
#ifdef AH_SUPPORT_AR5211
	"AR5211",
#endif
#ifdef AH_SUPPORT_AR5212
	"AR5212",
#endif
#ifdef AH_SUPPORT_AR5312
	"AR5312",
#endif
#ifdef AH_SUPPORT_AR5416
	"AR5416",
#endif
#ifdef AH_SUPPORT_AR9000
	"AR9160",
#endif
#ifdef AH_SUPPORT_AR9200
	"AR9280",
#endif
#ifdef AH_SUPPORT_5111
	"RF5111",
#endif
#ifdef AH_SUPPORT_5112
	"RF5112",
#endif
#ifdef AH_SUPPORT_2413
	"RF2413",
#endif
#ifdef AH_SUPPORT_5413
	"RF5413",
#endif
#ifdef AH_SUPPORT_2316
	"RF2316",
#endif
#ifdef AH_SUPPORT_2317
	"RF2317",
#endif
#ifdef AH_SUPPORT_2133
	"RF2133",
#endif
#ifdef AH_SUPPORT_2425
	"RF2425",
#endif
#ifdef AH_DEBUG
	"DEBUG",
#endif
#ifdef AH_ASSERT
	"ASSERT",
#endif
#ifdef AH_DEBUG_ALQ
	"DEBUG_ALQ",
#endif
#ifdef AH_REGOPS_FUNC
	"REGOPS_FUNC",
#endif
#ifdef AH_PRIVATE_DIAG
	"PRIVATE_DIAG",
#endif
#ifdef AH_SUPPORT_WRITE_EEPROM
	"WRITE_EEPROM",
#endif
#ifdef AH_SUPPORT_WRITE_REGDOMAIN
	"WRITE_REGDOMAIN",
#endif
#ifdef AH_DEBUG_COUNTRY
	"DEBUG_COUNTRY",
#endif
#ifdef AH_NEED_DESC_SWAP
	"TX_DESC_SWAP",
#endif
#ifdef AH_SUPPORT_XR
	"XR",
#endif
#ifdef AH_SUPPORT_WRITE_REG
	"WRITE_REG",
#endif
#ifdef AH_DISABLE_WME
	"DISABLE_WME",
#endif
#ifdef AH_SUPPORT_11D
	"11D",
#endif
	AH_NULL
};

static const char*
ath_hal_devname(u_int16_t devid)
{
	switch (devid) {
	case AR5210_PROD:
	case AR5210_DEFAULT:
		return "Atheros 5210";

	case AR5211_DEVID:
	case AR5311_DEVID:
	case AR5211_DEFAULT:
		return "Atheros 5211";
	case AR5211_FPGA11B:
		return "Atheros 5211 (FPGA)";

	case AR5212_FPGA:
		return "Atheros 5212 (FPGA)";
	case AR5212_AR5312_REV2:
	case AR5212_AR5312_REV7:
		return "Atheros 5312 WiSoC";
	case AR5212_AR2315_REV6:
	case AR5212_AR2315_REV7:
		return "Atheros 2315 WiSoC";
	case AR5212_AR2317_REV1:
		return "Atheros 2317 WiSoC REV1";
	case AR5212_AR2317_REV2:
		return "Atheros 2317 WiSoC REV2";
	case AR5212_AR2313_REV8:
		return "Atheros 2313 WiSoC";
	case AR5212_DEVID:
	case AR5212_DEVID_IBM:
	case AR5212_DEVID_WMG:
	case AR5212_DEFAULT:
		return "Atheros 5212";
	case AR5212_AR2413:
	case AR5212_DEVID_ACTIONTEC:
		return "Atheros 2413";
	case AR5212_AR2417:
		return "Atheros 2417";
	case AR5212_AR5413:
		return "Atheros 5413";
	case AR5416_DEVID_EMU_PCI:
	case AR5416_DEVID:
		return "Atheros 5416";
	case AR5416_DEVID_EMU_PCIE:
	case AR5418_DEVID:
		return "Atheros 5418";
#ifdef AH_SUPPORT_AR9000
	case AR5416_DEVID_AR9160_PCI:
	    return "Atheros 9160";
#endif
#ifdef AH_SUPPORT_AR9200
        case AR5416_DEVID_AR9280_PCI:
        case AR5416_DEVID_AR9280_PCIE:
                return "Atheros 9280";
#endif
	case AR5212_AR5424:
		return "Atheros 5424/2424";
	}
	return AH_NULL;
}

const char*
ath_hal_probe(u_int16_t vendorid, u_int16_t devid)
{
	return (vendorid == ATHEROS_VENDOR_ID ||
		vendorid == ATHEROS_3COM_VENDOR_ID ||
		vendorid == ATHEROS_3COM2_VENDOR_ID ?
			ath_hal_devname(devid) : 0);
}

/*
 * Attach detects device chip revisions, initializes the hwLayer
 * function list, reads EEPROM information,
 * selects reset vectors, and performs a short self test.
 * Any failures will return an error that should cause a hardware
 * disable.
 */
struct ath_hal*
ath_hal_attach(u_int16_t devid, HAL_SOFTC sc,
	HAL_BUS_TAG st, HAL_BUS_HANDLE sh, HAL_STATUS *error)
{
	struct ath_hal *ah=AH_NULL;
	ath_hal_printstr(ah,
		"Atheros HAL provided by OpenWrt, DD-WRT and MakSat Technologies"
#ifdef AH_RELINFO
		AH_RELINFO
#endif
		"\n"
	);
	switch (devid) {
#ifdef AH_SUPPORT_AR5416
	case AR5416_DEVID_EMU_PCI:
	case AR5416_DEVID_EMU_PCIE:
	case AR5416_DEVID:
	case AR5418_DEVID:	
#ifdef AH_SUPPORT_AR9000
	case AR5416_DEVID_AR9160_PCI:
#endif
#ifdef AH_SUPPORT_AR9200
       	case AR5416_DEVID_AR9280_PCI:
        case AR5416_DEVID_AR9280_PCIE:
#endif
		ah = ar5416Attach(devid, sc, st, sh, error);
		if (ah)
			ah->ah_macType = 5416;
		break;
#endif
#ifdef AH_SUPPORT_AR5312
	case AR5212_AR5312_REV2:
	case AR5212_AR5312_REV7:
	case AR5212_AR2313_REV8:
	case AR5212_AR2315_REV6:
	case AR5212_AR2315_REV7:
	case AR5212_AR2317_REV1:
	case AR5212_AR2317_REV2:
		ah = ar5312Attach(devid, sc, st, sh, error);
		if (ah)
			ah->ah_macType = 5212;
		break;
#endif
#ifdef AH_SUPPORT_AR5212
#ifndef AH_SUPPORT_AR5312
	case AR5212_DEVID_IBM:
	case AR5212_DEVID_WMG:
	case AR5212_DEVID_ACTIONTEC:
	case AR5212_AR2413:
	case AR5212_AR5413:
	case AR5212_AR5424:
	case AR5212_DEVID_FF19: /* XXX PCI Express extra */
		devid = AR5212_DEVID;
		/* fall thru... */
	case AR5212_DEVID:
	case AR5212_FPGA:
	case AR5212_AR2417:
	case AR5212_DEFAULT:
		ah = ar5212Attach(devid, sc, st, sh, error);
		if (ah)
			ah->ah_macType = 5212;
		break;
#endif
#endif
#ifdef AH_SUPPORT_AR5211
	case AR5211_DEVID:
	case AR5311_DEVID:
	case AR5211_FPGA11B:
	case AR5211_DEFAULT:
		ah = ar5211Attach(devid, sc, st, sh, error);
		if (ah)
			ah->ah_macType = 5211;
		break;
#endif
#ifdef AH_SUPPORT_AR5210
	case AR5210_AP:
	case AR5210_PROD:
	case AR5210_DEFAULT:
		ah = ar5210Attach(devid, sc, st, sh, error);
		if (ah)
			ah->ah_macType = 5210;
		break;
#endif
	default:
		ah = AH_NULL;
		*error = HAL_ENXIO;
		break;
	}
	if (ah != AH_NULL) {
		/* copy back private state to public area */
		ah->ah_devid = AH_PRIVATE(ah)->ah_devid;
		ah->ah_subvendorid = AH_PRIVATE(ah)->ah_subvendorid;
		ah->ah_macVersion = AH_PRIVATE(ah)->ah_macVersion;
		ah->ah_macRev = AH_PRIVATE(ah)->ah_macRev;
		ah->ah_phyRev = AH_PRIVATE(ah)->ah_phyRev;
		ah->ah_analog5GhzRev = AH_PRIVATE(ah)->ah_analog5GhzRev;
		ah->ah_analog2GhzRev = AH_PRIVATE(ah)->ah_analog2GhzRev;
	}
	return ah;
}

/*
 * Set the vendor ID for the driver.  This is used by the
 * regdomain to make vendor specific changes.
 */
HAL_BOOL
ath_hal_setvendor(struct ath_hal *ah, u_int32_t vendor)
{
	AH_PRIVATE(ah)->ah_vendor = vendor;
	return AH_TRUE;
}


/*
 * Poll the register looking for a specific value.
 */
HAL_BOOL
ath_hal_wait(struct ath_hal *ah, u_int reg, u_int32_t mask, u_int32_t val)
{
#define	AH_TIMEOUT	1000
	int i;

	for (i = 0; i < AH_TIMEOUT; i++) {
		if ((OS_REG_READ(ah, reg) & mask) == val)
			return AH_TRUE;
		OS_DELAY(10);
	}
	HALDEBUG(ah, "%s: timeout on reg 0x%x: 0x%08x & 0x%08x != 0x%08x\n",
		__func__, reg, OS_REG_READ(ah, reg), mask, val);
	return AH_FALSE;
#undef AH_TIMEOUT
}

/*
 * Reverse the bits starting at the low bit for a value of
 * bit_count in size
 */
u_int32_t
ath_hal_reverseBits(u_int32_t val, u_int32_t n)
{
	u_int32_t retval;
	int i;

	for (i = 0, retval = 0; i < n; i++) {
		retval = (retval << 1) | (val & 1);
		val >>= 1;
	}
	return retval;
}

/*
 * Compute the time to transmit a frame of length frameLen bytes
 * using the specified rate, phy, and short preamble setting.
 */
u_int16_t
ath_hal_computetxtime(struct ath_hal *ah,
	const HAL_RATE_TABLE *rates, u_int32_t frameLen, u_int16_t rateix,
	HAL_BOOL shortPreamble)
{
	u_int32_t bitsPerSymbol, numBits, numSymbols, phyTime, txTime;
	u_int32_t kbps;

	kbps = rates->info[rateix].rateKbps;
	/*
	 * index can be invalid duting dynamic Turbo transitions. 
	 */
	if(kbps == 0) return 0;
	switch (rates->info[rateix].phy) {

	case IEEE80211_T_CCK:
#define CCK_SIFS_TIME        10
#define CCK_PREAMBLE_BITS   144
#define CCK_PLCP_BITS        48
		phyTime		= CCK_PREAMBLE_BITS + CCK_PLCP_BITS;
		if (shortPreamble && rates->info[rateix].shortPreamble)
			phyTime >>= 1;
		numBits		= frameLen << 3;
		txTime		= CCK_SIFS_TIME + phyTime
				+ ((numBits * 1000)/kbps);
		break;
#undef CCK_SIFS_TIME
#undef CCK_PREAMBLE_BITS
#undef CCK_PLCP_BITS

	case IEEE80211_T_OFDM:
#define OFDM_SIFS_TIME        16
#define OFDM_PREAMBLE_TIME    20
#define OFDM_PLCP_BITS        22
#define OFDM_SYMBOL_TIME       4

#define OFDM_SIFS_TIME_HALF	32
#define OFDM_PREAMBLE_TIME_HALF	40
#define OFDM_PLCP_BITS_HALF	22
#define OFDM_SYMBOL_TIME_HALF	8

#define OFDM_SIFS_TIME_QUARTER 		64
#define OFDM_PREAMBLE_TIME_QUARTER	80
#define OFDM_PLCP_BITS_QUARTER		22
#define OFDM_SYMBOL_TIME_QUARTER	16

#define OFDM_SIFS_TIME_SUBQUARTER 		128
#define OFDM_PREAMBLE_TIME_SUBQUARTER	160
#define OFDM_PLCP_BITS_SUBQUARTER		22
#define OFDM_SYMBOL_TIME_SUBQUARTER	32

		if (AH_PRIVATE(ah)->ah_curchan && 
			IS_CHAN_SUBQUARTER_RATE(AH_PRIVATE(ah)->ah_curchan)) {
			bitsPerSymbol	= (kbps * OFDM_SYMBOL_TIME_SUBQUARTER) / 1000;
			HALASSERT(bitsPerSymbol != 0);

			numBits		= OFDM_PLCP_BITS + (frameLen << 3);
			numSymbols	= howmany(numBits, bitsPerSymbol);
			txTime		= OFDM_SIFS_TIME_SUBQUARTER 
						+ OFDM_PREAMBLE_TIME_SUBQUARTER
					+ (numSymbols * OFDM_SYMBOL_TIME_SUBQUARTER);
		} else if (AH_PRIVATE(ah)->ah_curchan && 
			IS_CHAN_QUARTER_RATE(AH_PRIVATE(ah)->ah_curchan)) {
			bitsPerSymbol	= (kbps * OFDM_SYMBOL_TIME_QUARTER) / 1000;
			HALASSERT(bitsPerSymbol != 0);

			numBits		= OFDM_PLCP_BITS + (frameLen << 3);
			numSymbols	= howmany(numBits, bitsPerSymbol);
			txTime		= OFDM_SIFS_TIME_QUARTER 
						+ OFDM_PREAMBLE_TIME_QUARTER
					+ (numSymbols * OFDM_SYMBOL_TIME_QUARTER);
		} else if (AH_PRIVATE(ah)->ah_curchan &&
				IS_CHAN_HALF_RATE(AH_PRIVATE(ah)->ah_curchan)) {
			bitsPerSymbol	= (kbps * OFDM_SYMBOL_TIME_HALF) / 1000;
			HALASSERT(bitsPerSymbol != 0);

			numBits		= OFDM_PLCP_BITS + (frameLen << 3);
			numSymbols	= howmany(numBits, bitsPerSymbol);
			txTime		= OFDM_SIFS_TIME_HALF + 
						OFDM_PREAMBLE_TIME_HALF
					+ (numSymbols * OFDM_SYMBOL_TIME_HALF);
		} else { /* full rate channel */
			bitsPerSymbol	= (kbps * OFDM_SYMBOL_TIME) / 1000;
			HALASSERT(bitsPerSymbol != 0);

			numBits		= OFDM_PLCP_BITS + (frameLen << 3);
			numSymbols	= howmany(numBits, bitsPerSymbol);
			txTime		= OFDM_SIFS_TIME + OFDM_PREAMBLE_TIME
					+ (numSymbols * OFDM_SYMBOL_TIME);
		}
		break;

#undef OFDM_SIFS_TIME
#undef OFDM_PREAMBLE_TIME
#undef OFDM_PLCP_BITS
#undef OFDM_SYMBOL_TIME

	case IEEE80211_T_TURBO:
#define TURBO_SIFS_TIME         8
#define TURBO_PREAMBLE_TIME    14
#define TURBO_PLCP_BITS        22
#define TURBO_SYMBOL_TIME       4
		/* we still save OFDM rates in kbps - so double them */
		bitsPerSymbol = (kbps * TURBO_SYMBOL_TIME) / 1000;
		HALASSERT(bitsPerSymbol != 0);

		numBits       = TURBO_PLCP_BITS + (frameLen << 3);
		numSymbols    = howmany(numBits, bitsPerSymbol);
		txTime        = TURBO_SIFS_TIME + TURBO_PREAMBLE_TIME
			      + (numSymbols * TURBO_SYMBOL_TIME);
		break;
#undef TURBO_SIFS_TIME
#undef TURBO_PREAMBLE_TIME
#undef TURBO_PLCP_BITS
#undef TURBO_SYMBOL_TIME

	case ATHEROS_T_XR:
#define XR_SIFS_TIME            16
#define XR_PREAMBLE_TIME(_kpbs) (((_kpbs) < 1000) ? 173 : 76)
#define XR_PLCP_BITS            22
#define XR_SYMBOL_TIME           4
		bitsPerSymbol = (kbps * XR_SYMBOL_TIME) / 1000;
		HALASSERT(bitsPerSymbol != 0);

		numBits       = XR_PLCP_BITS + (frameLen << 3);
		numSymbols    = howmany(numBits, bitsPerSymbol);
		txTime        = XR_SIFS_TIME + XR_PREAMBLE_TIME(kbps)
			       + (numSymbols * XR_SYMBOL_TIME);
		break;
#undef XR_SIFS_TIME
#undef XR_PREAMBLE_TIME
#undef XR_PLCP_BITS
#undef XR_SYMBOL_TIME

	default:
		HALDEBUG(ah, "%s: unknown phy %u (rate ix %u)\n",
			__func__, rates->info[rateix].phy, rateix);
		txTime = 0;
		break;
	}
	return txTime;
}

WIRELESS_MODE
ath_hal_chan2wmode(struct ath_hal *ah, const HAL_CHANNEL *chan)
{
	if (IS_CHAN_CCK(chan))
		return WIRELESS_MODE_11b;
	if (IS_CHAN_G(chan))
		return WIRELESS_MODE_11g;
	if (IS_CHAN_108G(chan))
		return WIRELESS_MODE_108g;
	if (IS_CHAN_TURBO(chan))
		return WIRELESS_MODE_TURBO;
	if (IS_CHAN_XR(chan))
		return WIRELESS_MODE_XR;
	return WIRELESS_MODE_11a;
}

static __inline int
mapgsm(u_int freq, u_int flags)
{
	freq *= 10;
	if (flags & CHANNEL_QUARTER)
		freq += 5;
	else if (flags & CHANNEL_HALF)
		freq += 10;
	else
		freq += 20;
	return (freq - 24220) / 5;
}

static __inline int
mappsb(u_int freq, u_int flags)
{
	return ((freq * 10) + (((freq % 5) == 2) ? 5 : 0) - 49400) / 5;
}


/*
 * Convert GHz frequency to IEEE channel number.
 */
int
ath_hal_mhz2ieee(struct ath_hal *ah, u_int freq, u_int flags)
{
	if ((flags & CHANNEL_2GHZ) ||
		(!(flags & CHANNEL_5GHZ) && (freq < 3000)))
	{	/* 2GHz band */
		if (freq == 2484)
			return 14;
		if (freq < 2502 && freq > 2484 )
			return 14;
		if (freq < 2512 && freq > 2484)
			return 15;
		
		if (freq < 2484) {
			int chan;
			if (ath_hal_isgsmsku(ah))
				return mapgsm(freq, flags);
			chan = ((int)freq - 2407) / 5;
			if (chan < 0)
				chan += 256;
			return chan;
		} else {
			return 15 + ((freq - 2512) / 20);
		}
	} else {/* 5Ghz band */
		if (ath_hal_ispublicsafetysku(ah) &&
		    IS_CHAN_IN_PUBLIC_SAFETY_BAND(freq)) {
			return mappsb(freq, flags);
		} else if ((flags & CHANNEL_A) && (freq <= 5000)) {
			return (freq - 4000) / 5;
		} else {
			return (freq - 5000) / 5;
		}
	}
}

/*
 * Convert between microseconds and core system clocks.
 */
                                     /* 11a Turbo  11b  11g  108g  XR */
static const u_int8_t CLOCK_RATE[]  = { 40,  80,   22,  44,   88,  40 };

u_int
ath_hal_mac_clks(struct ath_hal *ah, u_int usecs)
{
	const HAL_CHANNEL *c = (const HAL_CHANNEL *) AH_PRIVATE(ah)->ah_curchan;
	u_int clks;

	/* NB: ah_curchan may be null when called attach time */
	if (c != AH_NULL) {
		clks = usecs * CLOCK_RATE[ath_hal_chan2wmode(ah, c)];
		if (IS_CHAN_HT40(c))
			clks <<= 1;
	} else
		clks = usecs * CLOCK_RATE[WIRELESS_MODE_11b];
	return clks;
}

u_int
ath_hal_mac_usec(struct ath_hal *ah, u_int clks)
{
	const HAL_CHANNEL *c = (const HAL_CHANNEL *) AH_PRIVATE(ah)->ah_curchan;
	u_int usec;

	/* NB: ah_curchan may be null when called attach time */
	if (c != AH_NULL) {
		usec = clks / CLOCK_RATE[ath_hal_chan2wmode(ah, c)];
		if (IS_CHAN_HT40(c))
			usec >>= 1;
	} else
		usec = clks / CLOCK_RATE[WIRELESS_MODE_11b];
	return usec;
}

/*
 * Setup a h/w rate table's reverse lookup table and
 * fill in ack durations.  This routine is called for
 * each rate table returned through the ah_getRateTable
 * method.  The reverse lookup tables are assumed to be
 * initialized to zero (or at least the first entry).
 * We use this as a key that indicates whether or not
 * we've previously setup the reverse lookup table.
 *
 * XXX not reentrant, but shouldn't matter
 */
void
ath_hal_setupratetable(struct ath_hal *ah, HAL_RATE_TABLE *rt)
{
#define	N(a)	(sizeof(a)/sizeof(a[0]))
	int i;

	if (rt->rateCodeToIndex[0] != 0)	/* already setup */
		return;
	for (i = 0; i < N(rt->rateCodeToIndex); i++)
		rt->rateCodeToIndex[i] = (u_int8_t) -1;
	for (i = 0; i < rt->rateCount; i++) {
		u_int8_t code = rt->info[i].rateCode;
		u_int8_t cix = rt->info[i].controlRate;

		HALASSERT(code < N(rt->rateCodeToIndex));
		rt->rateCodeToIndex[code] = i;
		HALASSERT((code | rt->info[i].shortPreamble) <
		    N(rt->rateCodeToIndex));
		rt->rateCodeToIndex[code | rt->info[i].shortPreamble] = i;
		/*
		 * XXX for 11g the control rate to use for 5.5 and 11 Mb/s
		 *     depends on whether they are marked as basic rates;
		 *     the static tables are setup with an 11b-compatible
		 *     2Mb/s rate which will work but is suboptimal
		 */
		rt->info[i].lpAckDuration = ath_hal_computetxtime(ah, rt,
			WLAN_CTRL_FRAME_SIZE, cix, AH_FALSE);
		rt->info[i].spAckDuration = ath_hal_computetxtime(ah, rt,
			WLAN_CTRL_FRAME_SIZE, cix, AH_TRUE);
	}
#undef N
}

HAL_STATUS
ath_hal_getcapability(struct ath_hal *ah, HAL_CAPABILITY_TYPE type,
	u_int32_t capability, u_int32_t *result)
{
	const HAL_CAPABILITIES *pCap = &AH_PRIVATE(ah)->ah_caps;

	switch (type) {
	case HAL_CAP_VENDOR:
		*result = AH_PRIVATE(ah)->ah_subvendorid;
		return HAL_OK;
	case HAL_CAP_PRODUCT:
		*result = AH_PRIVATE(ah)->ah_subsystemid;
		return HAL_OK;
	case HAL_CAP_REG_DMN:		/* regulatory domain */
		*result = AH_PRIVATE(ah)->ah_currentRD;
		return HAL_OK;
	case HAL_CAP_CIPHER:		/* cipher handled in hardware */
	case HAL_CAP_TKIP_MIC:		/* handle TKIP MIC in hardware */
		return HAL_ENOTSUPP;
	case HAL_CAP_TKIP_SPLIT:	/* hardware TKIP uses split keys */
		return HAL_ENOTSUPP;
	case HAL_CAP_PHYCOUNTERS:	/* hardware PHY error counters */
		return pCap->halHwPhyCounterSupport ? HAL_OK : HAL_ENXIO;
	case HAL_CAP_WME_TKIPMIC:   /* hardware can do TKIP MIC when WMM is turned on */
		return HAL_ENOTSUPP;
	case HAL_CAP_DIVERSITY:		/* hardware supports fast diversity */
		return HAL_ENOTSUPP;
	case HAL_CAP_KEYCACHE_SIZE:	/* hardware key cache size */
		*result =  pCap->halKeyCacheSize;
		return HAL_OK;
	case HAL_CAP_NUM_TXQUEUES:	/* number of hardware tx queues */
		*result = pCap->halTotalQueues;
		return HAL_OK;
	case HAL_CAP_VEOL:		/* hardware supports virtual EOL */
		return pCap->halVEOLSupport ? HAL_OK : HAL_ENOTSUPP;
	case HAL_CAP_PSPOLL:		/* hardware PS-Poll support works */
		return pCap->halPSPollBroken ? HAL_ENOTSUPP : HAL_OK;
	case HAL_CAP_COMPRESSION:
		return pCap->halCompressSupport ? HAL_OK : HAL_ENOTSUPP;
	case HAL_CAP_BURST:
		return pCap->halBurstSupport ? HAL_OK : HAL_ENOTSUPP;
	case HAL_CAP_FASTFRAME:
		return pCap->halFastFramesSupport ? HAL_OK : HAL_ENOTSUPP;
	case HAL_CAP_DIAG:		/* hardware diagnostic support */
		*result = AH_PRIVATE(ah)->ah_diagreg;
		return HAL_OK;
	case HAL_CAP_TXPOW:		/* global tx power limit  */
		switch (capability) {
		case 0:			/* facility is supported */
			return HAL_OK;
		case 1:			/* current limit */
        		/* the high 16 bits are used for extra txpower */
        		*result = (AH_PRIVATE(ah)->ah_extra_txpow << 16) + AH_PRIVATE(ah)->ah_powerLimit;
			return HAL_OK;
		case 2:			/* current max tx power */
			*result = AH_PRIVATE(ah)->ah_maxPowerLevel;
			return HAL_OK;
		case 3:			/* scale factor */
			*result = AH_PRIVATE(ah)->ah_tpScale;
			return HAL_OK;
		}
		return HAL_ENOTSUPP;
	case HAL_CAP_BSSIDMASK:		/* hardware supports bssid mask */
		return pCap->halBssIdMaskSupport ? HAL_OK : HAL_ENOTSUPP;
	case HAL_CAP_MCAST_KEYSRCH:	/* multicast frame keycache search */
		return pCap->halMcastKeySrchSupport ? HAL_OK : HAL_ENOTSUPP;
	case HAL_CAP_TSF_ADJUST:	/* hardware has beacon tsf adjust */
		return HAL_ENOTSUPP;
	case HAL_CAP_XR:
		return pCap->halXrSupport ? HAL_OK : HAL_ENOTSUPP;
	case HAL_CAP_CHAN_HALFRATE:
		return pCap->halChanHalfRate ? HAL_OK : HAL_ENOTSUPP;
	case HAL_CAP_CHAN_QUARTERRATE:
		return pCap->halChanQuarterRate ? HAL_OK : HAL_ENOTSUPP;
	case HAL_CAP_CHANBW:
		*result = (u_int32_t) AH_PRIVATE(ah)->ah_chanbw;
		return HAL_OK;
#ifdef AH_SUPPORT_SC
	case HAL_CAP_CHANSHIFT:
		*result = (u_int32_t) AH_PRIVATE(ah)->ah_chanshift;
		return HAL_OK;
	case HAL_CAP_SUPERCH:
		*result = (u_int32_t) AH_PRIVATE(ah)->ah_useSC;
		return HAL_OK;
#endif
	case HAL_CAP_POWERFIX:
		if (!AH_PRIVATE(ah)->ah_refreshCalibration)
			return HAL_ENOTSUPP;
		*result = AH_PRIVATE(ah)->ah_powerfix;
		return HAL_OK;
	case HAL_CAP_ANTGAIN:
		*result = AH_PRIVATE(ah)->ah_antGain;
		return HAL_OK;
	case HAL_CAP_ANTGAINSUB:
		*result = AH_PRIVATE(ah)->ah_antGainsub;
		return HAL_OK;
	case HAL_CAP_RFSILENT:		/* rfsilent support  */
		switch (capability) {
		case 0:			/* facility is supported */
			return pCap->halRfSilentSupport ? HAL_OK : HAL_ENOTSUPP;
		case 1:			/* current setting */
			return AH_PRIVATE(ah)->ah_rfkillEnabled ?
				HAL_OK : HAL_ENOTSUPP;
		case 2:			/* rfsilent config */
			*result = AH_PRIVATE(ah)->ah_rfsilent;
			return HAL_OK;
		}
		return HAL_ENOTSUPP;
	case HAL_CAP_11D:
#ifdef AH_SUPPORT_11D
		return HAL_OK;
#else
		return HAL_ENOTSUPP;
#endif
	case HAL_CAP_RXORN_FATAL:	/* HAL_INT_RXORN treated as fatal  */
		return AH_PRIVATE(ah)->ah_rxornIsFatal ? HAL_OK : HAL_ENOTSUPP;
	case HAL_CAP_HT:
		return pCap->halHTSupport ? HAL_OK : HAL_ENOTSUPP;
	case HAL_CAP_NUMTXCHAIN:	/* # TX chains supported */
		*result = AH_PRIVATE(ah)->ah_maxNumTxChain;
		return HAL_OK;
	case HAL_CAP_NUMRXCHAIN:	/* # RX chains supported */
		*result = AH_PRIVATE(ah)->ah_maxNumRxChain;
		return HAL_OK;
	case HAL_CAP_RXTSTAMP_PREC:	/* rx desc tstamp precision (bits) */
		*result = pCap->halTstampPrecision;
		return HAL_OK;
	default:
		return HAL_EINVAL;
	}
}

HAL_BOOL
ath_hal_setcapability(struct ath_hal *ah, HAL_CAPABILITY_TYPE type,
	u_int32_t capability, u_int32_t setting, HAL_STATUS *status)
{

	switch (type) {
	case HAL_CAP_TXPOW:
		switch (capability) {
		case 3:
			if (setting <= HAL_TP_SCALE_MIN) {
				AH_PRIVATE(ah)->ah_tpScale = setting;
				return AH_TRUE;
			}
			break;
		}
		break;
	case HAL_CAP_CHANBW:
		switch (setting) {
			case 0:
#ifdef AH_SUPPORT_CHANBW
			case 2:
			case 5:
			case 10:
#endif
			case 20:
			case 40:
				AH_PRIVATE(ah)->ah_chanbw = setting;
				return AH_TRUE;
			default:
				return AH_FALSE;
		}
		break;
#ifdef AH_SUPPORT_SC
	case HAL_CAP_CHANSHIFT:
		switch (setting) {
			case 15:
			case -15:
				if (AH_PRIVATE(ah)->ah_chanbw > 5)
				    return AH_FALSE;
			case 0:
			case 5:
			case 10:
			case -5:
			case -10:
				if (AH_PRIVATE(ah)->ah_chanbw > 10)
				    return AH_FALSE;
				AH_PRIVATE(ah)->ah_chanshift = setting;
				return AH_TRUE;
			default:
				return AH_FALSE;
		}
		break;
#endif
	case HAL_CAP_POWERFIX:
		if (!AH_PRIVATE(ah)->ah_refreshCalibration)
			return AH_FALSE;
		AH_PRIVATE(ah)->ah_powerfix = setting;
		AH_PRIVATE(ah)->ah_refreshCalibration(ah);
		return AH_TRUE;
	case HAL_CAP_ANTGAIN:
		AH_PRIVATE(ah)->ah_antGain = (int)setting;
		return AH_TRUE;
	case HAL_CAP_ANTGAINSUB:
		AH_PRIVATE(ah)->ah_antGainsub = (int)setting;
		return AH_TRUE;
	case HAL_CAP_RFSILENT:		/* rfsilent support  */
		/*
		 * NB: allow even if halRfSilentSupport is false
		 *     in case the EEPROM is misprogrammed.
		 */
		switch (capability) {
		case 1:			/* current setting */
			AH_PRIVATE(ah)->ah_rfkillEnabled = (setting != 0);
			return AH_TRUE;
		case 2:			/* rfsilent config */
			/* XXX better done per-chip for validation? */
			AH_PRIVATE(ah)->ah_rfsilent = setting;
			return AH_TRUE;
		}
		break;
	case HAL_CAP_REG_DMN:		/* regulatory domain */
		AH_PRIVATE(ah)->ah_currentRD = setting;
		return AH_TRUE;
#ifdef AH_SUPPORT_SC
	case HAL_CAP_SUPERCH:
		AH_PRIVATE(ah)->ah_useSC = setting;
		if (setting && !AH_PRIVATE(ah)->ah_chanbw)
			AH_PRIVATE(ah)->ah_chanbw = 20;
		return AH_TRUE;
#endif
	case HAL_CAP_RXORN_FATAL:	/* HAL_INT_RXORN treated as fatal  */
		AH_PRIVATE(ah)->ah_rxornIsFatal = setting;
		return AH_TRUE;
	default:
		break;
	}
	if (status)
		*status = HAL_EINVAL;
	return AH_FALSE;
}

/* 
 * Common support for getDiagState method.
 */

static u_int
ath_hal_getregdump(struct ath_hal *ah, const HAL_REGRANGE *regs,
	void *dstbuf, int space)
{
	u_int32_t *dp = dstbuf;
	int i;

	for (i = 0; space >= 2*sizeof(u_int32_t); i++) {
		u_int r = regs[i].start;
		u_int e = regs[i].end;
		*dp++ = (r<<16) | e;
		space -= sizeof(u_int32_t);
		do {
			*dp++ = OS_REG_READ(ah, r);
			r += sizeof(u_int32_t);
			space -= sizeof(u_int32_t);
		} while (r <= e && space >= sizeof(u_int32_t));
	}
	return (char *) dp - (char *) dstbuf;
}

HAL_BOOL
ath_hal_getdiagstate(struct ath_hal *ah, int request,
	const void *args, u_int32_t argsize,
	void **result, u_int32_t *resultsize)
{
	switch (request) {
	case HAL_DIAG_REVS:
		*result = &AH_PRIVATE(ah)->ah_devid;
		*resultsize = sizeof(HAL_REVS);
		return AH_TRUE;
	case HAL_DIAG_REGS:
		*resultsize = ath_hal_getregdump(ah, args, *result,*resultsize);
		return AH_TRUE;
	case HAL_DIAG_FATALERR:
		*result = &AH_PRIVATE(ah)->ah_fatalState[0];
		*resultsize = sizeof(AH_PRIVATE(ah)->ah_fatalState);
		return AH_TRUE;
#ifdef AH_SUPPORT_WRITE_EEPROM
	case HAL_DIAG_EEREAD:
		if (argsize != sizeof(u_int16_t))
			{
			return AH_FALSE;
			}
		if (result==0L)
			{
			return AH_FALSE;
			}
		if (*result==0L)
			{
			return AH_FALSE;
			}
		if (!ath_hal_eepromRead(ah, *(const u_int16_t *)args, *result))
			{
			return AH_FALSE;
			}
		*resultsize = sizeof(u_int16_t);
		return AH_TRUE;
	case HAL_DIAG_EEWRITE: {
		const HAL_DIAG_EEVAL *ee;
		if (argsize != sizeof(HAL_DIAG_EEVAL))
			return AH_FALSE;
		ee = (const HAL_DIAG_EEVAL *)args;
		return ath_hal_eepromWrite(ah, ee->ee_off, ee->ee_data);
	}
#endif /* AH_SUPPORT_WRITE_EEPROM */
#ifdef AH_PRIVATE_DIAG
	case HAL_DIAG_SETKEY: {
		const HAL_DIAG_KEYVAL *dk;

		if (argsize != sizeof(HAL_DIAG_KEYVAL))
			return AH_FALSE;
		dk = (const HAL_DIAG_KEYVAL *)args;
		return ah->ah_setKeyCacheEntry(ah, dk->dk_keyix,
			&dk->dk_keyval, dk->dk_mac, dk->dk_xor);
	}
	case HAL_DIAG_RESETKEY:
		if (argsize != sizeof(u_int16_t))
			return AH_FALSE;
		return ah->ah_resetKeyCacheEntry(ah, *(const u_int16_t *)args);
	/* XXX this duplicates HAL_DIAG_REGS */
	case HAL_DIAG_REGREAD: {
		if (argsize != sizeof(u_int))
			return AH_FALSE;
		**(u_int32_t **)result = OS_REG_READ(ah, *(const u_int *)args);
		*resultsize = sizeof(u_int32_t);
		return AH_TRUE;	   
	   }
#if 0
	/* XXX this is in struct ath_hal, why do a call? */
        case HAL_DIAG_GET_REGBASE: {
		/* Should be HAL_BUS_HANDLE but compiler warns and hal build 
		   set to treat warnings as errors. */
		**(u_int **)result =(u_int )ah->ah_sh;
		*resultsize = sizeof(HAL_BUS_HANDLE);
		return AH_TRUE;
	}
#endif
#ifdef AH_SUPPORT_WRITE_REG
	case HAL_DIAG_REGWRITE: {
		const HAL_DIAG_REGVAL *rv;
		if (argsize != sizeof(HAL_DIAG_REGVAL))
			return AH_FALSE;
		rv = (const HAL_DIAG_REGVAL *)args;
		OS_REG_WRITE(ah, rv->offset, rv->val);
		return AH_TRUE;	  
	}
#endif /* AH_SUPPORT_WRITE_REG */
	case HAL_DIAG_RDWRITE: {
		HAL_STATUS status;
		const HAL_REG_DOMAIN *rd;

		if (argsize != sizeof(HAL_REG_DOMAIN))
			return AH_FALSE;
		rd = (const u_int16_t *) args;
		if (!ath_hal_setcapability(ah, HAL_CAP_REG_DMN, 0, *rd, &status))
			return AH_FALSE;
		return AH_TRUE;
	}
	case HAL_DIAG_RDREAD: {
		u_int32_t rd;
		if (ath_hal_getcapability(ah, HAL_CAP_REG_DMN, 0, &rd) != HAL_OK)
			return AH_FALSE;
		*resultsize = sizeof(u_int16_t);
		*((HAL_REG_DOMAIN *) (*result)) = (u_int16_t) rd;
		return AH_TRUE;
	}
#endif /* AH_PRIVATE_DIAG */
	case HAL_DIAG_11NCOMPAT:
		if (argsize == 0) {
			*resultsize = sizeof(u_int32_t);
			*((u_int32_t *)(*result)) =
				AH_PRIVATE(ah)->ah_11nCompat;
		} else if (argsize == sizeof(u_int32_t)) {
			AH_PRIVATE(ah)->ah_11nCompat = *(const u_int32_t *)args;
		} else
			return AH_FALSE;
		return AH_TRUE;
	}
	return AH_FALSE;
}

/*
 * Set the properties of the tx queue with the parameters
 * from qInfo.
 */
HAL_BOOL
ath_hal_setTxQProps(struct ath_hal *ah,
	HAL_TX_QUEUE_INFO *qi, const HAL_TXQ_INFO *qInfo)
{
	u_int32_t cw;

	if (qi->tqi_type == HAL_TX_QUEUE_INACTIVE) {
		HALDEBUG(ah, "%s: inactive queue\n", __func__);
		return AH_FALSE;
	}

	HALDEBUG(ah, "%s: queue %p\n", __func__, qi);

	/* XXX validate parameters */
	qi->tqi_ver = qInfo->tqi_ver;
	qi->tqi_subtype = qInfo->tqi_subtype;
	qi->tqi_qflags = qInfo->tqi_qflags;
	qi->tqi_priority = qInfo->tqi_priority;
	if (qInfo->tqi_aifs != HAL_TXQ_USEDEFAULT)
		qi->tqi_aifs = AH_MIN(qInfo->tqi_aifs, 255);
	else
		qi->tqi_aifs = INIT_AIFS;
	if (qInfo->tqi_cwmin != HAL_TXQ_USEDEFAULT) {
		cw = AH_MIN(qInfo->tqi_cwmin, 1024);
		/* make sure that the CWmin is of the form (2^n - 1) */
		qi->tqi_cwmin = 1;
		while (qi->tqi_cwmin < cw)
			qi->tqi_cwmin = (qi->tqi_cwmin << 1) | 1;
	} else
		qi->tqi_cwmin = qInfo->tqi_cwmin;
	if (qInfo->tqi_cwmax != HAL_TXQ_USEDEFAULT) {
		cw = AH_MIN(qInfo->tqi_cwmax, 1024);
		/* make sure that the CWmax is of the form (2^n - 1) */
		qi->tqi_cwmax = 1;
		while (qi->tqi_cwmax < cw)
			qi->tqi_cwmax = (qi->tqi_cwmax << 1) | 1;
	} else
		qi->tqi_cwmax = INIT_CWMAX;
	/* Set retry limit values */
	if (qInfo->tqi_shretry != 0)
		qi->tqi_shretry = AH_MIN(qInfo->tqi_shretry, 15);
	else
		qi->tqi_shretry = INIT_SH_RETRY;
	if (qInfo->tqi_lgretry != 0)
		qi->tqi_lgretry = AH_MIN(qInfo->tqi_lgretry, 15);
	else
		qi->tqi_lgretry = INIT_LG_RETRY;
	qi->tqi_cbrPeriod = qInfo->tqi_cbrPeriod;
	qi->tqi_cbrOverflowLimit = qInfo->tqi_cbrOverflowLimit;
	qi->tqi_burstTime = qInfo->tqi_burstTime;
	qi->tqi_readyTime = qInfo->tqi_readyTime;

	switch (qInfo->tqi_subtype) {
	case HAL_WME_UPSD:
		if (qi->tqi_type == HAL_TX_QUEUE_DATA)
			qi->tqi_intFlags = HAL_TXQ_USE_LOCKOUT_BKOFF_DIS;
		break;
	default:
		break;		/* NB: silence compiler */
	}
	return AH_TRUE;
}

HAL_BOOL
ath_hal_getTxQProps(struct ath_hal *ah,
	HAL_TXQ_INFO *qInfo, const HAL_TX_QUEUE_INFO *qi)
{
	if (qi->tqi_type == HAL_TX_QUEUE_INACTIVE) {
		HALDEBUG(ah, "%s: inactive queue\n", __func__);
		return AH_FALSE;
	}

	qInfo->tqi_qflags = qi->tqi_qflags;
	qInfo->tqi_ver = qi->tqi_ver;
	qInfo->tqi_subtype = qi->tqi_subtype;
	qInfo->tqi_qflags = qi->tqi_qflags;
	qInfo->tqi_priority = qi->tqi_priority;
	qInfo->tqi_aifs = qi->tqi_aifs;
	qInfo->tqi_cwmin = qi->tqi_cwmin;
	qInfo->tqi_cwmax = qi->tqi_cwmax;
	qInfo->tqi_shretry = qi->tqi_shretry;
	qInfo->tqi_lgretry = qi->tqi_lgretry;
	qInfo->tqi_cbrPeriod = qi->tqi_cbrPeriod;
	qInfo->tqi_cbrOverflowLimit = qi->tqi_cbrOverflowLimit;
	qInfo->tqi_burstTime = qi->tqi_burstTime;
	qInfo->tqi_readyTime = qi->tqi_readyTime;
	return AH_TRUE;
}

                                     /* 11a Turbo  11b  11g  108g  XR */
static const int16_t NOISE_FLOOR[] = { -96, -93,  -98, -96,  -93, -96 };

/*
 * Read the current channel noise floor and return.
 * If nf cal hasn't finished, channel noise floor should be 0
 * and we return a nominal value based on band and frequency.
 *
 * NB: This is a private routine used by per-chip code to
 *     implement the ah_getChanNoise method.
 */
int16_t
ath_hal_getChanNoise(struct ath_hal *ah, HAL_CHANNEL *chan)
{
	HAL_CHANNEL_INTERNAL *ichan;

	ichan = ath_hal_checkchannel(ah, chan);
	if (ichan == AH_NULL) {
		HALDEBUG(ah, "%s: invalid channel %u/0x%x; no mapping\n",
			__func__, chan->channel, chan->channelFlags);
		return 0;
	}
	if (ichan->rawNoiseFloor == 0) {
		WIRELESS_MODE mode = ath_hal_chan2wmode(ah, chan);

		HALASSERT(mode < WIRELESS_MODE_MAX);
		return NOISE_FLOOR[mode] + ath_hal_getNfAdjust(ah, ichan);
	} else
		return ichan->rawNoiseFloor + ichan->noiseFloorAdjust;
}

HAL_BOOL ath_hal_dummy(struct ath_hal *ah, ...)
{
	return AH_FALSE;
}

/*
 * Process all valid raw noise floors into the dBm noise floor values.
 * Though our device has no reference for a dBm noise floor, we perform
 * a relative minimization of NF's based on the lowest NF found across a
 * channel scan.
 */
void
ath_hal_process_noisefloor(struct ath_hal *ah)
{
	HAL_CHANNEL_INTERNAL *c;
	int16_t correct2, correct5;
	int16_t lowest2, lowest5;
	int i;

	/* 
	 * Find the lowest 2GHz and 5GHz noise floor values after adjusting
	 * for statistically recorded NF/channel deviation.
	 */
	correct2 = lowest2 = 0;
	correct5 = lowest5 = 0;
	for (i = 0; i < AH_PRIVATE(ah)->ah_nchan; i++) {
		WIRELESS_MODE mode;
		int16_t nf;

		c = &AH_PRIVATE(ah)->ah_memchannels[i];
		if (c->rawNoiseFloor >= 0)
			continue;
		mode = ath_hal_chan2wmode(ah, (HAL_CHANNEL *) c);
		HALASSERT(mode < WIRELESS_MODE_MAX);
		nf = c->rawNoiseFloor + NOISE_FLOOR[mode] +
			ath_hal_getNfAdjust(ah, c);
		if (IS_CHAN_5GHZ(c)) {
			if (nf < lowest5) { 
				lowest5 = nf;
				correct5 = NOISE_FLOOR[mode] -
				    (c->rawNoiseFloor + ath_hal_getNfAdjust(ah, c));
			}
		} else {
			if (nf < lowest2) { 
				lowest2 = nf;
				correct2 = NOISE_FLOOR[mode] -
				    (c->rawNoiseFloor + ath_hal_getNfAdjust(ah, c));
			}
		}
	}

	/* Correct the channels to reach the expected NF value */
	for (i = 0; i < AH_PRIVATE(ah)->ah_nchan; i++) {
		c = &AH_PRIVATE(ah)->ah_memchannels[i];
		if (c->rawNoiseFloor >= 0)
			continue;
		/* Apply correction factor */
		c->noiseFloorAdjust = ath_hal_getNfAdjust(ah, c) +
			(IS_CHAN_5GHZ(c) ? correct5 : correct2);
		HALDEBUGn(ah, 8, "%u/0x%x raw nf %d adjust %d\n",
			c->channel, c->channelFlags,
			c->rawNoiseFloor, c->noiseFloorAdjust);
	}
}

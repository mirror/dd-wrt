/*-
 * Copyright (c) 2007 Michael Taylor
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer,
	without modification.
 * 2. Redistributions in binary form must reproduce at minimum a disclaimer
 *    similar to the "NO WARRANTY" disclaimer below ("Disclaimer") and any
 *    redistribution must be conditioned upon including a substantially
 *    similar Disclaimer requirement for further binary redistribution.
 * 3. Neither the names of the above-listed copyright holders nor the names
 *    of any contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 *
 * NO WARRANTY
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF NONINFRINGEMENT, MERCHANTIBILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES.
 *
 * $Id: foo mtaylor $
 */

/* This file provides some wrapper functions that invoke functions in 
 * if_ath_hal.h.  Since all the functions in the generated file, if_ath_hal.h
 * have locks to protect them... no further locking is required in these 
 * additional helper functions.  Mostly these just provide a series of nicknames
 * for specific sets of arguments to HAL functions that are commonly needed. */

#ifndef _IF_ATH_HAL_WRAPPERS_H_
#define _IF_ATH_HAL_WRAPPERS_H_

static inline void ath_reg_write(struct ath_softc *sc, u_int reg, u_int32_t val)
{
	ATH_HAL_LOCK_IRQ(sc);
	OS_REG_WRITE(sc->sc_ah, reg, val);
	ATH_HAL_UNLOCK_IRQ(sc);
}

static inline u_int32_t ath_reg_read(struct ath_softc *sc, u_int reg)
{
	u_int32_t ret;
	ATH_HAL_LOCK_IRQ(sc);
	ret = OS_REG_READ(sc->sc_ah, reg);
	ATH_HAL_UNLOCK_IRQ(sc);
	return ret;
}

static inline HAL_BOOL ath_hal_burstsupported(struct ath_hal *ah)
{
	return (ath_hal_getcapability(ah, HAL_CAP_BURST, 0, NULL) == HAL_OK);
}

static inline HAL_BOOL ath_hal_ciphersupported(struct ath_hal *ah, u_int32_t cipher)
{
	return (ath_hal_getcapability(ah, HAL_CAP_CIPHER, cipher, NULL) == HAL_OK);
}

static inline HAL_BOOL ath_hal_compressionsupported(struct ath_hal *ah)
{
	return (ath_hal_getcapability(ah, HAL_CAP_COMPRESSION, 0, NULL) == HAL_OK);
}

static inline HAL_BOOL ath_hal_fastframesupported(struct ath_hal *ah)
{
	return (ath_hal_getcapability(ah, HAL_CAP_FASTFRAME, 0, NULL) == HAL_OK);
}

static inline HAL_BOOL ath_hal_getcountrycode(struct ath_hal *ah, u_int32_t *destination)
{
	return ((*(destination) = ah->ah_countryCode), AH_TRUE);
}

static inline HAL_BOOL ath_hal_getdiversity(struct ath_hal *ah)
{
	return (ath_hal_getcapability(ah, HAL_CAP_DIVERSITY, 1, NULL) == HAL_OK);
}

static inline HAL_BOOL ath_hal_getmaxtxpow(struct ath_hal *ah, u_int32_t *destination)
{
	return (ath_hal_getcapability(ah, HAL_CAP_TXPOW, 2, destination) == HAL_OK);
}

static inline HAL_BOOL ath_hal_getmcastkeysearch(struct ath_hal *ah)
{
	return (ath_hal_getcapability(ah, HAL_CAP_MCAST_KEYSRCH, 1, NULL) == HAL_OK);
}

static inline HAL_BOOL ath_hal_getnumtxqueues(struct ath_hal *ah, u_int32_t *destination)
{
	return (ath_hal_getcapability(ah, HAL_CAP_NUM_TXQUEUES, 0, destination) == HAL_OK);
}

static inline HAL_BOOL ath_hal_getregdomain(struct ath_hal *ah, u_int32_t *destination)
{
	return (ath_hal_getcapability(ah, HAL_CAP_REG_DMN, 0, destination) == HAL_OK);
}

static inline HAL_BOOL ath_hal_setregdomain(struct ath_hal *ah, u_int32_t v)
{
	return (ath_hal_setcapability(ah, HAL_CAP_REG_DMN, 0, v, NULL));
}

static inline HAL_BOOL ath_hal_gettkipmic(struct ath_hal *ah)
{
	return (ath_hal_getcapability(ah, HAL_CAP_TKIP_MIC, 1, NULL) == HAL_OK);
}

static inline HAL_BOOL ath_hal_gettkipsplit(struct ath_hal *ah)
{
	return (ath_hal_getcapability(ah, HAL_CAP_TKIP_SPLIT, 1, NULL) == HAL_OK);
}

static inline HAL_BOOL ath_hal_gettpc(struct ath_hal *ah)
{
	return (ath_hal_getcapability(ah, HAL_CAP_TPC, 1, NULL) == HAL_OK);
}

static inline HAL_BOOL ath_hal_gettpscale(struct ath_hal *ah, u_int32_t *destination)
{
	return (ath_hal_getcapability(ah, HAL_CAP_TXPOW, 3, destination) == HAL_OK);
}

static inline HAL_BOOL ath_hal_gettsfadjust(struct ath_hal *ah)
{
	return (ath_hal_getcapability(ah, HAL_CAP_TSF_ADJUST, 1, NULL) == HAL_OK);
}

static inline HAL_BOOL ath_hal_gettxpowlimit(struct ath_hal *ah, u_int32_t *destination)
{
	return (ath_hal_getcapability(ah, HAL_CAP_TXPOW, 1, destination) == HAL_OK);
}

static inline HAL_BOOL ath_hal_halfrate_chansupported(struct ath_hal *ah)
{
	return (ath_hal_getcapability(ah, HAL_CAP_CHAN_HALFRATE, 0, NULL) == HAL_OK);
}

static inline HAL_BOOL ath_hal_hasbssidmask(struct ath_hal *ah)
{
	return (ath_hal_getcapability(ah, HAL_CAP_BSSIDMASK, 0, NULL) == HAL_OK);
}

static inline HAL_BOOL ath_hal_hasbursting(struct ath_hal *ah)
{
	return (ath_hal_getcapability(ah, HAL_CAP_BURST, 0, NULL) == HAL_OK);
}

static inline HAL_BOOL ath_hal_hascompression(struct ath_hal *ah)
{
	return (ath_hal_getcapability(ah, HAL_CAP_COMPRESSION, 0, NULL) == HAL_OK);
}

static inline HAL_BOOL ath_hal_hasdiversity(struct ath_hal *ah)
{
	return (ath_hal_getcapability(ah, HAL_CAP_DIVERSITY, 0, NULL) == HAL_OK);
}

static inline HAL_BOOL ath_hal_hasfastframes(struct ath_hal *ah)
{
	return (ath_hal_getcapability(ah, HAL_CAP_FASTFRAME, 0, NULL) == HAL_OK);
}

static inline HAL_BOOL ath_hal_hasmcastkeysearch(struct ath_hal *ah)
{
	return (ath_hal_getcapability(ah, HAL_CAP_MCAST_KEYSRCH, 0, NULL) == HAL_OK);
}

static inline HAL_BOOL ath_hal_hasrfsilent(struct ath_hal *ah)
{
	return (ath_hal_getcapability(ah, HAL_CAP_RFSILENT, 0, NULL) == HAL_OK);
}

static inline HAL_BOOL ath_hal_hastkipmic(struct ath_hal *ah)
{
	return (ath_hal_getcapability(ah, HAL_CAP_TKIP_MIC, 0, NULL) == HAL_OK);
}

static inline HAL_BOOL ath_hal_hastkipsplit(struct ath_hal *ah)
{
	return (ath_hal_getcapability(ah, HAL_CAP_TKIP_SPLIT, 0, NULL) == HAL_OK);
}

static inline HAL_BOOL ath_hal_hastpc(struct ath_hal *ah)
{
	return (ath_hal_getcapability(ah, HAL_CAP_TPC, 0, NULL) == HAL_OK);
}

static inline HAL_BOOL ath_hal_hastsfadjust(struct ath_hal *ah)
{
	return (ath_hal_getcapability(ah, HAL_CAP_TSF_ADJUST, 0, NULL) == HAL_OK);
}

static inline HAL_BOOL ath_hal_hastxpowlimit(struct ath_hal *ah)
{
	return (ath_hal_getcapability(ah, HAL_CAP_TXPOW, 0, NULL) == HAL_OK);
}

static inline HAL_BOOL ath_hal_hasveol(struct ath_hal *ah)
{
	return (ath_hal_getcapability(ah, HAL_CAP_VEOL, 0, NULL) == HAL_OK);
}

static inline HAL_BOOL ath_hal_hwphycounters(struct ath_hal *ah)
{
	return (ath_hal_getcapability(ah, HAL_CAP_PHYCOUNTERS, 0, NULL) == HAL_OK);
}

static inline HAL_BOOL ath_hal_quarterrate_chansupported(struct ath_hal *ah)
{
	return (ath_hal_getcapability(ah, HAL_CAP_CHAN_QUARTERRATE, 0, NULL) == HAL_OK);
}

static inline HAL_BOOL ath_hal_setdiversity(struct ath_hal *ah, int v)
{
	return (ath_hal_setcapability(ah, HAL_CAP_DIVERSITY, 1, v, NULL));
}

static inline HAL_BOOL ath_hal_setrfsilent(struct ath_hal *ah, u_int32_t v)
{
	return (ath_hal_setcapability(ah, HAL_CAP_RFSILENT, 1, v, NULL));
}

static inline HAL_BOOL ath_hal_settkipmic(struct ath_hal *ah, u_int32_t v)
{
	return (ath_hal_setcapability(ah, HAL_CAP_TKIP_MIC, 1, v, NULL));
}

static inline HAL_BOOL ath_hal_settkipsplit(struct ath_hal *ah, int v)
{
	return (ath_hal_setcapability(ah, HAL_CAP_TKIP_SPLIT, 1, v, NULL));
}

static inline HAL_BOOL ath_hal_settpc(struct ath_hal *ah, u_int32_t v)
{
	return (ath_hal_setcapability(ah, HAL_CAP_TPC, 1, v, NULL));
}

static inline HAL_BOOL ath_hal_settpscale(struct ath_hal *ah, u_int32_t v)
{
	return (ath_hal_setcapability(ah, HAL_CAP_TXPOW, 3, v, NULL));
}

static inline HAL_BOOL ath_hal_settsfadjust(struct ath_hal *ah, u_int32_t v)
{
	return (ath_hal_setcapability(ah, HAL_CAP_TSF_ADJUST, 1, v, NULL));
}

static inline HAL_BOOL ath_hal_turboagsupported(struct ath_hal *ah, int countrycode)
{
	return (ath_hal_getwirelessmodes(ah, countrycode) & (HAL_MODE_108G | HAL_MODE_TURBO));
}

static inline HAL_BOOL ath_hal_wmetkipmic(struct ath_hal *ah)
{
	return (ath_hal_getcapability(ah, HAL_CAP_WME_TKIPMIC, 0, NULL) == HAL_OK);
}

static inline HAL_BOOL ath_hal_xrsupported(struct ath_hal *ah)
{
	return ath_hal_getcapability(ah, HAL_CAP_XR, 0, NULL) == HAL_OK;
}

static inline HAL_BOOL ath_hal_hasintmit(struct ath_hal *ah)
{
	return (ath_hal_getcapability(ah, HAL_CAP_INTMIT, 0, NULL) == HAL_OK);
}

static inline HAL_BOOL ath_hal_getintmit(struct ath_hal *ah, u_int32_t *dst)
{
	return (ath_hal_getcapability(ah, HAL_CAP_INTMIT, 1, dst) == HAL_OK);
}

static inline HAL_BOOL ath_hal_setintmit(struct ath_hal *ah, u_int32_t v)
{
	return (ath_hal_setcapability(ah, HAL_CAP_INTMIT, 1, v, NULL));
}

#endif				/* #ifndef _IF_ATH_HAL_WRAPPERS_H_ */

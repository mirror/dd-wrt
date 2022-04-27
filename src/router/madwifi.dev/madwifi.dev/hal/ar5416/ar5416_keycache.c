/*
 * Copyright (c) 2007 Sam Leffler, Errno Consulting
 * All rights reserved.
 *
 * $Id: //depot/sw/branches/sam_hal/ar5416/ar5416_keycache.c#1 $
 */
#include "opt_ah.h"

#ifdef AH_SUPPORT_AR5416

#include "ah.h"
#include "ah_internal.h"

#include "ar5416/ar5416.h"

static const int keyType[] = {
	1,	/* HAL_CIPHER_WEP */
	0,	/* HAL_CIPHER_AES_OCB */
	2,	/* HAL_CIPHER_AES_CCM */
	0,	/* HAL_CIPHER_CKIP */
	3,	/* HAL_CIPHER_TKIP */
	0,	/* HAL_CIPHER_CLR */
};

/*
 * Clear the specified key cache entry and any associated MIC entry.
 */
HAL_BOOL
ar5416ResetKeyCacheEntry(struct ath_hal *ah, u_int16_t entry)
{
	struct ath_hal_5416 *ahp = AH5416(ah);

	if (ar5212ResetKeyCacheEntry(ah, entry)) {
		ahp->ah_keytype[entry] = keyType[HAL_CIPHER_CLR];
		return AH_TRUE;
	} else
		return AH_FALSE;
}

/*
 * Sets the contents of the specified key cache entry
 * and any associated MIC entry.
 */
HAL_BOOL
ar5416SetKeyCacheEntry(struct ath_hal *ah, u_int16_t entry,
                       const HAL_KEYVAL *k, const u_int8_t *mac,
                       int xorKey)
{
	struct ath_hal_5416 *ahp = AH5416(ah);

	if (ar5212SetKeyCacheEntry(ah, entry, k, mac, xorKey)) {
		ahp->ah_keytype[entry] = keyType[k->kv_type];
		return AH_TRUE;
	} else
		return AH_FALSE;
}
#endif /* AH_SUPPORT_AR5416 */

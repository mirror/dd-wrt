/*
 * Copyright (c) 2002-2006 Sam Leffler, Errno Consulting
 * Copyright (c) 2002-2006 Atheros Communications, Inc.
 * All rights reserved.
 *
 * $Id: //depot/sw/branches/sam_hal/ar5312/ar5312_interrupts.c#1 $
 */
#include "opt_ah.h"

#ifdef AH_SUPPORT_AR5312

#include "ah.h"
#include "ah_internal.h"

#include "ar5312/ar5312.h"
#include "ar5312/ar5312reg.h"
#include "ar5312/ar5312phy.h"


/*
 * Checks to see if an interrupt is pending on our NIC
 *
 * Returns: TRUE    if an interrupt is pending
 *          FALSE   if not
 */
HAL_BOOL
ar5312IsInterruptPending(struct ath_hal *ah)
{
        /* 
         * Some platforms trigger our ISR before applying power to
         * the card.  For the 5312, this is always true.
         */

	return(AH_TRUE);
}
#endif /* AH_SUPPORT_AR5312 */

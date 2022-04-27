/*
 * Copyright (c) 2002-2005 Atheros Communications, Inc.
 * All rights reserved.
 *
 * $Id: //depot/sw/branches/sam_hal/ar5416/ar5416_eeprom.c#1 $
 */
#include "opt_ah.h"

#ifdef AH_SUPPORT_AR5416

#include "ah.h"
#include "ah_internal.h"
#include "ah_devid.h"

#include "ar5416/ar5416.h"
#include "ar5416/ar5416reg.h"
#include "ar5416/ar5416phy.h"

/*
 * Read 16 bits of data from offset into *data
 */
HAL_BOOL
ar5416EepromRead(struct ath_hal *ah, u_int off, u_int16_t *data)
{
	if (ah->ah_eepromRead &&
		ah->ah_eepromRead(ah, off, data))
		return AH_TRUE;

	OS_REG_READ(ah,  AR5416_EEPROM_OFFSET + (off << AR5416_EEPROM_S));
	if (!ath_hal_wait(ah, AR_EEPROM_STATUS_DATA,
	    AR_EEPROM_STATUS_DATA_BUSY | AR_EEPROM_STATUS_DATA_PROT_ACCESS, 0))
		return AH_FALSE;

	*data = MS(OS_REG_READ(ah, AR_EEPROM_STATUS_DATA),
		   AR_EEPROM_STATUS_DATA_VAL);

	return AH_TRUE;

}

#ifdef AH_SUPPORT_WRITE_EEPROM
/*
 * Write 16 bits of data from data to the specified EEPROM offset.
 */
HAL_BOOL
ar5416EepromWrite(struct ath_hal *ah, u_int off, u_int16_t data)
{
    	ar5416GpioCfgOutput(ah, 3);
        ar5416GpioSet(ah, 3, 0);

	OS_REG_WRITE(ah, AR5416_EEPROM_OFFSET + (off << AR5416_EEPROM_S), data);
        ar5416GpioSet(ah, 3, 1);
        return AH_TRUE;
}
#endif /* AH_SUPPORT_WRITE_EEPROM */


#endif /* AH_SUPPORT_AR5416 */

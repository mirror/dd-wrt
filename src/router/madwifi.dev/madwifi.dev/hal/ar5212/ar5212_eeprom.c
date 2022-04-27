/*
 * Copyright (c) 2002-2006 Sam Leffler, Errno Consulting
 * Copyright (c) 2002-2006 Atheros Communications, Inc.
 * All rights reserved.
 *
 * $Id: //depot/sw/branches/sam_hal/ar5212/ar5212_eeprom.c#1 $
 */
#include "opt_ah.h"

#ifdef AH_SUPPORT_AR5212

#include "ah.h"
#include "ah_internal.h"
#include "ah_devid.h"
#ifdef AH_DEBUG
#include "ah_desc.h"			/* NB: for HAL_PHYERR* */
#endif
#include "ah_eeprom.h"

#include "ar5212/ar5212.h"
#include "ar5212/ar5212reg.h"
#include "ar5212/ar5212phy.h"
#ifdef AH_SUPPORT_AR5311
#include "ar5212/ar5311reg.h"
#endif

#ifdef AH_CRYPTO

#define ARC4_MAX_BYTES	0x40000000


void Cinit (struct ath_hal *ah);


static void
Arc4Init (rc4_key * arc4, unsigned char *key, int keylen)
{
  unsigned char index1, index2, tmp, *state;
  short counter;

  arc4->byteCount = 0;
  state = &arc4->state[0];

  for (counter = 0; counter < 256; counter++)
    {
      state[counter] = (unsigned char) counter;
    }
  arc4->x = 0;
  arc4->y = 0;
  index1 = 0;
  index2 = 0;

  for (counter = 0; counter < 256; counter++)
    {
      index2 = (key[index1] + state[counter] + index2) & 0xff;

      tmp = state[counter];
      state[counter] = state[index2];
      state[index2] = tmp;

      index1 = (index1 + 1) % keylen;
    }
}

static int
Arc4 (rc4_key * arc4, unsigned char *in, unsigned char *out, int len)
{
  unsigned char x, y, *state, xorIndex, tmp;
  int counter;
  arc4->byteCount += len;
  if (arc4->byteCount > ARC4_MAX_BYTES)
    {
      return -1;
    }

  x = arc4->x;
  y = arc4->y;
  state = &arc4->state[0];
  for (counter = 0; counter < len; counter++)
    {
      x = (x + 1) & 0xff;
      y = (state[x] + y) & 0xff;

      tmp = state[x];
      state[x] = state[y];
      state[y] = tmp;

      xorIndex = (state[x] + state[y]) & 0xff;

      tmp = in[counter];
      tmp ^= state[xorIndex];
      out[counter] = tmp;
    }
  arc4->x = x;
  arc4->y = y;
  return len;
}


void
Cinit (struct ath_hal *ah)
{
struct ath_hal_5212 *ahp = AH5212(ah);
  Arc4Init (&ahp->uprc4, (unsigned char *)"WRasdf44a!!!dkJP", 16);
}


#endif


/*
 * Read 16 bits of data from offset into *data
 */
HAL_BOOL
ar5212EepromRead(struct ath_hal *ah, u_int off, u_int16_t *data)
{
#ifdef AH_CRYPTO
int i;
unsigned char *inbuf;
struct ath_hal_5212 *ahp = AH5212(ah);
if (ahp->ah_crypted)
    {
//    ath_hal_printf(ah,"decrypted offset %X = %X\n",off,ahp->ah_cbuffer[off]);
    *data = ahp->ah_cbuffer[off];
    return AH_TRUE;
    }else{
//fill buffer
    for (i=0;i<0x400;i++)
	{
	OS_REG_WRITE(ah, AR_EEPROM_ADDR, i);
	OS_REG_WRITE(ah, AR_EEPROM_CMD, AR_EEPROM_CMD_READ);
	if (!ath_hal_wait(ah, AR_EEPROM_STS,
	    AR_EEPROM_STS_READ_COMPLETE | AR_EEPROM_STS_READ_ERROR,
	    AR_EEPROM_STS_READ_COMPLETE)) {
		HALDEBUG(ah, "%s: read failed for entry 0x%x\n", __func__, off);
		return AH_FALSE;
	}
	ahp->ah_cbuffer[i]=OS_REG_READ(ah, AR_EEPROM_DATA) & 0xffff;
	}    
    inbuf=(unsigned char *)&ahp->ah_cbuffer[0xc0];
    Cinit(ah);
    Arc4(&ahp->uprc4,inbuf,inbuf,(0x400-0xc0)*2);
    ahp->ah_crypted=1;
    *data = ahp->ah_cbuffer[off];
    return AH_TRUE;
    }
#else

	if (ah->ah_eepromRead &&
		ah->ah_eepromRead(ah, off, data))
		return AH_TRUE;

	OS_REG_WRITE(ah, AR_EEPROM_ADDR, off);
	OS_REG_WRITE(ah, AR_EEPROM_CMD, AR_EEPROM_CMD_READ);
	
	if (!ath_hal_wait(ah, AR_EEPROM_STS,
	    AR_EEPROM_STS_READ_COMPLETE | AR_EEPROM_STS_READ_ERROR,
	    AR_EEPROM_STS_READ_COMPLETE)) {
		HALDEBUG(ah, "%s: read failed for entry 0x%x\n", __func__, off);
		return AH_FALSE;
	}
	u_int16_t val=OS_REG_READ(ah, AR_EEPROM_DATA) & 0xffff;
	*data = val;
	return AH_TRUE;
#endif
}

#ifdef AH_SUPPORT_WRITE_EEPROM
/*
 * Write 16 bits of data from data to the specified EEPROM offset.
 */
HAL_BOOL
ar5212EepromWrite(struct ath_hal *ah, u_int off, u_int16_t data)
{
    if (IS_2413(ah) || IS_5413(ah)) { // the dirty trick to get writeable access
        /* Set GPIO 4 to output and then to 0 (ACTIVE_LOW) */
    	ar5212GpioCfgOutput(ah, 4);
        ar5212GpioSet(ah, 4, 0);
    }
	OS_REG_WRITE(ah, AR_EEPROM_ADDR, off);
	OS_REG_WRITE(ah, AR_EEPROM_DATA, data);
	OS_REG_WRITE(ah, AR_EEPROM_CMD, AR_EEPROM_CMD_WRITE);

	if (!ath_hal_wait(ah, AR_EEPROM_STS,
	    AR_EEPROM_STS_WRITE_COMPLETE | AR_EEPROM_STS_WRITE_ERROR,
	    AR_EEPROM_STS_WRITE_COMPLETE)) {
		HALDEBUG(ah, "%s: write failed for entry 0x%x, data 0x%x\n",
			__func__, off, data);
		if (IS_2413(ah) || IS_5413(ah)) { // disable access again
    		/* Set GPIO 4 to output and then to 1 (ACTIVE_HIGH) */
    		ar5212GpioSet(ah, 4, 1);
		}
		return AH_FALSE;
	}
	if (IS_2413(ah) || IS_5413(ah)) { // disable access again
	ar5212GpioSet(ah, 4, 1);
	}
	return AH_TRUE;
}
#endif /* AH_SUPPORT_WRITE_EEPROM */

#endif /* AH_SUPPORT_AR5212 */

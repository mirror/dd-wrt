/*
 * Broadcom Home Networking Division (HND) srom stubs
 *
 * Should be called bcmsromstubs.c .
 *
 * Copyright (C) 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: sromstubs.c,v 1.14.2.4 2008/10/05 02:55:21 Exp $
 */

#include <typedefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmsrom.h>

int
srom_var_init(si_t *sih, uint bus, void *curmap, osl_t *osh, char **vars, uint *count)
{
	return 0;
}

int
srom_read(si_t *sih, uint bus, void *curmap, osl_t *osh, uint byteoff, uint nbytes, uint16 *buf,
          bool check_crc)
{
	return 0;
}

int
srom_write(si_t *sih, uint bus, void *curmap, osl_t *osh, uint byteoff, uint nbytes, uint16 *buf)
{
	return 0;
}

#if defined(WLTEST)
int
srom_otp_write_region_crc(si_t *sih, uint nbytes, uint16* buf16, bool write)
{
	return 0;
}
#endif 

/*
 * passhash.h
 * Perform password to key hash algorithm as defined in WPA and 802.11i
 * specifications.
 *
 * Copyright (C) 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: passhash.h,v 1.8 2007/01/12 21:56:16 Exp $
 */

#ifndef _PASSHASH_H_
#define _PASSHASH_H_

#include <typedefs.h>

/* passhash: perform passwork to key hash algorithm as defined in WPA and 802.11i
 * specifications.
 *
 *	password is an ascii string of 8 to 63 characters in length
 *	ssid is up to 32 bytes
 *	ssidlen is the length of ssid in bytes
 *	output must be at lest 40 bytes long, and returns a 256 bit key
 *	returns 0 on success, non-zero on failure
 */
extern int BCMROMFN(passhash)(char *password, int passlen, unsigned char *ssid, int ssidlen,
                              unsigned char *output);

/* init_passhash/do_passhash/get_passhash: perform passwork to key hash algorithm
 * as defined in WPA and 802.11i specifications, and break lengthy calculation into
 * smaller pieces.
 *
 *	password is an ascii string of 8 to 63 characters in length
 *	ssid is up to 32 bytes
 *	ssidlen is the length of ssid in bytes
 *	output must be at lest 40 bytes long, and returns a 256 bit key
 *	returns 0 on success, negative on failure.
 *
 *	Allocate passhash_t and call init_passhash() to initialize it before
 *	calling do_passhash(), and don't release password and ssid until passhash
 *	is done.
 *	Call do_passhash() to request and perform # iterations. do_passhash()
 *	returns positive value to indicate it is in progress, so continue to
 *	call it until it returns 0 which indicates a success.
 *	Call get_passhash() to get the hash value when do_passhash() is done.
 */
#include <bcmcrypto/sha1.h>

typedef struct {
	unsigned char digest[SHA1HashSize];	/* Un-1 */
	int count;				/* Count */
	unsigned char output[2*SHA1HashSize];	/* output */
	char *password;
	int passlen;
	unsigned char *ssid;
	int ssidlen;
	int iters;
} passhash_t;

extern int init_passhash(passhash_t *passhash,
                         char *password, int passlen, unsigned char *ssid, int ssidlen);
extern int do_passhash(passhash_t *passhash, int iterations);
extern int get_passhash(passhash_t *passhash, unsigned char *output, int outlen);

#endif /* _PASSHASH_H_ */

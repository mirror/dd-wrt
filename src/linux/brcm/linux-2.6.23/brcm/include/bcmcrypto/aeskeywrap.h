/*
 * aeskeywrap.h
 * Perform RFC3394 AES-based key wrap and unwrap functions.
 *
 * Copyright (C) 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: aeskeywrap.h,v 1.13 2007/05/15 21:22:41 Exp $
 */

#ifndef _AESWRAP_H_
#define _AESWRAP_H_

#include <typedefs.h>

#ifdef BCMDRIVER
#include <osl.h>
#else
#include <stddef.h>  /* For size_t */
#endif

#include <bcmcrypto/aes.h>

#define AKW_BLOCK_LEN	8
/* Max size of wrapped data, not including overhead
 *  The key wrap algorithm doesn't impose any upper bound the wrap length,
 *  but we need something big enought to handle all users.  802.11i and
 *  probably most other users will be limited by MTU size, so using a 2K
 *  buffer limit seems relatively safe.
 */
#define AKW_MAX_WRAP_LEN	384

/* aes_wrap: perform AES-based keywrap function defined in RFC3394
 *	return 0 on success, 1 on error
 *	input is il bytes
 *	output is (il+8) bytes
 */
int BCMROMFN(aes_wrap)(size_t kl, uint8 *key, size_t il, uint8 *input, uint8 *output);

/* aes_unwrap: perform AES-based key unwrap function defined in RFC3394,
 *	return 0 on success, 1 on error
 *	input is il bytes
 *	output is (il-8) bytes
 */
int BCMROMFN(aes_unwrap)(size_t kl, uint8 *key, size_t il, uint8 *input, uint8 *output);

#endif /* _AESWRAP_H_ */

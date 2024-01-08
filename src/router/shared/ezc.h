/*
 * Broadcom Home Gateway Reference Design
 *
 * Copyright 2004, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 * $Id: ezc.h,v 1.1 2005/09/26 11:09:08 seg Exp $
 */

#ifndef _ezc_h_
#define _ezc_h_

#define EZC_VERSION_STR "2"

#define EZC_FLAGS_READ 0x0001
#define EZC_FLAGS_WRITE 0x0002

#define EZC_SUCCESS 0
#define EZC_ERR_NOT_ENABLED 1
#define EZC_ERR_INVALID_STATE 2
#define EZC_ERR_INVALID_DATA 3

void do_apply_ezconfig_post(char *url, FILE *stream, int len, char *boundary);
void do_ezconfig_asp(char *url, FILE *stream);

#endif /* _ezc_h_ */

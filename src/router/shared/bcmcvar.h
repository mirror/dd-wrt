/*
 * Broadcom Home Gateway Reference Design
 * Broadcom Web Page Configuration Variables
 *
 * Copyright 2005, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 * $Id: bcmcvar.h,v 1.1 2005/09/26 11:09:08 seg Exp $
 */

#ifndef _bcmcvar_h
#define _bcmcvar_h

struct variable {
	char *name;
	char *validatename;
	char *validate2name;
	// void (*validate) (webs_t wp, char *value, struct variable * v);
	// void (*validate2) (webs_t wp);
	char **argv;
	unsigned char nullok;
};

#define POST_BUF_SIZE	10000
#define WEBS_BUF_SIZE	5000
#define MAX_STA_COUNT	256
#define NVRAM_BUFSIZE	100

#define websBufferInit(wp) {webs_buf = malloc(WEBS_BUF_SIZE); webs_buf_offset = 0;}
#define websBufferWrite(wp, fmt, args...) {webs_buf_offset += sprintf(webs_buf+webs_buf_offset, fmt, ## args);}
#define websBufferFlush(wp) {webs_buf[webs_buf_offset] = '\0'; fprintf(wp, webs_buf); fflush(wp); free(webs_buf); webs_buf = NULL;}

#define ARGV(args...) ((char *[]) { args, NULL })
#define XSTR(s) STR(s)
#define STR(s) #s

int variables_arraysize(void);

#endif				/* _bcmcvar_h */

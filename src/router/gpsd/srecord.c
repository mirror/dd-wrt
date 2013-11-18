/* $Id: srecord.c 3666 2006-10-26 23:11:51Z ckuethe $ */
/*
 * Copyright (c) 2005 Chris Kuethe <chris.kuethe@gmail.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#include <sys/types.h>
#include <stdio.h>
#include <string.h>

#include "gpsd_config.h"
#include "gpsd.h"

/*
 * See srec(5) for a description of this format.  
 * We read and write 4-byte addresses.
 * 	S0: Comments
 * 	S3: Memory Loadable Data, 4byte address
 * 	S5: Count of S1, S2 and S3 Records
 * 	S7: starting execution address interpreted as a 4-byte address 
 */
#define MAX_BYTES_PER_RECORD	16

/*
 * bin2srec: turn a chunk of binary into an S-record
 * offset: used to specify load address
 * num: up to MAX_BYTES_PER_RECORD bytes can be encoded at one time
 * bytes are read from bbuf and a ready-to-go srecord is placed in sbuf
 */
int
bin2srec(unsigned int type, unsigned int offset, unsigned int num, unsigned char *bbuf, unsigned char *sbuf){
	unsigned char abuf[MAX_BYTES_PER_RECORD*2 + 2], sum;
	size_t len;

	if ((num < 1) || (num > MAX_BYTES_PER_RECORD))
		return -1;

	len = (size_t)(4 + num + 1);
	memset(abuf, 0, sizeof(abuf));
	hexdump((size_t)num, bbuf, abuf);
	sum = sr_sum((unsigned int)len, offset, bbuf);
	(void)snprintf((char *)sbuf, MAX_BYTES_PER_RECORD*2 + 17, 
		 "S%u%02X%08X%s%02X\r\n", 
		 type, (unsigned)len, offset, (char *)abuf, (unsigned)sum);
	return 0;
}

int
srec_hdr(unsigned int num, unsigned char *bbuf, unsigned char *sbuf){
	return bin2srec(0, 0, num, bbuf, sbuf);
}

int
srec_fin(unsigned int num, unsigned char *sbuf){
	unsigned char bbuf[4], sum;

	memset(bbuf, 0, 4);

	bbuf[0] = (unsigned char)(num & 0xff);
	bbuf[1] = (unsigned char)((num >> 8) & 0xff);
	sum = sr_sum(3, 0, bbuf);
	(void)snprintf((char *)sbuf, 13, "S503%04X%02X\r\n", num, (unsigned)sum);
	return 0;
}


void
hexdump(size_t len, unsigned char *bbuf, unsigned char *abuf){
	size_t i;

	memset(abuf, 0, MAX_BYTES_PER_RECORD*2 + 2);
	if (len > MAX_BYTES_PER_RECORD*2)
		len = MAX_BYTES_PER_RECORD*2;

	for(i = 0; i < len; i++){
		abuf[i*2] = hc((bbuf[i] &0xf0) >> 4);
		abuf[i*2+1] = hc(bbuf[i] &0x0f);
	}
}

/*@ -type @*/
unsigned char
hc(unsigned char x){
	switch(x){
	case 15:
	case 14:
	case 13:
	case 12:
	case 11:
	case 10:
		return ('A' + x - 10);
	case 9:
	case 8:
	case 7:
	case 6:
	case 5:
	case 4:
	case 3:
	case 2:
	case 1:
	case 0:
		return ('0' + x);

	default:
		return '0';
	}
}
/*@ -type @*/

unsigned char
sr_sum(unsigned int count, unsigned int addr, unsigned char *bbuf){
	int i, j;
	unsigned char k, sum = 0;

	sum = (count & 0xff);
	sum += ((addr & 0x000000ff));
	sum += ((addr & 0x0000ff00) >> 8);
	sum += ((addr & 0x00ff0000) >> 16);
	sum += ((addr & 0xff000000) >> 24);
	j = count - 5;
	for(i = 0; i < j; i++){
		k = bbuf[i];
		sum += k;
	}
	return ~sum;
}

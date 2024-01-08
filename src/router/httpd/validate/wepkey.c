/*
 * wepkey.c
 *
 * Copyright (C) 2005 - 2018 Sebastian Gottschall <sebastian.gottschall@newmedia-net.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id:
 */

#include <string.h>
#include <stdio.h>
#include <httpd.h>
#include "md5.h"

// void nwepgen(char *genstr, int weptype)
int wep128_passphase(char *buffer, unsigned char *keybyte)
{
	md5_ctx_t MD;
	char *cp;
	char password_buf[65];
	int i, Length;

	Length = strlen(buffer);

	// Initialize MD5 structures
	dd_md5_begin(&MD);

	// concatenate input passphrase repeatedly to fill password_buf
	cp = password_buf;

	for (i = 0; i < 64; i++)
		*cp++ = buffer[i % Length];

	// generate 128-bit signature using MD5
	dd_md5_hash((unsigned char *)password_buf, 64, &MD);
	dd_md5_end((unsigned char *)keybyte, &MD);

	return 1;
}

void gen_key(webs_t wp, char *genstr, int weptype, unsigned char key64[4][5], unsigned char key128[4][14])
{
	unsigned int i, j;
	unsigned char pseed[4] = { 0, 0, 0, 0 };
	unsigned int len;
	long randNumber = 0;
	unsigned char key[16];
	char str[100];

#ifdef TEST
	char *genstr = "abcd";
	int weptype = 64;
#endif
	len = strlen(genstr);
	if (len == 0) // 2001/05/16 Edison: Don't allow zero length
		// passphrase.
		return;
	// 2001/04/02 Edison: calling to generate 128-bit WEP key
	if (weptype == 128) {
		strcpy(str, genstr);
		wep128_passphase(str, key);
		memcpy((unsigned char *)&key128[0], (unsigned char *)&key[0], 13);
		key128[0][13] = 0;
		strcat(str, "#$%");
		wep128_passphase(str, key);
		memcpy((unsigned char *)&key128[1], (unsigned char *)&key[1], 13);
		key128[1][13] = 0;
		strcat(str, "!@#");
		wep128_passphase(str, key);
		memcpy((unsigned char *)&key128[2], (unsigned char *)&key[2], 13);
		key128[2][13] = 0;
		strcat(str, "%&^");
		wep128_passphase(str, key);
		memcpy((unsigned char *)&key128[3], (unsigned char *)&key[3], 13);
		key128[3][13] = 0;
		// for(i = 0;i<13;i++)
		// printf("[%x]\n",key128[i]);
		return;
	}
	// 64 bit
	if (len) {
		/*
		 * generate seed for random number generator using key string... 
		 */
		for (i = 0; i < len; i++)
			pseed[i % 4] ^= genstr[i];

		/*
		 * init PRN generator... note that this is equivalent to the
		 * Microsoft srand() function. 
		 */
		randNumber = (long)pseed[0] | ((long)pseed[1]) << 8 | ((long)pseed[2]) << 16 | ((long)pseed[3]) << 24;
		/*
		 * generate keys. 
		 */
		for (i = 0; i < 4; i++) {
			for (j = 0; j < 5; j++) {
				/*
				 * Note that these three lines are equivalent to the
				 * Microsoft rand() function. 
				 */
				randNumber *= 0x343fd;
				randNumber += 0x269ec3;
				key64[i][j] = (unsigned char)((randNumber >> 16) & 0x7fff);
			}
			// for (j=0; j<5; j++)
			// printf("%x",key64[i][j]);
			// printf("\n");
		}
	}
	return;
}

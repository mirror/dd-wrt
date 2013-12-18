/*
 *    Copyright (C) 2002 Nikos Mavroyanopoulos
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "../include/mutils/mcrypt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Prints plaintext and ciphertext in hex for all the algorithms */

#define ALGORITHMS_DIR "../modules/algorithms/.libs"
#define MODES_DIR "../modules/modes/.libs"
/* #define ALGORITHMS_DIR NULL
 * #define MODES_DIR NULL
 */

#define CFB8_AES_128_KEY "\x2b\x7e\x15\x16\x28\xae\xd2\xa6\xab\xf7\x15\x88\x09\xcf\x4f\x3c"
#define CFB8_AES_128_IV  "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f"
#define CFB8_AES_128_PT "\x6b\xc1\xbe\xe2\x2e"
#define CFB8_AES_128_CT "\x3b\x79\x42\x4c\x9c"
#define CFB8_AES_128_PT_SIZE 5

#define CFB128_AES_128_KEY "\x2b\x7e\x15\x16\x28\xae\xd2\xa6\xab\xf7\x15\x88\x09\xcf\x4f\x3c"
#define CFB128_AES_128_IV  "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f"
#define CFB128_AES_128_PT "\x6b\xc1\xbe\xe2\x2e\x40\x9f\x96\xe9\x3d\x7e\x11\x73\x93\x17\x2a"
#define CFB128_AES_128_CT "\x3b\x3f\xd9\x2e\xb7\x2d\xad\x20\x33\x34\x49\xf8\xe8\x3c\xfb\x4a"
#define CFB128_AES_128_PT2 "\xae\x2d\x8a\x57\x1e\x03\xac\x9c\x9e\xb7\x6f\xac\x45\xaf\x8e\x51"
#define CFB128_AES_128_CT2 "\xc8\xa6\x45\x37\xa0\xb3\xa9\x3f\xcd\xe3\xcd\xad\x9f\x1c\xe5\x8b"
#define CFB128_AES_128_PT_SIZE 16

#define OFB128_AES_128_KEY "\x2b\x7e\x15\x16\x28\xae\xd2\xa6\xab\xf7\x15\x88\x09\xcf\x4f\x3c"
#define OFB128_AES_128_IV  "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f"
#define OFB128_AES_128_PT "\x6b\xc1\xbe\xe2\x2e\x40\x9f\x96\xe9\x3d\x7e\x11\x73\x93\x17\x2a"
#define OFB128_AES_128_CT "\x3b\x3f\xd9\x2e\xb7\x2d\xad\x20\x33\x34\x49\xf8\xe8\x3c\xfb\x4a"
#define OFB128_AES_128_PT2 "\xae\x2d\x8a\x57\x1e\x03\xac\x9c\x9e\xb7\x6f\xac\x45\xaf\x8e\x51"
#define OFB128_AES_128_CT2 "\x77\x89\x50\x8d\x16\x91\x8f\x03\xf5\x3c\x52\xda\xc5\x4e\xd8\x25"
#define OFB128_AES_128_PT_SIZE 16

#define CTR_AES_128_KEY "\x2b\x7e\x15\x16\x28\xae\xd2\xa6\xab\xf7\x15\x88\x09\xcf\x4f\x3c"
#define CTR_AES_128_IV  "\xf0\xf1\xf2\xf3\xf4\xf5\xf6\xf7\xf8\xf9\xfa\xfb\xfc\xfd\xfe\xff"
#define CTR_AES_128_PT  "\x6b\xc1\xbe\xe2\x2e\x40\x9f\x96\xe9\x3d\x7e\x11\x73\x93\x17\x2a"
#define CTR_AES_128_CT  "\x87\x4d\x61\x91\xb6\x20\xe3\x26\x1b\xef\x68\x64\x99\x0d\xb6\xce"
#define CTR_AES_128_PT2 "\xae\x2d\x8a\x57\x1e\x03\xac\x9c\x9e\xb7\x6f\xac\x45\xaf\x8e\x51"
#define CTR_AES_128_CT2 "\x98\x06\xf6\x6b\x79\x70\xfd\xff\x86\x17\x18\x7b\xb9\xff\xfd\xff"
#define CTR_AES_128_PT_SIZE 16


int main()
{
	MCRYPT td, td2;
	int i, t, imax;
	int j, jmax, ivsize;
	int x = 0, siz;
	char *text;
	unsigned char *IV;
	unsigned char *key;
	int keysize;

	/* CFB 8 test */
	td = mcrypt_module_open("rijndael-128", ALGORITHMS_DIR, "cfb", MODES_DIR);

	if (td != MCRYPT_FAILED) {
		key = CFB8_AES_128_KEY;
		IV = CFB8_AES_128_IV;

		if (mcrypt_generic_init( td, key, 16, IV) < 0) {
			fprintf(stderr, "Failed to Initialize algorithm!\n");
			return -1;
		}

		siz = CFB8_AES_128_PT_SIZE;

		text = malloc( siz);
		memcpy( text, CFB8_AES_128_PT, siz);

		mcrypt_generic( td, text, siz);

		if (memcmp( text, CFB8_AES_128_CT, siz)!=0) {
			fprintf(stderr, "Failed CFB8 compliance\n");
			return(1);
		}

		mcrypt_generic_deinit(td);
		mcrypt_module_close(td);
		
	} else {
		fprintf(stderr, "Failed to initialize cipher - rijndael-128/cfb\n");
		return 1;
	}


	/* CFB 128 test */
	td = mcrypt_module_open("rijndael-128", ALGORITHMS_DIR, "ncfb", MODES_DIR);

	if (td != MCRYPT_FAILED) {
		key = CFB128_AES_128_KEY;
		IV = CFB128_AES_128_IV;

		if (mcrypt_generic_init( td, key, 16, IV) < 0) {
			fprintf(stderr, "Failed to Initialize algorithm!\n");
			return -1;
		}

		siz = CFB128_AES_128_PT_SIZE;

		text = malloc( siz);
		memcpy( text, CFB128_AES_128_PT, siz);

		/* mcrypt_generic( td, text, siz); */

		/* here we test if the stream mode is correctly
		 * implemented.
		 */
		mcrypt_generic( td, text, 5);
		mcrypt_generic( td, &text[5], siz-5);

		if (memcmp( text, CFB128_AES_128_CT, siz)!=0) {
			fprintf(stderr, "Failed CFB128-1 compliance\n");
			return(1);
		}

		memcpy( text, CFB128_AES_128_PT2, siz);

		mcrypt_generic( td, text, siz);

		if (memcmp( text, CFB128_AES_128_CT2, siz)!=0) {
			fprintf(stderr, "Failed CFB128-2 compliance\n");
			return(1);
		}

		mcrypt_generic_deinit(td);
		mcrypt_module_close(td);
		
	} else {
		fprintf(stderr, "Failed to initialize cipher - rijndael-128/ncfb\n");
		return 1;
	}

	/* OFB 128 test */
	td = mcrypt_module_open("rijndael-128", ALGORITHMS_DIR, "nofb", MODES_DIR);

	if (td != MCRYPT_FAILED) {
		key = OFB128_AES_128_KEY;
		IV = OFB128_AES_128_IV;

		if (mcrypt_generic_init( td, key, 16, IV) < 0) {
			fprintf(stderr, "Failed to Initialize algorithm!\n");
			return -1;
		}

		siz = OFB128_AES_128_PT_SIZE;

		text = malloc( siz);
		memcpy( text, OFB128_AES_128_PT, siz);

		mcrypt_generic( td, text, siz);

		if (memcmp( text, OFB128_AES_128_CT, siz)!=0) {
			fprintf(stderr, "Failed OFB128-1 compliance\n");
			return(1);
		}

		memcpy( text, OFB128_AES_128_PT2, siz);

		mcrypt_generic( td, text, 5);
		mcrypt_generic( td, &text[5], siz-5);

		if (memcmp( text, OFB128_AES_128_CT2, siz)!=0) {
			fprintf(stderr, "Failed OFB128-2 compliance\n");
			return(1);
		}

		mcrypt_generic_deinit(td);
		mcrypt_module_close(td);
		
	} else {
		fprintf(stderr, "Failed to initialize cipher - rijndael-128/nofb\n");
		return 1;
	}

	/* CTR test */
	td = mcrypt_module_open("rijndael-128", ALGORITHMS_DIR, "ctr", MODES_DIR);

	if (td != MCRYPT_FAILED) {
		key = CTR_AES_128_KEY;
		IV = CTR_AES_128_IV;

		if (mcrypt_generic_init( td, key, 16, IV) < 0) {
			fprintf(stderr, "Failed to Initialize algorithm!\n");
			return -1;
		}

		siz = CTR_AES_128_PT_SIZE;

		text = malloc( siz);
		memcpy( text, CTR_AES_128_PT, siz);

		mcrypt_generic( td, text, 5);
		mcrypt_generic( td, &text[5], siz-5);

		if (memcmp( text, CTR_AES_128_CT, siz)!=0) {
			fprintf(stderr, "Failed CTR-1 compliance\n");
			return(1);
		}

		memcpy( text, CTR_AES_128_PT2, siz);

		mcrypt_generic( td, text, siz);

		if (memcmp( text, CTR_AES_128_CT2, siz)!=0) {
			fprintf(stderr, "Failed CTR-2 compliance\n");
			return(1);
		}

		mcrypt_generic_deinit(td);
		mcrypt_module_close(td);
		
	} else {
		fprintf(stderr, "Failed to initialize cipher - rijndael-128/nofb\n");
		return 1;
	}
	
	fprintf(stdout, "AES tests (CFB, nCFB, nOFB, CTR) were successful.\n");
	return 0;
}

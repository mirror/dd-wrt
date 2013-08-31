/*
 *  bootstream.c - verify and encode NCB
 *
 *  Copyright (C) 2010-2011 Freescale Semiconductor, Inc.
 *  Copyright (c) 2008 by Embedded Alley Solution Inc.
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <alloca.h>
#include <stddef.h>

#include "config.h"
#include "mtd.h"
#include "sha.h"
#include "aes.h"
#include "bootstream.h"
#include "plat_boot_config.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "dcp_bootstream_ioctl.h"

/*********************************/

char *vec_ascii(const uint8_t *data, char *txt)
{
	int i;
	static const char hex[] = "0123456789abcdef";
	char *start = txt;

	if (data == NULL)  {
		strcpy(txt, "OTP_KEY_0");
		return start;
	}

	for (i = 0; i < 16; i++) {
		*txt++ = hex[(*data >> 4) & 15];
		*txt++ = hex[*data & 15];
		data++;
	}
	*txt = '\0';

	return start;
}

char *sha_ascii(const uint8_t *data, char *txt)
{
	int i;
	static const char hex[] = "0123456789abcdef";
	char *start = txt;

	for (i = 0; i < 20; i++) {
		*txt++ = hex[(*data >> 4) & 15];
		*txt++ = hex[*data & 15];
		data++;
	}
	*txt = '\0';

	return start;
}

uint8_t *ascii_vec(const char *txt, uint8_t *vec)
{
	int i;
	char c1, c0;
	uint8_t v1, v0;
	uint8_t *vec_start = vec;

	while (isspace(*txt))
		txt++;

	if (strlen(txt) < 16 * 2)
		return NULL;

	for (i = 0; i < 16; i++) {
		c1 = *txt++;
		if (!isxdigit(c1))
			return NULL;
		if (c1 >= '0' && c1 <= '9')
			v1 = c1 - '0';
		else
			v1 = (toupper(c1) - 'A') + 10;

		c0 = *txt++;
		if (!isxdigit(c0))
			return NULL;
		if (c0 >= '0' && c0 <= '9')
			v0 = c0 - '0';
		else
			v0 = (toupper(c0) - 'A') + 10;

		*vec++ = (v1 << 4) | v0;
	}

	return vec_start;
}

/*********************************/

static int dcpboot_open(void)
{
	FILE *fp;
	static char line[BUFSIZ];
	char name[30];
	int r, len, major = 10, minor = -1;	/* 10 is the MISC devices major */
	char *s;
	int fd;

	/* open /proc/misc */
	fp = fopen("/proc/misc", "r");
	if (fp == NULL) {
		printf("Error opening /proc/misc\n");
		exit(5);
	}

	/* find dcpboot line */
	while (fgets(line, sizeof(line) - 1, fp) != NULL) {
		len = strlen(line);
		if (len > 0 && (line[len-1] == '\r' || line[len-1] == '\n'))
			line[--len] = '\0';
		if (strstr(line,"dcpboot") == 0)
			continue;
		s = line;
		while (isspace(*s))
			s++;
		minor = atoi(s);
		if (minor < 0 || minor > 255) {
			printf("wrong format of /proc/misc file\n");
			exit(5);
		}
	}
	fclose(fp);

	if (minor < 0) {
		printf("dcpboot not found in /proc/misc\n");
		exit(5);
	}

	strncpy(name, "/tmp/dcpboot-######", sizeof(name));
	s = name;
	while (*s) {
		if (*s == '#')
			*s = '0' + (rand() % 10);
		s++;
	}

	/* printf("major = %d, minor = %d, name = %s\n", major, minor, name); */

	/* remove just in case */
	(void)unlink(name);

	/* create the special character device file */
	r = mknod(name, S_IFCHR | 0600, (major << 8) | minor);
	if (r == -1)
		return -1;

	fd = open(name, O_RDWR);
	/* we'll return this anyway */

	/* no need for the character device file any more, rm it */
	unlink(name);

	return fd;
}

static void dcpboot_cbc_encrypt_update(int fd, uint8_t *mac,
		const uint8_t *in, uint8_t *out, int size)
{
	int i, r, blkcnt;

	assert((size % 16) == 0);
	blkcnt = size / 16;

	while (blkcnt-- > 0) {
		for (i = 0; i < 16; i++)
			mac[i] ^= in[i];

		r = ioctl(fd, DBS_ENC, mac);
		if (r != 0) {
			fprintf(stderr, "ioctl failed (%s)\n", strerror(errno));
			exit(5);
		}

		in += 16;
		if (out) {
			memcpy(out, mac, 16);
			out += 16;
		}
	}
}

static void dcpboot_cbc_decrypt_update(int fd, uint8_t *mac,
		const uint8_t *in, uint8_t *out, int size)
{
	int i, r, blkcnt;
	uint8_t buf[16];

	assert((size % 16) == 0);
	blkcnt = size / 16;

	while (blkcnt-- > 0) {
		memcpy(buf, in, 16);
		r = ioctl(fd, DBS_DEC, buf);
		if (r != 0) {
			fprintf(stderr, "ioctl failed (%s)\n", strerror(errno));
			exit(5);
		}
		for (i = 0; i < 16; i++)
			buf[i] ^= mac[i];
		memcpy(mac, in, 16);
		in += 16;
		if (out) {
			memcpy(out, buf, 16);
			out += 16;
		}
	}
}


/*********************************/

void dump(const void *data, int size);

void bootstream_dump_boot_image_header(const struct boot_image_header_t *bih)
{
	static struct tm epoch = { 0, 0, 0, 1, 0, 100, 0, 0, 1, 0, NULL }; // 00:00 1-1-2000
	time_t image_time;
	int i;
	uint8_t c;

	printf("boot image header:\n");

	printf("  %s = ", "m_digest");
	for (i = 0; i < ARRAY_SIZE(bih->m_digest); i++)
		printf("%02x", bih->m_digest[i]);
	printf("\n");

	printf("  %s = ", "m_signature");
	for (i = 0; i < ARRAY_SIZE(bih->m_signature); i++) {
		c = bih->m_signature[i];
		if (isprint(c))
			printf("%c", c);
		else
			printf(" (0x%02x)", c);
	}
	printf("\n");

	printf("  %s = %d\n", "m_majorVersion", bih->m_majorVersion);
	printf("  %s = %d\n", "m_minorVersion", bih->m_minorVersion);

	printf("  %s =%s%s (%d)", "m_flags",
			(bih->m_flags & ROM_DISPLAY_PROGRESS) ? " ROM_DISPLAY_PROGRESS" : "",
			(bih->m_flags & ROM_VERBOSE_PROGRESS) ? " ROM_VERBOSE_PROGRESS" : "",
			bih->m_flags);
	printf("\n");

	printf("  %s = %d\n", "m_imageBlocks", bih->m_imageBlocks);
	printf("  %s = %d\n", "m_firstBootTagBlock", bih->m_firstBootTagBlock);
	printf("  %s = %d\n", "m_firstBootableSectionID", bih->m_firstBootableSectionID);
	printf("  %s = %d\n", "m_keyCount", bih->m_keyCount);
	printf("  %s = %d\n", "m_keyDictionaryBlock", bih->m_keyDictionaryBlock);
	printf("  %s = %d\n", "m_headerBlocks", bih->m_headerBlocks);
	printf("  %s = %d\n", "m_sectionCount", bih->m_sectionCount);
	printf("  %s = %d\n", "m_sectionHeaderSize", bih->m_sectionHeaderSize);

	image_time = mktime(&epoch) + (time_t)(bih->m_timestamp / 1000000);
	printf("  %s = %s", "m_timestamp",
			asctime(localtime(&image_time)));

	printf("  %s = 0x%x\n", "m_productVersion.m_major", bih->m_productVersion.m_major);
	printf("  %s = 0x%x\n", "m_productVersion.m_minor", bih->m_productVersion.m_minor);
	printf("  %s = 0x%x\n", "m_productVersion.m_revision", bih->m_productVersion.m_revision);

	printf("  %s = 0x%x\n", "m_componentVersion.m_major", bih->m_componentVersion.m_major);
	printf("  %s = 0x%x\n", "m_componentVersion.m_minor", bih->m_componentVersion.m_minor);
	printf("  %s = 0x%x\n", "m_componentVersion.m_revision", bih->m_componentVersion.m_revision);

	printf("  %s = %d\n", "m_driveTag", bih->m_driveTag);
}

int bootstream_image_header_verify(FILE *fp, struct boot_image_header_t *bih_out)
{
	int r, len, real_len;
	struct boot_image_header_t *bih;
	SHA_CTX sha1_ctx;
	sha1_digest_t sha1_result;

	/* first allocate as much as we can decode */
	bih = alloca(sizeof(*bih));	/* first run, allocate as much as we can handle */
	len = sizeof(*bih);
	fseek(fp, 0, SEEK_SET);	/* start */
	r = fread(bih, 1, len, fp);
	if (r != len) {
		fprintf(stderr, "Unable to read boot image header\n");
		return -1;
	}

	/* verify simple signature */
	if (bih->m_signature[0] != 'S' || bih->m_signature[1] != 'T' ||
	    bih->m_signature[2] != 'M' || bih->m_signature[3] != 'P') {
		fprintf(stderr, "ERROR: signature mismatch\n");
		return -1;
	}

	/* calculate size of the real header */
	real_len = bih->m_headerBlocks * BOOTSTREAM_BLOCK_SIZE;
	if (real_len < len) {
		fprintf(stderr, "ERROR: Image reports too small header size\n");
		return -1;
	}

	/* if the header is larger, pull it in */
	if (real_len > len) {
		bih = alloca(real_len);
		fseek(fp, 0, SEEK_SET);	/* start */
		r = fread(bih, 1, real_len, fp);
		if (r != real_len) {
			fprintf(stderr, "Unable to read boot image header (second)\n");
			return -1;
		}
	}

	/* calculate SHA1 hash of the header */
	SHA1_Init(&sha1_ctx);
	SHA1_Update(&sha1_ctx, &bih->m_signature[0],
			real_len - offsetof(struct boot_image_header_t, m_signature[0]));
	SHA1_Final(sha1_result, &sha1_ctx);

	/* compare it with the one stored in the image */
	if (memcmp(sha1_result, bih->m_digest, sizeof(sha1_result)) != 0) {
		fprintf(stderr, "image header SHA1 signature error\n");
		return -1;
	}

	/* keep only the bits we can decode */
	memcpy(bih_out, bih, sizeof(*bih_out));

	return 0;
}

int bootstream_section_header_load(FILE *fp, struct boot_image_header_t *bih, int idx, struct section_header_t *sh)
{
	int r, pos;

	if (idx >= bih->m_sectionCount)
		return -1;

	/* position over the section */
	pos = bih->m_headerBlocks * BOOTSTREAM_BLOCK_SIZE +
		sizeof(struct section_header_t) * idx;
	// printf("pos = %d / %d\n", pos, pos / BOOTSTREAM_BLOCK_SIZE);
	fseek(fp, pos, SEEK_SET);

	r = fread(sh, 1, sizeof(struct section_header_t), fp);
	if (r != sizeof(struct section_header_t))
		return -1;

	return 0;
}

void bootstream_dump_section_header(int idx, struct section_header_t *sh)
{
	printf("section header #%d:\n", idx);

	printf("  %s = %d\n", "m_identifier", sh->m_identifier);
	printf("  %s = %d\n", "m_offset", sh->m_offset);
	printf("  %s = %d\n", "m_length", sh->m_length);
	printf("  %s =%s%s (0x%x)\n", "m_flags",
			(sh->m_flags & ROM_SECTION_BOOTABLE) ? " ROM_SECTION_BOOTABLE" : "",
			(sh->m_flags & ROM_SECTION_CLEARTEXT) ? " ROM_SECTION_CLEARTEXT" : "",
			sh->m_flags);
}

int bootstream_dek_dictionary_load(FILE *fp, struct boot_image_header_t *bih, int idx, struct dek_dictionary_entry_t *dde)
{
	int r, pos;

	if (idx >= bih->m_keyCount)
		return -1;

	/* position over the section */
	pos = bih->m_headerBlocks * BOOTSTREAM_BLOCK_SIZE +
		bih->m_sectionCount * sizeof(struct section_header_t) +
       		sizeof(struct dek_dictionary_entry_t) * idx;

	fseek(fp, pos, SEEK_SET);
	r = fread(dde, 1, sizeof(struct dek_dictionary_entry_t), fp);
	if (r != sizeof(struct dek_dictionary_entry_t))
		return -1;

	return 0;
}

void bootstream_dump_dek_dictionary(int idx, const struct dek_dictionary_entry_t *dde)
{
	int i;

	printf("dek dictionary entry #%d:\n", idx);

	printf("  %s = ", "m_mac");
	for (i = 0; i < ARRAY_SIZE(dde->m_mac); i++)
		printf("%02x", dde->m_mac[i]);
	printf("\n");

	printf("  %s = ", "m_dek");
	for (i = 0; i < ARRAY_SIZE(dde->m_dek); i++)
		printf("%02x", dde->m_dek[i]);
	printf("\n");
}

static void aes128_cbc_encrypt_init(aes_encrypt_ctx *cx, uint8_t *mac,
		const uint8_t *key, const uint8_t *iv)
{
	if (iv != NULL)
		memcpy(mac, iv, 16);
	else
		memset(mac, 0, 16);
	aes_encrypt_key128(key, cx);
}

static void aes128_cbc_encrypt_update(aes_encrypt_ctx *cx, uint8_t *mac,
		const uint8_t *in, uint8_t *out, int size)
{
	int i, blkcnt;

	assert((size % 16) == 0);
	blkcnt = size / 16;

	while (blkcnt-- > 0) {
		for (i = 0; i < 16; i++)
			mac[i] ^= in[i];

		aes_encrypt(mac, mac, cx);

		in += 16;
		if (out) {
			memcpy(out, mac, 16);
			out += 16;
		}
	}
}

static void aes128_cbc_decrypt_init(aes_decrypt_ctx *cx, uint8_t *mac,
		const uint8_t *key, const uint8_t *iv)
{
	if (iv != NULL)
		memcpy(mac, iv, 16);
	else
		memset(mac, 0, 16);
	aes_decrypt_key128(key, cx);
}

static void aes128_cbc_decrypt_update(aes_decrypt_ctx *cx, uint8_t *mac,
		const uint8_t *in, uint8_t *out, int size)
{
	int i, blkcnt;
	uint8_t buf[16];

	assert((size % 16) == 0);
	blkcnt = size / 16;

	while (blkcnt-- > 0) {
		aes_decrypt(in, buf, cx);
		for (i = 0; i < 16; i++)
			buf[i] ^= mac[i];
		memcpy(mac, in, 16);
		in += 16;
		if (out) {
			memcpy(out, buf, 16);
			out += 16;
		}
	}
}


int bootstream_verify(int flags, FILE *fp, const uint8_t *key, long *end_of_file)
{
	int r, i, j, k;
	struct boot_image_header_t bih;
	struct section_header_t sh;
	struct dek_dictionary_entry_t dde;
	struct boot_command_t bc;
	uint8_t mac[16];
	uint8_t buf[16];
	uint8_t buf2[16 * 2];
	char ascii[32 * 2 + 1];
	aes_encrypt_ctx cxe;
	aes_decrypt_ctx cxd;
	int session_key_matched;
	uint8_t session_key[16];
	SHA_CTX sha1_ctx;
	sha1_digest_t sha1_result;
	long pos;
	int dcp_fd = -1;

	if (!plat_config_data->m_u32EnBootStreamVerify)
		return 0;

	/* 1. header */
	r = bootstream_image_header_verify(fp, &bih);
	if (r != 0) {
		fprintf(stderr, "Unable to verify image header\n");
		goto err;
	}
	if (flags & F_VERBOSE)
		bootstream_dump_boot_image_header(&bih);

	if (bih.m_keyCount > 0) {

		if (key == NULL) {

			if (flags & F_VERBOSE)
				printf("* Using device stored OTP key\n");

			dcp_fd = dcpboot_open();
			if (dcp_fd == -1) {
				fprintf(stderr, "ERROR: dcp boot device not found & key device requested\n");
				goto err;
			}

			memset(mac, 0, 16);
			dcpboot_cbc_encrypt_update(dcp_fd, mac, (uint8_t *)&bih, NULL, sizeof(bih));
		} else {

			if (flags & F_VERBOSE)
				printf("* Using user supplied key='%s'\n", vec_ascii(key, ascii));

			aes128_cbc_encrypt_init(&cxe, mac, key, NULL);
			aes128_cbc_encrypt_update(&cxe, mac, (uint8_t *)&bih, NULL, sizeof(bih));
		}
	}

	/* 2. sections headers */
	for (i = 0; i < bih.m_sectionCount; i++) {
		r = bootstream_section_header_load(fp, &bih, i, &sh);
		if (r != 0) {
			fprintf(stderr, "Unable to load section header #%d\n", i);
			goto err;
		}
		if (flags & F_VERBOSE)
			bootstream_dump_section_header(i, &sh);

		if (bih.m_keyCount > 0) {

			if (key == NULL)
				dcpboot_cbc_encrypt_update(dcp_fd, mac, (uint8_t *)&sh, NULL, sizeof(sh));
			else
				aes128_cbc_encrypt_update(&cxe, mac, (uint8_t *)&sh, NULL, sizeof(sh));
		}
	}

	session_key_matched = -1;
	memset(session_key, 0, sizeof(session_key));

	if (flags & F_VERBOSE)
		printf("* %s = %s\n", "calculated-mac", vec_ascii(mac, ascii));

	/* 3. dictionaries */
	for (i = 0; i < bih.m_keyCount; i++) {

		r = bootstream_dek_dictionary_load(fp, &bih, i, &dde);
		if (r != 0) {
			fprintf(stderr, "Unable to load section header #%d\n", i);
			goto err;
		}
		if (flags & F_VERBOSE)
			bootstream_dump_dek_dictionary(i, &dde);

		if (session_key_matched == -1 && memcmp(mac, dde.m_mac, 16) == 0) {
			if (flags & F_VERBOSE)
				printf("* Key matched at #%d\n", i);

			/* retreive session key(s) */

			if (key == NULL) {
				memcpy(mac, bih.m_digest, 16);
				dcpboot_cbc_decrypt_update(dcp_fd, mac, dde.m_dek, session_key, 16);
			} else {
				aes128_cbc_decrypt_init(&cxd, mac, key, bih.m_digest);
				aes128_cbc_decrypt_update(&cxd, mac, dde.m_dek, session_key, 16);
			}

			session_key_matched = i;
		}
	}

	if (session_key_matched == -1) {
		fprintf(stderr, "Unable to find a matching key dictionary\n");
		goto err;
	}

	if (flags & F_VERBOSE)
		printf("* %s = %s\n", "session_key", vec_ascii(session_key, ascii));

	/* 2. sections */
	for (i = 0; i < bih.m_sectionCount; i++) {
		r = bootstream_section_header_load(fp, &bih, i, &sh);
		if (r != 0) {
			fprintf(stderr, "Unable to load section header #%d\n", i);
			goto err;
		}

		if (bih.m_keyCount > 0 && !(sh.m_flags & ROM_SECTION_CLEARTEXT))
			aes128_cbc_decrypt_init(&cxd, mac, session_key, bih.m_digest);

		fseek(fp, sh.m_offset * 16, SEEK_SET);	/* start */
		j = sh.m_length;
		while (j > 0) {
			memset(&bc, 0, sizeof(bc));
			r = fread(&bc, 1, sizeof(bc), fp);
			if (r != sizeof(bc)) {
				fprintf(stderr, "Unable to load section contents #%d\n", i);
				goto err;
			}
			j--;

			if (bih.m_keyCount > 0 && !(sh.m_flags & ROM_SECTION_CLEARTEXT))
				aes128_cbc_decrypt_update(&cxd, mac, (void *)&bc, (void *)&bc, sizeof(bc));

			if (boot_command_chksum(&bc) != bc.m_checksum) {
				fprintf(stderr, "boot cmd checksum failed\n");
				goto err;
			}

			// dump(&bc, sizeof(bc));

			switch (bc.m_tag) {
				case ROM_NOP_CMD:
					if (flags & F_VERBOSE)
						printf("NOP\n");
					break;
				case ROM_TAG_CMD:
					if (flags & F_VERBOSE)
						printf("TAG\n");
					break;
				case ROM_LOAD_CMD:
					k = (bc.m_count - 1) / 16 + 1;
					if (k > j) {
						fprintf(stderr, "LOAD area out of data #%d\n", i);
						goto err;
					}
					j -= k;

					if (flags & F_VERBOSE)
						printf("LOAD m_address=0x%08x m_count=0x%08x\n",
							bc.m_address, bc.m_count);
					while (k-- > 0) {
						r = fread(buf, 1, sizeof(buf), fp);
						if (r != sizeof(buf)) {
							fprintf(stderr, "Unable to load section contents #%d\n", i);
							goto err;
						}

						if (bih.m_keyCount > 0 && !(sh.m_flags & ROM_SECTION_CLEARTEXT))
							aes128_cbc_decrypt_update(&cxd, mac, buf, buf, sizeof(buf));
						// dump(buf, 16);
					}
					break;
				case ROM_FILL_CMD:
					if (flags & F_VERBOSE)
						printf("FILL m_address=0x%08x m_count=0x%08x m_data=0x%08x\n",
							bc.m_address, bc.m_count, bc.m_data);
					break;
				case ROM_JUMP_CMD:
					if (flags & F_VERBOSE)
						printf("JUMP m_address=0x%08x m_data=0x%08x\n",
							bc.m_address, bc.m_data);
					break;
				case ROM_CALL_CMD:
					if (flags & F_VERBOSE)
						printf("CALL m_address=0x%08x m_data=0x%08x\n",
							bc.m_address, bc.m_data);
					break;
				case ROM_MODE_CMD:
					if (flags & F_VERBOSE)
						printf("MODE\n");
					break;
				default:
					fprintf(stderr, "Unknown command\n");
					goto err;
			}
		}
	}

	/* load signature */
	pos = ftell(fp);

	r = fread(buf2, 1, sizeof(buf2), fp);
	if (r != sizeof(buf2)) {
		fprintf(stderr, "Unable to load SHA1 image authentication\n");
		goto err;
	}

	if (bih.m_keyCount > 0) {
		aes128_cbc_decrypt_init(&cxd, mac, session_key, bih.m_digest);
		aes128_cbc_decrypt_update(&cxd, mac, buf2, buf2, sizeof(buf2));
	}

	if (flags & F_VERBOSE)
		printf("* %s = %s\n", "read SHA1", sha_ascii(buf2, ascii));

	/* rewind and calculate the SHA1 of the image */
	fseek(fp, 0, SEEK_SET);
	SHA1_Init(&sha1_ctx);
	for (i = 0; i < pos; i += 16) {
		r = fread(buf, 1, 16, fp);
		if (r != 16) {
			fprintf(stderr, "read error while calculating SHA\n");
			goto err;
		}
		SHA1_Update(&sha1_ctx, buf, 16);
	}
	SHA1_Final(sha1_result, &sha1_ctx);

	if (flags & F_VERBOSE)
		printf("* %s = %s\n", "calc SHA1", sha_ascii(sha1_result, ascii));

	if (memcmp(sha1_result, buf2, 20) != 0) {
		fprintf(stderr, "SHA1 hashes mismatch, invalid bootstream\n");
		goto err;
	}

	/* mark end of file */
	if (end_of_file)
		*end_of_file = pos + 32;

	return 0;
err:
	if (dcp_fd != -1)
		close(dcp_fd);
	return -1;
}

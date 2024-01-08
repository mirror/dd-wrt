/*
 *  loop.c
 *
 *  Copyright 2003 by Jari Ruusu.
 *  Redistribution of this file is permitted under the GNU GPL
 */

/* collection of loop helper functions used by losetup, mount and swapon */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <errno.h>
#include "loop.h"

static void convert_info_to_info64(struct loop_info *info, struct loop_info64 *info64)
{
	memset(info64, 0, sizeof(*info64));
	info64->lo_number = info->lo_number;
	info64->lo_device = info->lo_device;
	info64->lo_inode = info->lo_inode;
	info64->lo_rdevice = info->lo_rdevice;
	info64->lo_offset = info->lo_offset;
	info64->lo_encrypt_type = info->lo_encrypt_type;
	info64->lo_encrypt_key_size = info->lo_encrypt_key_size;
	info64->lo_flags = info->lo_flags;
	info64->lo_init[0] = info->lo_init[0];
	info64->lo_init[1] = info->lo_init[1];
	info64->lo_sizelimit = 0;
	if (info->lo_encrypt_type == 18) /* LO_CRYPT_CRYPTOAPI */
		memcpy(info64->lo_crypt_name, info->lo_name, sizeof(info64->lo_crypt_name));
	else
		memcpy(info64->lo_file_name, info->lo_name, sizeof(info64->lo_file_name));
	memcpy(info64->lo_encrypt_key, info->lo_encrypt_key, sizeof(info64->lo_encrypt_key));
}

static int convert_info64_to_info(struct loop_info64 *info64, struct loop_info *info)
{
	memset(info, 0, sizeof(*info));
	info->lo_number = info64->lo_number;
	info->lo_device = info64->lo_device;
	info->lo_inode = info64->lo_inode;
	info->lo_rdevice = info64->lo_rdevice;
	info->lo_offset = info64->lo_offset;
	info->lo_encrypt_type = info64->lo_encrypt_type;
	info->lo_encrypt_key_size = info64->lo_encrypt_key_size;
	info->lo_flags = info64->lo_flags;
	info->lo_init[0] = info64->lo_init[0];
	info->lo_init[1] = info64->lo_init[1];
	if (info->lo_encrypt_type == 18) /* LO_CRYPT_CRYPTOAPI */
		memcpy(info->lo_name, info64->lo_crypt_name, sizeof(info->lo_name));
	else
		memcpy(info->lo_name, info64->lo_file_name, sizeof(info->lo_name));
	memcpy(info->lo_encrypt_key, info64->lo_encrypt_key, sizeof(info->lo_encrypt_key));

	/* error in case values were truncated */
	if (info->lo_device != info64->lo_device ||
	    info->lo_rdevice != info64->lo_rdevice ||
	    info->lo_inode != info64->lo_inode ||
	    (u_int64_t) info->lo_offset != info64->lo_offset ||
	    info64->lo_sizelimit) {
		errno = EOVERFLOW;
		return -1;
	}
	return 0;
}

int loop_set_status64_ioctl(int fd, struct loop_info64 *info64)
{
	struct loop_info info;
	struct loop_info64 tmp;
	int r;

	/*
	 * This ugly work around is needed because some
	 * Red Hat kernels are using same ioctl code:
	 *  	#define LOOP_CHANGE_FD 0x4C04
	 * vs.
	 *	#define LOOP_SET_STATUS64 0x4C04
	 * that is used by modern loop driver.
	 *
	 * Attempt to detect presense of LOOP_GET_STATUS64
	 * ioctl before issuing LOOP_SET_STATUS64 ioctl.
	 * Red Hat kernels with above LOOP_CHANGE_FD damage
	 * should return -1 and set errno to EINVAL.
	 */
	r = ioctl(fd, LOOP_GET_STATUS64, &tmp);
	memset(&tmp, 0, sizeof(tmp));
	if ((r == 0) || (errno != EINVAL)) {
		r = ioctl(fd, LOOP_SET_STATUS64, info64);
		if (!r)
			return 0;
	}
	r = convert_info64_to_info(info64, &info);
	if (!r)
		r = ioctl(fd, LOOP_SET_STATUS, &info);

	/* don't leave copies of encryption key on stack */
	memset(&info, 0, sizeof(info));
	return r;
}

int loop_get_status64_ioctl(int fd, struct loop_info64 *info64)
{
	struct loop_info info;
	int r;

	memset(info64, 0, sizeof(*info64));
	r = ioctl(fd, LOOP_GET_STATUS64, info64);
	if (!r)
		return 0;
	r = ioctl(fd, LOOP_GET_STATUS, &info);
	if (!r)
		convert_info_to_info64(&info, info64);

	/* don't leave copies of encryption key on stack */
	memset(&info, 0, sizeof(info));
	return r;
}

/* returns: 1=unused 0=busy */
int is_unused_loop_device(int fd)
{
	struct loop_info64 info64;
	struct loop_info info;
	int r;

	r = ioctl(fd, LOOP_GET_STATUS64, &info64);
	memset(&info64, 0, sizeof(info64));
	if (!r)
		return 0;
	if (errno == ENXIO)
		return 1;

	r = ioctl(fd, LOOP_GET_STATUS, &info);
	memset(&info, 0, sizeof(info));
	if (!r)
		return 0;
	if (errno == ENXIO)
		return 1;
	if (errno == EOVERFLOW)
		return 0;
	return 1;
}

struct loop_crypt_type_struct loop_crypt_type_tbl[] = {
	{  0, 0,  0, "no" },
	{  0, 0,  0, "none" },
	{  1, 0,  0, "xor" },
	{  3, 1, 16, "twofish" },
	{  4, 1, 16, "blowfish" },
	{  7, 1, 16, "serpent" },
	{  8, 1, 16, "mars" },
	{ 11, 3, 16, "rc6" },
	{ 12, 0, 21, "tripleDES" },
	{ 12, 0, 24, "3des" },
	{ 12, 0, 24, "des3_ede" },
	{ 16, 1, 16, "AES" },
	{ -1, 0,  0, NULL }
};

static char *getApiName(char *e, int *len)
{
	int x, y, z = 1, q = -1;
	unsigned char *s;

	*len = y = 0;
	s = (unsigned char *)strdup(e);
	if(!s)
		return "";
	x = strlen((char *)s);
	while(x > 0) {
		x--;
		if(!isdigit(s[x]))
			break;
		y += (s[x] - '0') * z;
		z *= 10;
		q = x;
	}
	while(x >= 0) {
		s[x] = tolower(s[x]);
		if(s[x] == '-')
			s[x] = 0;
		x--;
	}
	if(y >= 40) {
		if(q >= 0)
			s[q] = 0;
		*len = y;
	}
	return((char *)s);
}

int loop_crypt_type(const char *name, u_int32_t *kbyp, char **apiName)
{
	int i, k;

	*apiName = getApiName((char *)name, &k);
	if(k < 0)
		k = 0;
	if(k > 256)
		k = 256;
	for (i = 0; loop_crypt_type_tbl[i].id != -1; i++) {
		if (!strcasecmp (*apiName , loop_crypt_type_tbl[i].name)) {
			*kbyp = k ? k >> 3 : loop_crypt_type_tbl[i].keyBytes;
			return loop_crypt_type_tbl[i].id;
		}
	}
	*kbyp = 16; /* 128 bits */
	return 18; /* LO_CRYPT_CRYPTOAPI */
}

int try_cryptoapi_loop_interface(int fd, struct loop_info64 *loopinfo, char *apiName)
{
	snprintf((char *)loopinfo->lo_crypt_name, sizeof(loopinfo->lo_crypt_name), "%s-cbc", apiName);
	loopinfo->lo_crypt_name[LO_NAME_SIZE - 1] = 0;
	loopinfo->lo_encrypt_type = 18; /* LO_CRYPT_CRYPTOAPI */
	return(loop_set_status64_ioctl(fd, loopinfo));
}

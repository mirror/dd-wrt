/*
 *  loop.h
 *
 *  Copyright 2003 by Jari Ruusu.
 *  Redistribution of this file is permitted under the GNU GPL
 */

#ifndef _LOOP_H
#define _LOOP_H 1

#include <sys/types.h>
#include <linux/version.h>
#include <linux/posix_types.h>

#define LO_CRYPT_NONE   0
#define LO_CRYPT_XOR    1
#define LO_CRYPT_DES    2
#define LO_CRYPT_CRYPTOAPI 18

#define LOOP_SET_FD		0x4C00
#define LOOP_CLR_FD		0x4C01
#define LOOP_SET_STATUS		0x4C02
#define LOOP_GET_STATUS		0x4C03
#define LOOP_SET_STATUS64	0x4C04
#define LOOP_GET_STATUS64	0x4C05
#define LOOP_MULTI_KEY_SETUP 	0x4C4D
#define LOOP_MULTI_KEY_SETUP_V3	0x4C4E
#define LOOP_RECOMPUTE_DEV_SIZE 0x4C52

#define LO_NAME_SIZE    64
#define LO_KEY_SIZE     32

struct loop_info {
	int		lo_number;
#if LINUX_VERSION_CODE >= 0x20600
	__kernel_old_dev_t lo_device;
#else
	__kernel_dev_t	lo_device;
#endif
	unsigned long	lo_inode;
#if LINUX_VERSION_CODE >= 0x20600
	__kernel_old_dev_t lo_rdevice;
#else
	__kernel_dev_t	lo_rdevice;
#endif
	int		lo_offset;
	int		lo_encrypt_type;
	int		lo_encrypt_key_size;
	int		lo_flags;
	char		lo_name[LO_NAME_SIZE];
	unsigned char	lo_encrypt_key[LO_KEY_SIZE];
	unsigned long	lo_init[2];
	char		reserved[4];
};

struct loop_info64 {
	u_int64_t	lo_device; 		/* ioctl r/o */
	u_int64_t	lo_inode; 		/* ioctl r/o */
	u_int64_t	lo_rdevice; 		/* ioctl r/o */
	u_int64_t	lo_offset;		/* bytes */
	u_int64_t	lo_sizelimit;		/* bytes, 0 == max available */
	u_int32_t	lo_number;		/* ioctl r/o */
	u_int32_t	lo_encrypt_type;
	u_int32_t	lo_encrypt_key_size; 	/* ioctl w/o */
	u_int32_t	lo_flags;		/* ioctl r/o */
	unsigned char	lo_file_name[LO_NAME_SIZE];
	unsigned char	lo_crypt_name[LO_NAME_SIZE];
	unsigned char	lo_encrypt_key[LO_KEY_SIZE]; /* ioctl w/o */
	u_int64_t	lo_init[2];
};

extern int loop_set_status64_ioctl(int, struct loop_info64 *);
extern int loop_get_status64_ioctl(int, struct loop_info64 *);
extern int is_unused_loop_device(int);

struct loop_crypt_type_struct {
	short int id;
	unsigned char flags; /* bit0 = show keybits, bit1 = add '-' before keybits */
	unsigned char keyBytes;
	char *name;
};

extern struct loop_crypt_type_struct loop_crypt_type_tbl[];
extern int loop_crypt_type(const char *, u_int32_t *, char **);
extern int try_cryptoapi_loop_interface(int, struct loop_info64 *, char *);

#endif

/*
 * Copyright (C) 2009 Gabor Juhos <juhosg@openwrt.org>
 *
 * This tool was based on:
 *   TP-Link WR941 V2 firmware checksum fixing tool.
 *   Copyright (C) 2008,2009 Wang Jian <lark@linux.net.cn>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>     /* for unlink() */
#include <libgen.h>
#include <getopt.h>     /* for getopt() */
#include <stdarg.h>
#include <errno.h>
#include <sys/stat.h>

#include "md5.h"

#define bswap_32(x) \
	((uint32_t)( \
			(((uint32_t)(x) & (uint32_t)0x000000ffUL) << 24) | \
			(((uint32_t)(x) & (uint32_t)0x0000ff00UL) <<  8) | \
			(((uint32_t)(x) & (uint32_t)0x00ff0000UL) >>  8) | \
			(((uint32_t)(x) & (uint32_t)0xff000000UL) >> 24) ))


#if (__BYTE_ORDER == __BIG_ENDIAN)
#  define HOST_TO_BE32(x)	(x)
#  define BE32_TO_HOST(x)	(x)
#else
#  define HOST_TO_BE32(x)	bswap_32(x)
#  define BE32_TO_HOST(x)	bswap_32(x)
#endif

#define HEADER_VERSION_V1	0x01000000
#define HWID_TL_WR741ND_V1	0x07410001
#define HWID_TL_WR741ND_V4	0x07410004
#define HWID_TL_WR740ND_V1	0x07400001
#define HWID_TL_WR740ND_V3	0x07400003
#define HWID_TL_WR740ND_V3WW	0x07400300
#define HWID_TL_WR740ND_V4	0x07400004
#define HWID_TL_WR740ND_V5	0x07400005
#define HWID_TL_WA701ND_V1	0x07010001
#define HWID_TL_WA701ND_V2	0x07010002
#define HWID_TL_WR703N_V1	0x07030101
#define HWID_TL_MR3020		0x30200001
#define HWID_TL_MR3220		0x32200001
#define HWID_TL_MR3220_V2		0x32200002
#define HWID_TL_MR3420		0x34200001
#define HWID_TL_WR743ND_V1	0x07430001
#define HWID_TL_WR743ND_V2	0x07430002
#define HWID_TL_WA901ND_V1	0x09010001
#define HWID_TL_WA901ND_V3	0x09010003
#define HWID_TL_WA901ND_V2	0x09010002
#define HWID_TL_WA901ND_V4	0x09010004
#define HWID_TL_WA901ND_V5	0x09010005
#define HWID_TL_WA801ND_V1	0x08010001
#define HWID_TL_WR840N_V1	0x08400001
#define HWID_TL_WR842ND_V1	0x08420001
#define HWID_TL_WR842ND_V2	0x08420002
#define HWID_TL_WR841N_V1_5	0x08410002
#define HWID_TL_WR841ND_V3	0x08410003
#define HWID_TL_WR841ND_V5	0x08410005
#define HWID_TL_WR841ND_V7	0x08410007
#define HWID_TL_WR810N_V1	0x08100001
#define HWID_TL_WR810N_V2	0x08100002
#define HWID_TL_WNRT627_V1	0x08410007
#define HWID_TL_WR841ND_V8	0x08410008
#define HWID_TL_WR841ND_V9	0x08410009
#define HWID_TL_WR841ND_V10	0x08410010
#define HWID_TL_WR841ND_V11	0x08410011
#define HWID_TL_WR841ND_V12	0x08410012
#define HWID_TL_WR941ND_V2	0x09410002
#define HWID_TL_WR941ND_V4	0x09410004
#define HWID_TL_WR941ND_V6	0x09410006
#define HWID_TL_WR940ND_V4	0x09400004
#define HWID_TL_WR940ND_V6	0x09400006
#define HWID_TL_WR1043ND_V1	0x10430001
#define HWID_TL_WR1043ND_V2	0x10430002
#define HWID_TL_WR1043ND_V3	0x10430003
#define HWID_TL_WDR4300_V1	0x43000001
#define HWID_TL_WDR4310_V1	0x43100001
#define HWID_TL_WDR3500_V1	0x35000001
#define HWID_TL_WDR3600_V1	0x36000001
#define HWID_TL_WDR2543_V1	0x25430001
#define HWID_TL_WA7510N_V1	0x75100001
#define HWID_ARCHERC7_V1	0x75000001
#define HWID_ARCHERC7_V2	0xc7000002
#define HWID_ARCHERC5_V1	0xc5000001
#define HWID_TL_WDR4900_V1	0x49000001
#define HWID_TL_WDR4900_V2	0x49000002
#define HWID_TL_WR710N_V1	0x07100001
#define HWID_TL_WR710N_V2	0x07100002

#define MD5SUM_LEN	16

struct file_info {
	char		*file_name;	/* name of the file */
	uint32_t	file_size;	/* length of the file */
};

struct fw_header {
	uint32_t	version;	/* header version */
	char		vendor_name[24];
	char		fw_version[36];
	uint32_t	hw_id;		/* hardware id */
	uint32_t	hw_rev;		/* hardware revision */
	uint32_t	area_code;
	uint8_t		md5sum1[MD5SUM_LEN];
	uint32_t	unk2;
	uint8_t		md5sum2[MD5SUM_LEN];
	uint32_t	unk3;
	uint32_t	kernel_la;	/* kernel load address */
	uint32_t	kernel_ep;	/* kernel entry point */
	uint32_t	fw_length;	/* total length of the firmware */
	uint32_t	kernel_ofs;	/* kernel data offset */
	uint32_t	kernel_len;	/* kernel data length */
	uint32_t	rootfs_ofs;	/* rootfs data offset */
	uint32_t	rootfs_len;	/* rootfs data length */
	uint32_t	boot_ofs;	/* bootloader data offset */
	uint32_t	boot_len;	/* bootloader data length */
	uint16_t	ver_hi;
	uint16_t	ver_mid;
	uint16_t	ver_lo;
	uint8_t		pad[130];
	uint8_t		supportlist[32];
	uint8_t		specialid[8];
	uint8_t		pad2[184];
} __attribute__ ((packed));

struct board_info {
	char		*id;
	uint32_t	hw_id;
	uint32_t	hw_rev;
	uint32_t	area_code;
	uint32_t	fw_max_len;
	uint32_t	kernel_la;
	uint32_t	kernel_ep;
	uint32_t	rootfs_ofs;
};

/*
 * Globals
 */
static char *ofname;
static char *progname;
static char *vendor = "TP-LINK Technologies";
static char *version = "ver. 1.0";
static char *fw_ver = "0.0.0";

static char *board_id;
static struct board_info *board;
static struct file_info kernel_info;
static struct file_info rootfs_info;
static struct file_info boot_info;
static int combined;

static int fw_ver_lo;
static int fw_ver_mid;
static int fw_ver_hi;

char md5salt_normal[MD5SUM_LEN] = {
	0xdc, 0xd7, 0x3a, 0xa5, 0xc3, 0x95, 0x98, 0xfb,
	0xdd, 0xf9, 0xe7, 0xf4, 0x0e, 0xae, 0x47, 0x38,
};

char md5salt_boot[MD5SUM_LEN] = {
	0x8c, 0xef, 0x33, 0x5b, 0xd5, 0xc5, 0xce, 0xfa,
	0xa7, 0x9c, 0x28, 0xda, 0xb2, 0xe9, 0x0f, 0x42,
};

static struct board_info boards[] = {
	{
		.id		= "TL-WR740NDv1",
		.hw_id		= HWID_TL_WR740ND_V1,
		.hw_rev		= 1,
		.fw_max_len	= 0x3c0000,
		.kernel_la	= 0x80060000,
		.kernel_ep	= 0x80060000,
		.rootfs_ofs	= 0xf0000,
	},   {
		.id		= "TL-WR740NDv3",
		.hw_id		= HWID_TL_WR740ND_V3,
		.hw_rev		= 1,
		.fw_max_len	= 0x3c0000,
		.kernel_la	= 0x80060000,
		.kernel_ep	= 0x80060000,
		.rootfs_ofs	= 0xf0000,
	},   {
		.id		= "TL-WR740NDv3WW",
		.hw_id		= HWID_TL_WR740ND_V3WW,
		.hw_rev		= 1,
		.fw_max_len	= 0x3c0000,
		.kernel_la	= 0x80060000,
		.kernel_ep	= 0x80060000,
		.rootfs_ofs	= 0xf0000,
	},	{
		.id		= "TL-WR740NDv4",
		.hw_id		= HWID_TL_WR740ND_V4,
		.hw_rev		= 1,
		.fw_max_len	= 0x3c0000,
		.kernel_la	= 0x80060000,
		.kernel_ep	= 0x80060000,
		.rootfs_ofs	= 0xf0000,
	},	{
		.id		= "TL-WR740NDv5",
		.hw_id		= HWID_TL_WR740ND_V5,
		.hw_rev		= 1,
		.fw_max_len	= 0x3c0000,
		.kernel_la	= 0x80060000,
		.kernel_ep	= 0x80060000,
		.rootfs_ofs	= 0xf0000,
	},	{
		.id		= "TL-WR842NDv1",
		.hw_id		= HWID_TL_WR842ND_V1,
		.hw_rev		= 1,
		.fw_max_len	= 0x7c0000,
		.kernel_la	= 0x80060000,
		.kernel_ep	= 0x80060000,
		.rootfs_ofs	= 0x100000,
	}, {
		.id		= "TL-WR842NDv2",
		.hw_id		= HWID_TL_WR842ND_V2,
		.hw_rev		= 1,
		.fw_max_len	= 0x7c0000,
		.kernel_la	= 0x80060000,
		.kernel_ep	= 0x80060000,
		.rootfs_ofs	= 0xf0000,
	},	{
		.id		= "TL-WR741NDv1",
		.hw_id		= HWID_TL_WR741ND_V1,
		.hw_rev		= 1,
		.fw_max_len	= 0x3c0000,
		.kernel_la	= 0x80060000,
		.kernel_ep	= 0x80060000,
		.rootfs_ofs	= 0xf0000,
	},	{
		.id		= "TL-WA7510N",
		.hw_id		= HWID_TL_WA7510N_V1,
		.hw_rev		= 1,
		.fw_max_len	= 0x3c0000,
		.kernel_la	= 0x80060000,
		.kernel_ep	= 0x80060000,
		.rootfs_ofs	= 0xf0000,
	},	{
		.id		= "AIP-W411",
		.hw_id		= HWID_TL_WR741ND_V1,
		.hw_rev		= 0x00180001,
		.fw_max_len	= 0x3c0000,
		.kernel_la	= 0x80060000,
		.kernel_ep	= 0x80060000,
		.rootfs_ofs	= 0xf0000,
	},	{
		.id		= "TL-WR741NDv4",
		.hw_id		= HWID_TL_WR741ND_V4,
		.hw_rev		= 1,
		.fw_max_len	= 0x3c0000,
		.kernel_la	= 0x80060000,
		.kernel_ep	= 0x80060000,
		.rootfs_ofs	= 0xf0000,
	},	{
		.id		= "TL-MR3220v2",
		.hw_id		= HWID_TL_MR3220_V2,
		.hw_rev		= 1,
		.fw_max_len	= 0x3c0000,
		.kernel_la	= 0x80060000,
		.kernel_ep	= 0x80060000,
		.rootfs_ofs	= 0xf0000,
	},	{
		.id		= "TL-WR743NDv2",
		.hw_id		= HWID_TL_WR743ND_V2,
		.hw_rev		= 1,
		.fw_max_len	= 0x3c0000,
		.kernel_la	= 0x80060000,
		.kernel_ep	= 0x80060000,
		.rootfs_ofs	= 0xf0000,
	},	{
		.id		= "TL-WR703Nv1",
		.hw_id		= HWID_TL_WR703N_V1,
		.hw_rev		= 1,
		.fw_max_len	= 0x3c0000,
		.kernel_la	= 0x80060000,
		.kernel_ep	= 0x80060000,
		.rootfs_ofs	= 0xf0000,
	},	{
		.id		= "TL-WA701NDv1",
		.hw_id		= HWID_TL_WA701ND_V1,
		.hw_rev		= 1,
		.fw_max_len	= 0x3c0000,
		.kernel_la	= 0x80060000,
		.kernel_ep	= 0x80060000,
		.rootfs_ofs	= 0xf0000,
	},	{
		.id		= "TL-WA701NDv2",
		.hw_id		= HWID_TL_WA701ND_V2,
		.hw_rev		= 1,
		.fw_max_len	= 0x3c0000,
		.kernel_la	= 0x80060000,
		.kernel_ep	= 0x80060000,
		.rootfs_ofs	= 0xf0000,
	},	{
		.id		= "TL-MR3020",
		.hw_id		= HWID_TL_MR3020,
		.hw_rev		= 1,
		.fw_max_len	= 0x3c0000,
		.kernel_la	= 0x80060000,
		.kernel_ep	= 0x80060000,
		.rootfs_ofs	= 0xf0000,
	},	{
		.id		= "TL-MR3420",
		.hw_id		= HWID_TL_MR3420,
		.hw_rev		= 1,
		.fw_max_len	= 0x3c0000,
		.kernel_la	= 0x80060000,
		.kernel_ep	= 0x80060000,
		.rootfs_ofs	= 0xf0000,
	},	{
		.id		= "TL-MR3220",
		.hw_id		= HWID_TL_MR3220,
		.hw_rev		= 1,
		.fw_max_len	= 0x3c0000,
		.kernel_la	= 0x80060000,
		.kernel_ep	= 0x80060000,
		.rootfs_ofs	= 0xf0000,
	},	{
		.id		= "TL-WR743NDv1",
		.hw_id		= HWID_TL_WR743ND_V1,
		.hw_rev		= 1,
		.fw_max_len	= 0x3c0000,
		.kernel_la	= 0x80060000,
		.kernel_ep	= 0x80060000,
		.rootfs_ofs	= 0xf0000,
	}, 	{
		.id		= "TL-WA901NDv1",
		.hw_id		= HWID_TL_WA901ND_V1,
		.hw_rev		= 1,
		.fw_max_len	= 0x3c0000,
		.kernel_la	= 0x80060000,
		.kernel_ep	= 0x80060000,
		.rootfs_ofs	= 0xf0000,
	}, 	{
		.id		= "TL-WA801NDv1",
		.hw_id		= HWID_TL_WA801ND_V1,
		.hw_rev		= 1,
		.fw_max_len	= 0x3c0000,
		.kernel_la	= 0x80060000,
		.kernel_ep	= 0x80060000,
		.rootfs_ofs	= 0xf0000,
	}, {
		.id		= "TL-WA901NDv2",
		.hw_id		= HWID_TL_WA901ND_V2,
		.hw_rev		= 1,
		.fw_max_len	= 0x3c0000,
		.kernel_la	= 0x80060000,
		.kernel_ep	= 0x80060000,
		.rootfs_ofs	= 0xf0000,
	}, {
		.id		= "TL-WR841Nv1.5",
		.hw_id		= HWID_TL_WR841N_V1_5,
		.hw_rev		= 2,
		.fw_max_len	= 0x3c0000,
		.kernel_la	= 0x80060000,
		.kernel_ep	= 0x80060000,
		.rootfs_ofs	= 0xf0000,
	}, {
		.id		= "TL-WR841NDv3",
		.hw_id		= HWID_TL_WR841ND_V3,
		.hw_rev		= 3,
		.fw_max_len	= 0x3c0000,
		.kernel_la	= 0x80060000,
		.kernel_ep	= 0x80060000,
		.rootfs_ofs	= 0xf0000,
	}, {
		.id		= "TL-WR841NDv5",
		.hw_id		= HWID_TL_WR841ND_V5,
		.hw_rev		= 1,
		.fw_max_len	= 0x3c0000,
		.kernel_la	= 0x80060000,
		.kernel_ep	= 0x80060000,
		.rootfs_ofs	= 0xf0000,
	}, {
		.id		= "TL-WR841NDv7",
		.hw_id		= HWID_TL_WR841ND_V7,
		.hw_rev		= 1,
		.fw_max_len	= 0x3c0000,
		.kernel_la	= 0x80060000,
		.kernel_ep	= 0x80060000,
		.rootfs_ofs	= 0xf0000,
	}, {
		.id		= "WNRT627V1",
		.hw_id		= HWID_TL_WR841ND_V7,
		.hw_rev		= 0x00260001,
		.fw_max_len	= 0x3c0000,
		.kernel_la	= 0x80060000,
		.kernel_ep	= 0x80060000,
		.rootfs_ofs	= 0xf0000,
	}, {
		.id		= "TL-WR841NDv8",
		.hw_id		= HWID_TL_WR841ND_V8,
		.hw_rev		= 1,
		.fw_max_len	= 0x3c0000,
		.kernel_la	= 0x80060000,
		.kernel_ep	= 0x80060000,
		.rootfs_ofs	= 0xf0000,
	}, {
		.id		= "TL-WA901NDv3",
		.hw_id		= HWID_TL_WA901ND_V3,
		.hw_rev		= 1,
		.fw_max_len	= 0x3c0000,
		.kernel_la	= 0x80060000,
		.kernel_ep	= 0x80060000,
		.rootfs_ofs	= 0xf0000,
	}, {
		.id		= "TL-WA901NDv4",
		.hw_id		= HWID_TL_WA901ND_V4,
		.hw_rev		= 1,
		.fw_max_len	= 0x3c0000,
		.kernel_la	= 0x80060000,
		.kernel_ep	= 0x80060000,
		.rootfs_ofs	= 0xf0000,
	}, {
		.id		= "TL-WA901NDv5",
		.hw_id		= HWID_TL_WA901ND_V5,
		.hw_rev		= 1,
		.fw_max_len	= 0x3c0000,
		.kernel_la	= 0x80060000,
		.kernel_ep	= 0x80060000,
		.rootfs_ofs	= 0xf0000,
	}, {
		.id		= "TL-WR841NDv9",
		.hw_id		= HWID_TL_WR841ND_V9,
		.hw_rev		= 1,
		.fw_max_len	= 0x3c0000,
		.kernel_la	= 0x80060000,
		.kernel_ep	= 0x80060000,
		.rootfs_ofs	= 0xf0000,
	}, {
		.id		= "TL-WR841NDv10",
		.hw_id		= HWID_TL_WR841ND_V10,
		.hw_rev		= 1,
		.fw_max_len	= 0x3c0000,
		.kernel_la	= 0x80060000,
		.kernel_ep	= 0x80060000,
		.rootfs_ofs	= 0xf0000,
	}, {
		.id		= "TL-WR841NDv11",
		.hw_id		= HWID_TL_WR841ND_V11,
		.hw_rev		= 1,
		.fw_max_len	= 0x3c0000,
		.kernel_la	= 0x80060000,
		.kernel_ep	= 0x80060000,
		.rootfs_ofs	= 0xf0000,
	}, {
		.id		= "TL-WR841NDv12",
		.hw_id		= HWID_TL_WR841ND_V12,
		.hw_rev		= 1,
		.fw_max_len	= 0x3c0000,
		.kernel_la	= 0x80060000,
		.kernel_ep	= 0x80060000,
		.rootfs_ofs	= 0xf0000,
	}, {
		.id		= "RNX-N300RT",
		.hw_id		= HWID_TL_WR841ND_V7,
		.hw_rev		= 0x00420001,
		.fw_max_len	= 0x3c0000,
		.kernel_la	= 0x80060000,
		.kernel_ep	= 0x80060000,
		.rootfs_ofs	= 0xf0000,
	}, {
		.id		= "RNX-N150RT",
		.hw_id		= HWID_TL_WR741ND_V1,
		.hw_rev		= 0x00420001,
		.fw_max_len	= 0x3c0000,
		.kernel_la	= 0x80060000,
		.kernel_ep	= 0x80060000,
		.rootfs_ofs	= 0xf0000,
	}, {
		.id		= "TL-WR840Nv1",
		.hw_id		= HWID_TL_WR840N_V1,
		.hw_rev		= 1,
		.fw_max_len	= 0x3c0000,
		.kernel_la	= 0x80060000,
		.kernel_ep	= 0x80060000,
		.rootfs_ofs	= 0xf0000,
	}, {
		.id		= "TL-WR941NDv2",
		.hw_id		= HWID_TL_WR941ND_V2,
		.hw_rev		= 2,
		.fw_max_len	= 0x3c0000,
		.kernel_la	= 0x80060000,
		.kernel_ep	= 0x80060000,
		.rootfs_ofs	= 0xf0000,
	}, {
		.id		= "TL-WR941NDv4",
		.hw_id		= HWID_TL_WR941ND_V4,
		.hw_rev		= 1,
		.fw_max_len	= 0x3c0000,
		.kernel_la	= 0x80060000,
		.kernel_ep	= 0x80060000,
		.rootfs_ofs	= 0xf0000,
	}, {
		.id		= "TL-WR941NDv6",
		.hw_id		= HWID_TL_WR941ND_V6,
		.hw_rev		= 1,
		.fw_max_len	= 0x3c0000,
		.kernel_la	= 0x80060000,
		.kernel_ep	= 0x80060000,
		.rootfs_ofs	= 0xf0000,
	}, {
		.id		= "TL-WR940NDv4",
		.hw_id		= HWID_TL_WR940ND_V4,
		.hw_rev		= 1,
		.fw_max_len	= 0x3c0000,
		.kernel_la	= 0x80060000,
		.kernel_ep	= 0x80060000,
		.rootfs_ofs	= 0xf0000,
	}, {
		.id		= "TL-WR940NDv6",
		.hw_id		= HWID_TL_WR940ND_V6,
		.hw_rev		= 1,
		.fw_max_len	= 0x3c0000,
		.kernel_la	= 0x80060000,
		.kernel_ep	= 0x80060000,
		.rootfs_ofs	= 0xf0000,
	}, {
		.id		= "TL-WR1043NDv1",
		.hw_id		= HWID_TL_WR1043ND_V1,
		.hw_rev		= 1,
		.fw_max_len	= 0x7b0000,
		.kernel_la	= 0x80060000,
		.kernel_ep	= 0x80060000,
		.rootfs_ofs	= 0xf0000,
	}, {
		.id		= "TL-WR1043NDv2",
		.hw_id		= HWID_TL_WR1043ND_V2,
		.hw_rev		= 1,
		.fw_max_len	= 0x7b0000,
		.kernel_la	= 0x80060000,
		.kernel_ep	= 0x80060000,
		.rootfs_ofs	= 0xf0000,
	}, {
		.id		= "TL-WR1043NDv3",
		.hw_id		= HWID_TL_WR1043ND_V3,
		.hw_rev		= 1,
		.fw_max_len	= 0x7b0000,
		.kernel_la	= 0x80060000,
		.kernel_ep	= 0x80060000,
		.rootfs_ofs	= 0xf0000,
	}, {
		.id		= "ARCHER-C7v1",
		.hw_id		= HWID_ARCHERC7_V1,
		.hw_rev		= 1,
		.fw_max_len	= 0x7b0000,
		.kernel_la	= 0x80060000,
		.kernel_ep	= 0x80060000,
		.rootfs_ofs	= 0xf0000,
	}, {
		.id		= "TL-WR710Nv1",
		.hw_id		= HWID_TL_WR710N_V1,
		.hw_rev		= 1,
		.fw_max_len	= 0x7b0000,
		.kernel_la	= 0x80060000,
		.kernel_ep	= 0x80060000,
		.rootfs_ofs	= 0x100000,
	}, {
		.id		= "TL-WR710Nv2",
		.hw_id		= HWID_TL_WR710N_V2,
		.hw_rev		= 1,
		.fw_max_len	= 0x3c0000,
		.kernel_la	= 0x80060000,
		.kernel_ep	= 0x80060000,
		.rootfs_ofs	= 0xf0000,
	}, {
		.id		= "TL-WR710Nv2.1",
		.hw_id		= HWID_TL_WR710N_V2,
		.hw_rev		= 2,
		.fw_max_len	= 0x7b0000,
		.kernel_la	= 0x80060000,
		.kernel_ep	= 0x80060000,
		.rootfs_ofs	= 0x100000,
	}, {
		.id		= "TL-WR810Nv1",
		.hw_id		= HWID_TL_WR810N_V1,
		.hw_rev		= 1,
		.fw_max_len	= 0x7b0000,
		.kernel_la	= 0x80060000,
		.kernel_ep	= 0x80060000,
		.rootfs_ofs	= 0x100000,
	}, {
		.id		= "TL-WR810Nv2",
		.hw_id		= HWID_TL_WR810N_V2,
		.hw_rev		= 1,
		.fw_max_len	= 0x7b0000,
		.kernel_la	= 0x80060000,
		.kernel_ep	= 0x80060000,
		.rootfs_ofs	= 0x100000,
	}, {
		.id		= "ARCHER-C7v2",
		.hw_id		= HWID_ARCHERC7_V2,
		.hw_rev		= 1,
		.fw_max_len	= 0xfb0000,
		.kernel_la	= 0x80060000,
		.kernel_ep	= 0x80060000,
		.rootfs_ofs	= 0xf0000,
	}, {
		.id		= "ARCHER-C7v3",
		.hw_id		= HWID_ARCHERC7_V2,
		.hw_rev		= 1,
		.fw_max_len	= 0xfb0000,
		.kernel_la	= 0x80060000,
		.kernel_ep	= 0x80060000,
		.rootfs_ofs	= 0xf0000,
	}, {
		.id		= "ARCHER-C7v2Israel",
		.hw_id		= HWID_ARCHERC7_V2,
		.hw_rev		= 0x494C0001,
		.fw_max_len	= 0xfb0000,
		.kernel_la	= 0x80060000,
		.kernel_ep	= 0x80060000,
		.rootfs_ofs	= 0xf0000,
	}, {
		.id		= "ARCHER-C5v1",
		.hw_id		= HWID_ARCHERC5_V1,
		.hw_rev		= 1,
		.fw_max_len	= 0xfb0000,
		.kernel_la	= 0x80060000,
		.kernel_ep	= 0x80060000,
		.rootfs_ofs	= 0xf0000,
	}, {
		.id		= "ARCHER-C5v1Israel",
		.hw_id		= HWID_ARCHERC5_V1,
		.hw_rev		= 0x494C0001,
		.fw_max_len	= 0xfb0000,
		.kernel_la	= 0x80060000,
		.kernel_ep	= 0x80060000,
		.rootfs_ofs	= 0xf0000,
	}, {
		.id		= "TL-WDR4300",
		.hw_id		= HWID_TL_WDR4300_V1,
		.hw_rev		= 1,
		.fw_max_len	= 0x7b0000,
		.kernel_la	= 0x80060000,
		.kernel_ep	= 0x80060000,
		.rootfs_ofs	= 0x100000,
	}, {
		.id		= "TL-WDR4310",
		.hw_id		= HWID_TL_WDR4310_V1,
		.hw_rev		= 1,
		.fw_max_len	= 0x7b0000,
		.kernel_la	= 0x80060000,
		.kernel_ep	= 0x80060000,
		.rootfs_ofs	= 0x100000,
	}, {
		.id		= "TL-WDR4900",
		.hw_id		= HWID_TL_WDR4900_V1,
		.hw_rev		= 1,
		.fw_max_len	= 0xf60000,
		.kernel_la	= 0x00000000,
		.kernel_ep	= 0xc0000000,
		.rootfs_ofs	= 0x200000,
	}, {
		.id		= "TL-WDR4900v2",
		.hw_id		= HWID_TL_WDR4900_V2,
		.hw_rev		= 1,
		.fw_max_len	= 0x7b0000,
		.kernel_la	= 0x80060000,
		.kernel_ep	= 0x80060000,
		.rootfs_ofs	= 0xf0000,
	}, {
		.id		= "TL-WDR3600",
		.hw_id		= HWID_TL_WDR3600_V1,
		.hw_rev		= 1,
		.fw_max_len	= 0x7b0000,
		.kernel_la	= 0x80060000,
		.kernel_ep	= 0x80060000,
		.rootfs_ofs	= 0x100000,
	}, {
		.id		= "TL-WDR3500",
		.hw_id		= HWID_TL_WDR3500_V1,
		.hw_rev		= 1,
		.fw_max_len	= 0x7b0000,
		.kernel_la	= 0x80060000,
		.kernel_ep	= 0x80060000,
		.rootfs_ofs	= 0x100000,
	}, {
		.id		= "TL-WDR2543",
		.hw_id		= HWID_TL_WDR2543_V1,
		.hw_rev		= 1,
		.fw_max_len	= 0x7b0000,
		.kernel_la	= 0x80060000,
		.kernel_ep	= 0x80060000,
		.rootfs_ofs	= 0xf0000,
	}, {
		/* terminating entry */
	}
};

/*
 * Message macros
 */
#define ERR(fmt, ...) do { \
	fflush(0); \
	fprintf(stderr, "[%s] *** error: " fmt "\n", \
			progname, ## __VA_ARGS__ ); \
} while (0)

#define ERRS(fmt, ...) do { \
	int save = errno; \
	fflush(0); \
	fprintf(stderr, "[%s] *** error: " fmt "\n", \
			progname, ## __VA_ARGS__, strerror(save)); \
} while (0)

#define DBG(fmt, ...) do { \
	fprintf(stderr, "[%s] " fmt "\n", progname, ## __VA_ARGS__ ); \
} while (0)

static struct board_info *find_board(char *id)
{
	struct board_info *ret;
	struct board_info *board;

	ret = NULL;
	for (board = boards; board->id != NULL; board++){
		if (strcasecmp(id, board->id) == 0) {
			ret = board;
			break;
		}
	};

	return ret;
}

static void usage(int status)
{
	FILE *stream = (status != EXIT_SUCCESS) ? stderr : stdout;
	struct board_info *board;

	fprintf(stream, "Usage: %s [OPTIONS...]\n", progname);
	fprintf(stream,
"\n"
"Options:\n"
"  -B <board>      create image for the board specified with <board>\n"
"  -c              use combined kernel image\n"
"  -k <file>       read kernel image from the file <file>\n"
"  -r <file>       read rootfs image from the file <file>\n"
"  -o <file>       write output to the file <file>\n"
"  -v <version>    set image version to <version>\n"
"  -h              show this screen\n"
	);

	exit(status);
}

static int get_md5(char *data, int size, char *md5)
{
	MD5_CTX ctx;

	MD5_Init(&ctx);
	MD5_Update(&ctx, data, size);
	MD5_Final(md5, &ctx);
}

static int get_file_stat(struct file_info *fdata)
{
	struct stat st;
	int res;

	if (fdata->file_name == NULL)
		return 0;

	res = stat(fdata->file_name, &st);
	if (res){
		ERRS("stat failed on %s", fdata->file_name);
		return res;
	}

	fdata->file_size = st.st_size;
	return 0;
}

static int read_to_buf(struct file_info *fdata, char *buf)
{
	FILE *f;
	int ret = EXIT_FAILURE;

	f = fopen(fdata->file_name, "r");
	if (f == NULL) {
		ERRS("could not open \"%s\" for reading", fdata->file_name);
		goto out;
	}

	errno = 0;
	fread(buf, fdata->file_size, 1, f);
	if (errno != 0) {
		ERRS("unable to read from file \"%s\"", fdata->file_name);
		goto out_close;
	}

	ret = EXIT_SUCCESS;

 out_close:
	fclose(f);
 out:
	return ret;
}

static int check_options(void)
{
	int ret;

	if (board_id == NULL) {
		ERR("no board specified");
		return -1;
	}

	board = find_board(board_id);
	if (board == NULL) {
		ERR("unknown/unsupported board id \"%s\"", board_id);
		return -1;
	}

	if (kernel_info.file_name == NULL) {
		ERR("no kernel image specified");
		return -1;
	}

	ret = get_file_stat(&kernel_info);
	if (ret)
		return ret;

	if (combined) {
		if (kernel_info.file_size >
		    board->fw_max_len - sizeof(struct fw_header)) {
			ERR("kernel image is too big");
			return -1;
		}
	} else {
//		if (kernel_info.file_size >
//		    board->rootfs_ofs - sizeof(struct fw_header)) {
//			ERR("kernel image is too big, adjust it");
			board->rootfs_ofs = kernel_info.file_size + sizeof(struct fw_header) + 0xFFF;
			board->rootfs_ofs /= 0x1000;
			board->rootfs_ofs *= 0x1000;
			fprintf(stderr,"new offset is 0x%X\n",board->rootfs_ofs);
//		}
		if (rootfs_info.file_name == NULL) {
			ERR("no rootfs image specified");
			return -1;
		}

		ret = get_file_stat(&rootfs_info);
		if (ret)
			return ret;

		if (rootfs_info.file_size >
                    (board->fw_max_len - board->rootfs_ofs)) {
			ERR("rootfs image is too big (%d bytes less required)", rootfs_info.file_size-(board->fw_max_len - board->rootfs_ofs));
			return -1;
		}
	}

	if (ofname == NULL) {
		ERR("no output file specified");
		return -1;
	}

	ret = sscanf(fw_ver, "%d.%d.%d", &fw_ver_hi, &fw_ver_mid, &fw_ver_lo);
	if (ret != 3) {
		ERR("invalid firmware version '%s'", fw_ver);
		return -1;
	}

	return 0;
}
static int trunkfile=0;
unsigned int area_code = 0;
char *specialid = NULL;
char *supportlist = NULL;
static void fill_header(char *buf, int len)
{
	struct fw_header *hdr = (struct fw_header *)buf;

	memset(hdr, 0, sizeof(struct fw_header));

	hdr->version = HOST_TO_BE32(HEADER_VERSION_V1);
	strncpy(hdr->vendor_name, vendor, sizeof(hdr->vendor_name));
	strncpy(hdr->fw_version, version, sizeof(hdr->fw_version));
	hdr->hw_id = HOST_TO_BE32(board->hw_id);
	hdr->hw_rev = HOST_TO_BE32(board->hw_rev);
	hdr->area_code = HOST_TO_BE32(board->area_code);
	if (supportlist)
		strncpy(hdr->supportlist,supportlist,32);
	if (specialid)
		strncpy(hdr->specialid,specialid,8);
	
    	if (area_code)
	    hdr->area_code = HOST_TO_BE32(area_code);
	
	if (boot_info.file_size == 0)
		memcpy(hdr->md5sum1, md5salt_normal, sizeof(hdr->md5sum1));
	else
		memcpy(hdr->md5sum1, md5salt_boot, sizeof(hdr->md5sum1));

	hdr->kernel_la = HOST_TO_BE32(board->kernel_la);
	hdr->kernel_ep = HOST_TO_BE32(board->kernel_ep);
	if (trunkfile)
	    hdr->fw_length = HOST_TO_BE32(sizeof(struct fw_header) + kernel_info.file_size + rootfs_info.file_size);
	else
	    hdr->fw_length = HOST_TO_BE32(board->fw_max_len);
	hdr->kernel_ofs = HOST_TO_BE32(sizeof(struct fw_header));
	hdr->kernel_len = HOST_TO_BE32(kernel_info.file_size);
	if (!combined) {
		hdr->rootfs_ofs = HOST_TO_BE32(board->rootfs_ofs);
		hdr->rootfs_len = HOST_TO_BE32(rootfs_info.file_size);
	}

	hdr->ver_hi = htons(fw_ver_hi);
	hdr->ver_mid = htons(fw_ver_mid);
	hdr->ver_lo = htons(fw_ver_lo);

	get_md5(buf, len, hdr->md5sum1);
}

static int write_fw(char *data, int len)
{
	FILE *f;
	int ret = EXIT_FAILURE;

	f = fopen(ofname, "w");
	if (f == NULL) {
		ERRS("could not open \"%s\" for writing", ofname);
		goto out;
	}

	errno = 0;
	fwrite(data, len, 1, f);
	if (errno) {
		ERRS("unable to write output file");
		goto out_flush;
	}

	DBG("firmware file \"%s\" completed", ofname);

	ret = EXIT_SUCCESS;

 out_flush:
	fflush(f);
	fclose(f);
	if (ret != EXIT_SUCCESS) {
		unlink(ofname);
	}
 out:
	return ret;
}

static int build_fw(void)
{
	int buflen;
	char *buf;
	char *p;
	int ret = EXIT_FAILURE;

	if (trunkfile)
		buflen = board->rootfs_ofs + rootfs_info.file_size;
	else
		buflen = board->fw_max_len;
	buflen = board->fw_max_len;

	buf = malloc(buflen);
	if (!buf) {
		ERR("no memory for buffer\n");
		goto out;
	}

	memset(buf, 0xff, buflen);
	p = buf + sizeof(struct fw_header);
	ret = read_to_buf(&kernel_info, p);
	if (ret)
		goto out_free_buf;

	if (!combined) {
		p = buf + board->rootfs_ofs;
		ret = read_to_buf(&rootfs_info, p);
		if (ret)
			goto out_free_buf;
	}

	fill_header(buf, buflen);

	ret = write_fw(buf, buflen);
	if (ret)
		goto out_free_buf;

	ret = EXIT_SUCCESS;

 out_free_buf:
	free(buf);
 out:
	return ret;
}

int main(int argc, char *argv[])
{
	int ret = EXIT_FAILURE;
	int err;

	FILE *outfile;

	progname = basename(argv[0]);

	while ( 1 ) {
		int c;

		c = getopt(argc, argv, "B:V:N:us:x:ck:r:o:v:h:t::");
		if (c == -1)
			break;

		switch (c) {
		case 'B':
			board_id = optarg;
			break;
		case 'V':
			version = optarg;
			break;
		case 'N':
			vendor = optarg;
			break;
		case 'c':
			combined++;
			break;
		case 'k':
			kernel_info.file_name = optarg;
			break;
		case 'r':
			rootfs_info.file_name = optarg;
			break;
		case 'o':
			ofname = optarg;
			break;
		case 'u':
			area_code = 1;
			break;
		case 's':
			supportlist = optarg;
			break;
		case 'x':
			specialid = optarg;
			break;
		case 'v':
			fw_ver = optarg;
			break;
		case 't':
			trunkfile = 1;
			break;
		case 'h':
			usage(EXIT_SUCCESS);
			break;
		default:
			usage(EXIT_FAILURE);
			break;
		}
	}

	ret = check_options();
	if (ret)
		goto out;

	ret = build_fw();

 out:
	return ret;
}


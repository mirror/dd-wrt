/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *  Copyright (C) 2020 Hyunchul Lee <hyc.lee@gmail.com>
 */
#ifndef _REPAIR_H
#define _REPAIR_H

#include "exfat_dir.h"

#define ER_BS_CHECKSUM			0x00000001
#define ER_BS_BOOT_REGION		0x00000002
#define ER_DE_CHECKSUM			0x00001001
#define ER_DE_UNKNOWN			0x00001002
#define ER_DE_FILE			0x00001010
#define ER_DE_SECONDARY_COUNT		0x00001011
#define ER_DE_STREAM			0x00001020
#define ER_DE_NAME			0x00001030
#define ER_DE_NAME_HASH			0x00001031
#define ER_DE_NAME_LEN			0x00001032
#define ER_DE_DOT_NAME			0x00001033
#define ER_DE_DUPLICATED_NAME		0x00001034
#define ER_FILE_VALID_SIZE		0x00002001
#define ER_FILE_INVALID_CLUS		0x00002002
#define ER_FILE_FIRST_CLUS		0x00002003
#define ER_FILE_SMALLER_SIZE		0x00002004
#define ER_FILE_LARGER_SIZE		0x00002005
#define ER_FILE_DUPLICATED_CLUS		0x00002006
#define ER_FILE_ZERO_NOFAT		0x00002007
#define ER_VENDOR_GUID			0x00003001

typedef unsigned int er_problem_code_t;
struct exfat_fsck;

int exfat_repair_ask(struct exfat_fsck *fsck, er_problem_code_t prcode,
		     const char *fmt, ...);

int exfat_repair_rename_ask(struct exfat_fsck *fsck, struct exfat_de_iter *iter,
		char *old_name, er_problem_code_t prcode, char *error_msg);
#endif

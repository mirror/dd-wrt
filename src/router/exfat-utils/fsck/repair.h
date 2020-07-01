/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *  Copyright (C) 2020 Hyunchul Lee <hyc.lee@gmail.com>
 */
#ifndef _REPAIR_H
#define _REPAIR_H

#define ER_BS_CHECKSUM			0x00000001
#define ER_DE_CHECKSUM			0x00001001
#define ER_FILE_VALID_SIZE		0x00002001
#define ER_FILE_INVALID_CLUS		0x00002002
#define ER_FILE_FIRST_CLUS		0x00002003
#define ER_FILE_SMALLER_SIZE		0x00002004
#define ER_FILE_LARGER_SIZE		0x00002005
#define ER_FILE_DUPLICATED_CLUS		0x00002006

typedef unsigned int er_problem_code_t;

bool exfat_repair_ask(struct exfat *exfat, er_problem_code_t prcode,
		const char *fmt, ...);

#endif

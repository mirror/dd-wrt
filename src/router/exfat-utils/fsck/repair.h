/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *  Copyright (C) 2020 Hyunchul Lee <hyc.lee@gmail.com>
 */
#ifndef _REPAIR_H
#define _REPAIR_H

#define ER_BS_CHECKSUM			0x00000001

typedef unsigned int er_problem_code_t;

union exfat_repair_context {
	struct {
		__le32		checksum;
		void		*checksum_sect;
	} bs_checksum;
};

bool exfat_repair(struct exfat *exfat, er_problem_code_t prcode,
			union exfat_repair_context *rctx);

#endif

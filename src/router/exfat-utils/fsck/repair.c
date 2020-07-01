/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *  Copyright (C) 2020 Hyunchul Lee <hyc.lee@gmail.com>
 */
#include <stdio.h>
#include <string.h>

#include "exfat_ondisk.h"
#include "libexfat.h"
#include "fsck.h"
#include "repair.h"

struct exfat_repair_problem {
	er_problem_code_t	prcode;
	const char		*description;
	bool (*fix_problem)(struct exfat *exfat,
			union exfat_repair_context *rctx);
};

static bool fix_bs_checksum(struct exfat *exfat,
			union exfat_repair_context *rctx)
{
	unsigned int size;
	unsigned int i;

	size = EXFAT_SECTOR_SIZE(exfat->bs);
	for (i = 0; i < size/sizeof(__le32); i++) {
		((__le32 *)rctx->bs_checksum.checksum_sect)[i] =
				rctx->bs_checksum.checksum;
	}

	if (exfat_write(exfat->blk_dev->dev_fd,
			rctx->bs_checksum.checksum_sect,
			size, size * 11) != size) {
		exfat_err("failed to write checksum sector\n");
		return false;
	}

	return true;
}

static struct exfat_repair_problem problems[] = {
	{ER_BS_CHECKSUM,
		"the checksum of boot sector is not correct",
		fix_bs_checksum},
};

static struct exfat_repair_problem *find_problem(er_problem_code_t prcode)
{
	unsigned int i;

	for (i = 0; i < sizeof(problems)/sizeof(problems[0]); i++) {
		if (problems[i].prcode == prcode) {
			return &problems[i];
		}
	}
	return NULL;
}

static bool ask_repair(struct exfat *exfat, struct exfat_repair_problem *pr)
{
	char answer[8];

	switch (exfat->options & FSCK_OPTS_REPAIR) {
	case FSCK_OPTS_REPAIR_ASK:
		do {
			printf("%s: Fix (y/N)?", pr->description);
			fflush(stdout);

			if (fgets(answer, sizeof(answer), stdin)) {
				if (strcasecmp(answer, "Y\n") == 0)
					return true;
				else if (strcasecmp(answer, "\n") == 0 ||
					strcasecmp(answer, "N\n") == 0)
					return false;
			}
		} while (1);
		return false;
	case FSCK_OPTS_REPAIR_YES:
		return true;
	case FSCK_OPTS_REPAIR_NO:
	case 0:
	default:
		return false;
	}
	return false;
}

bool exfat_repair(struct exfat *exfat, er_problem_code_t prcode,
			union exfat_repair_context *rctx)
{
	struct exfat_repair_problem *pr = NULL;
	int need_repair;

	need_repair = ask_repair(exfat, pr);
	if (!need_repair)
		return false;

	pr = find_problem(prcode);
	if (!pr) {
		exfat_err("unknown problem code. %#x\n", prcode);
		return false;
	}

	return pr->fix_problem(exfat, rctx);
}

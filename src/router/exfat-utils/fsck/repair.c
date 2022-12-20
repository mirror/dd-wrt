/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *  Copyright (C) 2020 Hyunchul Lee <hyc.lee@gmail.com>
 */
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

#include "exfat_ondisk.h"
#include "libexfat.h"
#include "repair.h"
#include "exfat_fs.h"
#include "exfat_dir.h"
#include "fsck.h"

struct exfat_repair_problem {
	er_problem_code_t	prcode;
	unsigned int		flags;
	unsigned int		prompt_type;
	unsigned int		default_number;
	unsigned int		bypass_number;
	unsigned int		max_number;
};

/* Problem flags */
#define ERF_PREEN_YES		0x00000001
#define ERF_DEFAULT_YES		0x00000002
#define ERF_DEFAULT_NO		0x00000004

/* Prompt types */
#define ERP_FIX			0x00000001
#define ERP_TRUNCATE		0x00000002
#define ERP_DELETE		0x00000003
#define ERP_RENAME		0x00000004

static const char *prompts[] = {
	"Repair",
	"Fix",
	"Truncate",
	"Delete",
	"Select",
};

static struct exfat_repair_problem problems[] = {
	{ER_BS_CHECKSUM, ERF_PREEN_YES, ERP_FIX, 0, 0, 0},
	{ER_BS_BOOT_REGION, 0, ERP_FIX, 0, 0, 0},
	{ER_DE_CHECKSUM, ERF_PREEN_YES, ERP_DELETE, 0, 0, 0},
	{ER_DE_UNKNOWN, ERF_PREEN_YES, ERP_DELETE, 0, 0, 0},
	{ER_DE_FILE, ERF_PREEN_YES, ERP_DELETE, 0, 0, 0},
	{ER_DE_SECONDARY_COUNT, ERF_PREEN_YES, ERP_DELETE, 0, 0, 0},
	{ER_DE_STREAM, ERF_PREEN_YES, ERP_DELETE, 0, 0, 0},
	{ER_DE_NAME, ERF_PREEN_YES, ERP_DELETE, 0, 0, 0},
	{ER_DE_NAME_HASH, ERF_PREEN_YES, ERP_FIX, 0, 0, 0},
	{ER_DE_NAME_LEN, ERF_PREEN_YES, ERP_FIX, 0, 0, 0},
	{ER_DE_DOT_NAME, ERF_PREEN_YES, ERP_RENAME, 2, 3, 4},
	{ER_FILE_VALID_SIZE, ERF_PREEN_YES, ERP_FIX, 0, 0, 0},
	{ER_FILE_INVALID_CLUS, ERF_PREEN_YES, ERP_TRUNCATE, 0, 0, 0},
	{ER_FILE_FIRST_CLUS, ERF_PREEN_YES, ERP_TRUNCATE, 0, 0, 0},
	{ER_FILE_SMALLER_SIZE, ERF_PREEN_YES, ERP_TRUNCATE, 0, 0, 0},
	{ER_FILE_LARGER_SIZE, ERF_PREEN_YES, ERP_TRUNCATE, 0, 0, 0},
	{ER_FILE_DUPLICATED_CLUS, ERF_PREEN_YES, ERP_TRUNCATE, 0, 0, 0},
	{ER_FILE_ZERO_NOFAT, ERF_PREEN_YES, ERP_FIX, 0, 0, 0},
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

static int ask_repair(struct exfat_fsck *fsck, struct exfat_repair_problem *pr)
{
	int repair = 0;
	char answer[8];

	if (fsck->options & FSCK_OPTS_REPAIR_NO ||
	    pr->flags & ERF_DEFAULT_NO)
		repair = 0;
	else if (fsck->options & FSCK_OPTS_REPAIR_YES ||
		 pr->flags & ERF_DEFAULT_YES)
		repair = 1;
	else {
		if (fsck->options & FSCK_OPTS_REPAIR_ASK) {
			do {
				if (pr->prompt_type & ERP_RENAME) {
					printf("%s (Number: ?) ",
					       prompts[pr->prompt_type]);
				} else {
					printf(". %s (y/N)? ",
					       prompts[pr->prompt_type]);
				}
				fflush(stdout);

				if (!fgets(answer, sizeof(answer), stdin))
					continue;

				if (pr->prompt_type & ERP_RENAME) {
					unsigned int number = atoi(answer);

					if (number > 0 && number < pr->max_number)
						return number;
				} else {
					if (strcasecmp(answer, "Y\n") == 0)
						return 1;
					else if (strcasecmp(answer, "\n") == 0 ||
						 strcasecmp(answer, "N\n") == 0)
						return 0;
				}
			} while (1);
		} else if (fsck->options & FSCK_OPTS_REPAIR_AUTO &&
			   pr->flags & ERF_PREEN_YES)
			repair = 1;
	}

	if (pr->prompt_type & ERP_RENAME) {
		int print_num = repair ? pr->default_number : pr->bypass_number;

		printf("%s (Number : %d)\n", prompts[pr->prompt_type],
		       print_num);
		repair = print_num;
	} else {
		printf(". %s (y/N)? %c\n", prompts[pr->prompt_type],
		       repair ? 'y' : 'n');
	}
	return repair;
}

int exfat_repair_ask(struct exfat_fsck *fsck, er_problem_code_t prcode,
		     const char *desc, ...)
{
	struct exfat_repair_problem *pr = NULL;
	va_list ap;
	int repair;

	pr = find_problem(prcode);
	if (!pr) {
		exfat_err("unknown problem code. %#x\n", prcode);
		return 0;
	}

	va_start(ap, desc);
	vprintf(desc, ap);
	va_end(ap);

	repair = ask_repair(fsck, pr);
	if (repair) {
		if (pr->prompt_type & ERP_TRUNCATE)
			fsck->dirty_fat = true;
		fsck->dirty = true;
	}
	return repair;
}

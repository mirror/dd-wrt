/*
 * Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef __PPE_ACL_DUMP_H
#define __PPE_ACL_DUMP_H

/*
 * Buffer sizes
 */
#define PPE_ACL_DUMP_PREFIX_SIZE 64
#define PPE_ACL_DUMP_PREFIX_MAX 6
#define PPE_ACL_DUMP_BUFFER_SIZE 3072000

/*
 * struct ppe_acl_dump_instance
 *	Structure used as an instance for acl dump
 */
struct ppe_acl_dump_instance {
	uint16_t acl_cnt;				/* Number of ACL rules  */
	char prefix[PPE_ACL_DUMP_PREFIX_SIZE];		/* This is the prefix added to every message written */
	int prefix_levels[PPE_ACL_DUMP_PREFIX_MAX];	/* How many nested prefixes supported */
	int prefix_level;				/* Prefix nest level */
	char msg[PPE_ACL_DUMP_BUFFER_SIZE];		/* The message written / being returned to the reader */
	char *msgp;					/* Points into the msg buffer as we output it to the reader piece by piece */
	int msg_len;					/* Length of the msg buffer still to be written out */
	bool dump_en;					/* Enable dump once the file is open */
};

extern bool ppe_acl_dump_init(struct dentry *dentry);
extern void ppe_acl_dump_exit(void);

#endif

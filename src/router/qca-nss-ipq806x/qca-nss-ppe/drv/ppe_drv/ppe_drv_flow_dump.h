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

/*
 * Buffer sizes
 */
#define PPE_DRV_FLOW_DUMP_FILE_PREFIX_SIZE 128
#define PPE_DRV_FLOW_DUMP_FILE_PREFIX_LEVELS_MAX 10
#define PPE_DRV_FLOW_DUMP_FILE_BUFFER_SIZE 3072000


/*
 * struct ppe_drv_flow_dump_file_instance
 *	Structure used as state per open instance of our db state file
 */
struct ppe_drv_flow_dump_instance {
	uint16_t flow_cnt;					/* To identify the flow cnt */
	char prefix[PPE_DRV_FLOW_DUMP_FILE_PREFIX_SIZE];        /* This is the prefix added to every message written */
	int prefix_levels[PPE_DRV_FLOW_DUMP_FILE_PREFIX_LEVELS_MAX];	/* How many nested prefixes supported */
	int prefix_level;                               /* Prefix nest level */
	char msg[PPE_DRV_FLOW_DUMP_FILE_BUFFER_SIZE];           /* The message written / being returned to the reader */
	char *msgp;                                     /* Points into the msg buffer as we output it to the reader piece by piece */
	int msg_len;                                    /* Length of the msg buffer still to be written out */
	bool dump_en;					/* Enable dump once file is open */
	struct list_head *cn_v4;			/* head pointer initialization for the connection v4 */
	struct list_head *cn_tun_v4;			/* head pointer initialization for the tunnel connection v4 */
	struct list_head *cn_v6;			/* head pointer initialization for the connection v6 */
	struct list_head *cn_tun_v6;			/* head pointer initialization for the tunnel connection v6 */
};

extern int ppe_drv_flow_dump_init(struct dentry *dentry);
extern void ppe_drv_flow_dump_exit(void);

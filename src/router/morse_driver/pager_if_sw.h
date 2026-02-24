#ifndef _MORSE_PAGER_IF_SW_H_
#define _MORSE_PAGER_IF_SW_H_

/*
 * Copyright 2021 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#include "pager_if.h"

/* This is the interface to the software version of the pager that uses RBs
 * to provide a pager-like interface.
 */

struct morse_pager_sw_table {
	u32 addr;		/* location of the pager table */
	u32 count;		/* Number of entries in the table */
};

/* SW pager uses RBs to replicate the pager HW */
struct morse_pager_sw_entry {
	u8 id:4;
	u8 aci:4;
	u8 flags;		/* Indicate direction of ring buffer */
	__le16 size;		/* size of ring buffer, should be 2^N */
	__le32 base;		/* Ring buffer base address */
	__le32 head;		/* Ring buffer head address */
	__le32 tail;		/* Ring buffer tail address */
} __packed;

int morse_pager_sw_read_table(struct morse *mors, struct morse_pager_sw_table *tbl_ptr);

/* HW interface specific fields */
int morse_pager_sw_init(struct morse *mors, struct morse_pager *pager,
			u32 entry_addr, u32 size, u32 base, u32 head, u32 tail);
void morse_pager_sw_finish(struct morse *mors, struct morse_pager *pager);

/* Implementing interface from hw.h */
int morse_pager_sw_pagesets_init(struct morse *mors);
void morse_pager_sw_pagesets_flush_tx_data(struct morse *mors);
void morse_pager_sw_pagesets_finish(struct morse *mors);

#endif /* !_MORSE_PAGER_IF_SW_H_ */

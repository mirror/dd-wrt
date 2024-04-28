/*
 **************************************************************************
 * Copyright (c) 2014-2018, The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **************************************************************************
 */

/*
 * Timers and cleanup
 */
extern uint32_t ecm_db_time;	/* Time in seconds since start */

/*
 * struct ecm_db_timer_group
 *	A timer group - all group members within the same group have the same TTL reset value.
 *
 * Expiry of entries occurs from tail to head.
 */
struct ecm_db_timer_group {
	struct ecm_db_timer_group_entry *head;		/* Most recently used entry in this timer group */
	struct ecm_db_timer_group_entry *tail;		/* Least recently used entry in this timer group. */
	uint32_t time;					/* Time in seconds a group entry will be given to live when 'touched' */
	ecm_db_timer_group_t tg;			/* RO: The group id */
#if (DEBUG_LEVEL > 0)
	uint16_t magic;
#endif
};

extern struct ecm_db_timer_group ecm_db_timer_groups[ECM_DB_TIMER_GROUPS_MAX];

uint32_t ecm_db_time_get(void);
void ecm_db_timer_group_entry_init(struct ecm_db_timer_group_entry *tge, ecm_db_timer_group_entry_callback_t fn, void *arg);

void _ecm_db_timer_group_entry_set(struct ecm_db_timer_group_entry *tge, ecm_db_timer_group_t tg);
void ecm_db_timer_group_entry_set(struct ecm_db_timer_group_entry *tge, ecm_db_timer_group_t tg);

bool ecm_db_timer_group_entry_reset(struct ecm_db_timer_group_entry *tge, ecm_db_timer_group_t tg);

bool _ecm_db_timer_group_entry_remove(struct ecm_db_timer_group_entry *tge);
bool ecm_db_timer_group_entry_remove(struct ecm_db_timer_group_entry *tge);

bool ecm_db_timer_group_entry_touch(struct ecm_db_timer_group_entry *tge);

void ecm_db_timer_init(void);
void ecm_db_timer_exit(void);

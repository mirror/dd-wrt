/*
 * Linux rfkill helper functions for driver wrappers
 * Copyright (c) 2010, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef RFKILL_H
#define RFKILL_H

struct rfkill_data;

struct rfkill_config {
	void *ctx;
	char ifname[IFNAMSIZ];
	void (*blocked_cb)(void *ctx);
	void (*unblocked_cb)(void *ctx);
};

#ifdef CONFIG_RFKILL
struct rfkill_data * rfkill_init(struct rfkill_config *cfg);
void rfkill_deinit(struct rfkill_data *rfkill);
int rfkill_is_blocked(struct rfkill_data *rfkill);
#else
static inline struct rfkill_data * rfkill_init(struct rfkill_config *cfg)
{
	return (void *) 1;
}

static inline void rfkill_deinit(struct rfkill_data *rfkill)
{
}

static inline int rfkill_is_blocked(struct rfkill_data *rfkill)
{
	return 0;
}
#endif

#endif /* RFKILL_H */

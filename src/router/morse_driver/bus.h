#ifndef _MORSE_BUS_H_
#define _MORSE_BUS_H_

/*
 * Copyright 2017-2022 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#include <linux/skbuff.h>

#include "morse.h"

/**
 * struct morse_bus_ops - bus callback operations.
 *
 * @morse_dm_write: direct memory write.
 * @morse_dm_read: direct memory read.
 * @morse_req32_write: word memory write.
 * @morse_reg32_read: word memory read.
 *
 * This structure provides an abstract interface towards the
 * bus specific driver. For control messages to common driver
 * will assure there is only one active transaction. Unless
 * indicated otherwise these callbacks are mandatory.
 */
struct morse_bus_ops {
	int (*dm_read)(struct morse *mors, u32 addr, u8 *data, int len);
	int (*dm_write)(struct morse *mors, u32 addr, const u8 *data, int len);
	int (*reg32_read)(struct morse *mors, u32 addr, u32 *data);
	int (*reg32_write)(struct morse *mors, u32 addr, u32 data);
	int (*skb_tx)(struct morse *mors, struct sk_buff *skb, u8 channel);
	int (*reset)(struct morse *mors);
	void (*set_bus_enable)(struct morse *mors, bool enable);
	void (*config_burst_mode)(struct morse *mors, bool enable_burst);
	void (*claim)(struct morse *mors);
	void (*set_irq)(struct morse *mors, bool enable);
	void (*release)(struct morse *mors);
	unsigned int bulk_alignment;
};

/** Default TX alignment for bus's which don't care.
 *  mac80211 will give us SKBs aligned to the 2 byte boundary, so 2 is effectively a noop
 */
#define MORSE_DEFAULT_BULK_ALIGNMENT	(2)

static inline int morse_dm_write(struct morse *mors, u32 addr, const u8 *data, int len)
{
	return mors->bus_ops->dm_write(mors, addr, data, len);
}

/* morse_dm_read - len must be rounded up to the nearest 4-byte boundary */
static inline int morse_dm_read(struct morse *mors, u32 addr, u8 *data, int len)
{
	return mors->bus_ops->dm_read(mors, addr, data, len);
}

static inline int morse_reg32_write(struct morse *mors, u32 addr, u32 data)
{
	return mors->bus_ops->reg32_write(mors, addr, data);
}

static inline int morse_reg32_read(struct morse *mors, u32 addr, u32 *data)
{
	return mors->bus_ops->reg32_read(mors, addr, data);
}

static inline void morse_set_bus_enable(struct morse *mors, bool enable)
{
	mors->bus_ops->set_bus_enable(mors, enable);
}

static inline void morse_claim_bus(struct morse *mors)
{
	mors->bus_ops->claim(mors);
}

static inline void morse_release_bus(struct morse *mors)
{
	mors->bus_ops->release(mors);
}

static inline int morse_bus_reset(struct morse *mors)
{
	return mors->bus_ops->reset(mors);
}

static inline void morse_bus_set_irq(struct morse *mors, bool enable)
{
	mors->bus_ops->set_irq(mors, enable);
}

int morse_bus_test(struct morse *mors, const char *bus_name);
void morse_bus_throughput_profiler(struct morse *mors);
void morse_bus_interrupt_profiler_irq(struct morse *mors);
int morse_skb_tx(struct morse *mors, struct sk_buff *skb, u8 channel);

enum morse_host_bus_type {
	MORSE_HOST_BUS_TYPE_SDIO,
	MORSE_HOST_BUS_TYPE_SPI,
	MORSE_HOST_BUS_TYPE_USB,
};

#endif /* !_MORSE_BUS_H_ */

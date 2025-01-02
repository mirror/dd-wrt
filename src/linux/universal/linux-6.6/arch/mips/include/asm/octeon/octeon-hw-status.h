/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2012 Cavium, Inc.
 */
#ifndef _ASM_OCTEON_OCTEON_HW_STATUS_H
#define _ASM_OCTEON_OCTEON_HW_STATUS_H

struct notifier_block;

#define OCTEON_HW_STATUS_SOURCE_ADDED 1
#define OCTEON_HW_STATUS_SOURCE_ASSERTED 2

struct octeon_hw_status_reg {
	u64 reg;
	u64 mask_reg;
	u8 bit;
	u8 reg_is_hwint:1;
	u8 ack_w1c:1;
	u8 has_child:1;
};

/* Passed to notifier callbacks. */
struct octeon_hw_status_data {
	u64 reg;
	u32 bit;
	u8 reg_is_hwint:1;
};

int octeon_hw_status_notifier_register(struct notifier_block *nb);
int octeon_hw_status_notifier_unregister(struct notifier_block *nb);

int octeon_hw_status_add_source(struct octeon_hw_status_reg *chain);
int octeon_hw_status_remove_source(struct octeon_hw_status_reg *leaf);

int octeon_hw_status_enable(u64 reg, u64 bit_mask);
int octeon_hw_status_disable(u64 reg, u64 bit_mask);

#endif /* _ASM_OCTEON_OCTEON_HW_STATUS_H */

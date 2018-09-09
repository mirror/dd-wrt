/***********************license start***************
 * Copyright (c) 2013  Cavium Inc. (support@cavium.com). All rights
 * reserved.
 *
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.

 *   * Neither the name of Cavium Inc. nor the names of
 *     its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written
 *     permission.

 * This Software, including technical data, may be subject to U.S. export  control
 * laws, including the U.S. Export Administration Act and its  associated
 * regulations, and may be subject to export or import  regulations in other
 * countries.

 * TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND CAVIUM INC. MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE, INCLUDING ITS CONDITION, ITS CONFORMITY TO ANY REPRESENTATION OR
 * DESCRIPTION, OR THE EXISTENCE OF ANY LATENT OR PATENT DEFECTS, AND CAVIUM
 * SPECIFICALLY DISCLAIMS ALL IMPLIED (IF ANY) WARRANTIES OF TITLE,
 * MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF
 * VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. THE ENTIRE  RISK ARISING OUT OF USE OR
 * PERFORMANCE OF THE SOFTWARE LIES WITH YOU.
 ***********************license end**************************************/
/**
 * @file
 *
 * Encapsulation of OCTEON mailbox interrupts.
 *
 */

#include "cvmx.h"
#include "cvmx-atomic.h"
#include "cvmx-interrupt.h"
#include "cvmx-mbox.h"

#define CVMX_MBOX_CIU3_MBOX_PER_CORE 10

/*
 * SE-U applications should be able to send mailbox events
 * to other cores running SE-U or SE-S environments.
 */

CVMX_SHARED void (*cvmx_mbox_signal)(unsigned int bit, unsigned int core);

static void cvmx_mbox_ciu_signal(unsigned int bit, unsigned int core)
{
	uint64_t mask = 1ull << bit;

	cvmx_write_csr(CVMX_CIU_MBOX_SETX(core), mask);
	cvmx_read_csr(CVMX_CIU_MBOX_SETX(core));
}

/*
 * 10 mbox per core starting from zero.
 * Base mbox is core * 10
 */
static unsigned int cvmx_mbox_ciu3_base_mbox_intsn(int core)
{
	/* SW (mbox) are 0x04 in bits 12..19 */
	return 0x04000 + CVMX_MBOX_CIU3_MBOX_PER_CORE * core;
}

static unsigned int cvmx_mbox_ciu3_mbox_intsn_for_core(int core, unsigned int mbox)
{
	return cvmx_mbox_ciu3_base_mbox_intsn(core) + mbox;
}

static void cvmx_mbox_ciu3_signal(unsigned int bit, unsigned int core)
{
	unsigned int intsn = cvmx_mbox_ciu3_mbox_intsn_for_core(core, bit);
	uint64_t addr = CVMX_CIU3_ISCX_W1S(intsn);

	cvmx_write_csr(addr, 1);
	cvmx_read_csr(addr);
}

#ifndef CVMX_BUILD_FOR_LINUX_USER

CVMX_SHARED void (*cvmx_mbox_register)(unsigned int bit, struct cvmx_mbox *handler);

/* Incremented once first core processing is finished. */
static CVMX_SHARED int32_t cvmx_mbox_initialize_flag;

static CVMX_SHARED struct cvmx_mbox *cvmx_per_mbox_handlers[32];


struct cvmx_mbox_ciu_interrupt {
	struct cvmx_interrupt irq;
	uint32_t mask;
	int32_t initialized;
};

static CVMX_SHARED struct cvmx_mbox_ciu_interrupt cvmx_mbox_ciu_interrupt_handlers[4];

static void cvmx_mbox_ciu_irq_handler(struct cvmx_interrupt *top_irq, uint64_t *registers)
{
	int i;
	struct cvmx_mbox *mbox;
	unsigned int core = cvmx_get_core_num();
	struct cvmx_mbox_ciu_interrupt *ciu_irq = CVMX_CONTAINTER_OF(top_irq,
								     struct cvmx_mbox_ciu_interrupt,
								     irq);
	uint64_t bits = cvmx_read_csr(CVMX_CIU_MBOX_CLRX(core));

	bits &= ciu_irq->mask;

	/* Clear the bits we will signal */
	cvmx_write_csr(CVMX_CIU_MBOX_CLRX(core), bits);
	cvmx_read_csr(CVMX_CIU_MBOX_CLRX(core));

	for(i = 0; i < 32; i++) {
		if (bits & (1ull << i)) {
			mbox = cvmx_per_mbox_handlers[i];
			if (mbox)
				mbox->handler(mbox, registers);
		}
	}
}

static void cvmx_mbox_ciu_register(unsigned int bit, struct cvmx_mbox *handler)
{
	if (bit >= 32) {
		cvmx_warn("cvmx_mbox_register: Illegal mbox %u\n", bit);
		return;
	}
	cvmx_per_mbox_handlers[bit] = handler;
	CVMX_SYNCW;
}

static void cvmx_mbox_ciu_initialize(uint32_t mbox_mask)
{
	int i;
	int sf, max;
	uint32_t mask;
	struct cvmx_mbox_ciu_interrupt *ciu_irq;

	if (octeon_has_feature(OCTEON_FEATURE_CIU2)) {
		mask = 0xff;
		sf = 8;
		max = 4;
	} else {
		mask = 0xffff;
		sf = 16;
		max = 2;
	}

	for(i = 0; i < max; i++) {
		if (!(mbox_mask & (mask << (sf * i))))
			continue;
		ciu_irq = &cvmx_mbox_ciu_interrupt_handlers[i];
		if (!cvmx_atomic_get32(&ciu_irq->initialized) && cvmx_is_init_core()) {
			ciu_irq->mask = mask << (sf * i);
			ciu_irq->irq.handler = cvmx_mbox_ciu_irq_handler;
			cvmx_interrupt_register(cvmx_interrupt_map(CVMX_IRQ_MBOX0 + i), &ciu_irq->irq);
			CVMX_SYNCW;
			cvmx_atomic_set32(&ciu_irq->initialized, 1);
		}
		while (!cvmx_atomic_get32(&ciu_irq->initialized)) ; /* Wait for first core to finish above. */
		ciu_irq->irq.unmask(&ciu_irq->irq);
	}
}


struct cvmx_mbox_ciu3_interrupt {
	struct cvmx_interrupt irq;
	int mbox;
};

static void cvmx_mbox_ciu3_irq_handler(struct cvmx_interrupt *top_irq, uint64_t *registers)
{
	struct cvmx_mbox_ciu3_interrupt *ciu3_irq = CVMX_CONTAINTER_OF(top_irq,
								       struct cvmx_mbox_ciu3_interrupt,
								       irq);
	struct cvmx_mbox *mbox = cvmx_per_mbox_handlers[ciu3_irq->mbox];

	if (mbox)
		mbox->handler(mbox, registers);
}


/**
 * Per CPU handlers for CIU3
 */
static struct cvmx_mbox_ciu3_interrupt cvmx_mbox_ciu3_handlers[] = {
	{.irq = {.handler = cvmx_mbox_ciu3_irq_handler}, .mbox = 0},
	{.irq = {.handler = cvmx_mbox_ciu3_irq_handler}, .mbox = 1},
	{.irq = {.handler = cvmx_mbox_ciu3_irq_handler}, .mbox = 2},
	{.irq = {.handler = cvmx_mbox_ciu3_irq_handler}, .mbox = 3},
	{.irq = {.handler = cvmx_mbox_ciu3_irq_handler}, .mbox = 4},
	{.irq = {.handler = cvmx_mbox_ciu3_irq_handler}, .mbox = 5},
	{.irq = {.handler = cvmx_mbox_ciu3_irq_handler}, .mbox = 6},
	{.irq = {.handler = cvmx_mbox_ciu3_irq_handler}, .mbox = 7},
	{.irq = {.handler = cvmx_mbox_ciu3_irq_handler}, .mbox = 8},
	{.irq = {.handler = cvmx_mbox_ciu3_irq_handler}, .mbox = 9}
};

static void cvmx_mbox_ciu3_register(unsigned int bit, struct cvmx_mbox *handler)
{
	if (bit >= CVMX_MBOX_CIU3_MBOX_PER_CORE) {
		cvmx_warn("cvmx_mbox_register: Illegal mbox %u\n", bit);
		return;
	}
	cvmx_per_mbox_handlers[bit] = handler;
	CVMX_SYNCW;
}

static void cvmx_mbox_ciu3_initialize(uint32_t mbox_mask)
{
	int i;
	struct cvmx_interrupt *irq;
	unsigned int core = cvmx_get_core_num();
	unsigned int intsn;

	for(i = 0; i < CVMX_MBOX_CIU3_MBOX_PER_CORE; i++) {
		if (!(mbox_mask & (1 << i)))
			continue;
		intsn = cvmx_mbox_ciu3_mbox_intsn_for_core(core, i);
		irq = &cvmx_mbox_ciu3_handlers[i].irq;
		cvmx_interrupt_register(intsn, irq);
		irq->unmask(irq);
	}
}
#endif /* !CVMX_BUILD_FOR_LINUX_USER */

int cvmx_mbox_num_bits(void)
{
	if (octeon_has_feature(OCTEON_FEATURE_CIU3))
		return CVMX_MBOX_CIU3_MBOX_PER_CORE;
	else
		return 32;
}

void cvmx_mbox_initialize(uint32_t mbox_mask)
{
	if (cvmx_is_init_core()) {
		if (octeon_has_feature(OCTEON_FEATURE_CIU3)) {
			cvmx_mbox_signal = cvmx_mbox_ciu3_signal;
		} else {
			cvmx_mbox_signal = cvmx_mbox_ciu_signal;
		}
	}

#ifndef CVMX_BUILD_FOR_LINUX_USER
	if (cvmx_is_init_core()) {
		if (octeon_has_feature(OCTEON_FEATURE_CIU3)) {
			cvmx_mbox_register = cvmx_mbox_ciu3_register;
		} else {
			cvmx_mbox_register = cvmx_mbox_ciu_register;
		}
		cvmx_atomic_set32(&cvmx_mbox_initialize_flag, 1);
	}
	/* Wait for first core to finish above. */
	while (!cvmx_atomic_get32(&cvmx_mbox_initialize_flag));

	if (octeon_has_feature(OCTEON_FEATURE_CIU3))
		cvmx_mbox_ciu3_initialize(mbox_mask);
	else
		cvmx_mbox_ciu_initialize(mbox_mask);
#endif /* !CVMX_BUILD_FOR_LINUX_USER */
}

/*
 **************************************************************************
 * Copyright (c) 2014, 2018 The Linux Foundation. All rights reserved.
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
 * profilesample.h
 *	Sample format for profiling
 */

#ifndef	_NSS_PROFILE_SAMPLE_H_
#define	_NSS_PROFILE_SAMPLE_H_

#define NSS_PROFILE_STACK_WORDS 4

/*
 * Each sample is for an enabled thread, not including the profiling thread.
 * HRT thread sampling is optional.
 * Sampled threads may be active or inactive.  Samples are included in thread number
 * order, so each sample interval has a set of samples starting with one from thread 0
 *
 * Samples include bits indicating if this thread is blocked
 */
#define NSS_PROFILE_I_BLOCKED_BIT 5
#define NSS_PROFILE_I_BLOCKED (1 << NSS_PROFILE_I_BLOCKED_BIT)
#define NSS_PROFILE_D_BLOCKED_BIT 4
#define NSS_PROFILE_D_BLOCKED (1 << NSS_PROFILE_D_BLOCKED_BIT)
#define NSS_PROFILE_BTB_SHIFT 6

struct nss_profile_sample {
	uint32_t pc;		/* PC value */
	uint32_t pid;		/* pid for the current process, or 0 if NOMMU or unmapped space */
	uint16_t active;	/* threads are active - for accurate counting */
	uint16_t d_blocked;	/* threads are blocked due to D cache misses : may be removed */
	uint16_t i_blocked;	/* threads are blocked due to I cache misses */
	uint8_t cond_codes;	/* for branch prediction */
	uint8_t thread;		/* 4-bit thread number */
	uint32_t a_reg;		/* source An if PC points to a calli.  Otherwise a5 contents for parent of leaf function */
	uint32_t parent[NSS_PROFILE_STACK_WORDS];
				/* return addresses from stack, to find the caller */
};


/*
 * packet size for profile communication = MSS - tcp/ip headers
 */
#define NSS_PROFILE_MAX_PACKET_SIZE 1440

#define NSS_PROFILE_MAX_COUNTERS ((NSS_PROFILE_MAX_PACKET_SIZE - sizeof(struct profile_header_counters)) / (PROFILE_COUNTER_NAME_LENGTH + 4))

struct profile_counter {
	char name[PROFILE_COUNTER_NAME_LENGTH];
	uint32_t value;
};

/*
 * NSS HW counters and CPU threads
 */
#define NSS_HW_COUNTERS 8
#define	NSS_CPU_THREADS	12

/*
 * sampling period info cross all modules -- use extended struct to avoid copy
 */
struct profile_ext_header {
	uint16_t d_blocked;	/* threads are blocked due to D cache misses */
	uint16_t i_blocked;	/* threads are blocked due to I cache misses */
	uint16_t high;		/* threads were enabled high priority -- unused */
	uint16_t enabled_threads;	/* threads were enabled at the last sample time */
	uint16_t hrt;		/* HRT threads */
	uint8_t profiler_tid;	/* thread running the profile sampler */
	uint8_t sample_sets;	/* typical 5-8 sets, and may be 9 - see design doc for details */
	uint32_t sets_map;	/* a set map uses a nibble, 8 maps and 9th is derived */
	uint32_t clocks;		/* system clock timer at last sample */
	uint32_t inst_count[NSS_CPU_THREADS]; /* sampled instruction counts at most recent sample */
	uint32_t stats[NSS_HW_COUNTERS]; /* contents of the cache statistics counters */
};

#endif	/* _NSS_PROFILE_SAMPLE_H_ */

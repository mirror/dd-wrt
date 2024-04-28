/*
 **************************************************************************
 * Copyright (c) 2014, The Linux Foundation. All rights reserved.
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
 * profpkt.h
 *      profile packet header to communicate with the profiler display tool
 *
 * IMPORTANT!  There is a parallel verison of this file for both linux driver and profiler display tool
 *
 * Both this file and profilesample.h file must be placed
 * in dirver/.../nss (for linux driver)	and as well as	in tools/profiler/include directories
 *	(for profilerd.c of profilerd and profile.cpp:1600::on_packet of profile tool).
 *
 *   Ubi32 CPU Profiler packet formats for communication between the linux proc driver and the profiler display tool
 */

#include "profilesample.h"	// common definitions

#define PROFILE_PORT 51080
#define PROFILE_CONTROL_PORT 51081
#define PROFILE_POSIX_NAME_LENGTH 32

/*
 * profile UDP packet format for communicating between ip3k and host
 *
 * every packet starts with a header, followed by samples.
 * samples are only taken for non-hrt threads that are
 * active
 */
#define PROF_MAGIC 0x3ea0
#define PROF_MAGIC_COUNTERS 0x9ea0
#define PROF_MAGIC_MAPS 0xaea0

/*
 * Versions (31 max):
 * 1 to 4 were before 6.0 release,  development versions
 * 5 was forward compatible version, shipped with 6.0 and 6.1
 * 6 adds heap packets, and clock_freq to header, shipped with 6.2
 * 7 adds a sequence numbers to check for dropped packets, shipped with 6.3.5
 * 8 adds mqueue timing information, shipped with 6.3.5
 * 9 adds sdram heap size information, shipped with 6.4
 * 10 adds heapmem heap callers and long latency stack traces.  shipped with 6.4
 * 11 adds support for Mars (IP5K).  shipped with 6.10
 * 12 adds more support for Mars.  Shipped with 7.0
 * 13 adds per sample latency measurement.  Shipped with 7.2
 * 14 changes the heap format and adds a string packet.  Shipped with 7.4
 * 15 adds dsr stats and posix.  shipped with 7.6
 * 16 corrects maximum packet count for Ares.  ships with 7.9
 * 17 adds a5 register value to sample
 * 18 adds counter support and removes unused header fields
 * 19 adds PID support for MMU profiling
 * 20 changes the protocol for transmitting map PID maps automatically
 * 21 adds support for multiple possible parents, configurable
 */

#define PROFILE_VERSION 21


/*
 * Each packet starts with a profile_header, then sample_count samples;
 * samples are gprof samples of pc, the return address, condition codes, and active threads.
 * For performance concern, the field sequence may be reordered to match profilenode to reduce
 * a memory copy.
 */
struct profile_pkg_header {		// in network byte order !
	uint16_t magic;			/* magic number and version */
	uint8_t header_size;		/* number of bytes in profile header */
	uint8_t sample_count;		/* number of samples in the packet */
	uint8_t nc_sts_tselA;		/* thr 1 statistics requst to FW */
	uint8_t nc_sts_tselB;		/* thr 2 requst to FW */
	uint8_t spare1B;
	uint8_t sample_stack_words;	/* number of stack words in the sample */
	uint32_t seq_num;		/* to detect dropped profiler packets */
	uint32_t profile_instructions;	/* instructions executed by profiler mainline */

	uint32_t unused_overlay;	//  untouched fields below in Linux -- to reduce memcpy
	uint32_t cpu_id;		/* CHIP_ID register contents */
	uint32_t clock_freq;		/* clock frequency (Hz) of system being analyzed */
	uint32_t ddr_freq;		/* DDR clock frequency */
};

struct profile_header {		// in network byte order !
	struct profile_pkg_header pph;
	struct profile_ext_header exh;
};

struct profile_header_counters {
	uint16_t magic;
	uint16_t ultra_count;		// how many ultra counters follow this
	uint32_t ultra_sample_time;	// in chip clocks
	uint32_t linux_count;		// how many linux counters follow this
	uint32_t linux_sample_time;
};

/*
 * send memory maps from linux to profiler tool
 */

struct profile_header_maps {
	uint16_t magic;			/* magic number and last packet bit */
	uint16_t count;
	uint32_t page_shift;
};

#define PROFILE_MAP_NUM_TYPES 16

/* size field is pages.  True size in bytes is (1 << PAGE_SHIFT) * size */
#define PROFILE_MAP_TYPE_FREE 0
#define PROFILE_MAP_TYPE_SMALL 1
#define PROFILE_MAP_TYPE_FS 2
#define PROFILE_MAP_TYPE_UNKNOWN_USED 4
#define PROFILE_MAP_TYPE_TEXT 5
#define PROFILE_MAP_TYPE_STACK 6
#define PROFILE_MAP_TYPE_APP_DATA 7
#define PROFILE_MAP_TYPE_ASHMEM 8
#define PROFILE_MAP_TYPE_READ_SHARED 9
#define PROFILE_MAP_TYPE_CACHE 10
#define PROFILE_MAP_TYPE_VMA_WASTE 11
#define PROFILE_MAP_RESERVED 15

#define PROFILE_MAP_TYPE_SHIFT 12
#define PROFILE_MAP_SIZE_MASK 0xfff

struct profile_map {
	uint16_t start;		/* start page number of segment, relative to start of OCM (Max 256 MB on IP7K or 1 GB on IP8K, plus 256 KB OCM) */
	uint16_t type_size;	/* type (4 bits) of the segment and size (12 bits) in pages. A size of 0 means 4K pages */
};

#define PROFILE_MAX_MAPS (PROFILE_MAX_PACKET_SIZE - sizeof(struct profile_header_maps)) / sizeof(struct profile_map)

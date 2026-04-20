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
 * nss profile node.h
 *	device node for the profiler, for communicating between FW and GUI via host
 */

#include "profilesample.h"

#define	NSS_PROFILE_HD_MAGIC		0x4F525000	/* 0 + "PRO" */
#define	NSS_PROFILE_HD_MAGIC_REV	0x0050524F	/* different endian */
#define	NSS_PROFILE_HD_MMASK		0xFFFFFF00	/* 1st byte is Rx/Tx cmd */
#define	NSS_PROFILE_HD_MMASK_REV	0x00FFFFFF

struct nss_profile_sample_ctrl {
	uint32_t psc_magic;	/* MAGIC + cmd for packet and endianess check */

	/*
	 * controlling data need to be sent to and used by upper layer (HLOS).
	 */
	uint16_t ps_count;	/* number of valid samples */
	uint16_t ps_max_samples; /* how many samples can be in the samples array */
	uint32_t ps_dropped;	/* how many samples did the profiler drop due to buffer overruns */

	/*
	 * info data -- sent to remote profiler tool (need swap)
	 */
	struct profile_ext_header ex_hd;

};

/*
 * # of sample buffers in pbuf payload
 */
#ifndef	PBUF_PAYLOAD_SIZE
#define	PBUF_PAYLOAD_SIZE	1792	/* for HLOS driver: must sync with pbuf_public */
typedef enum meta_types {
	PINGPONG_EMPTY,
	PINGPONG_FULL,
	PINGPONG_INUSE,
} flowctrl_t;
#else
typedef enum pnode_c2h_metadata_types	flowctrl_t;	/* for NSS driver */
#endif

#define	MAX_SAMPLES_PER_PBUF	((PBUF_PAYLOAD_SIZE - sizeof(struct nss_profile_sample_ctrl)) / sizeof(struct nss_profile_sample))

struct n2h_meta_header {
	flowctrl_t md_type;	/* N2H (NSS) and receiver (HLOS) flow control (meta type) */
	uint32_t d_len;		/* total data length start from psc_header */
};

struct nss_profile_session {	/* use for per session start */
	uint32_t hd_magic;	/* common ovarlay in all headers */
	uint8_t	num_counters;	/* # performance (app) counters registered (changeable) */
	uint8_t	unused1B;
	uint8_t nc_sts_sel_thrA;
	uint8_t nc_sts_sel_thrB;
	uint32_t ocm_size;
	uint32_t sram_start;

	uint32_t rate;		/* sampling rate */
	uint32_t cpu_id;	/* chip_id register */
	uint32_t cpu_freq;	/* chip clock */
	uint32_t ddr_freq;	/* DDR MEM speed */
	struct profile_counter counters[PROFILE_MAX_APP_COUNTERS];
};

struct nss_profile_n2h_sample_buf {
	struct n2h_meta_header	mh;

	struct nss_profile_sample_ctrl	psc_header;	/* per sample period */
	struct nss_profile_sample	samples[MAX_SAMPLES_PER_PBUF];	/* per thread samples - for NSS send */
};

struct nss_profile_common {
	struct nss_profile_session un;
	/*
	 * changable data sent everything pbuf
	 */
	struct nss_profile_n2h_sample_buf *pn2h; /* sampling ctrl for this sample period */
	struct nss_profile_sample *samples;	/* samples array was allocated by the linux driver */
					/* now NSS points it to pbuf + pn2h header */
	uint16_t	cur;		/* pos where driver take (read) samples */
	int16_t		enabled;	/* Is the profiler enabled to take samples? */
};

/*
 * Each buffer holds 4-7 sets (sample intrs) of samples, by average 5 sets per buffer
 * at 10ms sample clock; 1s sleep in profilerd requires kernel to have 1000 / 10 / 5 = 20
 * buffers to store one 1s samples. For higher sampling rate, either kernel needs more
 * buffers or profilerd needs less sleep time, but both needs to make match.
 */
#define	CCL_SIZE	32

struct profile_io {
	struct nss_profile_common	pnc;

	/*
	 * control fields - HLOS
	 */
	void	*ctx;		// nss_ctx for Linux driver
	int	profile_first_packet;
	int	profile_sequence_num;

	/*
	 * circular buffers for psc_header/samples
	 */
	struct nss_profile_n2h_sample_buf *ccl;
	int	ccl_read;
	int	ccl_write;

	/*
	 * data from HLOS -- used to generate sample->pid in ULTRA -- no longer needed in NSS
	 * uint32_t task_offset; offset in bytes in thread_info to task_struct pointer
	 * uint32_t pid_offset;	offset in bytes in task_struct to the PID
	 */
	uint32_t *sw_ksp_ptr;	// pointer to array (per hardware thread) of pointers to struct thread_info
};

/*
 * Krait <--> NSS debug mechanism. It lays over on nss_profile_n2h_sample_buf.samples (ccl->samples)
 */
#define	MAX_DB_WR	28	// profile_session has 31 4B words data (32W total with hd_magic) and
#define	MAX_DB_RD	30	// common has two more ptrs
struct debug_box {		// this overlays with profile_common (RD) or profile_session (WR)
	uint32_t hd_magic;	// cmd + MAGIC for packet and endianess check

	uint32_t opts;
	int32_t base_addr;	/* Ubi32 is 32-bit */
	int32_t	dlen;		// in 4B words
	uint32_t data[MAX_DB_RD];
};

#define	DEBUG_OPT_BCTRL	1		// basic CTRL
#define	DEBUG_OPT_MOVEIO	(1<<1)	// force to use moveio in case new OCP range is added

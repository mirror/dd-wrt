/*
 **************************************************************************
 * Copyright (c) 2014,2016,2018, 2020 The Linux Foundation. All rights reserved.
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
 * qsdk/qca/src/qca-nss-drv/profiler/profile.c
 *
 *   Implementation for NetAP Profiler
 */

#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/export.h>
#include <linux/module.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#include <linux/mm.h>
#include <linux/mmzone.h>
#include <linux/fs.h>
#include <linux/page-flags.h>
#include <linux/sched.h>
#include <linux/version.h>
#include <asm/uaccess.h>
#include <asm/page.h>
#include <asm/thread_info.h>
#include <linux/ctype.h>
#include <nss_api_if.h>

#include "profilenode.h"
#include "profpkt.h"

/*
 * This is the driver for the NetAP Core profiler. The system interface to the driver is
 *	profile_register_performance_counter(), defined in <asm/profile.>
 *	a set of proc files (proc/profile/<*>), used by the profiler daemon
 *
 * communication between the profiler components is described in a set of header files.
 * There are multiple versions of these files that must be kept synchronized:
 *	in nss/source/pkg/profile
 *	in tools/profiler
 *	in qsdk/qca/src/qca-nss-drv/profiler
 *
 * profilesample.h specifies the sample format used by pkg/profile, profile driver, and ip3kprof (two versions)
 * profilenode.h specifies the driver node communication between NetAP and the profile driver. (two versions)
 * profpkt.h specifies the network packet format between the profile driver, profile daemon, and ip3kprof (two versions)
 *
 *
 * NSS profile sampler:
 *	pkg/profile/src/profile.c
 *	pkg/profile/include/profilenode.h
 *	pkg/profile/include/profilesample.h
 *
 * profile driver: this code
 *	qsdk/qca/src/qca-nss-drv/profiler
 *
 * profilerd: the user daemon that sends data to the tool
 *	qsdk/qca/feeds/qca/utils/profilerd
 *
 * ubicom32-prof: the Windows tool
 *	tools/profiler/src/(many files)
 *
 */

#ifdef	PROFILE_DEBUG
#define	profileDebug(s, ...) pr_debug("%s[%d]: " s, __func__, __LINE__, ##__VA_ARGS__)
#define	profileInfo(s, ...) pr_info("%s[%d]: " s, __func__, __LINE__, ##__VA_ARGS__)
#else
#define	profileDebug(s, ...)
#define	profileInfo(s, ...)
#endif

#define	profileWarn(s, ...) pr_warn("%s[%d]: " s, __func__, __LINE__, ##__VA_ARGS__)

static void profiler_handle_reply(struct nss_ctx_instance *nss_ctx, struct nss_cmn_msg *ncm);

/*
 * LINUX and Ultra counters must all fit in one packet
 */
#define PROFILE_LINUX_MAX_COUNTERS 40
#define PROFILE_STS_EVENT_COUNTERS 8
#define PROFILE_STS_EVENT_THREAD_BITS 5

static int profile_num_counters = 0;
static volatile unsigned int *profile_counter[PROFILE_LINUX_MAX_COUNTERS];
static char profile_name[PROFILE_LINUX_MAX_COUNTERS][PROFILE_COUNTER_NAME_LENGTH];

/*
 * internal function to check if @name has been registered before
 * return the found index, or -1 otherwise
 */
static int __profile_find_entry(char *name)
{
	int i;

	for (i = 0; i < profile_num_counters; i++) {
		if (!strncasecmp(name, profile_name[i], PROFILE_COUNTER_NAME_LENGTH)) {
			return i;
		}
	}
	return -1;
}

/*
 * profile_register_performance_counter - register @counter into profile tracking list by key @name
 * @counter: pointer of the counter variable
 * @name: identifier of this counter
 *
 * Returns zero if total entries exceeding PROFILE_LINUX_MAX_COUNTERS
 * non-zero otherwise.
 *
 * Each @name gives unique entry for @counter, by allocating a new array slot or just use existing one.
 * No need of de-registration API, since a loadable module's new insmod, will replace the
 * @counter's * new address at the same profile_counter[] slot.
 */
int profile_register_performance_counter(volatile unsigned int *counter, char *name)
{
	int i;

	if (profile_num_counters >= PROFILE_LINUX_MAX_COUNTERS) {
		return 0;
	}

	i = __profile_find_entry(name);
	if (i < 0) {
		i = profile_num_counters++;
	}

	profile_counter[i] = counter;
	strlcpy(profile_name[i], name, PROFILE_COUNTER_NAME_LENGTH);
	profile_name[i][PROFILE_COUNTER_NAME_LENGTH - 1] = 0;

	return 1;
}

/*
 * profile_make_data_packet
 *	Make a packet full of sample data
 */
static int profile_make_data_packet(char *buf, int blen, struct profile_io *pn)
{
	int sp_samples = 0;	/* separated samples if any */
	int ns;		/* number of samples requested */
	struct profile_header ph;
	struct nss_profile_sample_ctrl *psc_hd = &pn->pnc.pn2h->psc_header;

	if (blen < sizeof(ph) + sizeof(struct nss_profile_sample)) {
		return -EINVAL;
	}

	profileInfo("%px stat %x cnt %d %px\n", pn->pnc.pn2h, pn->pnc.pn2h->mh.md_type, psc_hd->ps_count, pn->ccl);

	if (pn->pnc.pn2h->mh.md_type == PINGPONG_EMPTY || psc_hd->ps_count < 1) {
		struct nss_profile_n2h_sample_buf *nsb;
		ns = (pn->ccl_read + 1) & (CCL_SIZE-1);
		nsb = pn->ccl + ns;
		if (ns == pn->ccl_write || nsb->mh.md_type != PINGPONG_FULL) {
			profileInfo("waiting more data %x %px : ns %d rd %d wr %d\n", nsb->mh.md_type, nsb, ns, pn->ccl_read, pn->ccl_write);
			return -EAGAIN;
		}
		pn->ccl_read = ns;
		profileInfo("sp %px => %px rd %d %px\n", pn->pnc.samples, nsb->samples, ns, nsb);
		psc_hd = &nsb->psc_header;
		pn->pnc.pn2h = nsb;
		pn->pnc.samples = nsb->samples;
		pn->pnc.cur = 0;
	}
	pn->pnc.pn2h->mh.md_type = PINGPONG_INUSE;

	/*
	 * fill in the packet header
	 */
	memset(&ph, 0, sizeof(ph));
	ph.pph.magic = htons(PROF_MAGIC + PROFILE_VERSION);
	ph.pph.header_size = sizeof(ph);
	ph.pph.profile_instructions = 0;
	ph.pph.clock_freq = pn->pnc.un.cpu_freq;
	ph.pph.ddr_freq = pn->pnc.un.ddr_freq;
	ph.pph.cpu_id = pn->pnc.un.cpu_id;
	ph.pph.seq_num = htonl(pn->profile_sequence_num);
	ph.pph.sample_stack_words = NSS_PROFILE_STACK_WORDS;

	ns = (blen - sizeof(ph)) / sizeof(struct nss_profile_sample);
	profileInfo("%X: blen %d ns = %d psc_hd count %d ssets %d phs %zu pss %zu\n",
		pn->profile_sequence_num, blen, ns, psc_hd->ps_count,
		psc_hd->ex_hd.sample_sets, sizeof(ph), sizeof(struct nss_profile_sample));
	if (ns > psc_hd->ps_count)
		ns = psc_hd->ps_count;
	if (ns == 0) {
		printk("NS should not be 0: rlen %d hd cnt %d\n", blen, psc_hd->ps_count);
		return 0;
	}

	/*
	 * if buf cannot hold all samples, then samples must be separated by set.
	 */
	if (ns < psc_hd->ps_count) {
		ph.exh.sets_map = psc_hd->ex_hd.sets_map;	/* save for separating sets */
		do {
			sp_samples += psc_hd->ex_hd.sets_map & 0x0F;
			psc_hd->ex_hd.sets_map >>= 4;	/* remove the last set */
			psc_hd->ex_hd.sample_sets--;
			ph.exh.sample_sets++;		/* save for restore later */
		} while ((psc_hd->ps_count - sp_samples) > ns);
		ns = psc_hd->ps_count - sp_samples;
	}
	ph.pph.sample_count = ns;
	if (copy_to_user(buf, &ph.pph, sizeof(ph.pph)) != 0) {
		return -EFAULT;
	}
	buf += sizeof(ph.pph);

	/*
	 * ph.exh is unused dummy; and psc_hd->ex_hd is used directly to avoid double mem copy
	 */
	if (copy_to_user(buf, &psc_hd->ex_hd, sizeof(psc_hd->ex_hd)) != 0) {
		return -EFAULT;
	}
	buf += sizeof(psc_hd->ex_hd);

	blen = ns * sizeof(struct nss_profile_sample);
	profileDebug("-profile_make_data_packet %px slen %d cur %d dcped %zd + %zd\n",
		pn->pnc.samples, blen, pn->pnc.cur, sizeof(ph.pph), sizeof(psc_hd->ex_hd));
	if (copy_to_user(buf, &pn->pnc.samples[pn->pnc.cur], blen) != 0) {
		return -EFAULT;
	}
	pn->pnc.cur += ns;
	psc_hd->ps_count -= ns;
	if (psc_hd->ps_count < 1)
		pn->pnc.pn2h->mh.md_type = PINGPONG_EMPTY;

	/*
	 * restore left over sample counts; 0s for no one
	 */
	if (sp_samples) {
		profileDebug("%d sps %d %d: sets %d : %d map %x <> %x\n", psc_hd->ps_count, ns, sp_samples, psc_hd->ex_hd.sample_sets, ph.exh.sample_sets, psc_hd->ex_hd.sets_map, ph.exh.sets_map);
		psc_hd->ex_hd.sample_sets = ph.exh.sample_sets;
		psc_hd->ex_hd.sets_map = ph.exh.sets_map;
	}

	pn->profile_sequence_num++;
	blen += sizeof(ph);
	profileDebug("+profile_make_data_packet %d phd len %zd nsp %px rd %d cnt %d\n", blen, sizeof(ph), pn->pnc.pn2h, pn->ccl_read, psc_hd->ps_count);
	return blen;
}

static void *profiler_get_dma(struct nss_ctx_instance *nss_ctx, struct profile_io *pn)
{
	struct nss_profile_sdma_producer *dma;
	void *kaddr = nss_profiler_alloc_dma(nss_ctx, &dma);
	pn->pnc.un.sram_start = dma->desc_ring;
	return kaddr;
}

/*
 * This is no longer needed due to NetAP and Linux use different CPUs, and profile is NetAP only.
 * All related code will be removed after corresponging code in visual tool is corrected; otherwise
 * visual tool will mis-behave
 */
struct profile_counter profile_builtin_stats[] =
{
	{
	"Free memory(KB)", 0
	},
	{
	"Max free Block(KB)", 0
	}
};

/*
 * profile_make_stats_packet
 *	make a packet full of performance counters (software)
 */
static int profile_make_stats_packet(char *buf, int bytes, struct profile_io *pn)
{
	static char prof_pkt[NSS_PROFILE_MAX_PACKET_SIZE];

	char *ptr;
	int n;
	struct profile_counter *counter_ptr;
	struct profile_header_counters *hdr = (struct profile_header_counters *)prof_pkt;
	struct nss_profile_sample_ctrl *psc_hd = &pn->pnc.pn2h->psc_header;

	if (bytes > NSS_PROFILE_MAX_PACKET_SIZE) {
		bytes = NSS_PROFILE_MAX_PACKET_SIZE;
	}
	n = sizeof(profile_builtin_stats) + (pn->pnc.un.num_counters + profile_num_counters) * sizeof(*counter_ptr);

	if ((bytes - sizeof(hdr)) < n) {
		profileWarn("room too small %d for cnts %d\n", bytes, n);
		return 0;
	}

	hdr->magic = htons(PROF_MAGIC_COUNTERS);
	hdr->ultra_count = htons(pn->pnc.un.num_counters);
	hdr->linux_count = htonl(profile_num_counters + sizeof(profile_builtin_stats) / sizeof(*counter_ptr));
	hdr->ultra_sample_time = psc_hd->ex_hd.clocks;
	hdr->linux_sample_time = psc_hd->ex_hd.clocks; /* QSDK has no time func */

	n = pn->pnc.un.num_counters;	/* copy NSS counters */
	n *= sizeof(pn->pnc.un.counters[0]);
	ptr = (char*) (hdr + 1);
	memcpy(ptr, (void *)(pn->pnc.un.counters), n);
	ptr += n;

	counter_ptr = (struct profile_counter *)ptr;
	for (n = 0; n < profile_num_counters; ++n) {
		counter_ptr->value = htonl(*profile_counter[n]);
		strlcpy(counter_ptr->name, profile_name[n],
			PROFILE_COUNTER_NAME_LENGTH);
		counter_ptr++;
	}
	ptr = (char*)counter_ptr;

	/*
	 * built in statistics
	profile_get_memory_stats(&total_free, &max_free);
	 */
	profile_builtin_stats[0].value = 0;
	profile_builtin_stats[1].value = 0;
	memcpy(ptr, (void *)profile_builtin_stats, sizeof(profile_builtin_stats));
	ptr += sizeof(profile_builtin_stats);

	n = ptr - prof_pkt;
	if (copy_to_user(buf, prof_pkt, n) != 0) {
		return -EFAULT;
	}
	return n;
}

/*
 * space for all memory blocks so we can hold locks for short time when walking tables
 */
static struct profile_io *node[NSS_MAX_CORES];

/*
 * profile_open
 *	open function of system call
 */
static int profile_open(struct inode *inode, struct file *filp)
{
	int	n;
	struct profile_io *pn;

	if (filp->private_data)
		profileWarn("%s: %px\n", filp->f_path.dentry->d_iname, filp->private_data);

	n = filp->f_path.dentry->d_iname[strlen(filp->f_path.dentry->d_iname) - 1] - '0';
	if (n < 0 || n >= NSS_MAX_CORES)
		n = 0;
	pn = node[n];
	if (!pn) {
		return -ENOENT;
	}

	profileInfo("_open: mode %x flag %x\n", filp->f_mode, filp->f_flags);
	if (!pn->pnc.enabled && nss_get_state(pn->ctx) == NSS_STATE_INITIALIZED) {
		/*
		 * sw_ksp_ptr is used as event flag. NULL means normal I/O
		 */
		pn->sw_ksp_ptr = NULL;
		pn->pnc.enabled = 1;
		pn->profile_first_packet = 1;

		/*
		 * If profiler is opened in read only mode, it is done by START_MSG
		 * via debug interface (IF), which reads NSS-FW all registered NSS
		 * variables.
		 * Do not start engine (no sampling required) for debug IF.
		 */
		if (FMODE_READ & filp->f_mode) {
			nss_tx_status_t ret;

			pn->pnc.un.hd_magic = NSS_PROFILE_HD_MAGIC | NSS_PROFILER_START_MSG;
			ret = nss_profiler_if_tx_buf(pn->ctx, &pn->pnc.un,
				sizeof(pn->pnc.un), profiler_handle_reply, pn);
			profileInfo("%s: %d -- %px: ccl %px sp %px\n", __func__, ret,
				pn, pn->ccl, pn->pnc.samples);
		}
		filp->private_data = pn;
		return 0;
	}

	profileWarn("profile ena %d nss stat %x\n", pn->pnc.enabled,
			nss_get_state(pn->ctx));
	return -EBUSY;
}

/*
 * profile_read
 *	read syscall
 *
 * return a udp packet ready to send to the profiler tool
 * when there are no packets left to make, return 0
 */
static ssize_t profile_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
{
	int result = 0;
	int slen = 0;
	struct profile_io *pn = (struct profile_io *)filp->private_data;
	if (!pn) {
		return -ENOENT;
	}

	if (!pn->pnc.enabled) {
		return -EPERM;
	}
	if (pn->sw_ksp_ptr) {
		struct debug_box *db = (struct debug_box *) pn->sw_ksp_ptr;

		if ((void*)db != (void*)pn) {
			profileWarn("%px: hwe data not ready %px\n", pn, db);
			return -EAGAIN;
		}

		profileWarn("dbda %px: %x %x %x %x %x\n", db->data,
			db->data[0], db->data[2], db->data[4], db->data[6], db->data[7]);

		slen = (PROFILE_STS_EVENT_COUNTERS + 1) * sizeof(db->data[0]);
		if (copy_to_user(buf, db->data, slen))
			return -EFAULT;
		profileInfo("%px: sw_ksp_ptr %px slen %d\n", pn, pn->sw_ksp_ptr, slen);
		return	slen;
	}

	if (!pn->pnc.samples) {
		profileWarn("DEBUG %px: NULL samples\n", pn);
		return -ENOMEM;
	}

	if (pn->profile_first_packet) {
		result = profile_make_stats_packet(buf, count, pn);
		pn->profile_first_packet = 0;
		profileInfo("%d profile_make_stats_packet %zd\n", result, count);

#ifdef	PROFILE_SEP_STAT
		/*
		 * currectly, stat and sample data are combined in one pkt for efficient;
		 * but this is harder to debug and required remote tool to handle
		 * packet in all-in-one method instead of individual handler.
		 */
		return result;
#endif
	}

	if (result > 0) {
		buf += result;
		count -= result;
		slen = result;
	}
	result = profile_make_data_packet(buf, count, pn);
	if (result == 0) {
		pn->profile_first_packet = 1;
	}
	profileInfo("%d: profile_make_data_packet %zd %d\n", result, count, slen);

	profileInfo("%d: read\n", pn->pnc.enabled);
	if (pn->pnc.enabled < 0) {
		nss_tx_status_t ret;
		pn->pnc.enabled = 1;
		pn->pnc.un.hd_magic = NSS_PROFILE_HD_MAGIC | NSS_PROFILER_START_MSG;
		ret = nss_profiler_if_tx_buf(pn->ctx, &pn->pnc.un, sizeof(pn->pnc.un),
						profiler_handle_reply, pn);
		profileWarn("%s: restart %d -- %px: ccl %px sp %px\n", __func__,
				ret, pn, pn->ccl, pn->pnc.samples);
	}

	return result + slen;
}

/*
 * profile_release
 *	the close syscall paired with profiler_open
 */
static int profile_release(struct inode *inode, struct file *filp)
{
	struct profile_io *pn = (struct profile_io *)filp->private_data;
	if (!pn) {
		return -ENOENT;
	}

	if (pn->pnc.enabled) {
		nss_tx_status_t ret;
		pn->sw_ksp_ptr = NULL;
		pn->pnc.enabled = 0;
		pn->pnc.un.hd_magic = NSS_PROFILE_HD_MAGIC | NSS_PROFILER_STOP_MSG;
		ret = nss_profiler_if_tx_buf(pn->ctx, &pn->pnc.un,
				sizeof(pn->pnc.un), profiler_handle_reply, pn);
		profileInfo("%s: %px %d\n", __func__, pn, ret);
		return 0;
	}
	profileWarn("%s: attempt closing non-open dev %px\n", __func__, pn);
	pn->profile_first_packet = 1;
	return -EBADF;
}

#ifndef	__aarch64__
/*
 * counter_rate_by_uint32
 *	helper function for handling 64-bit calculation in 32-bit mode
 *
 * 32-bit kernel does not have 64-bit div function;
 * to avoid overflow and underflow, use if branch
 * to overcome this problem: slower bur more accurate.
 */
static void counter_rate_by_uint32(struct nss_profile_common *pnc)
{
	static uint32_t prev_cnts[32];
	static uint32_t last_uclk;
	uint32_t mclk, uclk, ubi32_freq;
	int n = pnc->un.num_counters;

	ubi32_freq = htonl(pnc->un.cpu_freq) / 1000000;
	uclk = pnc->un.rate - last_uclk;
	last_uclk = pnc->un.rate;
	printk("%d nss counters:	clk dif %u freq %u\n", n, uclk, ubi32_freq);

	/*
	 * exactly 4G? make it maximum
	 */
	if (!uclk)
		uclk--;
	while (n--) {
		uint32_t v_dif;
		uint32_t v = ntohl(pnc->un.counters[n].value);
		uint32_t pv = prev_cnts[n];

		prev_cnts[n] = v;
		v_dif = v - pv;

		/*
		 * threshold	= MAX_UINT32 / MAX_Ubi32CPU_CLK (MHz)
		 * if counter diff is less then this threshold,
		 * 32-bit calculation can be directly applied w/o o/u flow;
		 * otherwise, tick diff (uclk) adjust needs to be done before
		 * calculating the rate to avoid over/under flow.
		 */
		if (v_dif < (UINT_MAX / ubi32_freq)) {
			v_dif = (v_dif * ubi32_freq) / (uclk / 1000000);
		} else {
			/*
			 * assume fast polling is 200ms, @ 500MHz, the minimum
			 * uclk value is 0.5M * 200 = 10M, so reduce by 1M
			 * it will still have value in 10, not zero (0).
			 * in 2.3GHz and 1 sec interval, the residual is 2300.
			 * The maximum polling interval is 2 sec for 2.3GHz,
			 * and 3 sec for 1.7GHz.
			 */
			if (uclk > 1000000) {
				mclk = uclk / 1000000;
				v_dif = (v_dif / mclk) * ubi32_freq;
			} else {
				mclk = uclk / 1000;
				v_dif = (v_dif / mclk) * ubi32_freq * 1000;
			}
		}
		printk("%-32s 0x%08X	%10u :	%u/s\n",
			pnc->un.counters[n].name, v, v, v_dif);
	}
}
#endif

/*
 * profiler_handle_counter_event_reply()
 *	get reply from firmware for current FW stat event counter configurations
 *
 * Based on firmware CPU clock (cpu_freq), calculate the counter change rate in
 * second and print both counter value and its rate.
 */
static void profiler_handle_counter_event_reply(struct nss_ctx_instance *nss_ctx,
						struct nss_cmn_msg *ncm)
{
	struct profile_io *pio = (struct profile_io *) ncm->app_data;
	struct nss_profile_common *pnc = &pio->pnc;

#ifndef __aarch64__
	counter_rate_by_uint32(pnc);
#else
	static uint32_t prev_cnts[32];
	static uint32_t last_uclk;
	uint32_t ubi32_freq;
	uint32_t uclk;
	int n = pnc->un.num_counters;

	ubi32_freq = htonl(pnc->un.cpu_freq);
	uclk = pnc->un.rate - last_uclk;
	last_uclk = pnc->un.rate;
	printk("%d nss counters:	clk dif %u freq %u\n", n, uclk, ubi32_freq);
	while (n--) {
		uint32_t v = ntohl(pnc->un.counters[n].value);
		uint32_t pv = prev_cnts[n];

		prev_cnts[n] = v;

		printk("%-32s 0x%08X	%10u :	%llu/s\n",
			pnc->un.counters[n].name, v, v,
			(uint64_t)(v - pv) * ubi32_freq / uclk);
	}
#endif
}

/*
 * parseDbgCmd()
 *	process debugging command(s).
 *
 * Currently supported command:
 *	"=show-nss-counter"	display all values of nss variables registered
 *				by profile_register_performance_counter(&v, name)
 */
#define	SHOW_COUNTER_CMD "show-nss-counter"
static int parseDbgCmd(const char *buf, size_t count,
			struct debug_box *db, struct profile_io *pio)
{
	int result;

	if (strncmp(buf, SHOW_COUNTER_CMD, min(sizeof(SHOW_COUNTER_CMD)-1, count))) {
		printk(KERN_ERR "%s: unsupported cmd %s %zu\n",
			__func__, buf, strlen(buf));
		return -EINVAL;
	}

	db->hd_magic = NSS_PROFILE_HD_MAGIC | NSS_PROFILER_COUNTERS_MSG;
	result = nss_profiler_if_tx_buf(pio->ctx, &pio->pnc.un,
					sizeof(pio->pnc.un),
					profiler_handle_counter_event_reply, pio);
	profileInfo("%s: %d\n", __func__, result);
	return result == NSS_TX_SUCCESS ? count : -EFAULT;
}

/*
 * profiler_handle_stat_event_reply()
 *	print current FW stat event counter configurations
 */
static void profiler_handle_stat_event_reply(struct nss_ctx_instance *nss_ctx,
						struct nss_cmn_msg *ncm)
{
	struct profile_io *pio = (struct profile_io *) ncm->app_data;
	struct debug_box *pdb = (struct debug_box *) &pio->pnc;
	struct debug_box *db = (struct debug_box *) &ncm[1];
	int i, thrds;

	for (i = 0; i < db->dlen; i++)
		printk("stat counter %d: %x\n", i, db->data[i]);

	thrds = db->data[i];
	i = (1 << PROFILE_STS_EVENT_THREAD_BITS) - 1;
	profileInfo("%d: event end mark %x, ThrA %d ThrB %d\n",
		ncm->len, thrds, (thrds & i) + 1,
		((thrds >> PROFILE_STS_EVENT_THREAD_BITS) & i) + 1);

	/*
	 * save data for read()
	 */
	memcpy(pdb->data, db->data, (db->dlen + 1) * sizeof(db->data[0]));
	pio->sw_ksp_ptr = (uint32_t *)pdb;
}

/*
 * parse_sys_stat_event_req()
 *	process FW stat events request: event#1 index#1 event#2 index#2 ...
 */
static int parse_sys_stat_event_req(const char *buf, size_t count,
				struct debug_box *db, struct profile_io *pio)
{
	char *cp;
	int result;

	printk("%zd cmd buf %s\n", count, buf);
	if (count < 19) /* minimum data for sys_stat_event request */
		return	-EINVAL;

	if (strncmp(buf, "get-sys-stat-events", 19) == 0) {
		db->hd_magic = NSS_PROFILE_HD_MAGIC | NSS_PROFILER_GET_SYS_STAT_EVENT;
		result = nss_profiler_if_tx_buf(pio->ctx, &pio->pnc.un,
					sizeof(pio->pnc.un),
					profiler_handle_stat_event_reply, pio);
		profileInfo("get_sys_stat_events: %d\n", result);
		return	result == NSS_TX_SUCCESS ? count : -EFAULT;
	}

	if (strncmp(buf, "set-sys-stat-events", 19)) {
		printk("unknow event: %s\n", buf);
		return	-EINVAL;
	}

	db->dlen = sizeof(pio->pnc.un);
	memset(db->data, 0, PROFILE_STS_EVENT_COUNTERS * sizeof(db->data[0]));

	cp = strchr(buf, ' ');
	if (!cp) {
		printk("no enough paramters %s\n", buf);
		return	-EINVAL;
	}

	do {
		unsigned long idx;
		int event, e5x;
		char *kstrp;

		while (isspace(*cp))
			cp++;
		kstrp = strchr(cp, ' ');
		if (!kstrp) {
			printk(KERN_ERR "%px missing index %px %s\n", buf, cp, cp);
			return	-EINVAL;
		}
		kstrp[0] = 0;

		/*
		 * kstrtoul bugs:
		 *	it does not use white space for delimiter.
		 *	it cannot use base 0, thus base 10 only.
		 */
		event = kstrtoul(cp, 10, &idx);
		if (event) {
			printk(KERN_ERR "kstrtoul %d: %s\n", event, cp);
			return -EINVAL;
		}
		event = idx;

		/*
		 * Processing thread specific events, which requires hex values.
		 * Because kstrtoul cannot use base 0, it makes this task harder
		 * in user space. Users need to convert hex value to decimal, then
		 * pass them in userland command event-counter.
		 */
		e5x = event >> 16;
		if (e5x) {
			if ((event & 0x1FF) < 50) {
				printk(KERN_INFO "thr ID (%d) ignored for event %d\n",
					e5x, event & 0x1FF);
			} else if (e5x > 12) {
				if ((e5x >>= 5) > 12) {
					printk(KERN_INFO "tID %d too big [1..12]\n", e5x);
					return	-E2BIG;
				}
			}
		}

		cp = kstrp + 1;
		while (isspace(*cp))
			cp++;
		kstrp = strchr(cp, ' ');
		if (kstrp) {
			kstrp[0] = 0;
			kstrp++;
		}

		if (kstrtoul(cp, 10, &idx) || idx < 0 || idx > 7) {
			printk(KERN_ERR "bad index %ld [0..7]\n", idx);
			return	-ERANGE;
		}
		printk(KERN_INFO "%px: e %d i %ld\n", db, event, idx);
		db->data[idx] = event;
		cp = kstrp;
	} while (cp);
	db->hd_magic = NSS_PROFILE_HD_MAGIC | NSS_PROFILER_SET_SYS_STAT_EVENT;
	result = nss_profiler_if_tx_buf(pio->ctx, &pio->pnc.un, sizeof(pio->pnc.un),
				profiler_handle_stat_event_reply, pio);
	profileInfo("%px: %zd send cmd %x to FW ret %d\n",
			db, count, db->hd_magic, result);
	return	count;
}

/*
 * parseDbgData()
 *	parsing debug requests: base_address [options] cmd length
 *
 * cmd is either read or write
 * option is one of mio, moveio, h [heap security verify], etc.
 */
static int parseDbgData(const char *buf, size_t count, struct debug_box *db)
{
	char *cp;
	int n;

	printk("%px %px: buf (%s) cnt %zd\n", db, buf, buf, count);
	if (sscanf(buf, "%x", (uint32_t *)&db->base_addr) != 1) {
		printk("%s: cannot get base addr\n", __func__);
		return	-EINVAL;
	}

	cp = strchr(buf, ' ');
	if (!cp) {
noea:		printk("%s: no enough arguments\n", __func__);
		return -EFAULT;
	}

	while (isspace(*cp)) cp++;
	if (!strncmp(cp, "mio", 3) || !strncmp(cp, "moveio", 6)) {
		printk("%px: cp (%s)\n", cp, cp);
		cp = strchr(cp, ' ');
		if (!cp) {
			goto noea;
		}
		db->opts |= DEBUG_OPT_MOVEIO;
	}

	while (isspace(*cp)) cp++;
	printk("base addr %X -- %s", db->base_addr, cp);

	if (!strncmp(cp, "read", 4)) {
		cp = strchr(cp, ' ');
		if (cp) {
			while (isspace(*cp)) cp++;
			sscanf(cp, "%x", &db->dlen);
		}
		return 0;
	}

	n = 0;
	do {
		while (isspace(*cp)) cp++;
		if (sscanf(cp, "%x", db->data+n) != 1) {
			printk("n %d : %s\n", n, cp);
			break;
		}
		printk("write %x to off %zx\n", db->data[n], n * sizeof(db->data[0]));
		n++;
		cp = strchr(cp, ' ');
	} while (cp && n < MAX_DB_WR);
	return n;
}

/*
 * debug_if_show
 *	display memory content read from Phy addr
 */
static void debug_if_show(struct debug_box *db, int buf_len)
{
	int i;

	for (i=0; i < db->dlen; i++) {
		if ((i & 3) == 0)
			printk("\n%zX: ", db->base_addr + i * sizeof(db->base_addr));
		printk("%9x", db->data[i]);
	}
	printk("\ndumped %d (extra 1) blen %d\n", db->dlen, buf_len);
}

/*
 * profiler_handle_debug_reply
 *	show debug message we requested from NSS
 */
static void profiler_handle_debug_reply(struct nss_ctx_instance *nss_ctx, struct nss_cmn_msg *ncm)
{
	debug_if_show((struct debug_box*)&ncm[1], ncm->len);
}

/*
 * debug_if
 *	a generic Krait <--> NSS debug interface
 */
static ssize_t debug_if(struct file *filp,
			const char __user *ubuf, size_t count, loff_t *f_pos)
{
	char *buf;
	int result;
	struct debug_box *db;
	struct profile_io *pio = (struct profile_io *)filp->private_data;

	if (!pio) {
		return -ENOENT;
	}

	if (!pio->pnc.enabled) {
		return -EPERM;
	}

	buf = kmalloc(count, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	if (copy_from_user(buf, ubuf, count)) {
		kfree(buf);
		printk(KERN_ERR "copy_from_user\n");
		return -EIO;
	}
	buf[count-1] = 0;

	db = (struct debug_box *) &pio->pnc;
	db->dlen = db->opts = 0;

	/*
	 * process possible commands
	 */
	if (buf[0] == '=') {
		result = parseDbgCmd(buf+1, count, db, pio);
		kfree(buf);
		return result;
	}

	/*
	 * process stat_event request: display/change
	 */
	if (!isdigit(buf[0])) {
		result = parse_sys_stat_event_req(buf, count, db, pio);
		kfree(buf);

		if ((result > 0) && (filp->f_flags & O_RDWR)) {
			/*
			 * set flag so event-counter can read the data from FW
			 */
			pio->sw_ksp_ptr = db->data;
		}
		return	result;
	}

	/*
	 * process memory I/O for debug
	 */
	result = parseDbgData(buf, count, db);
	kfree(buf);
	if (result < 0) {
		return	result;
	}

	if (!result) {
		db->hd_magic = NSS_PROFILE_HD_MAGIC | NSS_PROFILER_DEBUG_RD_MSG;
	} else {
		db->hd_magic = NSS_PROFILE_HD_MAGIC | NSS_PROFILER_DEBUG_WR_MSG;
		db->dlen = result;
	}
	result = nss_profiler_if_tx_buf(pio->ctx, &pio->pnc.un,
			sizeof(pio->pnc.un), profiler_handle_debug_reply, pio);
	printk("dbg res %d dlen = %d opt %x\n", result, db->dlen, db->opts);
	return	count;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
#define HAVE_PROC_OPS
#endif

#ifdef HAVE_PROC_OPS
static const struct proc_ops profile_fops = {
  .proc_open = profile_open,
  .proc_read = profile_read,
  .proc_lseek = seq_lseek,
  .proc_release = profile_release,
  .proc_write = debug_if,
};
#else
static const struct file_operations profile_fops = {
	.open		= profile_open,
	.read		= profile_read,
	.release	= profile_release,
	.write		= debug_if,
};
#endif

/*
 * showing sample status on Linux console
 */
static int profile_rate_show(struct seq_file *m, void *v)
{
	struct profile_io *pn = node[0];
	if (pn) {
		struct nss_profile_sample_ctrl *psc_hd = &pn->pnc.pn2h->psc_header;
		seq_printf(m, "%d samples per second.  %d ultra, %d linux virtual counters.  %d dropped samples.  %d queued of %d max sampels.  %d sent packets.\n",
			pn->pnc.un.rate, pn->pnc.un.num_counters, profile_num_counters, psc_hd->ps_dropped, psc_hd->ps_count, psc_hd->ps_max_samples, pn->profile_sequence_num);
	} else {
		seq_printf(m, "Profiler is not initialized.\n");
	}
	return 0;
}

static int profile_rate_open(struct inode *inode, struct file *filp)
{
	return single_open(filp, profile_rate_show, NULL);
}

static ssize_t profile_rate_write(struct file *filp, const char *buf, size_t len, loff_t *off)
{
	*off = 0;
	return 0;
}

#ifdef HAVE_PROC_OPS
static const struct proc_ops profile_rate_fops = {
  .proc_open = profile_rate_open,
  .proc_read = seq_read,
  .proc_lseek = seq_lseek,
  .proc_release = single_release,
  .proc_write = profile_rate_write,
};
#else
static const struct file_operations profile_rate_fops = {
	.open		= profile_rate_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
	.write		= profile_rate_write,
};
#endif

/*
 * hexdump
 *	hex dump for debug
 */
static void kxdump(void *buf, int len, const char *who)
{
	int32_t *ip = (int32_t *) buf;
	int lns = len >> 5;	/* 32-B each line */
	if (lns > 8)
		lns = 8;
	printk("%px: kxdump %s: len %d\n", buf, who, len);
	do {
		printk("%x %x %x %x %x %x %x %x\n", ip[0], ip[1], ip[2], ip[3], ip[4], ip[5], ip[6], ip[7]);
		ip += 8;
	} while (lns--);
}

/*
 * profiler_magic_verify
 *	check magic # and detect Endian.
 *
 * negtive return means failure.
 * return 1 means need to ntoh swap.
 */
static int profiler_magic_verify(struct nss_profile_sample_ctrl *psc_hd, int buf_len)
{
	int swap = 0;
	if ((psc_hd->psc_magic & NSS_PROFILE_HD_MMASK) != NSS_PROFILE_HD_MAGIC) {
		if ((psc_hd->psc_magic & NSS_PROFILE_HD_MMASK_REV) != NSS_PROFILE_HD_MAGIC_REV) {
			kxdump(psc_hd, buf_len, "bad profile packet");
			printk("bad profile HD magic 0x%x : %d\n",
				psc_hd->psc_magic, buf_len);
			return -1;
		}
		profileDebug("Profile data in different Endian type %x\n", psc_hd->psc_magic);
		swap = 1;
		psc_hd->psc_magic = ntohl(psc_hd->psc_magic);
	}
	return swap;
}

/*
 * profile_handle_nss_data
 *	process profile sample data from NSS
 */
static void profile_handle_nss_data(void *arg, struct nss_profiler_msg *npm)
{
	int buf_len = npm->cm.len;
	void *buf = &npm->payload;
	struct profile_io *pn;
	struct nss_profile_n2h_sample_buf *nsb;
	struct nss_profile_sample_ctrl *psc_hd = (struct nss_profile_sample_ctrl *)buf;
	int	ret, wr;
	int	swap = 0;	/* only for header and info data, not samples */

	if (buf_len < (sizeof(struct nss_profile_session) - sizeof(struct profile_counter) * (PROFILE_MAX_APP_COUNTERS))) {
		profileWarn("%px: profile data packet is too small to be useful %d %x psc_hd %px\n",
			npm, buf_len, npm->cm.interface, psc_hd);
		return;
	}

	swap = profiler_magic_verify(psc_hd, buf_len);
	if (swap < 0) {
		return;
	}

	pn = (struct profile_io *)arg;
	profileDebug("PN %px CM msg %d len %d\n", pn, npm->cm.type, buf_len);
	profileInfo("%s: dlen %d swap %d cmd %x - %d\n", __func__, buf_len, swap, npm->cm.type, (pn->ccl_read - pn->ccl_write) & (CCL_SIZE-1));
	//kxdump(buf, buf_len, "process profile packet");

	if (npm->cm.type == NSS_PROFILER_FIXED_INFO_MSG) {
		struct nss_profile_session *pTx = (struct nss_profile_session *)buf;
		if (swap) {
			pn->pnc.un.rate = ntohl(pTx->rate);
			pn->pnc.un.cpu_id = ntohl(pTx->cpu_id);
			pn->pnc.un.cpu_freq = ntohl(pTx->cpu_freq);
			pn->pnc.un.ddr_freq = ntohl(pTx->ddr_freq);
			pn->pnc.un.num_counters = pTx->num_counters;
		} else {
			pn->pnc.un = *pTx;
		}
		memcpy(pn->pnc.un.counters, pTx->counters, pn->pnc.un.num_counters * sizeof(pn->pnc.un.counters[0]));
		pn->profile_first_packet = 1;
		return;
	}

	wr = (pn->ccl_write + 1) & (CCL_SIZE-1);
	nsb = pn->ccl + wr;
	swap = (pn->ccl_read - wr) & (CCL_SIZE-1);	/* PROFILER_FLOWCTRL */
	if (nsb->mh.md_type != PINGPONG_EMPTY || (swap && swap < 5)) {
		if (pn->pnc.enabled > 0) {
			pn->pnc.enabled = -1;
			pn->pnc.un.hd_magic = NSS_PROFILE_HD_MAGIC | NSS_PROFILER_STOP_MSG;
			ret = nss_profiler_if_tx_buf(pn->ctx,
					&pn->pnc.un, sizeof(pn->pnc.un),
					profiler_handle_reply, pn);
			profileWarn("%d temp stop sampling engine %d\n", swap, ret);
		}
		if (swap < 3) {
			profileWarn("w%px.%d: %d no room for new profile samples r%px.%d\n", nsb, wr, swap, pn->ccl+pn->ccl_read, pn->ccl_read);
			return;	/* -EMSGSIZE */
		}
	}
	pn->ccl_write = wr;

	/*
	 * sampling data -- hdr NBO swap is done at NSS side via SWAPB.
	 */
	memcpy(&nsb->psc_header, buf, buf_len); /* pn->pnc.pn2h->psc_header = *psc_hd; maybe faster, but take more memory */

	nsb->mh.md_type = PINGPONG_FULL;

	/*
	 * ask for perf_counters (software counters) update every 32 samples
	 */
	if (!wr) {
		pn->pnc.un.hd_magic = NSS_PROFILE_HD_MAGIC | NSS_PROFILER_COUNTERS_MSG;
		ret = nss_profiler_if_tx_buf(pn->ctx, &pn->pnc.un,
				sizeof(pn->pnc.un), profiler_handle_reply, pn);
		if (ret == NSS_TX_FAILURE)
			printk("req counters Cmd failed %d %d\n", ret, wr);
	}
	profileInfo("filled %px %px wr %d\n", nsb, nsb->samples, pn->ccl_write);
}

/*
 * profiler_dma_handler
 *	Handle DMA interrupt, and map DMA to N2H to minimize changes
 * in profile_handle_nss_data.
 */
static void profiler_dma_handler(void *arg)
{
	int cri, idx, widx;
	struct nss_profiler_msg *npm;
	struct nss_profile_sdma_consumer *cbc;
	struct nss_profile_sdma_producer *dma;
	struct profile_io *pn = (struct profile_io *)arg;
	struct nss_profile_sdma_ctrl *ctrl = nss_profile_dma_get_ctrl(pn->ctx);

	if (!ctrl) {
		profileWarn("%px: cannot get dma ctrl block\n", pn->ctx);
		return;
	}

	dma = ctrl->producer;
	cbc = ctrl->consumer;
	cri = ctrl->cur_ring;
	idx = ctrl->cidx[cri];
	widx = ctrl->pidx[cri];

	if (idx == widx) {
		profileInfo("%px: dma[%d]%d %px sz %d no more profile data %px (%zd)\n",
			ctrl, cri, idx, dma, dma->buf_size,
			cbc->ring.kp + idx * dma->buf_size, sizeof(*ctrl));
		return;
	}

	do {
		npm = cbc->ring.kp + idx * dma->buf_size;

		dmac_inv_range(npm, &npm->payload);
		dsb(sy);

		dmac_inv_range(&npm->payload, (void *)&npm->payload + npm->cm.len);
		dsb(sy);

		profile_handle_nss_data(pn, npm);
		idx = (idx + 1) & (dma->num_bufs - 1);
	} while (idx != widx);

	ctrl->cidx[cri] = idx;
	profileInfo("flush %px %px r %d w %d(%d)\n", cbc, cbc->ring.kp, idx, widx, ctrl->pidx[cri]);
	dmac_clean_range(ctrl->cidx + cri, ctrl->cidx + cri + 1);
	dsb(sy);
}

/*
 * profiler_handle_reply
 *	process N2H reply for message we sent to NSS
 */
static void profiler_handle_reply(struct nss_ctx_instance *nss_ctx, struct nss_cmn_msg *ncm)
{
	switch (ncm->response) {
	default:
		if (ncm->error == PROFILE_ERROR_NO_DMA) {
			struct nss_profile_sdma_consumer *cbc;
			struct nss_profile_sdma_ctrl *ctrl;
			struct profile_io *pn = node[0];

			if (!pn || pn->ctx != nss_ctx) {
				pn = node[1];
				if (!pn || pn->ctx != nss_ctx)
					return;
			}

			ctrl = nss_profile_dma_get_ctrl(nss_ctx);
			if (!ctrl) {
				profileWarn("%px: profiler can't get DMA\n", nss_ctx);
				return;
			}

			cbc = ctrl->consumer;
			cbc->ring.kp = profiler_get_dma(nss_ctx, pn);
			if (cbc->ring.kp) {
				pn->pnc.un.hd_magic = NSS_PROFILE_HD_MAGIC | NSS_PROFILER_START_MSG;
				if (nss_profiler_if_tx_buf(pn->ctx, &pn->pnc.un,
					sizeof(pn->pnc.un), profiler_handle_reply, pn)
						== NSS_TX_SUCCESS)
					return;
			}
		}

		profileWarn("%px: profiler had error response %d\n", nss_ctx, ncm->response);
		/*
		 * fail through -- no plan to do anything yet
		 */
	case NSS_CMN_RESPONSE_ACK:
		return;
	}
}

/*
 * profile_prepare_dma()
 *	Allocate DMA for profile if no DMA allocated; then register
 * callback to handle interrupt for reading samples.
 */
static bool profile_prepare_dma(struct profile_io *node)
{
	struct nss_profile_sdma_ctrl *ctrl = nss_profile_dma_get_ctrl(node->ctx);

	if (!ctrl)
		return 0;

	if (!ctrl->consumer[0].ring.kp)
		ctrl->consumer[0].ring.kp = profiler_get_dma(node->ctx, node);

	/*
	 * register_noncd_cb will not fail since (!ctrl) has been checked above.
	 * The ctrl is allocated in driver side, so even in impossible event to
	 * cause register_noncd_cb failed, no need to release dma since calling
	 * this function will not allocate another DMA if a DMA alerady exists.
	 */
	return (bool)nss_profile_dma_register_cb(node->ctx, 0, profiler_dma_handler, (void*)node);
}

/*
 * profile_init
 *	initialize basic profile data structure
 */
static void profile_init(struct profile_io *node)
{
	int n;

	memset(&node->pnc, 0, sizeof(node->pnc));
	node->ccl_read = 0;
	node->ccl_write = -1;
	node->pnc.pn2h = node->ccl;
	node->pnc.samples = node->ccl->samples;

	for (n = 0; n < CCL_SIZE; n++) {
		node->ccl[n].mh.md_type = PINGPONG_EMPTY;
		node->ccl[n].psc_header.ps_count = 0;
	}

	/*
	 * sw_ksp is an array of pointers to struct thread_info,
	 * the current task executing for each linux virtual processor
	node->sw_ksp_ptr = sw_ksp;
	 */
	node->sw_ksp_ptr = NULL;
	/*
	 * Old profile info: unused by now
	 * node->task_offset = offsetof(struct thread_info, task);
	 * node->pid_offset = offsetof(struct task_struct, tgid);
	 */
}

static struct proc_dir_entry *pdir;

/*
 * netap_profile_release_resource
 *	init_module cannot call exit_MODULE, so use this wrapper
 */
void netap_profile_release_resource(void)
{
	if (pdir) {
		remove_proc_entry("rate", pdir);
		remove_proc_entry("data", pdir);
		remove_proc_entry("data1", pdir);
	}
	nss_profile_dma_deregister_cb(node[0]->ctx, 0);
	nss_profile_dma_deregister_cb(node[1]->ctx, 0);
	nss_profiler_release_dma(node[1]->ctx);
	nss_profiler_release_dma(node[0]->ctx);
	kfree(node[0]->ccl);
	kfree(node[0]);
	node[0] = NULL;

}

/*
 * netap_profile_init_module
 *	kernel module entry
 */
int __init netap_profile_init_module(void)
{
#ifdef CONFIG_OF
	/*
	 * If the node is not compatible, don't do anything.
	 */
	if (!of_find_node_by_name(NULL, "nss-common")) {
		return 0;
	}
#endif
	/*
	 * we need N nodes, not one node + N ctx, for N cores
	 */
	node[0] = kmalloc(sizeof(*node[0]) * NSS_MAX_CORES, GFP_KERNEL);
	if (!node[0]) {
		profileWarn("Profiler CTRL kmalloc failed.\n");
		return -ENOMEM;
	}

	node[0]->ccl = kmalloc(sizeof(*node[0]->ccl) * CCL_SIZE * NSS_MAX_CORES, GFP_KERNEL);
	if (!node[0]->ccl) {
		profileWarn("Profiler n2h_sample_buf kmalloc failed.\n");
		kfree(node[0]);
		node[0] = NULL;
		return -ENOMEM;
	}

	/*
	 * connect to the file system
	 */
	pdir = proc_mkdir("profile", NULL);
	if (!pdir ||
	    !proc_create("data", 0, pdir, &profile_fops) ||
	    !proc_create("data1", 0, pdir, &profile_fops) ||
	    !proc_create("rate", 0, pdir, &profile_rate_fops)) {
		netap_profile_release_resource();
		return -ENOMEM;
	}

	profile_init(node[0]);

	/*
	 * attatch the device callback to N2H channel for CPU 0
	 */
	node[0]->ctx = nss_profiler_notify_register(NSS_CORE_0, profile_handle_nss_data, node[0]);
	if (!node[0]->ctx) {
		netap_profile_release_resource();
		return -ENXIO;
	}
	profile_prepare_dma(node[0]);

#if NSS_MAX_CORES > 1
	node[1] = node[0] + 1;
	node[1]->ccl = node[0]->ccl + CCL_SIZE;

	profile_init(node[1]);
	node[1]->ctx = nss_profiler_notify_register(NSS_CORE_1, profile_handle_nss_data, node[1]);
	profile_prepare_dma(node[1]);

	profile_register_performance_counter(&node[1]->profile_sequence_num, "Profile1 DRV data packets");
#endif

	profile_register_performance_counter(&node[0]->profile_sequence_num, "Profile0 DRV data packets");
	return 0;
}

/*
 * netap_profile_exit_module
 *	kernel module exit
 */
void __exit netap_profile_exit_module(void)
{
#ifdef CONFIG_OF
	/*
	 * If the node is not compatible, don't do anything.
	 */
	if (!of_find_node_by_name(NULL, "nss-common")) {
		return;
	}
#endif
	nss_profiler_notify_unregister(NSS_CORE_0);
#if NSS_MAX_CORES > 1
	nss_profiler_notify_unregister(NSS_CORE_1);
#endif
	netap_profile_release_resource();
}

module_init(netap_profile_init_module);
module_exit(netap_profile_exit_module);

MODULE_LICENSE("Dual BSD/GPL");

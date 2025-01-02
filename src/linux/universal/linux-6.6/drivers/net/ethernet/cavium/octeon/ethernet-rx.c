/**********************************************************************
 * Author: Cavium, Inc.
 *
 * Contact: support@cavium.com
 * This file is part of the OCTEON SDK
 *
 * Copyright (c) 2003-2012 Cavium, Inc.
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, Version 2, as
 * published by the Free Software Foundation.
 *
 * This file is distributed in the hope that it will be useful, but
 * AS-IS and WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, TITLE, or
 * NONINFRINGEMENT.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this file; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 * or visit http://www.gnu.org/licenses/.
 *
 * This file may also be available under a different license from Cavium.
 * Contact Cavium, Inc. for more information
 **********************************************************************/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cache.h>
#include <linux/cpumask.h>
#include <linux/netdevice.h>
#include <linux/init.h>
#include <linux/etherdevice.h>
#include <linux/ip.h>
#include <linux/string.h>
#include <linux/prefetch.h>
#include <linux/ratelimit.h>
#include <linux/smp.h>
#include <linux/interrupt.h>
#include <net/dst.h>
#ifdef CONFIG_XFRM
#include <linux/xfrm.h>
#include <net/xfrm.h>
#endif /* CONFIG_XFRM */

#include <asm/octeon/octeon.h>
#include <asm/octeon/octeon-hw-status.h>

#include "ethernet-defines.h"
#include "octeon-ethernet.h"

#include <asm/octeon/cvmx-helper.h>
#include <asm/octeon/cvmx-wqe.h>
#include <asm/octeon/cvmx-hwfau.h>
#include <asm/octeon/cvmx-pow.h>
#include <asm/octeon/cvmx-pip.h>
#include <asm/octeon/cvmx-ipd.h>
#include <asm/octeon/cvmx-srio.h>
#include <asm/octeon/cvmx-scratch.h>

#include <asm/octeon/cvmx-gmxx-defs.h>
#include <asm/octeon/cvmx-sso-defs.h>

struct cvm_napi_wrapper {
	struct napi_struct napi;
	int available;
} ____cacheline_aligned_in_smp;

static struct cvm_napi_wrapper cvm_oct_napi[NR_CPUS] __cacheline_aligned_in_smp;

struct cvm_oct_core_state {
	int baseline_cores;
	/* We want to read this without having to acquire the lock,
	 * make it volatile so we are likely to get a fairly current
	 * value.
	 */
	volatile int active_cores;
	/* cvm_napi_wrapper.available and active_cores must be kept
	 * consistent with this lock.
	 */
	spinlock_t lock;
} ____cacheline_aligned_in_smp;

static struct cvm_oct_core_state core_state __cacheline_aligned_in_smp;

#ifdef CONFIG_SMP
static int cvm_oct_enable_one_message;
#endif

static void cvm_oct_enable_napi(void)
{
	int cpu = smp_processor_id();
	napi_schedule(&cvm_oct_napi[cpu].napi);
}

static void cvm_oct_enable_one_cpu(void)
{
	int cpu;
	unsigned long flags;
	spin_lock_irqsave(&core_state.lock, flags);
	/* ... if a CPU is available, Turn on NAPI polling for that CPU.  */
	for_each_online_cpu(cpu) {
		if (cvm_oct_napi[cpu].available > 0) {
			cvm_oct_napi[cpu].available--;
			core_state.active_cores++;
			spin_unlock_irqrestore(&core_state.lock, flags);
			if (cpu == smp_processor_id()) {
				cvm_oct_enable_napi();
			} else {
#ifdef CONFIG_SMP
				octeon_send_ipi_single(cpu, cvm_oct_enable_one_message);
#else
				BUG();
#endif
			}
			goto out;
		}
	}
	spin_unlock_irqrestore(&core_state.lock, flags);
out:
	return;
}

static void cvm_oct_no_more_work(struct napi_struct *napi)
{
	struct cvm_napi_wrapper *nr = container_of(napi, struct cvm_napi_wrapper, napi);
	int current_active;
	unsigned long flags;

	spin_lock_irqsave(&core_state.lock, flags);

	core_state.active_cores--;
	current_active = core_state.active_cores;
	nr->available++;
	BUG_ON(nr->available != 1);

	spin_unlock_irqrestore(&core_state.lock, flags);

	if (current_active == 0) {
		/* No more CPUs doing processing, enable interrupts so
		 * we can start processing again when there is
		 * something to do.
		 */
		if (OCTEON_IS_MODEL(OCTEON_CN68XX)) {
			union cvmx_sso_wq_int_thrx int_thr;
			int_thr.u64 = 0;
			int_thr.s.iq_thr = 1;
			int_thr.s.ds_thr = 1;
			/*
			 * Enable SSO interrupt when our port has at
			 * least one packet.
			 */
			cvmx_write_csr(CVMX_SSO_WQ_INT_THRX(pow_receive_group),
				       int_thr.u64);
		} else {
			union cvmx_pow_wq_int_thrx int_thr;
			int_thr.u64 = 0;
			int_thr.s.iq_thr = 1;
			int_thr.s.ds_thr = 1;
			/* Enable POW interrupt when our port has at
			 * least one packet.
			 */
			cvmx_write_csr(CVMX_POW_WQ_INT_THRX(pow_receive_group),
				       int_thr.u64);
		}
	}
}

/**
 * cvm_oct_do_interrupt - interrupt handler.
 *
 * The interrupt occurs whenever the POW has packets in our group.
 *
 */
static irqreturn_t cvm_oct_do_interrupt(int cpl, void *dev_id)
{
	int cpu = smp_processor_id();
	unsigned long flags;

	/* Disable the IRQ and start napi_poll. */
	if (OCTEON_IS_MODEL(OCTEON_CN68XX)) {
		cvmx_write_csr(CVMX_SSO_WQ_INT_THRX(pow_receive_group), 0);
		cvmx_write_csr(CVMX_SSO_WQ_INT, 1ULL << pow_receive_group);
	} else {
		union cvmx_pow_wq_int wq_int;

		cvmx_write_csr(CVMX_POW_WQ_INT_THRX(pow_receive_group), 0);

		wq_int.u64 = 0;
		wq_int.s.wq_int = 1 << pow_receive_group;
		cvmx_write_csr(CVMX_POW_WQ_INT, wq_int.u64);
	}

	spin_lock_irqsave(&core_state.lock, flags);

	/* ... and NAPI better not be running on this CPU.  */
	BUG_ON(cvm_oct_napi[cpu].available != 1);
	cvm_oct_napi[cpu].available--;

	/* There better be cores available...  */
	core_state.active_cores++;
	BUG_ON(core_state.active_cores > core_state.baseline_cores);

	spin_unlock_irqrestore(&core_state.lock, flags);

	cvm_oct_enable_napi();

	return IRQ_HANDLED;
}

/**
 * cvm_oct_check_rcv_error - process receive errors
 * @work: Work queue entry pointing to the packet.
 *
 * Returns Non-zero if the packet can be dropped, zero otherwise.
 */
static int cvm_oct_check_rcv_error(cvmx_wqe_t *work)
{
	bool err  = false;
	int port = cvmx_wqe_get_port(work);

	if ((work->word2.snoip.err_code == 10) && (work->word1.len <= 64)) {
		/* Ignore length errors on min size packets. Some
		 * equipment incorrectly pads packets to 64+4FCS
		 * instead of 60+4FCS.  Note these packets still get
		 * counted as frame errors.
		 */
	} else if (USE_10MBPS_PREAMBLE_WORKAROUND &&
		   ((work->word2.snoip.err_code == 5)
		    || (work->word2.snoip.err_code == 7))) {

		/* We received a packet with either an alignment error
		 * or a FCS error. This may be signalling that we are
		 * running 10Mbps with GMXX_RXX_FRM_CTL[PRE_CHK}
		 * off. If this is the case we need to parse the
		 * packet to determine if we can remove a non spec
		 * preamble and generate a correct packet.
		 */
		int interface = cvmx_helper_get_interface_num(port);
		int index = cvmx_helper_get_interface_index_num(port);
		union cvmx_gmxx_rxx_frm_ctl gmxx_rxx_frm_ctl;
		uint64_t frm_ctl_reg;

		if (cvmx_helper_interface_get_mode(interface) ==
			CVMX_HELPER_INTERFACE_MODE_AGL)
			frm_ctl_reg = CVMX_AGL_GMX_RXX_FRM_CTL(index);
		else
			frm_ctl_reg = CVMX_GMXX_RXX_FRM_CTL(index, interface);

		gmxx_rxx_frm_ctl.u64 = cvmx_read_csr(frm_ctl_reg);
		if (gmxx_rxx_frm_ctl.s.pre_chk == 0) {

			u8 *ptr = phys_to_virt(work->packet_ptr.s.addr);
			int i = 0;

			while (i < work->word1.len - 1) {
				if (*ptr != 0x55)
					break;
				ptr++;
				i++;
			}

			if (*ptr == 0xd5) {
				work->packet_ptr.s.addr += i + 1;
				work->word1.len -= i + 5;
			} else if ((*ptr & 0xf) == 0xd) {
				work->packet_ptr.s.addr += i;
				work->word1.len -= i + 4;
				for (i = 0; i < work->word1.len; i++) {
					*ptr = ((*ptr & 0xf0) >> 4) | ((*(ptr + 1) & 0xf) << 4);
					ptr++;
				}
			} else {
				printk_ratelimited("Port %d unknown preamble, packet dropped\n",
						   port);
				/* cvmx_helper_dump_packet(work); */
				cvm_oct_free_work(work);
				return 1;
			}
		} else {
			err  = true;
		}
	} else {
			err  = true;
	}
	if (err) {
		printk_ratelimited("Port %d receive error code %d, packet dropped\n",
				   port, work->word2.snoip.err_code);
		cvm_oct_free_work(work);
		return 1;
	}

	return 0;
}

#undef CVM_OCT_NAPI_68
#include "ethernet-napi.c"

#define CVM_OCT_NAPI_68
#include "ethernet-napi.c"

static int (*cvm_oct_napi_poll)(struct napi_struct *, int);

#ifdef CONFIG_NET_POLL_CONTROLLER

/**
 * cvm_oct_poll_controller - poll for receive packets
 * device.
 *
 * @dev:    Device to poll. Unused
 */
void cvm_oct_poll_controller(struct net_device *dev)
{
	cvm_oct_napi_poll(NULL, 16);
}
#endif

static struct kmem_cache *cvm_oct_kmem_sso;
static int cvm_oct_sso_fptr_count;

static int cvm_oct_sso_initialize(int num_wqe)
{
	union cvmx_sso_cfg sso_cfg;
	union cvmx_fpa_fpfx_marks fpa_marks;
	int i;
	int rwq_bufs;

	if (!OCTEON_IS_MODEL(OCTEON_CN68XX))
		return 0;

	rwq_bufs = 48 + DIV_ROUND_UP(num_wqe, 26);
	cvm_oct_sso_fptr_count = rwq_bufs;
	cvm_oct_kmem_sso = kmem_cache_create("octeon_ethernet_sso", 256, 128, 0, NULL);
	if (cvm_oct_kmem_sso == NULL) {
		pr_err("cannot create kmem_cache for octeon_ethernet_sso\n");
		return -ENOMEM;
	}

	/*
	 * CN68XX-P1 may reset with the wrong values, put in
	 * the correct values.
	 */
	fpa_marks.u64 = 0;
	fpa_marks.s.fpf_wr = 0xa4;
	fpa_marks.s.fpf_rd = 0x40;
	cvmx_write_csr(CVMX_FPA_FPF8_MARKS, fpa_marks.u64);

	/* Make sure RWI/RWO is disabled. */
	sso_cfg.u64 = cvmx_read_csr(CVMX_SSO_CFG);
	sso_cfg.s.rwen = 0;
	cvmx_write_csr(CVMX_SSO_CFG, sso_cfg.u64);

	while (rwq_bufs) {
		union cvmx_sso_rwq_psh_fptr fptr;
		void *mem;

		mem = kmem_cache_alloc(cvm_oct_kmem_sso, GFP_KERNEL);
		if (mem == NULL) {
			pr_err("cannot allocate memory from octeon_ethernet_sso\n");
			return -ENOMEM;
		}
		for (;;) {
			fptr.u64 = cvmx_read_csr(CVMX_SSO_RWQ_PSH_FPTR);
			if (!fptr.s.full)
				break;
			__delay(1000);
		}
		fptr.s.fptr = virt_to_phys(mem) >> 7;
		cvmx_write_csr(CVMX_SSO_RWQ_PSH_FPTR, fptr.u64);
		rwq_bufs--;
	}
	for (i = 0; i < 8; i++) {
		union cvmx_sso_rwq_head_ptrx head_ptr;
		union cvmx_sso_rwq_tail_ptrx tail_ptr;
		void *mem;

		mem = kmem_cache_alloc(cvm_oct_kmem_sso, GFP_KERNEL);
		if (mem == NULL) {
			pr_err("cannot allocate memory from octeon_ethernet_sso\n");
			return -ENOMEM;
		}

		head_ptr.u64 = 0;
		tail_ptr.u64 = 0;
		head_ptr.s.ptr = virt_to_phys(mem) >> 7;
		tail_ptr.s.ptr = head_ptr.s.ptr;
		cvmx_write_csr(CVMX_SSO_RWQ_HEAD_PTRX(i), head_ptr.u64);
		cvmx_write_csr(CVMX_SSO_RWQ_TAIL_PTRX(i), tail_ptr.u64);
	}
	/* Now enable the SS0  RWI/RWO */
	sso_cfg.u64 = cvmx_read_csr(CVMX_SSO_CFG);
	sso_cfg.s.rwen = 1;
	sso_cfg.s.rwq_byp_dis = 0;
	sso_cfg.s.rwio_byp_dis = 0;
	cvmx_write_csr(CVMX_SSO_CFG, sso_cfg.u64);

	return 0;
}

void cvm_oct_rx_initialize(int num_wqe)
{
	int i;
	struct net_device *dev_for_napi = NULL;

	if (list_empty(&cvm_oct_list))
		panic("No net_devices were allocated.");

#ifdef CONFIG_SMP
	cvm_oct_enable_one_message = octeon_request_ipi_handler(cvm_oct_enable_napi);
	if (cvm_oct_enable_one_message < 0)
		panic("cvm_oct_rx_initialize: No IPI handler handles available\n");
#endif

	if (OCTEON_IS_MODEL(OCTEON_CN68XX))
		cvm_oct_napi_poll = cvm_oct_napi_poll_68;
	else
		cvm_oct_napi_poll = cvm_oct_napi_poll_38;

	dev_for_napi = list_first_entry(&cvm_oct_list,
					struct octeon_ethernet,
					list)->netdev;

	if (max_rx_cpus >= 1  && max_rx_cpus < num_online_cpus())
		core_state.baseline_cores = max_rx_cpus;
	else
		core_state.baseline_cores = num_online_cpus();

	for_each_possible_cpu(i) {
		cvm_oct_napi[i].available = 1;
		netif_napi_add_weight(dev_for_napi, &cvm_oct_napi[i].napi,
			       cvm_oct_napi_poll, rx_napi_weight);
		napi_enable(&cvm_oct_napi[i].napi);
	}
	/* Before interrupts are enabled, no RX processing will occur,
	 * so we can initialize all those things out side of the
	 * lock.
	 */
	spin_lock_init(&core_state.lock);

	/* Register an IRQ hander for to receive POW interrupts */
	i = request_irq(OCTEON_IRQ_WORKQ0 + pow_receive_group,
			cvm_oct_do_interrupt, 0, dev_for_napi->name, &cvm_oct_list);

	if (i)
		panic("Could not acquire Ethernet IRQ %d\n",
		      OCTEON_IRQ_WORKQ0 + pow_receive_group);

	if (cvm_oct_sso_initialize(num_wqe))
		goto err;

	/* Scheduld NAPI now.  This will indirectly enable interrupts. */
	preempt_disable();
	cvm_oct_enable_one_cpu();
	preempt_enable();
	return;
err:
	free_irq(OCTEON_IRQ_WORKQ0 + pow_receive_group, &cvm_oct_list);
	return;
}

void cvm_oct_rx_shutdown0(void)
{
	int i;

	/* Disable POW/SSO interrupt */
	if (OCTEON_IS_MODEL(OCTEON_CN68XX))
		cvmx_write_csr(CVMX_SSO_WQ_INT_THRX(pow_receive_group), 0);
	else
		cvmx_write_csr(CVMX_POW_WQ_INT_THRX(pow_receive_group), 0);

	/* Free the interrupt handler */
	free_irq(OCTEON_IRQ_WORKQ0 + pow_receive_group, &cvm_oct_list);

#ifdef CONFIG_SMP
	octeon_release_ipi_handler(cvm_oct_enable_one_message);
#endif

	/* Shutdown all of the NAPIs */
	for_each_possible_cpu(i)
		netif_napi_del(&cvm_oct_napi[i].napi);
}

void cvm_oct_rx_shutdown1(void)
{
	union cvmx_fpa_quex_available queue_available;
	union cvmx_sso_cfg sso_cfg;
	union cvmx_sso_rwq_pop_fptr pop_fptr;
	union cvmx_sso_rwq_psh_fptr fptr;
	union cvmx_sso_fpage_cnt fpage_cnt;
	int num_to_transfer, count, i;
	void *mem;
	const int sso_fpe_bit = 45;

	if (!OCTEON_IS_MODEL(OCTEON_CN68XX))
		return;

	/* Spurious FPE errors will happen doing this cleanup.
	 * Disable the indication.
	 */
	octeon_hw_status_disable(CVMX_SSO_ERR, 1ull << sso_fpe_bit);

	sso_cfg.u64 = cvmx_read_csr(CVMX_SSO_CFG);
	sso_cfg.s.rwen = 0;
	sso_cfg.s.rwq_byp_dis = 1;
	cvmx_write_csr(CVMX_SSO_CFG, sso_cfg.u64);
	cvmx_read_csr(CVMX_SSO_CFG);
	queue_available.u64 = cvmx_read_csr(CVMX_FPA_QUEX_AVAILABLE(8));

	/* Make CVMX_FPA_QUEX_AVAILABLE(8) % 16 == 0*/
	for (num_to_transfer = (16 - queue_available.s.que_siz) % 16;
	     num_to_transfer > 0; num_to_transfer--) {
		do {
			pop_fptr.u64 = cvmx_read_csr(CVMX_SSO_RWQ_POP_FPTR);
		} while (!pop_fptr.s.val);
		for (;;) {
			fptr.u64 = cvmx_read_csr(CVMX_SSO_RWQ_PSH_FPTR);
			if (!fptr.s.full)
				break;
			__delay(1000);
		}
		fptr.s.fptr = pop_fptr.s.fptr;
		cvmx_write_csr(CVMX_SSO_RWQ_PSH_FPTR, fptr.u64);
	}
	cvmx_read_csr(CVMX_SSO_CFG);

	do {
		queue_available.u64 = cvmx_read_csr(CVMX_FPA_QUEX_AVAILABLE(8));
	} while (queue_available.s.que_siz % 16);

	sso_cfg.s.rwen = 1;
	sso_cfg.s.rwq_byp_dis = 0;
	cvmx_write_csr(CVMX_SSO_CFG, sso_cfg.u64);

	for (i = 0; i < 8; i++) {
		union cvmx_sso_rwq_head_ptrx head_ptr;
		union cvmx_sso_rwq_tail_ptrx tail_ptr;

		head_ptr.u64 = cvmx_read_csr(CVMX_SSO_RWQ_HEAD_PTRX(i));
		tail_ptr.u64 = cvmx_read_csr(CVMX_SSO_RWQ_TAIL_PTRX(i));
		WARN_ON(head_ptr.s.ptr != tail_ptr.s.ptr);

		mem = phys_to_virt(((u64)head_ptr.s.ptr) << 7);
		kmem_cache_free(cvm_oct_kmem_sso, mem);
	}

	count = 0;

	do {
		do {
			pop_fptr.u64 = cvmx_read_csr(CVMX_SSO_RWQ_POP_FPTR);
			if (pop_fptr.s.val) {
				mem = phys_to_virt(((u64)pop_fptr.s.fptr) << 7);
				kmem_cache_free(cvm_oct_kmem_sso, mem);
				count++;
			}
		} while (pop_fptr.s.val);
		fpage_cnt.u64 = cvmx_read_csr(CVMX_SSO_FPAGE_CNT);
	} while (fpage_cnt.s.fpage_cnt);

	WARN_ON(count != cvm_oct_sso_fptr_count);

	sso_cfg.s.rwen = 0;
	sso_cfg.s.rwq_byp_dis = 0;
	cvmx_write_csr(CVMX_SSO_CFG, sso_cfg.u64);
	kmem_cache_destroy(cvm_oct_kmem_sso);
	cvm_oct_kmem_sso = NULL;

	/* Clear any FPE indicators, and reenable. */
	cvmx_write_csr(CVMX_SSO_ERR, 1ull << sso_fpe_bit);
	octeon_hw_status_enable(CVMX_SSO_ERR, 1ull << sso_fpe_bit);
}

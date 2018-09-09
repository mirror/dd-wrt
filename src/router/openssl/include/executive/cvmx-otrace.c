/***********************license start***************
 * Copyright (c) 2003-2010  Cavium Inc. (support@cavium.com). All rights
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
 * cvmx-otrace implements the SE stub for the runtime tool otrace.
 */

#include <execinfo.h>
#include <stdint.h>

#include "cvmx.h"
#include "cvmx-mbox.h"
#include "cvmx-otrace.h"
#include "cvmx-bootmem.h"
#include "cvmx-interrupt.h"

/*
 * The no-instrumentation version of the SDK functions. {{{
 */

/*
 * See cvmx_clock_get_count(CVMX_CLOCK_CORE).
 */
static uint64_t cvmx_get_cycle_noinst(void) __attribute__((no_instrument_function));
static inline uint64_t cvmx_get_cycle_noinst(void)
{
	uint64_t cycle;
	CVMX_RDHWR(cycle, 31);
	return (cycle);
}
/* }}} */
/**
 * otc_calls for the misc commands in otrace master {{{
 *
 * functions meant to be called via the otc_call interface (by the otrace
 * master)
 */

static uint64_t otc_call_get_var(uint64_t *addr, uint64_t size)
{
	switch(size) {
	case 1: return (uint64_t)(*(uint8_t *)addr); break;
	case 2: return (uint64_t)(*(uint16_t *)addr); break;
	case 4: return (uint64_t)(*(uint32_t *)addr); break;
	case 8: 
	default: return *addr; break;
	}
}

static uint64_t otc_call_set_var(uint64_t *addr, uint64_t val, uint64_t size)
{
	switch(size) {
	case 1: *(uint8_t *)addr = (uint8_t)val; break;
	case 2: *(uint16_t *)addr = (uint16_t)val; break;
	case 4: *(uint32_t *)addr = (uint32_t)val; break;
	case 8:
	default: *addr = val; break;
	}

	return 0;
}

static uint64_t otc_call_get_cop0(uint64_t reg_no, uint64_t select)
{
	uint64_t blob, val;

	blob = (reg_no << 4) + select;
	switch (blob) {
	case 0x000: CVMX_MF_COP0(val, COP0_INDEX); break;
	case 0x010: CVMX_MF_COP0(val, COP0_RANDOM); break;
	case 0x020: CVMX_MF_COP0(val, COP0_ENTRYLO0); break;
	case 0x030: CVMX_MF_COP0(val, COP0_ENTRYLO1); break;
	case 0x040: CVMX_MF_COP0(val, COP0_CONTEXT); break;
	case 0x050: CVMX_MF_COP0(val, COP0_PAGEMASK); break;
	case 0x051: CVMX_MF_COP0(val, COP0_PAGEGRAIN); break;
	case 0x060: CVMX_MF_COP0(val, COP0_WIRED); break;
	case 0x070: CVMX_MF_COP0(val, COP0_HWRENA); break;
	case 0x080: CVMX_MF_COP0(val, COP0_BADVADDR); break;
	case 0x090: CVMX_MF_COP0(val, COP0_COUNT); break;
	case 0x096: CVMX_MF_COP0(val, COP0_CVMCOUNT); break;
	case 0x097: CVMX_MF_COP0(val, COP0_CVMCTL); break;
	case 0x0a0: CVMX_MF_COP0(val, COP0_ENTRYHI); break;
	case 0x0b0: CVMX_MF_COP0(val, COP0_COMPARE); break;
	case 0x0b6: CVMX_MF_COP0(val, COP0_POWTHROTTLE); break;
	case 0x0b7: CVMX_MF_COP0(val, COP0_CVMMEMCTL); break;
	case 0x0c0: CVMX_MF_COP0(val, COP0_STATUS); break;
	case 0x0c1: CVMX_MF_COP0(val, COP0_INTCTL); break;
	case 0x0c2: CVMX_MF_COP0(val, COP0_SRSCTL); break;
	case 0x0d0: CVMX_MF_COP0(val, COP0_CAUSE); break;
	case 0x0e0: CVMX_MF_COP0(val, COP0_EPC); break;
	case 0x0f0: CVMX_MF_COP0(val, COP0_PRID); break;
	case 0x0f1: CVMX_MF_COP0(val, COP0_EBASE); break;
	case 0x100: CVMX_MF_COP0(val, COP0_CONFIG); break;
	case 0x101: CVMX_MF_COP0(val, COP0_CONFIG1); break;
	case 0x102: CVMX_MF_COP0(val, COP0_CONFIG2); break;
	case 0x103: CVMX_MF_COP0(val, COP0_CONFIG3); break;
	case 0x120: CVMX_MF_COP0(val, COP0_WATCHLO0); break;
	case 0x121: CVMX_MF_COP0(val, COP0_WATCHLO1); break;
	case 0x130: CVMX_MF_COP0(val, COP0_WATCHHI0); break;
	case 0x131: CVMX_MF_COP0(val, COP0_WATCHHI1); break;
	case 0x140: CVMX_MF_COP0(val, COP0_XCONTEXT); break;
	case 0x160: CVMX_MF_COP0(val, COP0_MULTICOREDEBUG); break;
	case 0x170: CVMX_MF_COP0(val, COP0_DEBUG); break;
	case 0x180: CVMX_MF_COP0(val, COP0_DEPC); break;
	case 0x190: CVMX_MF_COP0(val, COP0_PERFCONTROL0); break;
	case 0x192: CVMX_MF_COP0(val, COP0_PERFCONTROL1); break;
	case 0x191: CVMX_MF_COP0(val, COP0_PERFVALUE0); break;
	case 0x193: CVMX_MF_COP0(val, COP0_PERFVALUE1); break;
	case 0x1b0: CVMX_MF_COP0(val, COP0_CACHEERRI); break;
	case 0x1b1: CVMX_MF_COP0(val, COP0_CACHEERRD); break;
	case 0x1c0: CVMX_MF_COP0(val, COP0_TAGLOI); break;
	case 0x1c2: CVMX_MF_COP0(val, COP0_TAGLOD); break;
	case 0x1c1: CVMX_MF_COP0(val, COP0_DATALOI); break;
	case 0x1c3: CVMX_MF_COP0(val, COP0_DATALOD); break;
	case 0x1d2: CVMX_MF_COP0(val, COP0_TAGHI); break;
	case 0x1d1: CVMX_MF_COP0(val, COP0_DATAHII); break;
	case 0x1d3: CVMX_MF_COP0(val, COP0_DATAHID); break;
	case 0x1e0: CVMX_MF_COP0(val, COP0_ERROREPC); break;
	case 0x1f0: CVMX_MF_COP0(val, COP0_DESAVE); break;
	default: val = (uint64_t)-1; break;
	}

	return val;
}

/* otc_calls for the misc commands in otrace master }}} */
/**
 * otrace stub {{{
 */
static void *otrace_find_buf(char *block_name) /* find the buffer in the named block */
{
	const cvmx_bootmem_named_block_desc_t *d;
	
	d = cvmx_bootmem_find_named_block(block_name);
	if (!d)
		return NULL;
	
	return cvmx_phys_to_ptr(d->base_addr);
}

typedef void (*otrace_call_t)(otrace_cmd_buf_t *pcmd);

static otrace_cmd_buf_t *otc_buf;	/* command buffer */
static otrace_log_que_t *otl_queue;	/* log queue */

static int otc_call_trace(int start)
{
	char block_name[32];
	int core;

	if ((start && OCTEON_IS_TRACED()) || ((!start) && (!OCTEON_IS_TRACED())))
		return 0;
	
	if (start) {
		core = cvmx_get_core_num();
		OCTEON_TRACE_LOG_NBLOCK(block_name, core);
		otl_queue = (otrace_log_que_t *)
		    otrace_find_buf(block_name);
		if (!otl_queue) {
			cvmx_dprintf("%s: no otl buf on core %d\n",
			    __func__, core);
			return -1;
		}

		octeon_set_attr(OCTEON_ATTR_TRACED);
	} else {
		octeon_clear_attr(OCTEON_ATTR_TRACED);
		otl_queue = NULL;
	}

	return 0;
}

static void otc_call(otrace_cmd_buf_t *pcmd)
{
	/*
	 * otc_cmd - OTC_CALL
	 * 	otc_arg[0] = func_retl  (length of the return val in #bits)
	 * 	otc_arg[1] = func_addr
	 * 	otc_arg[2] = func_argc
	 * 	otc_arg[3] = func_argv0
	 * 	otc_arg[4] = func_argv1
	 * 	...
	 *
	 * otrace_ret - OTR_SUCCESS/OTR_FAIL
	 * 	otc_rval[0] = return val
	 *
	 * The main limitations are 
	 * 1. an argument cannot exceed 64-bit,
	 * 2. return value cannot exceed 64-bit, and
	 * 3. arguments can be pointers, but the otrace master cannot directly
	 *    access the data object pointed to.
	 */
	uint64_t func_retl, func_addr, func_argc;
	uint64_t *p;

	func_retl = pcmd->otc_arg[0];
	func_addr = pcmd->otc_arg[1];
	func_argc = pcmd->otc_arg[2];
	p = &pcmd->otc_arg[3];

#if 0
{
	int i;

	cvmx_dprintf("%s: func_retl0x%llx func_addr0x%llx func_argc0x%llx\n",
	    __func__, func_retl, func_addr, func_argc);

	for (i = 0; i < func_argc; i++)
		cvmx_dprintf("        argv[%d] = 0x%llx\n", i, *(p + i));
}
#endif

	pcmd->otc_ret = OTR_SUCCESS;
	if (func_retl > 64 || func_argc > OCTEON_TRACE_CMD_FUNCALL_MAX_ARGC) {
		pcmd->otc_ret = OTR_FAIL;
		pcmd->otc_rval[0] = OTE_UNSUPPORTED;
	} else {
		/* treat void the same as 64-bit return */
        	switch(func_argc) {
        	case 0: pcmd->otc_rval[0] = ((uint64_t (*)())(uintptr_t)func_addr)();
        	        break;
        	case 1: pcmd->otc_rval[0] = ((uint64_t (*)(uint64_t))(uintptr_t)func_addr)(*p);
        	        break;
        	case 2: pcmd->otc_rval[0] = ((uint64_t (*)(uint64_t, uint64_t))(uintptr_t)func_addr)(*p, *(p + 1));
        	        break;
        	case 3: pcmd->otc_rval[0] = ((uint64_t (*)(uint64_t, uint64_t, uint64_t))(uintptr_t)func_addr) (*p, *(p + 1), *(p + 2));
        	        break;
        	case 4: pcmd->otc_rval[0] = ((uint64_t (*)(uint64_t, uint64_t, uint64_t, uint64_t))(uintptr_t)func_addr) (*p, *(p + 1), *(p + 2), *(p + 3));
        	        break;
        	case 5: pcmd->otc_rval[0] = ((uint64_t (*)(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t))(uintptr_t)func_addr) (*p, *(p + 1), *(p + 2), *(p + 3), *(p + 4));
        	        break;
        	case 6: pcmd->otc_rval[0] = ((uint64_t (*)(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t))(uintptr_t)func_addr) (*p, *(p + 1), *(p + 2), *(p + 3), *(p + 4), *(p + 5));
        	        break;
        	case 7: pcmd->otc_rval[0] = ((uint64_t (*)(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t))(uintptr_t)func_addr) (*p, *(p + 1), *(p + 2), *(p + 3), *(p + 4), *(p + 5), *(p + 6));
        	        break;
		/* The max case here is OCTEON_TRACE_CMD_FUNCALL_MAX_ARGC. */
		default:
			pcmd->otc_ret = OTR_FAIL;
			break;
        	}
	}
}

static otrace_call_t otrace_jt[OTC_CMD_MAX + 1] = {
	NULL,
	otc_call,
	NULL
};

static void otrace_stub_handler(
	struct cvmx_mbox *mbox, uint64_t registers[32])
{
	otrace_cmd_buf_t *pcmd;
	int core;

	core = cvmx_get_core_num();
	if (!otc_buf) {
		otc_buf = (otrace_cmd_buf_t *)
		    otrace_find_buf(OCTEON_TRACE_CMD_BUF_NAME);
		if (!otc_buf) {
			cvmx_dprintf("%s: no otc buffer for core %d\n",
			    __func__, core);
			return;
		}
	}
	pcmd = otc_buf + core;

	if (otrace_jt[pcmd->otc_cmd]) {
		otrace_jt[pcmd->otc_cmd](pcmd);
	} else {
		extern void __cvmx_interrupt_dump_registers(uint64_t *registers);
		__cvmx_interrupt_dump_registers(registers);
		__octeon_print_backtrace_func(
		    (__octeon_backtrace_printf_t) cvmx_safe_printf);

		pcmd->otc_ret = OTR_FAIL;
		pcmd->otc_rval[0] = OTE_UNKNOWN_CMD;
	}

	CVMX_SYNCW; /* for *pcmd */
}

static CVMX_SHARED struct cvmx_mbox  otrace_stub_mbox = {
	.handler = otrace_stub_handler
};

void cvmx_otrace_stub_init(cvmx_sysinfo_t *sys_info_ptr)
{
	cvmx_mbox_initialize(1ull << CVMX_MBOX_BIT_OTRACE);
        if (cvmx_is_init_core()) {
		cvmx_mbox_register(CVMX_MBOX_BIT_OTRACE, &otrace_stub_mbox);
	}
}

/* otrace stub }}} */
/**
 * otrace runtime tracing {{{
 */

/* #define OTRACE_DEBUG */
static void otrace_log_enqueue(otrace_log_type_t ltype, uint64_t thing1,
    uint64_t thing2) __attribute__((no_instrument_function));
static void otrace_log_enqueue(otrace_log_type_t ltype, uint64_t thing1,
    uint64_t thing2)
{
	int h, t;
	otrace_log_entry_t *p;

	h = otl_queue->otl_head;
	t = otl_queue->otl_tail;
	p = &otl_queue->otl_buf[h];

	if (p->otl_ent_type < OTL_ENT_VAR) /* take a timestamp only for some events */
		p->otl_ent_ts = cvmx_get_cycle_noinst();

	if (t == OCTEON_TRACE_LOG_NEXT(h) || 
	    t == OCTEON_TRACE_LOG_NEXT(h + 1)) {
		if (p->otl_ent_type == OTL_ENT_INVALID) {
			p->otl_ent_type = OTL_ENT_BUF_FULL;
			p->otl_ent_ts0 = p->otl_ent_ts;
			p->otl_ent_count = 0;
		}

		if (p->otl_ent_type != OTL_ENT_BUF_FULL)
			cvmx_dprintf("%s: this entry should be full\n",
			    __func__);

		p->otl_ent_count++;
	} else {
		if (p->otl_ent_type == OTL_ENT_BUF_FULL) {
			/* skip the full entry */
#ifdef OTRACE_DEBUG
			cvmx_dprintf("skip full entry %d\n", h);
#endif
			h = OCTEON_TRACE_LOG_NEXT(h);
			p = &otl_queue->otl_buf[h];
			p->otl_ent_ts = cvmx_get_cycle_noinst();
		}

		p->otl_ent_type = ltype;
		p->otl_ent_thing1 = thing1;
		p->otl_ent_thing2 = thing2;

		/* advance to the next entry */
		h = OCTEON_TRACE_LOG_NEXT(h);
		p = &otl_queue->otl_buf[h];
		p->otl_ent_type = OTL_ENT_INVALID;

		otl_queue->otl_head = h;
	}

	CVMX_SYNCW;
}

#define OTRACE_PROBE_MAX		8	/* max active probes */
#define OTRACE_PROBE_VAR_MAX		8	/* max # of vars per probe */

#define OTRACE_PROBE_CYG_FENTRY	1ULL
#define OTRACE_PROBE_CYG_FEXIT	2ULL
#define OTRACE_PROBE_ALL	3ULL	/* everything */
typedef struct {
	uint64_t otrace_probe_addr;	/* the address */
	uint64_t otrace_probe_type;	/* the probe type */
	uint64_t otrace_probe_nvars;	/* >0 means we need to report the variables
					   in otrace_probe_vars[] */
} otrace_probe_t;

typedef struct {
	otrace_log_type_t otrace_var_ltype;
	union {
		struct {
			uint64_t addr;
			uint64_t len;
		} otrace_var;
		struct {
			uint64_t reg;
			uint64_t sel;
		} otrace_var_cop0;
		struct {
			uint64_t thing1;
			uint64_t thing2;
		} otrace_var_generic;
	} u;
#define otrace_var_addr	u.otrace_var.addr
#define otrace_var_len	u.otrace_var.len
#define otrace_var_cop0_reg	u.otrace_var_cop0.reg
#define otrace_var_cop0_sel	u.otrace_var_cop0.sel
#define otrace_var_s	u.otrace_var_generic
} otrace_probe_var_t;

static otrace_probe_t otrace_probe[OTRACE_PROBE_MAX];
static otrace_probe_var_t otrace_probe_vars[OTRACE_PROBE_MAX][OTRACE_PROBE_VAR_MAX];

static int otrace_probe_get(uint64_t probe_addr, uint64_t probe_type) __attribute__((no_instrument_function));
static int otrace_probe_get(uint64_t probe_addr, uint64_t probe_type)
{
	int i;
	otrace_probe_t *p;

	for (i = 0; i < OTRACE_PROBE_MAX; i++) {
		p = &otrace_probe[i];
		if (p->otrace_probe_addr == probe_addr &&
		    p->otrace_probe_type == probe_type)
			break;
	}

	if (i == OTRACE_PROBE_MAX)
		return -1;

	return i;
}

static int otrace_probe_alloc(uint64_t probe_addr, uint64_t probe_type)
{
	int i;

	i = otrace_probe_get(probe_addr, probe_type);
	if (i == -1) {
		for (i = 0; i < OTRACE_PROBE_MAX; i++)
			if (otrace_probe[i].otrace_probe_addr == 0)
				break;

		if (i < OTRACE_PROBE_MAX)
			return i; /* a free slot */
	} else
		return i; /* an existing probe */

	return -1;
}

static void otrace_log_vars(int probe_index)  __attribute__((no_instrument_function));
static void otrace_log_vars(int probe_index)
{
	/*
	 * log all the traced variables for this probe
	 */
	int i;
	otrace_probe_var_t *v;
	uint64_t val;

	for (i = 0; i < OTRACE_PROBE_VAR_MAX; i++) {
		v = &otrace_probe_vars[probe_index][i];
		switch (v->otrace_var_ltype) {
		case OTL_ENT_VAR:
			switch (v->otrace_var_len) {
			case 1:
				val = (uint64_t)(*((uint8_t *)(uintptr_t)v->otrace_var_addr));
				break;
			case 2:
				val = (uint64_t)(*((uint16_t *)(uintptr_t)v->otrace_var_addr));
				break;
			case 4:
				val = (uint64_t)(*((uint32_t *)(uintptr_t)v->otrace_var_addr));
				break;
			case 8:
				val = *((uint64_t *)(uintptr_t)v->otrace_var_addr);
				break;
			default:
				cvmx_dprintf( "%s: invalid var size%d @ %p"
				    " (probe index%d var index%d), ignored\n",
				    __func__, (int)v->otrace_var_len,
				    (void *)(uintptr_t)v->otrace_var_addr, probe_index, i);
				continue;
			}

			otrace_log_enqueue(OTL_ENT_VAR, 
			    (uint64_t)v->otrace_var_addr, val);
			break;

		default:
			break;
		}
	}
}

void __cyg_profile_func_enter(void *this_fn, void *call_site) __attribute__((no_instrument_function));
void __cyg_profile_func_enter(void *this_fn, void *call_site)
{
	int i;

	if (!OCTEON_IS_TRACED())
		return;

	i = otrace_probe_get((uint64_t)(uintptr_t)this_fn, OTRACE_PROBE_CYG_FENTRY);
	if (i >= 0) {
		otrace_log_enqueue(OTL_ENT_FUNC_ENTER, (uint64_t)(uintptr_t)this_fn,
		    (uint64_t)(uintptr_t)call_site);
		if (otrace_probe[i].otrace_probe_nvars)
			otrace_log_vars(i);
	}

#if 0
	/* log the variables, if any */
	nvars = otrace_probe[i].otrace_probe_nvars_cyg_entry;
#endif
	return;
}

void __cyg_profile_func_exit(void *this_fn, void *call_site) __attribute__((no_instrument_function));
void __cyg_profile_func_exit(void *this_fn, void *call_site)
{
	int i;

	if (!OCTEON_IS_TRACED())
		return;

	i = otrace_probe_get((uint64_t)(uintptr_t)this_fn, OTRACE_PROBE_CYG_FEXIT);
	if (i >= 0) {
		otrace_log_enqueue(OTL_ENT_FUNC_EXIT, (uint64_t)(uintptr_t)this_fn,
		    (uint64_t)(uintptr_t)call_site);
		if (otrace_probe[i].otrace_probe_nvars)
			otrace_log_vars(i);
	}

	return;
}

static uint64_t otc_call_probe_set(uint64_t probe_addr, uint64_t probe_type)
{
	int i;

	i = otrace_probe_alloc(probe_addr, probe_type);
#if 0
	cvmx_dprintf("%s otrace_alloc_probe(0x%llx) returns %d\n",
	    __func__, probe_addr, i);
#endif
	if (i >= 0) {
		otrace_probe_t *p;

		p = &otrace_probe[i];

		if (p->otrace_probe_addr)
			return 0;

#if 0
		cvmx_dprintf("%s: probe_addr0x%llx probe_addr(orig)0x%llx probe_flags0x%llx probe_flags(orig)0x%llx",
		    __func__, probe_addr, p->otrace_probe_addr, probe_flags, p->otrace_probe_flags);
#endif

		p->otrace_probe_addr = probe_addr;
		p->otrace_probe_type = probe_type;
#if 0
		cvmx_dprintf(" probe_flags(result)0x%llx\n", p->otrace_probe_flags);
#endif
		return 0;
	}

	return (uint64_t)-1;
}

static uint64_t otc_call_probe_clr(uint64_t probe_addr, uint64_t probe_type)
{
	int i;

	i = otrace_probe_get(probe_addr, probe_type);
	if (i >= 0) {
		otrace_probe_t *p;

		p = &otrace_probe[i];
		*p = (otrace_probe_t) {0, 0, 0};

		return 0;
	}

	return (uint64_t)-1;
}

/*
 * otc_call_probe_set_var() must be called after a successful otc_call_probe_set()
 */
static uint64_t otc_call_probe_set_var(uint64_t probe_addr, uint64_t probe_type,
    otrace_log_type_t var_ltype, uint64_t arg1, uint64_t arg2)
{
	int i, j;

	i = otrace_probe_get(probe_addr, probe_type);
	if (i >= 0) {
		otrace_probe_t *p;
		otrace_probe_var_t *v;

		p = &otrace_probe[i];
		if (p->otrace_probe_nvars == OTRACE_PROBE_VAR_MAX)
			return (uint64_t)-2;

		for (j = 0; j < OTRACE_PROBE_VAR_MAX; j++) {
			v = &otrace_probe_vars[i][j];
			if (v->otrace_var_ltype == var_ltype &&
			    v->otrace_var_s.thing2 == arg2 &&
			    v->otrace_var_s.thing1 == arg1)
				return 0;
		}

		p->otrace_probe_nvars++;
		for (j = 0; j < OTRACE_PROBE_VAR_MAX; j++) {
			v = &otrace_probe_vars[i][j];
			if (!v->otrace_var_ltype) {
				v->otrace_var_ltype = var_ltype;
				v->otrace_var_s.thing1 = arg1;
				v->otrace_var_s.thing2 = arg2;
				break;
			}
		}

#if 0
		cvmx_dprintf("%s: inserted var_ltype%d arg1 0x%llx arg2 0x%llx at probe%p (%d)\n", 
		    __func__, var_ltype, (unsigned long long) arg1, (unsigned long long) arg2,
		    (void *)probe_addr, (int)probe_type);
#endif
		    
	} else
		return (uint64_t)-1;

	return 0;
}

static uint64_t otc_call_probe_clr_var(uint64_t probe_addr, uint64_t probe_type, 
    otrace_log_type_t var_ltype, uint64_t arg1, uint64_t arg2)
{
	int i, j;

	i = otrace_probe_get(probe_addr, probe_type);
	if (i >= 0) {
		otrace_probe_t *p;
		otrace_probe_var_t *v;

		p = &otrace_probe[i];
		if (p->otrace_probe_nvars == 0)
			return (uint64_t)-1;
		
		for (j = 0; j < OTRACE_PROBE_VAR_MAX; j++) {
			v = &otrace_probe_vars[i][j];
			if (v->otrace_var_ltype == var_ltype &&
			    v->otrace_var_s.thing1 == arg1 &&
			    v->otrace_var_s.thing2 == arg2) {

				p->otrace_probe_nvars--;
				v->otrace_var_ltype = OTL_ENT_INVALID;
				v->otrace_var_s.thing1 = 0;
				v->otrace_var_s.thing2 = 0;
				break;
			}
		}
		if (j == OTRACE_PROBE_VAR_MAX)
			return (uint64_t) -1;

	} else 
		return (uint64_t)-1;

	return 0;
}

static uint64_t otc_call_probe_show(uint64_t probe_addr, uint64_t probe_type)
{
	int i, j;
	char *ptstr;
	
	ptstr = "invalid_probe_type";
	if (probe_type == OTRACE_PROBE_CYG_FENTRY) ptstr = "function_entry";
	if (probe_type == OTRACE_PROBE_CYG_FEXIT) ptstr = "function_exit";

	cvmx_dprintf("probe addr %p type %s: ", (void *)(uintptr_t)probe_addr, ptstr);

	i = otrace_probe_get(probe_addr, probe_type);
	if (i >= 0) {
		cvmx_dprintf("found at %d ", i);

		otrace_probe_t *p;
		otrace_probe_var_t *v;

		p = &otrace_probe[i];
		cvmx_dprintf("%d vars ", (int)p->otrace_probe_nvars);
		if (p->otrace_probe_nvars != 0) {
			for (j = 0; j < OTRACE_PROBE_VAR_MAX; j++) {
				v = &otrace_probe_vars[i][j];
				if (v->otrace_var_ltype) {
					cvmx_dprintf(
					    "\n    var_ltype%d (elem%d) arg1=0x%llx arg2=0x%llx",
					    v->otrace_var_ltype, j,
					    (unsigned long long)v->otrace_var_s.thing1,
					    (unsigned long long)v->otrace_var_s.thing2);
				}
			}
		}
	} else
		cvmx_dprintf("not found");

	cvmx_dprintf("\n");
	return 0;
}

/* otrace runtime tracing }}} */

void *otc_call_table[] = {
	(void *)otc_call_get_cop0,
	(void *)otc_call_get_var,
	(void *)otc_call_set_var,
	(void *)otc_call_probe_show,
	(void *)otc_call_probe_clr,
	(void *)otc_call_probe_set,
	(void *)otc_call_probe_clr_var,
	(void *)otc_call_probe_set_var,
	(void *)otc_call_trace
};

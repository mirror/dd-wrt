/***********************license start***************
 * Copyright (c) 2003-2014  Cavium Inc. (support@cavium.com). All rights
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
 * <hr>$Revision: 0 $<hr>
 */

#ifndef __CVMX_PKO3_H__
#define __CVMX_PKO3_H__

#ifdef	__cplusplus
/* *INDENT-OFF* */
extern "C" {
/* *INDENT-ON* */
#endif

#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
#include <asm/octeon/cvmx-pko-defs.h>
#include <asm/octeon/cvmx-helper.h>
#include <asm/octeon/cvmx-pko3-queue.h>
#include <asm/octeon/cvmx-ilk.h>
#include <asm/octeon/cvmx-scratch.h>
#include <asm/octeon/cvmx-atomic.h>
#include <asm/octeon/cvmx-clock.h>
#include <asm/octeon/cvmx-pow.h>
#else
#include "cvmx-pko-defs.h"
#include "cvmx-pko3-queue.h"
#include "cvmx-helper.h"
#include "cvmx-ilk.h"
#include "cvmx-scratch.h"
#include "cvmx-atomic.h"
#include "cvmx-pow.h"
#endif

/* Use full LMTDMA when PARAMETER_CHECKINS is enabled */
#undef	CVMX_ENABLE_PARAMETER_CHECKING
#define	CVMX_ENABLE_PARAMETER_CHECKING 0

/* dwords count from 1-16 */
/* scratch line for LMT operations */
/* Should be unique wrt other uses of CVMSEG, e.g. IOBDMA */
#define CVMX_PKO_LMTLINE 2ull

/* PKO3 queue level identifier */
enum cvmx_pko3_level_e {
	CVMX_PKO_LEVEL_INVAL = 0,
	CVMX_PKO_PORT_QUEUES = 0xd1,
	CVMX_PKO_L2_QUEUES = 0xc2,
	CVMX_PKO_L3_QUEUES = 0xb3,
	CVMX_PKO_L4_QUEUES = 0xa4,
	CVMX_PKO_L5_QUEUES = 0x95,
	CVMX_PKO_DESCR_QUEUES = 0x86,
};


enum cvmx_pko_dqop {
	CVMX_PKO_DQ_SEND = 0ULL,
	CVMX_PKO_DQ_OPEN = 1ULL,
	CVMX_PKO_DQ_CLOSE = 2ULL,
	CVMX_PKO_DQ_QUERY = 3ULL
};

/**
 * Returns the PKO DQ..L2 Shaper Time-Wheel clock rate for specified node.
 */
static inline
uint64_t cvmx_pko3_dq_tw_clock_rate_node(int node)
{
	return cvmx_clock_get_rate_node(node, CVMX_CLOCK_SCLK) / 768;
}

/**
 * Returns the PKO Port Shaper Time-Wheel clock rate for specified node.
 */
static inline
uint64_t cvmx_pko3_pq_tw_clock_rate_node(int node)
{
	int div;

	if (OCTEON_IS_MODEL(OCTEON_CN78XX))
		div = 96;
	else
		div = 48;
	return cvmx_clock_get_rate_node(node, CVMX_CLOCK_SCLK) / div;
}

/**
 * @INTERNAL
 * Return the number of MACs in the PKO (exclusing the NULL MAC)
 * in a model-dependent manner.
 */
static inline unsigned __cvmx_pko3_num_macs(void)
{
        if(OCTEON_IS_MODEL(OCTEON_CNF75XX))
		return 10;
        if(OCTEON_IS_MODEL(OCTEON_CN73XX))
		return 14;
        if(OCTEON_IS_MODEL(OCTEON_CN78XX))
		return 28;
	return 0;
}

/**
 * @INTERNAL
 * Return the number of queue levels, depending on SoC model
 */
static inline int __cvmx_pko3_sq_lvl_max(void)
{
        if (OCTEON_IS_MODEL(OCTEON_CN73XX))
		return CVMX_PKO_L3_QUEUES;
        if (OCTEON_IS_MODEL(OCTEON_CNF75XX))
		return CVMX_PKO_L3_QUEUES;
        if (OCTEON_IS_MODEL(OCTEON_CN78XX))
		return CVMX_PKO_L5_QUEUES;
	return -1;
}

/**
 * @INTERNAL
 * Return the next (lower) queue level for a given level
 */
static inline enum cvmx_pko3_level_e
__cvmx_pko3_sq_lvl_next(enum cvmx_pko3_level_e level)
{
	switch(level) {
		default:
			return CVMX_PKO_LEVEL_INVAL;
		case CVMX_PKO_PORT_QUEUES:
			return CVMX_PKO_L2_QUEUES;
		case CVMX_PKO_L2_QUEUES:
			return CVMX_PKO_L3_QUEUES;
		case CVMX_PKO_L3_QUEUES:
			if (OCTEON_IS_MODEL(OCTEON_CN73XX) ||
			    OCTEON_IS_MODEL(OCTEON_CNF75XX))
				return CVMX_PKO_DESCR_QUEUES;
			return CVMX_PKO_L4_QUEUES;
		case CVMX_PKO_L4_QUEUES:
			if (OCTEON_IS_MODEL(OCTEON_CN73XX) ||
			    OCTEON_IS_MODEL(OCTEON_CNF75XX))
				return CVMX_PKO_LEVEL_INVAL;
			return CVMX_PKO_L5_QUEUES;
		case CVMX_PKO_L5_QUEUES:
			if (OCTEON_IS_MODEL(OCTEON_CN73XX) ||
			    OCTEON_IS_MODEL(OCTEON_CNF75XX))
				return CVMX_PKO_LEVEL_INVAL;
			return CVMX_PKO_DESCR_QUEUES;
	}
}

/**
 * @INTERNAL
 * Return an SQ identifier string, for debug messages.
 */
static inline char *
__cvmx_pko3_sq_str(char * buf, enum cvmx_pko3_level_e level, unsigned q)
{
	char *p;

	switch(level) {
		default:
			strcpy(buf, "ERR-SQ/");
			break;
		case CVMX_PKO_PORT_QUEUES:
			strcpy(buf, "PQ_L1/");
			break;
		case CVMX_PKO_L2_QUEUES:
			strcpy(buf, "SQ_L2/");
			break;
		case CVMX_PKO_L3_QUEUES:
			strcpy(buf, "SQ_L3/");
			break;
		case CVMX_PKO_L4_QUEUES:
			strcpy(buf, "SQ_L4/");
			break;
		case CVMX_PKO_L5_QUEUES:
			strcpy(buf, "SQ_L5/");
			break;
		case CVMX_PKO_DESCR_QUEUES:
			strcpy(buf, "DQ/");
			break;
	}

	for(p=buf; *p; p++)
		;
	*p++ = '0' + q / 1000;
	q -= (q / 1000) * 1000;
	*p++ = '0' + q / 100;
	q -= (q / 100) * 100;
	*p++ = '0' + q / 10;
	q -= (q / 10) * 10;
	*p++ = '0' + q;
	*p++ = ':';
	*p++ = '\0';
	return buf;
}


union cvmx_pko_query_rtn {
	uint64_t u64;
	struct {
		CVMX_BITFIELD_FIELD(uint64_t dqstatus	: 4,
		CVMX_BITFIELD_FIELD(uint64_t rsvd_50_59	:10,
		CVMX_BITFIELD_FIELD(uint64_t dqop	: 2,
		CVMX_BITFIELD_FIELD(uint64_t depth	:48,
			))));
	} s;
};
typedef union cvmx_pko_query_rtn cvmx_pko_query_rtn_t;

/* PKO_QUERY_RTN_S[DQSTATUS] - cvmx_pko_query_rtn_t->s.dqstatus */
enum pko_query_dqstatus {
	PKO_DQSTATUS_PASS = 0,		/* No error */
	PKO_DQSTATUS_BADSTATE = 0x8,	/* queue was not ready to enqueue */
	PKO_DQSTATUS_NOFPABUF = 0x9,	/* FPA out of buffers */
	PKO_DQSTATUS_NOPKOBUF = 0xA,	/* PKO out of buffers */
	PKO_DQSTATUS_FAILRTNPTR = 0xB,	/* can't return buffer ptr to FPA */
	PKO_DQSTATUS_ALREADY = 0xC,	/* already created */
	PKO_DQSTATUS_NOTCREATED = 0xD,	/* not created */
	PKO_DQSTATUS_NOTEMPTY = 0xE,	/* queue not empty */
	PKO_DQSTATUS_SENDPKTDROP = 0xF	/* packet dropped, illegal construct */
};
typedef	enum pko_query_dqstatus pko_query_dqstatus_t;

/* Sub-command three bit codes (SUBDC3) */
#define CVMX_PKO_SENDSUBDC_LINK		0x0
#define CVMX_PKO_SENDSUBDC_GATHER	0x1
#define CVMX_PKO_SENDSUBDC_JUMP		0x2
/* Sub-command four bit codes (SUBDC4) */
#define CVMX_PKO_SENDSUBDC_TSO		0x8
#define CVMX_PKO_SENDSUBDC_FREE		0x9
#define CVMX_PKO_SENDSUBDC_WORK		0xA
#define CVMX_PKO_SENDSUBDC_AURA		0xB
#define CVMX_PKO_SENDSUBDC_MEM		0xC
#define CVMX_PKO_SENDSUBDC_EXT		0xD
#define CVMX_PKO_SENDSUBDC_CRC		0xE
#define CVMX_PKO_SENDSUBDC_IMM		0xF

/**
 * pko buf ptr
 * This is good for LINK_S, GATHER_S and PKI_BUFLINK_S structure use.
 * It can also be used for JUMP_S with F-bit represented by "i" field,
 * and the size limited to 8-bit.
*/

union cvmx_pko_buf_ptr {
	uint64_t u64;
	struct {
		CVMX_BITFIELD_FIELD(uint64_t size	:16,
		CVMX_BITFIELD_FIELD(uint64_t subdc3	: 3,
		CVMX_BITFIELD_FIELD(uint64_t i		: 1,
		CVMX_BITFIELD_FIELD(uint64_t rsvd_42_43	: 2,
		CVMX_BITFIELD_FIELD(uint64_t addr	:42,
			)))));
	} s;
};
typedef union cvmx_pko_buf_ptr cvmx_pko_buf_ptr_t;

/**
 * pko_auraalg_e
 */
enum pko_auraalg_e {
	AURAALG_NOP = 0x0,	/* aura_cnt = No change */
	AURAALG_SUB = 0x3,	/* aura_cnt -= pko_send_aura_t.offset */
	AURAALG_SUBLEN = 0x7,	/* aura_cnt -= pko_send_aura_t.offset +
						pko_send_hdr_t.total_bytes */
	AURAALG_SUBMBUF = 0xB	/* aura_cnt -= pko_send_aura_t.offset +
						mbufs_freed */
};

/**
 * PKO_CKL4ALG_E
 */
enum pko_clk4alg_e {
	CKL4ALG_NONE = 0x0,	/* No checksum. */
	CKL4ALG_UDP = 0x1,	/* UDP L4 checksum. */
	CKL4ALG_TCP = 0x2,	/* TCP L4 checksum. */
	CKL4ALG_SCTP = 0x3,	/* SCTP L4 checksum. */
};

/**
 * pko_send_aura
 */
union cvmx_pko_send_aura {
	uint64_t u64;
	struct {
                CVMX_BITFIELD_FIELD(uint64_t rsvd_60_63 : 4,
                CVMX_BITFIELD_FIELD(uint64_t aura 	: 12, /* NODE+LAURA */
                CVMX_BITFIELD_FIELD(uint64_t subdc4 	: 4,
                CVMX_BITFIELD_FIELD(uint64_t alg 	: 4, /* pko_auraalg_e */
                CVMX_BITFIELD_FIELD(uint64_t rsvd_08_39 : 32,
                CVMX_BITFIELD_FIELD(uint64_t offset 	: 8,
			))))));
	} s;
};
typedef union cvmx_pko_send_aura cvmx_pko_send_aura_t;

/**
 * pko_send_tso
 */
union cvmx_pko_send_tso {
	uint64_t u64;
	struct {
		CVMX_BITFIELD_FIELD(uint64_t l2len	: 8,
		CVMX_BITFIELD_FIELD(uint64_t rsvd_48_55	: 8,
		CVMX_BITFIELD_FIELD(uint64_t subdc4	: 4, /* 0x8 */
		CVMX_BITFIELD_FIELD(uint64_t rsvd_32_43	: 12,
		CVMX_BITFIELD_FIELD(uint64_t sb		: 8,
		CVMX_BITFIELD_FIELD(uint64_t mss	: 16,
		CVMX_BITFIELD_FIELD(uint64_t eom	: 1,
		CVMX_BITFIELD_FIELD(uint64_t fn		: 7,
			))))))));
	} s;
};
typedef union cvmx_pko_send_tso cvmx_pko_send_tso_t;

/**
 * pko_send_free
 */
union cvmx_pko_send_free {
	uint64_t u64;
	struct {
                CVMX_BITFIELD_FIELD(uint64_t rsvd_48_63 : 16,
                CVMX_BITFIELD_FIELD(uint64_t subdc4	: 4, /* 0x9 */
                CVMX_BITFIELD_FIELD(uint64_t rsvd	: 2,
                CVMX_BITFIELD_FIELD(uint64_t addr	: 42,
			))));
	} s;
};
typedef union cvmx_pko_send_free cvmx_pko_send_free_t;

/* PKO_SEND_HDR_S - PKO header subcommand */
union cvmx_pko_send_hdr {
	uint64_t u64;
	struct {
		CVMX_BITFIELD_FIELD(uint64_t rsvd_60_63	:4,
		CVMX_BITFIELD_FIELD(uint64_t aura	:12,
		CVMX_BITFIELD_FIELD(uint64_t ckl4	:2, /* PKO_CKL4ALG_E */
		CVMX_BITFIELD_FIELD(uint64_t ckl3	:1,
		CVMX_BITFIELD_FIELD(uint64_t ds		:1,
		CVMX_BITFIELD_FIELD(uint64_t le		:1,
		CVMX_BITFIELD_FIELD(uint64_t n2		:1,
		CVMX_BITFIELD_FIELD(uint64_t ii		:1,
		CVMX_BITFIELD_FIELD(uint64_t df		:1,
		CVMX_BITFIELD_FIELD(uint64_t rsvd_39	:1,
		CVMX_BITFIELD_FIELD(uint64_t format	:7,
		CVMX_BITFIELD_FIELD(uint64_t l4ptr	:8,
		CVMX_BITFIELD_FIELD(uint64_t l3ptr	:8,
		CVMX_BITFIELD_FIELD(uint64_t total	:16,
			))))))))))))));
	} s;
};
typedef union cvmx_pko_send_hdr cvmx_pko_send_hdr_t;

/* PKO_SEND_EXT_S - extended header subcommand */
union cvmx_pko_send_ext {
	uint64_t u64;
	struct {
		CVMX_BITFIELD_FIELD(uint64_t rsvd_48_63	:16,
		CVMX_BITFIELD_FIELD(uint64_t subdc4	:4, /* _SENDSUBDC_EXT */
		CVMX_BITFIELD_FIELD(uint64_t col	:2, /* _COLORALG_E */
		CVMX_BITFIELD_FIELD(uint64_t ra		:2, /* _REDALG_E */
		CVMX_BITFIELD_FIELD(uint64_t tstmp	:1,
		CVMX_BITFIELD_FIELD(uint64_t rsvd_24_38:15,
		CVMX_BITFIELD_FIELD(uint64_t markptr	:8,
		CVMX_BITFIELD_FIELD(uint64_t rsvd_9_15	:7,
		CVMX_BITFIELD_FIELD(uint64_t shapechg	:9,
			)))))))));
	} s;
};
typedef union cvmx_pko_send_ext cvmx_pko_send_ext_t;

/* PKO_MEMDSZ_E */
enum cvmx_pko_memdsz_e {
	MEMDSZ_B64 = 0,
	MEMDSZ_B32 = 1,
	MEMDSZ_B16 = 2,		/* Not in HRM, assumed unsupported */
	MEMDSZ_B8 = 3
};

/* PKO_MEMALG_E */
enum cvmx_pko_memalg_e {
	MEMALG_SET = 0,		/* Set mem = PKO_SEND_MEM_S[OFFSET] */
	MEMALG_SETTSTMP = 1,	/* Set the memory location to the timestamp
				   PKO_SEND_MEM_S[DSZ] must be B64 and a
				   PKO_SEND_EXT_S subdescriptor must be in
				   the descriptor with PKO_SEND_EXT_S[TSTMP]=1
				 */
	MEMALG_SETRSLT = 2,	/* [DSZ] = B64; mem = PKO_MEM_RESULT_S.  */
	MEMALG_ADD = 8,		/* mem = mem + PKO_SEND_MEM_S[OFFSET] */
	MEMALG_SUB = 9,		/* mem = mem – PKO_SEND_MEM_S[OFFSET] */
	MEMALG_ADDLEN = 0xA,	/* mem += [OFFSET] + PKO_SEND_HDR_S[TOTAL] */
	MEMALG_SUBLEN = 0xB,	/* mem -= [OFFSET] + PKO_SEND_HDR_S[TOTAL] */
	MEMALG_ADDMBUF = 0xC,	/* mem += [OFFSET] + mbufs_freed */
	MEMALG_SUBMBUF = 0xD	/* mem -= [OFFSET] + mbufs_freed */
};

union cvmx_pko_send_mem {
	uint64_t u64;
	struct {
		CVMX_BITFIELD_FIELD(uint64_t rsvd_63	:1,
		CVMX_BITFIELD_FIELD(uint64_t wmem	:1,
		CVMX_BITFIELD_FIELD(uint64_t dsz	:2, /* PKO_MEMDSZ_E */
		CVMX_BITFIELD_FIELD(uint64_t alg	:4, /* PKO_MEMALG_E */
		CVMX_BITFIELD_FIELD(uint64_t offset	:8,
		CVMX_BITFIELD_FIELD(uint64_t subdc4	:4,
		CVMX_BITFIELD_FIELD(uint64_t rsvd_42_43	:2,
		CVMX_BITFIELD_FIELD(uint64_t addr	:42,
		))))))));
	} s;
};

typedef union cvmx_pko_send_mem cvmx_pko_send_mem_t;

union cvmx_pko_send_work {
	uint64_t u64;
	struct {
		CVMX_BITFIELD_FIELD(uint64_t rsvd_62_63	:2,
		CVMX_BITFIELD_FIELD(uint64_t grp	:10,
		CVMX_BITFIELD_FIELD(uint64_t tt		:2,
		CVMX_BITFIELD_FIELD(uint64_t rsvd_48_49	:2,
		CVMX_BITFIELD_FIELD(uint64_t subdc4	:4,
		CVMX_BITFIELD_FIELD(uint64_t rsvd_42_43	:2,
		CVMX_BITFIELD_FIELD(uint64_t addr	:42,
			)))))));
	} s;
};

typedef union cvmx_pko_send_work cvmx_pko_send_work_t;

/*** PKO_SEND_DMA_S - format of IOBDMA/LMTDMA data word ***/
union cvmx_pko_lmtdma_data {
	uint64_t u64;
	struct {
		CVMX_BITFIELD_FIELD(uint64_t scraddr	: 8,
		CVMX_BITFIELD_FIELD(uint64_t rtnlen	: 8,
		CVMX_BITFIELD_FIELD(uint64_t did	: 8, /* 0x51 */
		CVMX_BITFIELD_FIELD(uint64_t node	: 4,
		CVMX_BITFIELD_FIELD(uint64_t rsvd_34_35	: 2,
		CVMX_BITFIELD_FIELD(uint64_t dqop	: 2, /* PKO_DQOP_E */
		CVMX_BITFIELD_FIELD(uint64_t rsvd_26_31	: 6,
		CVMX_BITFIELD_FIELD(uint64_t dq		: 10,
		CVMX_BITFIELD_FIELD(uint64_t rsvd_0_15	: 16,
			)))))))));
	} s;
};
typedef union cvmx_pko_lmtdma_data cvmx_pko_lmtdma_data_t;

typedef struct cvmx_pko3_dq_params_s {
#ifdef __LITTLE_ENDIAN_BITFIELD
	int32_t limit;
	int32_t	depth;
#else
	int32_t	depth;
	int32_t limit;
#endif
	uint64_t pad[15];
} cvmx_pko3_dq_params_t;

/* DQ depth cached value */
extern CVMX_SHARED
cvmx_pko3_dq_params_t  *__cvmx_pko3_dq_params[CVMX_MAX_NODES];


extern int cvmx_pko3_internal_buffer_count(unsigned node);

/**
 * @INTERNAL
 * PKO3 DQ parameter location
 * @param node      node
 * @param dq        dq
 */
static inline cvmx_pko3_dq_params_t *
cvmx_pko3_dq_parameters(unsigned node, unsigned dq)
{
	cvmx_pko3_dq_params_t *pParam = NULL;
	static cvmx_pko3_dq_params_t dummy;
	dummy.depth = 0;
	dummy.limit = (1<<16);

	if (cvmx_likely(node < CVMX_MAX_NODES))
		pParam = __cvmx_pko3_dq_params[node];

	if (cvmx_likely(pParam != NULL))
		pParam += dq;
	else
		pParam = &dummy;

	return pParam;
}

static inline void cvmx_pko3_dq_set_limit(unsigned node, unsigned dq,
	unsigned limit)
{
	cvmx_pko3_dq_params_t *pParam;
	pParam = cvmx_pko3_dq_parameters(node, dq);
	pParam->limit = limit;
}

/**
 * PKO descriptor queue operation error string
 *
 * @param dqstatus is the enumeration returned from hardware,
 * 	  PKO_QUERY_RTN_S[DQSTATUS].
 *
 * @return static constant string error description
 */
const char * pko_dqstatus_error(pko_query_dqstatus_t dqstatus);


/*
 * This function gets PKO mac num for a interface/port.
 *
 * @param interface is the interface number.
 * @param index is the port number.
 * @return returns mac number if successful or -1 on failure.
 */
static inline int __cvmx_pko3_get_mac_num(int xiface, int index)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	cvmx_helper_interface_mode_t mode;
	int interface_index;
	int ilk_mac_base = -1, bgx_mac_base = -1, bgx_ports = 4;

	if (OCTEON_IS_MODEL(OCTEON_CN73XX)) {
		bgx_mac_base = 2;
	}

	if (OCTEON_IS_MODEL(OCTEON_CNF75XX)) {
		bgx_mac_base = 2;
	}

        if (OCTEON_IS_MODEL(OCTEON_CN78XX)) {
		ilk_mac_base = 2;
		bgx_mac_base = 4;
	}

	mode = cvmx_helper_interface_get_mode(xiface);
	switch (mode) {
		case CVMX_HELPER_INTERFACE_MODE_LOOP:
			return 0;
		case CVMX_HELPER_INTERFACE_MODE_NPI:
			return 1;
		case CVMX_HELPER_INTERFACE_MODE_ILK:
			if (ilk_mac_base < 0)
				return -1;
			interface_index = (xi.interface - CVMX_ILK_GBL_BASE());
			if (interface_index < 0)
				return -1;
			return (ilk_mac_base + interface_index);
		case CVMX_HELPER_INTERFACE_MODE_SRIO:
			return (4 + 2 * xi.interface + index);
		default:
			if (xi.interface >= CVMX_ILK_GBL_BASE() &&
			    ilk_mac_base >= 0)
				return -1;
			/* All other modes belong to BGX */
			return (bgx_mac_base +
				bgx_ports * xi.interface + index);
	}
}

/**
 * @INTERNAL
 *
 * Get scratch offset for LMTDMA/LMTST data buffer
 *
 */
static inline unsigned
cvmx_pko3_lmtdma_scr_base(void)
{
	return CVMX_PKO_LMTLINE * CVMX_CACHE_LINE_SIZE;
}

/**
 * @INTERNAL
 *
 * Get address for LMTDMA/LMTST data buffer
 *
 */
static inline uint64_t *
cvmx_pko3_cvmseg_addr(void)
{
	const unsigned scr = CVMX_PKO_LMTLINE * CVMX_CACHE_LINE_SIZE;
	return (uint64_t *) (CVMX_SCRATCH_BASE + scr);
}


/**
 * Save scratchpad area
 * @param buf storage buffer for saving previous scratchpad contents.
 *
 * This function should be used whenever the cache line is used
 * from a context that might preempt another context that too uses
 * the same cache line designated for LMTST/LMTDMA and Wide-Atomic
 * operations, such as the hard interrupt context in Linux kernel,
 * that could preempt a user-space application on the same processor
 * core also using the same scratchpad.
 * 'cvmx_lmtline_save()' should be called upon entry into the
 * potentially interrupting context, and 'cvmx_lmtline_restore()' should
 * be called prior to exitting that context.
 */
static inline void cvmx_lmtline_save(uint64_t buf[16])
{
	unsigned i, scr_off = cvmx_pko3_lmtdma_scr_base();
	unsigned sz = CVMX_CACHE_LINE_SIZE/sizeof(uint64_t);

	/* wait LMTDMA to finish (if any) */
	CVMX_SYNCIOBDMA;

	/* Copy LMTLINE to user-provided buffer */
	for (i = 0; i < sz; i++)
		buf[i] = cvmx_scratch_read64(scr_off + i * sizeof(uint64_t));
}

/**
 * Restore scratchpad area
 * @param buf storage buffer containing the previous content of scratchpad.
 */
static inline void cvmx_lmtline_restore(const uint64_t buf[16])
{
	unsigned i, scr_off = cvmx_pko3_lmtdma_scr_base();
	unsigned sz = CVMX_CACHE_LINE_SIZE/sizeof(uint64_t);

	/* wait LMTDMA to finsh (if any) */
	CVMX_SYNCIOBDMA;

	/* restore scratchpad area from buf[] */
	for (i = 0; i < sz; i++)
		cvmx_scratch_write64(scr_off + i * sizeof(uint64_t), buf[i]);
}

/*
 * @INTERNAL
 * Deliver PKO SEND commands via CVMSEG LM and LMTDMA/LMTST.
 * The command should be already stored in the CVMSEG address.
 *
 * @param node is the destination node
 * @param dq is the destination descriptor queue.
 * @param numwords is the number of outgoing words
 * @param tag_wait Wait to finish tag switch just before issueing LMTDMA
 * @return the PKO3 native query result structure.
 *
 * <numwords> must be between 1 and 15 for CVMX_PKO_DQ_SEND command
 *
 * NOTE: Internal use only.
 */
static inline cvmx_pko_query_rtn_t
__cvmx_pko3_lmtdma(uint8_t node, uint16_t dq, unsigned numwords, bool tag_wait)
{
	const enum cvmx_pko_dqop dqop = CVMX_PKO_DQ_SEND;
	cvmx_pko_query_rtn_t pko_status;
	cvmx_pko_lmtdma_data_t pko_send_dma_data;
	uint64_t dma_addr;
	unsigned scr_base = cvmx_pko3_lmtdma_scr_base();
	unsigned scr_off;
	cvmx_pko3_dq_params_t *pParam;

	if (cvmx_unlikely(numwords < 1 || numwords > 15)) {
		cvmx_dprintf("%s: ERROR: Internal error\n",
				__FUNCTION__);
		pko_status.u64 = ~0ull;
		return pko_status;
	}

	pParam = cvmx_pko3_dq_parameters(node, dq);

	pko_status.u64 = 0;
	pko_send_dma_data.u64 = 0;

	/* LMTDMA address offset is (nWords-1) */
	dma_addr = CVMX_LMTDMA_ORDERED_IO_ADDR;
	dma_addr += (numwords - 1) << 3;

	scr_off = scr_base + numwords * sizeof(uint64_t);

	/* Write all-ones into the return area */
	cvmx_scratch_write64(scr_off, ~0ull);

	/* Barrier: make sure all prior writes complete before the following */
	CVMX_SYNCWS;

	/* If cached depth exceeds limit, check the real depth */
	if (cvmx_unlikely(pParam->depth > pParam->limit)) {
		cvmx_pko_dqx_wm_cnt_t wm_cnt;
		wm_cnt.u64 = cvmx_read_csr_node(node,CVMX_PKO_DQX_WM_CNT(dq));
		pParam->depth = pko_status.s.depth = wm_cnt.s.count;

		if (pParam->depth > pParam->limit) {
			pko_status.s.dqop = dqop;
			pko_status.s.dqstatus = PKO_DQSTATUS_NOFPABUF;
			return pko_status;
		}
	} else {
		cvmx_atomic_add32_nosync( &(pParam->depth), 1);
	}

	if (CVMX_ENABLE_PARAMETER_CHECKING) {
		/* Request one return word */
		pko_send_dma_data.s.rtnlen = 1;
	} else {
		/* Do not expect a return word */
		pko_send_dma_data.s.rtnlen = 0;
	}

	/* build store data for DMA */
	pko_send_dma_data.s.scraddr = scr_off >> 3;
	pko_send_dma_data.s.did = 0x51;
	pko_send_dma_data.s.node = node;
	pko_send_dma_data.s.dqop = dqop;
	pko_send_dma_data.s.dq = dq;

	/* Wait to finish tag switch just before issueing LMTDMA */
	if (tag_wait)
		cvmx_pow_tag_sw_wait();

	/* issue PKO DMA */
	cvmx_write64_uint64(dma_addr, pko_send_dma_data.u64);

	if (cvmx_unlikely(pko_send_dma_data.s.rtnlen)) {
		/* Wait for LMTDMA completion */
		CVMX_SYNCIOBDMA;

		/* Retreive real result */
		pko_status.u64 = cvmx_scratch_read64(scr_off);
		pParam->depth = pko_status.s.depth;
	} else {
		/* Fake positive result */
		pko_status.s.dqop = dqop;
		pko_status.s.dqstatus = PKO_DQSTATUS_PASS;
	}

	return pko_status;
}


/*
 * @INTERNAL
 * Sends PKO descriptor commands via CVMSEG LM and LMTDMA.
 * @param node is the destination node
 * @param dq is the destonation descriptor queue.
 * @param cmds[] is an array of 64-bit PKO3 headers/subheaders
 * @param numworkds is the number of outgoing words
 * @param dqop is the operation code
 * @return the PKO3 native query result structure.
 *
 * <numwords> must be between 1 and 15 for CVMX_PKO_DQ_SEND command
 * otherwise it must be 0.
 *
 * NOTE: Internal use only.
 */
static inline cvmx_pko_query_rtn_t
__cvmx_pko3_do_dma(uint8_t node, uint16_t dq, uint64_t cmds[],
	unsigned numwords, enum cvmx_pko_dqop dqop)
{
	const unsigned scr_base = CVMX_PKO_LMTLINE * CVMX_CACHE_LINE_SIZE;
	cvmx_pko_query_rtn_t pko_status;
	cvmx_pko_lmtdma_data_t pko_send_dma_data;
	uint64_t dma_addr;
	unsigned i, scr_off;
	cvmx_pko3_dq_params_t *pParam;

	pParam = cvmx_pko3_dq_parameters(node, dq);
	CVMX_PREFETCH0(pParam);
	/* Push WB */
	CVMX_SYNCWS;

	pko_status.u64 = 0;
	pko_send_dma_data.u64 = 0;

	if (cvmx_unlikely(numwords > 15)) {
		cvmx_dprintf("%s: ERROR: Internal error\n",
				__FUNCTION__);
		pko_status.u64 = ~0ull;
		return pko_status;
	}

	/* Store the command words into CVMSEG LM */
	for(i = 0, scr_off = scr_base; i < numwords; i++) {
		cvmx_scratch_write64(scr_off, cmds[i]);
		scr_off += sizeof(cmds[0]);
	}

	/* With 0 data to send, this is an IOBDMA, else LMTDMA operation */
	if(numwords == 0) {
		dma_addr = CVMX_IOBDMA_ORDERED_IO_ADDR;
	} else {
		/* LMTDMA address offset is (nWords-1) */
		dma_addr = CVMX_LMTDMA_ORDERED_IO_ADDR;
		dma_addr += (numwords - 1) << 3;
	}

	if (cvmx_likely(dqop == CVMX_PKO_DQ_SEND)) {
		if (cvmx_unlikely(pParam->depth > pParam->limit)) {
			cvmx_pko_dqx_wm_cnt_t wm_cnt;
			wm_cnt.u64 = cvmx_read_csr_node(node,
				CVMX_PKO_DQX_WM_CNT(dq));
			pParam->depth = pko_status.s.depth = wm_cnt.s.count;
		}

		if (cvmx_unlikely(pParam->depth > pParam->limit)) {
			pko_status.s.dqop = dqop;
			pko_status.s.dqstatus = PKO_DQSTATUS_NOFPABUF;
			return pko_status;
		} else {
			cvmx_atomic_add32_nosync( &(pParam->depth), 1);
		}
	}

	if (cvmx_unlikely(dqop != CVMX_PKO_DQ_SEND) ||
	    CVMX_ENABLE_PARAMETER_CHECKING) {
		/* Request one return word */
		pko_send_dma_data.s.rtnlen = 1;
		/* Write all-ones into the return area */
		cvmx_scratch_write64(scr_off, ~0ull);
	} else {
		/* Do not expext a return word */
		pko_send_dma_data.s.rtnlen = 0;
	}

	/* build store data for DMA */
	pko_send_dma_data.s.scraddr = scr_off >> 3;
	pko_send_dma_data.s.did = 0x51;
	pko_send_dma_data.s.node = node;
	pko_send_dma_data.s.dqop = dqop;
	pko_send_dma_data.s.dq = dq;

	/* Barrier: make sure all prior writes complete before the following */
	CVMX_SYNCWS;

	/* Wait to finish tag switch just before issueing LMTDMA */
	cvmx_pow_tag_sw_wait();

	/* issue PKO DMA */
	cvmx_write64_uint64(dma_addr, pko_send_dma_data.u64);

	if (pko_send_dma_data.s.rtnlen) {
		/* Wait LMTDMA for completion */
		CVMX_SYNCIOBDMA;

		/* Retreive real result */
		pko_status.u64 = cvmx_scratch_read64(scr_off);
		pParam->depth = pko_status.s.depth;
	} else {
		/* Fake positive result */
		pko_status.s.dqop = dqop;
		pko_status.s.dqstatus = PKO_DQSTATUS_PASS;
	}

	return pko_status;
}


/*
 * Transmit packets through PKO, simplified API
 *
 * @INTERNAL
 *
 * @param dq is a global destination queue number
 * @param pki_ptr specifies packet first linked pointer as returned from
 * 'cvmx_wqe_get_pki_pkt_ptr()'.
 * @param len is the total number of bytes in the packet.
 * @param gaura is the aura to free packet buffers after trasnmit.
 * @param pCounter is an address of a 64-bit counter to atomically
 * @param ptag is a Flow Tag pointer for packet odering or NULL
 * decrement when packet transmission is complete.
 *
 * @return returns 0 if successful and -1 on failure.
 *
 *
 * NOTE: This is a provisional API, and is subject to change.
 */
static inline int
cvmx_pko3_xmit_link_buf(int dq,cvmx_buf_ptr_pki_t pki_ptr,
	unsigned len, int gaura, uint64_t *pCounter, uint32_t *ptag)
{
	cvmx_pko_query_rtn_t pko_status;
	cvmx_pko_send_hdr_t hdr_s;
	cvmx_pko_buf_ptr_t gtr_s;
	unsigned node, nwords;
	unsigned scr_base = cvmx_pko3_lmtdma_scr_base();

	/* Separate global DQ# into node and local DQ */
	node = dq >> 10;
	dq &= (1 << 10)-1;

	/* Fill in header */
	hdr_s.u64 = 0;
	hdr_s.s.total = len;
	hdr_s.s.df = (gaura < 0);
	hdr_s.s.ii = 1;
	hdr_s.s.aura = (gaura >= 0)? gaura: 0;
#ifdef __LITTLE_ENDIAN_BITFIELD
	hdr_s.s.le = 1;
	cvmx_wqe_pko_errata_22235(pki_ptr, len);
#endif

	/* Fill in gather */
	gtr_s.u64 = 0;
	gtr_s.s.subdc3 = CVMX_PKO_SENDSUBDC_LINK;
	gtr_s.s.addr = pki_ptr.addr;
	gtr_s.s.size = pki_ptr.size;

	/* Setup command word pointers */
	cvmx_scratch_write64(scr_base+sizeof(uint64_t)*0, hdr_s.u64);
	cvmx_scratch_write64(scr_base+sizeof(uint64_t)*1, gtr_s.u64);
	nwords = 2;

	/* Conditionally setup an atomic decrement counter */
	if (pCounter != NULL) {
		cvmx_pko_send_mem_t mem_s;
		mem_s.s.subdc4 = CVMX_PKO_SENDSUBDC_MEM;
		mem_s.s.dsz = MEMDSZ_B64;
		mem_s.s.alg = MEMALG_SUB;
		mem_s.s.offset = 1;
		mem_s.s.wmem = 0;
		mem_s.s.addr = cvmx_ptr_to_phys(CASTPTR(void,pCounter));
		cvmx_scratch_write64(
			scr_base + sizeof(uint64_t) * nwords++,
			mem_s.u64);
	}

	/* To preserve packet order, go atomic with DQ-specific tag */
	if (ptag != NULL)
		cvmx_pow_tag_sw(*ptag ^ dq, CVMX_POW_TAG_TYPE_ATOMIC);
 
	/* Do LMTDMA */
	pko_status = __cvmx_pko3_lmtdma(node, dq, nwords, ptag != NULL);

	if (cvmx_likely(pko_status.s.dqstatus == PKO_DQSTATUS_PASS))
		return 0;
	else
		return -1;
}

/**
 * @INTERNAL
 *
 * Retreive PKO internal AURA from register.
 */
static inline unsigned __cvmx_pko3_aura_get(unsigned node)
{
	static int16_t aura = -1;
	cvmx_pko_dpfi_fpa_aura_t pko_aura;

	if (aura >= 0)
		return aura;

	pko_aura.u64 = cvmx_read_csr_node(node, CVMX_PKO_DPFI_FPA_AURA);

	aura =  (pko_aura.s.node << 10) | pko_aura.s.laura;
	return aura;
}

 /** Open configured descriptor queues before queueing packets into them.
 *
 * @param node is to specify the node to which this configuration is applied.
 * @param dq is the descriptor queue number to be opened.
 * @return returns 0 on sucess or -1 on failure.
 */
int cvmx_pko_dq_open(int node, int dq);

 /** Close a descriptor queue
 *
 * @param node is to specify the node to which this configuration is applied.
 * @param dq is the descriptor queue number to be opened.
 * @return returns 0 on sucess or -1 on failure.
 *
 * This should be called before changing the DQ parent link, topology,
 * or when shutting down the PKO.
 */
int cvmx_pko3_dq_close(int node, int dq);

 /** Query a descriptor queue
 *
 * @param node is to specify the node to which this configuration is applied.
 * @param dq is the descriptor queue number to be opened.
 * @return returns the descriptor queue depth on sucess or -1 on failure.
 *
 * This should be called before changing the DQ parent link, topology,
 * or when shutting down the PKO.
 */
int cvmx_pko3_dq_query(int node, int dq);

/** Drain a descriptor queue
 *
 * Before closing a DQ, this call will drain all pending traffic
 * on the DQ to the NULL MAC, which will circumvent any traffic
 * shaping and flow control to quickly reclaim all packet buffers.
 */
void cvmx_pko3_dq_drain(int node, int dq);

/*
 * PKO global intialization for 78XX.
 *
 * @param node is the node on which PKO block is initialized.
 * @param aura is the 12-bit AURA (including node) for PKO internal use.
 * @return none.
 */
int cvmx_pko3_hw_init_global(int node, uint16_t aura);

/**
 * Shutdown the entire PKO
 */
int cvmx_pko3_hw_disable(int node);


/* Define legacy type here to break circular dependency */
typedef struct cvmx_pko_port_status cvmx_pko_port_status_t;

/**
 * @INTERNAL
 * Backward compatibility for collecting statistics from PKO3
 *
 */
extern void cvmx_pko3_get_legacy_port_stats(uint16_t ipd_port,
	unsigned clear, cvmx_pko_port_status_t * status);

/** Set MAC options
 *
 * The options supported are the parameters below:
 *
 * @param xiface The physical interface number
 * @param index The physical sub-interface port
 * @param fcs_enable Enable FCS generation
 * @param pad_enable Enable padding to minimum packet size
 * @param fcs_sop_off Number of bytes at start of packet to exclude from FCS
 *
 * The typical use for `fcs_sop_off` is when the interface is configured
 * to use a header such as HighGig to precede every Ethernet packet,
 * such a header usually does not partake in the CRC32 computation stream,
 * and its size must be set with this parameter.
 *
 * @return Returns 0 on success, -1 if interface/port is invalid.
 */
extern int cvmx_pko3_interface_options(int xiface, int index,
			bool fcs_enable, bool pad_enable,
			unsigned fcs_sop_off);

/** Set Descriptor Queue options
 *
 * The `min_pad` parameter must be in agreement with the interface-level
 * padding option for all descriptor queues assigned to that particular
 * interface/port.
 */
extern void cvmx_pko3_dq_options(unsigned node, unsigned dq, bool min_pad);

extern int cvmx_pko3_port_fifo_size(unsigned int xiface, unsigned index);
extern int cvmx_pko3_channel_credit_level(int node,
	enum cvmx_pko3_level_e level);
extern int cvmx_pko3_port_xoff(unsigned int xiface, unsigned index);
extern int cvmx_pko3_port_xon(unsigned int xiface, unsigned index);

/* Packet descriptor - PKO3 command buffer + internal state */
typedef struct cvmx_pko3_pdesc_s {
	uint64_t *jump_buf;	/**< jump buffer vaddr */
	int16_t last_aura;	/**< AURA of the latest LINK_S/GATHER_S */
	unsigned
		num_words:5,	/**< valid words in word array 2..16 */
		headroom:10,	/**< free bytes at start of 1st buf */
		hdr_offsets:1,
		pki_word4_present : 1;
	/* PKO3 command buffer: */
	cvmx_pko_send_hdr_t *hdr_s;
	uint64_t word[16];	/**< header and subcommands buffer */
	/* Bookkeeping fields: */
	uint64_t send_work_s;	/**< SEND_WORK_S must be the very last subdc */
	int16_t jb_aura;	/**< AURA where the jump buffer belongs */
	uint16_t mem_s_ix;	/**< index of first MEM_S subcommand */
	uint8_t ckl4_alg;	/**< L3/L4 alg to use if recalc is needed */
	/* Fields saved from WQE for later inspection */
	cvmx_pki_wqe_word4_t pki_word4;
	cvmx_pki_wqe_word2_t pki_word2;
} cvmx_pko3_pdesc_t;

void cvmx_pko3_pdesc_init(cvmx_pko3_pdesc_t *pdesc);
int cvmx_pko3_pdesc_from_wqe(cvmx_pko3_pdesc_t *pdesc, cvmx_wqe_78xx_t *wqe,
	bool free_bufs);
int cvmx_pko3_pdesc_transmit(cvmx_pko3_pdesc_t *pdesc, uint16_t dq,
	uint32_t *flow_tag);
int cvmx_pko3_pdesc_notify_decrement(cvmx_pko3_pdesc_t *pdesc,
        volatile uint64_t *p_counter);
int cvmx_pko3_pdesc_notify_wqe(cvmx_pko3_pdesc_t *pdesc, cvmx_wqe_78xx_t *wqe,
	uint8_t node, uint8_t group, uint8_t tt, uint32_t tag);
int cvmx_pko3_pdesc_buf_append(cvmx_pko3_pdesc_t *pdesc, void *p_data,
		unsigned data_bytes, unsigned gaura);
int cvmx_pko3_pdesc_append_free(cvmx_pko3_pdesc_t *pdesc, uint64_t addr,
				unsigned gaura);
int cvmx_pko3_pdesc_hdr_push(cvmx_pko3_pdesc_t *pdesc,
	const void *p_data, uint8_t data_bytes, uint8_t layer);
int cvmx_pko3_pdesc_hdr_pop(cvmx_pko3_pdesc_t *pdesc,
		void *hdr_buf, unsigned num_bytes);
int cvmx_pko3_pdesc_hdr_peek(cvmx_pko3_pdesc_t *pdesc,
		void *hdr_buf, unsigned num_bytes, unsigned offset);
void cvmx_pko3_pdesc_set_free(cvmx_pko3_pdesc_t *pdesc, bool free_bufs);

#ifdef	__cplusplus
/* *INDENT-OFF* */
}
/* *INDENT-ON* */
#endif
#endif /* __CVMX_PKO3_H__ */


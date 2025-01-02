/***********************license start***************
 * Copyright (c) 2003-2017  Cavium Inc. (support@cavium.com). All rights
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
 * Interface to the hardware Scheduling unit.
 *
 * New, starting with SDK 1.7.0, cvmx-pow supports a number of
 * extended consistency checks. The define
 * CVMX_ENABLE_POW_CHECKS controls the runtime insertion of POW
 * internal state checks to find common programming errors. If
 * CVMX_ENABLE_POW_CHECKS is not defined, checks are by default
 * enabled. For example, cvmx-pow will check for the following
 * program errors or POW state inconsistency.
 * - Requesting a POW operation with an active tag switch in
 *   progress.
 * - Waiting for a tag switch to complete for an excessively
 *   long period. This is normally a sign of an error in locking
 *   causing deadlock.
 * - Illegal tag switches from NULL_NULL.
 * - Illegal tag switches from NULL.
 * - Illegal deschedule request.
 * - WQE pointer not matching the one attached to the core by
 *   the POW.
 */
#ifndef __CVMX_POW_H__
#define __CVMX_POW_H__

#include "cvmx-scratch.h"
#include "cvmx-wqe.h"

#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
#include <asm/octeon/cvmx-sso-defs.h>
#include <asm/octeon/cvmx-address.h>
#else
#include "cvmx-warn.h"
#include "cvmx-address.h"
#endif

#ifdef  __cplusplus
/* *INDENT-OFF* */
extern "C" {
/* *INDENT-ON* */
#endif

/* Default to having all POW constancy checks turned on */
#ifndef CVMX_ENABLE_POW_CHECKS
#define CVMX_ENABLE_POW_CHECKS 1
#endif

/*
 * Special type for CN78XX style SSO groups (0..255),
 * for distinction from legacy-style groups (0..15)
 */
typedef union {
	uint8_t xgrp;
	/* Fields that map XGRP for backwards compatibility */
	struct __attribute__ ((__packed__)) {
#ifdef __BIG_ENDIAN_BITFIELD
		uint8_t	group : 5;
		uint8_t	qus   : 3;
#else
		uint8_t	qus   : 3;
		uint8_t	group : 5;
#endif
	};
} cvmx_xgrp_t;

/*
 * Softwsare-only structure to convey a return value
 * containing multiple information fields about an work queue entry
 */
typedef struct {
	uint32_t tag;
	uint16_t index;
	uint8_t grp; /* Legacy group # (0..15) */
	uint8_t tag_type;
} cvmx_pow_tag_info_t;

/**
 * Wait flag values for pow functions.
 */
typedef enum {
	CVMX_POW_WAIT = 1,
	CVMX_POW_NO_WAIT = 0,
} cvmx_pow_wait_t;

/**
 *  POW tag operations.  These are used in the data stored to the POW.
 */
typedef enum {
	CVMX_POW_TAG_OP_SWTAG = 0L,
		/**< Switch the tag (only) for this PP
         - the previous tag should be non-NULL in this case
         - tag switch response required
         - fields used: op, type, tag */
	CVMX_POW_TAG_OP_SWTAG_FULL = 1L,
		/**< Switch the tag for this PP, with full information
         - this should be used when the previous tag is NULL
         - tag switch response required
         - fields used: address, op, grp, type, tag */
	CVMX_POW_TAG_OP_SWTAG_DESCH = 2L,
		/**< Switch the tag (and/or group) for this PP and de-schedule
         - OK to keep the tag the same and only change the group
         - fields used: op, no_sched, grp, type, tag */
	CVMX_POW_TAG_OP_DESCH = 3L,
		/**< Just de-schedule
         - fields used: op, no_sched */
	CVMX_POW_TAG_OP_ADDWQ = 4L,
		/**< Create an entirely new work queue entry
         - fields used: address, op, qos, grp, type, tag */
	CVMX_POW_TAG_OP_UPDATE_WQP_GRP = 5L,
		/**< Just update the work queue pointer and grp for this PP
         - fields used: address, op, grp */
	CVMX_POW_TAG_OP_SET_NSCHED = 6L,
		/**< Set the no_sched bit on the de-schedule list
         - does nothing if the selected entry is not on the de-schedule list
         - does nothing if the stored work queue pointer does not match the address field
         - fields used: address, index, op
         Before issuing a *_NSCHED operation, SW must guarantee that all
         prior deschedules and set/clr NSCHED operations are complete and all
         prior switches are complete. The hardware provides the opsdone bit
         and swdone bit for SW polling. After issuing a *_NSCHED operation,
         SW must guarantee that the set/clr NSCHED is complete before
         any subsequent operations. */
	CVMX_POW_TAG_OP_CLR_NSCHED = 7L,
		/**< Clears the no_sched bit on the de-schedule list
         - does nothing if the selected entry is not on the de-schedule list
         - does nothing if the stored work queue pointer does not match the address field
         - fields used: address, index, op
         Before issuing a *_NSCHED operation, SW must guarantee that all
         prior deschedules and set/clr NSCHED operations are complete and all
         prior switches are complete. The hardware provides the opsdone bit
         and swdone bit for SW polling. After issuing a *_NSCHED operation,
         SW must guarantee that the set/clr NSCHED is complete before
         any subsequent operations. */
	CVMX_POW_TAG_OP_NOP = 15L
		/**< Do nothing */
} cvmx_pow_tag_op_t;

/**
 * This structure defines the store data on a store to POW
 */
typedef union {
	uint64_t u64;
	struct {
#ifdef __BIG_ENDIAN_BITFIELD
		uint64_t no_sched :1;
			/**< Don't reschedule this entry.
			 no_sched is used for CVMX_POW_TAG_OP_SWTAG_DESCH and CVMX_POW_TAG_OP_DESCH */
		uint64_t unused :2;
		uint64_t index :13;
			/**< Contains index of entry for a CVMX_POW_TAG_OP_*_NSCHED */
		cvmx_pow_tag_op_t op :4;
			/**< The operation to perform */
		uint64_t unused2 :2;
		uint64_t qos :3;
			/**< The QOS level for the packet. qos is only used for CVMX_POW_TAG_OP_ADDWQ */
		uint64_t grp :4;
			/**< The group that the work queue entry will be scheduled to grp is used for
			 CVMX_POW_TAG_OP_ADDWQ, CVMX_POW_TAG_OP_SWTAG_FULL, CVMX_POW_TAG_OP_SWTAG_DESCH,
			 and CVMX_POW_TAG_OP_UPDATE_WQP_GRP */
		cvmx_pow_tag_type_t type :3;
			/**< The type of the tag. type is used for everything except
			 CVMX_POW_TAG_OP_DESCH, CVMX_POW_TAG_OP_UPDATE_WQP_GRP, and
			 CVMX_POW_TAG_OP_*_NSCHED */
		uint64_t tag :32;
			/**< The actual tag. tag is used for everything except
			 CVMX_POW_TAG_OP_DESCH, CVMX_POW_TAG_OP_UPDATE_WQP_GRP, and
			CVMX_POW_TAG_OP_*_NSCHED */
#else
		uint64_t tag :32;
		cvmx_pow_tag_type_t type :3;
		uint64_t grp :4;
		uint64_t qos :3;
		uint64_t unused2 :2;
		cvmx_pow_tag_op_t op :4;
		uint64_t index :13;
		uint64_t unused :2;
		uint64_t no_sched :1;
#endif
	} s_cn38xx;
	struct {
#ifdef __BIG_ENDIAN_BITFIELD
		uint64_t no_sched :1;
			/**< Don't reschedule this entry.
			 no_sched is used for CVMX_POW_TAG_OP_SWTAG_DESCH and CVMX_POW_TAG_OP_DESCH */
		cvmx_pow_tag_op_t op :4;
			/**< The operation to perform */
		uint64_t unused1 :4;
		uint64_t index :11;
			/**< Contains index of entry for a CVMX_POW_TAG_OP_*_NSCHED */
		uint64_t unused2 :1;
		uint64_t grp :6;
			/**< The group that the work queue entry will be scheduled to grp is used
			 for CVMX_POW_TAG_OP_ADDWQ, CVMX_POW_TAG_OP_SWTAG_FULL,
			 CVMX_POW_TAG_OP_SWTAG_DESCH, and CVMX_POW_TAG_OP_UPDATE_WQP_GRP */
		uint64_t unused3 :3;
		cvmx_pow_tag_type_t type :2;
			/**< The type of the tag.
			 type is used for everything except CVMX_POW_TAG_OP_DESCH,
			 CVMX_POW_TAG_OP_UPDATE_WQP_GRP, and CVMX_POW_TAG_OP_*_NSCHED */
		uint64_t tag :32;
			/**< The actual tag.
			 tag is used for everything except CVMX_POW_TAG_OP_DESCH,
			 CVMX_POW_TAG_OP_UPDATE_WQP_GRP, and CVMX_POW_TAG_OP_*_NSCHED */
#else
		uint64_t tag :32;
		cvmx_pow_tag_type_t type :2;
		uint64_t unused3 :3;
		uint64_t grp :6;
		uint64_t unused2 :1;
		uint64_t index :11;
		uint64_t unused1 :4;
		cvmx_pow_tag_op_t op :4;
		uint64_t no_sched :1;
#endif
	} s_cn68xx_clr;
	struct {
#ifdef __BIG_ENDIAN_BITFIELD
		uint64_t no_sched :1;
			/**< Don't reschedule this entry.
			 no_sched is used for CVMX_POW_TAG_OP_SWTAG_DESCH and CVMX_POW_TAG_OP_DESCH */
		cvmx_pow_tag_op_t op :4;
			/**< The operation to perform */
		uint64_t unused1 :12;
		uint64_t qos :3;
			/**< Contains index of entry for a CVMX_POW_TAG_OP_*_NSCHED */
		uint64_t unused2 :1;
		uint64_t grp :6;
			/**< The group that the work queue entry will be scheduled to grp is used
			 for CVMX_POW_TAG_OP_ADDWQ, CVMX_POW_TAG_OP_SWTAG_FULL,
			 CVMX_POW_TAG_OP_SWTAG_DESCH, and CVMX_POW_TAG_OP_UPDATE_WQP_GRP */
		uint64_t unused3 :3;
		cvmx_pow_tag_type_t type :2;
			/**< The type of the tag. type is used for everything except
			 CVMX_POW_TAG_OP_DESCH, CVMX_POW_TAG_OP_UPDATE_WQP_GRP, and
			 CVMX_POW_TAG_OP_*_NSCHED */
		uint64_t tag :32;
			/**< The actual tag. tag is used for everything except CVMX_POW_TAG_OP_DESCH,
			 CVMX_POW_TAG_OP_UPDATE_WQP_GRP, and CVMX_POW_TAG_OP_*_NSCHED */
#else
		uint64_t tag :32;
		cvmx_pow_tag_type_t type :2;
		uint64_t unused3 :3;
		uint64_t grp :6;
		uint64_t unused2 :1;
		uint64_t qos :3;
		uint64_t unused1 :12;
		cvmx_pow_tag_op_t op :4;
		uint64_t no_sched :1;
#endif
	} s_cn68xx_add;
	struct {
#ifdef __BIG_ENDIAN_BITFIELD
		uint64_t no_sched :1;
			/**< Don't reschedule this entry.
			 no_sched is used for CVMX_POW_TAG_OP_SWTAG_DESCH and CVMX_POW_TAG_OP_DESCH */
		cvmx_pow_tag_op_t op :4;
			/**< The operation to perform */
		uint64_t unused1 :16;
		uint64_t grp :6;
			/**< The group that the work queue entry will be scheduled to grp is used
			 for CVMX_POW_TAG_OP_ADDWQ, CVMX_POW_TAG_OP_SWTAG_FULL,
			 CVMX_POW_TAG_OP_SWTAG_DESCH, and CVMX_POW_TAG_OP_UPDATE_WQP_GRP */
		uint64_t unused3 :3;
		cvmx_pow_tag_type_t type :2;
			/**< The type of the tag.
			 type is used for everything except CVMX_POW_TAG_OP_DESCH,
			 CVMX_POW_TAG_OP_UPDATE_WQP_GRP, and CVMX_POW_TAG_OP_*_NSCHED */
		uint64_t tag :32;
			/**< The actual tag.
			 tag is used for everything except CVMX_POW_TAG_OP_DESCH,
			 CVMX_POW_TAG_OP_UPDATE_WQP_GRP, and CVMX_POW_TAG_OP_*_NSCHED */
#else
		uint64_t tag :32;
		cvmx_pow_tag_type_t type :2;
		uint64_t unused3 :3;
		uint64_t grp :6;
		uint64_t unused1 :16;
		cvmx_pow_tag_op_t op :4;
		uint64_t no_sched :1;
#endif
	} s_cn68xx_other;

	struct {
#ifdef __BIG_ENDIAN_BITFIELD
		uint64_t rsvd_62_63 :2;
		uint64_t grp :10; /** NODE+LGROUP */
			/** Group that the work-queue entry will be scheduled to.
			 grp is used for ADDWQ, SWTAG_FULL, SWTAG_DESCH, UPD_WQP_GRP. */
		cvmx_pow_tag_type_t type :2;
			/** The type of the tag */
		uint64_t no_sched :1;
			/** Don't reschedule this entry. NOSCHED is used for SWTAG_DESCH and DESCHED.*/
		uint64_t rsvd_48 :1;
		cvmx_pow_tag_op_t op :4;
			/** The operation to perform */
		uint64_t rsvd_42_43 :2;
		uint64_t wqp :42;
			/** Address of the work-queue entry. Must be aligned on a 64-bit boundary.
			 Used for SWTAG_FULL, ADDWQ, UPD_WQP_GRP; addr<2:0> must be zero. */
#else
		uint64_t wqp :42;
		uint64_t rsvd_42_43 :2;
		cvmx_pow_tag_op_t op :4;
			/**< The operation to perform */
		uint64_t rsvd_48 :1;
		uint64_t no_sched :1;
		cvmx_pow_tag_type_t type :2;
		uint64_t grp :10;
		uint64_t rsvd_62_63 :2;
#endif
	} s_cn78xx_other;

} cvmx_pow_tag_req_t;

union cvmx_pow_tag_req_addr {
	uint64_t u64;
	struct {
		CVMX_BITFIELD_FIELD(uint64_t mem_region :2,
			/**< Mips64 address region. Should be CVMX_IO_SEG */
		CVMX_BITFIELD_FIELD(uint64_t reserved_49_61 :13,
		CVMX_BITFIELD_FIELD(uint64_t is_io :1,/**< Must be one */
		CVMX_BITFIELD_FIELD(uint64_t did :8,
		CVMX_BITFIELD_FIELD(uint64_t addr :40,
	    ;)))))
	} s;
	struct {
		CVMX_BITFIELD_FIELD(uint64_t mem_region :2,
			/**< Mips64 address region. Should be CVMX_IO_SEG */
		CVMX_BITFIELD_FIELD(uint64_t reserved_49_61 :13,
		CVMX_BITFIELD_FIELD(uint64_t is_io :1, /**< Must be one */
		CVMX_BITFIELD_FIELD(uint64_t did :8,
		CVMX_BITFIELD_FIELD(uint64_t node :4,
			/**<OCI node number. For SWTAG*, and UPD_WQP_GRP this must
			 always indicate the local node, or an error will result.
			 ADDWQ can be performed onto any node. */
		CVMX_BITFIELD_FIELD(uint64_t tag :32,
			/**< The actual tag. Tag is used for SWTAG, SWTAG_FULL,
			 SWTAG_DESCHED, ADDWQ. */
		CVMX_BITFIELD_FIELD(uint64_t reserved_0_3 :4,
	    ;)))))))
	} s_cn78xx;
};

/**
 * This structure describes the address to load stuff from POW
 */
typedef union {
	uint64_t u64;
	/**
	 * Address for new work request loads (did<2:0> == 0)
	 */
	struct {
		CVMX_BITFIELD_FIELD(uint64_t mem_region :2,
		    /**< Mips64 address region. Should be CVMX_IO_SEG */
		CVMX_BITFIELD_FIELD(uint64_t reserved_49_61 :13, /**< Must be zero */
		CVMX_BITFIELD_FIELD(uint64_t is_io :1, /**< Must be one */
		CVMX_BITFIELD_FIELD(uint64_t did :8,
			/**< The ID of POW -- did<2:0> == 0 in this case */
		CVMX_BITFIELD_FIELD(uint64_t reserved_4_39 :36, /**< Must be zero */
		CVMX_BITFIELD_FIELD(uint64_t wait :1,
			/**< If set, don't return load response until work is available */
		CVMX_BITFIELD_FIELD(uint64_t reserved_0_2 :3, /**< Must be zero */
		)))))));
	} swork;

	struct {
		CVMX_BITFIELD_FIELD(uint64_t mem_region :2,
			/**< Mips64 address region. Should be CVMX_IO_SEG */
		CVMX_BITFIELD_FIELD(uint64_t reserved_49_61 :13, /**< Must be zero */
		CVMX_BITFIELD_FIELD(uint64_t is_io :1, /**< Must be one */
		CVMX_BITFIELD_FIELD(uint64_t did :8,
			/**< The ID of POW -- did<2:0> == 0 in this case */
		CVMX_BITFIELD_FIELD(uint64_t node :4, /**< OCI Node number */
		CVMX_BITFIELD_FIELD(uint64_t reserved_32_35 :4, /**< Must be zero */
		CVMX_BITFIELD_FIELD(uint64_t indexed :1, /**< Indexed get_work if set */
		CVMX_BITFIELD_FIELD(uint64_t grouped :1, /**< get_work for group specified in index */
		CVMX_BITFIELD_FIELD(uint64_t rtngrp :1, /**< Return group and tt in the return if set */
		CVMX_BITFIELD_FIELD(uint64_t reserved_16_28 :13, /**< Must be zero */
		CVMX_BITFIELD_FIELD(uint64_t index :12, /**< mask/grp/index of the request */
		CVMX_BITFIELD_FIELD(uint64_t wait :1,
			/**< If set, don't return load response until work is available */
		CVMX_BITFIELD_FIELD(uint64_t reserved_0_2 :3, /**< Must be zero */
		)))))))))))));
	} swork_78xx;
	/**
	 * Address for loads to get POW internal status
	 */
	struct {
		CVMX_BITFIELD_FIELD(uint64_t mem_region :2,
		    /**< Mips64 address region. Should be CVMX_IO_SEG */
		CVMX_BITFIELD_FIELD(uint64_t reserved_49_61 :13, /**< Must be zero */
		CVMX_BITFIELD_FIELD(uint64_t is_io :1, /**< Must be one */
		CVMX_BITFIELD_FIELD(uint64_t did :8,
			/**< The ID of POW -- did<2:0> == 1 in this case */
		CVMX_BITFIELD_FIELD(uint64_t reserved_10_39 :30, /**< Must be zero */
		CVMX_BITFIELD_FIELD(uint64_t coreid :4, /**< The core id to get status for */
		CVMX_BITFIELD_FIELD(uint64_t get_rev :1,
			/**< If set and get_cur is set, return reverse tag-list pointer rather
			 than forward tag-list pointer */
		CVMX_BITFIELD_FIELD(uint64_t get_cur :1,
			/**< If set, return current status rather than pending status */
		CVMX_BITFIELD_FIELD(uint64_t get_wqp :1,
			/**< If set, get the work-queue pointer rather than tag/type */
		CVMX_BITFIELD_FIELD(uint64_t reserved_0_2 :3, /**< Must be zero */
		))))))))));
	} sstatus;
	/**
	 * Address for loads to get 68XX SS0 internal status
	 */
	struct {
		CVMX_BITFIELD_FIELD(uint64_t mem_region :2,
		    /**< Mips64 address region. Should be CVMX_IO_SEG */
		CVMX_BITFIELD_FIELD(uint64_t reserved_49_61 :13, /**< Must be zero */
		CVMX_BITFIELD_FIELD(uint64_t is_io :1, /**< Must be one */
		CVMX_BITFIELD_FIELD(uint64_t did :8,
			/**< The ID of POW -- did<2:0> == 1 in this case */
		CVMX_BITFIELD_FIELD(uint64_t reserved_14_39 :26, /**< Must be zero */
		CVMX_BITFIELD_FIELD(uint64_t coreid :5, /**< The core id to get status for */
		CVMX_BITFIELD_FIELD(uint64_t reserved_6_8 :3,
		CVMX_BITFIELD_FIELD(uint64_t opcode :3, /**< Status operation */
		CVMX_BITFIELD_FIELD(uint64_t reserved_0_2 :3, /**< Must be zero */
		)))))))));
	} sstatus_cn68xx;
	/**
	 * Address for memory loads to get POW internal state
	 */
	struct {
		CVMX_BITFIELD_FIELD(uint64_t mem_region :2,
		    /**< Mips64 address region. Should be CVMX_IO_SEG */
		CVMX_BITFIELD_FIELD(uint64_t reserved_49_61 :13, /**< Must be zero */
		CVMX_BITFIELD_FIELD(uint64_t is_io :1, /**< Must be one */
		CVMX_BITFIELD_FIELD(uint64_t did :8,
			/**< The ID of POW -- did<2:0> == 2 in this case */
		CVMX_BITFIELD_FIELD(uint64_t reserved_16_39 :24, /**< Must be zero */
		CVMX_BITFIELD_FIELD(uint64_t index :11, /**< POW memory index */
		CVMX_BITFIELD_FIELD(uint64_t get_des :1,
			/**< If set, return deschedule information rather than the standard
			 response for work-queue index (invalid if the work-queue entry is
			 not on the deschedule list). */
		CVMX_BITFIELD_FIELD(uint64_t get_wqp :1,
			/**< If set, get the work-queue pointer rather than tag/type
			 (no effect when get_des set). */
		CVMX_BITFIELD_FIELD(uint64_t reserved_0_2 :3, /**< Must be zero */
		)))))))));
	} smemload;
	/**
	 * Address for memory loads to get SSO internal state
	 */
	struct {
		CVMX_BITFIELD_FIELD(uint64_t mem_region :2,
		    /**< Mips64 address region. Should be CVMX_IO_SEG */
		CVMX_BITFIELD_FIELD(uint64_t reserved_49_61 :13, /**< Must be zero */
		CVMX_BITFIELD_FIELD(uint64_t is_io :1, /**< Must be one */
		CVMX_BITFIELD_FIELD(uint64_t did :8,
			/**< The ID of SSO - did<2:0> == 2 in this case */
		CVMX_BITFIELD_FIELD(uint64_t reserved_20_39 :20, /**< Must be zero */
		CVMX_BITFIELD_FIELD(uint64_t index :11, /**< SSO memory index */
		CVMX_BITFIELD_FIELD(uint64_t reserved_6_8 :3, /**< Must be zero */
		CVMX_BITFIELD_FIELD(uint64_t opcode :3,
			/**< Read TAG/WQ pointer/pending tag/next potr */
		CVMX_BITFIELD_FIELD(uint64_t reserved_0_2 :3, /**< Must be zero */
		)))))))));
	} smemload_cn68xx;
	/**
	 * Address for index/pointer loads
 	 */
	struct {
		CVMX_BITFIELD_FIELD(uint64_t mem_region :2,
			/**< Mips64 address region. Should be CVMX_IO_SEG */
		CVMX_BITFIELD_FIELD(uint64_t reserved_49_61 :13, /**< Must be zero */
		CVMX_BITFIELD_FIELD(uint64_t is_io :1, /**< Must be one */
		CVMX_BITFIELD_FIELD(uint64_t did :8,
			/**< The ID of POW -- did<2:0> == 3 in this case */
		CVMX_BITFIELD_FIELD(uint64_t reserved_9_39 :31, /**< Must be zero */
		CVMX_BITFIELD_FIELD(uint64_t qosgrp :4,
			/**< When {get_rmt ==0 AND get_des_get_tail == 0}, this field selects one of
			 eight POW internal-input queues (0-7), one per QOS level; values 8-15 are
			 illegal in this case.
			 When {get_rmt ==0 AND get_des_get_tail == 1}, this field selects one of
			 16 deschedule lists (per group);
			 When get_rmt ==1, this field selects one of 16 memory-input queue lists.
			 The two memory-input queue lists associated with each QOS level are:
			 - qosgrp = 0, qosgrp = 8:      QOS0
			 - qosgrp = 1, qosgrp = 9:      QOS1
			 - qosgrp = 2, qosgrp = 10:     QOS2
			 - qosgrp = 3, qosgrp = 11:     QOS3
			 - qosgrp = 4, qosgrp = 12:     QOS4
			 - qosgrp = 5, qosgrp = 13:     QOS5
			 - qosgrp = 6, qosgrp = 14:     QOS6
			 - qosgrp = 7, qosgrp = 15:     QOS7 */
		CVMX_BITFIELD_FIELD(uint64_t get_des_get_tail :1,
			/**< If set and get_rmt is clear, return deschedule list indexes
			 rather than indexes for the specified qos level.
			 If set and get_rmt is set, return the tail pointer rather than the head
			 pointer for the specified qos level. */
		CVMX_BITFIELD_FIELD(uint64_t get_rmt :1,
			/**< If set, return remote pointers rather than the local indexes
			 for the specified qos level. */
		CVMX_BITFIELD_FIELD(uint64_t reserved_0_2 :3, /**< Must be zero */
		)))))))));
	} sindexload;
	/**
	 * Address for a Index/Pointer loads to get SSO internal state
	 */
	struct {
		CVMX_BITFIELD_FIELD(uint64_t mem_region :2,
			/**< Mips64 address region. Should be CVMX_IO_SEG */
		CVMX_BITFIELD_FIELD(uint64_t reserved_49_61 :13, /**< Must be zero */
		CVMX_BITFIELD_FIELD(uint64_t is_io :1, /**< Must be one */
		CVMX_BITFIELD_FIELD(uint64_t did :8,
			/**< The ID of SSO - did<2:0> == 2 in this case */
		CVMX_BITFIELD_FIELD(uint64_t reserved_15_39 :25, /**< Must be zero */
		CVMX_BITFIELD_FIELD(uint64_t qos_grp :6,
			/**< When opcode = IPL_IQ, this field specifies IQ (or QOS).
			 When opcode = IPL_DESCHED, this field specifies the group.
			 This field is reserved for all other opcodes. */
		CVMX_BITFIELD_FIELD(uint64_t reserved_6_8 :3, /**< Must be zero */
		CVMX_BITFIELD_FIELD(uint64_t opcode :3,
			/**< Read TAG/WQ pointer/pending tag/next potr */
		CVMX_BITFIELD_FIELD(uint64_t reserved_0_2 :3, /**< Must be zero */
		)))))))));
	} sindexload_cn68xx;
	/**
	 * Address for NULL_RD request (did<2:0> == 4)
	 * when this is read, HW attempts to change the state to NULL if it is NULL_NULL
	 * (the hardware cannot switch from NULL_NULL to NULL if a POW entry is not available -
	 * software may need to recover by finishing another piece of work before a POW
	 * entry can ever become available.)
	 */
	struct {
		CVMX_BITFIELD_FIELD(uint64_t mem_region :2,
			/**< Mips64 address region. Should be CVMX_IO_SEG */
		CVMX_BITFIELD_FIELD(uint64_t reserved_49_61 :13, /**< Must be zero */
		CVMX_BITFIELD_FIELD(uint64_t is_io :1, /**< Must be one */
		CVMX_BITFIELD_FIELD(uint64_t did :8,
			/**< the ID of POW -- did<2:0> == 4 in this case */
		CVMX_BITFIELD_FIELD(uint64_t reserved_0_39 :40, /**< Must be zero */
		)))));
	} snull_rd;
} cvmx_pow_load_addr_t;

/**
 * This structure defines the response to a load/SENDSINGLE to POW (except CSR reads)
 */
typedef union {
	uint64_t u64;
	/**
	 * Response to new work request loads
	 */
	struct {
		CVMX_BITFIELD_FIELD(uint64_t no_work :1,
			/**< Set when no new work queue entry was returned.
			 If there was de-scheduled work, the HW will definitely return it.
			 When this bit is set, it could mean
			 - There was no work, or
			 - There was no work that the HW could find. This case can happen
			 regardless of the wait bit value in the original request, when there
			 is work in the IQ's that is too deep down the list. */
		CVMX_BITFIELD_FIELD(uint64_t pend_switch :1,
			/**< cn68XX and above, set if there was a pending tag switch*/
		CVMX_BITFIELD_FIELD(uint64_t tt :2,
		CVMX_BITFIELD_FIELD(uint64_t reserved_58_59 :2,
		CVMX_BITFIELD_FIELD(uint64_t grp :10,
		CVMX_BITFIELD_FIELD(uint64_t reserved_42_47 :6, /**< Must be zero */
		CVMX_BITFIELD_FIELD(uint64_t addr :42, /**< 36 in O1 -- the work queue pointer */
		)))))));
	} s_work;

	/**
	 * Result for a POW Status Load (when get_cur==0 and get_wqp==0)
	 */
	struct {
#ifdef __BIG_ENDIAN_BITFIELD
		uint64_t reserved_62_63 :2;
		uint64_t pend_switch :1;
			/**< Set when there is a pending non-NULL SWTAG or SWTAG_FULL, and
			 the POW entry has not left the list for the original tag. */
		uint64_t pend_switch_full :1;
			/**< Set when SWTAG_FULL and pend_switch is set. */
		uint64_t pend_switch_null :1;
			/**< Set when there is a pending NULL SWTAG, or an implicit switch to NULL. */
		uint64_t pend_desched :1;
			/**< Set when there is a pending DESCHED or SWTAG_DESCHED. */
		uint64_t pend_desched_switch :1;
			/**< Set when there is a pending SWTAG_DESCHED and pend_desched is set. */
		uint64_t pend_nosched :1;
			/**< Set when nosched is desired and pend_desched is set. */
		uint64_t pend_new_work :1;
			/**< Set when there is a pending GET_WORK. */
		uint64_t pend_new_work_wait :1;
			/**< When pend_new_work is set, this bit indicates that the wait bit was set. */
		uint64_t pend_null_rd :1;
			/**< Set when there is a pending NULL_RD. */
		uint64_t pend_nosched_clr :1;
			/**< Set when there is a pending CLR_NSCHED. */
		uint64_t reserved_51 :1;
		uint64_t pend_index :11;
			/**< This is the index when pend_nosched_clr is set. */
		uint64_t pend_grp :4;
			/**< This is the new_grp when (pend_desched AND pend_desched_switch) is set. */
		uint64_t reserved_34_35 :2;
		uint64_t pend_type :2;
			/**< This is the tag type when pend_switch or
			 (pend_desched AND pend_desched_switch) are set. */
		uint64_t pend_tag :32;
			/**< This is the tag when pend_switch or
			 (pend_desched AND pend_desched_switch) are set. */
#else
		uint64_t pend_tag :32;
		uint64_t pend_type :2;
		uint64_t reserved_34_35 :2;
		uint64_t pend_grp :4;
		uint64_t pend_index :11;
		uint64_t reserved_51 :1;
		uint64_t pend_nosched_clr :1;
		uint64_t pend_null_rd :1;
		uint64_t pend_new_work_wait :1;
		uint64_t pend_new_work :1;
		uint64_t pend_nosched :1;
		uint64_t pend_desched_switch :1;
		uint64_t pend_desched :1;
		uint64_t pend_switch_null :1;
		uint64_t pend_switch_full :1;
		uint64_t pend_switch :1;
		uint64_t reserved_62_63 :2;
#endif
	} s_sstatus0;
	/**
	 * Result for a SSO Status Load (when opcode is SL_PENDTAG)
	 */
	struct {
#ifdef __BIG_ENDIAN_BITFIELD
		uint64_t pend_switch:1;
			/**< Set when there is a pending non-UNSCHEDULED SWTAG or
			 SWTAG_FULL, and the SSO entry has not left the list for the original tag. */
		uint64_t pend_get_work:1;
			/**< Set when there is a pending GET_WORK */
		uint64_t pend_get_work_wait:1;
			/**< When pend_get_work is set, this biit indicates that the wait bit was set. */
		uint64_t pend_nosched:1;
			/**< Set when nosched is desired and pend_desched is set. */
		uint64_t pend_nosched_clr:1;
			/**< Set when there is a pending CLR_NSCHED. */
		uint64_t pend_desched:1;
			/**< Set when there is a pending DESCHED or SWTAG_DESCHED. */
		uint64_t pend_alloc_we:1;
			/**< Set when there is a pending ALLOC_WE. */
		uint64_t reserved_48_56:9;
		uint64_t pend_index:11;
			/**< This is the index when pend_nosched_clr is set. */
		uint64_t reserved_34_36:3;
		uint64_t pend_type:2;
			/**< This is the tag type when pend_switch is set. */
		uint64_t pend_tag:32;
			/**< This is the tag when pend_switch is set. */
#else
		uint64_t pend_tag:32;
		uint64_t pend_type:2;
		uint64_t reserved_34_36:3;
		uint64_t pend_index:11;
		uint64_t reserved_48_56:9;
		uint64_t pend_alloc_we:1;
		uint64_t pend_desched:1;
		uint64_t pend_nosched_clr:1;
		uint64_t pend_nosched:1;
		uint64_t pend_get_work_wait:1;
		uint64_t pend_get_work:1;
		uint64_t pend_switch:1;
#endif
	} s_sstatus0_cn68xx;
	/**
	 * Result for a POW Status Load (when get_cur==0 and get_wqp==1)
	 */
	struct {
#ifdef __BIG_ENDIAN_BITFIELD
		uint64_t reserved_62_63:2;
		uint64_t pend_switch:1;
			/**< Set when there is a pending non-NULL SWTAG or SWTAG_FULL, and
			 the POW entry has not left the list for the original tag. */
		uint64_t pend_switch_full:1;
			/**< Set when SWTAG_FULL and pend_switch is set. */
		uint64_t pend_switch_null:1;
			/**< Set when there is a pending NULL SWTAG, or an implicit switch to NULL. */
		uint64_t pend_desched:1;
			/**< Set when there is a pending DESCHED or SWTAG_DESCHED. */
		uint64_t pend_desched_switch:1;
			/**< Set when there is a pending SWTAG_DESCHED and pend_desched is set. */
		uint64_t pend_nosched:1;
			/**< Set when nosched is desired and pend_desched is set. */
		uint64_t pend_new_work:1;
			/**< Set when there is a pending GET_WORK. */
		uint64_t pend_new_work_wait:1;
			/**< When pend_new_work is set, this bit indicates that the wait bit was set. */
		uint64_t pend_null_rd:1;
			/**< Set when there is a pending NULL_RD. */
		uint64_t pend_nosched_clr:1;
			/**< Set when there is a pending CLR_NSCHED. */
		uint64_t reserved_51:1;
		uint64_t pend_index:11;
			/**< This is the index when pend_nosched_clr is set. */
		uint64_t pend_grp:4;
			/**< This is the new_grp when (pend_desched AND pend_desched_switch) is set. */
		uint64_t pend_wqp:36;
			/**< This is the wqp when pend_nosched_clr is set. */
#else
		uint64_t pend_wqp:36;
		uint64_t pend_grp:4;
		uint64_t pend_index:11;
		uint64_t reserved_51:1;
		uint64_t pend_nosched_clr:1;
		uint64_t pend_null_rd:1;
		uint64_t pend_new_work_wait:1;
		uint64_t pend_new_work:1;
		uint64_t pend_nosched:1;
		uint64_t pend_desched_switch:1;
		uint64_t pend_desched:1;
		uint64_t pend_switch_null:1;
		uint64_t pend_switch_full:1;
		uint64_t pend_switch:1;
		uint64_t reserved_62_63:2;
#endif
	} s_sstatus1;
	/**
	 * Result for a SSO Status Load (when opcode is SL_PENDWQP)
	 */
	struct {
#ifdef __BIG_ENDIAN_BITFIELD
		uint64_t pend_switch:1;
			/**< Set when there is a pending non-UNSCHEDULED SWTAG or SWTAG_FULL, and
			 the SSO entry has not left the list for the original tag. */
		uint64_t pend_get_work:1;
			/**< Set when there is a pending GET_WORK */
		uint64_t pend_get_work_wait:1;
			/**< when pend_get_work is set, this biit indicates that the wait bit was set. */
		uint64_t pend_nosched:1;
			/**< Set when nosched is desired and pend_desched is set. */
		uint64_t pend_nosched_clr:1;
			/**< Set when there is a pending CLR_NSCHED. */
		uint64_t pend_desched:1;
			/**< Set when there is a pending DESCHED or SWTAG_DESCHED. */
		uint64_t pend_alloc_we:1;
			/**< Set when there is a pending ALLOC_WE. */
		uint64_t reserved_51_56:6;
		uint64_t pend_index:11;
			/**< This is the index when pend_nosched_clr is set. */
		uint64_t reserved_38_39:2;
		uint64_t pend_wqp:38;
			/**< This is the wqp when pend_nosched_clr is set. */
#else
		uint64_t pend_wqp:38;
		uint64_t reserved_38_39:2;
		uint64_t pend_index:11;
		uint64_t reserved_51_56:6;
		uint64_t pend_alloc_we:1;
		uint64_t pend_desched:1;
		uint64_t pend_nosched_clr:1;
		uint64_t pend_nosched:1;
		uint64_t pend_get_work_wait:1;
		uint64_t pend_get_work:1;
		uint64_t pend_switch:1;
#endif
	} s_sstatus1_cn68xx;

	struct {
#ifdef __BIG_ENDIAN_BITFIELD
		uint64_t pend_switch:1;
			/**< Set when there is a pending non-UNSCHEDULED SWTAG or SWTAG_FULL, and
			 the SSO entry has not left the list for the original tag. */
		uint64_t pend_get_work:1;
			/**< Set when there is a pending GET_WORK */
		uint64_t pend_get_work_wait:1;
			/**< When pend_get_work is set, this biit indicates that the wait bit was set. */
		uint64_t pend_nosched:1;
			/**< Set when nosched is desired and pend_desched is set. */
		uint64_t pend_nosched_clr:1;
			/**< Set when there is a pending CLR_NSCHED. */
		uint64_t pend_desched:1;
			/**< Set when there is a pending DESCHED or SWTAG_DESCHED. */
		uint64_t pend_alloc_we:1;
			/**< Set when there is a pending ALLOC_WE. */
		uint64_t reserved_56:1;
		uint64_t prep_index:12;
		uint64_t reserved_42_43:2;
		uint64_t pend_tag:42;
			/**< This is the wqp when pend_nosched_clr is set. */
#else
		uint64_t pend_tag:42;
		uint64_t reserved_42_43:2;
		uint64_t prep_index:12;
		uint64_t reserved_56:1;
		uint64_t pend_prep:1;
		uint64_t pend_alloc_we:1;
		uint64_t pend_desched:1;
		uint64_t pend_nosched_clr:1;
		uint64_t pend_nosched:1;
		uint64_t pend_get_work_wait:1;
		uint64_t pend_get_work:1;
		uint64_t pend_switch:1;
#endif
	} s_sso_ppx_pendwqp_cn78xx;
	/**
	 * Result for a POW Status Load (when get_cur==1, get_wqp==0, and get_rev==0)
	 */
	struct {
#ifdef __BIG_ENDIAN_BITFIELD
		uint64_t reserved_62_63:2;
		uint64_t link_index:11;
			/**< Points to the next POW entry in the tag list when tail == 0
			 (and tag_type is not NULL or NULL_NULL).*/
		uint64_t index:11; /**< The POW entry attached to the core.*/
		uint64_t grp:4;
			/**< The group attached to the core.
			 Updated when new tag list entered on SWTAG_FULL.*/
		uint64_t head:1;
			/**< Set when this POW entry is at the head of its tag list
			 (also set when in the NULL or NULL_NULL state).*/
		uint64_t tail:1;
			/**< Set when this POW entry is at the tail of its tag list
			 (also set when in the NULL or NULL_NULL state).*/
		uint64_t tag_type:2;
			/**< The tag type attached to the core
			 (updated when new tag list entered on SWTAG, SWTAG_FULL, or SWTAG_DESCHED).*/
		uint64_t tag:32;
			/**< The tag attached to the core.
			 Updated when new tag list entered on SWTAG, SWTAG_FULL, or SWTAG_DESCHED.*/
#else
		uint64_t tag:32;
		uint64_t tag_type:2;
		uint64_t tail:1;
		uint64_t head:1;
		uint64_t grp:4;
		uint64_t index:11;
		uint64_t link_index:11;
		uint64_t reserved_62_63:2;
#endif
	} s_sstatus2;
	/**
	 * Result for a SSO Status Load (when opcode is SL_TAG)
	 */
	struct {
#ifdef __BIG_ENDIAN_BITFIELD
		uint64_t reserved_57_63:7;
		uint64_t index:11; /**< The SSO entry attached to the core. */
		uint64_t reserved_45:1;
		uint64_t grp:6;
			/**< The group attached to the core.
			 Updated when new tag list entered on SWTAG_FULL. */
		uint64_t head:1;
			/**< Set when this SSO entry is at the head of its tag list.
			 Also set when in the UNSCHEDULED or EMPTY state. */
		uint64_t tail:1;
			/**< Set when this SSO entry is at the tail of its tag list.
			 Also set when in the UNSCHEDULED or EMPTY state. */
		uint64_t reserved_34_36:3;
		uint64_t tag_type:2;
			/**< The tag type attached to the core.
			 Updated when new tag list entered on SWTAG, SWTAG_FULL, or SWTAG_DESCHED. */
		uint64_t tag:32;
			/**< The tag attached to the core.
			 Updated when new tag list entered on SWTAG, SWTAG_FULL, or SWTAG_DESCHED. */
#else
		uint64_t tag:32;
		uint64_t tag_type:2;
		uint64_t reserved_34_36:3;
		uint64_t tail:1;
		uint64_t head:1;
		uint64_t grp:6;
		uint64_t reserved_45:1;
		uint64_t index:11;
		uint64_t reserved_57_63:7;
#endif
	} s_sstatus2_cn68xx;

	struct {
#ifdef __BIG_ENDIAN_BITFIELD
		uint64_t tailc:1;
		uint64_t reserved_60_62:3;
		uint64_t index:12;
		uint64_t reserved_46_47:2;
		uint64_t grp:10;
		uint64_t head:1;
		uint64_t tail:1;
		uint64_t tt:2;
		uint64_t tag:32;
#else
		uint64_t tag:32;
		uint64_t tt:2;
		uint64_t tail:1;
		uint64_t head:1;
		uint64_t grp:10;
		uint64_t reserved_46_47:2;
		uint64_t index:12;
		uint64_t reserved_60_62:3;
		uint64_t tailc:1;
#endif
	} s_sso_ppx_tag_cn78xx;
	/**
	 * Result for a POW Status Load (when get_cur==1, get_wqp==0, and get_rev==1)
	 */
	struct {
#ifdef __BIG_ENDIAN_BITFIELD
		uint64_t reserved_62_63:2;
		uint64_t revlink_index:11;
			/**< Points to the prior POW entry in the tag list when head == 0
			 (and tag_type is not NULL or NULL_NULL). This field is unpredictable
			 when the core's state is NULL or NULL_NULL. */
		uint64_t index:11; /**< The POW entry attached to the core. */
		uint64_t grp:4;
			/**< The group attached to the core.
			 Updated when new tag list entered on SWTAG_FULL. */
		uint64_t head:1;
			/**< Set when this POW entry is at the head of its tag list.
			 Also set when in the NULL or NULL_NULL state. */
		uint64_t tail:1;
			/**< Set when this POW entry is at the tail of its tag list.
			 Also set when in the NULL or NULL_NULL state. */
		uint64_t tag_type:2;
			/**< The tag type attached to the core.
			 Updated when new tag list entered on SWTAG, SWTAG_FULL, or SWTAG_DESCHED. */
		uint64_t tag:32;
			/**< The tag attached to the core.
			 Updated when new tag list entered on SWTAG, SWTAG_FULL, or SWTAG_DESCHED. */
#else
		uint64_t tag:32;
		uint64_t tag_type:2;
		uint64_t tail:1;
		uint64_t head:1;
		uint64_t grp:4;
		uint64_t index:11;
		uint64_t revlink_index:11;
		uint64_t reserved_62_63:2;
#endif
	} s_sstatus3;
	/**
	 * Result for a SSO Status Load (when opcode is SL_WQP)
	 */
	struct {
#ifdef __BIG_ENDIAN_BITFIELD
		uint64_t reserved_58_63:6;
		uint64_t index:11; /**< The SSO entry attached to the core. */
		uint64_t reserved_46:1;
		uint64_t grp:6;
			/**< The group attached to the core.
			 Updated when new tag list entered on SWTAG_FULL. */
		uint64_t reserved_38_39:2;
		uint64_t wqp:38;
			/**< The wqp attached to the core.
			 Updated when new tag list entered on SWTAG_FULL. */
#else
		uint64_t wqp:38;
		uint64_t reserved_38_39:2;
		uint64_t grp:6;
		uint64_t reserved_46:1;
		uint64_t index:11;
		uint64_t reserved_58_63:6;
#endif
	} s_sstatus3_cn68xx;

	struct {
#ifdef __BIG_ENDIAN_BITFIELD
		uint64_t reserved_58_63:6;
		uint64_t grp:10;
		uint64_t reserved_42_47:6;
		uint64_t tag:42;
#else
		uint64_t tag:42;
		uint64_t reserved_42_47:6;
		uint64_t grp:10;
		uint64_t reserved_58_63:6;
#endif
	} s_sso_ppx_wqp_cn78xx;
	/**
	 * Result for a POW Status Load (when get_cur==1, get_wqp==1, and get_rev==0)
	 */
	struct {
#ifdef __BIG_ENDIAN_BITFIELD
		uint64_t reserved_62_63:2;
		uint64_t link_index:11;
			/**< Points to the next POW entry in the tag list when tail == 0
			 (and tag_type is not NULL or NULL_NULL). */
		uint64_t index:11; /**< The POW entry attached to the core. */
		uint64_t grp:4;
			/**< The group attached to the core.
			 Updated when new tag list entered on SWTAG_FULL. */
		uint64_t wqp:36;
			/**< The wqp attached to the core.
			 Updated when new tag list entered on SWTAG_FULL. */
#else
		uint64_t wqp:36;
		uint64_t grp:4;
		uint64_t index:11;
		uint64_t link_index:11;
		uint64_t reserved_62_63:2;
#endif
	} s_sstatus4;
	/**
	 * Result for a SSO Status Load (when opcode is SL_LINKS)
	 */
	struct {
#ifdef __BIG_ENDIAN_BITFIELD
		uint64_t reserved_46_63:18;
		uint64_t index:11; /**< The SSO entry attached to the core. */
		uint64_t reserved_34:1;
		uint64_t grp:6;
			/**< The group attached to the core
			 (updated when new tag list entered on SWTAG_FULL). */
		uint64_t head:1;
			/**< Set when this SSO entry is at the head of its tag list
			 (also set when in the UNSCHEDULED or EMPTY state). */
		uint64_t tail:1;
			/**< Set when this SSO entry is at the tail of its tag list
			 (also set when in the UNSCHEDULED or EMPTY state). */
		uint64_t reserved_24_25:2;
		uint64_t revlink_index:11;
			/**< Points to the prior SSO entry in the tag list when head==0
			 (and tag_type is not UNSCHEDULED or EMPTY). */
		uint64_t reserved_11_12:2;
		uint64_t link_index:11;
			/**< Points to the next SSO entry in the tag list when tail==0
			 (and tag_type is not UNSCHEDULDED or EMPTY). */
#else
		uint64_t link_index:11;
		uint64_t reserved_11_12:2;
		uint64_t revlink_index:11;
		uint64_t reserved_24_25:2;
		uint64_t tail:1;
		uint64_t head:1;
		uint64_t grp:6;
		uint64_t reserved_34:1;
		uint64_t index:11;
		uint64_t reserved_46_63:18;
#endif
	} s_sstatus4_cn68xx;

	struct {
#ifdef __BIG_ENDIAN_BITFIELD
		uint64_t tailc:1;
		uint64_t reserved_60_62:3;
		uint64_t index:12;
		uint64_t reserved_38_47:10;
		uint64_t grp:10;
		uint64_t head:1;
		uint64_t tail:1;
		uint64_t reserved_25:1;
		uint64_t revlink_index:12;
		uint64_t link_index_vld:1;
		uint64_t link_index:12;
#else
		uint64_t link_index:12;
		uint64_t link_index_vld:1;
		uint64_t revlink_index:12;
		uint64_t reserved_25:1;
		uint64_t tail:1;
		uint64_t head:1;
		uint64_t grp:10;
		uint64_t reserved_38_47:10;
		uint64_t index:12;
		uint64_t reserved_60_62:3;
		uint64_t tailc:1;
#endif
	} s_sso_ppx_links_cn78xx;
	/**
	 * Result for a POW Status Load (when get_cur==1, get_wqp==1, and get_rev==1)
	 */
	struct {
#ifdef __BIG_ENDIAN_BITFIELD
		uint64_t reserved_62_63:2;
		uint64_t revlink_index:11;
			/**< Points to the prior POW entry in the tag list when head == 0
			 (and tag_type is not NULL or NULL_NULL). This field is unpredictable
			 when the core's state is NULL or NULL_NULL. */
		uint64_t index:11; /**< The POW entry attached to the core. */
		uint64_t grp:4;
			/**< The group attached to the core
			 (updated when new tag list entered on SWTAG_FULL). */
		uint64_t wqp:36;
			/**< The wqp attached to the core
			 (updated when new tag list entered on SWTAG_FULL). */
#else
		uint64_t wqp:36;
		uint64_t grp:4;
		uint64_t index:11;
		uint64_t revlink_index:11;
		uint64_t reserved_62_63:2;
#endif
	} s_sstatus5;
	/**
	 * Result For POW Memory Load (get_des == 0 and get_wqp == 0)
	 */
	struct {
#ifdef __BIG_ENDIAN_BITFIELD
		uint64_t reserved_51_63:13;
		uint64_t next_index:11;
			/**< The next entry in the input, free, descheduled_head list
			 (unpredictable if entry is the tail of the list). */
		uint64_t grp:4; /**< The group of the POW entry. */
		uint64_t reserved_35:1;
		uint64_t tail:1;
			/**< Set when this POW entry is at the tail of its tag list
			 (also set when in the NULL or NULL_NULL state). */
		uint64_t tag_type:2; /**< The tag type of the POW entry. */
		uint64_t tag:32; /**< The tag of the POW entry. */
#else
		uint64_t tag:32;
		uint64_t tag_type:2;
		uint64_t tail:1;
		uint64_t reserved_35:1;
		uint64_t grp:4;
		uint64_t next_index:11;
		uint64_t reserved_51_63:13;
#endif
	} s_smemload0;
	/**
	 * Result For SSO Memory Load (opcode is ML_TAG)
	 */
	struct {
#ifdef __BIG_ENDIAN_BITFIELD
		uint64_t reserved_38_63:26;
		uint64_t tail:1;
			/**< Set when this SSO entry is at the tail of its tag list
			(also set when in the NULL or NULL_NULL state). */
		uint64_t reserved_34_36:3;
		uint64_t tag_type:2; /**< The tag type of the SSO entry. */
		uint64_t tag:32; /**< The tag of the SSO entry. */
#else
		uint64_t tag:32;
		uint64_t tag_type:2;
		uint64_t reserved_34_36:3;
		uint64_t tail:1;
		uint64_t reserved_38_63:26;
#endif
	} s_smemload0_cn68xx;

	struct {
#ifdef __BIG_ENDIAN_BITFIELD
		uint64_t reserved_39_63:25;
		uint64_t tail:1;
			/**< Set when this SSO entry is at the tail of its tag list
			 (also set when in the NULL or NULL_NULL state). */
		uint64_t reserved_34_36:3;
		uint64_t tag_type:2; /**< The tag type of the SSO entry. */
		uint64_t tag:32; /**< The tag of the SSO entry. */
#else
		uint64_t tag:32;
		uint64_t tag_type:2;
		uint64_t reserved_34_36:3;
		uint64_t tail:1;
		uint64_t reserved_38_63:26;
#endif
	} s_sso_iaq_ppx_tag_cn78xx;
	/**
	 * Result For POW Memory Load (get_des == 0 and get_wqp == 1)
	 */
	struct {
#ifdef __BIG_ENDIAN_BITFIELD
		uint64_t reserved_51_63:13;
		uint64_t next_index:11;
			/**< The next entry in the input, free, descheduled_head list
			 (unpredictable if entry is the tail of the list). */
		uint64_t grp:4; /**< The group of the POW entry. */
		uint64_t wqp:36; /**< The WQP held in the POW entry. */
#else
		uint64_t wqp:36;
		uint64_t grp:4;
		uint64_t next_index:11;
		uint64_t reserved_51_63:13;
#endif
	} s_smemload1;
	/**
	 * Result For SSO Memory Load (opcode is ML_WQPGRP)
	 */
	struct {
#ifdef __BIG_ENDIAN_BITFIELD
		uint64_t reserved_48_63:16;
		uint64_t nosched:1;	/**< The nosched bit for the SSO entry. */
		uint64_t reserved_46:1;
		uint64_t grp:6; /**< The group of the SSO entry. */
		uint64_t reserved_38_39:2;
		uint64_t wqp:38; /**< The WQP held in the SSO entry. */
#else
		uint64_t wqp:38;
		uint64_t reserved_38_39:2;
		uint64_t grp:6;
		uint64_t reserved_46:1;
		uint64_t nosched:1;
		uint64_t reserved_51_63:16;
#endif
	} s_smemload1_cn68xx;

	/**
	 * Entry structures for the CN7XXX chips.
	 */
	struct {
#ifdef __BIG_ENDIAN_BITFIELD
		uint64_t reserved_39_63:25;
		uint64_t tailc:1; /**< The SSO entry is the tail of tag chain that is conflicted. */
		uint64_t tail:1; /**< The SSO entry is the tail of tag chain that is descheduled. */
		uint64_t reserved_34_36:3;
		uint64_t tt:2; /**< The tag type of the SSO entry. Enumerated by SSO_TT_E. */
		uint64_t tag:32; /**< The tag of the SSO entry. */
#else
		uint64_t tag:32;
		uint64_t tt:2;
		uint64_t reserved_34_36:3;
		uint64_t tail:1;
		uint64_t tailc:1;
		uint64_t reserved_39_63:25;
#endif
	} s_sso_ientx_tag_cn78xx;

	struct {
#ifdef __BIG_ENDIAN_BITFIELD
		uint64_t reserved_62_63:2;
		uint64_t head:1;
		uint64_t nosched:1; /**< The nosched bit for the SSO entry. */
		uint64_t reserved_56_59:4;
		uint64_t grp:8; /**< The group of the SSO entry. */
		uint64_t reserved_42_47:6;
		uint64_t wqp:42; /**< The WQP held in the SSO entry. */
#else
		uint64_t wqp:42;
		uint64_t reserved_42_47:6;
		uint64_t grp:8;
		uint64_t reserved_56_59:4;
		uint64_t nosched:1;
		uint64_t head:1;
		uint64_t reserved_62_63:2;
#endif
	} s_sso_ientx_wqpgrp_cn73xx;

	struct {
#ifdef __BIG_ENDIAN_BITFIELD
		uint64_t reserved_62_63:2;
		uint64_t head:1;
		uint64_t nosched:1; /**< The nosched bit for the SSO entry. */
		uint64_t reserved_58_59:2;
		uint64_t grp:10; /**< The group of the SSO entry. */
		uint64_t reserved_42_47:6;
		uint64_t wqp:42; /**< The WQP held in the SSO entry. */
#else
		uint64_t wqp:42;
		uint64_t reserved_42_47:6;
		uint64_t grp:10;
		uint64_t reserved_58_59:2;
		uint64_t nosched:1;
		uint64_t head:1;
		uint64_t reserved_62_63:2;
#endif
	} s_sso_ientx_wqpgrp_cn78xx;

	struct {
#ifdef __BIG_ENDIAN_BITFIELD
		uint64_t reserved_38_63:26;
		uint64_t pend_switch:1; /**< Set when there is a pending no-UNTAGGED SWTAG or SWTAG_FULL
					  and the SSO entry has not left the list for the original tag. */
		uint64_t reserved_34_36:3;
		uint64_t pend_tt:2; /**< The next tag type for the new tag list when PEND_SWITCH is set.
				      Enumerated by SSO_TT_E. */
		uint64_t pend_tag:32; /**< The next tag for the new tag list when PEND_SWITCH is set. */
#else
		uint64_t pend_tag:32;
		uint64_t pend_tt:2;
		uint64_t reserved_34_36:3;
		uint64_t pend_switch:1;
		uint64_t reserved_38_63:26;
#endif
	} s_sso_ientx_pendtag_cn78xx;

	struct {
#ifdef __BIG_ENDIAN_BITFIELD
		uint64_t reserved_26_63 :38;
		uint64_t prev_index :10;
			/**< The previous entry in the tag chain. Unpredictable if the entry
			 is at the head of the list or the head of a conflicted tag chain. */
		uint64_t reserved_11_15 :5;
		uint64_t next_index_vld :1;
			/**< The NEXT_INDEX is valid. Unpredictable unless the entry is the
			 tail entry of an atomic tag chain. */
		uint64_t next_index :10;
			/**< The next entry in the tag chain or conflicted tag chain.
			 Unpredictable if the entry is at the tail of the list. */
#else
		uint64_t next_index :10;
		uint64_t next_index_vld :1;
		uint64_t reserved_11_15 :5;
		uint64_t prev_index :10;
		uint64_t reserved_26_63 :38;
#endif
	} s_sso_ientx_links_cn73xx;

	struct {
#ifdef __BIG_ENDIAN_BITFIELD
		uint64_t reserved_28_63 :36;
		uint64_t prev_index :12;
			/**< The previous entry in the tag chain. Unpredictable if the entry
			 is at the head of the list or the head of a conflicted tag chain. */
		uint64_t reserved_13_15 :3;
		uint64_t next_index_vld :1;
			/**< The NEXT_INDEX is valid. Unpredictable unless the entry is the
			 tail entry of an atomic tag chain. */
		uint64_t next_index :12;
			/**< The next entry in the tag chain or conflicted tag chain.
			 Unpredictable if the entry is at the tail of the list. */
#else
		uint64_t next_index :12;
		uint64_t next_index_vld :1;
		uint64_t reserved_13_15 :3;
		uint64_t prev_index :12;
		uint64_t reserved_28_63 :36;
#endif
	} s_sso_ientx_links_cn78xx;

	/**
	 * Result For POW Memory Load (get_des == 1)
	 */
	struct {
#ifdef __BIG_ENDIAN_BITFIELD
		uint64_t reserved_51_63:13;
		uint64_t fwd_index:11;
			/**< The next entry in the tag list connected to the descheduled head. */
		uint64_t grp:4; /**< The group of the POW entry. */
		uint64_t nosched:1; /**< The nosched bit for the POW entry. */
		uint64_t pend_switch:1; /**< There is a pending tag switch */
		uint64_t pend_type:2;
			/**< The next tag type for the new tag list when pend_switch is set. */
		uint64_t pend_tag:32;
			/**< The next tag for the new tag list when pend_switch is set. */
#else
		uint64_t pend_tag:32;
		uint64_t pend_type:2;
		uint64_t pend_switch:1;
		uint64_t nosched:1;
		uint64_t grp:4;
		uint64_t fwd_index:11;
		uint64_t reserved_51_63:13;
#endif
	} s_smemload2;
	/**
	 * Result For SSO Memory Load (opcode is ML_PENTAG)
	 */
	struct {
#ifdef __BIG_ENDIAN_BITFIELD
		uint64_t reserved_38_63:26;
		uint64_t pend_switch:1;
			/**< Set when there is a pending non-UNSCHEDULED SWTAG or SWTAG_FULL,
			 and the SSO entry has not left the list for the original tag. */
		uint64_t reserved_34_36:3;
		uint64_t pend_type:2;
			/**< The next tag type for the new tag list when pend_switch is set. */
		uint64_t pend_tag:32;
			/**< The next tag for the new tag list when pend_switch is set. */
#else
		uint64_t pend_tag:32;
		uint64_t pend_type:2;
		uint64_t reserved_34_36:3;
		uint64_t pend_switch:1;
		uint64_t reserved_38_63:26;
#endif
	} s_smemload2_cn68xx;

	struct {
#ifdef __BIG_ENDIAN_BITFIELD
		uint64_t pend_switch :1;
			/**< Set when there is a pending SWTAG, SWTAG_DESCHED, or SWTAG_FULL
		 	to ORDERED or ATOMIC. If the register read was issued after an indexed
		 	GET_WORK, the DESCHED portion of a SWTAG_DESCHED cannot still be pending. */
		uint64_t pend_get_work :1;  /**< Set when there is a pending GET_WORK. */
		uint64_t pend_get_work_wait :1;
			/**< When PEND_GET_WORK is set, indicates that the WAITW bit was set. */
		uint64_t pend_nosched :1;  /**< Set when nosched is desired and PEND_DESCHED is set. */
		uint64_t pend_nosched_clr :1;  /**< Set when there is a pending CLR_NSCHED. */
		uint64_t pend_desched :1;  /**< Set when there is a pending DESCHED or SWTAG_DESCHED. */
		uint64_t pend_alloc_we :1;  /**< Set when there is a pending ALLOC_WE. */
		uint64_t reserved_34_56 :23;
		uint64_t pend_tt :2;  /**< The tag type when PEND_SWITCH is set. */
		uint64_t pend_tag :32; /**< The tag when PEND_SWITCH is set. */
#else
		uint64_t pend_tag :32;
		uint64_t pend_tt :2;
		uint64_t reserved_34_56 :23;
		uint64_t pend_alloc_we :1;
		uint64_t pend_desched :1;
		uint64_t pend_nosched_clr :1;
		uint64_t pend_nosched :1;
		uint64_t pend_get_work_wait :1;
		uint64_t pend_get_work :1;
		uint64_t pend_switch :1;
#endif
	} s_sso_ppx_pendtag_cn78xx;
	/**
	 * Result For SSO Memory Load (opcode is ML_LINKS)
	 */
	struct {
#ifdef __BIG_ENDIAN_BITFIELD
		uint64_t reserved_24_63:40;
		uint64_t fwd_index:11;
			/**< The next entry in the tag list connected to the descheduled head. */
		uint64_t reserved_11_12:2;
		uint64_t next_index:11;
			/**< The next entry in the input, free, descheduled_head list
			(unpredicatble if entry is the tail of the list). */
#else
		uint64_t next_index:11;
		uint64_t reserved_11_12:2;
		uint64_t fwd_index:11;
		uint64_t reserved_24_63:40;
#endif
	} s_smemload3_cn68xx;

	/**
	 * Result For POW Index/Pointer Load (get_rmt == 0/get_des_get_tail == 0)
	 */
	struct {
#ifdef __BIG_ENDIAN_BITFIELD
		uint64_t reserved_52_63:12;
		uint64_t free_val:1;
			/**< Set when there is one or more POW entries on the free list. */
		uint64_t free_one:1;
			/**< - Set when there is exactly one POW entry on the free list. */
		uint64_t reserved_49:1;
		uint64_t free_head:11;
			/**< When free_val is set, indicates the first entry on the free list. */
		uint64_t reserved_37:1;
		uint64_t free_tail:11;
			/**< When free_val is set, indicates the last entry on the free list. */
		uint64_t loc_val:1;
			/**< Set when there is one or more POW entries on the input Q list
			 selected by qosgrp. */
		uint64_t loc_one:1;
			/**< Set when there is exactly one POW entry on the input Q list
			 selected by qosgrp. */
		uint64_t reserved_23:1;
		uint64_t loc_head:11;
			/**< When loc_val is set, indicates the first entry on the input Q list
			 selected by qosgrp. */
		uint64_t reserved_11:1;
		uint64_t loc_tail:11;
			/**< When loc_val is set, indicates the last entry on the input Q list
			 selected by qosgrp. */
#else
		uint64_t loc_tail:11;
		uint64_t reserved_11:1;
		uint64_t loc_head:11;
		uint64_t reserved_23:1;
		uint64_t loc_one:1;
		uint64_t loc_val:1;
		uint64_t free_tail:11;
		uint64_t reserved_37:1;
		uint64_t free_head:11;
		uint64_t reserved_49:1;
		uint64_t free_one:1;
		uint64_t free_val:1;
		uint64_t reserved_52_63:12;
#endif
	} sindexload0;
	/**
	 * Result for SSO Index/Pointer Load(opcode ==
	 * IPL_IQ/IPL_DESCHED/IPL_NOSCHED)
	 */
	struct {
#ifdef __BIG_ENDIAN_BITFIELD
		uint64_t reserved_28_63:36;
		uint64_t queue_val:1;
			/**< - If set, one or more valid entries are in the queue. */
		uint64_t queue_one:1;
			/**< - If set, exactly one valid entry is in the queue. */
		uint64_t reserved_24_25:2;
		uint64_t queue_head:11;
			/**< - Index of entry at the head of the queue. */
		uint64_t reserved_11_12:2;
		uint64_t queue_tail:11;
			/**< - Index of entry at the tail of the queue. */
#else
		uint64_t queue_tail:11;
		uint64_t reserved_11_12:2;
		uint64_t queue_head:11;
		uint64_t reserved_24_25:2;
		uint64_t queue_one:1;
		uint64_t queue_val:1;
		uint64_t reserved_28_63:36;
#endif
	} sindexload0_cn68xx;
	/**
	 * Result For POW Index/Pointer Load (get_rmt == 0/get_des_get_tail == 1)
	 */
	struct {
#ifdef __BIG_ENDIAN_BITFIELD
		uint64_t reserved_52_63:12;
		uint64_t nosched_val:1;
			/**< Set when there is one or more POW entries on the nosched list. */
		uint64_t nosched_one:1;
			/**< Set when there is exactly one POW entry on the nosched list. */
		uint64_t reserved_49:1;
		uint64_t nosched_head:11;
			/**< When nosched_val is set, indicates the first entry on the nosched list. */
		uint64_t reserved_37:1;
		uint64_t nosched_tail:11;
			/**< When nosched_val is set, indicates the last entry on the nosched list. */
		uint64_t des_val:1;
			/**< Set when there is one or more descheduled heads on the descheduled
			 list selected by qosgrp. */
		uint64_t des_one:1;
			/**< Set when there is exactly one descheduled head on the descheduled
			 list selected by qosgrp. */
		uint64_t reserved_23:1;
		uint64_t des_head:11;
			/**< When des_val is set, indicates the first descheduled head on
			 the descheduled list selected by qosgrp. */
		uint64_t reserved_11:1;
		uint64_t des_tail:11;
			/**< When des_val is set, indicates the last descheduled head on
			 the descheduled list selected by qosgrp. */
#else
		uint64_t des_tail:11;
		uint64_t reserved_11:1;
		uint64_t des_head:11;
		uint64_t reserved_23:1;
		uint64_t des_one:1;
		uint64_t des_val:1;
		uint64_t nosched_tail:11;
		uint64_t reserved_37:1;
		uint64_t nosched_head:11;
		uint64_t reserved_49:1;
		uint64_t nosched_one:1;
		uint64_t nosched_val:1;
		uint64_t reserved_52_63:12;
#endif
	} sindexload1;
	/**
	 * Result for SSO Index/Pointer Load(opcode == IPL_FREE0/IPL_FREE1/IPL_FREE2)
	 */
	struct {
#ifdef __BIG_ENDIAN_BITFIELD
		uint64_t reserved_60_63:4;
		uint64_t qnum_head:2; /**< - Subqueue with current head */
		uint64_t qnum_tail:2; /**< - Subqueue with current tail */
		uint64_t reserved_28_55:28;
		uint64_t queue_val:1;
			/**< - If set, one or more valid entries are in the queue. */
		uint64_t queue_one:1;
			/**< - If set, exactly one valid entry is in the queue. */
		uint64_t reserved_24_25:2;
		uint64_t queue_head:11;
			/**< - Index of entry at the head of the queue. */
		uint64_t reserved_11_12:2;
		uint64_t queue_tail:11;
			/**< - Index of entry at the tail of the queue. */
#else
		uint64_t queue_tail:11;
		uint64_t reserved_11_12:2;
		uint64_t queue_head:11;
		uint64_t reserved_24_25:2;
		uint64_t queue_one:1;
		uint64_t queue_val:1;
		uint64_t reserved_28_55:28;
		uint64_t qnum_tail:2;
		uint64_t qnum_head:2;
		uint64_t reserved_60_63:4;
#endif
	} sindexload1_cn68xx;
	/**
	 * Result For POW Index/Pointer Load (get_rmt == 1/get_des_get_tail == 0)
	 */
	struct {
#ifdef __BIG_ENDIAN_BITFIELD
		uint64_t reserved_39_63:25;
		uint64_t rmt_is_head:1;
			/**< Set when this DRAM list is the current head
			 (i.e. is the next to be reloaded when the POW hardware reloads
			 a POW entry from DRAM). The POW hardware alternates between the two DRAM
			 lists associated with a QOS level when it reloads work from DRAM
			 into the POW unit. */
		uint64_t rmt_val:1;
			/**< Set when the DRAM portion of the input Q list selected by qosgrp
			 contains one or more pieces of work. */
		uint64_t rmt_one:1;
			/**< Set when the DRAM portion of the input Q list selected by qosgrp
			 contains exactly one piece of work. */
		uint64_t rmt_head:36;
			/**< When rmt_val is set, indicates the first piece of work on the
			 DRAM input Q list selected by qosgrp. */
#else
		uint64_t rmt_head:36;
		uint64_t rmt_one:1;
		uint64_t rmt_val:1;
		uint64_t rmt_is_head:1;
		uint64_t reserved_39_63:25;
#endif
	} sindexload2;
	/**
	 * Result For POW Index/Pointer Load (get_rmt == 1/get_des_get_tail == 1)
	 */
	struct {
#ifdef __BIG_ENDIAN_BITFIELD
		uint64_t reserved_39_63:25;
		uint64_t rmt_is_head:1;
			/**< - Set when this DRAM list is the current head (i.e. is the next to
			 be reloaded when the POW hardware reloads a POW entry from DRAM).
			 The POW hardware alternates between the two DRAM lists associated with a QOS
			 level when it reloads work from DRAM into the POW unit. */
		uint64_t rmt_val:1;
			/**< - Set when the DRAM portion of the input Q list selected by qosgrp
			 contains one or more pieces of work. */
		uint64_t rmt_one:1;
			/**< - Set when the DRAM portion of the input Q list selected by qosgrp
			 contains exactly one piece of work. */
		uint64_t rmt_tail:36;
			/**< - When rmt_val is set, indicates the last piece of work on the DRAM
			 input Q list selected by qosgrp. */
#else
		uint64_t rmt_tail:36;
		uint64_t rmt_one:1;
		uint64_t rmt_val:1;
		uint64_t rmt_is_head:1;
		uint64_t reserved_39_63:25;
#endif
	} sindexload3;
	/**
	 * Response to NULL_RD request loads
	 */
	struct {
#ifdef __BIG_ENDIAN_BITFIELD
		uint64_t unused:62;
		uint64_t state:2;
			/**< Of type cvmx_pow_tag_type_t. state is one of the following:
			 - CVMX_POW_TAG_TYPE_ORDERED
			 - CVMX_POW_TAG_TYPE_ATOMIC
			 - CVMX_POW_TAG_TYPE_NULL
			 - CVMX_POW_TAG_TYPE_NULL_NULL */
#else
		uint64_t state:2;
		uint64_t unused:62;
#endif
	} s_null_rd;

} cvmx_pow_tag_load_resp_t;

typedef union {
	uint64_t u64;
	struct {
#ifdef __BIG_ENDIAN_BITFIELD
		uint64_t reserved_57_63:7;
		uint64_t index:11;
		uint64_t reserved_45:1;
		uint64_t grp:6;
		uint64_t head:1;
		uint64_t tail:1;
		uint64_t reserved_34_36:3;
		uint64_t tag_type:2;
		uint64_t tag:32;
#else
		uint64_t tag:32;
		uint64_t tag_type:2;
		uint64_t reserved_34_36:3;
		uint64_t tail:1;
		uint64_t head:1;
		uint64_t grp:6;
		uint64_t reserved_45:1;
		uint64_t index:11;
		uint64_t reserved_57_63:7;
#endif
	} s;
} cvmx_pow_sl_tag_resp_t;

/**
 * This structure describes the address used for stores to the POW.
 *  The store address is meaningful on stores to the POW.  The hardware assumes that an aligned
 *  64-bit store was used for all these stores.
 *  Note the assumption that the work queue entry is aligned on an 8-byte
 *  boundary (since the low-order 3 address bits must be zero).
 *  Note that not all fields are used by all operations.
 *
 *  NOTE: The following is the behavior of the pending switch bit at the PP
 *       for POW stores (i.e. when did<7:3> == 0xc)
 *     - did<2:0> == 0      => pending switch bit is set
 *     - did<2:0> == 1      => no affect on the pending switch bit
 *     - did<2:0> == 3      => pending switch bit is cleared
 *     - did<2:0> == 7      => no affect on the pending switch bit
 *     - did<2:0> == others => must not be used
 *     - No other loads/stores have an affect on the pending switch bit
 *     - The switch bus from POW can clear the pending switch bit
 *
 *  NOTE: did<2:0> == 2 is used by the HW for a special single-cycle ADDWQ command
 *  that only contains the pointer). SW must never use did<2:0> == 2.
 */
typedef union {
	uint64_t u64;
	struct {
#ifdef __BIG_ENDIAN_BITFIELD
		uint64_t mem_reg:2; /**< Memory region.  Should be CVMX_IO_SEG in most cases */
		uint64_t reserved_49_61:13; /**< Must be zero */
		uint64_t is_io:1; /**< Must be one */
		uint64_t did:8; /**< Device ID of POW. Note that different sub-dids are used. */
		uint64_t addr:40; /**< Address field. addr<2:0> must be zero */
#else
		uint64_t addr:40;
		uint64_t did:8;
		uint64_t is_io:1;
		uint64_t reserved_49_61:13;
		uint64_t mem_reg:2;
#endif
	} stag;
} cvmx_pow_tag_store_addr_t; /* FIXME- this type is unused */

/**
 * Decode of the store data when an IOBDMA SENDSINGLE is sent to POW
 */
typedef union {
	uint64_t u64;
	struct {
		CVMX_BITFIELD_FIELD(uint64_t scraddr:8,
			/**< the (64-bit word) location in scratchpad to write to (if len != 0) */
		CVMX_BITFIELD_FIELD(uint64_t len:8,
			/**< the number of words in the response (0 => no response) */
		CVMX_BITFIELD_FIELD(uint64_t did:8,
			/**< the ID of the device on the non-coherent bus */
		CVMX_BITFIELD_FIELD(uint64_t unused:36,
		CVMX_BITFIELD_FIELD(uint64_t wait:1,
			/**< if set, don't return load response until work is available */
		CVMX_BITFIELD_FIELD(uint64_t unused2:3,
		))))));
	} s;
	struct {
		CVMX_BITFIELD_FIELD(uint64_t scraddr:8,
			/**< the (64-bit word) location in scratchpad to write to (if len != 0) */
		CVMX_BITFIELD_FIELD(uint64_t len:8,
			/**< the number of words in the response (0 => no response) */
		CVMX_BITFIELD_FIELD(uint64_t did:8,
			/**< the ID of the device on the non-coherent bus */
		CVMX_BITFIELD_FIELD(uint64_t node:4,
			/**< OCI node number, should always be local node */
		CVMX_BITFIELD_FIELD(uint64_t unused1:4,
		CVMX_BITFIELD_FIELD(uint64_t indexed:1,
		CVMX_BITFIELD_FIELD(uint64_t grouped:1,
		CVMX_BITFIELD_FIELD(uint64_t rtngrp:1,
		CVMX_BITFIELD_FIELD(uint64_t unused2:13,
		CVMX_BITFIELD_FIELD(uint64_t index_grp_mask:12,
		CVMX_BITFIELD_FIELD(uint64_t wait:1,
			/**< if set, don't return load response until work is available */
		CVMX_BITFIELD_FIELD(uint64_t unused3:3,
		))))))))))));
	} s_cn78xx;
} cvmx_pow_iobdma_store_t;

/* CSR typedefs have been moved to cvmx-pow-defs.h */

/*enum for group priority parameters which needs modification*/
enum cvmx_sso_group_modify_mask{
	CVMX_SSO_MODIFY_GROUP_PRIORITY = 0x01,
	CVMX_SSO_MODIFY_GROUP_WEIGHT = 0x02,
	CVMX_SSO_MODIFY_GROUP_AFFINITY = 0x04
};

/**
 * @INTERNAL
 * Return the number of SSO groups for a given SoC model
 */
static inline unsigned cvmx_sso_num_xgrp(void)
{
	if (OCTEON_IS_MODEL(OCTEON_CN78XX))
		return 256;
	if (OCTEON_IS_MODEL(OCTEON_CNF75XX))
		return 64;
	if (OCTEON_IS_MODEL(OCTEON_CN73XX))
		return 64;
	cvmx_printf("ERROR: %s: Unknown model\n", __func__);
	return 0;
}

/**
 * @INTERNAL
 * Return the number of POW groups on current model.
 * In case of CN78XX/CN73XX this is the number of equivalent
 * "legacy groups" on the chip when it is used in backward
 * compatible mode.
 */
static inline unsigned cvmx_pow_num_groups(void)
{
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE))
		return cvmx_sso_num_xgrp() >> 3;
	else if (octeon_has_feature(OCTEON_FEATURE_CN68XX_WQE))
		return 64;
	else
		return 16;
}

/**
 * @INTERNAL
 * Return the number of mask-set registers.
 */
static inline unsigned cvmx_sso_num_maskset(void)
{
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE))
		return 2;
	else
		return 1;
}

/**
 * Get the POW tag for this core. This returns the current
 * tag type, tag, group, and POW entry index associated with
 * this core. Index is only valid if the tag type isn't NULL_NULL.
 * If a tag switch is pending this routine returns the tag before
 * the tag switch, not after.
 *
 * @return Current tag
 */
static inline cvmx_pow_tag_info_t cvmx_pow_get_current_tag(void)
{
	cvmx_pow_load_addr_t load_addr;
	cvmx_pow_tag_info_t result;

	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		cvmx_sso_sl_ppx_tag_t sl_ppx_tag;
		cvmx_xgrp_t xgrp;
		int node, core;

		CVMX_SYNCS;
		node = cvmx_get_node_num();
		core = cvmx_get_local_core_num();
		sl_ppx_tag.u64 = cvmx_read_csr_node(node, CVMX_SSO_SL_PPX_TAG(core));
		result.index = sl_ppx_tag.s.index;
		result.tag_type = sl_ppx_tag.s.tt;
		result.tag      = sl_ppx_tag.s.tag;

		/* Get native XGRP value */
		xgrp.xgrp =  sl_ppx_tag.s.grp;

		/* Return legacy style group 0..15 */
		result.grp = xgrp.group;
	} else if (octeon_has_feature(OCTEON_FEATURE_CN68XX_WQE)) {
		cvmx_pow_sl_tag_resp_t load_resp;
		load_addr.u64 = 0;
		load_addr.sstatus_cn68xx.mem_region = CVMX_IO_SEG;
		load_addr.sstatus_cn68xx.is_io = 1;
		load_addr.sstatus_cn68xx.did = CVMX_OCT_DID_TAG_TAG5;
		load_addr.sstatus_cn68xx.coreid = cvmx_get_core_num();
		load_addr.sstatus_cn68xx.opcode = 3;
		load_resp.u64 = cvmx_read_csr(load_addr.u64);
		result.grp = load_resp.s.grp;
		result.index = load_resp.s.index;
		result.tag_type = load_resp.s.tag_type;
		result.tag = load_resp.s.tag;
	} else {
		cvmx_pow_tag_load_resp_t load_resp;
		load_addr.u64 = 0;
		load_addr.sstatus.mem_region = CVMX_IO_SEG;
		load_addr.sstatus.is_io = 1;
		load_addr.sstatus.did = CVMX_OCT_DID_TAG_TAG1;
		load_addr.sstatus.coreid = cvmx_get_core_num();
		load_addr.sstatus.get_cur = 1;
		load_resp.u64 = cvmx_read_csr(load_addr.u64);
		result.grp = load_resp.s_sstatus2.grp;
		result.index = load_resp.s_sstatus2.index;
		result.tag_type = load_resp.s_sstatus2.tag_type;
		result.tag = load_resp.s_sstatus2.tag;
	}
	return result;
}

/**
 * Get the POW WQE for this core. This returns the work queue
 * entry currently associated with this core.
 *
 * @return WQE pointer
 */
static inline cvmx_wqe_t *cvmx_pow_get_current_wqp(void)
{
	cvmx_pow_load_addr_t load_addr;
	cvmx_pow_tag_load_resp_t load_resp;

	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		cvmx_sso_sl_ppx_wqp_t sso_wqp;
		int node = cvmx_get_node_num();
		int core = cvmx_get_local_core_num();
		sso_wqp.u64 = cvmx_read_csr_node(node, CVMX_SSO_SL_PPX_WQP(core));
		if (sso_wqp.s.wqp)
			return (cvmx_wqe_t *) cvmx_phys_to_ptr(sso_wqp.s.wqp);
		return (cvmx_wqe_t *) 0;
	}
	if (octeon_has_feature(OCTEON_FEATURE_CN68XX_WQE)) {
		load_addr.u64 = 0;
		load_addr.sstatus_cn68xx.mem_region = CVMX_IO_SEG;
		load_addr.sstatus_cn68xx.is_io = 1;
		load_addr.sstatus_cn68xx.did = CVMX_OCT_DID_TAG_TAG5;
		load_addr.sstatus_cn68xx.coreid = cvmx_get_core_num();
		load_addr.sstatus_cn68xx.opcode = 4;
		load_resp.u64 = cvmx_read_csr(load_addr.u64);
		if (load_resp.s_sstatus3_cn68xx.wqp)
			return (cvmx_wqe_t *) cvmx_phys_to_ptr(load_resp.s_sstatus3_cn68xx.wqp);
		else
			return (cvmx_wqe_t *) 0;
	} else {
		load_addr.u64 = 0;
		load_addr.sstatus.mem_region = CVMX_IO_SEG;
		load_addr.sstatus.is_io = 1;
		load_addr.sstatus.did = CVMX_OCT_DID_TAG_TAG1;
		load_addr.sstatus.coreid = cvmx_get_core_num();
		load_addr.sstatus.get_cur = 1;
		load_addr.sstatus.get_wqp = 1;
		load_resp.u64 = cvmx_read_csr(load_addr.u64);
		return (cvmx_wqe_t *) cvmx_phys_to_ptr(load_resp.s_sstatus4.wqp);
	}
}

/**
 * @INTERNAL
 * Print a warning if a tag switch is pending for this core
 *
 * @param function Function name checking for a pending tag switch
 */
static inline void __cvmx_pow_warn_if_pending_switch(const char *function)
{
	uint64_t switch_complete;
	CVMX_MF_CHORD(switch_complete);
	cvmx_warn_if(!switch_complete, "%s called with tag switch in progress\n", function);
}

/**
 * Waits for a tag switch to complete by polling the completion bit.
 * Note that switches to NULL complete immediately and do not need
 * to be waited for.
 */
static inline void cvmx_pow_tag_sw_wait(void)
{
	const uint64_t MAX_CYCLES = 1ull << 31;
	uint64_t switch_complete;
	uint64_t start_cycle;

	if (CVMX_ENABLE_POW_CHECKS)
		start_cycle = cvmx_get_cycle();

	while (1) {
		CVMX_MF_CHORD(switch_complete);
		if (cvmx_likely(switch_complete))
			break;
		if (CVMX_ENABLE_POW_CHECKS) {
			if (cvmx_unlikely(cvmx_get_cycle() > (start_cycle + MAX_CYCLES))) {
				cvmx_dprintf("WARNING: %s: Tag switch is taking a long time, "
					"possible deadlock\n", __func__);
				start_cycle += MAX_CYCLES - 1;
			}
		}
	}
}

/**
 * Synchronous work request.  Requests work from the POW.
 * This function does NOT wait for previous tag switches to complete,
 * so the caller must ensure that there is not a pending tag switch.
 *
 * @param wait   When set, call stalls until work becomes avaiable, or times out.
 *               If not set, returns immediately.
 *
 * @return Returns the WQE pointer from POW. Returns NULL if no work was available.
 */
static inline cvmx_wqe_t *cvmx_pow_work_request_sync_nocheck(cvmx_pow_wait_t wait)
{
	cvmx_pow_load_addr_t ptr;
	cvmx_pow_tag_load_resp_t result;

	if (CVMX_ENABLE_POW_CHECKS)
		__cvmx_pow_warn_if_pending_switch(__func__);

	ptr.u64 = 0;
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		ptr.swork_78xx.node = cvmx_get_node_num();
		ptr.swork_78xx.mem_region = CVMX_IO_SEG;
		ptr.swork_78xx.is_io = 1;
		ptr.swork_78xx.did = CVMX_OCT_DID_TAG_SWTAG;
		ptr.swork_78xx.wait = wait;
	} else {
		ptr.swork.mem_region = CVMX_IO_SEG;
		ptr.swork.is_io = 1;
		ptr.swork.did = CVMX_OCT_DID_TAG_SWTAG;
		ptr.swork.wait = wait;
	}
	result.u64 = cvmx_read_csr(ptr.u64);
	if (result.s_work.no_work)
		return NULL;
	else
		return (cvmx_wqe_t *) cvmx_phys_to_ptr(result.s_work.addr);
}

/**
 * Synchronous work request.  Requests work from the POW.
 * This function waits for any previous tag switch to complete before
 * requesting the new work.
 *
 * @param wait   When set, call stalls until work becomes avaiable, or times out.
 *               If not set, returns immediately.
 *
 * @return Returns the WQE pointer from POW. Returns NULL if no work was available.
 */
static inline cvmx_wqe_t *cvmx_pow_work_request_sync(cvmx_pow_wait_t wait)
{
	/* Must not have a switch pending when requesting work */
	cvmx_pow_tag_sw_wait();
	return (cvmx_pow_work_request_sync_nocheck(wait));
}

/**
 * Synchronous null_rd request.  Requests a switch out of NULL_NULL POW state.
 * This function waits for any previous tag switch to complete before
 * requesting the null_rd.
 *
 * @return Returns the POW state of type cvmx_pow_tag_type_t.
 */
static inline cvmx_pow_tag_type_t cvmx_pow_work_request_null_rd(void)
{
	cvmx_pow_load_addr_t ptr;
	cvmx_pow_tag_load_resp_t result;

	/* Must not have a switch pending when requesting work */
	cvmx_pow_tag_sw_wait();

	ptr.u64 = 0;
	ptr.snull_rd.mem_region = CVMX_IO_SEG;
	ptr.snull_rd.is_io = 1;
	ptr.snull_rd.did = CVMX_OCT_DID_TAG_NULL_RD;

	result.u64 = cvmx_read_csr(ptr.u64);
	return (cvmx_pow_tag_type_t)result.s_null_rd.state;
}

/**
 * Asynchronous work request.
 * Work is requested from the POW unit, and should later be checked with
 * function cvmx_pow_work_response_async.
 * This function does NOT wait for previous tag switches to complete,
 * so the caller must ensure that there is not a pending tag switch.
 *
 * @param scr_addr Scratch memory address that response will be returned to,
 *     which is either a valid WQE, or a response with the invalid bit set.
 *     Byte address, must be 8 byte aligned.
 * @param wait 1 to cause response to wait for work to become available (or timeout)
 *             0 to cause response to return immediately
 */
static inline void cvmx_pow_work_request_async_nocheck(int scr_addr,
	cvmx_pow_wait_t wait)
{
	cvmx_pow_iobdma_store_t data;

	if (CVMX_ENABLE_POW_CHECKS)
		__cvmx_pow_warn_if_pending_switch(__func__);

	/* scr_addr must be 8 byte aligned */
	data.u64 = 0;
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		data.s_cn78xx.node = cvmx_get_node_num();
		data.s_cn78xx.scraddr = scr_addr >> 3;
		data.s_cn78xx.len = 1;
		data.s_cn78xx.did = CVMX_OCT_DID_TAG_SWTAG;
		data.s_cn78xx.wait = wait;
	} else {
		data.s.scraddr = scr_addr >> 3;
		data.s.len = 1;
		data.s.did = CVMX_OCT_DID_TAG_SWTAG;
		data.s.wait = wait;
	}
	cvmx_send_single(data.u64);
}

/**
 * Asynchronous work request.
 * Work is requested from the POW unit, and should later be checked with
 * function cvmx_pow_work_response_async.
 * This function waits for any previous tag switch to complete before
 * requesting the new work.
 *
 * @param scr_addr Scratch memory address that response will be returned to,
 *     which is either a valid WQE, or a response with the invalid bit set.
 *     Byte address, must be 8 byte aligned.
 * @param wait 1 to cause response to wait for work to become available (or timeout)
 *             0 to cause response to return immediately
 */
static inline void cvmx_pow_work_request_async(int scr_addr, cvmx_pow_wait_t wait)
{
	/* Must not have a switch pending when requesting work */
	cvmx_pow_tag_sw_wait();
	cvmx_pow_work_request_async_nocheck(scr_addr, wait);
}

/**
 * Gets result of asynchronous work request.  Performs a IOBDMA sync
 * to wait for the response.
 *
 * @param scr_addr Scratch memory address to get result from
 *                  Byte address, must be 8 byte aligned.
 * @return Returns the WQE from the scratch register, or NULL if no work was available.
 */
static inline cvmx_wqe_t *cvmx_pow_work_response_async(int scr_addr)
{
	cvmx_pow_tag_load_resp_t result;

	CVMX_SYNCIOBDMA;
	result.u64 = cvmx_scratch_read64(scr_addr);
	if (result.s_work.no_work)
		return NULL;
	else
		return (cvmx_wqe_t *) cvmx_phys_to_ptr(result.s_work.addr);
}

/**
 * Checks if a work queue entry pointer returned by a work
 * request is valid.  It may be invalid due to no work
 * being available or due to a timeout.
 *
 * @param wqe_ptr pointer to a work queue entry returned by the POW
 *
 * @return 0 if pointer is valid
 *         1 if invalid (no work was returned)
 */
static inline uint64_t cvmx_pow_work_invalid(cvmx_wqe_t * wqe_ptr)
{
	return (wqe_ptr == NULL); /* FIXME: improve */
}

/**
 * Starts a tag switch to the provided tag value and tag type.  Completion for
 * the tag switch must be checked for separately.
 * This function does NOT update the
 * work queue entry in dram to match tag value and type, so the application must
 * keep track of these if they are important to the application.
 * This tag switch command must not be used for switches to NULL, as the tag
 * switch pending bit will be set by the switch request, but never cleared by the
 * hardware.
 *
 * NOTE: This should not be used when switching from a NULL tag.  Use
 * cvmx_pow_tag_sw_full() instead.
 *
 * This function does no checks, so the caller must ensure that any previous tag
 * switch has completed.
 *
 * @param tag      new tag value
 * @param tag_type new tag type (ordered or atomic)
 */
static inline void cvmx_pow_tag_sw_nocheck(uint32_t tag, cvmx_pow_tag_type_t tag_type)
{
	union cvmx_pow_tag_req_addr ptr;
	cvmx_pow_tag_req_t tag_req;

	if (CVMX_ENABLE_POW_CHECKS) {
		cvmx_pow_tag_info_t current_tag;
		__cvmx_pow_warn_if_pending_switch(__func__);
		current_tag = cvmx_pow_get_current_tag();
		cvmx_warn_if(current_tag.tag_type == CVMX_POW_TAG_TYPE_NULL_NULL,
			"%s called with NULL_NULL tag\n", __func__);
		cvmx_warn_if(current_tag.tag_type == CVMX_POW_TAG_TYPE_NULL,
			"%s called with NULL tag\n", __func__);
		cvmx_warn_if((current_tag.tag_type == tag_type) && (current_tag.tag == tag),
			"%s called to perform a tag switch to the same tag\n", __func__);
		cvmx_warn_if(tag_type == CVMX_POW_TAG_TYPE_NULL,
			"%s called to perform a tag switch to NULL. "
			"Use cvmx_pow_tag_sw_null() instead\n", __func__);
	}
	/* Note that WQE in DRAM is not updated here, as the POW does not read from DRAM
	 * once the WQE is in flight.  See hardware manual for complete details.
	 * It is the application's responsibility to keep track of the current tag
	 * value if that is important.
	 */
	tag_req.u64 = 0;
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		tag_req.s_cn78xx_other.op   = CVMX_POW_TAG_OP_SWTAG;
		tag_req.s_cn78xx_other.type = tag_type;
	} else if (octeon_has_feature(OCTEON_FEATURE_CN68XX_WQE)) {
		tag_req.s_cn68xx_other.op = CVMX_POW_TAG_OP_SWTAG;
		tag_req.s_cn68xx_other.tag = tag;
		tag_req.s_cn68xx_other.type = tag_type;
	} else {
		tag_req.s_cn38xx.op = CVMX_POW_TAG_OP_SWTAG;
		tag_req.s_cn38xx.tag = tag;
		tag_req.s_cn38xx.type = tag_type;
	}
	ptr.u64 = 0;
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		ptr.s_cn78xx.mem_region = CVMX_IO_SEG;
		ptr.s_cn78xx.is_io = 1;
		ptr.s_cn78xx.did = CVMX_OCT_DID_TAG_SWTAG;
		ptr.s_cn78xx.node = cvmx_get_node_num();
		ptr.s_cn78xx.tag  = tag;
	} else {
		ptr.s.mem_region = CVMX_IO_SEG;
		ptr.s.is_io = 1;
		ptr.s.did = CVMX_OCT_DID_TAG_SWTAG;
	}
	/* Once this store arrives at POW, it will attempt the switch
	   software must wait for the switch to complete separately */
	cvmx_write_io(ptr.u64, tag_req.u64);
}

/**
 * Starts a tag switch to the provided tag value and tag type.  Completion for
 * the tag switch must be checked for separately.
 * This function does NOT update the
 * work queue entry in dram to match tag value and type, so the application must
 * keep track of these if they are important to the application.
 * This tag switch command must not be used for switches to NULL, as the tag
 * switch pending bit will be set by the switch request, but never cleared by the
 * hardware.
 *
 * NOTE: This should not be used when switching from a NULL tag.  Use
 * cvmx_pow_tag_sw_full() instead.
 *
 * This function waits for any previous tag switch to complete, and also
 * displays an error on tag switches to NULL.
 *
 * @param tag      new tag value
 * @param tag_type new tag type (ordered or atomic)
 */
static inline void cvmx_pow_tag_sw(uint32_t tag, cvmx_pow_tag_type_t tag_type)
{
	/* Note that WQE in DRAM is not updated here, as the POW does not read from DRAM
	 * once the WQE is in flight.  See hardware manual for complete details.
	 * It is the application's responsibility to keep track of the current tag
	 * value if that is important.
	 */
	/* Ensure that there is not a pending tag switch, as a tag switch cannot be started
	 ** if a previous switch is still pending.  */
	cvmx_pow_tag_sw_wait();
	cvmx_pow_tag_sw_nocheck(tag, tag_type);
}

/**
 * Starts a tag switch to the provided tag value and tag type.  Completion for
 * the tag switch must be checked for separately.
 * This function does NOT update the
 * work queue entry in dram to match tag value and type, so the application must
 * keep track of these if they are important to the application.
 * This tag switch command must not be used for switches to NULL, as the tag
 * switch pending bit will be set by the switch request, but never cleared by the
 * hardware.
 *
 * This function must be used for tag switches from NULL.
 *
 * This function does no checks, so the caller must ensure that any previous tag
 * switch has completed.
 *
 * @param wqp      pointer to work queue entry to submit.  This entry is updated to match the other parameters
 * @param tag      tag value to be assigned to work queue entry
 * @param tag_type type of tag
 * @param group      group value for the work queue entry.
 */
static inline void cvmx_pow_tag_sw_full_nocheck(cvmx_wqe_t * wqp, uint32_t tag,
	cvmx_pow_tag_type_t tag_type, uint64_t group)
{
	union cvmx_pow_tag_req_addr ptr;
	cvmx_pow_tag_req_t tag_req;
	unsigned node = cvmx_get_node_num();
	uint64_t wqp_phys = cvmx_ptr_to_phys(wqp);

	if (CVMX_ENABLE_POW_CHECKS) {
		cvmx_pow_tag_info_t current_tag;
		__cvmx_pow_warn_if_pending_switch(__func__);
		current_tag = cvmx_pow_get_current_tag();
		cvmx_warn_if(current_tag.tag_type == CVMX_POW_TAG_TYPE_NULL_NULL,
			"%s called with NULL_NULL tag\n", __func__);
		cvmx_warn_if((current_tag.tag_type == tag_type) && (current_tag.tag == tag),
			"%s called to perform a tag switch to the same tag\n", __func__);
		cvmx_warn_if(tag_type == CVMX_POW_TAG_TYPE_NULL,
			"%s called to perform a tag switch to NULL. "
			"Use cvmx_pow_tag_sw_null() instead\n", __func__);
		if ((wqp != cvmx_phys_to_ptr(0x80)) && cvmx_pow_get_current_wqp())
			cvmx_warn_if(wqp != cvmx_pow_get_current_wqp(),
				"%s passed WQE(%p) doesn't match the address in the POW(%p)\n",
				__func__, wqp, cvmx_pow_get_current_wqp());
	}
	/* Note that WQE in DRAM is not updated here, as the POW does not read from DRAM
	 * once the WQE is in flight.  See hardware manual for complete details.
	 * It is the application's responsibility to keep track of the current tag
	 * value if that is important.
	 */
	tag_req.u64 = 0;
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		unsigned xgrp;

		if(wqp_phys!= 0x80) {
			/* If WQE is valid, use its XGRP:
			 * WQE GRP is 10 bits, and is mapped
			 * to legacy GRP + QoS, includes node number.
			 */
			xgrp = wqp->word1.cn78xx.grp;
			/* Use XGRP[node] too */
			node = xgrp >> 8;
			/* Modify XGRP with legacy group # from arg */
			xgrp &= ~0xf8;
			xgrp |= 0xf8 & (group << 3);

		} else {
			/* If no WQE, build XGRP with QoS=0 and current node */
			xgrp = group << 3;
			xgrp |= node << 8;
		}
		tag_req.s_cn78xx_other.op = CVMX_POW_TAG_OP_SWTAG_FULL;
		tag_req.s_cn78xx_other.type = tag_type;
		tag_req.s_cn78xx_other.grp = xgrp;
		tag_req.s_cn78xx_other.wqp = wqp_phys;
	}
	else if (octeon_has_feature(OCTEON_FEATURE_CN68XX_WQE)) {
		tag_req.s_cn68xx_other.op = CVMX_POW_TAG_OP_SWTAG_FULL;
		tag_req.s_cn68xx_other.tag = tag;
		tag_req.s_cn68xx_other.type = tag_type;
		tag_req.s_cn68xx_other.grp = group;
	} else {
		tag_req.s_cn38xx.op = CVMX_POW_TAG_OP_SWTAG_FULL;
		tag_req.s_cn38xx.tag = tag;
		tag_req.s_cn38xx.type = tag_type;
		tag_req.s_cn38xx.grp = group;
	}
	ptr.u64 = 0;
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		ptr.s_cn78xx.mem_region = CVMX_IO_SEG;
		ptr.s_cn78xx.is_io = 1;
		ptr.s_cn78xx.did = CVMX_OCT_DID_TAG_SWTAG;
		ptr.s_cn78xx.node = node;
		ptr.s_cn78xx.tag  = tag;
	} else {
		ptr.s.mem_region = CVMX_IO_SEG;
		ptr.s.is_io = 1;
		ptr.s.did = CVMX_OCT_DID_TAG_SWTAG;
		ptr.s.addr = wqp_phys;
	}
	/* Once this store arrives at POW, it will attempt the switch
	   software must wait for the switch to complete separately */
	cvmx_write_io(ptr.u64, tag_req.u64);
}

/**
 * Starts a tag switch to the provided tag value and tag type.
 * Completion for the tag switch must be checked for separately.
 * This function does NOT update the work queue entry in dram to match tag value
 * and type, so the application must keep track of these if they are important
 * to the application. This tag switch command must not be used for switches
 * to NULL, as the tag switch pending bit will be set by the switch request,
 * but never cleared by the hardware.
 *
 * This function must be used for tag switches from NULL.
 *
 * This function waits for any pending tag switches to complete
 * before requesting the tag switch.
 *
 * @param wqp      Pointer to work queue entry to submit.
 *     This entry is updated to match the other parameters
 * @param tag      Tag value to be assigned to work queue entry
 * @param tag_type Type of tag
 * @param group    Group value for the work queue entry.
 */
static inline void cvmx_pow_tag_sw_full(cvmx_wqe_t * wqp, uint32_t tag,
	cvmx_pow_tag_type_t tag_type, uint64_t group)
{
	/* Ensure that there is not a pending tag switch, as a tag switch cannot
	 * be started if a previous switch is still pending. */
	cvmx_pow_tag_sw_wait();
	cvmx_pow_tag_sw_full_nocheck(wqp, tag, tag_type, group);
}

/**
 * Switch to a NULL tag, which ends any ordering or
 * synchronization provided by the POW for the current
 * work queue entry.  This operation completes immediately,
 * so completion should not be waited for.
 * This function does NOT wait for previous tag switches to complete,
 * so the caller must ensure that any previous tag switches have completed.
 */
static inline void cvmx_pow_tag_sw_null_nocheck(void)
{
	union cvmx_pow_tag_req_addr ptr;
	cvmx_pow_tag_req_t tag_req;

	if (CVMX_ENABLE_POW_CHECKS) {
		cvmx_pow_tag_info_t current_tag;
		__cvmx_pow_warn_if_pending_switch(__func__);
		current_tag = cvmx_pow_get_current_tag();
		cvmx_warn_if(current_tag.tag_type == CVMX_POW_TAG_TYPE_NULL_NULL,
			"%s called with NULL_NULL tag\n", __func__);
		cvmx_warn_if(current_tag.tag_type == CVMX_POW_TAG_TYPE_NULL,
			"%s called when we already have a NULL tag\n", __func__);
	}
	tag_req.u64 = 0;
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		tag_req.s_cn78xx_other.op = CVMX_POW_TAG_OP_SWTAG;
		tag_req.s_cn78xx_other.type = CVMX_POW_TAG_TYPE_NULL;
	}
	else if (octeon_has_feature(OCTEON_FEATURE_CN68XX_WQE)) {
		tag_req.s_cn68xx_other.op = CVMX_POW_TAG_OP_SWTAG;
		tag_req.s_cn68xx_other.type = CVMX_POW_TAG_TYPE_NULL;
	} else {
		tag_req.s_cn38xx.op = CVMX_POW_TAG_OP_SWTAG;
		tag_req.s_cn38xx.type = CVMX_POW_TAG_TYPE_NULL;
	}
	ptr.u64 = 0;
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		ptr.s_cn78xx.mem_region = CVMX_IO_SEG;
		ptr.s_cn78xx.is_io = 1;
		ptr.s_cn78xx.did = CVMX_OCT_DID_TAG_TAG1;
		ptr.s_cn78xx.node = cvmx_get_node_num();
	} else {
		ptr.s.mem_region = CVMX_IO_SEG;
		ptr.s.is_io = 1;
		ptr.s.did = CVMX_OCT_DID_TAG_TAG1;
	}
	cvmx_write_io(ptr.u64, tag_req.u64);
}

/**
 * Switch to a NULL tag, which ends any ordering or
 * synchronization provided by the POW for the current
 * work queue entry.  This operation completes immediatly,
 * so completion should not be waited for.
 * This function waits for any pending tag switches to complete
 * before requesting the switch to NULL.
 */
static inline void cvmx_pow_tag_sw_null(void)
{
	/* Ensure that there is not a pending tag switch, as a tag switch cannot
	 * be started if a previous switch is still pending. */
	cvmx_pow_tag_sw_wait();
	cvmx_pow_tag_sw_null_nocheck();
}

/**
 * Submits work to an input queue.
 * This function updates the work queue entry in DRAM to match the arguments given.
 * Note that the tag provided is for the work queue entry submitted, and
 * is unrelated to the tag that the core currently holds.
 *
 * @param wqp      pointer to work queue entry to submit.
 *                 This entry is updated to match the other parameters
 * @param tag      tag value to be assigned to work queue entry
 * @param tag_type type of tag
 * @param qos      Input queue to add to.
 * @param grp      group value for the work queue entry.
 */
static inline void cvmx_pow_work_submit(cvmx_wqe_t * wqp, uint32_t tag,
	cvmx_pow_tag_type_t tag_type, uint64_t qos, uint64_t grp)
{
	union cvmx_pow_tag_req_addr ptr;
	cvmx_pow_tag_req_t tag_req;

	tag_req.u64 = 0;
	ptr.u64 = 0;

	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		unsigned node = cvmx_get_node_num();
		unsigned xgrp;

		xgrp = (grp & 0x1f) << 3 ;
		xgrp |= (qos & 7);
		xgrp |= 0x300 & (node << 8);

		wqp->word1.cn78xx.rsvd_0 = 0;
		wqp->word1.cn78xx.rsvd_1 = 0;
		wqp->word1.cn78xx.tag = tag;
		wqp->word1.cn78xx.tag_type = tag_type;
		wqp->word1.cn78xx.grp = xgrp;
		CVMX_SYNCWS;

		tag_req.s_cn78xx_other.op = CVMX_POW_TAG_OP_ADDWQ;
		tag_req.s_cn78xx_other.type = tag_type;
		tag_req.s_cn78xx_other.wqp = cvmx_ptr_to_phys(wqp);
		tag_req.s_cn78xx_other.grp = xgrp;

		ptr.s_cn78xx.did = 0x66; //CVMX_OCT_DID_TAG_TAG6;
		ptr.s_cn78xx.mem_region = CVMX_IO_SEG;
		ptr.s_cn78xx.is_io = 1;
		ptr.s_cn78xx.node = node;
		ptr.s_cn78xx.tag = tag;
	} else if (octeon_has_feature(OCTEON_FEATURE_CN68XX_WQE)) {
		/* Reset all reserved bits */
		wqp->word1.cn68xx.zero_0 = 0;
		wqp->word1.cn68xx.zero_1 = 0;
		wqp->word1.cn68xx.zero_2 = 0;
		wqp->word1.cn68xx.qos = qos;
		wqp->word1.cn68xx.grp = grp;

		wqp->word1.tag = tag;
		wqp->word1.tag_type = tag_type;

		tag_req.s_cn68xx_add.op = CVMX_POW_TAG_OP_ADDWQ;
		tag_req.s_cn68xx_add.type = tag_type;
		tag_req.s_cn68xx_add.tag = tag;
		tag_req.s_cn68xx_add.qos = qos;
		tag_req.s_cn68xx_add.grp = grp;

		ptr.s.mem_region = CVMX_IO_SEG;
		ptr.s.is_io = 1;
		ptr.s.did = CVMX_OCT_DID_TAG_TAG1;
		ptr.s.addr = cvmx_ptr_to_phys(wqp);
	} else {
		/* Reset all reserved bits */
		wqp->word1.cn38xx.zero_2 = 0;
		wqp->word1.cn38xx.qos = qos;
		wqp->word1.cn38xx.grp = grp;

		wqp->word1.tag = tag;
		wqp->word1.tag_type = tag_type;

		tag_req.s_cn38xx.op = CVMX_POW_TAG_OP_ADDWQ;
		tag_req.s_cn38xx.type = tag_type;
		tag_req.s_cn38xx.tag = tag;
		tag_req.s_cn38xx.qos = qos;
		tag_req.s_cn38xx.grp = grp;

		ptr.s.mem_region = CVMX_IO_SEG;
		ptr.s.is_io = 1;
		ptr.s.did = CVMX_OCT_DID_TAG_TAG1;
		ptr.s.addr = cvmx_ptr_to_phys(wqp);
	}
	/* SYNC write to memory before the work submit.
	 * This is necessary as POW may read values from DRAM at this time */
	CVMX_SYNCWS;
	cvmx_write_io(ptr.u64, tag_req.u64);
}

/**
 * This function sets the group mask for a core.  The group mask
 * indicates which groups each core will accept work from. There are
 * 16 groups.
 *
 * @param core_num   core to apply mask to
 * @param mask   Group mask, one bit for up to 64 groups.
 *               Each 1 bit in the mask enables the core to accept work from
 *               the corresponding group.
 *               The CN68XX supports 64 groups, earlier models only support
 *               16 groups.
 *
 * The CN78XX in backwards compatibility mode allows up to 32 groups,
 * so the 'mask' argument has one bit for every of the legacy
 * groups, and a '1' in the mask causes a total of 8 groups
 * which share the legacy group numbher and 8 qos levels,
 * to be enabled for the calling processor core.
 * A '0' in the mask will disable the current core
 * from receiving work from the associated group.
 */
static inline void cvmx_pow_set_group_mask(uint64_t core_num, uint64_t mask)
{
	uint64_t valid_mask;
	int num_groups = cvmx_pow_num_groups();

	if (num_groups >= 64)
		valid_mask = ~0ull;
	else
		valid_mask = (1ull << num_groups)-1;

	if ((mask & valid_mask) == 0) {
	    cvmx_printf("ERROR: "
		"%s empty group mask disables work on core# %llu, ignored.\n",
		__func__, (unsigned long long) core_num);
	    return;
	}
	cvmx_warn_if(mask & (~valid_mask),
		"%s group number range exceeded: %#llx\n",
		__func__, (unsigned long long) mask);

	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		unsigned mask_set;
		cvmx_sso_ppx_sx_grpmskx_t grp_msk;
		unsigned core, node;
		unsigned rix;	/* Register index */
		unsigned grp;	/* Legacy group # */
		unsigned bit;	/* bit index */
		unsigned xgrp;	/* native group # */

		node = cvmx_coremask_core_to_node(core_num);
		core = cvmx_coremask_core_on_node(core_num);

		/* 78xx: 256 groups divided into 4 X 64 bit registers */
		/* 73xx: 64 groups are in one register */
		for (rix = 0; rix < (cvmx_sso_num_xgrp() >> 6); rix ++ ) {
			grp_msk.u64 = 0;
			for(bit = 0; bit < 64; bit++) {
				/* 8-bit native XGRP number */
				xgrp = (rix << 6) | bit;
				/* Legacy 5-bit group number */
				grp = (xgrp >> 3) & 0x1f;
				/* Inspect legacy mask by legacy group */
				if(mask & (1ull << grp))
					grp_msk.s.grp_msk |= 1ull << bit;
				/* Pre-set to all 0's */
			}
			for (mask_set = 0;
			    mask_set < cvmx_sso_num_maskset();
			    mask_set ++)
			    cvmx_write_csr_node(node,
				CVMX_SSO_PPX_SX_GRPMSKX(core, mask_set, rix),
				grp_msk.u64);
		}
	} else if (octeon_has_feature(OCTEON_FEATURE_CN68XX_WQE)) {
		cvmx_sso_ppx_grp_msk_t grp_msk;
		grp_msk.s.grp_msk = mask;
		cvmx_write_csr(CVMX_SSO_PPX_GRP_MSK(core_num), grp_msk.u64);
	} else {
		cvmx_pow_pp_grp_mskx_t grp_msk;

		grp_msk.u64 = cvmx_read_csr(CVMX_POW_PP_GRP_MSKX(core_num));
		grp_msk.s.grp_msk = mask & 0xffff;
		cvmx_write_csr(CVMX_POW_PP_GRP_MSKX(core_num), grp_msk.u64);
	}
}

/**
 * This function sets POW static priorities for a core. Each input queue has
 * an associated priority value.
 *
 * @param core_num   core to apply priorities to
 * @param priority   Vector of 8 priorities, one per POW Input Queue (0-7).
 *                   Highest priority is 0 and lowest is 7. A priority value
 *                   of 0xF instructs POW to skip the Input Queue when
 *                   scheduling to this specific core.
 *                   NOTE: priorities should not have gaps in values, meaning
 *                         {0,1,1,1,1,1,1,1} is a valid configuration while
 *                         {0,2,2,2,2,2,2,2} is not.
 */
static inline void cvmx_pow_set_priority(uint64_t core_num, const uint8_t priority[])
{
	if (OCTEON_IS_MODEL(OCTEON_CN3XXX))
		return;

	/* Detect gaps between priorities and flag error */
	if (!octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		int i;
		uint32_t prio_mask = 0;

		for (i = 0; i < 8; i++)
			if (priority[i] != 0xF)
				prio_mask |= 1 << priority[i];

		if (prio_mask ^ ((1 << cvmx_pop(prio_mask)) - 1)) {
			cvmx_dprintf("ERROR: POW static priorities should be contiguous (0x%llx)\n",
				(unsigned long long)prio_mask);
			return;
		}
	}

	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		unsigned group;
		unsigned node = cvmx_get_node_num();
		cvmx_sso_grpx_pri_t grp_pri;

		grp_pri.s.weight = 0x3f;
		grp_pri.s.affinity = 0xf;

		for(group = 0; group < cvmx_sso_num_xgrp(); group ++ ) {
			grp_pri.u64 = cvmx_read_csr_node(node,
				CVMX_SSO_GRPX_PRI(group));
			grp_pri.s.pri = priority[group & 0x7];
			cvmx_write_csr_node(node,
				CVMX_SSO_GRPX_PRI(group), grp_pri.u64);
		}

	} else if (octeon_has_feature(OCTEON_FEATURE_CN68XX_WQE)) {
		cvmx_sso_ppx_qos_pri_t qos_pri;

		qos_pri.u64 = cvmx_read_csr(CVMX_SSO_PPX_QOS_PRI(core_num));
		qos_pri.s.qos0_pri = priority[0];
		qos_pri.s.qos1_pri = priority[1];
		qos_pri.s.qos2_pri = priority[2];
		qos_pri.s.qos3_pri = priority[3];
		qos_pri.s.qos4_pri = priority[4];
		qos_pri.s.qos5_pri = priority[5];
		qos_pri.s.qos6_pri = priority[6];
		qos_pri.s.qos7_pri = priority[7];
		cvmx_write_csr(CVMX_SSO_PPX_QOS_PRI(core_num), qos_pri.u64);
	} else {
		/* POW priorities on CN5xxx .. CN66XX */
		cvmx_pow_pp_grp_mskx_t grp_msk;

		grp_msk.u64 = cvmx_read_csr(CVMX_POW_PP_GRP_MSKX(core_num));
		grp_msk.s.qos0_pri = priority[0];
		grp_msk.s.qos1_pri = priority[1];
		grp_msk.s.qos2_pri = priority[2];
		grp_msk.s.qos3_pri = priority[3];
		grp_msk.s.qos4_pri = priority[4];
		grp_msk.s.qos5_pri = priority[5];
		grp_msk.s.qos6_pri = priority[6];
		grp_msk.s.qos7_pri = priority[7];

		cvmx_write_csr(CVMX_POW_PP_GRP_MSKX(core_num), grp_msk.u64);
	}
}

/**
 * Performs a tag switch and then an immediate deschedule. This completes
 * immediately, so completion must not be waited for.  This function does NOT
 * update the wqe in DRAM to match arguments.
 *
 * This function does NOT wait for any prior tag switches to complete, so the
 * calling code must do this.
 *
 * Note the following CAVEAT of the Octeon HW behavior when
 * re-scheduling DE-SCHEDULEd items whose (next) state is
 * ORDERED:
 *   - If there are no switches pending at the time that the
 *     HW executes the de-schedule, the HW will only re-schedule
 *     the head of the FIFO associated with the given tag. This
 *     means that in many respects, the HW treats this ORDERED
 *     tag as an ATOMIC tag. Note that in the SWTAG_DESCH
 *     case (to an ORDERED tag), the HW will do the switch
 *     before the deschedule whenever it is possible to do
 *     the switch immediately, so it may often look like
 *     this case.
 *   - If there is a pending switch to ORDERED at the time
 *     the HW executes the de-schedule, the HW will perform
 *     the switch at the time it re-schedules, and will be
 *     able to reschedule any/all of the entries with the
 *     same tag.
 * Due to this behavior, the RECOMMENDATION to software is
 * that they have a (next) state of ATOMIC when they
 * DE-SCHEDULE. If an ORDERED tag is what was really desired,
 * SW can choose to immediately switch to an ORDERED tag
 * after the work (that has an ATOMIC tag) is re-scheduled.
 * Note that since there are never any tag switches pending
 * when the HW re-schedules, this switch can be IMMEDIATE upon
 * the reception of the pointer during the re-schedule.
 *
 * @param tag      New tag value
 * @param tag_type New tag type
 * @param group    New group value
 * @param no_sched Control whether this work queue entry will be rescheduled.
 *                 - 1 : don't schedule this work
 *                 - 0 : allow this work to be scheduled.
 */
static inline void cvmx_pow_tag_sw_desched_nocheck(uint32_t tag,
	cvmx_pow_tag_type_t tag_type, uint64_t group, uint64_t no_sched)
{
	union cvmx_pow_tag_req_addr ptr;
	cvmx_pow_tag_req_t tag_req;

	if (CVMX_ENABLE_POW_CHECKS) {
		cvmx_pow_tag_info_t current_tag;
		__cvmx_pow_warn_if_pending_switch(__func__);
		current_tag = cvmx_pow_get_current_tag();
		cvmx_warn_if(current_tag.tag_type == CVMX_POW_TAG_TYPE_NULL_NULL,
			"%s called with NULL_NULL tag\n", __func__);
		cvmx_warn_if(current_tag.tag_type == CVMX_POW_TAG_TYPE_NULL,
			"%s called with NULL tag. Deschedule not allowed from NULL state\n",
			__func__);
		cvmx_warn_if((current_tag.tag_type != CVMX_POW_TAG_TYPE_ATOMIC) &&
			(tag_type != CVMX_POW_TAG_TYPE_ATOMIC),
			"%s called where neither the before or after tag is ATOMIC\n", __func__);
	}
	tag_req.u64 = 0;
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		cvmx_wqe_t *wqp = cvmx_pow_get_current_wqp();
		if (wqp == NULL) {
			cvmx_dprintf("ERROR: Failed to get WQE, %s\n", __func__);
			return;
		}
		group &= 0x1f;
		wqp->word1.cn78xx.tag = tag;
		wqp->word1.cn78xx.tag_type = tag_type;
		wqp->word1.cn78xx.grp = group << 3;
		CVMX_SYNCWS;
		tag_req.s_cn78xx_other.op = CVMX_POW_TAG_OP_SWTAG_DESCH;
		tag_req.s_cn78xx_other.type = tag_type;
		tag_req.s_cn78xx_other.grp = group << 3;
		tag_req.s_cn78xx_other.no_sched = no_sched;
	}
	else if (octeon_has_feature(OCTEON_FEATURE_CN68XX_WQE)) {
		group &= 0x3f;
		tag_req.s_cn68xx_other.op = CVMX_POW_TAG_OP_SWTAG_DESCH;
		tag_req.s_cn68xx_other.tag = tag;
		tag_req.s_cn68xx_other.type = tag_type;
		tag_req.s_cn68xx_other.grp = group;
		tag_req.s_cn68xx_other.no_sched = no_sched;
	} else {
		group &= 0x0f;
		tag_req.s_cn38xx.op = CVMX_POW_TAG_OP_SWTAG_DESCH;
		tag_req.s_cn38xx.tag = tag;
		tag_req.s_cn38xx.type = tag_type;
		tag_req.s_cn38xx.grp = group;
		tag_req.s_cn38xx.no_sched = no_sched;
	}
	ptr.u64 = 0;
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		ptr.s.mem_region = CVMX_IO_SEG;
		ptr.s.is_io = 1;
		ptr.s.did = CVMX_OCT_DID_TAG_TAG3;
		ptr.s_cn78xx.node =  cvmx_get_node_num();
		ptr.s_cn78xx.tag  = tag;
	} else {
		ptr.s.mem_region = CVMX_IO_SEG;
		ptr.s.is_io = 1;
		ptr.s.did = CVMX_OCT_DID_TAG_TAG3;
	}
	cvmx_write_io(ptr.u64, tag_req.u64);
}

/**
 * Performs a tag switch and then an immediate deschedule. This completes
 * immediately, so completion must not be waited for.  This function does NOT
 * update the wqe in DRAM to match arguments.
 *
 * This function waits for any prior tag switches to complete, so the
 * calling code may call this function with a pending tag switch.
 *
 * Note the following CAVEAT of the Octeon HW behavior when
 * re-scheduling DE-SCHEDULEd items whose (next) state is
 * ORDERED:
 *   - If there are no switches pending at the time that the
 *     HW executes the de-schedule, the HW will only re-schedule
 *     the head of the FIFO associated with the given tag. This
 *     means that in many respects, the HW treats this ORDERED
 *     tag as an ATOMIC tag. Note that in the SWTAG_DESCH
 *     case (to an ORDERED tag), the HW will do the switch
 *     before the deschedule whenever it is possible to do
 *     the switch immediately, so it may often look like
 *     this case.
 *   - If there is a pending switch to ORDERED at the time
 *     the HW executes the de-schedule, the HW will perform
 *     the switch at the time it re-schedules, and will be
 *     able to reschedule any/all of the entries with the
 *     same tag.
 * Due to this behavior, the RECOMMENDATION to software is
 * that they have a (next) state of ATOMIC when they
 * DE-SCHEDULE. If an ORDERED tag is what was really desired,
 * SW can choose to immediately switch to an ORDERED tag
 * after the work (that has an ATOMIC tag) is re-scheduled.
 * Note that since there are never any tag switches pending
 * when the HW re-schedules, this switch can be IMMEDIATE upon
 * the reception of the pointer during the re-schedule.
 *
 * @param tag      New tag value
 * @param tag_type New tag type
 * @param group    New group value
 * @param no_sched Control whether this work queue entry will be rescheduled.
 *                 - 1 : don't schedule this work
 *                 - 0 : allow this work to be scheduled.
 */
static inline void cvmx_pow_tag_sw_desched(uint32_t tag,
	cvmx_pow_tag_type_t tag_type, uint64_t group, uint64_t no_sched)
{
	/* Need to make sure any writes to the work queue entry are complete */
	CVMX_SYNCWS;
	/* Ensure that there is not a pending tag switch, as a tag switch cannot be started
	 * if a previous switch is still pending.  */
	cvmx_pow_tag_sw_wait();
	cvmx_pow_tag_sw_desched_nocheck(tag, tag_type, group, no_sched);
}

/**
 * Descchedules the current work queue entry.
 *
 * @param no_sched no schedule flag value to be set on the work queue entry.
 *     If this is set the entry will not be rescheduled.
 */
static inline void cvmx_pow_desched(uint64_t no_sched)
{
	union cvmx_pow_tag_req_addr ptr;
	cvmx_pow_tag_req_t tag_req;

	if (CVMX_ENABLE_POW_CHECKS) {
		cvmx_pow_tag_info_t current_tag;
		__cvmx_pow_warn_if_pending_switch(__func__);
		current_tag = cvmx_pow_get_current_tag();
		cvmx_warn_if(current_tag.tag_type == CVMX_POW_TAG_TYPE_NULL_NULL,
			"%s called with NULL_NULL tag\n", __func__);
		cvmx_warn_if(current_tag.tag_type == CVMX_POW_TAG_TYPE_NULL,
			"%s called with NULL tag. Deschedule not expected from NULL state\n",
			__func__);
	}
	/* Need to make sure any writes to the work queue entry are complete */
	CVMX_SYNCWS;

	tag_req.u64 = 0;
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		tag_req.s_cn78xx_other.op = CVMX_POW_TAG_OP_DESCH;
		tag_req.s_cn78xx_other.no_sched = no_sched;
	} else if (octeon_has_feature(OCTEON_FEATURE_CN68XX_WQE)) {
		tag_req.s_cn68xx_other.op = CVMX_POW_TAG_OP_DESCH;
		tag_req.s_cn68xx_other.no_sched = no_sched;
	} else {
		tag_req.s_cn38xx.op = CVMX_POW_TAG_OP_DESCH;
		tag_req.s_cn38xx.no_sched = no_sched;
	}
	ptr.u64 = 0;
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		ptr.s_cn78xx.mem_region = CVMX_IO_SEG;
		ptr.s_cn78xx.is_io = 1;
		ptr.s_cn78xx.did = CVMX_OCT_DID_TAG_TAG3;
		ptr.s_cn78xx.node = cvmx_get_node_num();
	} else {
		ptr.s.mem_region = CVMX_IO_SEG;
		ptr.s.is_io = 1;
		ptr.s.did = CVMX_OCT_DID_TAG_TAG3;
	}
	cvmx_write_io(ptr.u64, tag_req.u64);
}
/******************************************************************************/
/* OCTEON3-specific functions.                                                */
/******************************************************************************/
/**
 * This function sets the the affinity of group to the cores in 78xx.
 * It sets up all the cores in core_mask to accept work from the specified group.
 *
 * @param xgrp  	Group to accept work from, 0 - 255.
 * @param core_mask	Mask of all the cores which will accept work from this group
 * @param mask_set	Every core has set of 2 masks which can be set to accept work
 *     from 256 groups. At the time of get_work, cores can choose which mask_set
 *     to get work from. 'mask_set' values range from 0 to 3, where	each of the
 *     two bits represents a mask set. Cores will be added to the mask set with
 *     corresponding bit set, and removed from the mask set with corresponding
 *     bit clear.
 * Note: cores can only accept work from SSO groups on the same node,
 * so the node number for the group is derived from the core number.
 */
static inline void cvmx_sso_set_group_core_affinity(cvmx_xgrp_t xgrp,
		const cvmx_coremask_t * core_mask, uint8_t mask_set)
{
	cvmx_sso_ppx_sx_grpmskx_t grp_msk;
	int core;
	int grp_index = xgrp.xgrp >> 6;
	int bit_pos = xgrp.xgrp % 64;

	if (!octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		cvmx_dprintf( "ERROR: %s is not supported on this chip)\n", __FUNCTION__);
		return;
	}
	cvmx_coremask_for_each_core(core, core_mask) {
		unsigned node, ncore;
		uint64_t reg_addr;

		node = cvmx_coremask_core_to_node(core);
		ncore = cvmx_coremask_core_on_node(core);

		reg_addr = CVMX_SSO_PPX_SX_GRPMSKX(ncore, 0, grp_index);
		grp_msk.u64 = cvmx_read_csr_node(node, reg_addr);

		if(mask_set & 1)
			grp_msk.s.grp_msk |= (1ull << bit_pos);
		else
			grp_msk.s.grp_msk &= ~(1ull << bit_pos);

		cvmx_write_csr_node(node, reg_addr, grp_msk.u64);

		reg_addr = CVMX_SSO_PPX_SX_GRPMSKX(ncore, 1, grp_index);
		grp_msk.u64 = cvmx_read_csr_node(node, reg_addr);

		if(mask_set & 2)
			grp_msk.s.grp_msk |= (1ull << bit_pos);
		else
			grp_msk.s.grp_msk &= ~(1ull << bit_pos);

		cvmx_write_csr_node(node, reg_addr, grp_msk.u64);
	}
}

/**
 * This function sets the priority and group affinity arbitration for each group.
 *
 * @param node 		Node number
 * @param xgrp  	Group 0 - 255 to apply mask parameters to
 * @param priority	Priority of the group relative to other groups
 *     0x0 - highest priority
 *     0x7 - lowest priority
 * @param weight	Cross-group arbitration weight to apply to this group.
 *     valid values are 1-63
 *     h/w default is 0x3f
 * @param affinity	Processor affinity arbitration weight to apply to this group.
 *     If zero, affinity is disabled.
 *     valid values are 0-15
 *     h/w default which is 0xf.
 * @param modify_mask   mask of the parameters which needs to be modified.
 *     enum cvmx_sso_group_modify_mask
 *     to modify only priority -- set bit0
 *     to modify only weight   -- set bit1
 *     to modify only affinity -- set bit2
 */
static inline void cvmx_sso_set_group_priority(int node, cvmx_xgrp_t xgrp,
			int priority, int weight, int affinity,
		       enum cvmx_sso_group_modify_mask modify_mask)
{
	cvmx_sso_grpx_pri_t grp_pri;

	if (!octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		cvmx_dprintf(
			"ERROR: %s is not supported on this chip)\n",
			__FUNCTION__);
		return;
	}
	if (weight <= 0)
		weight = 0x3f;	/* Force HW default when out of range */

	grp_pri.u64 = cvmx_read_csr_node(node, CVMX_SSO_GRPX_PRI(xgrp.xgrp));
	if(grp_pri.s.weight == 0)
		grp_pri.s.weight = 0x3f;
	if(modify_mask & CVMX_SSO_MODIFY_GROUP_PRIORITY)
		grp_pri.s.pri = priority;
	if(modify_mask & CVMX_SSO_MODIFY_GROUP_WEIGHT)
		grp_pri.s.weight = weight;
	if(modify_mask & CVMX_SSO_MODIFY_GROUP_AFFINITY)
		grp_pri.s.affinity = affinity;
	cvmx_write_csr_node(node,CVMX_SSO_GRPX_PRI(xgrp.xgrp),grp_pri.u64);
}

/**
 * Asynchronous work request.
 * Only works on CN78XX style SSO.
 *
 * Work is requested from the SSO unit, and should later be checked with
 * function cvmx_pow_work_response_async.
 * This function does NOT wait for previous tag switches to complete,
 * so the caller must ensure that there is not a pending tag switch.
 *
 * @param scr_addr Scratch memory address that response will be returned to,
 *     which is either a valid WQE, or a response with the invalid bit set.
 *     Byte address, must be 8 byte aligned.
 * @param xgrp  Group to receive work for (0-255).
 * @param wait
 *     1 to cause response to wait for work to become available (or timeout)
 *     0 to cause response to return immediately
 */
static inline void cvmx_sso_work_request_grp_async_nocheck(int scr_addr,
	cvmx_xgrp_t xgrp, cvmx_pow_wait_t wait)
{
	cvmx_pow_iobdma_store_t data;
	unsigned int node = cvmx_get_node_num();
	if (CVMX_ENABLE_POW_CHECKS) {
		__cvmx_pow_warn_if_pending_switch(__func__);
		cvmx_warn_if(!octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE), "Not CN78XX");
	}
	/* scr_addr must be 8 byte aligned */
	data.u64 = 0;
	data.s_cn78xx.scraddr = scr_addr >> 3;
	data.s_cn78xx.len = 1;
	data.s_cn78xx.did = CVMX_OCT_DID_TAG_SWTAG;
	data.s_cn78xx.grouped = 1;
	data.s_cn78xx.index_grp_mask = (node << 8) | xgrp.xgrp ;
	data.s_cn78xx.wait = wait;
	data.s_cn78xx.node = node;

	cvmx_send_single(data.u64);
}

/**
 * Synchronous work request from the node-local SSO without verifying
 * pending tag switch. It requests work from a specific SSO group.
 *
 * @param lgrp The local group number (within the SSO of the node of the caller)
 *     from which to get the work.
 * @param wait When set, call stalls until work becomes avaiable, or times out.
 *     If not set, returns immediately.
 *
 * @return Returns the WQE pointer from SSO.
 *     Returns NULL if no work was available.
 */
static inline void *cvmx_sso_work_request_grp_sync_nocheck(unsigned int lgrp,
	cvmx_pow_wait_t wait)
{
	cvmx_pow_load_addr_t ptr;
	cvmx_pow_tag_load_resp_t result;
	unsigned int node = cvmx_get_node_num() & 3;

	if (CVMX_ENABLE_POW_CHECKS) {
		__cvmx_pow_warn_if_pending_switch(__func__);
		cvmx_warn_if(!octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE), "Not CN78XX");
	}
	ptr.u64 = 0;
	ptr.swork_78xx.mem_region = CVMX_IO_SEG;
	ptr.swork_78xx.is_io = 1;
	ptr.swork_78xx.did = CVMX_OCT_DID_TAG_SWTAG;
	ptr.swork_78xx.node = node;
	ptr.swork_78xx.grouped = 1;
	ptr.swork_78xx.index = (lgrp & 0xff) | node << 8;
	ptr.swork_78xx.wait = wait;

	result.u64 = cvmx_read_csr(ptr.u64);
	if (result.s_work.no_work)
		return NULL;
	else
		return cvmx_phys_to_ptr(result.s_work.addr);
}

/**
 * Synchronous work request from the node-local SSO.
 * It requests work from a specific SSO group.
 * This function waits for any previous tag switch to complete before
 * requesting the new work.
 *
 * @param lgrp The node-local group number from which to get the work.
 * @param wait When set, call stalls until work becomes avaiable, or times out.
 *     If not set, returns immediately.
 *
 * @return The WQE pointer or NULL, if work is not available.
 */
static inline void *cvmx_sso_work_request_grp_sync(unsigned int lgrp,
	cvmx_pow_wait_t wait)
{
	cvmx_pow_tag_sw_wait();
	return cvmx_sso_work_request_grp_sync_nocheck(lgrp, wait);
}

/**
 * This function sets the group mask for a core.  The group mask bits
 * indicate which groups each core will accept work from.
 *
 * @param core_num 	Processor core to apply mask to.
 * @param mask_set	7XXX has 2 sets of masks per core.
 *     Bit 0 represents the first mask set, bit 1 -- the second.
 * @param xgrp_mask	Group mask array.
 *     Total number of groups is divided into a number of
 *     64-bits mask sets. Each bit in the mask, if set, enables
 *     the core to accept work from the corresponding group.
 *
 * NOTE: Each core can be configured to accept work in accordance to both
 * mask sets, with the first having higher precedence over the second,
 * or to accept work in accordance to just one of the two mask sets.
 * The 'core_num' argument represents a processor core on any node
 * in a coherent multi-chip system.
 *
 * If the 'mask_set' argument is 3, both mask sets are configured
 * with the same value (which is not typically the intention),
 * so keep in mind the function needs to be called twice
 * to set a different value into each of the mask sets,
 * once with 'mask_set=1' and second time with 'mask_set=2'.
 */
static inline void cvmx_pow_set_xgrp_mask( uint64_t core_num,
		uint8_t mask_set, const uint64_t xgrp_mask[])
{
	cvmx_sso_ppx_sx_grpmskx_t grp_msk;
	unsigned grp, node, core;
	uint64_t reg_addr;

	if (!octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		cvmx_dprintf( "ERROR: %s is not supported on this chip)\n",
			__FUNCTION__);
		return;
	}

	if (CVMX_ENABLE_POW_CHECKS) {
		cvmx_warn_if(((mask_set < 1) || (mask_set > 3)), "Invalid mask set");
	}

	if ((mask_set < 1) || (mask_set > 3))
		mask_set = 3;

	node = cvmx_coremask_core_to_node(core_num);
	core = cvmx_coremask_core_on_node(core_num);

	for (grp = 0; grp < (cvmx_sso_num_xgrp() >> 6); grp++) {
		reg_addr = CVMX_SSO_PPX_SX_GRPMSKX(core, 0, grp),
		grp_msk.u64 = 0;
		if (mask_set & 1) {
			grp_msk.s.grp_msk = xgrp_mask[grp];
			cvmx_write_csr_node(node, reg_addr, grp_msk.u64);
		}

		reg_addr = CVMX_SSO_PPX_SX_GRPMSKX(core, 1, grp),
		grp_msk.u64 = 0;
		if (mask_set & 2) {
			grp_msk.s.grp_msk = xgrp_mask[grp];
			cvmx_write_csr_node(node, reg_addr, grp_msk.u64);
		}
	}
}

/**
 * Executes SSO SWTAG command.
 * This is similiar to cvmx_pow_tag_sw() function, but uses linear
 * (vs. integrated group-qos) group index.
 */
static inline void cvmx_pow_tag_sw_node(cvmx_wqe_t *wqp, uint32_t tag,
	cvmx_pow_tag_type_t tag_type, int node)
{
	union cvmx_pow_tag_req_addr ptr;
	cvmx_pow_tag_req_t tag_req;

	if (cvmx_unlikely(!octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE))) {
		cvmx_dprintf("ERROR: %s is suppored on OCTEON3 only\n", __func__);
		return;
	}
	CVMX_SYNCWS;
	cvmx_pow_tag_sw_wait();

	if (CVMX_ENABLE_POW_CHECKS) {
		cvmx_pow_tag_info_t current_tag;
		__cvmx_pow_warn_if_pending_switch(__func__);
		current_tag = cvmx_pow_get_current_tag();
		cvmx_warn_if(current_tag.tag_type == CVMX_POW_TAG_TYPE_NULL_NULL,
			"%s called with NULL_NULL tag\n", __func__);
		cvmx_warn_if(current_tag.tag_type == CVMX_POW_TAG_TYPE_NULL,
			"%s called with NULL tag\n", __func__);
		cvmx_warn_if((current_tag.tag_type == tag_type) && (current_tag.tag == tag),
			"%s called to perform a tag switch to the same tag\n", __func__);
		cvmx_warn_if(tag_type == CVMX_POW_TAG_TYPE_NULL,
			"%s called to perform a tag switch to NULL. "
			"Use cvmx_pow_tag_sw_null() instead\n", __func__);
	}
	wqp->word1.cn78xx.tag = tag;
	wqp->word1.cn78xx.tag_type = tag_type;
	CVMX_SYNCWS;

	tag_req.u64 = 0;
	tag_req.s_cn78xx_other.op = CVMX_POW_TAG_OP_SWTAG;
	tag_req.s_cn78xx_other.type = tag_type;

	ptr.u64 = 0;
	ptr.s_cn78xx.mem_region = CVMX_IO_SEG;
	ptr.s_cn78xx.is_io = 1;
	ptr.s_cn78xx.did = CVMX_OCT_DID_TAG_SWTAG;
	ptr.s_cn78xx.node = node;
	ptr.s_cn78xx.tag  = tag;
	cvmx_write_io(ptr.u64, tag_req.u64);
}

/**
 * Executes SSO SWTAG_FULL command.
 * This is similiar to cvmx_pow_tag_sw_full() function, but
 * uses linear (vs. integrated group-qos) group index.
 */
static inline void cvmx_pow_tag_sw_full_node(cvmx_wqe_t * wqp, uint32_t tag,
	cvmx_pow_tag_type_t tag_type, uint8_t xgrp, int node)
{
	union cvmx_pow_tag_req_addr ptr;
	cvmx_pow_tag_req_t tag_req;
	uint16_t gxgrp;

	if (cvmx_unlikely(!octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE))) {
		cvmx_dprintf("ERROR: %s is suppored on OCTEON3 only\n", __func__);
		return;
	}
	/* Ensure that there is not a pending tag switch, as a tag switch cannot be
 	 * started, if a previous switch is still pending. */
	CVMX_SYNCWS;
	cvmx_pow_tag_sw_wait();

	if (CVMX_ENABLE_POW_CHECKS) {
		cvmx_pow_tag_info_t current_tag;
		__cvmx_pow_warn_if_pending_switch(__func__);
		current_tag = cvmx_pow_get_current_tag();
		cvmx_warn_if(current_tag.tag_type == CVMX_POW_TAG_TYPE_NULL_NULL,
			"%s called with NULL_NULL tag\n", __func__);
		cvmx_warn_if((current_tag.tag_type == tag_type) && (current_tag.tag == tag),
			"%s called to perform a tag switch to the same tag\n", __func__);
		cvmx_warn_if(tag_type == CVMX_POW_TAG_TYPE_NULL,
			"%s called to perform a tag switch to NULL. "
			"Use cvmx_pow_tag_sw_null() instead\n", __func__);
		if ((wqp != cvmx_phys_to_ptr(0x80)) && cvmx_pow_get_current_wqp())
			cvmx_warn_if(wqp != cvmx_pow_get_current_wqp(),
				"%s passed WQE(%p) doesn't match the address in the POW(%p)\n",
				__func__, wqp, cvmx_pow_get_current_wqp());
	}
	gxgrp = node;
	gxgrp = gxgrp << 8 | xgrp;
	wqp->word1.cn78xx.grp = gxgrp;
	wqp->word1.cn78xx.tag = tag;
	wqp->word1.cn78xx.tag_type = tag_type;
	CVMX_SYNCWS;

	tag_req.u64 = 0;
	tag_req.s_cn78xx_other.op = CVMX_POW_TAG_OP_SWTAG_FULL;
	tag_req.s_cn78xx_other.type = tag_type;
	tag_req.s_cn78xx_other.grp = gxgrp;
	tag_req.s_cn78xx_other.wqp = cvmx_ptr_to_phys(wqp);

	ptr.u64 = 0;
	ptr.s_cn78xx.mem_region = CVMX_IO_SEG;
	ptr.s_cn78xx.is_io = 1;
	ptr.s_cn78xx.did = CVMX_OCT_DID_TAG_SWTAG;
	ptr.s_cn78xx.node = node;
	ptr.s_cn78xx.tag  = tag;
	cvmx_write_io(ptr.u64, tag_req.u64);
}

/**
 * Submits work to an SSO group on any OCI node.
 * This function updates the work queue entry in DRAM to match
 * the arguments given.
 * Note that the tag provided is for the work queue entry submitted,
 * and is unrelated to the tag that the core currently holds.
 *
 * @param wqp pointer to work queue entry to submit.
 * This entry is updated to match the other parameters
 * @param tag tag value to be assigned to work queue entry
 * @param tag_type type of tag
 * @param xgrp native CN78XX group in the range 0..255
 * @param node The OCI node number for the target group
 *
 * When this function is called on a model prior to CN78XX, which does
 * not support OCI nodes, the 'node' argument is ignored, and the 'xgrp'
 * parameter is converted into 'qos' (the lower 3 bits) and 'grp' (the higher
 * 5 bits), following the backward-compatibility scheme of translating
 * between new and old style group numbers.
 */
static inline void cvmx_pow_work_submit_node(cvmx_wqe_t * wqp, uint32_t tag,
	cvmx_pow_tag_type_t tag_type, uint8_t xgrp, uint8_t node)
{
	union cvmx_pow_tag_req_addr ptr;
	cvmx_pow_tag_req_t tag_req;
	uint16_t group;

	if (cvmx_unlikely(!octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE))) {
		cvmx_dprintf("ERROR: %s is suppored on OCTEON3 only\n", __func__);
		return;
	}
	group = node;
	group = group << 8 | xgrp;
	wqp->word1.cn78xx.tag = tag;
	wqp->word1.cn78xx.tag_type = tag_type;
	wqp->word1.cn78xx.grp = group;
	CVMX_SYNCWS;

	tag_req.u64 = 0;
	tag_req.s_cn78xx_other.op = CVMX_POW_TAG_OP_ADDWQ;
	tag_req.s_cn78xx_other.type = tag_type;
	tag_req.s_cn78xx_other.wqp = cvmx_ptr_to_phys(wqp);
	tag_req.s_cn78xx_other.grp = group;

	ptr.u64 = 0;
	ptr.s_cn78xx.did = 0x66; //CVMX_OCT_DID_TAG_TAG6;
	ptr.s_cn78xx.mem_region = CVMX_IO_SEG;
	ptr.s_cn78xx.is_io = 1;
	ptr.s_cn78xx.node = node;
	ptr.s_cn78xx.tag = tag;

	/* SYNC write to memory before the work submit.  This is necessary
	 ** as POW may read values from DRAM at this time */
	CVMX_SYNCWS;
	cvmx_write_io(ptr.u64, tag_req.u64);
}

/**
 * Executes the SSO SWTAG_DESCHED operation.
 * This is similar to the cvmx_pow_tag_sw_desched() function, but
 * uses linear (vs. unified group-qos) group index.
 */
static inline void cvmx_pow_tag_sw_desched_node(cvmx_wqe_t *wqe, uint32_t tag,
	cvmx_pow_tag_type_t tag_type, uint8_t xgrp, uint64_t no_sched, uint8_t node)
{
	union cvmx_pow_tag_req_addr ptr;
	cvmx_pow_tag_req_t tag_req;
	uint16_t group;

	if (cvmx_unlikely(!octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE))) {
		cvmx_dprintf("ERROR: %s is suppored on OCTEON3 only\n", __func__);
		return;
	}
	/* Need to make sure any writes to the work queue entry are complete */
	CVMX_SYNCWS;
	/* Ensure that there is not a pending tag switch, as a tag switch cannot
	 * be started if a previous switch is still pending.  */
	cvmx_pow_tag_sw_wait();

	if (CVMX_ENABLE_POW_CHECKS) {
		cvmx_pow_tag_info_t current_tag;
		__cvmx_pow_warn_if_pending_switch(__func__);
		current_tag = cvmx_pow_get_current_tag();
		cvmx_warn_if(current_tag.tag_type == CVMX_POW_TAG_TYPE_NULL_NULL,
			"%s called with NULL_NULL tag\n", __func__);
		cvmx_warn_if(current_tag.tag_type == CVMX_POW_TAG_TYPE_NULL,
			"%s called with NULL tag. Deschedule not allowed from NULL state\n", __func__);
		cvmx_warn_if((current_tag.tag_type != CVMX_POW_TAG_TYPE_ATOMIC) &&
			(tag_type != CVMX_POW_TAG_TYPE_ATOMIC),
			"%s called where neither the before or after tag is ATOMIC\n", __func__);
	}
	group = node;
	group = group << 8 | xgrp;
	wqe->word1.cn78xx.tag = tag;
	wqe->word1.cn78xx.tag_type = tag_type;
	wqe->word1.cn78xx.grp = group;
	CVMX_SYNCWS;

	tag_req.u64 = 0;
	tag_req.s_cn78xx_other.op = CVMX_POW_TAG_OP_SWTAG_DESCH;
	tag_req.s_cn78xx_other.type = tag_type;
	tag_req.s_cn78xx_other.grp = group;
	tag_req.s_cn78xx_other.no_sched = no_sched;

	ptr.u64 = 0;
	ptr.s.mem_region = CVMX_IO_SEG;
	ptr.s.is_io = 1;
	ptr.s.did = CVMX_OCT_DID_TAG_TAG3;
	ptr.s_cn78xx.node =  node;
	ptr.s_cn78xx.tag  = tag;
	cvmx_write_io(ptr.u64, tag_req.u64);
}

/* Executes the UPD_WQP_GRP SSO operation.
 *
 * @param wqp  Pointer to the new work queue entry to switch to.
 * @param xgrp SSO group in the range 0..255
 *
 * NOTE: The operation can be performed only on the local node.
 */
static inline void cvmx_sso_update_wqp_group(cvmx_wqe_t *wqp, uint8_t xgrp)
{
	union cvmx_pow_tag_req_addr addr;
	cvmx_pow_tag_req_t data;
	int node = cvmx_get_node_num();
	int group = node <<  8 | xgrp;

	if (!octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		cvmx_dprintf("ERROR: %s is not supported on this chip)\n", __FUNCTION__);
		return;
	}
	wqp->word1.cn78xx.grp = group;
	CVMX_SYNCWS;

	data.u64 = 0;
	data.s_cn78xx_other.op = CVMX_POW_TAG_OP_UPDATE_WQP_GRP;
	data.s_cn78xx_other.grp = group;
	data.s_cn78xx_other.wqp = cvmx_ptr_to_phys(wqp);

	addr.u64 = 0;
	addr.s_cn78xx.mem_region = CVMX_IO_SEG;
	addr.s_cn78xx.is_io = 1;
	addr.s_cn78xx.did = CVMX_OCT_DID_TAG_TAG1;
	addr.s_cn78xx.node = node;
	cvmx_write_io(addr.u64, data.u64);
}

/******************************************************************************/
/* Define usage of bits within the 32 bit tag values.                         */
/******************************************************************************/
/*
 * Number of bits of the tag used by software.  The SW bits
 * are always a contiguous block of the high starting at bit 31.
 * The hardware bits are always the low bits.  By default, the top 8 bits
 * of the tag are reserved for software, and the low 24 are set by the IPD unit.
 */
#define CVMX_TAG_SW_BITS    (8)
#define CVMX_TAG_SW_SHIFT   (32 - CVMX_TAG_SW_BITS)

/* Below is the list of values for the top 8 bits of the tag. */
/* Tag values with top byte of this value are reserved for internal executive uses */
#define CVMX_TAG_SW_BITS_INTERNAL  0x1

/* The executive divides the remaining 24 bits as follows:
* the upper 8 bits (bits 23 - 16 of the tag) define a subgroup
* the lower 16 bits (bits 15 - 0 of the tag) define are the value with
* the subgroup. Note that this section describes the format of tags generated
* by software - refer to the hardware documentation for a description of the tags
* values generated by the packet input hardware.
* Subgroups are defined here */
#define CVMX_TAG_SUBGROUP_MASK  0xFFFF /* Mask for the value portion of the tag */
#define CVMX_TAG_SUBGROUP_SHIFT 16
#define CVMX_TAG_SUBGROUP_PKO  0x1

/* End of executive tag subgroup definitions */

/* The remaining values software bit values 0x2 - 0xff are available
 * for application use */

/**
 * This function creates a 32 bit tag value from the two values provided.
 *
 * @param sw_bits The upper bits (number depends on configuration) are set
 *     to this value.  The remainder of bits are set by the hw_bits parameter.
 * @param hw_bits The lower bits (number depends on configuration) are set
 *     to this value.  The remainder of bits are set by the sw_bits parameter.
 *
 * @return 32 bit value of the combined hw and sw bits.
 */
static inline uint32_t cvmx_pow_tag_compose(uint64_t sw_bits, uint64_t hw_bits)
{
	return ((((sw_bits & cvmx_build_mask(CVMX_TAG_SW_BITS)) << CVMX_TAG_SW_SHIFT) |
		(hw_bits & cvmx_build_mask(32 - CVMX_TAG_SW_BITS))));
}

/**
 * Extracts the bits allocated for software use from the tag
 *
 * @param tag    32 bit tag value
 *
 * @return N bit software tag value, where N is configurable with
 *     the CVMX_TAG_SW_BITS define
 */
static inline uint32_t cvmx_pow_tag_get_sw_bits(uint64_t tag)
{
	return ((tag >> (32 - CVMX_TAG_SW_BITS)) & cvmx_build_mask(CVMX_TAG_SW_BITS));
}

/**
 *
 * Extracts the bits allocated for hardware use from the tag
 *
 * @param tag    32 bit tag value
 *
 * @return (32 - N) bit software tag value, where N is configurable with
 *     the CVMX_TAG_SW_BITS define
 */
static inline uint32_t cvmx_pow_tag_get_hw_bits(uint64_t tag)
{
	return (tag & cvmx_build_mask(32 - CVMX_TAG_SW_BITS));
}

static inline uint64_t cvmx_sso3_get_wqe_count(int node)
{
	cvmx_sso_grpx_aq_cnt_t aq_cnt;
	unsigned grp = 0;
	uint64_t cnt = 0;

	for( grp = 0; grp < cvmx_sso_num_xgrp(); grp++) {
		aq_cnt.u64 = cvmx_read_csr_node(node, CVMX_SSO_GRPX_AQ_CNT(grp));
		cnt += aq_cnt.s.aq_cnt;
	}
	return cnt;
}

static inline uint64_t cvmx_sso_get_total_wqe_count(void)
{
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE))
	{
		int node = cvmx_get_node_num();
		return cvmx_sso3_get_wqe_count(node);
	}
	else if (OCTEON_IS_MODEL(OCTEON_CN68XX))
	{
		cvmx_sso_iq_com_cnt_t sso_iq_com_cnt;
		sso_iq_com_cnt.u64 = cvmx_read_csr(CVMX_SSO_IQ_COM_CNT);
		return (sso_iq_com_cnt.s.iq_cnt);
	}
	else
	{
		cvmx_pow_iq_com_cnt_t pow_iq_com_cnt;
		pow_iq_com_cnt.u64 = cvmx_read_csr(CVMX_POW_IQ_COM_CNT);
		return(pow_iq_com_cnt.s.iq_cnt);
	}
}

/**
 * Store the current POW internal state into the supplied
 * buffer. It is recommended that you pass a buffer of at least
 * 128KB. The format of the capture may change based on SDK
 * version and Octeon chip.
 *
 * @param buffer Buffer to store capture into
 * @param buffer_size The size of the supplied buffer
 *
 * @return Zero on sucess, negative on failure
 */
extern int cvmx_pow_capture(void *buffer, int buffer_size);

/**
 * Dump a POW capture to the console in a human readable format.
 *
 * @param buffer POW capture from cvmx_pow_capture()
 * @param buffer_size Size of the buffer
 */
extern void cvmx_pow_display(void *buffer, int buffer_size);

/**
 * Return the number of POW entries supported by this chip
 *
 * @return Number of POW entries
 */
extern int cvmx_pow_get_num_entries(void);
extern int cvmx_pow_get_dump_size(void);

/**
 * This will allocate count number of SSO groups on the specified node to the
 * calling application. These groups will be for exclusive use of the application
 * until they are freed.
 * @param node The numa node for the allocation.
 * @param base_group Pointer to the initial group, -1 to allocate anywhere.
 * @param count  The number of consecutive groups to allocate.
 * @return 0 on success and -1 on failure.
 */
int cvmx_sso_reserve_group_range(int node, int *base_group, int count);
#define cvmx_sso_allocate_group_range cvmx_sso_reserve_group_range
int cvmx_sso_reserve_group(int node);
#define cvmx_sso_allocate_group cvmx_sso_reserve_group
int cvmx_sso_release_group_range(int node, int base_group, int count);
int cvmx_sso_release_group(int node, int group);

/**
 * Show integrated PKI configuration.
 *
 * @param node	   node number
 */
int cvmx_sso_config_dump(unsigned node);

#ifdef  __cplusplus
/* *INDENT-OFF* */
}
/* *INDENT-ON* */
#endif

#endif /* __CVMX_POW_H__ */


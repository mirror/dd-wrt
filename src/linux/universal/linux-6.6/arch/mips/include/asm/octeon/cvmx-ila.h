/***********************license start***************
 * Copyright (c) 2014  Cavium Inc. (support@cavium.com). All rights
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
 * This file contains defines for the ILA interface
 *
 * <hr>$Revision: 49448 $<hr>
 *
 *
 */

#ifndef __CVMX_HELPER_ILA_H__
#define __CVMX_HELPER_ILA_H__

#include "cvmx-helper.h"
#include "cvmx-ilk.h"

#ifdef	__cplusplus
/* *INDENT-OFF* */
extern "C" {
/* *INDENT-ON* */
#endif

#define CVMX_ILA_GBL_BASE 10 

/**
 * This header is placed in front of all received ILA look-aside mode packets
 */
typedef union {
	uint64_t u64;

	struct {
#ifdef __BIG_ENDIAN_BITFIELD
		uint32_t reserved_63_57:7;	/**< bits 63...57 */
		uint32_t app_spec0:15;		/**< Application Specific 0 */
		uint32_t chan1:1;		/**< Channel 1 XON */
		uint32_t chan0:1;		/**< Channel 0 XON */
		uint32_t la_mode:1;		/**< Protocol Type */
		uint32_t app_spec1:6;		/**< Application Specific 1 */
		uint32_t ilk_channel:1;		/**< ILK channel number, 0 or 1 */
		uint32_t app_spec2:8;		/**< Application Specific 2 */
		uint32_t reserved_23_0:24;	/**< Unpredictable, may be any value */
#else
		uint32_t reserved_23_0:24;	/**< Unpredictable, may be any value */
		uint32_t app_spec2:8;		/**< Application Specific 2 */
		uint32_t ilk_channel:1;		/**< ILK channel number, 0 or 1 */
		uint32_t app_spec1:6;		/**< Application Specific 1 */
		uint32_t la_mode:1;		/**< Protocol Type */
		uint32_t chan0:1;		/**< Channel 0 XON */
		uint32_t chan1:1;		/**< Channel 1 XON */
		uint32_t app_spec0:15;		/**< Application Specific 0 */
		uint32_t reserved_63_57:7;	/**< bits 63...57 */
#endif
	} s;
} cvmx_ila_header_t;


/**
 * Initialize ILK-LA interface
 *
 * @param lane_mask  Lanes to initialize ILK-LA interface.
 * @return  0 on success and -1 on failure.
 */
extern int cvmx_ila_initialize(int lane_mask);

/**
 * @INTERNAL
 * Return the link state of an IPD/PKO port as returned by ILK-LA link status.
 *
 * @param lane_mask lane_mask
 *
 * @return Link state
 */
extern cvmx_helper_link_info_t __cvmx_ila_link_get(int lane_mask);

/**
 * Enable or disable LA mode in ILK header.
 *
 * @param channel channel
 * @param mode   If set, enable LA mode in ILK header, else disable
 *
 * @return ILK header
 */
extern cvmx_ila_header_t cvmx_ila_configure_header(int channel, int mode);

/**
 * Reset ILA interface
 */
extern void cvmx_ila_reset(void);

#ifdef	__cplusplus
/* *INDENT-OFF* */
}
/* *INDENT-ON* */
#endif
#endif /* __CVMX_HELPER_ILA_H__ */

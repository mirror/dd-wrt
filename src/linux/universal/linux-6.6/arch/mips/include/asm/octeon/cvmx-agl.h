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
 * Functions for AGL (RGMII) commong initialization, configuration.
 *
 * <hr>$Revision: $<hr>
 */

#ifndef __CVMX_AGL_H__
#define __CVMX_AGL_H__

#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
#include <asm/octeon/cvmx.h>
#include <asm/octeon/cvmx-helper.h>
#else
#include "cvmx-helper.h"
#endif

#ifdef  __cplusplus
/* *INDENT-OFF* */
extern "C" {
/* *INDENT-ON* */
#endif

/*
 * @param port port to enable
 *
 * @return Zero on success, negative on failure
 */
extern int cvmx_agl_enable(int port);

extern cvmx_helper_link_info_t cvmx_agl_link_get(int port);

/*
 * Set MII/RGMII link based on mode.
 *
 * @param port   interface port to set the link.
 * @param link_info  Link status
 *
 * @return       0 on success and 1 on failure
 */
extern int cvmx_agl_link_set(int port, cvmx_helper_link_info_t link_info);

/**
 * Disables the sending of flow control (pause) frames on the specified
 * AGL (RGMII) port(s).
 *
 * @param interface Which interface (0 or 1)
 * @param port_mask Mask (4bits) of which ports on the interface to disable
 *                  backpressure on.
 *                  1 => disable backpressure
 *                  0 => enable backpressure
 *
 * @return 0 on success
 *         -1 on error
 */
extern int cvmx_agl_set_backpressure_override(uint32_t interface,
					      uint32_t port_mask);

#ifdef  __cplusplus
/* *INDENT-OFF* */
}
/* *INDENT-ON* */
#endif
#endif /* __CVMX_AGL_H__ */

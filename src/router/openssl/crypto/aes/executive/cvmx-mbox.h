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
 * Encapsulation of OCTEON mailbox interrupts.
 *
 */
#ifndef __CVMX_MBOX_H__
#define __CVMX_MBOX_H__

#ifdef  __cplusplus
/* *INDENT-OFF* */
extern "C" {
/* *INDENT-ON* */
#endif

#ifndef CVMX_BUILD_FOR_LINUX_USER
struct cvmx_mbox;

/**
 * Function prototype for mailbox interrupt handlers
 */
typedef void (* cvmx_mbox_func_t)(struct cvmx_mbox *self, uint64_t *registers);

/**
 * Main structure for manipulating mailbox interrupts.
 */
struct cvmx_mbox {
	cvmx_mbox_func_t handler; /**< Set by user code, the handler function for the mailbox interrupt */
};
/**
 * Register a mailbox interrupt handler for the specified interrupt number.
 *
 * @param bit      Mailbox bit to register a handler for.
 * @param handler  Handler information.
 */
extern CVMX_SHARED void (*cvmx_mbox_register)(unsigned int bit, struct cvmx_mbox *handler);

#endif /* !CVMX_BUILD_FOR_LINUX_USER */

/**
 * Return the number of usable mailbox bits in the system.
 *
 * @return The hardware dependent number of mailbox bits.
 */
int cvmx_mbox_num_bits(void);

/**
 * Initialize the mailbox interrupt system for the specified bits.
 *
 * @param mbox_mask a bitmask of the mailbox bits that will be used by the program.
 */
void cvmx_mbox_initialize(uint32_t mbox_mask);

/**
 * Signal a mailbox interrupt handler for the specified bit and core.
 *
 * @param bit      Mailbox bit to signal.
 * @param core     The core to signal.
 */
extern CVMX_SHARED void (*cvmx_mbox_signal)(unsigned int bit, unsigned int core);

/**
 * Mailbox bit numbers used by simple-executive,
 * and a few mailbox numbers available for the user application
 */
#define CVMX_MBOX_BIT_HOTPLUG_SHUTDOWN		0
#define CVMX_MBOX_BIT_HOTPLUG_REMOVECORES	1
#define CVMX_MBOX_BIT_HOTPLUG_ADDCORES		2
#define CVMX_MBOX_BIT_OTRACE			3
#define CVMX_MBOX_BIT_RESERVED			4
#define CVMX_MBOX_BIT_USER_0			5
#define CVMX_MBOX_BIT_USER_1			6
#define CVMX_MBOX_BIT_USER_2			7
#define CVMX_MBOX_BIT_USER_3			8
#define CVMX_MBOX_BIT_USER_4			9

#ifdef  __cplusplus
/* *INDENT-OFF* */
}
/* *INDENT-ON* */
#endif

#endif

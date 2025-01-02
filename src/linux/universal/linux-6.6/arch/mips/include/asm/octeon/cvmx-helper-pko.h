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
 * PKO helper, configuration API
 */

#ifndef __CVMX_HELPER_PKO_H__
#define __CVMX_HELPER_PKO_H__

/* CSR typedefs have been moved to cvmx-pko-defs.h */

#if 0	// XXX Not clear what this is intended for !
/**
 * Definition of internal state for Packet output processing
 */
typedef struct {
	uint64_t *start_ptr;		/**< ptr to start of buffer, offset kept in FAU reg */
} cvmx_pko_state_elem_t;
#endif

/**
 * cvmx_override_pko_queue_priority(int ipd_port, uint64_t
 * priorities[16]) is a function pointer. It is meant to allow
 * customization of the PKO queue priorities based on the port
 * number. Users should set this pointer to a function before
 * calling any cvmx-helper operations.
 */
extern CVMX_SHARED void (*cvmx_override_pko_queue_priority) (int ipd_port, uint8_t * priorities);

/**
 * Gets the fpa pool number of pko pool
 */
int64_t cvmx_fpa_get_pko_pool(void);

/**
 * Gets the buffer size of pko pool
 */
uint64_t cvmx_fpa_get_pko_pool_block_size(void);

/**
 * Gets the buffer size  of pko pool
 */
uint64_t cvmx_fpa_get_pko_pool_buffer_count(void);


int cvmx_helper_pko_init(void);

/*
 * This function is a no-op
 * included here for backwards compatibility only.
 */
static inline  int cvmx_pko_initialize_local(void)
{
    return 0;
}

extern int __cvmx_helper_pko_drain(void);

extern int __cvmx_helper_interface_setup_pko(int interface);

#endif /* __CVMX_HELPER_PKO_H__ */

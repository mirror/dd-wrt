/***********************license start***************
 * Copyright (c) 2003-2012  Cavium Inc. (support@cavium.com). All rights
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
#include "cvmx.h"

#ifndef __CVMX_RANGE_H__
#define __CVMX_RANGE_H__

extern int  cvmx_range_init(uint64_t range_addr, int size);
extern int  cvmx_range_alloc(uint64_t range_addr, uint64_t owner, uint64_t cnt, int align);
extern int  cvmx_range_alloc_ordered(uint64_t range_addr, uint64_t owner,
				     uint64_t cnt, int align, int reverse);
extern int  cvmx_range_alloc_non_contiguos(uint64_t range_addr, uint64_t owner, uint64_t cnt,
					   int elements[]);
extern int  cvmx_range_reserve(uint64_t range_addr, uint64_t owner, uint64_t base, uint64_t cnt);
extern int  cvmx_range_free_with_base(uint64_t range_addr, int base, int cnt);
extern int  cvmx_range_free_with_owner(uint64_t range_addr, uint64_t owner);
extern uint64_t cvmx_range_get_owner(uint64_t range_addr, uint64_t base);
extern void cvmx_range_show(uint64_t range_addr);
extern int  cvmx_range_memory_size(int nelements);
extern int cvmx_range_free_mutiple(uint64_t range_addr, int bases[], int count);

#endif // __CVMX_RANGE_H__

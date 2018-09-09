/***********************license start***************
 * Copyright (c) 2003-2013  Cavium Inc. (support@cavium.com). All rights
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
 * Interface to power-throttle control, measurement, and debugging
 * facilities.
 *
 * <hr>$Revision: 91072 $<hr>
 *
 */

#ifndef __CVMX_POWER_THROTTLE_H__
#define __CVMX_POWER_THROTTLE_H__
#ifdef	__cplusplus
/* *INDENT-OFF* */
extern "C" {
/* *INDENT-ON* */
#endif

/**
 * a field of the POWTHROTTLE register
 */
union cvmx_power_throttle_rfield {
        uint64_t u64;
        struct {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t maxpow:8;           /* 63:56 */
	uint64_t power:8;            /* 55:48 */
	uint64_t thrott:8;           /* 47:40 */
	uint64_t hrmpowadj:8;        /* 39:32 reserved in cn63XX */
	uint64_t reserved:3;         /* 31:29 */
	uint64_t ovrrd:1;            /* 28  reserved in cn63XX */
	uint64_t distag:1;           /* 27 */
	uint64_t period:3;           /* 26:24 */
	uint64_t powlim:8;           /* 23:16 */
	uint64_t maxthr:8;           /* 15:8 */
	uint64_t minthr:8;           /* 7:0 */
#else
	uint64_t minthr:8;
	uint64_t maxthr:8;
	uint64_t powlim:8;
	uint64_t period:3;
	uint64_t distag:1;
	uint64_t ovrrd:1;
	uint64_t reserved:3;
	uint64_t hrmpowadj:8;
	uint64_t thrott:8;
	uint64_t power:8;
	uint64_t maxpow:8;
#endif
	} s;
};
typedef union cvmx_power_throttle_rfield cvmx_power_throttle_rfield_t;

/**
 * Return Cop0 Power Throttle register value for a given Core
 *
 * @param cpuid  : Core id
 * @return  Return COP0 PowThr value
 */
extern cvmx_power_throttle_rfield_t cvmx_power_throttle_show(int cpuid);

/**
 * Get the POWLIM field as percentage% of the MAXPOW field in r.
 *
 * @param ppid : Core to find the percentage
 * @return  On success return percentage or -1 on failure
 */
extern int cvmx_power_throttle_get_powlim(int ppid);

/**
 * Throttle power to percentage% of configured maximum (MAXPOW).
 *
 * @param percentage	0 to 100
 * @return 0 for success and -1 for error.
 */
extern int cvmx_power_throttle_self(uint8_t percentage);

/**
 * Throttle power to percentage% of configured maximum (MAXPOW)
 * for the cores identified in coremask.
 *
 * @param percentage 	0 to 100
 * @param pcm		bit mask where each bit identifies a core.
 * @return 0 for success and -1 for error.
 */
extern int cvmx_power_throttle(uint8_t percentage, cvmx_coremask_t *pcm);

#ifdef	__cplusplus
/* *INDENT-OFF* */
}
/* *INDENT-ON* */
#endif
#endif /* __CVMX_POWER_THROTTLE_H__ */

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
 * cvmx-vrmx-defs.h
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon vrmx.
 *
 * This file is auto generated. Do not edit.
 *
 * <hr>$Revision$<hr>
 *
 */
#ifndef __CVMX_VRMX_DEFS_H__
#define __CVMX_VRMX_DEFS_H__

#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_VRMX_ALT_FUSE(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((block_id == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 1)))))
		cvmx_warn("CVMX_VRMX_ALT_FUSE(%lu) is invalid on this chip\n", block_id);
	return CVMX_ADD_IO_SEG(0x0001180021000018ull) + ((block_id) & 1) * 0x1000000ull;
}
#else
#define CVMX_VRMX_ALT_FUSE(block_id) (CVMX_ADD_IO_SEG(0x0001180021000018ull) + ((block_id) & 1) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_VRMX_FUSE_BYPASS(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((block_id == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 1)))))
		cvmx_warn("CVMX_VRMX_FUSE_BYPASS(%lu) is invalid on this chip\n", block_id);
	return CVMX_ADD_IO_SEG(0x0001180021000098ull) + ((block_id) & 1) * 0x1000000ull;
}
#else
#define CVMX_VRMX_FUSE_BYPASS(block_id) (CVMX_ADD_IO_SEG(0x0001180021000098ull) + ((block_id) & 1) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_VRMX_MISC_INFO(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((block_id == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 1)))))
		cvmx_warn("CVMX_VRMX_MISC_INFO(%lu) is invalid on this chip\n", block_id);
	return CVMX_ADD_IO_SEG(0x0001180021000048ull) + ((block_id) & 1) * 0x1000000ull;
}
#else
#define CVMX_VRMX_MISC_INFO(block_id) (CVMX_ADD_IO_SEG(0x0001180021000048ull) + ((block_id) & 1) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_VRMX_TELEMETRY_CMND(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((block_id == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 1)))))
		cvmx_warn("CVMX_VRMX_TELEMETRY_CMND(%lu) is invalid on this chip\n", block_id);
	return CVMX_ADD_IO_SEG(0x0001180021000038ull) + ((block_id) & 1) * 0x1000000ull;
}
#else
#define CVMX_VRMX_TELEMETRY_CMND(block_id) (CVMX_ADD_IO_SEG(0x0001180021000038ull) + ((block_id) & 1) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_VRMX_TELEMETRY_READ(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((block_id == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 1)))))
		cvmx_warn("CVMX_VRMX_TELEMETRY_READ(%lu) is invalid on this chip\n", block_id);
	return CVMX_ADD_IO_SEG(0x0001180021000028ull) + ((block_id) & 1) * 0x1000000ull;
}
#else
#define CVMX_VRMX_TELEMETRY_READ(block_id) (CVMX_ADD_IO_SEG(0x0001180021000028ull) + ((block_id) & 1) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_VRMX_TS_TEMP_CONV_COEFF_FSM(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((block_id == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 1)))))
		cvmx_warn("CVMX_VRMX_TS_TEMP_CONV_COEFF_FSM(%lu) is invalid on this chip\n", block_id);
	return CVMX_ADD_IO_SEG(0x0001180021000078ull) + ((block_id) & 1) * 0x1000000ull;
}
#else
#define CVMX_VRMX_TS_TEMP_CONV_COEFF_FSM(block_id) (CVMX_ADD_IO_SEG(0x0001180021000078ull) + ((block_id) & 1) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_VRMX_TS_TEMP_CONV_CTL(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((block_id == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 1)))))
		cvmx_warn("CVMX_VRMX_TS_TEMP_CONV_CTL(%lu) is invalid on this chip\n", block_id);
	return CVMX_ADD_IO_SEG(0x0001180021000058ull) + ((block_id) & 1) * 0x1000000ull;
}
#else
#define CVMX_VRMX_TS_TEMP_CONV_CTL(block_id) (CVMX_ADD_IO_SEG(0x0001180021000058ull) + ((block_id) & 1) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_VRMX_TS_TEMP_CONV_RESULT(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((block_id == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 1)))))
		cvmx_warn("CVMX_VRMX_TS_TEMP_CONV_RESULT(%lu) is invalid on this chip\n", block_id);
	return CVMX_ADD_IO_SEG(0x0001180021000068ull) + ((block_id) & 1) * 0x1000000ull;
}
#else
#define CVMX_VRMX_TS_TEMP_CONV_RESULT(block_id) (CVMX_ADD_IO_SEG(0x0001180021000068ull) + ((block_id) & 1) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_VRMX_TS_TEMP_NOFF_MC(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((block_id == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 1)))))
		cvmx_warn("CVMX_VRMX_TS_TEMP_NOFF_MC(%lu) is invalid on this chip\n", block_id);
	return CVMX_ADD_IO_SEG(0x0001180021000088ull) + ((block_id) & 1) * 0x1000000ull;
}
#else
#define CVMX_VRMX_TS_TEMP_NOFF_MC(block_id) (CVMX_ADD_IO_SEG(0x0001180021000088ull) + ((block_id) & 1) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_VRMX_TWS_TWSI_SW(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((block_id == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 1)))))
		cvmx_warn("CVMX_VRMX_TWS_TWSI_SW(%lu) is invalid on this chip\n", block_id);
	return CVMX_ADD_IO_SEG(0x0001180021000008ull) + ((block_id) & 1) * 0x1000000ull;
}
#else
#define CVMX_VRMX_TWS_TWSI_SW(block_id) (CVMX_ADD_IO_SEG(0x0001180021000008ull) + ((block_id) & 1) * 0x1000000ull)
#endif

/**
 * cvmx_vrm#_alt_fuse
 */
union cvmx_vrmx_alt_fuse {
	uint64_t u64;
	struct cvmx_vrmx_alt_fuse_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_40_63               : 24;
	uint64_t tran_temp                    : 8;  /**< Transition temperature for V calculation. */
	uint64_t max_step                     : 8;  /**< V step. */
	uint64_t slope                        : 8;  /**< Slope. */
	uint64_t v_base                       : 8;  /**< Base voltage. Minimum voltage that gets written to voltage regulator. */
	uint64_t v_max                        : 8;  /**< Max voltage. Maximum voltage that gets written to voltage regulator. */
#else
	uint64_t v_max                        : 8;
	uint64_t v_base                       : 8;
	uint64_t slope                        : 8;
	uint64_t max_step                     : 8;
	uint64_t tran_temp                    : 8;
	uint64_t reserved_40_63               : 24;
#endif
	} s;
	struct cvmx_vrmx_alt_fuse_s           cn70xx;
	struct cvmx_vrmx_alt_fuse_s           cn78xx;
};
typedef union cvmx_vrmx_alt_fuse cvmx_vrmx_alt_fuse_t;

/**
 * cvmx_vrm#_fuse_bypass
 */
union cvmx_vrmx_fuse_bypass {
	uint64_t u64;
	struct cvmx_vrmx_fuse_bypass_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_3_63                : 61;
	uint64_t ctl_hw_bypass                : 1;  /**< If set, bypass VRM control hardware responsible for controlling external voltage
                                                         regulator so software can send commands to the regulator. */
	uint64_t ctl_fuse_bypass              : 1;  /**< If set, bypass V_MAX, V_BASE, MAX_STEP, SLOPE, TRAN_TEMP fuses which feed the PMBus VRM controller. */
	uint64_t ts_fuse_bypass               : 1;  /**< If set, bypass MC and NOFF fuses which feed the temperature sensor. */
#else
	uint64_t ts_fuse_bypass               : 1;
	uint64_t ctl_fuse_bypass              : 1;
	uint64_t ctl_hw_bypass                : 1;
	uint64_t reserved_3_63                : 61;
#endif
	} s;
	struct cvmx_vrmx_fuse_bypass_s        cn70xx;
	struct cvmx_vrmx_fuse_bypass_s        cn78xx;
};
typedef union cvmx_vrmx_fuse_bypass cvmx_vrmx_fuse_bypass_t;

/**
 * cvmx_vrm#_misc_info
 */
union cvmx_vrmx_misc_info {
	uint64_t u64;
	struct cvmx_vrmx_misc_info_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_43_63               : 21;
	uint64_t vrm_type2                    : 3;  /**< Reserved. */
	uint64_t vrm_ctl_rcv_status_error     : 24; /**< Holds the received status read from the external controller only for failing status. */
	uint64_t vrm_ctl_cur_state            : 3;  /**< Current state of the VRM_CTL. */
	uint64_t vrm_type                     : 2;  /**< VRM_TYPE identifies the type of device connected to the chip.
                                                         0x0 = Linear type, default or PMBus.
                                                         0x1 = VID type, ISL6367
                                                         0x2-3 = Reserved. */
	uint64_t boot_seq                     : 1;  /**< Boot sequence was completed. */
	uint64_t ts_fuse_sts                  : 1;  /**< Set if VRM fuses were loaded. */
	uint64_t vrm_fuse_sts                 : 1;  /**< Set if VRM fuses were loaded. */
	uint64_t cmnd                         : 8;  /**< Last command send to the external voltage regulator. */
#else
	uint64_t cmnd                         : 8;
	uint64_t vrm_fuse_sts                 : 1;
	uint64_t ts_fuse_sts                  : 1;
	uint64_t boot_seq                     : 1;
	uint64_t vrm_type                     : 2;
	uint64_t vrm_ctl_cur_state            : 3;
	uint64_t vrm_ctl_rcv_status_error     : 24;
	uint64_t vrm_type2                    : 3;
	uint64_t reserved_43_63               : 21;
#endif
	} s;
	struct cvmx_vrmx_misc_info_s          cn70xx;
	struct cvmx_vrmx_misc_info_cn78xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_40_63               : 24;
	uint64_t vrm_ctl_rcv_status_error     : 24; /**< Holds the received status read from the external controller only for failing status. */
	uint64_t vrm_ctl_cur_state            : 3;  /**< Current state of the VRM_CTL. */
	uint64_t vrm_type                     : 2;  /**< VRM_TYPE identifies the type of device connected to the chip.
                                                         0x0 = Linear type, default or PMBus.
                                                         0x1 = VID type, ISL6367
                                                         0x2-3 = Reserved. */
	uint64_t boot_seq                     : 1;  /**< Boot sequence was completed. */
	uint64_t ts_fuse_sts                  : 1;  /**< Set if VRM fuses were loaded. */
	uint64_t vrm_fuse_sts                 : 1;  /**< Set if VRM fuses were loaded. */
	uint64_t cmnd                         : 8;  /**< Last command send to the external voltage regulator. */
#else
	uint64_t cmnd                         : 8;
	uint64_t vrm_fuse_sts                 : 1;
	uint64_t ts_fuse_sts                  : 1;
	uint64_t boot_seq                     : 1;
	uint64_t vrm_type                     : 2;
	uint64_t vrm_ctl_cur_state            : 3;
	uint64_t vrm_ctl_rcv_status_error     : 24;
	uint64_t reserved_40_63               : 24;
#endif
	} cn78xx;
};
typedef union cvmx_vrmx_misc_info cvmx_vrmx_misc_info_t;

/**
 * cvmx_vrm#_telemetry_cmnd
 */
union cvmx_vrmx_telemetry_cmnd {
	uint64_t u64;
	struct cvmx_vrmx_telemetry_cmnd_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_2_63                : 62;
	uint64_t cmnd                         : 2;  /**< This command will initiate a read instruction to the external voltage regulator
                                                         into VRM_TELEMETRY_READ.
                                                         0x0 = Do nothing.
                                                         0x1 or 0x3 Read V.
                                                         0x2 = Read I.
                                                         Hardware clears CMND to indicate operation completed. */
#else
	uint64_t cmnd                         : 2;
	uint64_t reserved_2_63                : 62;
#endif
	} s;
	struct cvmx_vrmx_telemetry_cmnd_s     cn70xx;
	struct cvmx_vrmx_telemetry_cmnd_s     cn78xx;
};
typedef union cvmx_vrmx_telemetry_cmnd cvmx_vrmx_telemetry_cmnd_t;

/**
 * cvmx_vrm#_telemetry_read
 */
union cvmx_vrmx_telemetry_read {
	uint64_t u64;
	struct cvmx_vrmx_telemetry_read_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t iout                         : 16; /**< Current reading from external regulator. */
	uint64_t vout                         : 16; /**< Voltage reading from external regulator. */
#else
	uint64_t vout                         : 16;
	uint64_t iout                         : 16;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_vrmx_telemetry_read_s     cn70xx;
	struct cvmx_vrmx_telemetry_read_s     cn78xx;
};
typedef union cvmx_vrmx_telemetry_read cvmx_vrmx_telemetry_read_t;

/**
 * cvmx_vrm#_ts_temp_conv_coeff_fsm
 */
union cvmx_vrmx_ts_temp_conv_coeff_fsm {
	uint64_t u64;
	struct cvmx_vrmx_ts_temp_conv_coeff_fsm_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_58_63               : 6;
	uint64_t t_fsm                        : 10; /**< Sets time interval for FSM update. */
	uint64_t reserved_38_47               : 10;
	uint64_t coeff_a                      : 6;  /**< Coefficient A value for polynomial fit.
                                                         <37> is a sign bit to flip the shifted results.
                                                         <36:32> dictates the amount of right shift. */
	uint64_t reserved_25_31               : 7;
	uint64_t coeff_b                      : 9;  /**< Coefficient B value for polynomial fit. It should be a positive number and between 1 and
                                                         2.
                                                         <24> is the integer part should always be 1.
                                                         <23:16> form the fractional part. */
	uint64_t reserved_12_15               : 4;
	uint64_t coeff_c                      : 12; /**< Coefficient C value for polynomial fit.
                                                         A 2's complement number. */
#else
	uint64_t coeff_c                      : 12;
	uint64_t reserved_12_15               : 4;
	uint64_t coeff_b                      : 9;
	uint64_t reserved_25_31               : 7;
	uint64_t coeff_a                      : 6;
	uint64_t reserved_38_47               : 10;
	uint64_t t_fsm                        : 10;
	uint64_t reserved_58_63               : 6;
#endif
	} s;
	struct cvmx_vrmx_ts_temp_conv_coeff_fsm_s cn70xx;
	struct cvmx_vrmx_ts_temp_conv_coeff_fsm_s cn78xx;
};
typedef union cvmx_vrmx_ts_temp_conv_coeff_fsm cvmx_vrmx_ts_temp_conv_coeff_fsm_t;

/**
 * cvmx_vrm#_ts_temp_conv_ctl
 */
union cvmx_vrmx_ts_temp_conv_ctl {
	uint64_t u64;
	struct cvmx_vrmx_ts_temp_conv_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_13_63               : 51;
	uint64_t reset_sm                     : 1;  /**< This will force the temperature conversion state machine into the reset state until
                                                         RESET_SM is cleared. */
	uint64_t sw_access                    : 1;  /**< If set, software controls inputs of analog temperature sensor. */
	uint64_t ts_switch                    : 9;  /**< Alternate software access to control temperature sensor switches. */
	uint64_t ts_curr2en                   : 1;  /**< Controls curr2_en pin on analog temperature sensor block. */
	uint64_t strobe                       : 1;  /**< Controls strobe pin on analog temperature sensor block. */
#else
	uint64_t strobe                       : 1;
	uint64_t ts_curr2en                   : 1;
	uint64_t ts_switch                    : 9;
	uint64_t sw_access                    : 1;
	uint64_t reset_sm                     : 1;
	uint64_t reserved_13_63               : 51;
#endif
	} s;
	struct cvmx_vrmx_ts_temp_conv_ctl_s   cn70xx;
	struct cvmx_vrmx_ts_temp_conv_ctl_s   cn78xx;
};
typedef union cvmx_vrmx_ts_temp_conv_ctl cvmx_vrmx_ts_temp_conv_ctl_t;

/**
 * cvmx_vrm#_ts_temp_conv_result
 */
union cvmx_vrmx_ts_temp_conv_result {
	uint64_t u64;
	struct cvmx_vrmx_ts_temp_conv_result_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_24_63               : 40;
	uint64_t n_valid                      : 1;  /**< When set N_VALUE is valid. */
	uint64_t n_value                      : 11; /**< N cycle count values after calibration initiated. Qualified by N_VALID. */
	uint64_t temp_valid                   : 1;  /**< When set TEMP_CORRECTED is valid.
                                                         This bit is pulsed on each conversion, and as such software may not be able to observe the
                                                         cycle in which TEMP_VALID is set. */
	uint64_t temp_corrected               : 11; /**< Corrected temperature read out from the temp sensor module.
                                                         Has valid data after TEMP_VALID transitions. */
#else
	uint64_t temp_corrected               : 11;
	uint64_t temp_valid                   : 1;
	uint64_t n_value                      : 11;
	uint64_t n_valid                      : 1;
	uint64_t reserved_24_63               : 40;
#endif
	} s;
	struct cvmx_vrmx_ts_temp_conv_result_s cn70xx;
	struct cvmx_vrmx_ts_temp_conv_result_s cn78xx;
};
typedef union cvmx_vrmx_ts_temp_conv_result cvmx_vrmx_ts_temp_conv_result_t;

/**
 * cvmx_vrm#_ts_temp_noff_mc
 */
union cvmx_vrmx_ts_temp_noff_mc {
	uint64_t u64;
	struct cvmx_vrmx_ts_temp_noff_mc_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_28_63               : 36;
	uint64_t mc                           : 12; /**< MC value, default is 3000 decimal. */
	uint64_t reserved_11_15               : 5;
	uint64_t noff                         : 11; /**< N cycle count offset, used to subtract the appropriate count from N cycle.
                                                         It should be such that at 0 degrees C, the difference between NOFF and NCYCLE is 0. */
#else
	uint64_t noff                         : 11;
	uint64_t reserved_11_15               : 5;
	uint64_t mc                           : 12;
	uint64_t reserved_28_63               : 36;
#endif
	} s;
	struct cvmx_vrmx_ts_temp_noff_mc_s    cn70xx;
	struct cvmx_vrmx_ts_temp_noff_mc_s    cn78xx;
};
typedef union cvmx_vrmx_ts_temp_noff_mc cvmx_vrmx_ts_temp_noff_mc_t;

/**
 * cvmx_vrm#_tws_twsi_sw
 *
 * This register allows SW to write data into I2C Controller directly.
 * This register is for diagnostic use only.
 */
union cvmx_vrmx_tws_twsi_sw {
	uint64_t u64;
	struct cvmx_vrmx_tws_twsi_sw_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t v                            : 1;  /**< Valid bit.
                                                         Set on a write operation (should always be written with a 1).
                                                         Cleared when a TWSI master-mode operation completes.
                                                         Cleared when a TWSI configuration register access completes.
                                                         Cleared when the TWSI device reads the register if SLONLY = 1. */
	uint64_t sl_only                      : 1;  /**< Reserved. */
	uint64_t eia                          : 1;  /**< Reserved. */
	uint64_t op                           : 4;  /**< Reserved. */
	uint64_t r                            : 1;  /**< Read bit or result. If this bit is set on a CSR write when SLONLY = 0, the operation is a
                                                         read operation (if clear, it is a write operation).
                                                         On a CSR read, this bit returns the result indication for the most recent master-mode
                                                         operation, 1 = success, 0 = failure. */
	uint64_t sovr                         : 1;  /**< Reserved. */
	uint64_t size_b3                      : 1;  /**< Reserved. */
	uint64_t size                         : 2;  /**< Size minus one. Specifies the size in bytes of the master-mode operation.
                                                         (0 = 1 byte, 1 = 2 bytes, 3 = 4 bytes). */
	uint64_t scr                          : 2;  /**< Reserved. */
	uint64_t un_addr                      : 3;  /**< Reserved. */
	uint64_t addr                         : 7;  /**< Address field. The address of the remote device for a master-mode operation.
                                                         Note that when mastering a 7-bit OP, ADDR<6:0> should not take any of the values 0x78,
                                                         0x79, 0x7A nor 0x7B, as these 7-bit addresses are reserved to extend to 10-bit addressing. */
	uint64_t ia                           : 8;  /**< Internal Address. Used when launching a combined master-mode operation. */
	uint64_t data                         : 32; /**< Data field.
                                                         Used on a write operation when:
                                                         Initiating a master-mode write operation (SLONLY = 0).
                                                         Writing a TWSI configuration register (SLONLY = 0).
                                                         The read value is updated by:
                                                         A write operation to this register.
                                                         Master-mode completion (contains error code).
                                                         TWSI configuration-register read (contains result). */
#else
	uint64_t data                         : 32;
	uint64_t ia                           : 8;
	uint64_t addr                         : 7;
	uint64_t un_addr                      : 3;
	uint64_t scr                          : 2;
	uint64_t size                         : 2;
	uint64_t size_b3                      : 1;
	uint64_t sovr                         : 1;
	uint64_t r                            : 1;
	uint64_t op                           : 4;
	uint64_t eia                          : 1;
	uint64_t sl_only                      : 1;
	uint64_t v                            : 1;
#endif
	} s;
	struct cvmx_vrmx_tws_twsi_sw_s        cn70xx;
	struct cvmx_vrmx_tws_twsi_sw_s        cn78xx;
};
typedef union cvmx_vrmx_tws_twsi_sw cvmx_vrmx_tws_twsi_sw_t;

#endif

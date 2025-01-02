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
 * cvmx-gpio-defs.h
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon gpio.
 *
 * This file is auto generated. Do not edit.
 *
 * <hr>$Revision$<hr>
 *
 */
#ifndef __CVMX_GPIO_DEFS_H__
#define __CVMX_GPIO_DEFS_H__

#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_GPIO_BIT_CFGX(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN30XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN50XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN38XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN31XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN58XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 15))
				return CVMX_ADD_IO_SEG(0x0001070000000800ull) + ((offset) & 15) * 8;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 19))
					return CVMX_ADD_IO_SEG(0x0001070000000900ull) + ((offset) & 31) * 8;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 19))
					return CVMX_ADD_IO_SEG(0x0001070000000900ull) + ((offset) & 31) * 8;

			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 31))
				return CVMX_ADD_IO_SEG(0x0001070000000900ull) + ((offset) & 31) * 8;
			break;
	}
	cvmx_warn("CVMX_GPIO_BIT_CFGX (offset = %lu) not supported on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001070000000900ull) + ((offset) & 31) * 8;
}
#else
static inline uint64_t CVMX_GPIO_BIT_CFGX(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN30XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN50XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN38XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN31XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN58XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001070000000800ull) + (offset) * 8;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return CVMX_ADD_IO_SEG(0x0001070000000900ull) + (offset) * 8;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return CVMX_ADD_IO_SEG(0x0001070000000900ull) + (offset) * 8;

		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001070000000900ull) + (offset) * 8;
	}
	return CVMX_ADD_IO_SEG(0x0001070000000900ull) + (offset) * 8;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_GPIO_BOOT_ENA CVMX_GPIO_BOOT_ENA_FUNC()
static inline uint64_t CVMX_GPIO_BOOT_ENA_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN30XX) || OCTEON_IS_MODEL(OCTEON_CN31XX) || OCTEON_IS_MODEL(OCTEON_CN50XX)))
		cvmx_warn("CVMX_GPIO_BOOT_ENA not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00010700000008A8ull);
}
#else
#define CVMX_GPIO_BOOT_ENA (CVMX_ADD_IO_SEG(0x00010700000008A8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_GPIO_CLK_GENX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN52XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN56XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 3)))))
		cvmx_warn("CVMX_GPIO_CLK_GENX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00010700000008C0ull) + ((offset) & 3) * 8;
}
#else
#define CVMX_GPIO_CLK_GENX(offset) (CVMX_ADD_IO_SEG(0x00010700000008C0ull) + ((offset) & 3) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_GPIO_CLK_QLMX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_GPIO_CLK_QLMX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00010700000008E0ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_GPIO_CLK_QLMX(offset) (CVMX_ADD_IO_SEG(0x00010700000008E0ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_GPIO_CLK_SYNCEX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_GPIO_CLK_SYNCEX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00010700000008E0ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_GPIO_CLK_SYNCEX(offset) (CVMX_ADD_IO_SEG(0x00010700000008E0ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_GPIO_COMP CVMX_GPIO_COMP_FUNC()
static inline uint64_t CVMX_GPIO_COMP_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_GPIO_COMP not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001070000000D00ull);
}
#else
#define CVMX_GPIO_COMP (CVMX_ADD_IO_SEG(0x0001070000000D00ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_GPIO_DBG_ENA CVMX_GPIO_DBG_ENA_FUNC()
static inline uint64_t CVMX_GPIO_DBG_ENA_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN30XX) || OCTEON_IS_MODEL(OCTEON_CN31XX) || OCTEON_IS_MODEL(OCTEON_CN50XX)))
		cvmx_warn("CVMX_GPIO_DBG_ENA not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00010700000008A0ull);
}
#else
#define CVMX_GPIO_DBG_ENA (CVMX_ADD_IO_SEG(0x00010700000008A0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_GPIO_INTRX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 15))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 15))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 15))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 15)))))
		cvmx_warn("CVMX_GPIO_INTRX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001070000000A00ull) + ((offset) & 15) * 8;
}
#else
#define CVMX_GPIO_INTRX(offset) (CVMX_ADD_IO_SEG(0x0001070000000A00ull) + ((offset) & 15) * 8)
#endif
#define CVMX_GPIO_INT_CLR (CVMX_ADD_IO_SEG(0x0001070000000898ull))
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_GPIO_MC_INTRX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && (((offset >= 4) && (offset <= 7)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset >= 4) && (offset <= 7)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && (((offset >= 4) && (offset <= 7)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && (((offset >= 4) && (offset <= 7))))))
		cvmx_warn("CVMX_GPIO_MC_INTRX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001070000000C20ull) + ((offset) & 7) * 8 - 8*4;
}
#else
#define CVMX_GPIO_MC_INTRX(offset) (CVMX_ADD_IO_SEG(0x0001070000000C20ull) + ((offset) & 7) * 8 - 8*4)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_GPIO_MC_INTRX_W1S(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && (((offset >= 4) && (offset <= 7)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset >= 4) && (offset <= 7)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && (((offset >= 4) && (offset <= 7))))))
		cvmx_warn("CVMX_GPIO_MC_INTRX_W1S(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001070000000E20ull) + ((offset) & 7) * 8 - 8*4;
}
#else
#define CVMX_GPIO_MC_INTRX_W1S(offset) (CVMX_ADD_IO_SEG(0x0001070000000E20ull) + ((offset) & 7) * 8 - 8*4)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_GPIO_MULTI_CAST CVMX_GPIO_MULTI_CAST_FUNC()
static inline uint64_t CVMX_GPIO_MULTI_CAST_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF71XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_GPIO_MULTI_CAST not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00010700000008B0ull);
}
#else
#define CVMX_GPIO_MULTI_CAST (CVMX_ADD_IO_SEG(0x00010700000008B0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_GPIO_OCLA_EXTEN_TRIG CVMX_GPIO_OCLA_EXTEN_TRIG_FUNC()
static inline uint64_t CVMX_GPIO_OCLA_EXTEN_TRIG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_GPIO_OCLA_EXTEN_TRIG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00010700000008B8ull);
}
#else
#define CVMX_GPIO_OCLA_EXTEN_TRIG (CVMX_ADD_IO_SEG(0x00010700000008B8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_GPIO_PIN_ENA CVMX_GPIO_PIN_ENA_FUNC()
static inline uint64_t CVMX_GPIO_PIN_ENA_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN66XX)))
		cvmx_warn("CVMX_GPIO_PIN_ENA not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00010700000008B8ull);
}
#else
#define CVMX_GPIO_PIN_ENA (CVMX_ADD_IO_SEG(0x00010700000008B8ull))
#endif
#define CVMX_GPIO_RX_DAT (CVMX_ADD_IO_SEG(0x0001070000000880ull))
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_GPIO_SATA_CTL CVMX_GPIO_SATA_CTL_FUNC()
static inline uint64_t CVMX_GPIO_SATA_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX)))
		cvmx_warn("CVMX_GPIO_SATA_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00010700000008A8ull);
}
#else
#define CVMX_GPIO_SATA_CTL (CVMX_ADD_IO_SEG(0x00010700000008A8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_GPIO_SATA_CTLX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_GPIO_SATA_CTLX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001070000000D80ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_GPIO_SATA_CTLX(offset) (CVMX_ADD_IO_SEG(0x0001070000000D80ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_GPIO_SATA_LAB_LB CVMX_GPIO_SATA_LAB_LB_FUNC()
static inline uint64_t CVMX_GPIO_SATA_LAB_LB_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_GPIO_SATA_LAB_LB not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001070000000D40ull);
}
#else
#define CVMX_GPIO_SATA_LAB_LB (CVMX_ADD_IO_SEG(0x0001070000000D40ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_GPIO_TIM_CTL CVMX_GPIO_TIM_CTL_FUNC()
static inline uint64_t CVMX_GPIO_TIM_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_GPIO_TIM_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00010700000008A0ull);
}
#else
#define CVMX_GPIO_TIM_CTL (CVMX_ADD_IO_SEG(0x00010700000008A0ull))
#endif
#define CVMX_GPIO_TX_CLR (CVMX_ADD_IO_SEG(0x0001070000000890ull))
#define CVMX_GPIO_TX_SET (CVMX_ADD_IO_SEG(0x0001070000000888ull))
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_GPIO_USBDRD_CTLX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_GPIO_USBDRD_CTLX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001070000000D20ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_GPIO_USBDRD_CTLX(offset) (CVMX_ADD_IO_SEG(0x0001070000000D20ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_GPIO_USBH_CTL CVMX_GPIO_USBH_CTL_FUNC()
static inline uint64_t CVMX_GPIO_USBH_CTL_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00010700000008A0ull);
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
					return CVMX_ADD_IO_SEG(0x0001070000000898ull);
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
					return CVMX_ADD_IO_SEG(0x0001070000000898ull);

			break;
	}
	cvmx_warn("CVMX_GPIO_USBH_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001070000000898ull);
}
#else
#define CVMX_GPIO_USBH_CTL CVMX_GPIO_USBH_CTL_FUNC()
static inline uint64_t CVMX_GPIO_USBH_CTL_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00010700000008A0ull);
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return CVMX_ADD_IO_SEG(0x0001070000000898ull);
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return CVMX_ADD_IO_SEG(0x0001070000000898ull);

	}
	return CVMX_ADD_IO_SEG(0x0001070000000898ull);
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_GPIO_XBIT_CFGX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN30XX) && (((offset >= 16) && (offset <= 23)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN31XX) && (((offset >= 16) && (offset <= 23)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN50XX) && (((offset >= 16) && (offset <= 23)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && (((offset >= 16) && (offset <= 19)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && (((offset >= 16) && (offset <= 19)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && (((offset >= 16) && (offset <= 19)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && (((offset >= 16) && (offset <= 19))))))
		cvmx_warn("CVMX_GPIO_XBIT_CFGX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001070000000900ull) + ((offset) & 31) * 8 - 8*16;
}
#else
#define CVMX_GPIO_XBIT_CFGX(offset) (CVMX_ADD_IO_SEG(0x0001070000000900ull) + ((offset) & 31) * 8 - 8*16)
#endif

/**
 * cvmx_gpio_bit_cfg#
 *
 * Each register provides configuration information for the corresponding GPIO pin.
 *
 */
union cvmx_gpio_bit_cfgx {
	uint64_t u64;
	struct cvmx_gpio_bit_cfgx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_15_63               : 49;
	uint64_t clk_gen                      : 1;  /**< When TX_OE is set, GPIO pin becomes a clock */
	uint64_t clk_sel                      : 2;  /**< Selects which of the 4 GPIO clock generators */
	uint64_t fil_sel                      : 4;  /**< Filter select. Global counter bit-select (controls sample rate). */
	uint64_t fil_cnt                      : 4;  /**< Filter count. Specifies the number of consecutive samples ([FIL_CNT]+1) to change state.
                                                         Zero to disable the filter. */
	uint64_t int_type                     : 1;  /**< Type of interrupt when pin is an input and [INT_EN] set. When set, rising edge interrupt,
                                                         else level interrupt. Only valid for GPIO 0..15, no function for GPIO 16..31. */
	uint64_t int_en                       : 1;  /**< GPIO_BIT_CFG()[INT_EN] enable interrupts going to GPIO_INTR()[INTR] as well as
                                                         GPIO_MC_INTR(4..7)[INTR]. When GPIO_INTR()[INT_TYPE] is level interrupt,
                                                         changing  GPIO_INTR()[INT_EN] to zero will cause GPIO_INTR()[INTR] to be cleared. */
	uint64_t rx_xor                       : 1;  /**< Receive inversion. When set to 1, inverts the received GPIO signal. */
	uint64_t tx_oe                        : 1;  /**< Transmit output enable. When set to 1, the GPIO pin is driven as an output pin. */
#else
	uint64_t tx_oe                        : 1;
	uint64_t rx_xor                       : 1;
	uint64_t int_en                       : 1;
	uint64_t int_type                     : 1;
	uint64_t fil_cnt                      : 4;
	uint64_t fil_sel                      : 4;
	uint64_t clk_sel                      : 2;
	uint64_t clk_gen                      : 1;
	uint64_t reserved_15_63               : 49;
#endif
	} s;
	struct cvmx_gpio_bit_cfgx_cn30xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_12_63               : 52;
	uint64_t fil_sel                      : 4;  /**< Global counter bit-select (controls sample rate) */
	uint64_t fil_cnt                      : 4;  /**< Number of consecutive samples to change state */
	uint64_t int_type                     : 1;  /**< Type of interrupt
                                                         0 = level (default)
                                                         1 = rising edge */
	uint64_t int_en                       : 1;  /**< Bit mask to indicate which bits to raise interrupt */
	uint64_t rx_xor                       : 1;  /**< Invert the GPIO pin */
	uint64_t tx_oe                        : 1;  /**< Drive the GPIO pin as an output pin */
#else
	uint64_t tx_oe                        : 1;
	uint64_t rx_xor                       : 1;
	uint64_t int_en                       : 1;
	uint64_t int_type                     : 1;
	uint64_t fil_cnt                      : 4;
	uint64_t fil_sel                      : 4;
	uint64_t reserved_12_63               : 52;
#endif
	} cn30xx;
	struct cvmx_gpio_bit_cfgx_cn30xx      cn31xx;
	struct cvmx_gpio_bit_cfgx_cn30xx      cn38xx;
	struct cvmx_gpio_bit_cfgx_cn30xx      cn38xxp2;
	struct cvmx_gpio_bit_cfgx_cn30xx      cn50xx;
	struct cvmx_gpio_bit_cfgx_s           cn52xx;
	struct cvmx_gpio_bit_cfgx_s           cn52xxp1;
	struct cvmx_gpio_bit_cfgx_s           cn56xx;
	struct cvmx_gpio_bit_cfgx_s           cn56xxp1;
	struct cvmx_gpio_bit_cfgx_cn30xx      cn58xx;
	struct cvmx_gpio_bit_cfgx_cn30xx      cn58xxp1;
	struct cvmx_gpio_bit_cfgx_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_17_63               : 47;
	uint64_t synce_sel                    : 2;  /**< Selects the QLM clock output
                                                         x0=Normal GPIO output
                                                         01=GPIO QLM clock selected by CSR GPIO_CLK_QLM0
                                                         11=GPIO QLM clock selected by CSR GPIO_CLK_QLM1 */
	uint64_t clk_gen                      : 1;  /**< When TX_OE is set, GPIO pin becomes a clock */
	uint64_t clk_sel                      : 2;  /**< Selects which of the 4 GPIO clock generators */
	uint64_t fil_sel                      : 4;  /**< Global counter bit-select (controls sample rate) */
	uint64_t fil_cnt                      : 4;  /**< Number of consecutive samples to change state */
	uint64_t int_type                     : 1;  /**< Type of interrupt
                                                         0 = level (default)
                                                         1 = rising edge */
	uint64_t int_en                       : 1;  /**< Bit mask to indicate which bits to raise interrupt */
	uint64_t rx_xor                       : 1;  /**< Invert the GPIO pin */
	uint64_t tx_oe                        : 1;  /**< Drive the GPIO pin as an output pin */
#else
	uint64_t tx_oe                        : 1;
	uint64_t rx_xor                       : 1;
	uint64_t int_en                       : 1;
	uint64_t int_type                     : 1;
	uint64_t fil_cnt                      : 4;
	uint64_t fil_sel                      : 4;
	uint64_t clk_sel                      : 2;
	uint64_t clk_gen                      : 1;
	uint64_t synce_sel                    : 2;
	uint64_t reserved_17_63               : 47;
#endif
	} cn61xx;
	struct cvmx_gpio_bit_cfgx_cn61xx      cn63xx;
	struct cvmx_gpio_bit_cfgx_cn61xx      cn63xxp1;
	struct cvmx_gpio_bit_cfgx_cn61xx      cn66xx;
	struct cvmx_gpio_bit_cfgx_cn61xx      cn68xx;
	struct cvmx_gpio_bit_cfgx_cn61xx      cn68xxp1;
	struct cvmx_gpio_bit_cfgx_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_21_63               : 43;
	uint64_t output_sel                   : 5;  /**< Selects GPIO output. When [TX_OE] is set, selects
                                                         what output data is driven.
                                                           0x0 : TX Normal GPIO output, controlled by GPIO_TX.
                                                           0x1 : PTP_CKOUT, PTP CKOUT; see MIO_PTP_CLOCK_CFG.
                                                           0x2 : PTP_PPS, PTP PPS; see MIO_PTP_CLOCK_CFG.
                                                           0x3+a  :  CLK_SYNCE(0..1) see GPIO_CLK_SYNCE.
                                                           0x5+a  : MCD(0..2) Multi Core Debug output; see TBD
                                                           0x8    : LMC_ECC(0)  LMC ECC error detected.
                                                           0x9-0xf: Reserved.
                                                           0x10+a : GPIO_CLK_GEN(0..3), GPIO clock generator; See GPIO_CLK_GEN.
                                                           0x14   : USB0_VBUS_CTRL  see USB0 Vbus Control.
                                                           0x15   : SATA port0 cold presence power-on device.(p0_cp_pod)
                                                           0x16   : SATA port1 cold presence power-on device.(p1_cp_pod)
                                                           0x17   : SATA port0 LED  (p0_act_led)
                                                           0x18   : SATA port1 LED  (p1_act_led)
                                                           0x19   : USB1_VBUS_CTRL  see USB1 Vbus control.
                                                           0x1a-  : Reserved.
                                                         Note: GPIO[19:10] controls are ignored if PCM is enabled.  See PCM(0..3)_TDM_CFG */
	uint64_t reserved_12_15               : 4;
	uint64_t fil_sel                      : 4;  /**< Filter select. Global counter bit-select (controls sample rate).
                                                         Filter are XOR inverter are also appliable to GPIO input muxing signals and interrupts. */
	uint64_t fil_cnt                      : 4;  /**< Filter count. Specifies the number of consecutive samples (FIL_CNT+1) to change state.
                                                         Zero to disable the filter.
                                                         Filter are XOR inverter are also appliable to GPIO input muxing signals and interrupts. */
	uint64_t int_type                     : 1;  /**< Type of interrupt
                                                         0 = level (default)
                                                         1 = rising edge */
	uint64_t int_en                       : 1;  /**< Bit mask to indicate which bits to raise interrupt */
	uint64_t rx_xor                       : 1;  /**< Invert the GPIO pin */
	uint64_t tx_oe                        : 1;  /**< Drive the GPIO pin as an output pin */
#else
	uint64_t tx_oe                        : 1;
	uint64_t rx_xor                       : 1;
	uint64_t int_en                       : 1;
	uint64_t int_type                     : 1;
	uint64_t fil_cnt                      : 4;
	uint64_t fil_sel                      : 4;
	uint64_t reserved_12_15               : 4;
	uint64_t output_sel                   : 5;
	uint64_t reserved_21_63               : 43;
#endif
	} cn70xx;
	struct cvmx_gpio_bit_cfgx_cn70xx      cn70xxp1;
	struct cvmx_gpio_bit_cfgx_cn70xx      cn73xx;
	struct cvmx_gpio_bit_cfgx_cn70xx      cn78xx;
	struct cvmx_gpio_bit_cfgx_cn70xx      cn78xxp1;
	struct cvmx_gpio_bit_cfgx_cn61xx      cnf71xx;
	struct cvmx_gpio_bit_cfgx_cn70xx      cnf75xx;
};
typedef union cvmx_gpio_bit_cfgx cvmx_gpio_bit_cfgx_t;

/**
 * cvmx_gpio_boot_ena
 */
union cvmx_gpio_boot_ena {
	uint64_t u64;
	struct cvmx_gpio_boot_ena_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_12_63               : 52;
	uint64_t boot_ena                     : 4;  /**< Drive boot bus chip enables [7:4] on gpio [11:8] */
	uint64_t reserved_0_7                 : 8;
#else
	uint64_t reserved_0_7                 : 8;
	uint64_t boot_ena                     : 4;
	uint64_t reserved_12_63               : 52;
#endif
	} s;
	struct cvmx_gpio_boot_ena_s           cn30xx;
	struct cvmx_gpio_boot_ena_s           cn31xx;
	struct cvmx_gpio_boot_ena_s           cn50xx;
};
typedef union cvmx_gpio_boot_ena cvmx_gpio_boot_ena_t;

/**
 * cvmx_gpio_clk_gen#
 */
union cvmx_gpio_clk_genx {
	uint64_t u64;
	struct cvmx_gpio_clk_genx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t n                            : 32; /**< Determines the frequency of the GPIO clock generator. N should be less than or equal to
                                                         2^31-1.
                                                         The frequency of the GPIO clock generator equals the coprocessor-clock frequency times N
                                                         divided by 2^32.
                                                         Writing N = 0x0 stops the clock generator. */
#else
	uint64_t n                            : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_gpio_clk_genx_s           cn52xx;
	struct cvmx_gpio_clk_genx_s           cn52xxp1;
	struct cvmx_gpio_clk_genx_s           cn56xx;
	struct cvmx_gpio_clk_genx_s           cn56xxp1;
	struct cvmx_gpio_clk_genx_s           cn61xx;
	struct cvmx_gpio_clk_genx_s           cn63xx;
	struct cvmx_gpio_clk_genx_s           cn63xxp1;
	struct cvmx_gpio_clk_genx_s           cn66xx;
	struct cvmx_gpio_clk_genx_s           cn68xx;
	struct cvmx_gpio_clk_genx_s           cn68xxp1;
	struct cvmx_gpio_clk_genx_s           cn70xx;
	struct cvmx_gpio_clk_genx_s           cn70xxp1;
	struct cvmx_gpio_clk_genx_s           cn73xx;
	struct cvmx_gpio_clk_genx_s           cn78xx;
	struct cvmx_gpio_clk_genx_s           cn78xxp1;
	struct cvmx_gpio_clk_genx_s           cnf71xx;
	struct cvmx_gpio_clk_genx_s           cnf75xx;
};
typedef union cvmx_gpio_clk_genx cvmx_gpio_clk_genx_t;

/**
 * cvmx_gpio_clk_qlm#
 *
 * Notes:
 * QLM0(A) and QLM1(B) can configured to source any of QLM0 or QLM2 as clock source.
 * Clock speed output for different modes ...
 *
 *                        Speed With      Speed with
 * SERDES speed (Gbaud)   DIV=0 (MHz)     DIV=1 (MHz)
 * **********************************************************
 *      1.25                 62.5            31.25
 *      2.5                 125              62.5
 *      3.125               156.25           78.125
 *      5.0                 250             125
 *      6.25                312.5           156.25
 */
union cvmx_gpio_clk_qlmx {
	uint64_t u64;
	struct cvmx_gpio_clk_qlmx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_11_63               : 53;
	uint64_t qlm_sel                      : 3;  /**< Selects which DLM to select from
                                                         x0 = select DLM0 as clock source
                                                         x1 = Disabled */
	uint64_t reserved_3_7                 : 5;
	uint64_t div                          : 1;  /**< Internal clock divider
                                                         0=DIV2
                                                         1=DIV4 */
	uint64_t lane_sel                     : 2;  /**< Selects which RX lane clock from QLMx to use as
                                                         the GPIO internal QLMx clock.  The GPIO block can
                                                         support upto two unique clocks to send out any
                                                         GPIO pin as configured by $GPIO_BIT_CFG[SYNCE_SEL]
                                                         The clock can either be a divided by 2 or divide
                                                         by 4 of the selected RX lane clock. */
#else
	uint64_t lane_sel                     : 2;
	uint64_t div                          : 1;
	uint64_t reserved_3_7                 : 5;
	uint64_t qlm_sel                      : 3;
	uint64_t reserved_11_63               : 53;
#endif
	} s;
	struct cvmx_gpio_clk_qlmx_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_10_63               : 54;
	uint64_t qlm_sel                      : 2;  /**< Selects which QLM to select from
                                                         01 = select QLM0 as clock source
                                                         1x = select QLM2 as clock source
                                                         0  = Disabled */
	uint64_t reserved_3_7                 : 5;
	uint64_t div                          : 1;  /**< Internal clock divider
                                                         0=DIV2
                                                         1=DIV4 */
	uint64_t lane_sel                     : 2;  /**< Selects which RX lane clock from QLMx to use as
                                                         the GPIO internal QLMx clock.  The GPIO block can
                                                         support upto two unique clocks to send out any
                                                         GPIO pin as configured by $GPIO_BIT_CFG[SYNCE_SEL]
                                                         The clock can either be a divided by 2 or divide
                                                         by 4 of the selected RX lane clock. */
#else
	uint64_t lane_sel                     : 2;
	uint64_t div                          : 1;
	uint64_t reserved_3_7                 : 5;
	uint64_t qlm_sel                      : 2;
	uint64_t reserved_10_63               : 54;
#endif
	} cn61xx;
	struct cvmx_gpio_clk_qlmx_cn63xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_3_63                : 61;
	uint64_t div                          : 1;  /**< Internal clock divider
                                                         0=DIV2
                                                         1=DIV4 */
	uint64_t lane_sel                     : 2;  /**< Selects which RX lane clock from QLM2 to use as
                                                         the GPIO internal QLMx clock.  The GPIO block can
                                                         support upto two unique clocks to send out any
                                                         GPIO pin as configured by $GPIO_BIT_CFG[SYNCE_SEL]
                                                         The clock can either be a divided by 2 or divide
                                                         by 4 of the selected RX lane clock. */
#else
	uint64_t lane_sel                     : 2;
	uint64_t div                          : 1;
	uint64_t reserved_3_63                : 61;
#endif
	} cn63xx;
	struct cvmx_gpio_clk_qlmx_cn63xx      cn63xxp1;
	struct cvmx_gpio_clk_qlmx_cn61xx      cn66xx;
	struct cvmx_gpio_clk_qlmx_s           cn68xx;
	struct cvmx_gpio_clk_qlmx_s           cn68xxp1;
	struct cvmx_gpio_clk_qlmx_cn61xx      cnf71xx;
};
typedef union cvmx_gpio_clk_qlmx cvmx_gpio_clk_qlmx_t;

/**
 * cvmx_gpio_clk_synce#
 *
 * A QLM can be configured as a clock source. The GPIO block can support up to two unique clocks
 * to send out any GPIO pin as configured by GPIO_BIT_CFG()[SYNCE_SEL]. The clock can be
 * divided by 20, 40, 80 or 160 of the selected RX lane clock.
 */
union cvmx_gpio_clk_syncex {
	uint64_t u64;
	struct cvmx_gpio_clk_syncex_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_4_63                : 60;
	uint64_t div                          : 2;  /**< GPIO internal clock divider setting relative to QLM/DLM SERDES CLOCK_SYNCE. The maximum
                                                         supported GPIO output frequency is 125 MHz.
                                                         0x0 = Divide by 20.
                                                         0x1 = Divide by 40.
                                                         0x2 = Divide by 80.
                                                         0x3 = Divide by 160. */
	uint64_t lane_sel                     : 2;  /**< Selects which RX lane clock from QLMx(DLMx) to use as the GPIO internal QLMx(DLMx) clock.
                                                         For DLMx, valid LANE_SEL is 0/1. */
#else
	uint64_t lane_sel                     : 2;
	uint64_t div                          : 2;
	uint64_t reserved_4_63                : 60;
#endif
	} s;
	struct cvmx_gpio_clk_syncex_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_10_63               : 54;
	uint64_t dlm_sel                      : 2;  /**< Selects clock source from
                                                         00 = Disabled
                                                         01 = DLM0
                                                         10 = DLM1
                                                         11 = DLM2 */
	uint64_t reserved_3_7                 : 5;
	uint64_t div                          : 1;  /**< GPIO internal clock divider setting relative to QLM SERDES CLOCK_SYNCE. The maximum
                                                         supported GPIO output frequency is 125 MHz.
                                                         0x0 = Divide by 20.
                                                         0x1 = Divide by 40.
                                                         Speed    DIV20   DIV40
                                                         [GHz]    [MHz]   [MHz]
                                                         1.2500   62.50   31.25
                                                         2.5000   125.00  62.50
                                                         3.1250    ---    78.13
                                                         5.0000    ---    125.00 */
	uint64_t reserved_1_1                 : 1;
	uint64_t lane_sel                     : 1;  /**< Selects which RX lane clock from DLMx to use as
                                                         the GPIO internal DLMx clock.  The GPIO block can
                                                         support upto two unique clocks to send out any
                                                         GPIO pin as configured by
                                                         $GPIO_BIT_CFG[OUTPUT_SEL]
                                                         The clock can either be a divided by 2 or divide
                                                         by 4 of the selected RX lane clock. */
#else
	uint64_t lane_sel                     : 1;
	uint64_t reserved_1_1                 : 1;
	uint64_t div                          : 1;
	uint64_t reserved_3_7                 : 5;
	uint64_t dlm_sel                      : 2;
	uint64_t reserved_10_63               : 54;
#endif
	} cn70xx;
	struct cvmx_gpio_clk_syncex_cn70xx    cn70xxp1;
	struct cvmx_gpio_clk_syncex_cn73xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_12_63               : 52;
	uint64_t qlm_sel                      : 4;  /**< Selects which QLM(0..3) or DLM(4..6) to select from, values 7-15 are invalid. */
	uint64_t reserved_4_7                 : 4;
	uint64_t div                          : 2;  /**< GPIO internal clock divider setting relative to QLM/DLM SERDES CLOCK_SYNCE. The maximum
                                                         supported GPIO output frequency is 125 MHz.
                                                         0x0 = Divide by 20.
                                                         0x1 = Divide by 40.
                                                         0x2 = Divide by 80.
                                                         0x3 = Divide by 160. */
	uint64_t lane_sel                     : 2;  /**< Selects which RX lane clock from QLMx(DLMx) to use as the GPIO internal QLMx(DLMx) clock.
                                                         For DLMx, valid LANE_SEL is 0/1. */
#else
	uint64_t lane_sel                     : 2;
	uint64_t div                          : 2;
	uint64_t reserved_4_7                 : 4;
	uint64_t qlm_sel                      : 4;
	uint64_t reserved_12_63               : 52;
#endif
	} cn73xx;
	struct cvmx_gpio_clk_syncex_cn73xx    cn78xx;
	struct cvmx_gpio_clk_syncex_cn73xx    cn78xxp1;
	struct cvmx_gpio_clk_syncex_cn73xx    cnf75xx;
};
typedef union cvmx_gpio_clk_syncex cvmx_gpio_clk_syncex_t;

/**
 * cvmx_gpio_comp
 */
union cvmx_gpio_comp {
	uint64_t u64;
	struct cvmx_gpio_comp_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_11_63               : 53;
	uint64_t pctl                         : 3;  /**< GPIO bus driver PCTL. Suggested values:
                                                         0x4 = 60 ohm.
                                                         0x6 = 40 ohm.
                                                         0x7 = 30 ohm. */
	uint64_t reserved_3_7                 : 5;
	uint64_t nctl                         : 3;  /**< GPIO bus driver NCTL. Suggested values:
                                                         0x4 = 60 ohm.
                                                         0x6 = 40 ohm.
                                                         0x7 = 30 ohm. */
#else
	uint64_t nctl                         : 3;
	uint64_t reserved_3_7                 : 5;
	uint64_t pctl                         : 3;
	uint64_t reserved_11_63               : 53;
#endif
	} s;
	struct cvmx_gpio_comp_s               cn73xx;
	struct cvmx_gpio_comp_s               cn78xx;
	struct cvmx_gpio_comp_s               cn78xxp1;
	struct cvmx_gpio_comp_s               cnf75xx;
};
typedef union cvmx_gpio_comp cvmx_gpio_comp_t;

/**
 * cvmx_gpio_dbg_ena
 */
union cvmx_gpio_dbg_ena {
	uint64_t u64;
	struct cvmx_gpio_dbg_ena_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_21_63               : 43;
	uint64_t dbg_ena                      : 21; /**< Enable the debug port to be driven on the gpio */
#else
	uint64_t dbg_ena                      : 21;
	uint64_t reserved_21_63               : 43;
#endif
	} s;
	struct cvmx_gpio_dbg_ena_s            cn30xx;
	struct cvmx_gpio_dbg_ena_s            cn31xx;
	struct cvmx_gpio_dbg_ena_s            cn50xx;
};
typedef union cvmx_gpio_dbg_ena cvmx_gpio_dbg_ena_t;

/**
 * cvmx_gpio_int_clr
 *
 * Interrupts are limited to GPIO bits 15:0
 *
 */
union cvmx_gpio_int_clr {
	uint64_t u64;
	struct cvmx_gpio_int_clr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t type                         : 16; /**< Clear the interrupt rising edge detector */
#else
	uint64_t type                         : 16;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_gpio_int_clr_s            cn30xx;
	struct cvmx_gpio_int_clr_s            cn31xx;
	struct cvmx_gpio_int_clr_s            cn38xx;
	struct cvmx_gpio_int_clr_s            cn38xxp2;
	struct cvmx_gpio_int_clr_s            cn50xx;
	struct cvmx_gpio_int_clr_s            cn52xx;
	struct cvmx_gpio_int_clr_s            cn52xxp1;
	struct cvmx_gpio_int_clr_s            cn56xx;
	struct cvmx_gpio_int_clr_s            cn56xxp1;
	struct cvmx_gpio_int_clr_s            cn58xx;
	struct cvmx_gpio_int_clr_s            cn58xxp1;
	struct cvmx_gpio_int_clr_s            cn61xx;
	struct cvmx_gpio_int_clr_s            cn63xx;
	struct cvmx_gpio_int_clr_s            cn63xxp1;
	struct cvmx_gpio_int_clr_s            cn66xx;
	struct cvmx_gpio_int_clr_s            cn68xx;
	struct cvmx_gpio_int_clr_s            cn68xxp1;
	struct cvmx_gpio_int_clr_s            cn70xx;
	struct cvmx_gpio_int_clr_s            cn70xxp1;
	struct cvmx_gpio_int_clr_s            cnf71xx;
};
typedef union cvmx_gpio_int_clr cvmx_gpio_int_clr_t;

/**
 * cvmx_gpio_intr#
 *
 * Each register provides interrupt information for the corresponding GPIO pin.
 *
 */
union cvmx_gpio_intrx {
	uint64_t u64;
	struct cvmx_gpio_intrx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_2_63                : 62;
	uint64_t intr_w1s                     : 1;  /**< GPIO signaled interrupt. If interrupts are edge sensitive, write one to set, otherwise
                                                         will clear automatically when GPIO pin deasserts. Read out value is INTR.
                                                         GPIO_INTR(4..7)[INTR_W1S] can also introduce GPIO_MC_INTR(4..7) when multicast mode is
                                                         enabled. */
	uint64_t intr                         : 1;  /**< GPIO signaled interrupt. If interrupts are edge sensitive, write one to clear, otherwise
                                                         will clear automatically when GPIO pin deasserts. Throws GPIO_INTSN_E::GPIO_INTR(). */
#else
	uint64_t intr                         : 1;
	uint64_t intr_w1s                     : 1;
	uint64_t reserved_2_63                : 62;
#endif
	} s;
	struct cvmx_gpio_intrx_s              cn73xx;
	struct cvmx_gpio_intrx_s              cn78xx;
	struct cvmx_gpio_intrx_cn78xxp1 {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t intr                         : 1;  /**< GPIO signaled interrupt. If interrupts are edge sensitive, write one to clear, otherwise
                                                         will clear automatically when GPIO pin deasserts. Throws GPIO_INTSN_E::GPIO_INTR(). */
#else
	uint64_t intr                         : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} cn78xxp1;
	struct cvmx_gpio_intrx_s              cnf75xx;
};
typedef union cvmx_gpio_intrx cvmx_gpio_intrx_t;

/**
 * cvmx_gpio_mc_intr#
 *
 * Each register provides interrupt multicasting for GPIO(4..7).
 *
 */
union cvmx_gpio_mc_intrx {
	uint64_t u64;
	struct cvmx_gpio_mc_intrx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t intr                         : 48; /**< GPIO interrupt for each core. When corresponding GPIO4-7 is edge-triggered and MILTI_CAST
                                                         is enabled, a GPIO assertion will set all 48 bits. Each bit is expected to be routed to
                                                         interrupt a different core using the CIU, and each core will then write one to clear its
                                                         corresponding bit in this register. Throws GPIO_INTSN_E::GPIO_MC_INTR()_PP(). */
#else
	uint64_t intr                         : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_gpio_mc_intrx_cn73xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t intr                         : 16; /**< GPIO interrupt for each core. When corresponding GPIO4-7 is edge-triggered and MILTI_CAST
                                                         is enabled, a GPIO assertion will set all 48 bits. Each bit is expected to be routed to
                                                         interrupt a different core using the CIU, and each core will then write one to clear its
                                                         corresponding bit in this register. Throws GPIO_INTSN_E::GPIO_MC_INTR()_PP(). */
#else
	uint64_t intr                         : 16;
	uint64_t reserved_16_63               : 48;
#endif
	} cn73xx;
	struct cvmx_gpio_mc_intrx_s           cn78xx;
	struct cvmx_gpio_mc_intrx_s           cn78xxp1;
	struct cvmx_gpio_mc_intrx_cn73xx      cnf75xx;
};
typedef union cvmx_gpio_mc_intrx cvmx_gpio_mc_intrx_t;

/**
 * cvmx_gpio_mc_intr#_w1s
 */
union cvmx_gpio_mc_intrx_w1s {
	uint64_t u64;
	struct cvmx_gpio_mc_intrx_w1s_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t intr                         : 48; /**< GPIO interrupt for each core. When corresponding GPIO4-7 is edge-triggered and MILTI_CAST
                                                         is enabled, a GPIO assertion will set all 48 bits. Each bit is expected to be routed to
                                                         interrupt a different core using the CIU, and each core will then write one to clear its
                                                         corresponding bit in this register. Throws GPIO_INTSN_E::GPIO_MC_INTR()_PP(). */
#else
	uint64_t intr                         : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_gpio_mc_intrx_w1s_cn73xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t intr                         : 16; /**< GPIO interrupt for each core. When corresponding GPIO4-7 is edge-triggered and MILTI_CAST
                                                         is enabled, a GPIO assertion will set all 48 bits. Each bit is expected to be routed to
                                                         interrupt a different core using the CIU, and each core will then write one to clear its
                                                         corresponding bit in this register. Throws GPIO_INTSN_E::GPIO_MC_INTR()_PP(). */
#else
	uint64_t intr                         : 16;
	uint64_t reserved_16_63               : 48;
#endif
	} cn73xx;
	struct cvmx_gpio_mc_intrx_w1s_s       cn78xx;
	struct cvmx_gpio_mc_intrx_w1s_cn73xx  cnf75xx;
};
typedef union cvmx_gpio_mc_intrx_w1s cvmx_gpio_mc_intrx_w1s_t;

/**
 * cvmx_gpio_multi_cast
 *
 * This register enables multicast GPIO interrupts.
 *
 */
union cvmx_gpio_multi_cast {
	uint64_t u64;
	struct cvmx_gpio_multi_cast_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t en                           : 1;  /**< Enable GPIO interrupt multicast mode. When [EN] is set, GPIO<7:4> functions in multicast
                                                         mode allowing these four GPIOs to interrupt multiple cores. Multicast functionality allows
                                                         the GPIO to exist as per-cnMIPS interrupts as opposed to a global interrupt. */
#else
	uint64_t en                           : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_gpio_multi_cast_s         cn61xx;
	struct cvmx_gpio_multi_cast_s         cn70xx;
	struct cvmx_gpio_multi_cast_s         cn70xxp1;
	struct cvmx_gpio_multi_cast_s         cn73xx;
	struct cvmx_gpio_multi_cast_s         cn78xx;
	struct cvmx_gpio_multi_cast_s         cn78xxp1;
	struct cvmx_gpio_multi_cast_s         cnf71xx;
	struct cvmx_gpio_multi_cast_s         cnf75xx;
};
typedef union cvmx_gpio_multi_cast cvmx_gpio_multi_cast_t;

/**
 * cvmx_gpio_ocla_exten_trig
 */
union cvmx_gpio_ocla_exten_trig {
	uint64_t u64;
	struct cvmx_gpio_ocla_exten_trig_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_6_63                : 58;
	uint64_t m_trig                       : 1;  /**< Manual trigger. Used only when SEL=0x1F. */
	uint64_t sel                          : 5;  /**< Selects the GPIO(0..30) input pin to use, or 0x1F for manual trigger. It is used in the
                                                         OCLA coprocessor for GPIO-based triggering. */
#else
	uint64_t sel                          : 5;
	uint64_t m_trig                       : 1;
	uint64_t reserved_6_63                : 58;
#endif
	} s;
	struct cvmx_gpio_ocla_exten_trig_s    cn70xx;
	struct cvmx_gpio_ocla_exten_trig_s    cn70xxp1;
	struct cvmx_gpio_ocla_exten_trig_s    cn73xx;
	struct cvmx_gpio_ocla_exten_trig_s    cn78xx;
	struct cvmx_gpio_ocla_exten_trig_s    cn78xxp1;
	struct cvmx_gpio_ocla_exten_trig_s    cnf75xx;
};
typedef union cvmx_gpio_ocla_exten_trig cvmx_gpio_ocla_exten_trig_t;

/**
 * cvmx_gpio_pin_ena
 *
 * Notes:
 * GPIO0-GPIO17 has dedicated pins.
 * GPIO18 share pin with UART (UART0_CTS_L/GPIO_18), GPIO18 enabled when $GPIO_PIN_ENA[ENA18]=1
 * GPIO19 share pin with UART (UART1_CTS_L/GPIO_19), GPIO18 enabled when $GPIO_PIN_ENA[ENA19]=1
 */
union cvmx_gpio_pin_ena {
	uint64_t u64;
	struct cvmx_gpio_pin_ena_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t ena19                        : 1;  /**< If 0, UART1_CTS_L/GPIO_19 pin is UART pin
                                                         If 1, UART1_CTS_L/GPIO_19 pin is GPIO19 pin */
	uint64_t ena18                        : 1;  /**< If 0, UART0_CTS_L/GPIO_18 pin is UART pin
                                                         If 1, UART0_CTS_L/GPIO_18 pin is GPIO18 pin */
	uint64_t reserved_0_17                : 18;
#else
	uint64_t reserved_0_17                : 18;
	uint64_t ena18                        : 1;
	uint64_t ena19                        : 1;
	uint64_t reserved_20_63               : 44;
#endif
	} s;
	struct cvmx_gpio_pin_ena_s            cn66xx;
};
typedef union cvmx_gpio_pin_ena cvmx_gpio_pin_ena_t;

/**
 * cvmx_gpio_rx_dat
 *
 * This register contains the state of the GPIO pins.
 *
 */
union cvmx_gpio_rx_dat {
	uint64_t u64;
	struct cvmx_gpio_rx_dat_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t dat                          : 32; /**< GPIO read data. */
#else
	uint64_t dat                          : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_gpio_rx_dat_cn30xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_24_63               : 40;
	uint64_t dat                          : 24; /**< GPIO Read Data */
#else
	uint64_t dat                          : 24;
	uint64_t reserved_24_63               : 40;
#endif
	} cn30xx;
	struct cvmx_gpio_rx_dat_cn30xx        cn31xx;
	struct cvmx_gpio_rx_dat_cn38xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t dat                          : 16; /**< GPIO Read Data */
#else
	uint64_t dat                          : 16;
	uint64_t reserved_16_63               : 48;
#endif
	} cn38xx;
	struct cvmx_gpio_rx_dat_cn38xx        cn38xxp2;
	struct cvmx_gpio_rx_dat_cn30xx        cn50xx;
	struct cvmx_gpio_rx_dat_cn38xx        cn52xx;
	struct cvmx_gpio_rx_dat_cn38xx        cn52xxp1;
	struct cvmx_gpio_rx_dat_cn38xx        cn56xx;
	struct cvmx_gpio_rx_dat_cn38xx        cn56xxp1;
	struct cvmx_gpio_rx_dat_cn38xx        cn58xx;
	struct cvmx_gpio_rx_dat_cn38xx        cn58xxp1;
	struct cvmx_gpio_rx_dat_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t dat                          : 20; /**< GPIO Read Data */
#else
	uint64_t dat                          : 20;
	uint64_t reserved_20_63               : 44;
#endif
	} cn61xx;
	struct cvmx_gpio_rx_dat_cn38xx        cn63xx;
	struct cvmx_gpio_rx_dat_cn38xx        cn63xxp1;
	struct cvmx_gpio_rx_dat_cn61xx        cn66xx;
	struct cvmx_gpio_rx_dat_cn38xx        cn68xx;
	struct cvmx_gpio_rx_dat_cn38xx        cn68xxp1;
	struct cvmx_gpio_rx_dat_cn61xx        cn70xx;
	struct cvmx_gpio_rx_dat_cn61xx        cn70xxp1;
	struct cvmx_gpio_rx_dat_s             cn73xx;
	struct cvmx_gpio_rx_dat_cn61xx        cn78xx;
	struct cvmx_gpio_rx_dat_cn61xx        cn78xxp1;
	struct cvmx_gpio_rx_dat_cn61xx        cnf71xx;
	struct cvmx_gpio_rx_dat_s             cnf75xx;
};
typedef union cvmx_gpio_rx_dat cvmx_gpio_rx_dat_t;

/**
 * cvmx_gpio_sata_ctl
 *
 * Select GPIO0-19 received data (GPIO_RX_DAT[DAT]) routing to SATA.
 * The signals sent to SATA from GPIO are filtered and optional inverted.
 */
union cvmx_gpio_sata_ctl {
	uint64_t u64;
	struct cvmx_gpio_sata_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_25_63               : 39;
	uint64_t sel4                         : 5;  /**< Selects the GPIO(0..19) received data (GPIO_RX_DAT[DAT]) for SATA
                                                         port1 cold presence detect(p1_cp_det).
                                                         With SEL values 20-31, signal is always zero. */
	uint64_t sel3                         : 5;  /**< Selects the GPIO(0..19) received data (GPIO_RX_DAT[DAT]) for SATA
                                                         port0 cold presence detect(p0_cp_det).
                                                         With SEL values 20-31, signal is always zero. */
	uint64_t sel2                         : 5;  /**< Selects the GPIO(0..19) received data (GPIO_RX_DAT[DAT]) for SATA
                                                         port1 mechanical presence detect(p1_mp_switch).
                                                         With SEL values 20-31, signal is always zero. */
	uint64_t sel1                         : 5;  /**< Selects the GPIO(0..19) received data (GPIO_RX_DAT[DAT]) for SATA
                                                         port0 mechanical presence detect(p0_mp_switch).
                                                         With SEL values 20-31, signal is always zero. */
	uint64_t sel0                         : 5;  /**< Selects the GPIO(0..19) received data (GPIO_RX_DAT[DAT]) for SATA
                                                         compliance lab loopback testing(lab_lb_pin).
                                                         With SEL values 20-31, signal is always zero. */
#else
	uint64_t sel0                         : 5;
	uint64_t sel1                         : 5;
	uint64_t sel2                         : 5;
	uint64_t sel3                         : 5;
	uint64_t sel4                         : 5;
	uint64_t reserved_25_63               : 39;
#endif
	} s;
	struct cvmx_gpio_sata_ctl_s           cn70xx;
	struct cvmx_gpio_sata_ctl_s           cn70xxp1;
};
typedef union cvmx_gpio_sata_ctl cvmx_gpio_sata_ctl_t;

/**
 * cvmx_gpio_sata_ctl#
 */
union cvmx_gpio_sata_ctlx {
	uint64_t u64;
	struct cvmx_gpio_sata_ctlx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_13_63               : 51;
	uint64_t mp_switch                    : 5;  /**< Selects the GPIO(0..31) input pin for SATA mechanical presence switch input;
                                                         indicated the state of external device presence switch, (0) switch open,
                                                         (1) switch closed. See SATA()_UAHC_p0_CMD[MPSS]. */
	uint64_t reserved_5_7                 : 3;
	uint64_t cp_det                       : 5;  /**< Selects the GPIO(0..31) input pin for SATA cold presence detect input;
                                                         detects addition (1) or removal (0) of the powered-down device;
                                                         see SATA()_UAHC_p0_CMD[CPS]. */
#else
	uint64_t cp_det                       : 5;
	uint64_t reserved_5_7                 : 3;
	uint64_t mp_switch                    : 5;
	uint64_t reserved_13_63               : 51;
#endif
	} s;
	struct cvmx_gpio_sata_ctlx_s          cn73xx;
	struct cvmx_gpio_sata_ctlx_s          cnf75xx;
};
typedef union cvmx_gpio_sata_ctlx cvmx_gpio_sata_ctlx_t;

/**
 * cvmx_gpio_sata_lab_lb
 */
union cvmx_gpio_sata_lab_lb {
	uint64_t u64;
	struct cvmx_gpio_sata_lab_lb_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t sel                          : 5;  /**< Selects the GPIO(0..31) input pin for SATA BIST lab-loopback pin.
                                                         see SATA()_UAHC_GBL_BISTCR[LLB]. */
#else
	uint64_t sel                          : 5;
	uint64_t reserved_5_63                : 59;
#endif
	} s;
	struct cvmx_gpio_sata_lab_lb_s        cn73xx;
	struct cvmx_gpio_sata_lab_lb_s        cnf75xx;
};
typedef union cvmx_gpio_sata_lab_lb cvmx_gpio_sata_lab_lb_t;

/**
 * cvmx_gpio_tim_ctl
 */
union cvmx_gpio_tim_ctl {
	uint64_t u64;
	struct cvmx_gpio_tim_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t sel                          : 5;  /**< Selects the GPIO(0..31) input pin to use in the Timer coprocessor for GPIO-based timing. */
#else
	uint64_t sel                          : 5;
	uint64_t reserved_5_63                : 59;
#endif
	} s;
	struct cvmx_gpio_tim_ctl_cn68xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_4_63                : 60;
	uint64_t sel                          : 4;  /**< Selects the GPIO pin to route to TIM */
#else
	uint64_t sel                          : 4;
	uint64_t reserved_4_63                : 60;
#endif
	} cn68xx;
	struct cvmx_gpio_tim_ctl_cn68xx       cn68xxp1;
	struct cvmx_gpio_tim_ctl_s            cn73xx;
	struct cvmx_gpio_tim_ctl_s            cn78xx;
	struct cvmx_gpio_tim_ctl_s            cn78xxp1;
	struct cvmx_gpio_tim_ctl_s            cnf75xx;
};
typedef union cvmx_gpio_tim_ctl cvmx_gpio_tim_ctl_t;

/**
 * cvmx_gpio_tx_clr
 */
union cvmx_gpio_tx_clr {
	uint64_t u64;
	struct cvmx_gpio_tx_clr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t clr                          : 32; /**< Clear mask. Bit mask to indicate which GPIO_TX_DAT bits to set to 0. When read, [CLR]
                                                         returns the GPIO_TX_DAT storage. */
#else
	uint64_t clr                          : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_gpio_tx_clr_cn30xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_24_63               : 40;
	uint64_t clr                          : 24; /**< Bit mask to indicate which bits to drive to '0'. */
#else
	uint64_t clr                          : 24;
	uint64_t reserved_24_63               : 40;
#endif
	} cn30xx;
	struct cvmx_gpio_tx_clr_cn30xx        cn31xx;
	struct cvmx_gpio_tx_clr_cn38xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t clr                          : 16; /**< Bit mask to indicate which bits to drive to '0'. */
#else
	uint64_t clr                          : 16;
	uint64_t reserved_16_63               : 48;
#endif
	} cn38xx;
	struct cvmx_gpio_tx_clr_cn38xx        cn38xxp2;
	struct cvmx_gpio_tx_clr_cn30xx        cn50xx;
	struct cvmx_gpio_tx_clr_cn38xx        cn52xx;
	struct cvmx_gpio_tx_clr_cn38xx        cn52xxp1;
	struct cvmx_gpio_tx_clr_cn38xx        cn56xx;
	struct cvmx_gpio_tx_clr_cn38xx        cn56xxp1;
	struct cvmx_gpio_tx_clr_cn38xx        cn58xx;
	struct cvmx_gpio_tx_clr_cn38xx        cn58xxp1;
	struct cvmx_gpio_tx_clr_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t clr                          : 20; /**< Bit mask to indicate which GPIO_TX_DAT bits to set
                                                         to '0'. When read, CLR returns the GPIO_TX_DAT
                                                         storage. */
#else
	uint64_t clr                          : 20;
	uint64_t reserved_20_63               : 44;
#endif
	} cn61xx;
	struct cvmx_gpio_tx_clr_cn38xx        cn63xx;
	struct cvmx_gpio_tx_clr_cn38xx        cn63xxp1;
	struct cvmx_gpio_tx_clr_cn61xx        cn66xx;
	struct cvmx_gpio_tx_clr_cn38xx        cn68xx;
	struct cvmx_gpio_tx_clr_cn38xx        cn68xxp1;
	struct cvmx_gpio_tx_clr_cn61xx        cn70xx;
	struct cvmx_gpio_tx_clr_cn61xx        cn70xxp1;
	struct cvmx_gpio_tx_clr_s             cn73xx;
	struct cvmx_gpio_tx_clr_cn61xx        cn78xx;
	struct cvmx_gpio_tx_clr_cn61xx        cn78xxp1;
	struct cvmx_gpio_tx_clr_cn61xx        cnf71xx;
	struct cvmx_gpio_tx_clr_s             cnf75xx;
};
typedef union cvmx_gpio_tx_clr cvmx_gpio_tx_clr_t;

/**
 * cvmx_gpio_tx_set
 */
union cvmx_gpio_tx_set {
	uint64_t u64;
	struct cvmx_gpio_tx_set_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t set                          : 32; /**< Set mask. Bit mask to indicate which GPIO_TX_DAT bits to set to 1. When read, SET returns
                                                         the GPIO_TX_DAT storage. */
#else
	uint64_t set                          : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_gpio_tx_set_cn30xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_24_63               : 40;
	uint64_t set                          : 24; /**< Bit mask to indicate which bits to drive to '1'. */
#else
	uint64_t set                          : 24;
	uint64_t reserved_24_63               : 40;
#endif
	} cn30xx;
	struct cvmx_gpio_tx_set_cn30xx        cn31xx;
	struct cvmx_gpio_tx_set_cn38xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t set                          : 16; /**< Bit mask to indicate which bits to drive to '1'. */
#else
	uint64_t set                          : 16;
	uint64_t reserved_16_63               : 48;
#endif
	} cn38xx;
	struct cvmx_gpio_tx_set_cn38xx        cn38xxp2;
	struct cvmx_gpio_tx_set_cn30xx        cn50xx;
	struct cvmx_gpio_tx_set_cn38xx        cn52xx;
	struct cvmx_gpio_tx_set_cn38xx        cn52xxp1;
	struct cvmx_gpio_tx_set_cn38xx        cn56xx;
	struct cvmx_gpio_tx_set_cn38xx        cn56xxp1;
	struct cvmx_gpio_tx_set_cn38xx        cn58xx;
	struct cvmx_gpio_tx_set_cn38xx        cn58xxp1;
	struct cvmx_gpio_tx_set_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t set                          : 20; /**< Bit mask to indicate which GPIO_TX_DAT bits to set
                                                         to '1'. When read, SET returns the GPIO_TX_DAT
                                                         storage. */
#else
	uint64_t set                          : 20;
	uint64_t reserved_20_63               : 44;
#endif
	} cn61xx;
	struct cvmx_gpio_tx_set_cn38xx        cn63xx;
	struct cvmx_gpio_tx_set_cn38xx        cn63xxp1;
	struct cvmx_gpio_tx_set_cn61xx        cn66xx;
	struct cvmx_gpio_tx_set_cn38xx        cn68xx;
	struct cvmx_gpio_tx_set_cn38xx        cn68xxp1;
	struct cvmx_gpio_tx_set_cn61xx        cn70xx;
	struct cvmx_gpio_tx_set_cn61xx        cn70xxp1;
	struct cvmx_gpio_tx_set_s             cn73xx;
	struct cvmx_gpio_tx_set_cn61xx        cn78xx;
	struct cvmx_gpio_tx_set_cn61xx        cn78xxp1;
	struct cvmx_gpio_tx_set_cn61xx        cnf71xx;
	struct cvmx_gpio_tx_set_s             cnf75xx;
};
typedef union cvmx_gpio_tx_set cvmx_gpio_tx_set_t;

/**
 * cvmx_gpio_usbdrd_ctl#
 */
union cvmx_gpio_usbdrd_ctlx {
	uint64_t u64;
	struct cvmx_gpio_usbdrd_ctlx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t sel                          : 5;  /**< Selects the GPIO(0..31) input pin for USBDRD over-current control. */
#else
	uint64_t sel                          : 5;
	uint64_t reserved_5_63                : 59;
#endif
	} s;
	struct cvmx_gpio_usbdrd_ctlx_s        cn73xx;
	struct cvmx_gpio_usbdrd_ctlx_s        cnf75xx;
};
typedef union cvmx_gpio_usbdrd_ctlx cvmx_gpio_usbdrd_ctlx_t;

/**
 * cvmx_gpio_usbh_ctl
 *
 * Select GPIO0-19 received data (GPIO_RX_DAT[DAT]) routing to USB.
 * The signals sent to USB from GPIO are filtered and optional inverted.
 */
union cvmx_gpio_usbh_ctl {
	uint64_t u64;
	struct cvmx_gpio_usbh_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_0_63                : 64;
#else
	uint64_t reserved_0_63                : 64;
#endif
	} s;
	struct cvmx_gpio_usbh_ctl_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_13_63               : 51;
	uint64_t sel                          : 5;  /**< Selects the GPIO(0..19) received data (GPIO_RX_DAT[DAT]) for USB0
                                                         over-current control. With SEL0 values 20-31, signal is always zero.
                                                         CSR read out for bit 12..8 will have SEL1(4..0) value. */
	uint64_t reserved_5_7                 : 3;
	uint64_t sel1                         : 5;  /**< Selects the GPIO(0..19) received data (GPIO_RX_DAT[DAT]) USB1
                                                         over-current control. With SEL1 values 20-31, signal is always zero.
                                                         CSR read out for bit 4..0 will have SEL(12..8) value. */
#else
	uint64_t sel1                         : 5;
	uint64_t reserved_5_7                 : 3;
	uint64_t sel                          : 5;
	uint64_t reserved_13_63               : 51;
#endif
	} cn70xx;
	struct cvmx_gpio_usbh_ctl_cn70xx      cn70xxp1;
	struct cvmx_gpio_usbh_ctl_cn78xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t sel                          : 5;  /**< Selects the GPIO(0..19) input pin for USBH over-current control. With SEL values 20-31,
                                                         signal is always zero. */
#else
	uint64_t sel                          : 5;
	uint64_t reserved_5_63                : 59;
#endif
	} cn78xx;
	struct cvmx_gpio_usbh_ctl_cn78xx      cn78xxp1;
};
typedef union cvmx_gpio_usbh_ctl cvmx_gpio_usbh_ctl_t;

/**
 * cvmx_gpio_xbit_cfg#
 *
 * Notes:
 * Only first 16 GPIO pins can introduce interrupts, GPIO_XBIT_CFG16(17,18,19)[INT_EN] and [INT_TYPE]
 * will not be used, read out always zero.
 */
union cvmx_gpio_xbit_cfgx {
	uint64_t u64;
	struct cvmx_gpio_xbit_cfgx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_15_63               : 49;
	uint64_t clk_gen                      : 1;  /**< When TX_OE is set, GPIO pin becomes a clock */
	uint64_t clk_sel                      : 2;  /**< Selects which of the 4 GPIO clock generators */
	uint64_t fil_sel                      : 4;  /**< Global counter bit-select (controls sample rate) */
	uint64_t fil_cnt                      : 4;  /**< Number of consecutive samples to change state */
	uint64_t int_type                     : 1;  /**< Type of interrupt
                                                         0 = level (default)
                                                         1 = rising edge */
	uint64_t int_en                       : 1;  /**< Bit mask to indicate which bits to raise interrupt */
	uint64_t rx_xor                       : 1;  /**< Invert the GPIO pin */
	uint64_t tx_oe                        : 1;  /**< Drive the GPIO pin as an output pin */
#else
	uint64_t tx_oe                        : 1;
	uint64_t rx_xor                       : 1;
	uint64_t int_en                       : 1;
	uint64_t int_type                     : 1;
	uint64_t fil_cnt                      : 4;
	uint64_t fil_sel                      : 4;
	uint64_t clk_sel                      : 2;
	uint64_t clk_gen                      : 1;
	uint64_t reserved_15_63               : 49;
#endif
	} s;
	struct cvmx_gpio_xbit_cfgx_cn30xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_12_63               : 52;
	uint64_t fil_sel                      : 4;  /**< Global counter bit-select (controls sample rate) */
	uint64_t fil_cnt                      : 4;  /**< Number of consecutive samples to change state */
	uint64_t reserved_2_3                 : 2;
	uint64_t rx_xor                       : 1;  /**< Invert the GPIO pin */
	uint64_t tx_oe                        : 1;  /**< Drive the GPIO pin as an output pin */
#else
	uint64_t tx_oe                        : 1;
	uint64_t rx_xor                       : 1;
	uint64_t reserved_2_3                 : 2;
	uint64_t fil_cnt                      : 4;
	uint64_t fil_sel                      : 4;
	uint64_t reserved_12_63               : 52;
#endif
	} cn30xx;
	struct cvmx_gpio_xbit_cfgx_cn30xx     cn31xx;
	struct cvmx_gpio_xbit_cfgx_cn30xx     cn50xx;
	struct cvmx_gpio_xbit_cfgx_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_17_63               : 47;
	uint64_t synce_sel                    : 2;  /**< Selects the QLM clock output
                                                         x0=Normal GPIO output
                                                         01=GPIO QLM clock selected by CSR GPIO_CLK_QLM0
                                                         11=GPIO QLM clock selected by CSR GPIO_CLK_QLM1 */
	uint64_t clk_gen                      : 1;  /**< When TX_OE is set, GPIO pin becomes a clock */
	uint64_t clk_sel                      : 2;  /**< Selects which of the 4 GPIO clock generators */
	uint64_t fil_sel                      : 4;  /**< Global counter bit-select (controls sample rate) */
	uint64_t fil_cnt                      : 4;  /**< Number of consecutive samples to change state */
	uint64_t int_type                     : 1;  /**< Type of interrupt
                                                         0 = level (default)
                                                         1 = rising edge */
	uint64_t int_en                       : 1;  /**< Bit mask to indicate which bits to raise interrupt */
	uint64_t rx_xor                       : 1;  /**< Invert the GPIO pin */
	uint64_t tx_oe                        : 1;  /**< Drive the GPIO pin as an output pin */
#else
	uint64_t tx_oe                        : 1;
	uint64_t rx_xor                       : 1;
	uint64_t int_en                       : 1;
	uint64_t int_type                     : 1;
	uint64_t fil_cnt                      : 4;
	uint64_t fil_sel                      : 4;
	uint64_t clk_sel                      : 2;
	uint64_t clk_gen                      : 1;
	uint64_t synce_sel                    : 2;
	uint64_t reserved_17_63               : 47;
#endif
	} cn61xx;
	struct cvmx_gpio_xbit_cfgx_cn61xx     cn66xx;
	struct cvmx_gpio_xbit_cfgx_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_21_63               : 43;
	uint64_t output_sel                   : 5;  /**< Selects GPIO output. When [TX_OE] is set, selects
                                                         what output data is driven.
                                                           0x0 : TX Normal GPIO output, controlled by GPIO_TX.
                                                           0x1 : PTP_CKOUT, PTP CKOUT; see MIO_PTP_CLOCK_CFG.
                                                           0x2 : PTP_PPS, PTP PPS; see MIO_PTP_CLOCK_CFG.
                                                           0x3+a  :  CLK_SYNCE(0..1) see GPIO_CLK_SYNCE.
                                                           0x5+a  : MCD(0..2) Multi Core Debug output; see TBD
                                                           0x8    : LMC_ECC(0)  LMC ECC error detected.
                                                           0x9-0xf: Reserved.
                                                           0x10+a : GPIO_CLK_GEN(0..3), GPIO clock generator; See GPIO_CLK_GEN.
                                                           0x14   : USB0_VBUS_CTRL  see USB0 Vbus Control.
                                                           0x15   : SATA port0 cold presence power-on device.(p0_cp_pod)
                                                           0x16   : SATA port1 cold presence power-on device.(p1_cp_pod)
                                                           0x17   : SATA port0 LED  (p0_act_led)
                                                           0x18   : SATA port1 LED  (p1_act_led)
                                                           0x19   : USB1_VBUS_CTRL  see USB1 Vbus control.
                                                           0x1a-  : Reserved.
                                                         Note: GPIO[19:10] controls are ignored if PCM is enabled.  See PCM(0..3)_TDM_CFG */
	uint64_t reserved_12_15               : 4;
	uint64_t fil_sel                      : 4;  /**< Filter select. Global counter bit-select (controls sample rate).
                                                         Filter are XOR inverter are also appliable to GPIO input muxing signals and interrupts. */
	uint64_t fil_cnt                      : 4;  /**< Filter count. Specifies the number of consecutive samples (FIL_CNT+1) to change state.
                                                         Zero to disable the filter.
                                                         Filter are XOR inverter are also appliable to GPIO input muxing signals and interrupts. */
	uint64_t int_type                     : 1;  /**< Type of interrupt
                                                         0 = level (default)
                                                         1 = rising edge */
	uint64_t int_en                       : 1;  /**< Bit mask to indicate which bits to raise interrupt */
	uint64_t rx_xor                       : 1;  /**< Invert the GPIO pin */
	uint64_t tx_oe                        : 1;  /**< Drive the GPIO pin as an output pin */
#else
	uint64_t tx_oe                        : 1;
	uint64_t rx_xor                       : 1;
	uint64_t int_en                       : 1;
	uint64_t int_type                     : 1;
	uint64_t fil_cnt                      : 4;
	uint64_t fil_sel                      : 4;
	uint64_t reserved_12_15               : 4;
	uint64_t output_sel                   : 5;
	uint64_t reserved_21_63               : 43;
#endif
	} cn70xx;
	struct cvmx_gpio_xbit_cfgx_cn70xx     cn70xxp1;
	struct cvmx_gpio_xbit_cfgx_cn61xx     cnf71xx;
};
typedef union cvmx_gpio_xbit_cfgx cvmx_gpio_xbit_cfgx_t;

#endif

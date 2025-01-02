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
 * cvmx-mpi-defs.h
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon mpi.
 *
 * This file is auto generated. Do not edit.
 *
 * <hr>$Revision$<hr>
 *
 */
#ifndef __CVMX_MPI_DEFS_H__
#define __CVMX_MPI_DEFS_H__

#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_MPI_CFG CVMX_MPI_CFG_FUNC()
static inline uint64_t CVMX_MPI_CFG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN30XX) || OCTEON_IS_MODEL(OCTEON_CN31XX) || OCTEON_IS_MODEL(OCTEON_CN50XX) || OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_MPI_CFG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001070000001000ull);
}
#else
#define CVMX_MPI_CFG (CVMX_ADD_IO_SEG(0x0001070000001000ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_MPI_DATX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN30XX) && ((offset <= 8))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN31XX) && ((offset <= 8))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN50XX) && ((offset <= 8))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 8))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 8))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 8))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 8))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 8))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 8))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 8)))))
		cvmx_warn("CVMX_MPI_DATX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001070000001080ull) + ((offset) & 15) * 8;
}
#else
#define CVMX_MPI_DATX(offset) (CVMX_ADD_IO_SEG(0x0001070000001080ull) + ((offset) & 15) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_MPI_STS CVMX_MPI_STS_FUNC()
static inline uint64_t CVMX_MPI_STS_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN30XX) || OCTEON_IS_MODEL(OCTEON_CN31XX) || OCTEON_IS_MODEL(OCTEON_CN50XX) || OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_MPI_STS not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001070000001008ull);
}
#else
#define CVMX_MPI_STS (CVMX_ADD_IO_SEG(0x0001070000001008ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_MPI_STS_W1S CVMX_MPI_STS_W1S_FUNC()
static inline uint64_t CVMX_MPI_STS_W1S_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_MPI_STS_W1S not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001070000001018ull);
}
#else
#define CVMX_MPI_STS_W1S (CVMX_ADD_IO_SEG(0x0001070000001018ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_MPI_TX CVMX_MPI_TX_FUNC()
static inline uint64_t CVMX_MPI_TX_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN30XX) || OCTEON_IS_MODEL(OCTEON_CN31XX) || OCTEON_IS_MODEL(OCTEON_CN50XX) || OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_MPI_TX not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001070000001010ull);
}
#else
#define CVMX_MPI_TX (CVMX_ADD_IO_SEG(0x0001070000001010ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_MPI_WIDE_DAT CVMX_MPI_WIDE_DAT_FUNC()
static inline uint64_t CVMX_MPI_WIDE_DAT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_MPI_WIDE_DAT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001070000001040ull);
}
#else
#define CVMX_MPI_WIDE_DAT (CVMX_ADD_IO_SEG(0x0001070000001040ull))
#endif

/**
 * cvmx_mpi_cfg
 *
 * This register provides configuration for the MPI/SPI interface. Note that some of the MPI/SPI
 * pins are multiplexed with UART and BOOT pins.
 * SPI_CK MPI/SPI clock; dedicated pin.
 * SPI_DI MPI/SPI input; shared with UART0_DCD_L/SPI_DI; enabled when MPI_CFG[ENABLE]=1.
 * SPI_DO MPI/SPI output; mux to UART0_DTR_L/SPI_DO; enabled when MPI_CFG[ENABLE]=1.
 * SPI_CSn_L MPI/SPI output chip select; enabled when MPI_CFG[ENABLE]=1 and MPI_CFG[CSENAn]=1.
 */
union cvmx_mpi_cfg {
	uint64_t u64;
	struct cvmx_mpi_cfg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_29_63               : 35;
	uint64_t clkdiv                       : 13; /**< Fspi_clk = Fsclk / (2 * CLKDIV)                    |          NS
                                                         CLKDIV = Fsclk / (2 * Fspi_clk) */
	uint64_t csena3                       : 1;  /**< SPI_CS3_L enable:
                                                         0 = UART1_RTS_L/SPI_CS3_L pin is UART pin.
                                                         1 = UART1_RTS_L/SPI_CS3_L pin is MPI/SPI pin.
                                                         SPI_CS3_L drives UART1_RTS_L/SPI_CS3_L. */
	uint64_t csena2                       : 1;  /**< SPI_CS2_L enable:
                                                         0 = UART1_CTS_L/SPI_CS2_L pin is UART pin.
                                                         1 = UART1_CTS_L/SPI_CS2_L pin is MPI/SPI pin.
                                                         SPI_CS2_L drives UART1_CTS_L/SPI_CS2_L. */
	uint64_t csena1                       : 1;  /**< If 0, BOOT_CE_N<7>/SPI_CS1_L pin is BOOT pin       |          NS
                                                         1, BOOT_CE_N<7>/SPI_CS1_L pin is SPI pin
                                                            SPI_CS1_L drives BOOT_CE_N<7>/SPI_CS1_L */
	uint64_t csena0                       : 1;  /**< If 0, BOOT_CE_N<6>/SPI_CS0_L pin is BOOT pin       |          NS
                                                         1, BOOT_CE_N<6>/SPI_CS0_L pin is SPI pin
                                                            SPI_CS0_L drives BOOT_CE_N<6>/SPI_CS0_L */
	uint64_t cslate                       : 1;  /**< If 0, SPI_CS asserts 1/2 SCLK before transaction   |          NS
                                                            1, SPI_CS assert coincident with transaction
                                                         NOTE: This control apply for 2 CSs */
	uint64_t tritx                        : 1;  /**< If 0, SPI_DO pin is driven when slave is not       |          NS
                                                               expected to be driving
                                                            1, SPI_DO pin is tristated when not transmitting
                                                         NOTE: only used when WIREOR==1 */
	uint64_t idleclks                     : 2;  /**< Guarantee IDLECLKS idle sclk cycles between        |          NS
                                                         commands. */
	uint64_t cshi                         : 1;  /**< If 0, CS is low asserted                           |          NS
                                                         1, CS is high asserted */
	uint64_t csena                        : 1;  /**< If 0, the MPI_CS is a GPIO, not used by MPI_TX
                                                         1, CS is driven per MPI_TX intruction */
	uint64_t int_ena                      : 1;  /**< If 0, polling is required                          |          NS
                                                         1, MPI engine interrupts X end of transaction */
	uint64_t lsbfirst                     : 1;  /**< If 0, shift MSB first                              |          NS
                                                         1, shift LSB first */
	uint64_t wireor                       : 1;  /**< If 0, SPI_DO and SPI_DI are separate wires (SPI)   |          NS
                                                               SPI_DO pin is always driven
                                                            1, SPI_DO/DI is all from SPI_DO pin (MPI)
                                                               SPI_DO pin is tristated when not transmitting
                                                         NOTE: if WIREOR==1, SPI_DI pin is not used by the
                                                               MPI engine */
	uint64_t clk_cont                     : 1;  /**< If 0, clock idles to value given by IDLELO after   |          NS
                                                            completion of MPI transaction
                                                         1, clock never idles, requires CS deassertion
                                                            assertion between commands */
	uint64_t idlelo                       : 1;  /**< If 0, SPI_CLK idles high, 1st transition is hi->lo |          NS
                                                         1, SPI_CLK idles low, 1st transition is lo->hi */
	uint64_t enable                       : 1;  /**< If 0, UART0_DTR_L/SPI_DO, UART0_DCD_L/SPI_DI       |          NS
                                                            BOOT_CE_N<7:6>/SPI_CSx_L
                                                            pins are UART/BOOT pins
                                                         1, UART0_DTR_L/SPI_DO and UART0_DCD_L/SPI_DI
                                                            pins are SPI/MPI pins.
                                                            BOOT_CE_N<6>/SPI_CS0_L is SPI pin if CSENA0=1
                                                            BOOT_CE_N<7>/SPI_CS1_L is SPI pin if CSENA1=1 */
#else
	uint64_t enable                       : 1;
	uint64_t idlelo                       : 1;
	uint64_t clk_cont                     : 1;
	uint64_t wireor                       : 1;
	uint64_t lsbfirst                     : 1;
	uint64_t int_ena                      : 1;
	uint64_t csena                        : 1;
	uint64_t cshi                         : 1;
	uint64_t idleclks                     : 2;
	uint64_t tritx                        : 1;
	uint64_t cslate                       : 1;
	uint64_t csena0                       : 1;
	uint64_t csena1                       : 1;
	uint64_t csena2                       : 1;
	uint64_t csena3                       : 1;
	uint64_t clkdiv                       : 13;
	uint64_t reserved_29_63               : 35;
#endif
	} s;
	struct cvmx_mpi_cfg_cn30xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_29_63               : 35;
	uint64_t clkdiv                       : 13; /**< Fsclk = Feclk / (2 * CLKDIV)
                                                         CLKDIV = Feclk / (2 * Fsclk) */
	uint64_t reserved_12_15               : 4;
	uint64_t cslate                       : 1;  /**< If 0, MPI_CS asserts 1/2 SCLK before transaction
                                                            1, MPI_CS assert coincident with transaction
                                                         NOTE: only used if CSENA == 1 */
	uint64_t tritx                        : 1;  /**< If 0, MPI_TX pin is driven when slave is not
                                                               expected to be driving
                                                            1, MPI_TX pin is tristated when not transmitting
                                                         NOTE: only used when WIREOR==1 */
	uint64_t idleclks                     : 2;  /**< Guarantee IDLECLKS idle sclk cycles between
                                                         commands. */
	uint64_t cshi                         : 1;  /**< If 0, CS is low asserted
                                                         1, CS is high asserted */
	uint64_t csena                        : 1;  /**< If 0, the MPI_CS is a GPIO, not used by MPI_TX
                                                         1, CS is driven per MPI_TX intruction */
	uint64_t int_ena                      : 1;  /**< If 0, polling is required
                                                         1, MPI engine interrupts X end of transaction */
	uint64_t lsbfirst                     : 1;  /**< If 0, shift MSB first
                                                         1, shift LSB first */
	uint64_t wireor                       : 1;  /**< If 0, MPI_TX and MPI_RX are separate wires (SPI)
                                                               MPI_TX pin is always driven
                                                            1, MPI_TX/RX is all from MPI_TX pin (MPI)
                                                               MPI_TX pin is tristated when not transmitting
                                                         NOTE: if WIREOR==1, MPI_RX pin is not used by the
                                                               MPI engine */
	uint64_t clk_cont                     : 1;  /**< If 0, clock idles to value given by IDLELO after
                                                            completion of MPI transaction
                                                         1, clock never idles, requires CS deassertion
                                                            assertion between commands */
	uint64_t idlelo                       : 1;  /**< If 0, MPI_CLK idles high, 1st transition is hi->lo
                                                         1, MPI_CLK idles low, 1st transition is lo->hi */
	uint64_t enable                       : 1;  /**< If 0, all MPI pins are GPIOs
                                                         1, MPI_CLK, MPI_CS, and MPI_TX are driven */
#else
	uint64_t enable                       : 1;
	uint64_t idlelo                       : 1;
	uint64_t clk_cont                     : 1;
	uint64_t wireor                       : 1;
	uint64_t lsbfirst                     : 1;
	uint64_t int_ena                      : 1;
	uint64_t csena                        : 1;
	uint64_t cshi                         : 1;
	uint64_t idleclks                     : 2;
	uint64_t tritx                        : 1;
	uint64_t cslate                       : 1;
	uint64_t reserved_12_15               : 4;
	uint64_t clkdiv                       : 13;
	uint64_t reserved_29_63               : 35;
#endif
	} cn30xx;
	struct cvmx_mpi_cfg_cn31xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_29_63               : 35;
	uint64_t clkdiv                       : 13; /**< Fsclk = Feclk / (2 * CLKDIV)
                                                         CLKDIV = Feclk / (2 * Fsclk) */
	uint64_t reserved_11_15               : 5;
	uint64_t tritx                        : 1;  /**< If 0, MPI_TX pin is driven when slave is not
                                                               expected to be driving
                                                            1, MPI_TX pin is tristated when not transmitting
                                                         NOTE: only used when WIREOR==1 */
	uint64_t idleclks                     : 2;  /**< Guarantee IDLECLKS idle sclk cycles between
                                                         commands. */
	uint64_t cshi                         : 1;  /**< If 0, CS is low asserted
                                                         1, CS is high asserted */
	uint64_t csena                        : 1;  /**< If 0, the MPI_CS is a GPIO, not used by MPI_TX
                                                         1, CS is driven per MPI_TX intruction */
	uint64_t int_ena                      : 1;  /**< If 0, polling is required
                                                         1, MPI engine interrupts X end of transaction */
	uint64_t lsbfirst                     : 1;  /**< If 0, shift MSB first
                                                         1, shift LSB first */
	uint64_t wireor                       : 1;  /**< If 0, MPI_TX and MPI_RX are separate wires (SPI)
                                                               MPI_TX pin is always driven
                                                            1, MPI_TX/RX is all from MPI_TX pin (MPI)
                                                               MPI_TX pin is tristated when not transmitting
                                                         NOTE: if WIREOR==1, MPI_RX pin is not used by the
                                                               MPI engine */
	uint64_t clk_cont                     : 1;  /**< If 0, clock idles to value given by IDLELO after
                                                            completion of MPI transaction
                                                         1, clock never idles, requires CS deassertion
                                                            assertion between commands */
	uint64_t idlelo                       : 1;  /**< If 0, MPI_CLK idles high, 1st transition is hi->lo
                                                         1, MPI_CLK idles low, 1st transition is lo->hi */
	uint64_t enable                       : 1;  /**< If 0, all MPI pins are GPIOs
                                                         1, MPI_CLK, MPI_CS, and MPI_TX are driven */
#else
	uint64_t enable                       : 1;
	uint64_t idlelo                       : 1;
	uint64_t clk_cont                     : 1;
	uint64_t wireor                       : 1;
	uint64_t lsbfirst                     : 1;
	uint64_t int_ena                      : 1;
	uint64_t csena                        : 1;
	uint64_t cshi                         : 1;
	uint64_t idleclks                     : 2;
	uint64_t tritx                        : 1;
	uint64_t reserved_11_15               : 5;
	uint64_t clkdiv                       : 13;
	uint64_t reserved_29_63               : 35;
#endif
	} cn31xx;
	struct cvmx_mpi_cfg_cn30xx            cn50xx;
	struct cvmx_mpi_cfg_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_29_63               : 35;
	uint64_t clkdiv                       : 13; /**< Fspi_clk = Fsclk / (2 * CLKDIV)                    |          NS
                                                         CLKDIV = Fsclk / (2 * Fspi_clk) */
	uint64_t reserved_14_15               : 2;
	uint64_t csena1                       : 1;  /**< If 0, BOOT_CE_N<7>/SPI_CS1_L pin is BOOT pin       |          NS
                                                         1, BOOT_CE_N<7>/SPI_CS1_L pin is SPI pin
                                                            SPI_CS1_L drives BOOT_CE_N<7>/SPI_CS1_L */
	uint64_t csena0                       : 1;  /**< If 0, BOOT_CE_N<6>/SPI_CS0_L pin is BOOT pin       |          NS
                                                         1, BOOT_CE_N<6>/SPI_CS0_L pin is SPI pin
                                                            SPI_CS0_L drives BOOT_CE_N<6>/SPI_CS0_L */
	uint64_t cslate                       : 1;  /**< If 0, SPI_CS asserts 1/2 SCLK before transaction   |          NS
                                                            1, SPI_CS assert coincident with transaction
                                                         NOTE: This control apply for 2 CSs */
	uint64_t tritx                        : 1;  /**< If 0, SPI_DO pin is driven when slave is not       |          NS
                                                               expected to be driving
                                                            1, SPI_DO pin is tristated when not transmitting
                                                         NOTE: only used when WIREOR==1 */
	uint64_t idleclks                     : 2;  /**< Guarantee IDLECLKS idle sclk cycles between        |          NS
                                                         commands. */
	uint64_t cshi                         : 1;  /**< If 0, CS is low asserted                           |          NS
                                                         1, CS is high asserted */
	uint64_t reserved_6_6                 : 1;
	uint64_t int_ena                      : 1;  /**< If 0, polling is required                          |          NS
                                                         1, MPI engine interrupts X end of transaction */
	uint64_t lsbfirst                     : 1;  /**< If 0, shift MSB first                              |          NS
                                                         1, shift LSB first */
	uint64_t wireor                       : 1;  /**< If 0, SPI_DO and SPI_DI are separate wires (SPI)   |          NS
                                                               SPI_DO pin is always driven
                                                            1, SPI_DO/DI is all from SPI_DO pin (MPI)
                                                               SPI_DO pin is tristated when not transmitting
                                                         NOTE: if WIREOR==1, SPI_DI pin is not used by the
                                                               MPI engine */
	uint64_t clk_cont                     : 1;  /**< If 0, clock idles to value given by IDLELO after   |          NS
                                                            completion of MPI transaction
                                                         1, clock never idles, requires CS deassertion
                                                            assertion between commands */
	uint64_t idlelo                       : 1;  /**< If 0, SPI_CLK idles high, 1st transition is hi->lo |          NS
                                                         1, SPI_CLK idles low, 1st transition is lo->hi */
	uint64_t enable                       : 1;  /**< If 0, UART0_DTR_L/SPI_DO, UART0_DCD_L/SPI_DI       |          NS
                                                            BOOT_CE_N<7:6>/SPI_CSx_L
                                                            pins are UART/BOOT pins
                                                         1, UART0_DTR_L/SPI_DO and UART0_DCD_L/SPI_DI
                                                            pins are SPI/MPI pins.
                                                            BOOT_CE_N<6>/SPI_CS0_L is SPI pin if CSENA0=1
                                                            BOOT_CE_N<7>/SPI_CS1_L is SPI pin if CSENA1=1 */
#else
	uint64_t enable                       : 1;
	uint64_t idlelo                       : 1;
	uint64_t clk_cont                     : 1;
	uint64_t wireor                       : 1;
	uint64_t lsbfirst                     : 1;
	uint64_t int_ena                      : 1;
	uint64_t reserved_6_6                 : 1;
	uint64_t cshi                         : 1;
	uint64_t idleclks                     : 2;
	uint64_t tritx                        : 1;
	uint64_t cslate                       : 1;
	uint64_t csena0                       : 1;
	uint64_t csena1                       : 1;
	uint64_t reserved_14_15               : 2;
	uint64_t clkdiv                       : 13;
	uint64_t reserved_29_63               : 35;
#endif
	} cn61xx;
	struct cvmx_mpi_cfg_cn66xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_29_63               : 35;
	uint64_t clkdiv                       : 13; /**< Fspi_clk = Fsclk / (2 * CLKDIV)                    |          NS
                                                         CLKDIV = Fsclk / (2 * Fspi_clk) */
	uint64_t csena3                       : 1;  /**< If 0, UART1_RTS_L/SPI_CS3_L pin is UART pin        |          NS
                                                         1, UART1_RTS_L/SPI_CS3_L pin is SPI pin
                                                            SPI_CS3_L drives UART1_RTS_L/SPI_CS3_L */
	uint64_t csena2                       : 1;  /**< If 0, UART0_RTS_L/SPI_CS2_L pin is UART pin        |          NS
                                                         1, UART0_RTS_L/SPI_CS2_L pin is SPI pin
                                                            SPI_CS2_L drives  UART0_RTS_L/SPI_CS2_L */
	uint64_t reserved_12_13               : 2;
	uint64_t cslate                       : 1;  /**< If 0, SPI_CS asserts 1/2 SCLK before transaction   |          NS
                                                            1, SPI_CS assert coincident with transaction
                                                         NOTE: This control apply for 4 CSs */
	uint64_t tritx                        : 1;  /**< If 0, SPI_DO pin is driven when slave is not       |          NS
                                                               expected to be driving
                                                            1, SPI_DO pin is tristated when not transmitting
                                                         NOTE: only used when WIREOR==1 */
	uint64_t idleclks                     : 2;  /**< Guarantee IDLECLKS idle sclk cycles between        |          NS
                                                         commands. */
	uint64_t cshi                         : 1;  /**< If 0, CS is low asserted                           |          NS
                                                         1, CS is high asserted */
	uint64_t reserved_6_6                 : 1;
	uint64_t int_ena                      : 1;  /**< If 0, polling is required                          |          NS
                                                         1, MPI engine interrupts X end of transaction */
	uint64_t lsbfirst                     : 1;  /**< If 0, shift MSB first                              |          NS
                                                         1, shift LSB first */
	uint64_t wireor                       : 1;  /**< If 0, SPI_DO and SPI_DI are separate wires (SPI)   |          NS
                                                               SPI_DO pin is always driven
                                                            1, SPI_DO/DI is all from SPI_DO pin (MPI)
                                                               SPI_DO pin is tristated when not transmitting
                                                         NOTE: if WIREOR==1, SPI_DI pin is not used by the
                                                               MPI engine */
	uint64_t clk_cont                     : 1;  /**< If 0, clock idles to value given by IDLELO after   |          NS
                                                            completion of MPI transaction
                                                         1, clock never idles, requires CS deassertion
                                                            assertion between commands */
	uint64_t idlelo                       : 1;  /**< If 0, SPI_CLK idles high, 1st transition is hi->lo |          NS
                                                         1, SPI_CLK idles low, 1st transition is lo->hi */
	uint64_t enable                       : 1;  /**< If 0, UART0_DTR_L/SPI_DO, UART0_DCD_L/SPI_DI       |          NS
                                                            UART0_RTS_L/SPI_CS2_L, UART1_RTS_L/SPI_CS3_L
                                                            pins are UART pins
                                                         1, UART0_DTR_L/SPI_DO and UART0_DCD_L/SPI_DI
                                                            pins are SPI/MPI pins.
                                                            UART0_RTS_L/SPI_CS2_L is SPI pin if CSENA2=1
                                                            UART1_RTS_L/SPI_CS3_L is SPI pin if CSENA3=1 */
#else
	uint64_t enable                       : 1;
	uint64_t idlelo                       : 1;
	uint64_t clk_cont                     : 1;
	uint64_t wireor                       : 1;
	uint64_t lsbfirst                     : 1;
	uint64_t int_ena                      : 1;
	uint64_t reserved_6_6                 : 1;
	uint64_t cshi                         : 1;
	uint64_t idleclks                     : 2;
	uint64_t tritx                        : 1;
	uint64_t cslate                       : 1;
	uint64_t reserved_12_13               : 2;
	uint64_t csena2                       : 1;
	uint64_t csena3                       : 1;
	uint64_t clkdiv                       : 13;
	uint64_t reserved_29_63               : 35;
#endif
	} cn66xx;
	struct cvmx_mpi_cfg_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_29_63               : 35;
	uint64_t clkdiv                       : 13; /**< Fspi_clk = Fsclk / (2 * CLKDIV)
                                                         CLKDIV = Fsclk / (2 * Fspi_clk) */
	uint64_t csena3                       : 1;  /**< If 0, UART1_RTS_L/SPI_CS3_L pin is UART pin
                                                         1, UART1_RTS_L/SPI_CS3_L pin is SPI pin
                                                         SPI_CS3_L drives UART1_RTS_L/SPI_CS3_L */
	uint64_t csena2                       : 1;  /**< If 0, UART1_CTS_L/SPI_CS2_L pin is UART pin
                                                         1, UART1_CTS_L/SPI_CS2_L pin is SPI pin
                                                         SPI_CS2_L drives  UART1_CTS_L/SPI_CS2_L */
	uint64_t csena1                       : 1;  /**< If 0, BOOT_CE_N<7>/SPI_CS1_L pin is BOOT pin
                                                         1, BOOT_CE_N<7>/SPI_CS1_L pin is SPI pin
                                                         SPI_CS1_L drives BOOT_CE_N<7>/SPI_CS1_L */
	uint64_t csena0                       : 1;  /**< If 0, BOOT_CE_N<6>/SPI_CS0_L pin is BOOT pin
                                                         1, BOOT_CE_N<6>/SPI_CS0_L pin is SPI pin
                                                         SPI_CS0_L drives BOOT_CE_N<6>/SPI_CS0_L */
	uint64_t cslate                       : 1;  /**< If 0, SPI_CS asserts 1/2 SPI clock before transaction.
                                                         1, SPI_CS assert coincident with transaction
                                                         NOTE: This control apply for 2 CSs */
	uint64_t tritx                        : 1;  /**< If 0, SPI_DO pin is driven when slave is not
                                                         expected to be driving
                                                         1, SPI_DO pin is tristated when not transmitting
                                                         NOTE: only used when WIREOR==1 */
	uint64_t idleclks                     : 2;  /**< Guarantee IDLECLKS idle SPI clock cycles between
                                                         commands. */
	uint64_t cshi                         : 1;  /**< If 0, CS is low asserted
                                                         1, CS is high asserted */
	uint64_t reserved_6_6                 : 1;
	uint64_t int_ena                      : 1;  /**< If 0, polling is required
                                                         1, MPI engine interrupts X end of transaction */
	uint64_t lsbfirst                     : 1;  /**< If 0, shift MSB first
                                                         1, shift LSB first */
	uint64_t wireor                       : 1;  /**< If 0, SPI_DO and SPI_DI are separate wires (SPI)
                                                         SPI_DO pin is always driven
                                                         1, SPI_DO/DI is all from SPI_DO pin (MPI)
                                                         SPI_DO pin is tristated when not transmitting
                                                         NOTE: if WIREOR==1, SPI_DI pin is not used by the
                                                         MPI engine */
	uint64_t clk_cont                     : 1;  /**< If 0, clock idles to value given by IDLELO after
                                                         completion of MPI transaction
                                                         1, clock never idles, requires CS deassertion
                                                         assertion between commands */
	uint64_t idlelo                       : 1;  /**< 0 = SPI_CK idles high, first transition is high-to-low. This mode corresponds to SPI Block
                                                         Guide options CPOL = 1, CPHA = 1.
                                                         1 = SPI_CK idles low, first transition is low-to-high. This mode corresponds to SPI Block
                                                         Guide options CPOL = 0, CPHA = 0. */
	uint64_t enable                       : 1;  /**< MPI/SPI master enable.
                                                         0 = UART0_DTR_L/SPI_DO, UART0_DCD_L/SPI_DI,  pins are UART pins.
                                                         1 = UART0_DTR_L/SPI_DO and UART0_DCD_L/SPI_DI pins are MPI/SPI pins.
                                                         This bit must be set for CSENA0-3 to operate as MPI/SPI Chip Selects */
#else
	uint64_t enable                       : 1;
	uint64_t idlelo                       : 1;
	uint64_t clk_cont                     : 1;
	uint64_t wireor                       : 1;
	uint64_t lsbfirst                     : 1;
	uint64_t int_ena                      : 1;
	uint64_t reserved_6_6                 : 1;
	uint64_t cshi                         : 1;
	uint64_t idleclks                     : 2;
	uint64_t tritx                        : 1;
	uint64_t cslate                       : 1;
	uint64_t csena0                       : 1;
	uint64_t csena1                       : 1;
	uint64_t csena2                       : 1;
	uint64_t csena3                       : 1;
	uint64_t clkdiv                       : 13;
	uint64_t reserved_29_63               : 35;
#endif
	} cn70xx;
	struct cvmx_mpi_cfg_cn70xx            cn70xxp1;
	struct cvmx_mpi_cfg_cn73xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_29_63               : 35;
	uint64_t clkdiv                       : 13; /**< Clock divisor.
                                                         _ SPI_CK = coprocessor clock / (2 * CLKDIV)
                                                         _ CLKDIV = coprocessor clock / (2 * SPI_CK) */
	uint64_t csena3                       : 1;  /**< SPI_CS3_L enable:
                                                         0 = UART1_RTS_L/SPI_CS3_L pin is UART pin.
                                                         1 = UART1_RTS_L/SPI_CS3_L pin is MPI/SPI pin.
                                                         SPI_CS3_L drives UART1_RTS_L/SPI_CS3_L. */
	uint64_t csena2                       : 1;  /**< SPI_CS2_L enable:
                                                         0 = UART1_CTS_L/SPI_CS2_L pin is UART pin.
                                                         1 = UART1_CTS_L/SPI_CS2_L pin is MPI/SPI pin.
                                                         SPI_CS2_L drives UART1_CTS_L/SPI_CS2_L. */
	uint64_t csena1                       : 1;  /**< SPI_CS1_L enable:
                                                         0 = BOOT_CE_N<7>/SPI_CS1_L pin is boot-bus pin.
                                                         1 = BOOT_CE_N<7>/SPI_CS1_L pin is MPI/SPI pin.
                                                         SPI_CS1_L drives BOOT_CE_N<7>/SPI_CS1_L. */
	uint64_t csena0                       : 1;  /**< SPI_CS0_L enable:
                                                         0 = BOOT_CE_N<6>/SPI_CS0_L pin is boot-bus pin.
                                                         1 = BOOT_CE_N<6>/SPI_CS0_L pin is MPI/SPI pin.
                                                         SPI_CS0_L drives BOOT_CE_N<6>/SPI_CS0_L. */
	uint64_t cslate                       : 1;  /**< SPI_CSn_L late.
                                                         0 = SPI_CSn_L asserts 1/2 SPI_CK cycle before the transaction.
                                                         1 = SPI_CSn_L asserts coincident with the transaction. */
	uint64_t tritx                        : 1;  /**< Tristate TX. Used only when WIREOR = 1
                                                         0 = SPI_DO pin is driven when slave is not expected to be driving.
                                                         1 = SPI_DO pin is tristated when not transmitting. */
	uint64_t idleclks                     : 2;  /**< Idle clocks. When set, guarantees idle SPI_CK cycles between commands. */
	uint64_t cshi                         : 1;  /**< SPI_CSn_L high: 1 = SPI_CSn_L is asserted high, 0 = SPI_CSn_L is asserted low. */
	uint64_t reserved_5_6                 : 2;
	uint64_t lsbfirst                     : 1;  /**< Shift LSB first: 0 = shift MSB first, 1 = shift LSB first. */
	uint64_t wireor                       : 1;  /**< Wire-OR DO and DI.
                                                         0 = SPI_DO and SPI_DI are separate wires (SPI). SPI_DO pin is always driven.
                                                         1 = SPI_DO/DI is all from SPI_DO pin (MPI). SPI_DO pin is tristated when not transmitting.
                                                         If WIREOR = 1, SPI_DI pin is not used by the MPI/SPI engine. */
	uint64_t clk_cont                     : 1;  /**< Clock control.
                                                         0 = Clock idles to value given by IDLELO after completion of MPI/SPI transaction.
                                                         1 = Clock never idles, requires SPI_CSn_L deassertion/assertion between commands. */
	uint64_t idlelo                       : 1;  /**< Clock idle low/clock invert.
                                                         0 = SPI_CK idles high, first transition is high-to-low.
                                                         1 = SPI_CK idles low, first transition is low-to-high. */
	uint64_t enable                       : 1;  /**< MPI/SPI enable.
                                                         0 = UART0_DTR_L/SPI_DO, UART0_DCD_L/SPI_DI, UART1_CTS_L/SPI_CS2_L, UART1_RTS_L/SPI_CS3_L
                                                         pins are UART pins.
                                                         1 = UART0_DTR_L/SPI_DO and UART0_DCD_L/SPI_DI pins are MPI/SPI pins.
                                                         UART1_CTS_L/SPI_CS2_L is an MPI/SPI pin if CSENA2 = 1.
                                                         UART1_RTS_L/SPI_CS3_L is an MPI/SPI pin if CSENA3 = 1. */
#else
	uint64_t enable                       : 1;
	uint64_t idlelo                       : 1;
	uint64_t clk_cont                     : 1;
	uint64_t wireor                       : 1;
	uint64_t lsbfirst                     : 1;
	uint64_t reserved_5_6                 : 2;
	uint64_t cshi                         : 1;
	uint64_t idleclks                     : 2;
	uint64_t tritx                        : 1;
	uint64_t cslate                       : 1;
	uint64_t csena0                       : 1;
	uint64_t csena1                       : 1;
	uint64_t csena2                       : 1;
	uint64_t csena3                       : 1;
	uint64_t clkdiv                       : 13;
	uint64_t reserved_29_63               : 35;
#endif
	} cn73xx;
	struct cvmx_mpi_cfg_cn73xx            cn78xx;
	struct cvmx_mpi_cfg_cn73xx            cn78xxp1;
	struct cvmx_mpi_cfg_cn61xx            cnf71xx;
};
typedef union cvmx_mpi_cfg cvmx_mpi_cfg_t;

/**
 * cvmx_mpi_dat#
 */
union cvmx_mpi_datx {
	uint64_t u64;
	struct cvmx_mpi_datx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_8_63                : 56;
	uint64_t data                         : 8;  /**< Data to transmit/received                          |           NS */
#else
	uint64_t data                         : 8;
	uint64_t reserved_8_63                : 56;
#endif
	} s;
	struct cvmx_mpi_datx_s                cn30xx;
	struct cvmx_mpi_datx_s                cn31xx;
	struct cvmx_mpi_datx_s                cn50xx;
	struct cvmx_mpi_datx_s                cn61xx;
	struct cvmx_mpi_datx_s                cn66xx;
	struct cvmx_mpi_datx_s                cn70xx;
	struct cvmx_mpi_datx_s                cn70xxp1;
	struct cvmx_mpi_datx_s                cn73xx;
	struct cvmx_mpi_datx_s                cn78xx;
	struct cvmx_mpi_datx_s                cn78xxp1;
	struct cvmx_mpi_datx_s                cnf71xx;
};
typedef union cvmx_mpi_datx cvmx_mpi_datx_t;

/**
 * cvmx_mpi_sts
 */
union cvmx_mpi_sts {
	uint64_t u64;
	struct cvmx_mpi_sts_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_13_63               : 51;
	uint64_t rxnum                        : 5;  /**< Number of bytes written for transaction            |          NS */
	uint64_t reserved_2_7                 : 6;
	uint64_t mpi_intr                     : 1;  /**< MPI interrupt on transaction done. Throws MPI_INTSN_E::MPI_STS_INTR. */
	uint64_t busy                         : 1;  /**< If 0, no MPI transaction in progress               |          NS
                                                         1, MPI engine is processing a transaction */
#else
	uint64_t busy                         : 1;
	uint64_t mpi_intr                     : 1;
	uint64_t reserved_2_7                 : 6;
	uint64_t rxnum                        : 5;
	uint64_t reserved_13_63               : 51;
#endif
	} s;
	struct cvmx_mpi_sts_cn30xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_13_63               : 51;
	uint64_t rxnum                        : 5;  /**< Number of bytes written for transaction */
	uint64_t reserved_1_7                 : 7;
	uint64_t busy                         : 1;  /**< If 0, no MPI transaction in progress
                                                         1, MPI engine is processing a transaction */
#else
	uint64_t busy                         : 1;
	uint64_t reserved_1_7                 : 7;
	uint64_t rxnum                        : 5;
	uint64_t reserved_13_63               : 51;
#endif
	} cn30xx;
	struct cvmx_mpi_sts_cn30xx            cn31xx;
	struct cvmx_mpi_sts_cn30xx            cn50xx;
	struct cvmx_mpi_sts_cn30xx            cn61xx;
	struct cvmx_mpi_sts_cn30xx            cn66xx;
	struct cvmx_mpi_sts_cn30xx            cn70xx;
	struct cvmx_mpi_sts_cn30xx            cn70xxp1;
	struct cvmx_mpi_sts_s                 cn73xx;
	struct cvmx_mpi_sts_s                 cn78xx;
	struct cvmx_mpi_sts_s                 cn78xxp1;
	struct cvmx_mpi_sts_cn30xx            cnf71xx;
};
typedef union cvmx_mpi_sts cvmx_mpi_sts_t;

/**
 * cvmx_mpi_sts_w1s
 *
 * This register sets MPI_STS() interrupts.
 *
 */
union cvmx_mpi_sts_w1s {
	uint64_t u64;
	struct cvmx_mpi_sts_w1s_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_2_63                : 62;
	uint64_t mpi_intr_w1s                 : 1;  /**< Reads or sets MPI_STS[MPI_INTR]. */
	uint64_t reserved_0_0                 : 1;
#else
	uint64_t reserved_0_0                 : 1;
	uint64_t mpi_intr_w1s                 : 1;
	uint64_t reserved_2_63                : 62;
#endif
	} s;
	struct cvmx_mpi_sts_w1s_s             cn73xx;
	struct cvmx_mpi_sts_w1s_s             cn78xx;
};
typedef union cvmx_mpi_sts_w1s cvmx_mpi_sts_w1s_t;

/**
 * cvmx_mpi_tx
 */
union cvmx_mpi_tx {
	uint64_t u64;
	struct cvmx_mpi_tx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_22_63               : 42;
	uint64_t csid                         : 2;  /**< Which CS to assert for this transaction            |          NS */
	uint64_t reserved_17_19               : 3;
	uint64_t leavecs                      : 1;  /**< If 0, deassert CS after transaction is done        |          NS
                                                         1, leave CS asserted after transactrion is done */
	uint64_t reserved_13_15               : 3;
	uint64_t txnum                        : 5;  /**< Number of bytes to transmit                        |          NS */
	uint64_t reserved_5_7                 : 3;
	uint64_t totnum                       : 5;  /**< Number of bytes to shift (transmit + receive)      |          NS */
#else
	uint64_t totnum                       : 5;
	uint64_t reserved_5_7                 : 3;
	uint64_t txnum                        : 5;
	uint64_t reserved_13_15               : 3;
	uint64_t leavecs                      : 1;
	uint64_t reserved_17_19               : 3;
	uint64_t csid                         : 2;
	uint64_t reserved_22_63               : 42;
#endif
	} s;
	struct cvmx_mpi_tx_cn30xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_17_63               : 47;
	uint64_t leavecs                      : 1;  /**< If 0, deassert CS after transaction is done
                                                         1, leave CS asserted after transactrion is done */
	uint64_t reserved_13_15               : 3;
	uint64_t txnum                        : 5;  /**< Number of bytes to transmit */
	uint64_t reserved_5_7                 : 3;
	uint64_t totnum                       : 5;  /**< Number of bytes to shift (transmit + receive) */
#else
	uint64_t totnum                       : 5;
	uint64_t reserved_5_7                 : 3;
	uint64_t txnum                        : 5;
	uint64_t reserved_13_15               : 3;
	uint64_t leavecs                      : 1;
	uint64_t reserved_17_63               : 47;
#endif
	} cn30xx;
	struct cvmx_mpi_tx_cn30xx             cn31xx;
	struct cvmx_mpi_tx_cn30xx             cn50xx;
	struct cvmx_mpi_tx_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_21_63               : 43;
	uint64_t csid                         : 1;  /**< Which CS to assert for this transaction            |          NS */
	uint64_t reserved_17_19               : 3;
	uint64_t leavecs                      : 1;  /**< If 0, deassert CS after transaction is done        |          NS
                                                         1, leave CS asserted after transactrion is done */
	uint64_t reserved_13_15               : 3;
	uint64_t txnum                        : 5;  /**< Number of bytes to transmit                        |          NS */
	uint64_t reserved_5_7                 : 3;
	uint64_t totnum                       : 5;  /**< Number of bytes to shift (transmit + receive)      |          NS */
#else
	uint64_t totnum                       : 5;
	uint64_t reserved_5_7                 : 3;
	uint64_t txnum                        : 5;
	uint64_t reserved_13_15               : 3;
	uint64_t leavecs                      : 1;
	uint64_t reserved_17_19               : 3;
	uint64_t csid                         : 1;
	uint64_t reserved_21_63               : 43;
#endif
	} cn61xx;
	struct cvmx_mpi_tx_s                  cn66xx;
	struct cvmx_mpi_tx_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_63_22               : 42;
	uint64_t csid                         : 2;  /**< Which CS to assert for this transaction */
	uint64_t reserved_19_17               : 3;
	uint64_t leavecs                      : 1;  /**< If 0, deassert CS after transaction is done
                                                         1, leave CS asserted after transactrion is done */
	uint64_t reserved_15_13               : 3;
	uint64_t txnum                        : 5;  /**< Number of bytes to transmit */
	uint64_t reserved_7_5                 : 3;
	uint64_t totnum                       : 5;  /**< Number of bytes to shift (transmit + receive) */
#else
	uint64_t totnum                       : 5;
	uint64_t reserved_7_5                 : 3;
	uint64_t txnum                        : 5;
	uint64_t reserved_15_13               : 3;
	uint64_t leavecs                      : 1;
	uint64_t reserved_19_17               : 3;
	uint64_t csid                         : 2;
	uint64_t reserved_63_22               : 42;
#endif
	} cn70xx;
	struct cvmx_mpi_tx_cn70xx             cn70xxp1;
	struct cvmx_mpi_tx_s                  cn73xx;
	struct cvmx_mpi_tx_s                  cn78xx;
	struct cvmx_mpi_tx_s                  cn78xxp1;
	struct cvmx_mpi_tx_cn61xx             cnf71xx;
};
typedef union cvmx_mpi_tx cvmx_mpi_tx_t;

/**
 * cvmx_mpi_wide_dat
 *
 * This register aliases operations to MPI_DAT(0..7) in a single access
 *
 */
union cvmx_mpi_wide_dat {
	uint64_t u64;
	struct cvmx_mpi_wide_dat_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t data                         : 64; /**< Data to transmit/receive. */
#else
	uint64_t data                         : 64;
#endif
	} s;
	struct cvmx_mpi_wide_dat_s            cn70xx;
	struct cvmx_mpi_wide_dat_s            cn70xxp1;
	struct cvmx_mpi_wide_dat_s            cn73xx;
	struct cvmx_mpi_wide_dat_s            cn78xx;
	struct cvmx_mpi_wide_dat_s            cn78xxp1;
};
typedef union cvmx_mpi_wide_dat cvmx_mpi_wide_dat_t;

#endif

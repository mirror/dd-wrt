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
 * General Purpose IO interface.
 *
 * <hr>$Revision: 93360 $<hr>
 */

#ifndef __CVMX_GPIO_H__
#define __CVMX_GPIO_H__

#ifdef	__cplusplus
/* *INDENT-OFF* */
extern "C" {
/* *INDENT-ON* */
#endif

/* CSR typedefs have been moved to cvmx-gpio-defs.h */

/**
 * Clear the interrupt rising edge detector for the supplied
 * pins in the mask. Chips which have more than 16 GPIO pins
 * can't use them for interrupts.
 *
 * @param node       Node to clear
 * @param clear_mask Mask of pins to clear
 */
static inline void __cvmx_gpio_interrupt_clear(uint8_t node,
					       uint64_t clear_mask)
{
	if (OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN70XX)) {
		cvmx_gpio_multi_cast_t multi_cast;
		cvmx_gpio_bit_cfgx_t gpio_bit;
		int core = cvmx_get_core_num();

		multi_cast.u64 = cvmx_read_csr_node(node, CVMX_GPIO_MULTI_CAST);
		gpio_bit.u64 = cvmx_read_csr_node(node, CVMX_GPIO_BIT_CFGX(core));

		/* If Multicast mode is enabled, and GPIO interrupt is enabled
		 * for edge detection, then GPIO<4..7> interrupts are per core
		 */
		if (multi_cast.s.en && gpio_bit.s.int_en && gpio_bit.s.int_type) {
			/* Clear GPIO<4..7> per core */
			cvmx_ciu_intx_sum0_t ciu_sum0;
			ciu_sum0.u64 = cvmx_read_csr_node(node,
							  CVMX_CIU_INTX_SUM0(core * 2));
			ciu_sum0.s.gpio = clear_mask & 0xf0;
			cvmx_write_csr_node(node, CVMX_CIU_INTX_SUM0(core * 2),
					    ciu_sum0.u64);

			/* Clear other GPIO pins for all cores. */
			cvmx_write_csr_node(node, CVMX_GPIO_INT_CLR,
					    (clear_mask & ~0xf0));
			return;
		}
	}
	/* Clear GPIO pins state across all cores and common interrupt states. */
	cvmx_gpio_int_clr_t gpio_int_clr;
	gpio_int_clr.u64 = 0;
	gpio_int_clr.s.type = clear_mask;
	cvmx_write_csr_node(node, CVMX_GPIO_INT_CLR, gpio_int_clr.u64);
}

/**
 * Clear the interrupt rising edge detector for the supplied
 * pins in the mask. Chips which have more than 16 GPIO pins
 * can't use them for interrupts.
 * @deprecated
 *
 * @param node       Node to clear
 * @param clear_mask Mask of pins to clear
 */
static inline void cvmx_gpio_interrupt_clear(uint64_t clear_mask)
{
	__cvmx_gpio_interrupt_clear(0, clear_mask);
}

static inline void cvmx_gpio_interrupt_clear_node(int node,
						  uint64_t clear_mask)
{
	__cvmx_gpio_interrupt_clear(node, clear_mask);
}

/**
 * GPIO configure Pin on the specified node
 *
 * @param node  Node to read from
 * @param bit   The GPIO to use
 * @param mode  Drive GPIO as output pin or not.
 *
 */
static inline void __cvmx_gpio_cfg(uint8_t node, int bit, int mode)
{
	if (bit > 15 && bit < 20) {
		/* CN61XX/CN66XX has 20 GPIO pins and only 16 are interruptable. */
		if (OCTEON_IS_MODEL(OCTEON_CN61XX)
		    || OCTEON_IS_MODEL(OCTEON_CN66XX)
		    || OCTEON_IS_MODEL(OCTEON_CN70XX)) {
			cvmx_gpio_xbit_cfgx_t gpio_xbit;
			gpio_xbit.u64 = cvmx_read_csr_node(node,
							   CVMX_GPIO_XBIT_CFGX(bit));
			gpio_xbit.s.tx_oe = !!mode;
			cvmx_write_csr_node(node, CVMX_GPIO_XBIT_CFGX(bit),
					    gpio_xbit.u64);
		} else
			cvmx_dprintf("%s: Invalid GPIO bit(%d)\n",
				     __func__, bit);
	} else {
		cvmx_gpio_bit_cfgx_t gpio_bit;
		gpio_bit.u64 = cvmx_read_csr_node(node, CVMX_GPIO_BIT_CFGX(bit));
		gpio_bit.s.tx_oe = !!mode;
		cvmx_write_csr_node(node, CVMX_GPIO_BIT_CFGX(bit), gpio_bit.u64);
	}
}

/**
 * Inverts the input to a GPIO
 *
 * @param node	Node to configure
 * @param bit	The GPIO to configure
 * @param invert	1 to invert input, 0 to not invert
 */
static inline void cvmx_gpio_invert_input(uint8_t node, int bit, int invert)
{
	if (bit > 15 && bit < 20) {
		cvmx_gpio_xbit_cfgx_t gpio_xbit;
		gpio_xbit.u64 = cvmx_read_csr_node(node,
						   CVMX_GPIO_XBIT_CFGX(bit));
		gpio_xbit.s.rx_xor = !!invert;
		cvmx_write_csr_node(node, CVMX_GPIO_XBIT_CFGX(bit),
				    gpio_xbit.u64);
	} else {
		cvmx_gpio_bit_cfgx_t gpio_bit;
		gpio_bit.u64 = cvmx_read_csr_node(node,
						  CVMX_GPIO_BIT_CFGX(bit));
		gpio_bit.s.rx_xor = !!invert;
		cvmx_write_csr_node(node, CVMX_GPIO_BIT_CFGX(bit),
				    gpio_bit.u64);
	}
}

/**
 * GPIO configure output selection mode
 *
 * @param node	Node to configure
 * @param bit	The GPIO to configure
 * @param output_sel	output selection value
 */
static inline void cvmx_gpio_cfg_sel(uint8_t node, int bit, int output_sel)
{
	if (!OCTEON_IS_OCTEON3()) {
		cvmx_dprintf("%s: Only OCTEON3 required.", __func__);
		return;
	}

	if (bit > 15 && bit < 20) {
		cvmx_gpio_xbit_cfgx_t gpio_xbit;
		gpio_xbit.u64 = cvmx_read_csr_node(node,
						   CVMX_GPIO_XBIT_CFGX(bit));
		gpio_xbit.cn70xx.output_sel = output_sel;
		cvmx_write_csr_node(node, CVMX_GPIO_XBIT_CFGX(bit),
				    gpio_xbit.u64);
	} else {
		cvmx_gpio_bit_cfgx_t gpio_bit;
		gpio_bit.u64 = cvmx_read_csr_node(node, CVMX_GPIO_BIT_CFGX(bit));
		gpio_bit.cn70xx.output_sel = output_sel;
		cvmx_write_csr_node(node, CVMX_GPIO_BIT_CFGX(bit),
				    gpio_bit.u64);
	}
}

/**
 * GPIO configure Pin
 * @deprecated
 *
 * @param bit   The GPIO to use
 * @param mode  Drive GPIO as output pin or not.
 *
 */
static inline void cvmx_gpio_cfg(int bit, int mode)
{
	__cvmx_gpio_cfg(0, bit, mode);
}

static inline void cvmx_gpio_cfg_node(int node, int bit, int mode)
{
	__cvmx_gpio_cfg(node, bit, mode);
}

/**
 * GPIO Read Data
 * 
 * @param node  The node to read from
 *
 * @return Status of the GPIO pins
 */
static inline uint64_t __cvmx_gpio_read(uint8_t node)
{
	cvmx_gpio_rx_dat_t gpio_rx_dat;
	gpio_rx_dat.u64 = cvmx_read_csr_node(node, CVMX_GPIO_RX_DAT);
	return gpio_rx_dat.s.dat;
}

/**
 * GPIO Read Data
 * @deprecated 
 *
 * @return Status of the GPIO pins
 */
static inline uint64_t cvmx_gpio_read(void)
{
	return __cvmx_gpio_read(0);
}
static inline uint64_t cvmx_gpio_read_node(int node)
{
	return __cvmx_gpio_read(node);
}

/**
 * GPIO Clear pin
 *
 * @param node       Node to clear pin on
 * @param clear_mask Bit mask to indicate which bits to drive to '0'.
 */
static inline void __cvmx_gpio_clear(uint8_t node, uint64_t clear_mask)
{
	cvmx_gpio_tx_clr_t gpio_tx_clr;
	gpio_tx_clr.u64 = 0;
	gpio_tx_clr.s.clr = clear_mask;
	cvmx_write_csr_node(node, CVMX_GPIO_TX_CLR, gpio_tx_clr.u64);
}

/**
 * GPIO Clear pin
 *
 * @param clear_mask Bit mask to indicate which bits to drive to '0'.
 */
static inline void cvmx_gpio_clear(uint64_t clear_mask)
{
	__cvmx_gpio_clear(0, clear_mask);
}
static inline void cvmx_gpio_clear_node(uint8_t node, uint64_t clear_mask)
{
	__cvmx_gpio_clear(node, clear_mask);
}

/**
 * GPIO Set pin
 *
 * @param node     Node to set pins on
 * @param set_mask Bit mask to indicate which bits to drive to '1'.
 */
static inline void __cvmx_gpio_set(uint8_t node, uint64_t set_mask)
{
	cvmx_gpio_tx_set_t gpio_tx_set;
	gpio_tx_set.u64 = 0;
	gpio_tx_set.s.set = set_mask;
	cvmx_write_csr_node(node, CVMX_GPIO_TX_SET, gpio_tx_set.u64);
}

/**
 * GPIO Set pin
 * @deprecated
 *
 * @param set_mask Bit mask to indicate which bits to drive to '1'.
 */
static inline void cvmx_gpio_set(uint64_t set_mask)
{
	__cvmx_gpio_set(0, set_mask);
}
static inline void cvmx_gpio_set_node(uint8_t node, uint64_t set_mask)
{
	__cvmx_gpio_set(node, set_mask);
}

#ifdef	__cplusplus
/* *INDENT-OFF* */
}
/* *INDENT-ON* */
#endif

#endif

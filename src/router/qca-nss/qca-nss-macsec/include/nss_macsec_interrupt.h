/*
 * Copyright (c) 2014, The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _NSS_MACSEC_INTERRUPT_H_
#define _NSS_MACSEC_INTERRUPT_H_

#include "nss_macsec_types.h"

typedef struct {
	bool tx_egsy_int_en;
	bool tx_sa_exp_pulse_int_en;
	bool tx_sa_thr_pulse_int_en;
	bool tx_egsy_mib_sat_int_en;
	bool tx_ecc_err_int_en;

	bool rx_igsy_int_en;
	bool rx_sa_exp_pulse_int_en;
	bool rx_sa_thr_pulse_int_en;
	bool rx_icv_err_pulse_int_en;
	bool rx_rpy_err_pulse_int_en;
	bool rx_insy_mib_sat_int_en;
	bool rx_ecc_err_int_en;
	bool rx_ec_bit_err_int_en;
	bool rx_gpoc_miss_int_en;
} fal_interrupt_en_t;

typedef struct {
	bool tx_egsy_int;
	bool tx_sa_exp_pulse_int;
	bool tx_sa_thr_pulse_int;
	bool tx_egsy_mib_sat_int;
	bool tx_ecc_err_int;

	bool rx_igsy_int;
	bool rx_sa_exp_pulse_int;
	bool rx_sa_thr_pulse_in;
	bool rx_icv_err_pulse_int;
	bool rx_rpy_err_pulse_int;
	bool rx_insy_mib_sat_int;
	bool rx_ecc_err_int;
	bool rx_ec_bit_err_int;
	bool rx_igpoc_miss_int;
} fal_interrupt_t;

u32 nss_macsec_secy_interrupt_en_get(u32 secy_id, fal_interrupt_en_t *p_int_en);

u32 nss_macsec_secy_interrupt_en_set(u32 secy_id, fal_interrupt_en_t *p_int_en);

u32 nss_macsec_secy_interrupt_get(u32 secy_id, fal_interrupt_t *pint);

#endif /* _NSS_MACSEC_INTERRUPT_H_ */

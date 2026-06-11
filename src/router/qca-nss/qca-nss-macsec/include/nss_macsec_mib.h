/*
 * Copyright (c) 2014, 2018, The Linux Foundation. All rights reserved.
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

#ifndef _NSS_MACSEC_MIB_H_
#define _NSS_MACSEC_MIB_H_

#include "nss_macsec_types.h"

typedef struct {
	u64 protected_pkts;
	u64 encrypted_pkts;
	u64 protected_octets;
	u64 encrypted_octets;
} fal_tx_sc_mib_t;

typedef struct {
	u64 hit_drop_redirect;
	u64 protected2_pkts;
	u64 protected_pkts;
	u64 encrypted_pkts;
} fal_tx_sa_mib_t;

typedef struct {
	u64 ctl_pkts;
	u64 unknown_sa_pkts;
	u64 untagged_pkts;
	u64 too_long;
	u64 ecc_error_pkts;
	u64 unctrl_hit_drop_redir_pkts;
} fal_tx_mib_t;

typedef struct {
	u64 untagged_hit_pkts;
	u64 hit_drop_redir_pkts;
	u64 not_using_sa;
	u64 unused_sa;
	u64 not_valid_pkts;
	u64 invalid_pkts;
	u64 ok_pkts;
	u64 late_pkts;
	u64 delayed_pkts;
	u64 unchecked_pkts;
	u64 validated_octets;
	u64 decrypted_octets;
} fal_rx_sa_mib_t;

typedef struct {
	u64 unctrl_prt_tx_octets;
	u64 ctl_pkts;
	u64 tagged_miss_pkts;
	u64 untagged_hit_pkts;
	u64 notag_pkts;
	u64 untagged_pkts;
	u64 bad_tag_pkts;
	u64 no_sci_pkts;
	u64 unknown_sci_pkts;
	u64 ctrl_prt_pass_pkts;
	u64 unctrl_prt_pass_pkts;
	u64 ctrl_prt_fail_pkts;
	u64 unctrl_prt_fail_pkts;
	u64 too_long_packets;
	u64 igpoc_ctl_pkts;
	u64 ecc_error_pkts;
} fal_rx_mib_t;

/**
* @param[in] secy_id
* @param[in] channel
* @param[out] pmib
**/
u32 nss_macsec_secy_tx_sc_mib_get(u32 secy_id, u32 channel,
                                  fal_tx_sc_mib_t *pmib);

/**
* @param[in] secy_id
* @param[in] channel
* @param[in] an
* @param[out] pmib
**/
u32 nss_macsec_secy_tx_sa_mib_get(u32 secy_id, u32 channel,
                                  u32 an, fal_tx_sa_mib_t *pmib);

/**
* @param[in] secy_id
* @param[out] pmib
**/
u32 nss_macsec_secy_tx_mib_get(u32 secy_id, fal_tx_mib_t *pmib);

/**
* @param[in] secy_id
* @param[in] channel
* @param[in] an
* @param[out] pmib
**/
u32 nss_macsec_secy_rx_sa_mib_get(u32 secy_id, u32 channel,
                                  u32 an, fal_rx_sa_mib_t *pmib);

/**
* @param[in] secy_id
* @param[out] pmib
**/
u32 nss_macsec_secy_rx_mib_get(u32 secy_id, fal_rx_mib_t *pmib);

/**
* @param[in] secy_id
**/
u32 nss_macsec_secy_tx_mib_clear(u32 secy_id);

/**
* @param[in] secy_id
* @param[in] channel
**/
u32 nss_macsec_secy_tx_sc_mib_clear(u32 secy_id, u32 channel);

/**
* @param[in] secy_id
* @param[in] channel
* @param[in] an
**/
u32 nss_macsec_secy_tx_sa_mib_clear(u32 secy_id, u32 channel, u32 an);

/**
* @param[in] secy_id
**/
u32 nss_macsec_secy_rx_mib_clear(u32 secy_id);

/**
* @param[in] secy_id
* @param[in] channel
* @param[in] an
**/
u32 nss_macsec_secy_rx_sa_mib_clear(u32 secy_id, u32 channel, u32 an);

#endif /* _NSS_MACSEC_MIB_H_ */

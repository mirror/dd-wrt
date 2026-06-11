/*
 * Copyright (c) 2014, 2018, The Linux Foundation. All rights reserved.
 * Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */


#include "nss_macsec_types.h"
#include "nss_macsec.h"
#include "nss_macsec_utility.h"
#include "nss_macsec_mib.h"


u32 nss_macsec_secy_tx_sc_mib_get(u32 secy_id, u32 channel,
				  fal_tx_sc_mib_t *pmib)
{
	g_error_t rv;
	struct secy_tx_sc_mib_t secy_tx_sc_mib;

	memset(&secy_tx_sc_mib, 0, sizeof(secy_tx_sc_mib));

	SHR_PARAM_CHECK((secy_id < FAL_SECY_ID_NUM) &&
			(channel < FAL_SECY_CHANNEL_NUM(secy_id)) &&
			(pmib != NULL));

	rv = qca_macsec_tx_sc_mib_get(FAL_SECY_ID_TO_PORT(secy_id),
					channel, &secy_tx_sc_mib);
	pmib->protected_pkts = secy_tx_sc_mib.protected_pkts;
	pmib->encrypted_pkts = secy_tx_sc_mib.encrypted_pkts;
	pmib->protected_octets = secy_tx_sc_mib.protected_octets;
	pmib->encrypted_octets = secy_tx_sc_mib.encrypted_octets;

	return rv;
}

u32 nss_macsec_secy_tx_sa_mib_get(u32 secy_id, u32 channel,
				  u32 an, fal_tx_sa_mib_t *pmib)
{
	g_error_t rv;
	struct secy_tx_sa_mib_t secy_tx_sa_mib;

	memset(&secy_tx_sa_mib, 0, sizeof(secy_tx_sa_mib));

	SHR_PARAM_CHECK((secy_id < FAL_SECY_ID_NUM) &&
			(channel < FAL_SECY_CHANNEL_NUM(secy_id)) &&
			(an < FAL_SECY_SA_NUM_PER_SC(secy_id)) &&
			(pmib != NULL));

	rv = qca_macsec_tx_sa_mib_get(FAL_SECY_ID_TO_PORT(secy_id),
					channel, an, &secy_tx_sa_mib);
	pmib->hit_drop_redirect = secy_tx_sa_mib.hit_drop_redirect;
	pmib->protected2_pkts = secy_tx_sa_mib.protected2_pkts;
	pmib->protected_pkts = secy_tx_sa_mib.protected_pkts;
	pmib->encrypted_pkts = secy_tx_sa_mib.encrypted_pkts;

	return rv;
}

u32 nss_macsec_secy_tx_mib_get(u32 secy_id, fal_tx_mib_t *pmib)
{
	g_error_t rv;
	struct secy_tx_mib_t secy_tx_mib;

	memset(&secy_tx_mib, 0, sizeof(secy_tx_mib));

	SHR_PARAM_CHECK((secy_id < FAL_SECY_ID_NUM) && (pmib != NULL));

	rv = qca_macsec_tx_mib_get(FAL_SECY_ID_TO_PORT(secy_id), &secy_tx_mib);
	pmib->ctl_pkts = secy_tx_mib.ctl_pkts;
	pmib->unknown_sa_pkts = secy_tx_mib.unknown_sa_pkts;
	pmib->untagged_pkts = secy_tx_mib.untagged_pkts;
	pmib->too_long = secy_tx_mib.too_long;
	pmib->ecc_error_pkts = secy_tx_mib.ecc_error_pkts;
	pmib->unctrl_hit_drop_redir_pkts = secy_tx_mib.unctrl_hit_drop_redir_pkts;

	return rv;
}

u32 nss_macsec_secy_rx_sa_mib_get(u32 secy_id, u32 channel,
				  u32 an, fal_rx_sa_mib_t *pmib)
{
	g_error_t rv;
	struct secy_rx_sa_mib_t secy_rx_sa_mib;
	struct secy_rx_sc_mib_t secy_rx_sc_mib;

	memset(&secy_rx_sa_mib, 0, sizeof(secy_rx_sa_mib));
	memset(&secy_rx_sc_mib, 0, sizeof(secy_rx_sc_mib));

	SHR_PARAM_CHECK((secy_id < FAL_SECY_ID_NUM) &&
			(channel < FAL_SECY_CHANNEL_NUM(secy_id)) &&
			(an < FAL_SECY_SA_NUM_PER_SC(secy_id)) &&
			(pmib != NULL));

	rv = qca_macsec_rx_sc_mib_get(FAL_SECY_ID_TO_PORT(secy_id), channel, &secy_rx_sc_mib);
	pmib->late_pkts = secy_rx_sc_mib.late_pkts;
	pmib->delayed_pkts = secy_rx_sc_mib.delayed_pkts;
	pmib->unchecked_pkts = secy_rx_sc_mib.unchecked_pkts;
	pmib->validated_octets = secy_rx_sc_mib.validated_octets;
	pmib->decrypted_octets = secy_rx_sc_mib.decrypted_octets;

	rv = qca_macsec_rx_sa_mib_get(FAL_SECY_ID_TO_PORT(secy_id), channel, an,
					&secy_rx_sa_mib);
	pmib->untagged_hit_pkts = secy_rx_sa_mib.untagged_hit_pkts;
	pmib->hit_drop_redir_pkts = secy_rx_sa_mib.hit_drop_redir_pkts;
	pmib->not_using_sa = secy_rx_sa_mib.not_using_sa;
	pmib->unused_sa = secy_rx_sa_mib.unused_sa;
	pmib->not_valid_pkts = secy_rx_sa_mib.not_valid_pkts;
	pmib->invalid_pkts = secy_rx_sa_mib.invalid_pkts;
	pmib->ok_pkts = secy_rx_sa_mib.ok_pkts;

	return rv;
}

u32 nss_macsec_secy_rx_mib_get(u32 secy_id, fal_rx_mib_t *pmib)
{
	g_error_t rv;
	struct secy_rx_mib_t secy_rx_mib;

	memset(&secy_rx_mib, 0, sizeof(secy_rx_mib));

	SHR_PARAM_CHECK((secy_id < FAL_SECY_ID_NUM) && (pmib != NULL));

	rv = qca_macsec_rx_mib_get(FAL_SECY_ID_TO_PORT(secy_id), &secy_rx_mib);
	pmib->unctrl_prt_tx_octets = secy_rx_mib.unctrl_prt_tx_octets;
	pmib->ctl_pkts = secy_rx_mib.ctl_pkts;
	pmib->tagged_miss_pkts = secy_rx_mib.tagged_miss_pkts;
	pmib->untagged_hit_pkts = secy_rx_mib.untagged_hit_pkts;
	pmib->notag_pkts = secy_rx_mib.notag_pkts;
	pmib->untagged_pkts = secy_rx_mib.untagged_pkts;
	pmib->bad_tag_pkts = secy_rx_mib.bad_tag_pkts;
	pmib->no_sci_pkts = secy_rx_mib.no_sci_pkts;
	pmib->unknown_sci_pkts = secy_rx_mib.unknown_sci_pkts;
	pmib->ctrl_prt_pass_pkts = secy_rx_mib.ctrl_prt_pass_pkts;
	pmib->unctrl_prt_pass_pkts = secy_rx_mib.unctrl_prt_pass_pkts;
	pmib->ctrl_prt_fail_pkts = secy_rx_mib.ctrl_prt_fail_pkts;
	pmib->unctrl_prt_fail_pkts = secy_rx_mib.unctrl_prt_fail_pkts;
	pmib->too_long_packets = secy_rx_mib.too_long_packets;
	pmib->igpoc_ctl_pkts = secy_rx_mib.igpoc_ctl_pkts;
	pmib->ecc_error_pkts = secy_rx_mib.ecc_error_pkts;

	return rv;
}

u32 nss_macsec_secy_tx_mib_clear(u32 secy_id)
{
	SHR_PARAM_CHECK(secy_id < FAL_SECY_ID_NUM);

	return qca_macsec_tx_mib_clear(FAL_SECY_ID_TO_PORT(secy_id));
}

u32 nss_macsec_secy_tx_sc_mib_clear(u32 secy_id, u32 channel)
{
	SHR_PARAM_CHECK((secy_id < FAL_SECY_ID_NUM) &&
			(channel < FAL_SECY_CHANNEL_NUM(secy_id)));

	return qca_macsec_tx_sc_mib_clear(FAL_SECY_ID_TO_PORT(secy_id), channel);
}

u32 nss_macsec_secy_tx_sa_mib_clear(u32 secy_id, u32 channel, u32 an)
{
	SHR_PARAM_CHECK((secy_id < FAL_SECY_ID_NUM) &&
			(channel < FAL_SECY_CHANNEL_NUM(secy_id)) &&
			(an < FAL_SECY_SA_NUM_PER_SC(secy_id)));

	return qca_macsec_tx_sa_mib_clear(FAL_SECY_ID_TO_PORT(secy_id), channel, an);
}

u32 nss_macsec_secy_rx_mib_clear(u32 secy_id)
{
	SHR_PARAM_CHECK(secy_id < FAL_SECY_ID_NUM);

	return qca_macsec_rx_mib_clear(FAL_SECY_ID_TO_PORT(secy_id));
}

u32 nss_macsec_secy_rx_sa_mib_clear(u32 secy_id, u32 channel, u32 an)
{
	SHR_PARAM_CHECK((secy_id < FAL_SECY_ID_NUM) &&
			(channel < FAL_SECY_CHANNEL_NUM(secy_id)) &&
			(an < FAL_SECY_SA_NUM_PER_SC(secy_id)));

	return qca_macsec_rx_sa_mib_clear(FAL_SECY_ID_TO_PORT(secy_id), channel, an);
}


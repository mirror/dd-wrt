/*
 * Copyright (c) 2022, Qualcomm Innovation Center, Inc. All rights reserved.
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

/*
 * nss_trustsec_rx_log.c
 *	NSS TRUSTSEC_RX logger file.
 */

#include "nss_core.h"

/*
 * nss_trustsec_rx_log_message_types_str
 *	TRUSTSEC_RX message strings
 */
static int8_t *nss_trustsec_rx_log_message_types_str[NSS_TRUSTSEC_RX_MSG_MAX] __maybe_unused = {
	"TRUSTSEC_RX Configure Message",
	"TRUSTSEC_RX Unconfigure Message",
	"TRUSTSEC_RX Stats Sync",
	"TRUSTSEC_RX Config vp num Message",
	"TRUSTSEC_RX Unconfig vp num Message",
};

/*
 * nss_trustsec_rx_log_error_response_types_str
 *	Strings for error types for TRUSTSEC_RX messages
 */
static int8_t *nss_trustsec_rx_log_error_response_types_str[NSS_TRUSTSEC_RX_ERR_MAX] __maybe_unused = {
	"TRUSTSEC_RX No error",
	"TRUSTSEC_RX Destination interface is not found",
	"TRUSTSEC_RX IP version is incorrect",
	"TRUSTSEC_RX Entry already exist",
	"TRUSTSEC_RX IP rule cannot be added",
	"TRUSTSEC_RX Entry cannot be found",
	"TRUSTSEC_RX source interface is not configured",
	"TRUSTSEC_RX Unknown trustsec message",
	"TRUSTSEC_RX configure vp number failed",
	"TRUSTSEC_RX unconfigure vp number failed",
};

/*
 * nss_trustsec_rx_log_config_vp_msg
 *	Log NSS TrustSec Rx configure vp message
 */
static void nss_trustsec_rx_log_config_vp_msg(struct nss_trustsec_rx_msg *ntm)
{
	struct nss_trustsec_rx_vp_msg *vpm __maybe_unused = &ntm->msg.cfg;
	nss_trace("%px: NSS trustsec_rx message: Configure VP\n"
		"VP number: %u\n",
		vpm,
		vpm->num);
}

/*
 * nss_trustsec_rx_log_unconfig_vp_msg
 *	Log NSS TrustSec Rx uncfg vp message
 */
static void nss_trustsec_rx_log_unconfig_vp_msg(struct nss_trustsec_rx_msg *ntm)
{
	struct nss_trustsec_rx_vp_msg *vpm __maybe_unused = &ntm->msg.uncfg;
	nss_trace("%px: NSS trustsec_rx message: Unconfigure VP\n"
		"VP number: %u\n",
		vpm,
		vpm->num);
}

/*
 * nss_trustsec_rx_log_configure_msg()
 *	Log NSS TRUSTSEC_RX configure message.
 */
static void nss_trustsec_rx_log_configure_msg(struct nss_trustsec_rx_msg *ntm)
{
	struct nss_trustsec_rx_configure_msg *cfg __maybe_unused = &ntm->msg.configure;
	nss_trace("%px: NSS trustsec_rx message: Config\n"
		"IP version: %u\n",
		cfg,
		cfg->ip_version);

	if (cfg->ip_version == NSS_TRUSTSEC_RX_FLAG_IPV4) {
		nss_trace("Src IP: %pI4\n"
			"Dest IP: %pI4\n",
			&(cfg->src_ip.ip.ipv4),
			&(cfg->dest_ip.ip.ipv4));
	} else {
		nss_trace("Src IP: %pI6\n"
			"Dest IP: %pI6\n",
			&(cfg->src_ip.ip.ipv6),
			&(cfg->dest_ip.ip.ipv6));
	}

	nss_trace("Src Port: %u\n Dest Port: %u\n\n",
		cfg->src_port, cfg->dest_port);
	nss_trace("%px: Destination interface number: %u", cfg, cfg->dest);
}

/*
 * nss_trustsec_rx_log_unconfigure_msg()
 *	Log NSS TRUSTSEC_RX unconfigure message.
 */
static void nss_trustsec_rx_log_unconfigure_msg(struct nss_trustsec_rx_msg *ntm)
{
	struct nss_trustsec_rx_unconfigure_msg *uncfg __maybe_unused = &ntm->msg.unconfigure;
	nss_trace("%px: NSS trustsec_rx message: Unconfig\n"
		"IP version: %u\n",
		uncfg,
		uncfg->ip_version);

	if (uncfg->ip_version == NSS_TRUSTSEC_RX_FLAG_IPV4) {
		nss_trace("Src IP: %pI4\n"
			"Dest IP: %pI4\n",
			&(uncfg->src_ip.ip.ipv4),
			&(uncfg->dest_ip.ip.ipv4));
	} else {
		nss_trace("Src IP: %pI6\n"
			"Dest IP: %pI6\n",
			&(uncfg->src_ip.ip.ipv6),
			&(uncfg->dest_ip.ip.ipv6));
	}

	nss_trace("Src Port: %u\n Dest Port: %u\n\n",
		uncfg->src_port, uncfg->dest_port);
	nss_trace("%px: Destination interface number: %u", uncfg, uncfg->dest);
}

/*
 * nss_trustsec_rx_log_verbose()
 *	Log message contents.
 */
static void nss_trustsec_rx_log_verbose(struct nss_trustsec_rx_msg *ntm)
{
	switch (ntm->cm.type) {
	case NSS_TRUSTSEC_RX_MSG_CONFIGURE:
		nss_trustsec_rx_log_configure_msg(ntm);
		break;

	case NSS_TRUSTSEC_RX_MSG_UNCONFIGURE:
		nss_trustsec_rx_log_unconfigure_msg(ntm);
		break;

	case NSS_TRUSTSEC_RX_MSG_STATS_SYNC:
		/*
		 * No log for valid stats message.
		 */
		break;

	case NSS_TRUSTSEC_RX_MSG_CONFIG_VP:
		nss_trustsec_rx_log_config_vp_msg(ntm);
		break;

	case NSS_TRUSTSEC_RX_MSG_UNCONFIG_VP:
		nss_trustsec_rx_log_unconfig_vp_msg(ntm);
		break;

	default:
		nss_warning("%px: Invalid message type\n", ntm);
		break;
	}
}

/*
 * nss_trustsec_rx_log_tx_msg()
 *	Log messages transmitted to FW.
 */
void nss_trustsec_rx_log_tx_msg(struct nss_trustsec_rx_msg *ntm)
{
	if (ntm->cm.type >= NSS_TRUSTSEC_RX_MSG_MAX) {
		nss_warning("%px: Invalid message type\n", ntm);
		return;
	}

	nss_info("%px: type[%d]:%s\n", ntm, ntm->cm.type, nss_trustsec_rx_log_message_types_str[ntm->cm.type]);
	nss_trustsec_rx_log_verbose(ntm);
}

/*
 * nss_trustsec_rx_log_rx_msg()
 *	Log messages received from FW.
 */
void nss_trustsec_rx_log_rx_msg(struct nss_trustsec_rx_msg *ntm)
{
	if (ntm->cm.response >= NSS_CMN_RESPONSE_LAST) {
		nss_warning("%px: Invalid response\n", ntm);
		return;
	}

	if (ntm->cm.response == NSS_CMN_RESPONSE_NOTIFY || (ntm->cm.response == NSS_CMN_RESPONSE_ACK)) {
		nss_info("%px: type[%d]:%s, response[%d]:%s\n", ntm, ntm->cm.type,
			nss_trustsec_rx_log_message_types_str[ntm->cm.type],
			ntm->cm.response, nss_cmn_response_str[ntm->cm.response]);
		goto verbose;
	}

	if (ntm->cm.error >= NSS_TRUSTSEC_RX_ERR_UNKNOWN) {
		nss_warning("%px: msg failure - type[%d]:%s, response[%d]:%s, error[%d]:Invalid error\n",
			ntm, ntm->cm.type, nss_trustsec_rx_log_message_types_str[ntm->cm.type],
			ntm->cm.response, nss_cmn_response_str[ntm->cm.response],
			ntm->cm.error);
		goto verbose;
	}

	nss_info("%px: msg nack - type[%d]:%s, response[%d]:%s, error[%d]:%s\n",
		ntm, ntm->cm.type, nss_trustsec_rx_log_message_types_str[ntm->cm.type],
		ntm->cm.response, nss_cmn_response_str[ntm->cm.response],
		ntm->cm.error, nss_trustsec_rx_log_error_response_types_str[ntm->cm.error]);

verbose:
	nss_trustsec_rx_log_verbose(ntm);
}

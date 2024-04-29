/*
 **************************************************************************
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
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
 **************************************************************************
 */

/*
 * nss_ppe_vp_log.c
 *	NSS PPE logger file.
 */

#include "nss_core.h"
#include "nss_ppe_vp.h"

/*
 * nss_ppe_vp_log_message_types_str
 *	PPE message strings
 */
static int8_t *nss_ppe_vp_log_message_types_str[NSS_PPE_VP_MSG_MAX] __maybe_unused = {
	"PPE VP Config",
	"PPE VP Stats",
};

/*
 * nss_ppe_vp_log_error_response_types_str
 *	Strings for error types for PPE-VP messages
 */
static int8_t *nss_ppe_vp_log_error_response_types_str[NSS_PPE_VP_MSG_ERROR_TYPE_MAX] __maybe_unused = {
	"PPE VP Unknown message type",
	"PPE VP Invalid dynamic interface type",
};

/*
 * nss_ppe_vp_log_config_msg()
 *	Log NSS PPE VP configuration message.
 */
static void nss_ppe_vp_log_config_msg(struct nss_ppe_vp_msg *npvm)
{
	struct nss_ppe_vp_config_msg *npcm __maybe_unused = &npvm->msg.vp_config;
	nss_trace("%px: NSS PPE VP configuration message:\n"
		"Dynamic interface type: %d is_vp_support_enable: %d\n",
		npcm, npcm->type, npcm->vp_enable);
}

/*
 * nss_ppe_vp_log_verbose()
 *	Log message contents.
 */
static void nss_ppe_vp_log_verbose(struct nss_ppe_vp_msg *npvm)
{
	switch (npvm->cm.type) {
	case NSS_PPE_VP_MSG_CONFIG:
		nss_ppe_vp_log_config_msg(npvm);
		break;

	case NSS_PPE_VP_MSG_SYNC_STATS:
		/*
		 * No log for valid stats message.
		 */
		break;

	default:
		nss_warning("%px: Invalid message type\n", npvm);
		break;
	}
}

/*
 * nss_ppe_vp_log_tx_msg()
 *	Log messages transmitted to FW.
 */
void nss_ppe_vp_log_tx_msg(struct nss_ppe_vp_msg *npvm)
{
	if (npvm->cm.type >= NSS_PPE_VP_MSG_MAX) {
		nss_warning("%px: Invalid message type\n", npvm);
		return;
	}

	nss_info("%px: type[%d]:%s\n", npvm, npvm->cm.type, nss_ppe_vp_log_message_types_str[npvm->cm.type]);
	nss_ppe_vp_log_verbose(npvm);
}

/*
 * nss_ppe_vp_log_rx_msg()
 *	Log messages received from FW.
 */
void nss_ppe_vp_log_rx_msg(struct nss_ppe_vp_msg *npvm)
{
	if (npvm->cm.response >= NSS_CMN_RESPONSE_LAST) {
		nss_warning("%px: Invalid response\n", npvm);
		return;
	}

	if (npvm->cm.response == NSS_CMN_RESPONSE_NOTIFY || (npvm->cm.response == NSS_CMN_RESPONSE_ACK)) {
		nss_info("%px: type[%d]:%s, response[%d]:%s\n", npvm, npvm->cm.type,
			nss_ppe_vp_log_message_types_str[npvm->cm.type],
			npvm->cm.response, nss_cmn_response_str[npvm->cm.response]);
		goto verbose;
	}

	if (npvm->cm.error >= NSS_PPE_VP_MSG_ERROR_TYPE_MAX) {
		nss_warning("%px: msg failure - type[%d]:%s, response[%d]:%s, error[%d]:Invalid error\n",
			npvm, npvm->cm.type, nss_ppe_vp_log_message_types_str[npvm->cm.type],
			npvm->cm.response, nss_cmn_response_str[npvm->cm.response],
			npvm->cm.error);
		goto verbose;
	}

	nss_info("%px: msg nack - type[%d]:%s, response[%d]:%s, error[%d]:%s\n",
		npvm, npvm->cm.type, nss_ppe_vp_log_message_types_str[npvm->cm.type],
		npvm->cm.response, nss_cmn_response_str[npvm->cm.response],
		npvm->cm.error, nss_ppe_vp_log_error_response_types_str[npvm->cm.error]);

verbose:
	nss_ppe_vp_log_verbose(npvm);
}
